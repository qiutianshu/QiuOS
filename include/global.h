

PUBLIC int disp_pos;

PUBLIC	u8	gdt_ptr[6];
PUBLIC	u8  idt_ptr[6];

PUBLIC Descriptor	gdt[GDT_SIZE]; 
PUBLIC Gate			idt[IDT_SIZE];	
