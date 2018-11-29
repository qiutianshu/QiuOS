#include "type.h"
#include "const.h"
#include "tty.h"
#include "console.h"
#include "protect.h"
#include "proc.h"
#include "proto.h"
#include "global.h"



PUBLIC int kernel_main(){
	disp_str("------------------kernel main begins---------------------\n");
	TASK*		p_task 			= task_table;						//有待加入进程表的进程信息
	PROCESS* 	p_proc 			= proc_table;						//进程表
	char*		p_task_stack	= task_stack + STACK_SIZE_TOTAL;
	u16			selector_ldt 	= SELECTOR_LDT_FIRST;
	int 	i;
	u8 		privilege;
	u8 		rpl;
	int 	eflags;
	int prior;

	for(i=0; i<NR_TASKS + NR_PROCS; i++){
		if(i < NR_TASKS){							//任务
			p_task = task_table + i;
			privilege = PRIVILEGE_TASK;
			rpl = SA_RPL1;
			eflags = 0x1202;						//开启所有IO权限，打开中断
			prior = 15;
		}
		else{
			p_task = user_proc_table + (i - NR_TASKS);
			privilege = 3;//PRIVILEGE_USER;
			rpl = SA_RPL3;
			eflags = 0x202; 						//剥夺IO权限
			prior = 5;
		}
		/*初始化进程表工作*/
		strcpy(p_proc->p_name,p_task->name);		
		p_proc->pid = i;							
		p_proc->ldt_sel = selector_ldt;

		memcpy(&p_proc->ldts[0],&gdt[SELECTOR_KERNEL_CS >> 3],sizeof(Descriptor));		//复制内核全局描述符
		p_proc->ldts[0].attr1 = DA_C | privilege << 5;									//修改特权级为1
		memcpy(&p_proc->ldts[1],&gdt[SELECTOR_KERNEL_DS >> 3],sizeof(Descriptor));
		p_proc->ldts[1].attr1 = DA_DAW | privilege << 5;

		p_proc->regs.cs = (0 & 0xfffc & 0xfffb) | SA_L | rpl;
		p_proc->regs.ds = (8 & 0xfffc & 0xfffb) | SA_L | rpl;
		p_proc->regs.es = (8 & 0xfffc & 0xfffb) | SA_L | rpl;
		p_proc->regs.fs = (8 & 0xfffc & 0xfffb) | SA_L | rpl;
		p_proc->regs.ss = (8 & 0xfffc & 0xfffb) | SA_L | rpl;
		p_proc->regs.gs = (SELECTOR_KERNEL_GS & 0xfffc) | rpl;
		p_proc->regs.eip = (u32)p_task->init_eip;
		p_proc->regs.esp = (u32)p_task_stack;
		p_proc->regs.eflags = eflags;			

		p_task_stack -= p_task->stack_size;

		p_proc->ticks = p_proc->priority = prior;	//设置优先级

		p_proc->p_flags = 0;
		p_proc->p_msg = 0;
		p_proc->p_recvfrom = NO_TASK;
		p_proc->p_sendto = NO_TASK;
		p_proc->has_int_msg = 0;
		p_proc->q_sending = 0;
		p_proc->next_sending = 0;
		
		p_proc++;
		p_task++;
		selector_ldt +=1 << 3;

		p_proc->nr_tty = 0;							//初始化指向0号
	}
	init_clock();									//初始化时钟
	init_keyboard();								//初始化键盘

	/*指定进程对应的控制台*/
	proc_table[1].nr_tty = 0;
	proc_table[2].nr_tty = 0;
	proc_table[3].nr_tty = 2;

	

	ticks = 0;
	k_reenter = 0;
	
	p_proc_ready = proc_table;
	restart();
	while(1){}
}


void TestA(){
	MESSAGE msg;
	while(1){
		reset_msg(&msg);
		msg.type = GET_TICKS;
		send_recv(BOTH, TASK_SYS, &msg);
		printf("<Ticks:%d> ", msg.RETVAL);
		milli_delay(200);					//20 ticks，打印下一个A之前发生20次时钟中断
	}
}

void TestB(){
	while(1){
		milli_delay(200);
	}
}

void TestC(){
	while(1){
	//	disp_color_str("C",0xd);
	//	disp_str(" ");
		milli_delay(200);
	}
}
