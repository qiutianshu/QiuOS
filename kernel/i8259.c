#include "const.h"
#include "type.h"
#include "protect.h"
#include "proto.h"

void init_8259A(){

	out_byte(INT_M_CTL,0x11);			//ICW1   
	out_byte(INT_S_CTL,0x11);			

	out_byte(INT_M_CTLMASK,IRQ0);		//ICW2
	out_byte(INT_S_CTLMASK,IRQ8);

	out_byte(INT_M_CTLMASK,0x4);		//ICW3
	out_byte(INT_S_CTLMASK,0x2);

	out_byte(INT_M_CTLMASK,0x1);		//ICW4
	out_byte(INT_S_CTLMASK,0X1);		

	out_byte(INT_M_CTLMASK,0xff);		//OCW1
	out_byte(INT_S_CTLMASK,0xff);
}