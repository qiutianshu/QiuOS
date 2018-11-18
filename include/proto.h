/*
全局函数声明
*/
PUBLIC void out_byte(u16 port, u8 value);
PUBLIC void disp_str(char* info);
PUBLIC void disp_int(int input);
PUBLIC void disp_color_str(char* info,int color);
PUBLIC void memcpy(void *Dest,void *Src,int size);
PUBLIC void init_8259A();
PUBLIC void delay();
PUBLIC void TestA();
PUBLIC void memset(void* dest,char chr,int size);
