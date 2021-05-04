#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <string.h>

#include "daemonize.h"

#define LOG_FILE "/home/asus/home/projects/os-unix/6-semestr/lab4/part1/log.txt"
#define BUF_SIZE 0x100

struct print_info
{
    char name[NAME_MAX];
    int ignore_newline;
    void (*print_file)(FILE *);
    void (*print_name)(char *);
};

void *get_proc_filename(char *name, int pid, const char *shortname);
void print_proc_file(int pid, struct print_info *info);
void print_proc_stat(FILE *);
void print_proc_statm(FILE *);
void print_proc_mem(FILE *);
void print_proc_fd(char *);
void print_symlink(char *);
void print_fields(FILE *, char **fields);

FILE *f;

int main(int argc, char *argv[])
{
    daemonize("DAEMON");
    // if (already_running())
    // {
    //     syslog(LOG_ERR, "демон уже запущен");
    //     exit(1);
    // }

    f = fopen(LOG_FILE, "w");

    char buf[BUF_SIZE];
    int pid;

    struct print_info files[][8] = {
        {"cmdline"},
        {"cwd", 0, NULL, print_symlink},
        {"environ"},
        // {"exe", 1},
        {"exe", 0, NULL, print_symlink},
        {"fd", 0, NULL, print_proc_fd},
        {"maps"},
        {"mem"},
        {"root", 0, NULL, print_symlink},
        {"stat", 0, print_proc_stat},
        {"statm", 0, print_proc_statm},
    };

    int n_files = sizeof(files) / sizeof(files[0]);

    if (argc != 2 || (pid = atoi(argv[1])) <= 0)
        pid = getpid();

    fprintf(f, "PID = %d\n", pid);

    for (int i = 0; i < n_files; i++)
    {
        print_proc_file(pid, files[i]);
    }
    // getchar();

    fclose(f);

    return 0;
}

void *get_proc_filename(char *name, int pid, const char *shortname)
{
    sprintf(name, "/proc/%d/%s", pid, shortname);
}

void print_proc_file(int pid, struct print_info *info)
{
    fprintf(f, "\n===== %s =====\n", info->name);

    char name[PATH_MAX];

    get_proc_filename(name, pid, info->name);

    // printf("%s", name);

    if (info->print_name)
        return info->print_name(name);

    FILE *file = fopen(name, "r");
    if (!file)
        return;

    if (info->print_file)
    {
        info->print_file(file);
        fclose(file);
        return;
    }

    char buf[BUF_SIZE];
    int len, i;

    while ((len = fread(buf, 1, BUF_SIZE, file)) > 0)
    {
        if (!info->ignore_newline)
            for (i = 0; i < len; i++)
                if (buf[i] == 0)
                    buf[i] = 10; // '\n'

        buf[len] = 0;
        fprintf(f, "%s", buf);
    }

    fclose(file);
}

void print_proc_stat(FILE *file)
{
    static char *fields[] = {
        "pid", "comm", "state", "ppid", "pgrp", "session", "tty_nr", "tpgid",
        "flags", "minflt", "cminflt", "majflt", "cmajflt", "utime", "stime",
        "cutime", "cstime", "priority", "nice", "num_threads", "itrealvalue",
        "starttime", "vsize", "rss", "rsslim", "startcode", "endcode",
        "startstack", "kstkesp", "kstkeip", "signal", "blocked", "sigignore",
        "sigcatch", "wchan", "nswap", "cnswap", "exit_signal", "processor",
        "rt_priority", "policy", "delayacct_blkio_ticks", "guest_time",
        "cguest_time", "start_data", "end_data", "start_brk", "arg_start",
        "arg_end", "env_start", "env_end", "exit_code", NULL};

    print_fields(file, fields);
}

void print_proc_statm(FILE *file)
{
    static char *fields[] = {
        "size", "resident", "shared", "text", "lib", "data", "dt", NULL};

    print_fields(file, fields);
}

void print_proc_fd(char *name)
{
    DIR *dir = opendir(name);
    if (!dir)
    {
        fprintf(stderr, "opendir('%s'): %s\n", name, strerror(errno));
        exit(EXIT_FAILURE);
    }

    int flag = 1;
    char path[PATH_MAX];

    struct dirent *dirp = NULL;
    while (flag && (dirp = readdir(dir)))
    {
        if (strcmp(dirp->d_name, ".") == 0)
        {
            flag = 1;
        }
        else if (strcmp(dirp->d_name, "..") == 0)
        {
            flag = 1;
        }
        else
        {
            snprintf(path, sizeof(path), "%s/%s", name, dirp->d_name);
            print_symlink(path);
        }
    }

    closedir(dir);
}

void print_symlink(char *path)
{
    char str[PATH_MAX];
    const int n = readlink(path, str, sizeof(str));
    str[n] = '\0';

    fprintf(f, "%s\t->\t%s\n", path, str);
}

void print_fields(FILE *file, char **fields)
{
    char buf[BUF_SIZE];
    const size_t len = fread(buf, 1, BUF_SIZE, file);
    buf[len - 1] = '\0';

    char *value, **pfield;

    for (
        value = strtok(buf, " "), pfield = fields;
        value && *pfield; ++pfield)
    {
        fprintf(f, "%-22s %s\n", *pfield, value);
        value = strtok(NULL, " ");
    }
}

void print_proc_mem(FILE *file)
{
    // int mem_fd = open(name, O_RDONLY);
    // ptrace(PTRACE_ATTACH, pid, NULL, NULL);
    // waitpid(pid, NULL, 0);
    // lseek(mem_fd, offset, SEEK_SET);
    // read(mem_fd, buf, _SC_PAGE_SIZE);
    // ptrace(PTRACE_DETACH, pid, NULL, NULL);

    char buf[BUF_SIZE];
    int len, i;

    while ((len = fread(buf, 1, BUF_SIZE, file)) > 0)
    {
        for (i = 0; i < len; i++)
            if (buf[i] == 0)
                buf[i] = 10; // '\n'

        buf[len] = 0;
        fprintf(f, "%s", buf);
    }
}
