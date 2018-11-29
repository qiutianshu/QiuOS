/*
全局函数声明
*/
PUBLIC void out_byte(u16 port, u8 value);
PUBLIC u8	in_byte(u16 port);
PUBLIC void disp_str(char* info);
PUBLIC void disp_int(int input);
PUBLIC void disp_color_str(char* info,int color);
PUBLIC char* atoi(char* str,int num);
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
PUBLIC int printf(char* fmt, ...);
PUBLIC int vsprintf(char* buf, char* fmt, char* argv);			
PUBLIC int ldt_seg_linear(PROCESS* p, int idx);
PUBLIC void* va2la(int pid, void* va);
PUBLIC void reset_msg(MESSAGE* msg);
PUBLIC int send_rec(int funtion, int sec_dest, MESSAGE* msg);	//封装了sendrec
PUBLIC void panic(char* fmt, ...);

PUBLIC void task_sys();											//系统进程

/*   以下是系统调用相关		*/
PUBLIC void sys_call();
PUBLIC int get_ticks();											//用户调用接口
PUBLIC int sys_get_ticks();										//中断处理例程

PUBLIC void write(char* buf, int len);
PUBLIC int sys_write(char* buf, int len, PROCESS* p_proc);

PUBLIC int sendrec(int function, int src_dest, MESSAGE* m);
PUBLIC int sys_sendrec(int function, int src_dest, MESSAGE* m, PROCESS* p);

PUBLIC int printx(char* buf);
PUBLIC int sys_printx(int unused1, int unused2, char* ch, PROCESS* p_proc);
