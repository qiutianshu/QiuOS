#include "type.h"
#include "protect.h"
#include "proc.h"
#include "const.h"
#include "proto.h"
#include "global.h"


PUBLIC void clock_handler(){
	ticks++;
  	p_proc_ready->ticks--;

    if(k_reenter != 0)
    	return;

    if(p_proc_ready->ticks > 0)
    	return;

    schedule();
}

/*	毫秒延时函数	*/
PUBLIC void milli_delay(int milli_sec){			
	int t = get_ticks();
	while(((get_ticks()-t)*1000/HZ)<milli_sec){}
}

