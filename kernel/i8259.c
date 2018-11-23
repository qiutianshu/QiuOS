#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "proc.h"
#include "global.h"

void init_8259A(){

	out_byte(INT_M_CTL,0x11);			//ICW1   
	out_byte(INT_S_CTL,0x11);			

	out_byte(INT_M_CTLMASK,IRQ0);		//ICW2
	out_byte(INT_S_CTLMASK,IRQ8);

	out_byte(INT_M_CTLMASK,0x4);		//ICW3
	out_byte(INT_S_CTLMASK,0x2);

	out_byte(INT_M_CTLMASK,0x1);		//ICW4
	out_byte(INT_S_CTLMASK,0X1);		

	out_byte(INT_M_CTLMASK,0xff);		//屏蔽所有中断
	out_byte(INT_S_CTLMASK,0xff);

	int i;
	for(i=0; i<NR_IRQ; i++)
		irq_table[i] = (void*)0;
}

PUBLIC void put_irq_handler(int irq, irq_handler handler){
	disable_irq(irq);
	irq_table[irq] = handler;
}

