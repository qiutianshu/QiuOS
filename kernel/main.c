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
		p_proc->regs.eflags = 0x1202;

		p_task_stack -= p_task->stack_size;
		p_proc++;
		p_task++;
		selector_ldt +=1 << 3;
	}
	put_irq_handler(CLOCK_IRQ,clock_handler);
	enable_irq(CLOCK_IRQ);

	/*初始化8253 PIT*/
	out_byte(TIMER_MODE,RATE_GENERATOR);		//设置模式控制寄存器为时钟中断模式
	out_byte(TIMER0,(u8)(TIMER_FREQ/HZ));		//写入低8位
	out_byte(TIMER0,(u8)((TIMER_FREQ/HZ) >> 8));//写入高8位

	k_reenter = 0;
	ticks = 0;
	p_proc_ready = proc_table;
	restart();
	while(1){}
}


void TestA(){
	while(1){
		disp_str("A");
		disp_str(" ");
		milli_delay(1000);
	}
}

void TestB(){
	while(1){
		disp_str("B");
		disp_str(" ");
		milli_delay(1000);
	}
}

void TestC(){
	while(1){
		disp_str("C");
		disp_str(" ");
		milli_delay(1000);
	}
}