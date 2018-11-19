#ifndef _QIUOS_CONST_H_
#define _QIUOS_CONST_H_

#define PUBLIC 
#define PRIVATE static
#define EXTERN	extern 


//8259A 设置相关
#define INT_M_CTL 			0x20
#define	INT_M_CTLMASK		0x21
#define INT_S_CTL			0xa0
#define	INT_S_CTLMASK		0xa1
#define EOI                 0x20    

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

//进程相关常量
#define NR_TASKS					1		//进程数量
#define STACK_SIZE_TOTAL			0x8000

//描述符相关常量
#define DA_32	0x4000				//32位段
#define DA_DPL0	0x0 				//DPL = 0
#define DA_DPL1	0x1 				//DPL = 1
#define DA_DPL2	0x2 				//DPL = 2
#define DA_DPL3	0x3 				//DPL = 3
//存储段
#define DA_DR 	0x90				//存在的只读数据段类型值
#define DA_DAW 	0x92				//存在的可读写数据段属性值
#define DA_DRWA	0x93				//存在的已访问可读写数据段类型值
#define DA_C    0x98				//存在的只执行代码段属性值
#define DA_CR   0x9a				//存在的可执行可读代码段属性值
#define DA_CCO  0x9c				//存在的只执行一致代码段属性值
#define DA_CCOR 0x9e				//存在的可执行可读一致代码段属性值
//系统段
#define DA_LDT			0x82 		//局部描述符表段类型值
#define DA_TaskGate		0x85		//任务门类型值
#define DA_386TSS		0x89 		//可用 386 任务状态段类型值
#define DA_386CGate		0x8c		//386 调用门类型值
#define DA_386IGate		0x8e		//386 中断门类型值
#define DA_386TGate		0x8f		//386 陷阱门类型值


/* 权限 */
#define	PRIVILEGE_KRNL	0
#define	PRIVILEGE_TASK	1
#define	PRIVILEGE_USER	3

//段选择子类型说明
#define SA_RPL0			0
#define SA_RPL1			1
#define SA_RPL2			2
#define SA_RPL3			3

#define SA_G			0			//GDT
#define SA_L 			4			//LDT 

//描述符索引
#define INDEX_DUMMY		0
#define INDEX_VIDEO		1
#define INDEX_FALT_RW	2
#define INDEX_FLAT_C	3
#define INDEX_TSS		4
#define INDEX_LDT_FIRST	5



#endif