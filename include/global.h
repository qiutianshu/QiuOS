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

EXTERN PROCESS proc_table[NR_TASKS + NR_PROCS];	//进程表
EXTERN int total_memory_size;						//可用内存大小

EXTERN PROCESS* p_proc_ready;
EXTERN TSS tss;
EXTERN char task_stack[STACK_SIZE_TOTAL];
EXTERN int k_reenter;
EXTERN int ticks;

EXTERN int current_console;				//当前控制台

EXTERN MESSAGE fs_msg;
EXTERN MESSAGE mm_msg;

EXTERN struct file_desc file_desc_table[NR_FILE_DESC];		//文件描述符
EXTERN struct inode 	inode_table[NR_INODES]; 			
EXTERN	struct inode *		root_inode;
EXTERN struct super_block super_block[NR_SUPER_BLOCK];
EXTERN PROCESS* caller; 
EXTERN keyboard_pressed;

extern TASK task_table[];
extern TASK user_proc_table[];
extern irq_handler irq_table[];
extern system_call sys_call_table[];
extern TTY tty_table[];
extern CONSOLE console_table[];
extern struct dev_drv_map dd[];
extern u8* fsbuf;
extern const int FSBUF_SIZE;
