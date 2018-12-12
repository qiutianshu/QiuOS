#include "type.h"
#include "const.h"
#include "fs.h"
#include "tty.h"
#include "console.h"
#include "protect.h"
#include "proc.h"
#include "proto.h"
#include "hd.h" 
#include "global.h"


PUBLIC void task_mm(){
	total_memory_size = *(int*)MEMSIZE;
	printl("Total memory size: %dMb \n", (total_memory_size)/(1024 * 1024));
	while(1){
		send_recv(RECEIVE, ANY, &mm_msg);
		int src = mm_msg.source;
		int reply = 1;

		switch(mm_msg.type){
			case FORK:
				mm_msg.RETVAL = do_fork();
				break;
			default:
				panic("unknown message!");
				break;
		}
		if(reply){
			mm_msg.type = SYSCALL_RET;
			send_recv(SEND, src, &mm_msg);
		}
	}
}

/*	分配用户进程空间
*
*参数：子进程号，大小
*返回值：子进程空间线性地址
*
*/
PUBLIC int alloc_mem(int pid, int size){
	assert(pid >= NR_TASKS + NR_NATIVE_PROCS);
	if(size > PROC_IMG_DEFAULT_SIZE)
		panic("unsupported memory request size %d,(should be less than %d)", size, PROC_IMG_DEFAULT_SIZE);

	int base = PROC_BASE + (pid - NR_TASKS - NR_NATIVE_PROCS) * PROC_IMG_DEFAULT_SIZE;		//子进程空间粒度为１Ｍｂ

	if(base + size >= total_memory_size)					//超过可用内存大小
		panic("memory alloc failed! pid: %d", pid);

	return base;
}