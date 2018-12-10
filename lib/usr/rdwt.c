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

PUBLIC int read(int fd, void* buf, int count){
	MESSAGE msg;
	msg.type = 	FILE_READ;
	msg.BUF = buf;
	msg.FD = fd;
	msg.COUNT = count;

	send_recv(BOTH, TASK_FS, &msg);

	return msg.COUNT;
}

PUBLIC int write(int fd, void* buf, int count){
	MESSAGE msg;
	msg.type = FILE_WRITE;
	msg.BUF = buf;
	msg.COUNT = count;
	msg.FD = fd;

	send_recv(BOTH, TASK_FS, &msg);

	return msg.COUNT;
}