;参数1=指向字符串的指针，长度默认10,dh指定行号
DispStrInRM:
	push bp
	push es
	mov bp,sp
	mov ax,ds
	mov es,ax
	mov cx,10							;长度
	mov ax,0x1301
	mov bx,0x0007
	mov dl,0							
	mov bp,[bp+6]							;指向字符串的指针
	int 10h

	pop es
	pop bp
	ret