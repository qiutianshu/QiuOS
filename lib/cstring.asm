[section .text]

global memcpy     ;导出的函数符号
global memset
global strcpy

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

;-------------------------------------------------------------------------------------------
;	void memset(void* dest,char chr,int size)
;-------------------------------------------------------------------------------------------
memset:
	push ebp
	mov ebp,esp
	push ecx
	push edi
	push ebx

	mov ecx,[ebp+16]		;size
	mov ebx,[ebp+12]			;char
	mov edi,[ebp+8]			;dest
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

;-----------------------------------------------------------------------------------------------
;	void strcpy(void* dest, void* src)
;-----------------------------------------------------------------------------------------------
strcpy:
	push ebp
	push esi
	push edi
	mov ebp,esp

	mov esi,[ebp+20]
	mov edi,[ebp+16]
.1:
	mov al,[esi]
	mov byte [edi],al
	inc esi
	inc edi
	cmp al,0
	jne .1					;不为 ‘\0’

	pop edi
	pop esi
	pop ebp
	ret
