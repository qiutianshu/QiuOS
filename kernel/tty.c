#include "type.h"
#include "const.h"
#include "keyboard.h"
#include "tty.h"
#include "console.h"
#include "proto.h"
#include "protect.h"
#include "proc.h"
#include "global.h"

PRIVATE void init_tty(TTY* p_tty){
	p_tty->count = 0;
	p_tty->head = p_tty->tail = p_tty->buf;

	int nr_tty = p_tty - tty_table;
	p_tty->p_console = console_table + nr_tty;

	int console_mem_size = (CONSOLE_SIZE >> 1) / NR_CONSOLES;		//每个控制台分配的显存大小
	p_tty->p_console->origin_addr = nr_tty * console_mem_size;		//当前现存位置
	p_tty->p_console->mem_limit = console_mem_size;
	p_tty->p_console->current_start_addr = nr_tty * console_mem_size;
	p_tty->p_console->cursor = p_tty->p_console->origin_addr;		//当前光标位置定为起始显示位置

	if(nr_tty == 0){
		p_tty->p_console->cursor = disp_pos / 2;					//第一个控制台的光标位置为
		//disp_pos = 0;
	}
	else{
		disp_char(p_tty->p_console,nr_tty + '0');
		disp_char(p_tty->p_console,'#');
	}
	set_cursor(p_tty->p_console->cursor);

}

PUBLIC int is_current_console(CONSOLE* p_cone){
	return (p_cone == console_table+current_console);
}

PRIVATE void tty_read(TTY* p_tty){
	if(is_current_console(p_tty->p_console))
		keyboard_read(p_tty);
}

PRIVATE void tty_write(TTY* p_tty){
	if(p_tty->count){
		char ch = *(p_tty->tail);
		p_tty->tail++;
		if(p_tty->tail == p_tty->buf + TTY_BUF_SIZE)
			p_tty->tail = p_tty->buf;

		p_tty->count--;

		disp_char(p_tty->p_console,ch);
	}
}

PUBLIC void task_tty(){
	TTY* p_tty;	
	for(p_tty=tty_table; p_tty<tty_table+NR_CONSOLES; p_tty++)			//初始化tty_table
		init_tty(p_tty);

	switch_console(0);			//初始化为0号控制台

	while(1){
		for(p_tty=tty_table; p_tty<tty_table+NR_CONSOLES; p_tty++){
			tty_read(p_tty);
			tty_write(p_tty);
		}
	}
}


PUBLIC void in_process(TTY* p_tty,u32 key){
	char output[2] = {'\0','\0'};
	if(!(key & 0x100)){						//显示可打印字符
		if(p_tty->count < TTY_BUF_SIZE){
			*(p_tty->head) = key;			//写入tty缓冲区
			p_tty->head++;
			if(p_tty->head == p_tty->buf+TTY_BUF_SIZE)
				p_tty->head = p_tty->buf;
			p_tty->count++;
		}
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
	u8* pos = (u8*)(CONSOLE_BASE + p_cone->cursor * 2);	//指向当前控制台显示地址
	*pos++ = ch;										//显示一个空格
	*pos++ = CONSOLE_COLOR;								//字体颜色 增加一个空格
	p_cone->cursor++;
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