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

PUBLIC int getpid(){
	MESSAGE msg;
	msg.type = GET_PID;
	send_recv(BOTH, TASK_SYS, &msg);
	assert(msg.type == SYSCALL_RET);
	return msg.PID;
};