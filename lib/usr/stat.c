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

PUBLIC int stat(char* pathname, struct status* s){
	MESSAGE msg;
	msg.type = FILE_STAT;
	msg.PATHNAME = (void*)pathname;
	msg.STAT = (void*)s;
	msg.NAME_LEN = strlen(pathname);
	send_recv(BOTH, TASK_FS, &msg);
	assert(msg.type == SYSCALL_RET);
	return msg.RETVAL;
}