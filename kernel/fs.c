#include "type.h"
#include "const.h"
#include "tty.h"
#include "console.h"
#include "protect.h"
#include "proc.h"
#include "proto.h"
#include "hd.h"
#include "global.h"

PUBLIC void task_fs(){
	MESSAGE msg;
	msg.type = DEV_OPEN;
	send_recv(BOTH, TASK_HD, &msg);
	spin("FS loop");
}