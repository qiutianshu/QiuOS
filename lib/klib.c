#include "type.h"
#include "const.h"
#include "proto.h"


PUBLIC char* atoi(char* str,int num){
	char *p = str;
	int i = 0;
	char ch;

	*p++ = '0';
	*p++ = 'x';

	for(i=28;i>=0;i-=4){
		ch = (num>>i) & 0xf;
		if(ch<0xa)
			ch += '0';
		else
			ch = ch-0xa+'a';

		*p++ = ch; 
	}
	*p = 0;
	return str;

}

PUBLIC void disp_int(int input){
	char output[16];
	atoi(output,input);
	disp_str(output);
}

PUBLIC void delay(int time){
	int i,j,k;
	for(k = 0; k < time; k++){
		for(i = 0; i <10; i++){
			for(j = 0; j < 10000; j++){}
		}
	}
}

PUBLIC void disable_irq(int irq){
	if(irq<8)
		out_byte(INT_M_CTLMASK, in_byte(INT_M_CTLMASK)|(1<<irq));
	else
		out_byte(INT_S_CTLMASK, in_byte(INT_S_CTLMASK)|(1<<(irq % 8)));
}

PUBLIC void enable_irq(int irq){
	if(irq<8)
		out_byte(INT_M_CTLMASK,in_byte(INT_M_CTLMASK)&(0xff-(1<<irq)));
	else
		out_byte(INT_S_CTLMASK,in_byte(INT_S_CTLMASK)&(0xff-(1<<(irq%8))));
}