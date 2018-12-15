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

/*		创建子进程
*成功：	父进程中返回子进程的PID，子进程中返回0		
*
*失败：	父进程中返回-1
*
*/
PUBLIC int fork(){
	MESSAGE msg;
	msg.type = FORK;

	send_recv(BOTH, TASK_MM, &msg);

	assert(msg.type == SYSCALL_RET);
	assert(msg.RETVAL == 0);

	return msg.PID;
}