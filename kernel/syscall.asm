%include "sconst.inc"

global get_ticks  				;系统调用用户接口

bits 32
[section .text]
get_ticks:
	mov eax,_NR_get_tricks
	int INT_VECTOR_SYS_CALL
	ret