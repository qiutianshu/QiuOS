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
	u32 pid;
	char p_name[32];
	int nr_tty;
}PROCESS;

typedef struct s_task{
	task_f	init_eip;
	int 	stack_size;
	char	name[32];
}TASK;

#define LDT_SIZE	128