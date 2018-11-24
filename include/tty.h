#define TTY_BUF_SIZE	256			//tty缓冲区大小

struct s_console;

typedef struct s_tty{
	u32 buf[TTY_BUF_SIZE];			//tty缓冲区
	u32* head;					//缓冲区下一个空闲位置
	u32* tail;					//下一个读取位置
	int count;

	struct s_console* p_console;	//对应的控制台
}TTY;