%include "pm.inc"

	org 0x100
	jmp BEGIN 

;段描述符
[SECTION .gdt]
;								段基址			段界限					段属性
GDT:		Descriptor       	 0,             0,                      0					;空描述符
CODE32:		Descriptor	 		 0, 		    Code32Len - 1,          0x98 + 0x4000  		;32位代码段
CODE16:		Descriptor			 0,			 	0xffff,		 			0x98				;16位代码段
VIDEO:		Descriptor	  		 0xb8000,		0xffff,		 			0x93+0x60			;显存
TEST:		Descriptor	  		 0x500000,	    0xffff,		 			0x92				;测试段
DATA:		Descriptor	  		 0,			 	DataLen - 1,		 	0x92+0x60			;数据段
STACK:		Descriptor	  		 0,			 	TopOfStack - 1,	 		0x92 + 0x4000  		;32位栈段	
NORMAL:		Descriptor	    	 0,			 	0xffff,		 			0x92				;普通段
LDT:		Descriptor	  		 0,			 	LdtLen-1,		 		0x82				;LDT段
STACKRING3:	Descriptor			 0,				TopOfStackR3-1,			0x4000+0x93+0x60	;R3栈段
CODE_RING3:	Descriptor			 0,   			CodeR3Len-1,			0x4000+0x98+0x60	;R3代码段
TSS 		Descriptor			 0,				TSSLen-1,				0X89 				;TSS段
CODE_SEG_RING0:	Descriptor       0,				CodeRing0Len-1,			0x4000+0X98         ;Ring0代码段

;调用门							段选择子			偏移						参数个数				属性
CODE_R3_GATE:	Gate        SelectorCodeRing0,	0,						0,					0x8c+0x60


;GDT寄存器
GdtPtr		dw	$-GDT-1																		;GDT界限
			dd	0																			;GDT基地址(16位模式下低20位有效)

;段选择子
SelectorCode32		equ	 CODE32	-	GDT
SelectorCode16		equ	 CODE16 - 	GDT
SelectorVideo		equ	 VIDEO	-	GDT
SelectorTest		equ	 TEST   -   GDT
SelectorStack		equ	 STACK  -	GDT
SelectorData		equ	 DATA	-	GDT
SelectorNormal		equ	 NORMAL	-	GDT
SelectorLdt		    equ	 LDT	-	GDT
SelectorStackR3		equ	 STACKRING3 - GDT + 0x3         
SelectorCodeR3		equ	 CODE_RING3 - GDT + 0x3
SelectorTss			equ	 TSS - GDT
SelectorCodeRing0	equ	 CODE_SEG_RING0 - GDT

SelectorCallGate	equ	CODE_R3_GATE - GDT + 0x3

;LDT描述符	
[SECTION .ldt]
;						基地址			段界限			段属性
LABEL_LDT_DEST:
LABEL_LDT:	Descriptor	0,			LocalTaskLen-1,		0x4000+0x98							;32位非一致代码段

LdtLen		equ		$ - LABEL_LDT

;LDT段选择子
SelectorLocalTask	equ	LABEL_LDT - LABEL_LDT_DEST + 0100b

[SECTION .data]
LABEL_DATA:
	Message1			db 	"Protect Mode",0
	offsetMessage1		equ	Message1 - $$
	Message2			db	"Real Mode",0
	offsetMessage2		equ	Message2 - $$
	Message3			db	"abcdefgh",0
	offsetMessage3		equ	Message3 - $$
	Message4			db	"In Local Task",0
	offsetMessage4		equ	Message4 - $$
	Message5			db	"Ring0 -> Ring3",0
	offsetMessage5		equ	Message5 - $$
	Message6 			db  "Ring3 -> Ring0",0
	offsetMessage6		equ	Message6 - $$
	DataLen				equ	$ - LABEL_DATA


[SECTION .stack]
LABEL_STACK:
	times	512	db	0
	TopOfStack		equ	$ - LABEL_STACK	

[SECTION .stack_r3]
LABEL_STACK_RING3:
	times   512 db  0
	TopOfStackR3      equ $ - LABEL_STACK_RING3

[SECTION .tss]
ALIGN	32
[BITS	32]
LABEL_TSS:
		DD	0			    ; Back
		DD	TopOfStack		; 0 级堆栈
		DD	SelectorStack	
		DD	0		     	; 1 级堆栈
		DD	0		    	; 
		DD	0		    	; 2 级堆栈
		DD	0		    	; 
		DD	0				; CR3
		DD	0				; EIP
		DD	0				; EFLAGS
		DD	0				; EAX
		DD	0				; ECX
		DD	0				; EDX
		DD	0				; EBX
		DD	0				; ESP
		DD	0				; EBP
		DD	0				; ESI
		DD	0				; EDI
		DD	0				; ES
		DD	0				; CS
		DD	0				; SS
		DD	0				; DS
		DD	0				; FS
		DD	0				; GS
		DD	0				; LDT
		DW	0				; 调试陷阱标志
		DW	$ - LABEL_TSS + 2	; I/O位图基址
		DB	0ffh			; I/O位图结束标志
TSSLen		equ	$ - LABEL_TSS

[SECTION .code16]
[BITS 16]
BEGIN:
	mov ax,cs
	mov ds,ax
	mov es,ax
	mov ss,ax
	mov [GO_BACK_TO_REAL+3],cs

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
	xor eax,eax
	mov ax,cs
	shl eax,4
	add eax,SEG_CODE16
	mov word [CODE16+2],ax
	shr eax,16
	mov byte [CODE16+4],al
	mov byte [CODE16+7],ah

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
	mov ax,ss
	shl eax,4
	add eax,LABEL_STACK
	mov word [STACK+2],ax
	shr eax,16
	mov byte [STACK+4],al
	mov byte [STACK+7],ah

;设置LDT在GDT中的描述符
	xor eax,eax
	mov ax,cs
	shl eax,4
	add eax,LABEL_LDT
	mov word [LDT+2],ax
	shr eax,16
	mov byte [LDT+4],al	
	mov byte [LDT+7],ah

;设置LDT描述符
	xor eax,eax
	mov ax,cs
	shl eax,4
	add eax,LOCAL_TASK
	mov word [LABEL_LDT+2],ax
	shr eax,16
	mov byte [LABEL_LDT+4],al
	mov byte [LABEL_LDT+7],ah

;设置Ring3代码段
	xor eax,eax
	mov ax,cs
	shl eax,4
	add eax,CODE32_RING3
	mov word [CODE_RING3+2],ax
	shr eax,16
	mov byte [CODE_RING3+4],al
	mov byte [CODE_RING3+7],ah

;设置Ring3栈段
	xor eax,eax
	mov ax,ss
	shl eax,4
	add eax,LABEL_STACK_RING3
	mov word [STACKRING3+2],ax
	shr eax,16
	mov byte [STACKRING3+4],al
	mov byte [STACKRING3+7],ah

;设置TSS段
	xor eax,eax
	mov ax,ds
	shl eax,4
	add eax,LABEL_TSS
	mov word [TSS+2],ax
	shr eax,16
	mov byte [TSS+4],al
	mov byte [TSS+7],ah

;设置Ring0代码段
	xor eax,eax
	mov ax,cs
	shl eax,4
	add eax,CODE_RING0
	mov word [CODE_SEG_RING0+2],ax
	shr eax,16
	mov byte [CODE_SEG_RING0+4],al
	mov byte [CODE_SEG_RING0+7],ah


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
	jmp dword SelectorCode32:0						;远跳转刷新cs寄存器进入32位代码段
	
[SECTION .code32]
[BITS 32]
SEG_CODE32:
	mov ax,SelectorData								;装入段选择子
	mov ds,ax
	mov ax,SelectorStack
	mov ss,ax
	mov ax,SelectorVideo
	mov gs,ax
	mov ax,SelectorTest
	mov es,ax
	mov esp,TopOfStack-1

	xor edi,edi
	xor esi,esi
	mov esi,offsetMessage1
	mov edi,(80*10+0)*2
	mov ah,0x0a										;绿色字体
	cld												;正向传输
.1:
	lodsb
;	call DispAl
	test al,al
	jz .2
	mov [gs:edi],ax
	add edi,2
	jmp .1

.2:
	call NextLine	
	call Read
	call Write
	call Read

;	jmp SelectorCode16:0							;恢复cs高速缓冲器
;	mov ax,SelectorLdt
;	lldt ax
;	jmp SelectorLocalTask:0
	
	mov ax,SelectorTss
	ltr ax
	
	push SelectorStackR3
	push TopOfStackR3-1
	push SelectorCodeR3
	push 0
	retf

%include "libc.inc"

Code32Len	equ	$-SEG_CODE32

[SECTION .s16]
[BITS 16]
;跳回实模式
SEG_CODE16:
	mov ax,SelectorNormal								;恢复数据段描述符高速缓存器
	mov ds,ax
	mov es,ax
	mov gs,ax
	mov ss,ax

	mov eax,cr0
	and al,011111110b
	mov cr0,eax

GO_BACK_TO_REAL:
	jmp 0:REAL_ENTRY

REAL_ENTRY:
	mov ax,cs											;恢复段寄存器
	mov ds,ax
	mov es,ax
	mov gs,ax
	mov ss,ax

	in al,92h											;打开A20地址线
	and al,011111101b
	out 92h,al

	sti													;开中断
	mov ax,0x4c00
	int 21h


[SECTION .localtask]									;局部任务代码段
[BITS 32]
ALIGN 32
LOCAL_TASK:
	mov ax,SelectorVideo
	mov gs,ax
	mov edi,(80*0)*2
	mov ah,0x0c
	xor esi,esi
	mov esi,offsetMessage4
	cld
.1:
	lodsb
	test al,al
	jz .2												;跳回16位代码段返回实模式
	mov [gs:edi],ax
	add edi,2
	jmp .1
.2:
	jmp SelectorCode16:0

LocalTaskLen	equ	$ - LOCAL_TASK

[SECTION .Ring3]									    ;局部任务代码段
[BITS 32]
ALIGN 32
CODE32_RING3:
	mov ax,SelectorVideo
	mov gs,ax
	mov ax,SelectorData
	mov ds,ax
	mov edi,(80*0)*2
	mov ah,0x0c
	xor esi,esi
	mov esi,offsetMessage5

	cld
.1:
	lodsb
	test al,al
	jz .2
	mov [gs:edi],ax
	add edi,2
	jmp .1
.2:
	call SelectorCallGate:0

CodeR3Len     equ      $ - CODE32_RING3

[SECTION .code_ring0]									;RING0代码段
[BITS 32]
ALIGN 32
CODE_RING0:
	mov ax,SelectorVideo
	mov gs,ax
	mov edi,(80*1)*2
	mov ah,0x0c
	xor esi,esi
	mov esi,offsetMessage6
	cld
.1:
	lodsb
	test al,al
	jz .2												;跳回16位代码段返回实模式
	mov [gs:edi],ax
	add edi,2
	jmp .1
.2:
	jmp SelectorCode16:0

CodeRing0Len	equ	$ - CODE_RING0























