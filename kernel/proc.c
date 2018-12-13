#include "type.h"
#include "const.h"
#include "fs.h"
#include "protect.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"

/*
	系统调用处理例程
*/
PUBLIC int sys_get_ticks(){
	return ticks;
}

PUBLIC void schedule(){
	PROCESS* p;
	int greatest_ticks = 0;
	while(!greatest_ticks){
		for(p = proc_table; p<proc_table + NR_TASKS + NR_PROCS; p++){
			if(!p->p_flags){											//可运行状态
				if(p->ticks > greatest_ticks){
					greatest_ticks = p->ticks;
					p_proc_ready = p;
				}
			}
		}
		if(!greatest_ticks)
			for(p=proc_table; p<proc_table+NR_TASKS + NR_PROCS; p++)
				if(!p->p_flags)
					p->ticks = p->priority;
	}
	
}

//PUBLIC void sys_sendrec()
PUBLIC int ldt_seg_linear(PROCESS* p, int idx){
	Descriptor* d = &p->ldts[idx];
	return d->base_high << 24 | d->base_mid << 16 | d->base_low;
}

PUBLIC void* va2la(int pid, void* va){
	PROCESS* p = &proc_table[pid];
	u32 seg_base = ldt_seg_linear(p, 1);
	u32 la = seg_base + (u32)va;
	return (void*) la;
}

PUBLIC void reset_msg(MESSAGE* msg){
	memset(msg, 0, sizeof(MESSAGE));
}


PRIVATE void block(PROCESS* p){
	assert(p->p_flags);
	schedule();
}

PRIVATE void unblock(PROCESS* p){
	assert(p->p_flags == 0);
}

/*	检测消息死锁
*	src  谁发送消息
*	dest 谁接收消息
*/
PRIVATE int deadlock(int src, int dest){		
	PROCESS* p = proc_table + dest;
	while(1){
		if(p->p_flags & SENDING){				//目标进程正在发送消息
			if(p->p_sendto == src)				//找到闭环
				return 1;
			p = proc_table + p->p_sendto;		//沿消息队列寻找
		}
		else
			break;
	}
	return 0;									//不存在死锁
}

/*	发送消息
*	p_proc 	当前进程
*	dest 	目标进程
*	msg     指向消息的指针
*/
PRIVATE int send_msg(PROCESS* p_proc, int dest, MESSAGE* msg){
	PROCESS* sender = p_proc;
	PROCESS* p_dest = proc_table + dest;
	assert(proc2pid(sender) != dest);

	if(deadlock(proc2pid(sender), dest))						//检测死锁
		panic("deadlocl detected!! from %s to %s",sender->p_name, proc_table[dest].p_name);

	if((p_dest->p_flags & RECEIVING) && 						//此时B进程应该是挂起的
		((p_dest->p_recvfrom == proc2pid(sender)) 
			|| (p_dest->p_recvfrom == ANY))){
		assert(p_dest->p_msg);									//确保B进程已经准备好接收消息体
		assert(msg);											//消息体有效

		phy_cpy(va2la(dest, p_dest->p_msg), va2la(proc2pid(sender), msg), sizeof(MESSAGE));		//复制消息到目标进程

		p_dest->p_msg = 0; 										//消息发送成功，B进程表相关变量清除
		p_dest->p_flags &= ~RECEIVING;
		p_dest->p_recvfrom = NO_TASK;
		unblock(p_dest);
		assert(p_dest->p_flags == 0);							//确保B进程相关标志清除
		assert(p_dest->p_msg == 0);
		assert(p_dest->p_recvfrom == NO_TASK);
		assert(p_dest->p_sendto == NO_TASK);
		assert(sender->p_flags == 0);						
		assert(sender->p_msg == 0);
		assert(sender->p_recvfrom == NO_TASK);
		assert(sender->p_sendto == NO_TASK);
	}
	else{											//B进程未准备好接收消息，则A进程设置好进程表，添加消息队列，挂起
		sender->p_flags |= SENDING;
		sender->p_sendto = dest;
		sender->p_msg = msg;
		/*加入消息队列*/
		PROCESS* p;
		if(p_dest->q_sending){						//消息队列不为空
			p = p_dest->q_sending;
			while(p->next_sending)					//指向下一个进程块
				p = p->next_sending;
			p->next_sending = sender;				//加入队列
		}
		else
			p_dest->q_sending = sender;				//消息队列为空
		sender->next_sending = 0;					//队列尾

		block(sender);								//阻塞当前进程

		assert(sender->p_flags == SENDING);			//确保A进程表相关变量已经设置到位
		assert(sender->p_msg != 0);
		assert(sender->p_sendto == dest);
		assert(sender->p_recvfrom == NO_TASK);
	}
	return 0;
}

/*	接收消息
*	p_proc 	当前进程
*	dest 	目标进程
*	msg     指向消息的指针
*/
PRIVATE int recv_msg(PROCESS* p_proc, int src, MESSAGE* msg){
	PROCESS* p_recv = p_proc;
	PROCESS* p_from = 0;
	PROCESS* prev = 0;
	int copy = 0;

	assert(proc2pid(p_recv) != src);

	if((p_recv->has_int_msg) &&
		 ((src == ANY) || 
		 	(src == INTERRUPT))){
		MESSAGE m;
		reset_msg(&m);
		m.source = INTERRUPT;
		m.type = HARD_INT;
		assert(msg);
		phy_cpy(va2la(proc2pid(p_recv), msg), &m, sizeof(MESSAGE));

		p_recv->has_int_msg = 0;
		assert(p_recv->p_flags == 0);
		assert(p_recv->p_msg == 0);
		assert(p_recv->p_sendto == NO_TASK);
		assert(p_recv->has_int_msg == 0);
		return 0;

	}

	if((!(p_recv->has_int_msg)) && (src == INTERRUPT)){		//中断消息还没来
		p_recv->p_flags |= RECEIVING;					
		p_recv->p_msg = msg;
		p_recv->p_recvfrom = INTERRUPT;
		block(p_recv);									

		assert(p_recv->p_flags == RECEIVING);	
		assert(p_recv->p_msg != 0);
		assert(p_recv->p_recvfrom = INTERRUPT);
		assert(p_recv->p_sendto == NO_TASK);
		assert(p_recv->has_int_msg == 0);

		return 0;
	}
	if(src == ANY){									
		if(p_recv->q_sending){									//消息队列不为空
			p_from = p_recv->q_sending;
			copy = 1;	
			assert(p_recv->p_flags == 0);
			assert(p_recv->p_msg == 0);
			assert(p_recv->p_recvfrom == NO_TASK);				//   ???????
			assert(p_recv->p_sendto == NO_TASK);
			assert(p_recv->q_sending != 0);
			assert(p_from->p_flags == SENDING);
			assert(p_from->p_msg != 0);
			assert(p_from->p_recvfrom == NO_TASK);
			assert(p_from->p_sendto == proc2pid(p_recv));
		}
	}
	else{														//接收特定进程的消息
		p_from = &proc_table[src];	

		if((p_from->p_flags & SENDING) && (p_from->p_sendto == proc2pid(p_recv))){
			copy = 1;

			PROCESS* p = p_recv->q_sending;
			assert(p);
			while(p){
				if(proc2pid(p) == src){
					p_from = p;
					break;
				}
				prev = p;
				p = p->next_sending;
			}
			assert(p_recv->p_flags == 0);							//此时A进程处于阻塞状态
			assert(p_recv->p_msg == 0);
			assert(p_recv->p_sendto == NO_TASK);
			assert(p_recv->q_sending != 0);
			assert(p_from->p_flags == SENDING);
			assert(p_from->p_msg != 0);
			assert(p_from->p_recvfrom == NO_TASK);
			assert(p_from->p_sendto == proc2pid(p_recv));
		}
	}
	if(copy){
		if(p_from == p_recv->q_sending){				//消息队列头
			assert(prev == 0);
			p_recv->q_sending = p_from->next_sending;
			p_from->next_sending = 0;
		}
		else{
			assert(prev);
			prev->next_sending = p_from->next_sending;	//从消息队列中删除特定消息
			p_from->next_sending = 0;
		}

		assert(msg);
		assert(p_from->p_msg);
		phy_cpy(va2la(proc2pid(p_recv), msg), va2la(proc2pid(p_from), p_from->p_msg), sizeof(MESSAGE));

		p_from->p_msg = 0;								//A进程相关标志清除
		p_from->p_sendto = NO_TASK;
		p_from->p_flags &= ~SENDING;
		unblock(p_from);
	}
	else{
		p_recv->p_flags |= RECEIVING;					//设置B进程相关标志
		p_recv->p_msg = msg;
		if(src == ANY)
			p_recv->p_recvfrom = ANY;
		else
			p_recv->p_recvfrom = proc2pid(p_from);

		block(p_recv);									//当消息队列为空，或者目标进程未准备好，进程阻塞

		assert(p_recv->p_flags == RECEIVING);			//确保B进程相关标志设置正确
		assert(p_recv->p_msg != 0);
		assert(p_recv->p_recvfrom != NO_TASK);
		assert(p_recv->p_sendto == NO_TASK);
		assert(p_recv->has_int_msg == 0);
	}
	return 0;
}

PUBLIC int sys_sendrec(int function, int src_dest, MESSAGE* m, PROCESS* p){
	assert(k_reenter == 0);						//确保在核心态
	assert((src_dest >= 0 && src_dest < NR_TASKS + NR_TASKS) || 
		src_dest == ANY || src_dest == INTERRUPT);

	int caller = proc2pid(p);
	MESSAGE* mla = (MESSAGE*)va2la(caller, m);
	mla->source = caller;

	if(function == SEND)
		return (send_msg(p, src_dest, m));

	if(function == RECEIVE)
		return (recv_msg(p, src_dest, m));
	else
		panic("unknown message function in kernel/proc.c line: 305");

	return 0;
}


PUBLIC void info_task(int pid){
	PROCESS* p = proc_table + pid;
	if((p->p_flags == RECEIVING) && (p->p_recvfrom == INTERRUPT || p->p_recvfrom == ANY)){			//B进程等待接收中断消息
		p->p_flags &= ~RECEIVING;
		p->p_msg->source = INTERRUPT;
		p->p_msg->type = HARD_INT;
		p->p_msg = 0;
		p->p_recvfrom = NO_TASK;
		p->p_sendto = NO_TASK;
		p->has_int_msg = 0;

		unblock(p);
	}
	else
		p->has_int_msg = 1;						
}











