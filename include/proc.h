//进程相关常量
#define NR_TASKS					5		//任务数量   
#define NR_PROCS					16		//用户进程数
#define NR_NATIVE_PROCS				4		//初始进程数   A B C Init

#define UNUSED						-10
#define TASK_TTY					0		//TTY任务
#define TASK_SYS					1		//系统进程号
#define TASK_HD						2		//硬盘驱动进程号
#define TASK_FS						3		//文件系统
#define TASK_MM 					4		//内存管理

#define STACK_SIZE_DEFAULT			0x4000					//进程默认栈１６ｋｂ
#define STACK_SIZE_INIT             STACK_SIZE_DEFAULT		//init进程
#define STACK_SIZE_TESTA            STACK_SIZE_DEFAULT
#define STACK_SIZE_TESTB            STACK_SIZE_DEFAULT
#define STACK_SIZE_TESTC            STACK_SIZE_DEFAULT
#define STACK_SIZE_TTY				STACK_SIZE_DEFAULT		//tty任务
#define STACK_SIZE_SYS_TASK			STACK_SIZE_DEFAULT		//提供系统节拍
#define STACK_SIZE_HD				STACK_SIZE_DEFAULT		//硬盘驱动
#define STACK_SIZE_FS				STACK_SIZE_DEFAULT		//文件系统
#define STACK_SIZE_MM				STACK_SIZE_DEFAULT		//内存管理
#define STACK_SIZE_TOTAL			(STACK_SIZE_FS + STACK_SIZE_HD + STACK_SIZE_TESTA + STACK_SIZE_TESTB + \
									 STACK_SIZE_TESTC + STACK_SIZE_TTY + STACK_SIZE_SYS_TASK + STACK_SIZE_MM +\
									 STACK_SIZE_INIT)

#define INDEX_LDT_C 				0
#define INDEX_LDT_D 				1

#define PROC_IMG_DEFAULT_SIZE 		0x100000				//用户进程空间默认大小(1M粒度)
#define PROC_BASE					0xa00000				//10M以上分配给用户进程

#define MEMSIZE						0x900					//存放可用内存大小的地方


typedef struct s_stackframe{
	u32	gs;
	u32 fs;
	u32 es;
	u32 ds;
	u32 edi;
	u32 esi;
	u32 ebp;
	u32 kernel_esp;
	u32 ebx;
	u32 edx;
	u32 ecx;
	u32 eax;
	u32 retaddr;
	u32 eip;
	u32 cs;
	u32 eflags;
	u32 esp;
	u32 ss;
}STACK_FRAME;


typedef struct s_proc{
	STACK_FRAME regs;
	u16 ldt_sel;
	Descriptor ldts[LDT_SIZE];
	int ticks;
	int priority;
//	u32 pid;
	u32 p_parent;
	char p_name[32];
	int p_flags;
	MESSAGE* p_msg;
	int p_recvfrom;
	int p_sendto;
	int has_int_msg;
	struct s_proc* q_sending;					//指向自身消息队列头
	struct s_proc* next_sending;				
	struct file_desc* filp[NR_FILES];			//文件打开表
	u32 exit_status;
}PROCESS;

typedef struct s_task{
	task_f	init_eip;
	int 	stack_size;
	char	name[32];
}TASK;

#define LDT_SIZE	128
#define proc2pid(x)	(x - proc_table)

#define phy_cpy		memcpy