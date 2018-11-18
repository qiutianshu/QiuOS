

PUBLIC int disp_pos;

PUBLIC	u8	gdt_ptr[6];
PUBLIC	u8  idt_ptr[6];

PUBLIC Descriptor	gdt[GDT_SIZE]; 
PUBLIC Gate			idt[IDT_SIZE];	

PUBLIC PROCESS proc_table[NR_TASKS];

PUBLIC PROCESS* p_proc_ready;
PUBLIC TSS tss;
PUBLIC char task_stack[STACK_SIZE_TOTAL];