#include "type.h"
#include "protect.h"
#include "proc.h"
#include "const.h"
#include "proto.h"
#include "global.h"


PUBLIC void clock_handler(){
	ticks++;
    p_proc_ready++;
    if(k_reenter != 0)
    	disp_str("#");
    if(p_proc_ready >= proc_table+NR_TASKS)
        p_proc_ready = proc_table;
}

/*	毫秒延时函数	*/
PUBLIC void milli_delay(int milli_sec){			
	int t = get_ticks();
	while(((get_ticks()-t)*1000/HZ)<milli_sec){}
}

