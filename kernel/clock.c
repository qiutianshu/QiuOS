#include "type.h"
#include "protect.h"
#include "fs.h"
#include "proc.h"
#include "const.h"
#include "tty.h"
#include "console.h"
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


PUBLIC void init_clock(){
	/*初始化8253 PIT*/
	out_byte(TIMER_MODE,RATE_GENERATOR);		//设置模式控制寄存器为时钟中断模式
	out_byte(TIMER0,(u8)(TIMER_FREQ/HZ));		//写入低8位
	out_byte(TIMER0,(u8)((TIMER_FREQ/HZ) >> 8));//写入高8位
	put_irq_handler(CLOCK_IRQ,clock_handler);
	enable_irq(CLOCK_IRQ);
}
