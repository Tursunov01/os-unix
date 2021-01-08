#include <stdio.h>
#include <time.h> 
#include <windows.h> 

volatile LONG nr = 0; 
volatile BOOL wrt = 0; 
volatile LONG waiting_readers = 0; 
volatile LONG waiting_writers = 0; 

HANDLE c_read; 
HANDLE c_write; 

HANDLE rwMutex;

#define READERS 5 
#define WRITERS 4 
#define RW_COUNT (READERS + WRITERS) 

#define ITERS 10

#define MAX_READER_SLEEP 3000 
#define MAX_WRITER_SLEEP 1700 

unsigned short int buffer = 0; 

void start_read() 
{ 
    WaitForSingleObject(rwMutex, INFINITE); 
    InterlockedIncrement(&waiting_readers); 
    if (wrt || waiting_writers) 
        WaitForSingleObject(c_read, INFINITE); 
    ResetEvent(c_read); 
    InterlockedDecrement(&waiting_readers); 
    InterlockedIncrement(&nr); 
    SetEvent(c_read);
    ReleaseMutex(rwMutex); 
}
void stop_read() 
{ 
    InterlockedDecrement(&nr); 
    if (nr == 0) 
        SetEvent(c_write); 
} 
void start_write() 
{ 
    WaitForSingleObject(rwMutex, INFINITE); 
    InterlockedIncrement(&waiting_writers); 
    if (nr > 0 || wrt) 
        WaitForSingleObject(c_write, INFINITE); 
    ResetEvent(c_write); 
    InterlockedDecrement(&waiting_writers); 
    wrt = 1;
    ReleaseMutex(rwMutex);
}

void stop_write() 
{ 
    wrt = 0; 
    if (waiting_readers > 0) 
        SetEvent(c_read); 
    else 
        SetEvent(c_write); 
} 
// Читатель 
DWORD reader(LPVOID lpParameter) 
{ 
    int reader_id = *((int *)lpParameter);
    while (buffer < ITERS * WRITERS) 
    { 
        Sleep(rand() % MAX_READER_SLEEP); 
        start_read();
         printf("\tReader #%d read: %d\n", reader_id, buffer); 
         stop_read(); 
    } 
    return 0; 
} 
// Писатель 
DWORD writer(LPVOID lpParameter) 
{ 
    int writer_id = *((int *)lpParameter); 
    for (int i = 0; i < ITERS; i++) 
    { 
        Sleep(rand() % MAX_WRITER_SLEEP); 
        start_write(); 
        ++buffer; 
        printf("Writer #%d wrote: %d\n", writer_id, buffer); 
        stop_write(); 
    } 
    return 0; 
}

int main() 
{ 
    HANDLE threads[RW_COUNT] = {0}; 
    int id[RW_COUNT] = {0}; 
    
    rwMutex = CreateMutex(NULL, FALSE, NULL); 
    if (rwMutex == NULL) 
    { 
        perror("Error: CreateMutex!\n"); 
        return EXIT_FAILURE; 
    } 
    
    c_read = CreateEvent(NULL, FALSE, TRUE, NULL); 
    if (c_read == NULL) 
    { 
        perror("Error: CreateEvent for can_read!\n"); 
        return EXIT_FAILURE; 
    } 
    
    c_write = CreateEvent(NULL, TRUE, TRUE, NULL);
    if (c_write == NULL) 
    { 
        perror("Error: CreateEvent for can_write!\n"); 
        return EXIT_FAILURE; 
    } 
    
    for (int i = 0; i < RW_COUNT; ++i) 
    { 
        if (i < READERS) 
        { 
            id[i] = i; threads[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&reader, (id + i), 0, NULL); 
            if (threads[i] == NULL) 
            { 
                perror("Error: CreateThread readers!\n"); 
                return EXIT_FAILURE; 
            } 
        } 
        else 
        { 
            id[i] = i - READERS; 
            threads[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&writer, (id + i), 0, NULL); 
            if (threads[i] == NULL) 
            { 
                perror("Error: CreateThread writers!\n"); 
                return EXIT_FAILURE; 
            } 
        } 
    } 
    WaitForMultipleObjects(RW_COUNT, threads, TRUE, INFINITE); 
    
    CloseHandle(rwMutex);
    CloseHandle(c_read); 
    CloseHandle(c_write); 
    return EXIT_SUCCESS; 
}