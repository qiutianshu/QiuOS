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
			case GET_PID:
				msg.PID = src;
				msg.type = SYSCALL_RET;
				send_recv(SEND, src, &msg);
				break;
			case FIND_PROCS:
				if(proc_table[msg.PID].p_flags != FREE_SLOT){
					char* name = va2la(src, msg.BUF);
					int len = strlen(proc_table[msg.PID].p_name);
					phy_cpy(name, proc_table[msg.PID].p_name, len);
					name[len] = 0;
					msg.FLAGS = 1;
				}
				else
					msg.FLAGS = 0;
				msg.type = SYSCALL_RET;
				
				send_recv(SEND, src, &msg);
				break;
			default:
				panic("unknown message type!!");
				break;
		}
	}
}