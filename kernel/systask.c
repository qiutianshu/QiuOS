#include "type.h"
#include "const.h"
#include "fs.h"
#include "tty.h"
#include "console.h"
#include "protect.h"
#include "proc.h"
#include "proto.h"
#include "global.h"

PUBLIC void task_sys(){
	MESSAGE msg;
	while(1){
		send_recv(RECEIVE, ANY, &msg);				//接收进程A的请求消息
		int src = msg.source;
		switch(msg.type){
			case GET_TICKS:
				msg.RETVAL = ticks;
				send_recv(SEND, src, &msg);
				break;
			default:
				panic("unknown message type!!");
				break;
		}
	}
}