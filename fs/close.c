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

PUBLIC int do_close(){
	int i = 0;
	int fd = fs_msg.FD;
	put_inode(caller->filp[fd]->fd_inode);				//inode计数减一
	caller->filp[fd]->fd_inode = 0;
	caller->filp[fd] = 0;
	return 0;
}