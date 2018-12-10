#define TTY_BUF_SIZE	256			//tty缓冲区大小

struct s_console;

typedef struct s_tty{
	u32 buf[TTY_BUF_SIZE];			//tty缓冲区
	u32* head;					//缓冲区下一个空闲位置
	u32* tail;					//下一个读取位置
	int count;

	int tty_caller;				//向tty发送消息的进程
	int tty_procnr;				//向tty请求数据的进程
	void* req_buf;				//进程p缓冲区
	int tty_left_cnt;			//剩余字节数
	int tty_trans_cnt;			//已向进程p传送的字节数

	struct s_console* p_console;	//对应的控制台
}TTY; 