#include "type.h"
#include "const.h"
#include "proto.h"
#include "protect.h"
#include "proc.h"
#include "global.h"



PUBLIC int kernel_main(){
	disp_str("------------------kernel main begins---------------------\n");
	TASK*		p_task 			= task_table;
	PROCESS* 	p_proc 			= proc_table;
	char*		p_task_stack	= task_stack + STACK_SIZE_TOTAL;
	u16			selector_ldt 	= SELECTOR_LDT_FIRST;
	int i;

	for(i=0; i<NR_TASKS; i++){
		strcpy(p_proc->p_name,p_task->name);		//进程名
		p_proc->pid = i;							//进程号
		p_proc->ldt_sel = SELECTOR_LDT_FIRST;

		memcpy(&p_proc->ldts[0],&gdt[SELECTOR_KERNEL_CS >> 3],sizeof(Descriptor));		//复制内核全局描述符
		p_proc->ldts[0].attr1 = DA_C | PRIVILEGE_TASK << 5;								//修改特权级为1
		memcpy(&p_proc->ldts[1],&gdt[SELECTOR_KERNEL_DS >> 3],sizeof(Descriptor));
		p_proc->ldts[1].attr1 = DA_DAW | PRIVILEGE_TASK << 5;

		p_proc->regs.cs = (0 & 0xfffc & 0xfffb) | SA_L | SA_RPL1;
		p_proc->regs.ds = (8 & 0xfffc & 0xfffb) | SA_L | SA_RPL1;
		p_proc->regs.es = (8 & 0xfffc & 0xfffb) | SA_L | SA_RPL1;
		p_proc->regs.fs = (8 & 0xfffc & 0xfffb) | SA_L | SA_RPL1;
		p_proc->regs.ss = (8 & 0xfffc & 0xfffb) | SA_L | SA_RPL1;
		p_proc->regs.gs = (SELECTOR_KERNEL_GS & 0xfffc) | SA_RPL1;
		p_proc->regs.eip = (u32)p_task->init_eip;
		p_proc->regs.esp = (u32)p_task_stack;
		p_proc->regs.eflags = 0x1202;					//设置IF位打开中断

		p_task_stack -= p_task->stack_size;
		p_proc++;
		p_task++;
		selector_ldt +=1 << 3;
	}
	init_clock();									//初始化时钟
	init_keyboard();								//初始化键盘

	proc_table[0].ticks = proc_table[0].priority = 15;
	proc_table[1].ticks = proc_table[1].priority = 3;
	proc_table[2].ticks = proc_table[2].priority = 3;
	proc_table[3].ticks = proc_table[3].priority = 3;

	

	ticks = 0;

//	delay(200);
//	disp_pos = 0;									//延时2秒清屏
//	for(i=0; i<80*25; i++)
//		disp_str(" ");
//	disp_pos = 0;

	k_reenter = 0;
	
	p_proc_ready = proc_table;
	restart();
	while(1){}
}


void TestA(){
	while(1){
	//	disp_color_str("A",0xc);
	//	disp_str(" ");
		milli_delay(10);					//20 ticks，打印下一个A之前发生20次时钟中断
	}
}

void TestB(){
	while(1){
	//	disp_color_str("B",0xb);
	//	disp_str(" ");
		milli_delay(10);
	}
}

void TestC(){
	while(1){
	//	disp_color_str("C",0xd);
	//	disp_str(" ");
		milli_delay(10);
	}
}

