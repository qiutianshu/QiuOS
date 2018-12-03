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
	msg.DEVICE = MINOR(ROOT_DEV);						
	send_recv(BOTH, dd[MAJOR(ROOT_DEV)].drv_pid, &msg);		//发送根设备请求
	spin("FS loop");
}