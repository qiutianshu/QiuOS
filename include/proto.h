/*
全局函数声明
*/
PUBLIC void out_byte(u16 port, u8 value);
PUBLIC u8	in_byte(u16 port);
PUBLIC void disp_str(char* info);
PUBLIC void disp_int(int input);
PUBLIC void disp_color_str(char* info,int color);
PUBLIC void init_descriptor(Descriptor* desc, u32 base, u32 limit,u16 attribute);	//初始化描述符表
PUBLIC char* atoi(char* str,int num);
PUBLIC int get_kernel_map(u32* base, u32* size);
PUBLIC void memcpy(void *Dest,void *Src,int size);
PUBLIC void init_8259A();
PUBLIC void delay();
PUBLIC void TestA();
PUBLIC void TestB();
PUBLIC void TestC();
PUBLIC void Init();
PUBLIC void memset(void* dest,char chr,int size);
PUBLIC void strcpy(void* dest, void* src);
PUBLIC int strcmp(char* str1, char* str2);
PUBLIC void clock_handler();									
PUBLIC void disable_irq(int irq);
PUBLIC void enable_irq(int irq);
PUBLIC void put_irq_handler(int irq, irq_handler handler);		//设置中断向量表
PUBLIC void milli_delay(int milli_sec);							//毫秒延时函数
PUBLIC void keyboard_handler();									//键盘中断处理
PUBLIC void keyboard_read(TTY* p_tty);							//tty任务读取键盘输入
PUBLIC void init_keyboard();
PUBLIC void init_clock();
PUBLIC void disable_int();										//关中断
PUBLIC void enable_int();										//开中断
PUBLIC void hd_handler();										//硬盘中断处理
PUBLIC void in_process(TTY* p_tty, u32 key);
PUBLIC void disp_char(CONSOLE* p_cone,char ch);
PUBLIC void set_cursor(unsigned int position);
PUBLIC void set_vga_start_addr(u32 addr);
PUBLIC void switch_console(int nr_console);						//切换控制台
PUBLIC void scroll_screen(CONSOLE* p_cone,int direction);		//翻屏
PUBLIC int print(char* fmt, ...);
PUBLIC int printl(char* fmt, ...);
PUBLIC int vsprintf(char* buf, char* fmt, char* argv);
PUBLIC int sprintf(char *buf, const char *fmt, ...);		
PUBLIC int ldt_seg_linear(PROCESS* p, int idx);
PUBLIC void* va2la(int pid, void* va);
PUBLIC void reset_msg(MESSAGE* msg);
PUBLIC int send_rec(int funtion, int sec_dest, MESSAGE* msg);	//封装了sendrec
PUBLIC void panic(char* fmt, ...);
PUBLIC void port_read(int port, void* buf, int size);			//从端口读消息
PUBLIC void port_write(int port, void* buf, int size);			//端口写
PUBLIC void info_task(int pid);									//唤醒进程
PUBLIC void untar(const char* filename);
PUBLIC void ti_shell(const char* tty);								//终端

PUBLIC void task_sys();											//系统进程
PUBLIC void task_hd();											//硬盘驱动
PUBLIC void task_fs();											//文件系统
PUBLIC void task_mm();											//内存管理
PUBLIC void task_tty();											//tty任务

PUBLIC int do_open();											//打开文件
PUBLIC int do_close();											//关闭文件
PUBLIC int do_rw();												//文件读写
PUBLIC int do_unlink();											//文件删除
PUBLIC int fs_stat();

PUBLIC int do_fork();											//创建子进程
PUBLIC void do_exit(int status);							
PUBLIC void do_exit(int status);
PUBLIC int do_exec();

PUBLIC int strip_path(char*filename, char* path, struct inode** ppinode);
PUBLIC void rw_sector(int type, int dev, int pos_in_sector, int bytes, int proc, void* buf);
PUBLIC int search_file(char* path);								//搜索文件成功返回inode
PUBLIC struct inode* create_file(char* path, int flag);
PUBLIC void put_inode(struct inode* pnode);
PUBLIC void sync_inode(struct inode* pode);
PUBLIC struct inode* get_inode(int dev, int num);
PUBLIC struct super_block* get_super_block(int dev);
PUBLIC void read_super_block(int dev);

PUBLIC int open(char* filepath, int flag);
PUBLIC int close(int fd);
PUBLIC int write(int fd, void* buf, int count);
PUBLIC int read(int fd, void* buf, int count);
PUBLIC int unlink(char* filepath);
PUBLIC int stat(char* pathname, struct status* s);

PUBLIC int fork();												//创建子进程
PUBLIC void exit(int status);									//退出进程
PUBLIC int wait(int* status);									//等待子进程返回状态码
PUBLIC int execl(const char* path ,const char* arg1,...);
PUBLIC int execv(const char* path, char* argv[]);

/*   以下是系统调用相关		*/
PUBLIC void sys_call();
PUBLIC int get_ticks();											//用户调用接口
PUBLIC int getpid();
PUBLIC int sys_get_ticks();										//中断处理例程

PUBLIC int sendrec(int function, int src_dest, MESSAGE* m);
PUBLIC int sys_sendrec(int function, int src_dest, MESSAGE* m, PROCESS* p);

PUBLIC int printx(char* buf);
PUBLIC int sys_printx(int unused1, int unused2, char* ch, PROCESS* p_proc);
