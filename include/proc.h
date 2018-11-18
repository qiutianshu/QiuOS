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
	u32 pid;
	char p_name[16];
}PROCESS;

#define LDT_SIZE	128