typedef struct s_console{
	unsigned int  current_start_addr;		//当前显示的位置
	unsigned int origin_addr;				//当前控制台对应的显存的位置
	unsigned int mem_limit;					//当前控制台占用显存大小
	unsigned int cursor;					//当前光标位置
}CONSOLE;

#define SCR_UP			1
#define SCR_DOWN		-1
#define SCREEN_WIDTH	80