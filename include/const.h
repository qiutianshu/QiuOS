#ifndef _QIUOS_CONST_H_
#define _QIUOS_CONST_H_

#define PUBLIC 
#define PRIVATE static
#define EXTERN	extern 

#define printf  print

/*主设备号*/
#define NO_DEV			-1
#define DEV_CHAR_TTY	0		//tty设备
#define DEV_HD			2		//硬盘

void assertion_failure(char* exp,char* file,char* base_file,int line);
#define assert(exp)		if(exp);\
else assertion_failure(#exp, __FILE__, __BASE_FILE__, __LINE__)
		

#define min(a,b)		(a < b) ? a : b

#define MAG_CH_PANIC	0x02
#define MAG_CH_ASSERT	0x03

/*进程的状态*/
#define RUNNING			0			//正在运行或者就绪
#define SENDING 		0x2 		//消息未发送出去正在发送
#define RECEIVING		0x4 		//未接收到消息正在接收
#define ANY				(NR_TASKS + NR_PROCS + 10)
#define NO_TASK			(NR_TASKS + NR_PROCS + 20)
#define INTERRUPT		-10

#define SEND 			1
#define RECEIVE			2
#define BOTH			3	


enum msgtype{						//消息类型
	HARD_INT	=	1,
	GET_TICKS,
	DEV_OPEN,
	DEV_CLOSE,
	DEV_READ,
	DEV_WRITE,
	DEV_IOCTL,
	FILE_OPEN,
	FILE_CLOSE,
	FILE_WRITE,
	FILE_READ, 
	FILE_DELETE,
	SYSCALL_RET,
	SUSPEND_PROC,
	RESUME_PROC
};

#define DIOCTL_GET_GEO	1

/*VGA显示设置*/
#define CRT_CTRL_REG	0x3d4		//地址寄存器
#define CRT_DATA_REG	0x3d5		//数据寄存器
#define START_ADDR_H	0xc 		//
#define START_ADDR_L	0xd 
#define CURSOR_H 		0xe 
#define CURSOR_L		0xf 

/*控制台数量*/
#define NR_CONSOLES		3
#define CONSOLE_BASE	0xb8000
#define CONSOLE_SIZE	0x8000 		//显存总大小
#define CONSOLE_COLOR	0xd 		//控制台默认显示颜色
#define COLOR_RED		0xc

/*
	8253端口说明
	0x40    Count0   输出到IRQ0，每隔一定时间产生时钟中断
	0x41	Count1	 通常设为18，每隔15us刷新RAM
	0x42	Count2	 连接喇叭
	0x43 	Mode Control Register

	模式控制寄存器：
	计数器选择位  bit7、bit6 
	读写锁位		bit5、bit4
	计数器模式位	bit3、bit2、bit1
	0=计数器使用二进制位，1=使用BCD码

*/
 

//8253设置相关
#define TIMER0				0x40 
#define TIMER_MODE 			0x43
#define RATE_GENERATOR		0x34 			//00 11 010 0

#define TIMER_FREQ			1193182L		//时钟震荡频率
#define HZ					100				//时钟中断频率 intval = TIMMER_FREQ / HZ


//8259A 设置相关
#define INT_M_CTL 			0x20
#define	INT_M_CTLMASK		0x21
#define INT_S_CTL			0xa0
#define	INT_S_CTLMASK		0xa1
#define EOI                 0x20  

#define IRQ0				0x20
#define	IRQ8				0x28

#define NR_IRQ				0x10			//中断数量
#define NR_SYS_CALL			0x10			//系统调用数量


//中断号
#define CLOCK_IRQ			0
#define KEYBOARD_IRQ		1 				//键盘
#define EN_SLAVER			2 				//从片使能
#define HD_IRQ				14				//硬盘 

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
#define INT_VECTOR_SYS_CALL			0x90


//进程相关常量
#define NR_TASKS					4		//任务数量
#define NR_PROCS					3		//用户进程数

#define UNUSED						-10
#define TASK_TTY					0		//TTY任务
#define TASK_SYS					1		//系统进程号
#define TASK_HD						2		//硬盘驱动进程号
#define TASK_FS						3		//文件系统

#define STACK_SIZE_TESTA            0x8000
#define STACK_SIZE_TESTB            0x8000
#define STACK_SIZE_TESTC            0x8000
#define STACK_SIZE_TTY				0x8000
#define STACK_SIZE_SYS_TASK			0x8000
#define STACK_SIZE_HD				0x8000
#define STACK_SIZE_FS				0x8000
#define STACK_SIZE_TOTAL			(STACK_SIZE_FS+ STACK_SIZE_HD+ STACK_SIZE_TESTA+ STACK_SIZE_TESTB+ STACK_SIZE_TESTC+ STACK_SIZE_TTY+ STACK_SIZE_SYS_TASK)

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