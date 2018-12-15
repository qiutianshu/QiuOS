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

/*  创建子进程
*执行成功返回0,失败返回-1
*
*/
PUBLIC int do_fork(){
	PROCESS* p = proc_table;
	int i;
	for(i = 0; i < NR_TASKS + NR_PROCS; i++,p++){					//查找空进程表
		if(p->p_flags == FREE_SLOT)
			break;
	}
	assert(i >= NR_TASKS + NR_NATIVE_PROCS);
	if(i == NR_TASKS + NR_PROCS){
		printl("full full");
		return -1;		  											//进程表已满
	}

	int child_pid = i;												//子进程号
	int pid = mm_msg.source;										//父进程号
	u16 child_ldt_sel = p->ldt_sel;
	memcpy(p, &proc_table[pid], sizeof(PROCESS));					//复制父进程表
	p->ldt_sel = child_ldt_sel;
	p->p_parent = pid;												//设置父进程号
	sprintf(p->p_name, "%s %d", proc_table[pid].p_name, child_pid);		//子进程名 = 父进程名　＋　子进程号

	/*复制父进程ldt*/
    Descriptor* des = &proc_table[pid].ldts[INDEX_LDT_C];	//父进程代码段

	int parent_T_base = (des->base_high << 24) + (des->base_mid << 16) + des->base_low;				//代码段基址
	int parent_T_limit = ((des->limit_high_attr2 & 0xf) << 16) + des->limit_low;				//代码段界限
	int parent_T_size = (parent_T_limit + 1) * ((des->limit_high_attr2 & 0x80) ? 4096 : 1); //代码段长度

	des = &proc_table[pid].ldts[INDEX_LDT_D];						//父进程数据段

	int parent_DS_base = (des->base_high << 24) + (des->base_mid << 16) + des->base_low;				//数据段基址
	int parent_DS_limit = ((des->limit_high_attr2 & 0xf) << 16) + des->limit_low;				//数据段界限
	int parent_DS_size = (parent_T_limit + 1) * ((des->limit_high_attr2 & 0x80) ? 4096 : 1);//数据段长度

	assert((parent_T_base == parent_DS_base) && 
		   (parent_T_limit == parent_DS_limit) &&
		   (parent_T_size == parent_DS_size));

	int child_base = alloc_mem(child_pid, parent_T_size);
	/*复制父进程内容至子进程*/
	phy_cpy((void*)child_base, (void*)parent_T_base, parent_T_size);
	/*修改子进程描述符*/
	init_descriptor(&p->ldts[INDEX_LDT_C], 
		            child_base, 
		            ((PROC_IMG_DEFAULT_SIZE - 1) >> 12),   					//除以４k
		            DA_32 | DA_C | DA_LIMIT_4K | PRIVILEGE_USER << 5);	//32位４K代码段
	init_descriptor(&p->ldts[INDEX_LDT_D],
					child_base,
					((PROC_IMG_DEFAULT_SIZE - 1) >> 12),
					DA_32 | DA_DRW | DA_LIMIT_4K | PRIVILEGE_USER << 5);//32位４k数据段

	MESSAGE msg2fs;
	msg2fs.type = FORK;
	msg2fs.PID = child_pid;
	send_recv(BOTH, TASK_FS, &msg2fs);							//父进程通知文件系统共享文件

	mm_msg.PID = child_pid;										//子进程号返回父进程

	MESSAGE msg;
	msg.type = SYSCALL_RET;
	msg.RETVAL = 0;
	msg.PID = 0;
	send_recv(SEND, child_pid, &msg);							//子进程解除阻塞，同时通知子进程其进程号为０

	return 0;
}