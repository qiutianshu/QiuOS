extern disp_pos

[SECTION .text]

global disp_str
global disp_color_str
global out_byte
global in_byte
global disable_int
global enable_int 
global port_read
global port_write
;--------------------------------------------------------------------------------------
;				void disp_str(char* str)
;--------------------------------------------------------------------------------------
disp_str:								;显示字符串
	push ebx
	push edi
	push esi

	mov esi,[esp+0x10]					;指向字符串地址
	mov edi,[disp_pos]
	mov ah,0x0a

	cld									;正向传输
.3:
	lodsb
	test al,al
	jz .1
	cmp al,0xa							;回车
	jne .2
	push eax
	mov eax,edi
	mov bl,160
	div bl
	and eax,0xff						;取行数
	inc eax
	mov bl,160
	mul bl
	mov edi,eax
	pop eax
	jmp .3
.2:
	mov [gs:edi],ax
	add edi,2
	jmp .3
.1:
	mov [disp_pos],edi
	pop esi
	pop edi
	pop ebx
	ret

;--------------------------------------------------------------------------------------
;				void disp_color_str(char* str,int color)
;--------------------------------------------------------------------------------------
disp_color_str:								;显示字符串
	push ebx
	push edi
	push esi

	mov esi,[esp+0x10]					;指向字符串地址
	mov edi,[disp_pos]
	mov ah,[esp+0x14]					;字体颜色

	cld									;正向传输
.3:
	lodsb
	test al,al
	jz .1
	cmp al,0xa							;回车
	jne .2
	push eax
	mov eax,edi
	mov bl,160
	div bl
	and eax,0xff						;取行数
	inc eax
	mov bl,160
	mul bl
	mov edi,eax
	pop eax
	jmp .3
.2:
	mov [gs:edi],ax
	add edi,2
	jmp .3
.1:
	mov [disp_pos],edi
	pop esi
	pop edi
	pop ebx
	ret

;------------------------------------------------------------------------------------------
;					void out_byte(u16 port, u8 value)
;------------------------------------------------------------------------------------------
out_byte:
	mov edx,[esp+4]		;端口号
	mov al,[esp+8]		;值
	out dx,al
	nop
	nop
	ret

;-------------------------------------------------------------------------------------------
;					u8 in_byte(u16 port)
;-------------------------------------------------------------------------------------------
in_byte:
	mov edx,[esp+4]		;端口号
	xor eax,eax
	in al,dx
	nop
	nop
	ret

;----------------------------------------------------------------------------------------------
;		void disable_int()
;----------------------------------------------------------------------------------------------
disable_int:
	cli
	ret

;---------------------------------------------------------------------------------------------
;		void enable_int()
;---------------------------------------------------------------------------------------------
enable_int:
	sti 
	ret

;---------------------------------------------------------------------------------------------
;		void port_read(int port, char* buf, int size)
;---------------------------------------------------------------------------------------------
port_read:
	mov edx, [esp + 4]				;port
	mov edi, [esp + 8]				;dest
	mov ecx, [esp + 12]				;size
	shr ecx, 1						;字 
	cld
	rep insw
	ret

;--------------------------------------------------------------------------------------------
;		void port_write(int port, char* buf, int size)
;--------------------------------------------------------------------------------------------
port_write:
	mov edx, [esp + 4]				;port
	mov esi, [esp + 8]				;buf
	mov ecx, [esp + 12]				;size
	shr ecx, 1
	cld
	rep outsw
	ret