#include "const.h"
#include "type.h"
#include "protect.h"

PUBLIC	void memcpy(void *Dest,void *Src,int size);

PUBLIC	disp_str(char *str);

PUBLIC	Descriptor	gdt[GDT_SIZE];

PUBLIC	u8	gdt_ptr[6];


PUBLIC void cstart(){
	disp_str("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
	disp_str("--------------cstart--------------\n");

	memcpy(&gdt,(void*)(*((u32*)(&gdt_ptr[2]))),*((u16*)(&gdt_ptr[0]))+1);

	*((u16*)(&gdt_ptr[0])) = GDT_SIZE * sizeof(Descriptor) - 1;
	*((u32*)(&gdt_ptr[2])) = (u32)&gdt;
}