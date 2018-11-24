/*
全局函数声明
*/
PUBLIC void out_byte(u16 port, u8 value);
PUBLIC u8	in_byte(u16 port);
PUBLIC void disp_str(char* info);
PUBLIC void disp_int(int input);
PUBLIC void disp_color_str(char* info,int color);
PUBLIC void memcpy(void *Dest,void *Src,int size);
PUBLIC void init_8259A();
PUBLIC void delay();
PUBLIC void TestA();
PUBLIC void TestB();
PUBLIC void TestC();
PUBLIC void task_tty();
PUBLIC void memset(void* dest,char chr,int size);
PUBLIC void strcpy(void* dest, void* src);
PUBLIC void clock_handler();
PUBLIC void disable_irq(int irq);
PUBLIC void enable_irq(int irq);
PUBLIC void put_irq_handler(int irq, irq_handler handler);
PUBLIC void milli_delay(int milli_sec);							//毫秒延时函数
PUBLIC void keyboard_handler();									//键盘中断处理
PUBLIC void keyboard_read(TTY* p_tty);							//tty任务读取键盘输入
PUBLIC void init_keyboard();
PUBLIC void init_clock();
PUBLIC void disable_int();										//关中断
PUBLIC void enable_int();										//开中断
PUBLIC void in_process(TTY* p_tty, u32 key);
PUBLIC void disp_char(CONSOLE* p_cone,char ch);
PUBLIC void set_cursor(unsigned int position);
PUBLIC void switch_console(int nr_console);						//切换控制台
PUBLIC void scroll_screen(CONSOLE* p_cone,int direction);		//翻屏

/*   以下是系统调用相关		*/
PUBLIC void sys_call();
PUBLIC int get_ticks();											//用户调用接口
PUBLIC int sys_get_ticks();										//中断处理例程
