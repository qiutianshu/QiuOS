%include "sconst.inc"

global get_ticks  				;系统调用用户接口
global write

bits 32
[section .text]
get_ticks:
	mov eax,_NR_get_tricks
	int INT_VECTOR_SYS_CALL
	ret

write:
	mov eax,_NR_write
	mov ebx,[esp+4]			;buf
	mov ecx,[esp+8]			;len
	int INT_VECTOR_SYS_CALL
	ret