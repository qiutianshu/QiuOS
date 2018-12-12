#include "type.h"
#include "const.h"
#include "fs.h"
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
		else if(i >= NR_TASKS && i < (NR_TASKS + NR_NATIVE_PROCS)){
			p_task = user_proc_table + (i - NR_TASKS);
			privilege = 3;							//PRIVILEGE_USER;
			rpl = SA_RPL3;
			eflags = 0x202; 						//剥夺IO权限
			prior = 5;
		}
		else{
			p_proc->p_flags = FREE_SLOT;					//进程表未使用
			continue;
		}
		/*初始化进程表工作*/
		strcpy(p_proc->p_name,p_task->name);		
//		p_proc->pid = i;	
		p_proc->p_parent = NO_TASK;
//		p_proc->ldt_sel = selector_ldt;
		if(strcmp(p_task->name , "Init") != 0){		//普通进程
			memcpy(&p_proc->ldts[INDEX_LDT_C],&gdt[SELECTOR_KERNEL_CS >> 3],sizeof(Descriptor));		//复制内核全局描述符
			p_proc->ldts[INDEX_LDT_C].attr1 = DA_C | privilege << 5;									//修改特权级为1
			memcpy(&p_proc->ldts[INDEX_LDT_D],&gdt[SELECTOR_KERNEL_DS >> 3],sizeof(Descriptor));
			p_proc->ldts[INDEX_LDT_D].attr1 = DA_DRW | privilege << 5;
		}
		else{
			u32 k_base;
			u32 k_limit;
			int ret = get_kernel_map(&k_base, &k_limit);
			printl("k_base is %d, k_limit is %d\n", k_base,k_limit);
			assert(ret == 0);
			init_descriptor(&p_proc->ldts[INDEX_LDT_C], 
							0, 
							(k_base + k_limit) >> 12,          //除以4k
							DA_32 | DA_LIMIT_4K | DA_C | privilege << 5);
			init_descriptor(&p_proc->ldts[INDEX_LDT_D],
							0,
							(k_base + k_limit) >> 12,
							DA_32 | DA_LIMIT_4K | DA_DRW | privilege << 5);
		}

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

		for(int k = 0; k < NR_FILES; k++)
			p_proc->filp[k] = 0;					//初始化文件打开表
		
		p_proc++;
//		selector_ldt +=1 << 3;
	}
	init_clock();									//初始化时钟

	ticks = 0;
	k_reenter = 0;
	
	p_proc_ready = proc_table;
	restart();
	while(1){}
}


void TestA(){
	while(1){
	}
/*	char* filename[] = {"/qts","/foo2","foo3","foo4"};
	char* rfilename[] = {"/qts","/foo2","/dev_tty1","/"};
	int fd;
	for(int i = 0; i < 4; i++){
		fd = open(filename[i], O_CREATE | O_RW);			//新建QTS文件
	//	write(fd, "qwertyui", 8);
		close(fd);
	}
	spin("create file");
	for(int j = 0; j < 4; j++){
		if(unlink(rfilename[j]) == 0)
			printl("file removed : %s\n ", rfilename[j]);
		else
			printl("file removed failed : %s\n ",rfilename[j]);
	}

	spin("TestA");*/
}

void TestB(){
	char tty_name[] = "/dev_tty2";

	int fd_stdin = open(tty_name, O_RW);
	int fd_stdout = open(tty_name, O_RW);
	char rdbuf[128];

	printf("\n");

	while(1){
		printf("%s", "$ ");
		int r = read(fd_stdin, rdbuf, 77);
		rdbuf[r] = 0;
 
		if(strcmp(rdbuf, "hallo") == 0){
			printf("%s\n", "hallo qts");
		}
		else{
			if(rdbuf[0]){
				printf("{%s}\n", rdbuf);
			}
		}
	}
	assert(0);
}

void TestC(){
	while(1){
		milli_delay(200);
	}
}

void Init(){
	int fd_stdin = open("/dev_tty1", O_RW);
	assert(fd_stdin == 0);
	int fd_stdout = open("/dev_tty1", O_RW);
	assert(fd_stdout == 1);

	printf("Init() is running     \n");

	int pid = fork();
	if(pid){
		printf("in parent process, child pid is: %d\n", pid);
		spin("parent");
	}
	else{
		printf("in child process, child pid is: %d\n", pid);
		spin("child");
	}
}