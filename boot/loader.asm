	org 0x100

	times 512 nop
	mov ax,0xb800
	mov gs,ax
	mov ah,0xc
	mov al,"L"
	mov [gs:((80*1+0)*2)],ax
	mov al,"o"
	mov [gs:((80*1+1)*2)],ax
	mov al,"a"
	mov [gs:((80*1+2)*2)],ax
	mov al,"d"
	mov [gs:((80*1+3)*2)],ax
	mov al,"e"
	mov [gs:((80*1+4)*2)],ax
	mov al,"d"
	mov [gs:((80*1+5)*2)],ax
	mov al," "
	mov [gs:((80*1+6)*2)],ax
	mov al,"S"
	mov [gs:((80*1+7)*2)],ax
	mov al,"u"
	mov [gs:((80*1+8)*2)],ax
	mov al,"c"
	mov [gs:((80*1+9)*2)],ax
	mov al,"c"
	mov [gs:((80*1+10)*2)],ax
	mov al,"e"
	mov [gs:((80*1+11)*2)],ax
	mov al,"s"
	mov [gs:((80*1+12)*2)],ax
	mov al,"s"
	mov [gs:((80*1+13)*2)],ax
	mov al,"f"
	mov [gs:((80*1+14)*2)],ax
	mov al,"u"
	mov [gs:((80*1+15)*2)],ax
	mov al,"l"
	mov [gs:((80*1+16)*2)],ax
	mov al,"l"
	mov [gs:((80*1+17)*2)],ax
	mov al,"y"
	mov [gs:((80*1+18)*2)],ax
	mov al,"!"
	mov [gs:((80*1+19)*2)],ax
	
	jmp $
