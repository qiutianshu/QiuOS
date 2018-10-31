;The struct of Descriptor
;	dd	base
;	dd	limit(低20位有效)
;	dw	attribute（低12位有效）

%macro Descriptor 3
	dw	%2 & 0xffff				;段界限1
	dw	%1 & 0xffff				;段基址1
	db	(%1>>16) & 0xff				;段基址2
	dw	((%2 >> 8) & 0F00h) | (%3 & 0F0FFh)	;属性     8-11位无效
	db	(%1>>24) &0xff				;段基址3
%endmacro

	org 0x7c00
	jmp BEGIN 

;段描述符
[SECTION .gdt]
;				段基址			段界限			段属性
GDT:		Descriptor        0,                     0,                      0		;空描述符
CODE32:		Descriptor	  0, 		         Code32Len - 1,          0x98 + 0x4000  ;32位代码段
VIDEO:		Descriptor	  0xb8000,		 0xffff,		 0x92		;显存
TEST:		Descriptor	  0x5000000,	         0xffff,		 0x92		;测试段
DATA:		Descriptor	  0,			 DataLen - 1,		 0x92		;数据段
STACK:		Descriptor	  0,			 TopOfStack - 1		 0x92		;栈段	
NORMAL:		Descriptor	  0,			 0xffff,		 0x92		;普通段

;GDT寄存器
GdtPtr		dw	$-GDT-1									;GDT界限
		dd	0									;GDT基地址(16位模式下低20位有效)

;段选择子
SelectorCode32		equ	 CODE32	-	GDT
SelectorVideo		equ	 VIDEO	-	GDT
SelectorTest		equ	 TEST   -       GDT
SelectorStack		equ	 STACK  -	GDT
SelectorData		equ	 DATA	-	GDT
SelectorNormal		equ	 NORMAL	-	GDT

[SECTION .data]
LABEL_DATA:
	Message1		db 	"Protect Mode"
	offsetMessage1		equ	Message1 - $$
	Message2		db	"Real Mode"
	offsetMessage2		equ	Message2 - $$
	DataLen			equ	$ - LABEL_DATA

[SECTION .stack]
LABEL_STACK:
	times	512	db	0
	TopOfStack		equ	$ - LABEL_STACK	

[SECTION .code16]
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

;设置数据段描述符
	xor eax,eax
	mov ax,ds
	shl eax,4
	add eax,LABEL_DATA
	mov word [DATA+2],ax
	shr eax,16
	mov byte [DATA+4],al
	mov byte [DATA+7],ah

;设置栈段描述符
	xor eax,eax
	mov
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
	
[SECTION .code32]
[BITS 32]
SEG_CODE32:
	mov ax,SelectorVideo
	mov gs,ax			;装入视频段选择子
	mov edi,(80*10+0)*2		;第10行开始显示
	mov ah,0x0c			;绿字体
	mov al,'Q'
	mov [gs:edi],ax
	jmp $

Code32Len	equ	$-SEG_CODE32


