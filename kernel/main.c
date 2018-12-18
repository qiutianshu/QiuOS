#include "type.h"
#include "const.h"
#include "fs.h"
#include "tty.h"
#include "console.h"
#include "protect.h"
#include "proc.h"
#include "proto.h"
#include "global.h"

/*tar包解压缩*/
PUBLIC void untar(const char* filename){
	printf("extract file: %s\n", filename);
	struct posix_tar_header* phd;
	char buf[8192];
	int fd_out;
	int size;												//压缩包内每个文件的大小
	int i = 0;													
	int fd = open(filename, O_RW);
	assert(fd != -1);
	while(1){
		read(fd, buf, 512);									//读取tar文件头
		if(buf[0] == 0){
			if(i == 0)
				printf("cmd.tar is the latest\n");
			break;
		}
		i++;

		size = 0;
		phd = (struct posix_tar_header*)buf;
		fd_out = open(phd->name, O_CREATE | O_RW | O_TRUNC);		
		if(fd_out == -1){
			printf("failed to extract %s\n aborted", phd->name);
			close(fd);
			return;
		}			
		assert(fd_out != -1);
		char* p = phd->size;
		while(*p)
			size = (size * 8) + (*p++ - '0');

		printf(" %s(%d)\n", phd->name, size);
		while(size){
			int bytes = min(size, 8192);
			read(fd, buf, ((bytes - 1) / 512 + 1) * 512);	//读取整个扇区(tar文件扇区对齐)
			write(fd_out, buf, bytes);
			size -= bytes;
		}
		close(fd_out);
	}
	if(i){
		proc_table[getpid()].filp[fd]->fd_pos = 0;			//tar包第一个字节设置为0
		buf[0] = 0;
		int n = write(fd, buf, 1);
		assert(n == 1);
	}
	close(fd);
	printf("extract done!\n");
} 

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
		
		if((i >= NR_TASKS + NR_NATIVE_PROCS) && (i < NR_TASKS + NR_PROCS)){
			proc_table[i].p_flags = FREE_SLOT;					//进程表未使用
			continue;
		}
		/*初始化进程表工作*/
		strcpy(p_proc->p_name,p_task->name);		
		p_proc->p_parent = NO_TASK;
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
	}
	init_clock();									//初始化时钟

	ticks = 0;
	k_reenter = 0;
	
	p_proc_ready = proc_table;
	restart();
	while(1){}
}


void TestA(){
//	while(1){}
/*	char buf[32];
	int fd = open("/qts", O_RW | O_CREATE);
	int fd1 = open("/qwerr",O_RW | O_CREATE);
	int fd2 = open("/ccc", O_RW | O_CREATE);
	int fd3 = open("/iiii",O_RW | O_CREATE);
	int fd4 = open("/dev_tty2",O_RW);


	write(fd3,"eeee",4);
	close(fd3);
	int fd5 = open("/iiii",O_RW);
	read(fd5,buf,4);

	buf[4] = 0;
	write(fd4,buf,4);
	
	close(fd5);
	close(fd4);
	close(fd1);
	close(fd2);
	close(fd);*/
	while(1){}

}

void TestB(){
	while(1){}
}

void TestC(){
	while(1){}
}

void Init(){
	int fd_stdin = open("/dev_tty0", O_RW);
	assert(fd_stdin == 0);
	int fd_stdout = open("/dev_tty0", O_RW);
	assert(fd_stdout == 1); 
	
	int i;

	/*解压缩应用程序*/
	untar("cmd.tar");
	close(fd_stdin);
	close(fd_stdout);
	char* tty_list[] = {"/dev_tty1","/dev_tty2"};
	
	for(i = 0; i < sizeof(tty_list)/sizeof(tty_list[0]); i++){
		int pid = fork();
		if(pid);
		else{
			ti_shell(tty_list[i]);			//ti_shell
			assert(0);
		}
	}
	while (1) {
		int s;
		int child = wait(&s);
		printf("child (%d) exited with status: %d.\n", child, s);
	}
	while(1){}
	
}