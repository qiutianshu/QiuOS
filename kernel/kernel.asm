SELECTOR_KERNEL_CS		equ			0x18

extern cstart

extern gdt_ptr

[SECTION .bss]
StackSpace			resb	2*1024
StackTop:

[SECTION .text]

global _start

_start:
	mov esp,StackTop

	sgdt [gdt_ptr]
	call cstart
	lgdt [gdt_ptr]
	jmp $

	jmp SELECTOR_KERNEL_CS:csinit

csinit:
	push 0
	popfd

	hlt

