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

	org 0x100
	jmp BEGIN 

;段描述符
[SECTION .gdt]
;				段基址			段界限			段属性
GDT:		Descriptor       	 0,             0,                      0				;空描述符
CODE32:		Descriptor	 		 0, 		    Code32Len - 1,          0x98 + 0x4000  	;32位代码段
CODE16:		Descriptor			 0,			 	0xffff,		 			0x98			;16位代码段
VIDEO:		Descriptor	  		 0xb8000,		0xffff,		 			0x92			;显存
TEST:		Descriptor	  		 0x500000,	    0xffff,		 			0x92			;测试段
DATA:		Descriptor	  		 0,			 	DataLen - 1,		 	0x92			;数据段
STACK:		Descriptor	  		 0,			 	TopOfStack - 1,	 		0x92 + 0x4000  	;32位栈段	
NORMAL:		Descriptor	    	 0,			 	0xffff,		 			0x92			;普通段
LDT:		Descriptor	  		 0,			 	LdtLen-1,		 		0x82			;LDT段

;GDT寄存器
GdtPtr		dw	$-GDT-1							;GDT界限
			dd	0								;GDT基地址(16位模式下低20位有效)

;段选择子
SelectorCode32		equ	 CODE32	-	GDT
SelectorCode16		equ	 CODE16 - 	GDT
SelectorVideo		equ	 VIDEO	-	GDT
SelectorTest		equ	 TEST   -   GDT
SelectorStack		equ	 STACK  -	GDT
SelectorData		equ	 DATA	-	GDT
SelectorNormal		equ	 NORMAL	-	GDT
SelectorLdt		    equ	 LDT	-	GDT

;LDT描述符	
[SECTION .ldt]
;						基地址			段界限			段属性
LABEL_LDT_DEST:
LABEL_LDT:	Descriptor	0,			LocalTaskLen-1,		0x4000+0x98		;32位非一致代码段

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
	DataLen				equ	$ - LABEL_DATA


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
;	mov [GO_BACK_TO_REAL+3],cs

;设置32位代码段描述符
	xor eax,eax
	mov ax,cs
	shl eax,4
	add eax,SEG_CODE32	;计算32位代码段基地址
	mov word [CODE32+2],ax	;代码段基址1
	shr eax,16
	mov byte [CODE32+4],al	;代码段基址2
	mov byte [CODE32+7],ah	;代码段基址3

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
	mov ax,SelectorData		;装入段选择子
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
	mov ah,0x0a			;绿色字体
	cld				;正向传输
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

;	jmp SelectorCode16:0		;恢复cs高速缓冲器
	mov ax,SelectorLdt
	lldt ax

	jmp SelectorLocalTask:0
	

;函数定义
NextLine:				;换行显示    edi始终指向要显示的下一行
	push eax
	push ebx
	xor eax,eax
	mov eax,edi
	mov bl,160
	div bl				;al存放行号
	and eax,0xff			;取得行号
	add eax,1			;下一行
	mov bl,160
	mul bl
	mov edi,eax
	pop ebx
	pop eax
	ret

Read:					;读取8字节
	xor esi,esi
	mov cx,8
.1:
	mov al,[es:esi]
	call DispAl
	dec cx
	jz .2
	inc esi
	jmp .1

.2:
	call NextLine
	ret

DispAl:
	push eax
	push ecx
	push edx
	mov ecx,2
	mov ah,0xa
	mov dl,al			;al的副本
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
	pop edx
	pop ecx
	pop eax
	ret

Write:
	push esi
	push edi
	push eax
	xor esi,esi
	xor edi,edi
	mov esi,offsetMessage3
	cld
.1:
	lodsb
	test al,al
	jz .end
	mov [es:edi],al
	inc edi
	jmp .1

.end:
	pop eax
	pop edi
	pop esi
	ret	

Code32Len	equ	$-SEG_CODE32

[SECTION .s16]
[BITS 16]
;跳回实模式
SEG_CODE16:
	mov ax,SelectorNormal		;恢复数据段描述符高速缓存器
	mov ds,ax
	mov es,ax
	mov gs,ax
	mov ss,ax

	mov eax,cr0
	and al,011111110b
	mov cr0,eax

;GO_BACK_TO_REAL:
;	jmp 0:REAL_ENTRY

REAL_ENTRY:
;	mov ax,cs			;恢复段寄存器
;	mov ds,ax
;	mov es,ax
;	mov gs,ax
;	mov ss,ax

	in al,92h			;打开A20地址线
	and al,011111101b
	out 92h,al

	sti				;开中断
	mov ax,0x4c00
	int 21h


[SECTION .localtask]			;局部任务代码段
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
	jz .2				;跳回16位代码段返回实模式
	mov [gs:edi],ax
	add edi,2
	jmp .1
.2:
	jmp SelectorCode16:0

LocalTaskLen	equ	$ - LOCAL_TASK