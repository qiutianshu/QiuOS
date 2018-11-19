/*
    全局变量被定义在kernel.elf文件的.data段
*/
#ifdef GLOBAL_VARIABLES_HERE
#undef	EXTERN
#define EXTERN
#endif

EXTERN int disp_pos;

EXTERN	u8	gdt_ptr[6];
EXTERN	u8  idt_ptr[6];

EXTERN Descriptor	gdt[GDT_SIZE]; 
EXTERN Gate			idt[IDT_SIZE];	

EXTERN PROCESS proc_table[NR_TASKS];

EXTERN PROCESS* p_proc_ready;
EXTERN TSS tss;
EXTERN char task_stack[STACK_SIZE_TOTAL];
EXTERN int k_reenter;