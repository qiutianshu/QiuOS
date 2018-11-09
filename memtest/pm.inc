;The struct of Descriptor
;	dd	base
;	dd	limit(低20位有效)
;	dw	attribute（低12位有效）

%macro Descriptor 3
	dw	%2 & 0xffff																			;段界限1
	dw	%1 & 0xffff																			;段基址1
	db	(%1>>16) & 0xff																		;段基址2
	dw	((%2 >> 8) & 0F00h) | (%3 & 0F0FFh)													;属性8-11位无效
	db	(%1>>24) &0xff																		;段基址3
%endmacro

;The struct of Gate
;	dw  selector
;	dd  offset
;	db  count
;	db	attribute
%macro Gate 4
	dw  (%2 & 0xffff)
	dw  %1
	dw	(%3 & 1Fh) | ((%4 << 8) & 0FF00h)
	dw	((%2 >> 16) & 0FFFFh)
%endmacro


