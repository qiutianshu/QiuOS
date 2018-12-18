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

PUBLIC void set_vga_start_addr(u32 addr){
	disable_int();
	out_byte(CRT_CTRL_REG,START_ADDR_H);
	out_byte(CRT_DATA_REG,(addr >> 8) & 0xff);
	out_byte(CRT_CTRL_REG,START_ADDR_L);
	out_byte(CRT_DATA_REG,addr & 0xff);
	enable_int();
}

PUBLIC void switch_console(int nr_console){						//切换控制台
	if(nr_console < 0 || nr_console >=NR_CONSOLES)
		return;
	current_console = nr_console;
	set_vga_start_addr(console_table[nr_console].current_start_addr);
	set_cursor(console_table[nr_console].cursor);
}

PUBLIC void scroll_screen(CONSOLE* p_cone,int direction){
	if(direction == SCR_UP){									//往前翻
		if(p_cone->current_start_addr > p_cone->origin_addr)
			p_cone->current_start_addr -= SCREEN_WIDTH;
	}
	else if(direction == SCR_DOWN){								//往后翻
		if((p_cone->current_start_addr + 80 * 25) < (p_cone->origin_addr + p_cone->mem_limit))
			p_cone->current_start_addr += SCREEN_WIDTH;
	}								
	set_vga_start_addr(p_cone->current_start_addr);
	set_cursor(p_cone->cursor);
}