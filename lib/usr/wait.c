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

/*		等待子进程退出
*参数：指向整数的指针，该指针用于返回子进程的退出状态码
*
*返回值：执行成功返回子进程号，失败则返回-1
*/
PUBLIC int wait(int* status){
	MESSAGE msg;
	msg.type = WAIT;
	send_recv(BOTH, TASK_MM, &msg);
	*status = msg.STATUS;
	return ((msg.PID == NO_TASK) ? -1 : msg.PID);
}