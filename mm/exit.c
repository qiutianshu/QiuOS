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

/*进程表p_flags置空，发送消息给父进程解除阻塞*/
PRIVATE void clean(int pid){
	proc_table[pid].p_flags = FREE_SLOT;
	MESSAGE msg2parent;
	msg2parent.type = SYSCALL_RET;
	msg2parent.PID = pid;
	msg2parent.STATUS = proc_table[pid].exit_status;  
	send_recv(SEND, proc_table[pid].p_parent, &msg2parent);		//子进程返回状态码，解除父进程阻塞
}

PUBLIC void do_exit(int status){
	int pid = mm_msg.source;          	//要退出的进程号
	PROCESS* p = &proc_table[pid];		//要退出的进程
	int parent_pid = proc_table[pid].p_parent;
	int i;
	/*通知文件系统，解除对文件的占用*/
	MESSAGE msg2fs;
	msg2fs.type = EXIT;
	msg2fs.PID = pid;
	send_recv(BOTH, TASK_FS, &msg2fs);
//	printl("exit %d\n ", msg2fs.type);
	/*释放进程空间*/
//	free_mm(pid);
	p->exit_status = status;

	if(proc_table[parent_pid].p_flags & WAITING){		//父进程正在等待
		proc_table[parent_pid].p_flags &= ~WAITING;		//解除父进程等待标志
		clean(pid);										//清除子进程表
	}
	else{
		p->p_flags |= HANGING;
	}													//父进程没有等待
	for(i = NR_TASKS + NR_NATIVE_PROCS; i < NR_TASKS + NR_PROCS; i++){
		if(proc_table[i].p_parent == pid){				//子进程的子进程就是孤儿进程
			proc_table[i].p_parent = PROC_INIT;	        //为孤儿进程找个Init好爸爸，这个爸爸真是好，把那些不好好干，想走的孤儿统统杀掉
			if((proc_table[i].p_flags & HANGING) && (proc_table[PROC_INIT].p_flags & WAITING)){   //孤儿进程正在退出并且ｉｎｉｔ等待
				proc_table[PROC_INIT].p_flags &= ~WAITING;                                        //否则等到下次ｉｎｉｔ等待的时候处理
				clean(i);
			}
		}
	}
}

PUBLIC void do_wait(){
	int i;
	int children = 0;
	PROCESS* p = &proc_table[NR_TASKS + NR_NATIVE_PROCS];
	int parent = mm_msg.source;
	for(i = NR_TASKS + NR_NATIVE_PROCS; i < NR_TASKS + NR_PROCS; i++,p++){
		if(p->p_parent == parent){
			children++;
			if(p->p_flags & HANGING){
				clean(i);
				return;
			}
		}
	}
	if(children){
		proc_table[parent].p_flags |= WAITING;
	}
	else{
		MESSAGE msg;
		msg.type = SYSCALL_RET;
		msg.PID = NO_TASK;
		msg.STATUS = -1;
		send_recv(SEND, parent, &msg);
	}
}

