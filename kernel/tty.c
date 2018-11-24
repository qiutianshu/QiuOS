#include "type.h"
#include "const.h"
#include "proto.h"
#include "protect.h"
#include "proc.h"
#include "global.h"

PUBLIC void task_tty(){
	while(1){
		keyboard_read();
		disable_int();
		out_byte(CRT_CTRL_REG,CURSOR_H);
		out_byte(CRT_DATA_REG,((disp_pos/2) >> 8) & 0xff);
		out_byte(CRT_CTRL_REG,CURSOR_L);
		out_byte(CRT_DATA_REG,(disp_pos/2) & 0xff);  
		enable_int();
	}
}