/*
protect.h文件设置了保护模式下的参数
*/



#ifndef _QIUOS_PROTECT_H_
#define _QIUOS_PROTECT_H_

//段描述符
typedef struct descriptor{
	u16	limit_low;		/* Limit */
	u16	base_low;		/* Base */
	u8	base_mid;		/* Base */
	u8	attr1;			/* P(1) DPL(2) DT(1) TYPE(4) */
	u8	limit_high_attr2;	/* G(1) D(1) 0(1) AVL(1) LimitHigh(4) */
	u8	base_high;		/* Base */
}Descriptor;

//门描述符

/*说明:

 (1) P:    存在(Present)位。
		P=1 表示描述符对地址转换是有效的，或者说该描述符所描述的段存在，即在内存中；
		P=0 表示描述符对地址转换无效，即该段不存在。使用该描述符进行内存访问时会引起异常。

 (2) DPL:  表示描述符特权级(Descriptor Privilege level)，共2位。它规定了所描述段的特权级，用于特权检查，以决定对该段能否访问。 

 (3) S:   说明描述符的类型。
		对于存储段描述符而言，S=1，以区别与系统段描述符和门描述符(S=0)。 

 (4) TYPE: 说明存储段描述符所描述的存储段的具体属性。

系统段类型	类型编码	说明
;			----------------------------------
;			0		<未定义>
;			1		可用286TSS
;			2		LDT
;			3		忙的286TSS
;			4		286调用门
;			5		任务门
;			6		286中断门
;			7		286陷阱门
;			8		未定义
;			9		可用386TSS
;			A		<未定义>
;			B		忙的386TSS
;			C		386调用门
;			D		<未定义>
;			E		386中断门
;			F		386陷阱门

 */
typedef struct s_gate
{
	u16	offset_low;	/* Offset Low */
	u16	selector;	/* Selector */
	u8	dcount;		/* 该字段只在调用门描述符中有效。如果在利用
				   调用门调用子程序时引起特权级的转换和堆栈
				   的改变，需要将外层堆栈中的参数复制到内层
				   堆栈。该双字计数字段就是用于说明这种情况
				   发生时，要复制的双字参数的数量。*/
	u8	attr;		/* P(1) DPL(2) DT(1) TYPE(4) */
	u16	offset_high;	/* Offset High */
}Gate;


//权限
#define PRIVILEGE_KERNEL	0				//ring0级
#define	PRIVILEGE_USER		3				//ring3级

//描述符类型
#define DA_386IGATE			0x8e			//i386中断门描述符

#define GDT_SIZE 128						//全局描述符数量
#define IDT_SIZE 256						//中断门数量

#endif