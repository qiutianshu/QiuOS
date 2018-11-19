
%include "sconst.inc"

SELECTOR_KERNEL_CS		equ			0x18
SELECTOR_TSS			equ			0x20

extern cstart						;导入函数
extern exception_handler
extern spurious_irq
extern kernel_main
extern disp_str
extern delay

extern gdt_ptr						;导入全局变量
extern idt_ptr
extern disp_pos
extern tss
extern p_proc_ready
extern k_reenter

[SECTION .bss]
StackSpace			resb	2*1024	;内核栈(基地址0x8000)，加载时分配空间
StackTop:

[SECTION .data]
clock_int_msg			db 			"^"

[SECTION .text]
;---------------------------------------------------------------------
;异常处理
;---------------------------------------------------------------------
global _start
global divide_error
global single_step_exception
global nmi
global breakpoint_exception
global overflow
global bounds_check
global inval_opcode
global copr_not_available
global double_fault
global copr_seg_overrun
global inval_tss
global segment_not_present
global stack_exception
global general_protection
global page_fault
global copr_error

;---------------------------------------------------------------------
;硬件中断
;---------------------------------------------------------------------
global hwint00						;0号中断
global hwint01
global hwint02
global hwint03
global hwint04
global hwint05
global hwint06
global hwint07
global hwint08
global hwint09
global hwint10
global hwint11
global hwint12
global hwint13
global hwint14
global hwint15

global restart

_start:
	mov esp,StackTop
	mov dword [disp_pos],0
	sgdt [gdt_ptr]
	call cstart
	lgdt [gdt_ptr]
	lidt [idt_ptr]

	jmp SELECTOR_KERNEL_CS:csinit

csinit:
	
	xor	eax, eax
	mov	ax, SELECTOR_TSS
	ltr	ax
	sti 
	jmp kernel_main



divide_error:
	push 0xffffffff					;没有错误码
	push 0							;中断向量号
	jmp exception

single_step_exception:
	push 0xffffffff
	push 1
	jmp exception

nmi:
	push 0xffffffff
	push 2
	jmp exception

breakpoint_exception:
	push 0xffffffff
	push 3
	jmp exception

overflow:
	push 0xffffffff
	push 4
	jmp exception

bounds_check:
	push 0xffffffff
	push 5
	jmp exception

inval_opcode:
	push 0xffffffff
	push 6
	jmp exception

copr_not_available:
	push 0xffffffff
	push 7
	jmp exception

double_fault:
	push 8
	jmp exception

copr_seg_overrun:
	push 0xffffffff
	push 9
	jmp exception

inval_tss:
	push 10
	jmp exception

segment_not_present:
	push 11
	jmp exception

stack_exception:
	push 12
	jmp exception

general_protection:
	push 13
	jmp exception

page_fault:
	push 14
	jmp exception

copr_error:
	push 0xffffffff
	push 16
	jmp exception


;---------------------------------------------------------------------------
;中断处理
;---------------------------------------------------------------------------
hwint00:
	sub esp,4
	pushad
	push ds
	push es
	push fs
	push gs
	mov dx,ss 
	mov ds,dx
	mov es,dx

	inc byte [gs:0]

	mov al,EOI					;发送EOI
	out INT_M_CTL,al

	inc dword [k_reenter]
	cmp dword [k_reenter],0
	jne .exit 

	mov esp,StackTop			;切换到内核栈
	sti 						;此时的中断已经转移到内核栈中

	push clock_int_msg
	call disp_str
	add esp,4

	push 1
	call delay
	add esp,4

	cli 

	mov esp,[p_proc_ready]		;离开内核栈

	lea eax,[esp+P_STACKTOP]
	mov dword [tss+TSS_S_SP0],eax

.exit:
	dec dword [k_reenter]
	pop gs
	pop fs
	pop es
	pop ds
	popad
	add esp,4

	iretd
			

hwint01:
	push 1		
	jmp hwint

hwint02:
	push 2		
	jmp hwint

hwint03:
	push 3		
	jmp hwint

hwint04:
	push 4		
	jmp hwint

hwint05:
	push 5		
	jmp hwint

hwint06:
	push 6		
	jmp hwint

hwint07:
	push 7		
	jmp hwint

hwint08:
	push 8		
	jmp hwint

hwint09:
	push 9		
	jmp hwint

hwint10:
	push 10		
	jmp hwint

hwint11:
	push 11		
	jmp hwint

hwint12:
	push 12		
	jmp hwint

hwint13:
	push 13		
	jmp hwint

hwint14:
	push 14		
	jmp hwint

hwint15:
	push 15		
	jmp hwint

exception:
	call exception_handler
	add esp,8						;栈顶指向eip
	hlt

hwint:
	call spurious_irq
	add esp,4
	hlt


restart:
	mov esp,[p_proc_ready]
	lldt [esp+P_LDT_SEL]
	lea eax,[esp+P_STACKTOP]
	mov dword [tss+TSS_S_SP0],eax
	pop	gs
	pop	fs
	pop	es
	pop	ds
	popad

	add	esp, 4
	iretd