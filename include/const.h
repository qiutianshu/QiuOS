#ifndef _QIUOS_CONST_H_
#define _QIUOS_CONST_H_

#define PUBLIC 
#define PRIVATE static
#define EXTERN	extern 


//8259端口
#define INT_M_CTL 			0x20
#define	INT_M_CTLMASK		0x21
#define INT_S_CTL			0xa0
#define	INT_S_CTLMASK		0xa1

#define IRQ0				0x20
#define	IRQ8				0x28

//中断向量
#define	INT_VECTOR_DIVIDE			0x0
#define	INT_VECTOR_DEBUG			0x1
#define	INT_VECTOR_NMI				0x2
#define	INT_VECTOR_BREAKPOINT		0x3
#define	INT_VECTOR_OVERFLOW			0x4
#define	INT_VECTOR_BOUNDS			0x5
#define	INT_VECTOR_INVAL_OP			0x6
#define	INT_VECTOR_COPROC_NOT		0x7
#define	INT_VECTOR_DOUBLE_FAULT		0x8
#define	INT_VECTOR_COPROC_SEG		0x9
#define	INT_VECTOR_INVAL_TSS		0xA
#define	INT_VECTOR_SEG_NOT			0xB
#define	INT_VECTOR_STACK_FAULT		0xC
#define	INT_VECTOR_PROTECTION		0xD
#define	INT_VECTOR_PAGE_FAULT		0xE
#define	INT_VECTOR_COPROC_ERR		0x10


#endif