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

PUBLIC int execl(const char* path ,const char* arg1,...){
	char* parg = (char*)&arg1;
	char **p = (char**)parg;					//指向指针数组的指针
	return execv(path, p);
}

PUBLIC int execv(const char* path, char* argv[]){
	MESSAGE msg2mm;
	char **p = argv;
	int len = 0;
	while(*p++)
		len += sizeof(char*);		//计算给指针数组预留的空间（参数个数 × 4）

	msg2mm.type = EXEC;
	msg2mm.PATHNAME = (char*)path;
	msg2mm.NAME_LEN = strlen(path);
	msg2mm.BUF      = argv;
	msg2mm.BUF_LEN	= len;
	send_recv(BOTH, TASK_MM, &msg2mm);
	assert(msg2mm.type == SYSCALL_RET);

	return msg2mm.RETVAL;
}