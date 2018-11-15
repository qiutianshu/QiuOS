#include "const.h"
#include "type.h"
#include "protect.h"
#include "proto.h"
#include "global.h"
	

PUBLIC void cstart(){
	disp_str("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
	disp_str("--------------Setup interrupt and exception--------------\n");

	memcpy(&gdt,(void*)(*((u32*)(&gdt_ptr[2]))),*((u16*)(&gdt_ptr[0]))+1);				//复制GDT

	*((u16*)(&gdt_ptr[0])) = GDT_SIZE * sizeof(Descriptor) - 1;
	*((u32*)(&gdt_ptr[2])) = (u32)&gdt;

	*((u16*)(&idt_ptr[0])) = IDT_SIZE * sizeof(Gate) - 1;								//设置idtr
	*((u32*)(&idt_ptr[2])) = (u32)&idt;

	init_prot();
	disp_str("--------------       Setup done!    --------------\n");
}