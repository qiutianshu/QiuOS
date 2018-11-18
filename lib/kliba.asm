extern disp_pos

[SECTION .text]

global disp_str
global disp_color_str
global memcpy
global out_byte
global memset
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

;-----------------------------------------------------------------------------------------
;			void memcpy(void* es:Dest, void* ds:Src, int size)
;-----------------------------------------------------------------------------------------
memcpy:		
	push ebp
	mov ebp,esp
	push edi
	push esi
	push ecx

	mov edi,[ebp+8]		;目的地址
	mov esi,[ebp+12]	;源地址
	mov ecx,[ebp+16]	;长度

.1:
	cmp ecx,0
	jz .2
	mov al,[ds:esi]
	mov byte [es:edi],al
	inc edi
	inc esi
	dec ecx
	jmp .1
.2:
	mov eax,[ebp+8]		;返回目的地址
	pop ecx
	pop esi
	pop edi
	pop ebp

	ret

;------------------------------------------------------------------------------------------
;					void out_byte(u16 port, u8 value);
;------------------------------------------------------------------------------------------
out_byte:
	mov edx,[esp+4]		;端口号
	mov al,[esp+8]		;值
	out dx,al
	nop
	nop
	ret

;-------------------------------------------------------------------------------------------
;	void memset(void* dest,char chr,int size)
;-------------------------------------------------------------------------------------------
memset:
	push ebp
	mov ebp,esp
	push ecx
	push edi
	push ebx

	mov ecx,[ebp+12]		;size
	mov ebx,[ebp+8]			;char
	mov edi,[ebp+4]			;dest
.1:
	cmp ecx,0
	je .2
	mov byte [edi],bl
	inc edi
	dec ecx
	jmp .1
.2:
	pop ebx
	pop edi
	pop ecx
	pop ebp
	ret