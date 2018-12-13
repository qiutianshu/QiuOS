#include "type.h"
#include "const.h"
#include "elf.h"
#include "fs.h"
#include "protect.h"
#include "tty.h"
#include "console.h"
#include "proc.h"
#include "proto.h"
#include "global.h"


PUBLIC void disp_int(int input){
	char output[16];
	atoi(output,input);
	disp_str(output);
}

PUBLIC void delay(int time){
	int i,j,k;
	for(k = 0; k < time; k++){
		for(i = 0; i <10; i++){
			for(j = 0; j < 10000; j++){}
		}
	}
}

PUBLIC void disable_irq(int irq){
	if(irq<8)
		out_byte(INT_M_CTLMASK, in_byte(INT_M_CTLMASK)|(1<<irq));
	else
		out_byte(INT_S_CTLMASK, in_byte(INT_S_CTLMASK)|(1<<(irq % 8)));
}

PUBLIC void enable_irq(int irq){
	if(irq<8)
		out_byte(INT_M_CTLMASK,in_byte(INT_M_CTLMASK)&(0xff-(1<<irq)));
	else
		out_byte(INT_S_CTLMASK,in_byte(INT_S_CTLMASK)&(0xff-(1<<(irq%8))));
}

/*ring 0 状态运行，读取loader设置的启动参数
*	kernel.bin物理地址 0x80000
*
*参数：	memory base of kernel
*		memory size of kernel
*返回值：成功返回0，失败返回-1
*/
PUBLIC int get_kernel_map(u32* base, u32* size){
	void* kernel_file = (void*)0x80000;
	Elf32_Ehdr* elf_header	= (Elf32_Ehdr*)kernel_file;			//kernel.bin 物理地址
	Elf32_Shdr* section_header;
	if(elf_header->e_ident[1] != 'E' || elf_header->e_ident[2] != 'L' || elf_header->e_ident[3] != 'F')
		return -1;
	*base = ~0;													//0xffffffff 地址空间最大处
	u32 t = 0;
	int i;
	int bottom,top;
	for(i = 0; i < elf_header->e_shnum; i++){ 
		section_header = (kernel_file + elf_header->e_shoff + i * elf_header->e_shentsize);
		if(section_header->sh_flags & SHF_ALLOC){
			bottom = section_header->sh_addr;					//段在进程空间的虚拟地址
			top = section_header->sh_addr + section_header->sh_size;
			if(*base > bottom)
				*base = bottom;
			if(t < top)
				t = top;	
		}
	}
	assert(*base < t);
	*size = t - *base - 1;

	return 0;
}