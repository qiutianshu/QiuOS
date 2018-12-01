
%include "sconst.inc"

SELECTOR_KERNEL_CS		equ			0x18
SELECTOR_TSS			equ			0x20

extern cstart						;导入函数
extern exception_handler
extern spurious_irq
extern kernel_main
extern disp_str
extern delay
extern clock_handler


extern gdt_ptr						;导入全局变量
extern idt_ptr
extern disp_pos
extern tss
extern p_proc_ready
extern k_reenter
extern irq_table
extern sys_call_table

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
global sys_call

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
;主8259A中断处理
;---------------------------------------------------------------------------
%macro hwint_master		1
	call save					;当前EOI未发送，不可能有时钟中断
	in al,INT_M_CTLMASK
	or al,(1 << %1)				;屏蔽当前中断，不发生同类型中断
	out INT_M_CTLMASK,al
	mov al,EOI					;发送EOI
	out INT_M_CTL,al
	sti 				
	push %1		
	call [irq_table + 4 * %1]	;中断处理
	add esp,4 
	cli 
	in al,INT_M_CTLMASK
	and al,0xff-(1 << %1)		;恢复当前中断
	out INT_M_CTLMASK,al
	ret 
%endmacro
;---------------------------------------------------------------------------
;从8259A中断处理
;---------------------------------------------------------------------------
%macro hwint_slaver		1
	call save					
	in al,INT_S_CTLMASK
	or al,(1 << %1 - 8)				
	out INT_S_CTLMASK,al
	mov al,EOI					
	out INT_M_CTL,al
	nop 				
	nop  						;一定的时间延迟
	out INT_S_CTL,al 			;主从都要置EOI
	sti 				
	push %1		
	call [irq_table + 4 * %1]	
	add esp,4 
	cli 
	in al,INT_S_CTLMASK			;恢复中断
	and al,0xff-(1 << %1 - 8)		
	out INT_S_CTLMASK,al
	ret 										;0030507
%endmacro

hwint00:
	hwint_master	0
	
hwint01:
	hwint_master	1

hwint02:
	hwint_master	2

hwint03:
	hwint_master	3

hwint04:
	hwint_master	4

hwint05:
	hwint_master	5

hwint06:
	hwint_master	6

hwint07:
	hwint_master	7

hwint08:
	hwint_slaver	8

hwint09:
	hwint_slaver	9

hwint10:
	hwint_slaver	10

hwint11:
	hwint_slaver	11

hwint12:
	hwint_slaver	12

hwint13:
	hwint_slaver	13

hwint14:							;硬盘中断
	hwint_slaver	14

hwint15:
	hwint_slaver	15

exception:
	call exception_handler
	add esp,8						;栈顶指向eip
	hlt

restart:
	mov esp,[p_proc_ready]
	lldt [esp+P_LDT_SEL]
	lea eax,[esp+P_STACKTOP]
	mov dword [tss+TSS_S_SP0],eax

restart_reenter:
	dec dword [k_reenter]
	pop	gs
	pop	fs
	pop	es
	pop	ds
	popad

	add	esp, 4
	iretd

save:
	pushad
	push ds
	push es
	push fs
	push gs
	mov esi,edx 							;暂存edx
	mov dx,ss 
	mov ds,dx
	mov es,dx
	mov edx,esi								;恢复edx

	mov esi,esp
	inc dword [k_reenter]
	cmp dword [k_reenter],0
	jne .reenter 
	mov esp,StackTop 						;进入内核栈
	push restart
	jmp [esi + RETADR - P_STACKBASE]		;返回到call的下一条指令
.reenter:
	push restart_reenter
	jmp [esi + RETADR - P_STACKBASE]

sys_call:
	call save
	push esi
	push dword [p_proc_ready]
	push edx
	push ecx 
	push ebx
	sti
	call [sys_call_table + eax * 4]
	add esp,16
	pop esi
	mov [esi + EAXREG - P_STACKBASE],eax 	;压入返回值
	cli
	ret
