DispInt:											;16进制显示一个整形数
	push eax
	mov eax,[esp+8]
	shr eax,24
	call DispAl

	mov eax,[esp+8]
	shr eax,16
	call DispAl

	mov eax,[esp+8]
	shr eax,8
	call DispAl

	mov eax,[esp+8]
	call DispAl

	push edi
	mov ah,0xa
	mov al,"h"
	mov edi,[dwDispPos]
	mov [gs:edi],ax
	add edi,4
	mov [dwDispPos],edi

	pop edi
	pop eax

	ret


DispAl:												;显示AL中的内容
	push eax
	push ecx
	push edx
	push edi

	mov edi,[dwDispPos]
	mov ecx,2
	mov ah,0xa
	mov dl,al										;al的副本
	shr al,4
.3:
	and al,01111b
	cmp al,9
	ja .1
	add al,"0"
	jmp .2
.1:
	sub al,0xa
	add al,"A"
.2:
	mov [gs:edi],ax
	add edi,2
	dec ecx
	jz .4
	mov al,dl
	jmp .3
.4:
	mov [dwDispPos],edi
	pop edi
	pop edx
	pop ecx
	pop eax
	ret
	
DispStr:								;显示字符串
	push ebx
	push edi
	push esi

	mov esi,[esp+0x10]					;指向字符串地址
	mov edi,[dwDispPos]
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
	mov [dwDispPos],edi
	pop esi
	pop edi
	pop ebx
	ret

DispReturn:								;换行
	push szReturn
	call DispStr
	add esp,4
	ret


MemCpy:		
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
;	mov esp,ebp
	pop ebp

	ret
