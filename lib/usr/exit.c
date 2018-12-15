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

/*		退出进程
*参数：子进程的状态码返回给父进程
*
*/
PUBLIC void exit(int status){
	MESSAGE msg;
	msg.type = EXIT;
	msg.STATUS = status;
	send_recv(BOTH, TASK_MM, &msg);
	assert(msg.RETVAL == SYSCALL_RET); 
}