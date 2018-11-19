#include "type.h"
#include "const.h"
#include "proto.h"
#include "protect.h"
#include "proc.h"
#include "global.h"



PUBLIC int kernel_main(){
	disp_str("------------------kernel main begins---------------------\n");
	k_reenter = -1;
	PROCESS* p_proc = proc_table;
	p_proc->ldt_sel = SELECTOR_LDT_FIRST;
	memcpy(&p_proc->ldts[0],&gdt[SELECTOR_KERNEL_CS >> 3],sizeof(Descriptor));
	p_proc->ldts[0].attr1 = DA_C | PRIVILEGE_TASK << 5;
	memcpy(&p_proc->ldts[1],&gdt[SELECTOR_KERNEL_DS >> 3],sizeof(Descriptor));
	p_proc->ldts[1].attr1 = DA_DAW | PRIVILEGE_TASK << 5;

	p_proc->regs.cs = (0 & 0xfffc & 0xfffb) | SA_L | SA_RPL1;
	p_proc->regs.ds = (8 & 0xfffc & 0xfffb) | SA_L | SA_RPL1;
	p_proc->regs.es = (8 & 0xfffc & 0xfffb) | SA_L | SA_RPL1;
	p_proc->regs.fs = (8 & 0xfffc & 0xfffb) | SA_L | SA_RPL1;
	p_proc->regs.ss = (8 & 0xfffc & 0xfffb) | SA_L | SA_RPL1;
	p_proc->regs.gs = (SELECTOR_KERNEL_GS & 0xfffc) | 1;
	p_proc->regs.eip = (u32)TestA;
	p_proc->regs.esp = (u32)task_stack + STACK_SIZE_TOTAL;
	p_proc->regs.eflags = 0x1202;
	p_proc_ready = proc_table;
	restart();
	while(1){}
}


void TestA(){
	int i = 0;
	while(1){
		disp_str("A");
	//	disp_int(i++);
		disp_str(" ");
		delay(1);
	}
}