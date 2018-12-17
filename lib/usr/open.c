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

PUBLIC int open(char* filepath, int flag){
	MESSAGE msg;
	msg.type		= FILE_OPEN;
	msg.PATHNAME	= filepath;
	msg.FLAGS 		= flag;
	msg.NAME_LEN	= strlen(filepath);

	send_recv(BOTH, TASK_FS, &msg);
	assert(msg.type == SYSCALL_RET);
	return msg.FD;
}

PUBLIC int close(int fd){
	MESSAGE msg;
	msg.type		= FILE_CLOSE;
	msg.FD 			= fd;
	send_recv(BOTH, TASK_FS, &msg);
	assert(msg.type == SYSCALL_RET); 
	return msg.RETVAL;
}