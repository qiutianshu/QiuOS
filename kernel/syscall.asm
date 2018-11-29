%include "sconst.inc"

global get_ticks  				;系统调用用户接口
global write
global sendrec
global printx

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

sendrec:
	mov eax,_NR_sendrec
	mov ebx,[esp+4]			;function
	mov ecx,[esp+8]			;src_dest
	mov edx,[esp+12]		;p_msg
	int INT_VECTOR_SYS_CALL
	ret

printx:
	mov eax,_NR_printx
	mov edx,[esp+4]			;buf
	int INT_VECTOR_SYS_CALL
	ret						