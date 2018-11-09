%include "pm.inc"
	

	org 0x100
	jmp BEGIN 

;段描述符
[SECTION .gdt]
;								段基址			段界限					段属性
GDT:		Descriptor       	 0,             0,                      0					;空描述符
CODE32:		Descriptor	 		 0, 		    Code32Len - 1,          0x9a + 0x4000  		;32位代码段
CODE16:		Descriptor			 0,			 	0xffff,		 			0x98				;16位代码段
VIDEO:		Descriptor	  		 0xb8000,		0xffff,		 			0x93				;显存
DATA:		Descriptor	  		 0,			 	DataLen - 1,		 	0x92				;数据段
STACK:		Descriptor	  		 0,			 	TopOfStack,	 		    0x92 + 0x4000  		;32位栈段	
NORMAL:		Descriptor	    	 0,			 	0xffff,		 			0x92				;普通段

;GDT寄存器
GdtPtr		dw	$-GDT-1																		;GDT界限
			dd	0																			;GDT基地址(16位模式下低20位有效)

[SECTION .idt]
ALIGN 32
[BITS 32]
LABEL_IDT:
;								目标段选择子			偏移					count            属性
%rep 32				
				Gate			SelectorCode32,		SpuriousHandler,	0,				0x8e	;i386中断门
%endrep
.20h:			Gate			SelectorCode32,		ClockHandler,		0,				0x8e
%rep 96
				Gate			SelectorCode32,		SpuriousHandler,	0,				0x8e	;i386中断门
%endrep

IdtLen			equ			$ - LABEL_IDT

IdtPtr			dw     IdtLen - 1				;idt界限
				dd 	   0						;idt基地址
;段选择子
SelectorCode32		equ	 CODE32	-	GDT
SelectorCode16		equ	 CODE16 - 	GDT
SelectorVideo		equ	 VIDEO	-	GDT
SelectorStack		equ	 STACK  -	GDT
SelectorData		equ	 DATA	-	GDT
SelectorNormal		equ	 NORMAL	-	GDT


[SECTION .data]
ALIGN 32
[BITS 32]
SEG_DATA:
_szPMMessage:		db	"In Protect Mode now.", 0Ah, 0Ah, 0								;进入保护模式后显示此字符串
_szMemChkTitle:		db	"BaseAddrL BaseAddrH LengthLow LengthHigh   Type", 0Ah, 0			; 进入保护模式后显示此字符串
_szRAMSize			db	"RAM size:", 0
_szReturn			db	0Ah, 0																;换行符

_wSPValueInRealMode	dw  0
_dwMCRNumber:		dd	0																	;内存块数量
_dwDispPos:			dd	(80 * 6 + 0) * 2	; 屏幕第 6 行, 第 0 列。							;指向下一个显示位置
_dwMemSize:			dd	0		
_PageTableNumber	dd  0															
_ARDStruct:																					;返回结果的数据结构
	_dwBaseAddrLow:		dd	0																
	_dwBaseAddrHigh:	dd	0
	_dwLengthLow:		dd	0
	_dwLengthHigh:		dd	0
	_dwType:			dd	0

_MemChkBuf:	times	256	db	0																;存放int 15h返回结果


szPMMessage 		equ		_szPMMessage - $$
szMemChkTitle		equ		_szMemChkTitle - $$
szRAMSize 			equ		_szRAMSize - $$
szReturn 			equ		_szReturn - $$
dwMCRNumber 		equ		_dwMCRNumber - $$
dwDispPos 			equ     _dwDispPos - $$
dwMemSize 			equ		_dwMemSize - $$
ARDStruct 			equ		_ARDStruct - $$
	dwBaseAddrLow 	equ		_dwBaseAddrLow - $$
	dwBaseAddrHigh  equ		_dwBaseAddrHigh - $$
	dwLengthLow 	equ		_dwLengthLow - $$
	dwLengthHigh 	equ		_dwLengthHigh - $$
	dwType 			equ		_dwType - $$
MemChkBuf 			equ		_MemChkBuf - $$
PageTableNumber     equ     _PageTableNumber - $$

DataLen				equ		$ - SEG_DATA

;全局堆栈
[SECTION .stack]
ALIGN 32
[BITS 32]
SEG_STACK:
times 		512 	db 	0

TopOfStack			equ		$ - SEG_STACK - 1


[SECTION .s16]
[BITS 16]
BEGIN:
	mov ax,cs
	mov ds,ax
	mov es,ax
	mov gs,ax
	mov ss,ax
	mov sp,0x100
	mov [_wSPValueInRealMode],sp

;设置32位代码段描述符
	xor eax,eax
	mov ax,cs
	shl eax,4
	add eax,SEG_CODE32											;计算32位代码段基地址
	mov word [CODE32+2],ax										;代码段基址1
	shr eax,16
	mov byte [CODE32+4],al										;代码段基址2
	mov byte [CODE32+7],ah										;代码段基址3

;设置16位代码段描述符
;	xor eax,eax
;	mov ax,cs
;	shl eax,4
;	add eax,SEG_CODE16
;	mov word [CODE16+2],ax
;	shr eax,16
;	mov byte [CODE16+4],al
;	mov byte [CODE16+7],ah

;设置数据段描述符
	xor eax,eax
	mov ax,ds
	shl eax,4
	add eax,SEG_DATA
	mov word [DATA+2],ax
	shr eax,16
	mov byte [DATA+4],al
	mov byte [DATA+7],ah

;设置栈段描述符
	xor eax,eax
	mov ax,ss
	shl eax,4
	add eax,SEG_STACK
	mov word [STACK+2],ax
	shr eax,16
	mov byte [STACK+4],al
	mov byte [STACK+7],ah

	;设置GDT寄存器	
	xor eax,eax
	mov ax,ds
	shl eax,4
	add eax,GDT
	mov dword [GdtPtr+2],eax

	;设置LDTR寄存器	
	xor eax,eax
	mov ax,ds
	shl eax,4
	add eax,LABEL_IDT
	mov dword [IdtPtr+2],eax
	
;加载GDT
	lgdt [GdtPtr]

;关中断
	cli

;加载IDTR
	lidt [IdtPtr]

;打开A20地址线
	in al,92h
	or al,00000010b
	out 92h,al

;设置CR0寄存器
	mov eax,cr0
	or  eax,1
	mov cr0,eax

	jmp dword SelectorCode32:0

[SECTION .s32]
[BITS 32]
SEG_CODE32:
	mov ax,SelectorData								;装入段选择子
	mov ds,ax
	mov es,ax
	mov ax,SelectorStack
	mov ss,ax
	mov ax,SelectorVideo
	mov gs,ax
	mov esp,TopOfStack

	call Init8259A
	int 0x80
	sti 											;打开中断屏蔽
	jmp $
	
io_delay:
	nop
	nop
	nop
	nop
	ret 

Init8259A:
	mov al,0x11					;主ICW1
	out 0x20,al
	call io_delay

	out 0xa0,al 				;从ICW1
	call io_delay

	mov al,0x20					;主ICW2
	out 0x21,al
	call io_delay

	mov al,0x28					;从ICW2
	out 0xa1,al
	call io_delay

	mov al,0x4 					;主ICW3
	out 0x21,al
	call io_delay

	mov al,0x2 					;从ICW3
	out 0xa1,al
	call io_delay

	mov al,0x1 					;主ICW4
	out 0x21,al
	call io_delay

	out 0xa1,al 				;从ICW4
	call io_delay

	mov al,11111110b			;打开主8259A的时钟中断
	out 0x21,al
	call io_delay

	mov al,11111111b			;屏蔽从8259A所有中断
	out 0xa1,al
	call io_delay

	ret

_SpuriousHandler:
SpuriousHandler   equ  _SpuriousHandler - $$
	mov ah,0xc
	mov al,"0"
	mov [gs:((80*0+0)*2)],ax
	iretd

_ClockHandler:
ClockHandler equ  _ClockHandler - $$			;0-9循环
	inc byte [gs:0]
	mov al,0x20									;发送EOI
	out 0x20,al
	iretd
	


Code32Len 		equ  $ - SEG_CODE32