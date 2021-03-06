#include "type.h"
#include "const.h"
#include "fs.h"
#include "keyboard.h"
#include "tty.h"
#include "console.h"
#include "protect.h"
#include "proc.h"
#include "proto.h"
#include "global.h"

PRIVATE void init_tty(TTY* p_tty){
	p_tty->count = 0;
	p_tty->head = p_tty->tail = p_tty->buf;

	int nr_tty = p_tty - tty_table;
	p_tty->p_console = console_table + nr_tty;

	int console_mem_size = (CONSOLE_SIZE >> 1) / NR_CONSOLES ; 		//每个控制台分配的显存大小
	p_tty->p_console->origin_addr = nr_tty * console_mem_size;		//当前现存位置
	p_tty->p_console->mem_limit = console_mem_size;
	p_tty->p_console->current_start_addr = nr_tty * console_mem_size;
	p_tty->p_console->cursor = p_tty->p_console->origin_addr;		//当前光标位置定为起始显示位置

	if(nr_tty == 0){
		p_tty->p_console->cursor = disp_pos / 2;					//第一个控制台的光标位置为
	}
	set_cursor(p_tty->p_console->cursor);

}

PRIVATE void send_to_tty_buf(TTY* p_tty, char ch){
	if(p_tty->count < TTY_BUF_SIZE){
			*(p_tty->head) = ch;			//写入tty缓冲区
			p_tty->head++;
			if(p_tty->head == p_tty->buf+TTY_BUF_SIZE)
				p_tty->head = p_tty->buf;
			p_tty->count++;
		}
}

PUBLIC int is_current_console(CONSOLE* p_cone){
	return (p_cone == console_table+current_console);
}

PRIVATE void tty_read(TTY* p_tty){
	if(is_current_console(p_tty->p_console))
		keyboard_read(p_tty);
}

PRIVATE void tty_write(TTY* tty){
	while(tty->count){
		char ch = *(tty->tail);
		tty->tail++;
		if(tty->tail >= tty->buf + TTY_BUF_SIZE)
			tty->tail = tty->buf;
		tty->count--;

		if(tty->tty_left_cnt){
			if(ch >= ' ' && ch <= '~'){		//可显示字符
				disp_char(tty->p_console, ch);
				void* p = tty->req_buf + tty->tty_trans_cnt;		
				phy_cpy(p, va2la(TASK_TTY, &ch), 1);      //写入进程p缓冲区 
				tty->tty_trans_cnt++;
				tty->tty_left_cnt--;
			}
			else if(ch == '\b' && tty->tty_trans_cnt){
				disp_char(tty->p_console, ch);
				tty->tty_trans_cnt--;
				tty->tty_left_cnt++;
			}
			if(ch == '\n' || tty->tty_left_cnt == 0){
				disp_char(tty->p_console, ch);
				MESSAGE msg;
				msg.type = RESUME_PROC;
				msg.PROC_NR = tty->tty_procnr;
				msg.COUNT = tty->tty_trans_cnt;
				send_recv(SEND, tty->tty_caller, &msg);		//通知文件系统解除对进程p的阻塞
				tty->tty_left_cnt = 0;
			}
		}
	}
}
/*
			弃用

PRIVATE void tty_write(TTY* p_tty){
	if(p_tty->count){
		char ch = *(p_tty->tail);
		p_tty->tail++;
		if(p_tty->tail == p_tty->buf + TTY_BUF_SIZE)
			p_tty->tail = p_tty->buf;

		p_tty->count--;

		disp_char(p_tty->p_console,ch);
	}
}*/

PRIVATE void tty_do_read(TTY* tty, MESSAGE* msg){
	/*设置对应tty成员变量*/
	tty->tty_caller = msg->source;
	tty->tty_procnr = msg->PROC_NR;
	tty->req_buf = (void*)va2la(tty->tty_procnr, msg->BUF);
	tty->tty_left_cnt = msg->COUNT;
	tty->tty_trans_cnt = 0;

	msg->type = SUSPEND_PROC;				//挂起请求进程p
	//msg->COUNT = tty->tty_left_cnt;
	send_recv(SEND, tty->tty_caller, msg);
}

PRIVATE void tty_do_write(TTY* tty, MESSAGE* msg){
	char* p = (char*)va2la(msg->PROC_NR, msg->BUF);
	int i = msg->COUNT;

	while(i){
		disp_char(tty->p_console, *p);
		p++;
		i--;
	}
	msg->type = SYSCALL_RET;
	send_recv(SEND, msg->source, &msg);
}

PUBLIC void task_tty(){
	TTY* p_tty;	
	MESSAGE msg;


	init_keyboard();
												                        //初始化键盘
	for(p_tty=tty_table; p_tty<tty_table+NR_CONSOLES; p_tty++)			//初始化tty_table
		init_tty(p_tty);

	switch_console(0);			//初始化为0号控制台

	keyboard_pressed = 0;
	while(1){
		for(p_tty=tty_table; p_tty<tty_table+NR_CONSOLES; p_tty++){
			do{
				tty_read(p_tty);
				tty_write(p_tty);
			}while(p_tty->count);
		}
		send_recv(RECEIVE, ANY, &msg);
		int src = msg.source;
		assert(src != TASK_TTY);

		TTY* ptty = &tty_table[msg.DEVICE];

		switch(msg.type){
			case DEV_OPEN:
				reset_msg(&msg);
				msg.type = SYSCALL_RET;
				send_recv(SEND, src, &msg);
				break;
			case DEV_READ:
				tty_do_read(ptty, &msg);
				break;
			case DEV_WRITE:
				tty_do_write(ptty, &msg);
				break;
			case HARD_INT:
				keyboard_pressed = 0;				//键盘记录清零	
				break;
			default:
				break;
		}
	}
}


PUBLIC void in_process(TTY* p_tty,u32 key){
	char output[2] = {'\0','\0'};
	if(!(key & 0x100)){						//显示可打印字符
		send_to_tty_buf(p_tty,key);
	}
	else{
		int raw_code = key & 0x1ff;
		switch(raw_code){
			case UP:
				if((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R))
					scroll_screen(p_tty->p_console,SCR_UP);
				break;
			case DOWN:
				if((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R))
					scroll_screen(p_tty->p_console,SCR_DOWN);
				break;
			case ENTER:
				send_to_tty_buf(p_tty,'\n');
				break;
			case BACKSPACE:
				send_to_tty_buf(p_tty,'\b');
				break;
			case F1:
			case F2:
			case F3:
			case F4:
			case F5:
			case F6:
			case F7:
			case F8:
			case F9:
			case F10:
			case F11:
			case F12:
				if((key & FLAG_ALT_L) || (key & FLAG_ALT_R))
					switch_console(raw_code - F1);
				break;
			default:
				break;
		}
	}
}

PUBLIC void disp_char(CONSOLE* p_cone, char ch){
	u8* pos;
	int nr_tty = p_cone - console_table;
	/***************************************************************************/
	if(p_cone->cursor  >= p_cone->origin_addr + p_cone->mem_limit - SCREEN_WIDTH){				//超过显示范围
		pos = CONSOLE_BASE + p_cone->origin_addr * 2;
		while(pos < CONSOLE_BASE + p_cone->origin_addr * 2 + p_cone->mem_limit * 2){			//清屏
			*pos++ = ' ';			
			*pos++ = CONSOLE_COLOR;
		}
		p_cone->cursor = p_cone->origin_addr;
		p_cone->current_start_addr = p_cone->origin_addr;
		set_vga_start_addr(p_cone->current_start_addr);
		set_cursor(p_cone->cursor);
	}
	/***************************************************************************/
	pos = (u8*)(CONSOLE_BASE + p_cone->cursor * 2);	//指向当前控制台显示地址
	if(ch == '\n')
		if(p_cone->cursor < p_cone->origin_addr + p_cone->mem_limit - SCREEN_WIDTH)
			p_cone->cursor = p_cone->origin_addr + SCREEN_WIDTH * ((p_cone->cursor - p_cone->origin_addr) / SCREEN_WIDTH + 1);

    if(ch == '\b')
		if(p_cone->cursor > p_cone->origin_addr){
			p_cone->cursor--;
			*(pos - 2) = ' ';
			*(pos - 1) = CONSOLE_COLOR;
		}
	 if(ch !='\n' && ch != '\b' && (p_cone->cursor < p_cone->origin_addr + p_cone->mem_limit)){
		*pos++ = ch;									
		*pos++ = CONSOLE_COLOR;								
		p_cone->cursor++;
	}
	while(p_cone->cursor >= p_cone->current_start_addr + 80 *25)			//自动向下翻页
		scroll_screen(p_cone,SCR_DOWN);

	while(p_cone->cursor < p_cone->current_start_addr){			     	    //删除时自动向上翻页
		scroll_screen(p_cone,SCR_UP);
	}																
	set_cursor(p_cone->cursor);							//光标跟随
}

PUBLIC void set_cursor(unsigned int position){
	disable_int();
	out_byte(CRT_CTRL_REG,CURSOR_H);
	out_byte(CRT_DATA_REG,(position >> 8) & 0xff);
	out_byte(CRT_CTRL_REG,CURSOR_L);
	out_byte(CRT_DATA_REG,position & 0xff);  
	enable_int();
}

PUBLIC int sys_printx(int unused1, int unused2, char* str, PROCESS* p_proc){
	const char* p;
	char ch;
	char reenter_error[] = "^ k_reenter is encorrect for unknown reason";
	reenter_error[0] = MAG_CH_PANIC;

	if(k_reenter == 0)								//ring1-ring3
		p = va2la(proc2pid(p_proc), str);			//虚拟地址转化线性地址		
	else if(k_reenter > 0)
		p = str;
	else 
		p = reenter_error;

	if((*p == MAG_CH_PANIC) || ((*p == MAG_CH_ASSERT) && p_proc_ready < &proc_table[NR_TASKS])){
		disable_int();
		char* v = (char*)CONSOLE_BASE;
		char* q = p + 1;									//跳过MAG_CH
		while(v < (char*)(CONSOLE_BASE + CONSOLE_SIZE)){
			*v++ = *q++;
			*v++ = COLOR_RED;
			if(!*q){
					while(((int)v - CONSOLE_BASE) % (SCREEN_WIDTH * 16)){
						*v++ = ' ';
						*v++ = CONSOLE_COLOR;
					}
				q = p +1;
			}
		}
		__asm__ __volatile__("hlt");
	}

	while((ch = *p++) != 0){
		disp_char(tty_table[0].p_console, ch);
	}

	return 0;
}