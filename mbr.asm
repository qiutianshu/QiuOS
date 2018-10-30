;The struct of Descriptor
;	dd	base
;	dd	limit(低20位有效)
;	dw	attribute（低12位有效）

%macro Descriptor 3
	dw	%2 & 0xffff		;段界限1
	dw	%1 & 0xffff		;段基址1
	db	(%1>>16) & 0xff		;段基址2
	db	%3 & 0xff		;0-3 Type 类型，4 S（0系统段，1代码段或者数据段），5-6 DPL 特权级，7 P （1表示在内存中，0表示不在内存中）
	db	((%2>>16) & 0xf) | (%3>>4) & 0xf0	;0-3段界限2，4 AVL，6 D/B，7 G
	db	(%1>>24) &0xff		;段基址3
%endmacro

	org 0x7c00
	jmp BEGIN 

;段描述符
[SECTION.gdt]
;				段基址			段界限			段属性
GDT:		Descriptor        0,                      0,                      0		;空描述符
CODE32：	Descriptor	  0, 		         Code32Len                                       ;32位代码段
VIDEO:		Descriptor	  0xb8000,		  0xffff,				;显存

;GDT寄存器
GdtPtr		dw	$-GDT-1		;GDT界限
		dd	0		;GDT基地址(16位模式下低20位有效)

;段选择子
SelectorCode32		equ	 CODE32	-	GDT
SelectorVideo		equ	 VIDEO	-	GDT

[SECTION.code16]
[BITS 16]
BEGIN:
	mov ax,cs
	mov ds,ax
	mov es,ax
	mov ss,ax

;设置32位代码段描述符
	xor eax,eax
	mov ax,cs
	shl eax,4
	add eax,SEG_CODE32	;计算32位代码段基地址
	mov word [CODE32+2],ax	;代码段基址1
	shr eax,16
	mov byte [CODE32+4],al	;代码段基址2
	mov byte [CODE32+7],ah	;代码段基址3

;设置GDT寄存器	
	xor eax,eax
	mov ax,ds
	shl eax,4
	add eax,GDT
	mov dword [GdtPtr+2],eax
	
;加载GDT
	lgdt [GdtPtr]	

;关中断
	cli

;打开A20地址线
	in al,92h
	or al,00000010b
	out 92h,al

;设置CR0寄存器
	mov eax,cr0
	or  eax,1
	mov cr0,eax

;进入保护模式
	jmp dword SelectorCode32:0	;远跳转刷新cs寄存器进入32位代码段
	

[SECTION.code32]
[BITS 32]
SEG_CODE32:
	mov ax,SelectorVideo
	mov gs,ax			;装入视频段选择子
	mov edi,(80*10+0)*2		;第10行开始显示
	mov ah,0x0a			;绿字体
	mov al,'Q'
	mov dword [gs:edi],ax
	jmp $



Code32Len	equ	$-SEG_CODE32



