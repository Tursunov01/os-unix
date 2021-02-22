.386p

descr struc
	lim 	dw 0
	base_l 	dw 0
	base_m 	db 0
	attr_1	db 0
	attr_2	db 0
	base_h 	db 0
descr ends

int_descr struc
	offs_l 	dw 0
	sel		dw 0
	counter db 0
	attr	db 0
	offs_h 	dw 0
int_descr ends


; Protected mode
PM_seg	SEGMENT PARA PUBLIC 'CODE' USE32
	    ASSUME CS:PM_seg
	; в защищенном режиме адресация идет по сегментам	
  	GDT	label byte
  	gdt_null	descr <>
  	gdt_flatDS	descr <0FFFFh,0,0,92h,11001111b,0>	; 92h = 10010010b
  	gdt_16bitCS	descr <RM_seg_size-1,0,0,98h,0,0>	; 98h = 10011010b для реального режима
  	gdt_32bitCS	descr <PM_seg_size-1,0,0,98h,01000000b,0>
  	gdt_32bitDS	descr <PM_seg_size-1,0,0,92h,01000000b,0>
  	gdt_32bitSS	descr <stack_l-1,0,0, 92h, 01000000b,0>
	gdt_vidBf descr <3999,8000h,0bh,92h,01001111b,0>

  	gdt_size = $-GDT
  	gdtr	df 0 ; указывает на начало таблицы gdt

	;с пощью селектора обращаемся к дискриптору(CS, DS). 
	;После у нас появляется базовый адрес сегмента кода или данных ... Они ссылаются на GDT->cs/ds
	; И в итоге получаю линейный физический адрес 
	;адресация с третьего бита -> 2**3 
    SEL_flatDS	equ   8
    SEL_16bitCS	equ   16
    SEL_32bitCS	equ   24
    SEL_32bitDS	equ   32
    SEL_32bitSS	equ   40
	SEL_videoBf equ   48

    IDT	label	byte
	; 13 икслючение - общая защита при нарушении прав доступа
	; первые 32 дискриптора отведены под исключение. int08 int09
	; Базовый вектор занимает 32 бита в защ режиме(8 бит в реальном)
	int_descr 13 dup (<0, SEL_32bitCS,0, 8Eh, 0>)
	def_exp int_descr <0, SEL_32bitCS,0, 8Eh, 0>
	int_descr 18 dup (<0, SEL_32bitCS,0, 8Eh, 0>)
    int08 int_descr <0, SEL_32bitCS,0, 8Eh, 0>
    int09 int_descr	<0, SEL_32bitCS,0, 8Eh, 0>
	
    idt_size = $-IDT ;
    idtr		df 0
    idtr_real 	dw 3FFh,0,0 ; содержимое регистра IDTR в реальном режиме

    master		db 0 ; маска прерываний ведущего контроллера
    slave		db 0 ; маска прерываний ведомого контроллера

    escape		db 0 ; флаг - пора выходить в реальный режим, если == 1
    time_08		dd 0 ; счетчик прошедших тиков таймера

	msg1 db 'In Real Mode now. To move to Protected Mode press any key...$'
	msg2 db 'In Real Mode again!$'
	msg3 db 'P r o t e c t e d _ M o d e'

	; Таблица символов ASCII для перевода из скан кода в код ASCII.
	; Номер скан кода = номеру соответствующего элемента в таблице:
	ASCII_table		db 0, 0, 49, 50, 51, 52, 53, 54, 55, 56, 57, 48, 45, 61, 0, 0
					db 81, 87, 69, 82, 84, 89, 85, 73, 79, 80, 91, 93, 0, 0, 65, 83
					db 68, 70, 71, 72, 74, 75, 76, 59, 39, 96, 0, 92, 90, 88, 67
					db 86, 66, 78, 77, 44, 46, 47
	out_position	dd 1E0h ; Позиция печати вводимого текста

print_str macro str
		mov ah,9
		mov dx, str
		int 21h
endm

;7 -> '7', 15 -> 'F'
create_number macro
		local number1
			cmp dl,10
			jl number1
			add dl,'A' - '0' - 10
		number1:
			add dl,'0'
			mov dh, 00046h ;цвет печати и цвет фона
endm

; макрос печати на экран значения регистра ЕАХ через видеобуффер
my_print_eax macro
		local prcyc1
			push ecx
			push dx
			
			mov ecx,8
			add edi,172
			;add ebp,0B8010h

		prcyc1:
			mov dl,al
			and dl,0Fh
			create_number 0 ; превращаем это число в символ
			mov es:[edi],dx
			ror eax,4
			sub edi,2
			loop prcyc1

			;sub ebp,0B8010h
			pop dx
			pop ecx
endm

; точка входа в 32-битный защищенный режим
PM_entry:  ;загружаем селекторы в сегментные регистры для адресации
		mov	ax,SEL_32bitDS
		mov	ds,ax
		mov	ax,SEL_flatDS
		mov	fs,ax
		mov ax,SEL_videoBf
		mov es,ax
		mov	ax,SEL_32bitSS
		mov	ebx,stack_l
		mov	ss,ax
		mov	esp,ebx
		
		mov esi,offset msg3
		xor edi,edi
		mov edi,0
		push ecx
		mov ecx,29
		rep movsb
		pop ecx
		
		sti  ; Уст. флага маскрируемых прерываний 
		call	compute_memory
		
work:
		test	escape, 1
		jz	work
goback:
		cli
		db	0EAh
		dd	offset RM_return
		dw	SEL_16bitCS
		
new_dev_exp:
	pop EAX
	mov ax, 0Dh
	iretd
	
new_int08:
		push eax
		push ebp
		push ecx
		push dx
		mov  eax,time_08
		mov edi, 2
		my_print_eax 0

		inc eax
		mov time_08,eax

		pop dx
		pop ecx
		pop ebp

		mov	al,20h
		out	20h,al
		pop eax

		iretd

new_int09:
		push eax
		push ebx
		push ebp
		push edx
		
		in	al,60h ; Получаем скан-код нажатой клавиши из порта клавиатуры

		cmp	al,1Ch ; Сравниваем с кодом энтера
		jne	not_leave
		mov escape,1
		jmp leav
not_leave:
		cmp al,80h  ; Сравним какой скан-код пришел: нажатой клавиши или отжатой?
		ja leav 	 ; Если отжатой, то ничего не выводим
		xor ah,ah	 ; Если нажатой, то выведем на экран
		mov bp,ax
		mov dl,ASCII_table[ebp]
		
		mov ebx,out_position
		mov es:[ebx],dl

		add ebx,2
		mov out_position,ebx

leav:
		in	al,61h  ; сброс прерывания от клавиатуры
		or	al,80h
		out	61h,al

		mov	al,20h
		out	20h,al

		pop edx
		pop ebp
		pop ebx
		pop	eax
		iretd

compute_memory	proc
		push	ds
		mov	ax, SEL_flatDS
		mov	ds, ax
		mov	ebx, 100001h
		mov	dl,	10101010b 
		; считываем значение из несуществующего байта памяти вернёт все нули (или все единицы)
		; в каждый байт мы пишем какое-то значение, а потом смотрим, что прочитается
		mov	ecx, 0FFEFFFFEh ;кладем всю оставшую память
check:
		mov	dh, ds:[ebx]
		mov	ds:[ebx], dl
		cmp	ds:[ebx], dl
		jnz	end_of_memory
		mov	ds:[ebx], dh
		inc	ebx
		loop	check
end_of_memory:
		pop	ds
		xor	edx, edx
		mov	eax, ebx
		mov	ebx, 100000h
		div	ebx

		xor edi,edi
		mov edi,20
		my_print_eax 0
		ret
	compute_memory	endp
	PM_seg_size = $-GDT
PM_seg	ENDS

stack_seg	SEGMENT  PARA STACK 'STACK'
	stack_start	db	100h dup(?)
	stack_l = $-stack_start ; длина стека для инициализации ESP
stack_seg 	ENDS

; Real Mode
; USE16 - используем нижние части регистров, АХ ВХ СХ
RM_seg	SEGMENT PARA PUBLIC 'CODE' USE16 
	ASSUME CS:RM_seg, DS:PM_seg, SS:stack_seg
start:
		mov   ax,PM_seg
		mov   ds,ax

		mov ah, 09h
		mov edx, offset msg1
		int 21h

		;ожидаем ввода клавиатуры 
		push eax
		mov ah,10h
		int 16h
		pop eax

		; очистить экран
		mov	ax,3
		int	10h

		push PM_seg
		pop ds

		xor	eax,eax
		mov	ax,RM_seg
		shl	eax,4
		mov	word ptr gdt_16bitCS.base_l,ax
		shr	eax,16
		mov	byte ptr gdt_16bitCS.base_m,al
		mov	ax,PM_seg
		shl	eax,4
		push eax
		push eax
		mov	word ptr GDT_32bitCS.base_l,ax
		mov	word ptr GDT_32bitSS.base_l,ax
		mov	word ptr GDT_32bitDS.base_l,ax
		shr	eax,16
		mov	byte ptr GDT_32bitCS.base_m,al
		mov	byte ptr GDT_32bitSS.base_m,al
		mov	byte ptr GDT_32bitDS.base_m,al

		pop eax
		add	eax,offset GDT
		
		mov	dword ptr gdtr+2,eax
		mov word ptr gdtr, gdt_size-1
		
		lgdt	fword ptr gdtr
		pop	eax
		add	eax,offset IDT
		mov	dword ptr idtr+2,eax
		mov word ptr idtr, idt_size-1

		mov	eax, offset new_dev_exp
		mov	def_exp.offs_l, ax
		shr	eax, 16
		mov	def_exp.offs_h, ax
		
		mov	eax, offset new_int08
		mov	int08.offs_l, ax
		shr	eax, 16
		mov	int08.offs_h, ax
		mov	eax, offset new_int09
		mov	int09.offs_l, ax
		shr	eax, 16
		mov	int09.offs_h, ax

		; сохраним маски прерываний контроллеров
		in	al, 21h
		mov	master, al
		in	al, 0A1h
		mov	slave, al

		mov	al, 11h
		out	20h, al
		mov	AL, 20h
		out	21h, al
		mov	al, 4

		out	21h, al
		mov	al, 1
		out	21h, al
		
		mov	al, 0FCh
		out	21h, al

		mov	al, 0FFh
		out	0A1h, al

		lidt	fword ptr idtr

		in	al,92h
		or	al,2
		out	92h,al

		; отключить маскируемые прерывания
		cli
		in	al,70h
		or	al,80h
		out	70h,al

		; перейти в непосредственно защищенный режим
		mov	eax,cr0
		or	al,1
		mov	cr0,eax

		db	66h
		db	0EAh
		dd	offset PM_entry
		dw	SEL_32bitCS
; теневой регистр - сопоставлен с сегментным регистром. 
;Информация из дескриптора сегмета. Исключает постоянное обращение к памяти
RM_return:
	; переход в реальный режим
		mov	eax,cr0
		and	al,0FEh
		mov	cr0,eax

		db	0EAh ;обновление теневого регистра
		dw	$+4
		dw	RM_seg

		; восстановить регистры для работы в реальном режиме
		mov	ax,PM_seg
		mov	ds,ax
		mov	es,ax
		mov	ax,stack_seg
		mov	bx,stack_l 
		mov	ss,ax
		mov	sp,bx

		;перепрограммируем ведущий контроллер обратно на вектор 8
		mov	al, 11h
		out	20h, al
		mov	al, 8
		out	21h, al
		mov	al, 4
		out	21h, al
		mov	al, 1
		out	21h, al

		;восстанавливаем предусмотрительно сохраненные ранее маски контроллеров прерываний
		mov	al, master
		out	21h, al
		mov	al, slave
		out	0A1h, al

		; загружаем таблицу дескрипторов прерываний реального режима
		lidt	fword ptr idtr_real

		; разрешаем обратно немаскируемые прерывания
		in	al,70h
		and	al,07FH
		out	70h,al
		; а затем маскируемые
		sti
		mov	ax,3
		int	10h
		; печать сообщения о выходе из защищенного
		mov ah, 09h
		mov edx, offset msg2
		int 21h

		mov	ah,4Ch
		int	21h

RM_seg_size = $-start
RM_seg	ENDS
END start