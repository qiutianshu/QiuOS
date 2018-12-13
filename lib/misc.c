#include "type.h"
#include "const.h"
#include "fs.h"
#include "tty.h"
#include "console.h"
#include "protect.h"
#include "proc.h"
#include "proto.h"
#include "global.h"

/*     进程间通信
*
*参数：	动作(SEND、RECEIVE、BOTH)
*		目标进程
*		消息指针
*/
PUBLIC int send_recv(int function, int src_dest, MESSAGE* msg){
	if(function == RECEIVE)
		memset(msg, 0, sizeof(MESSAGE));				//消息清零准备接收

	if(function == RECEIVE || function == SEND)				
		return (sendrec(function, src_dest, msg));

	if(function == BOTH)
		if(!sendrec(SEND, src_dest, msg)){
	//		reset_msg(msg);
			sendrec(RECEIVE, src_dest, msg);
		}

	return 0;
}

PUBLIC void spin(char * func_name)
{
	printl("\nspinning in %s ...\n", func_name);
	while (1) {}
}

PUBLIC void assertion_failure(char* exp, char* file, char* base_file, int line){
	printl("%c  assert(%s) failed: file: %s, base_file: %s, line: %d", MAG_CH_ASSERT, exp, file, base_file, line);

	spin("assertion_failure()");	    			

	__asm__ __volatile__("ud2");
}

PUBLIC void panic(char* fmt, ...){
	int i;
	char buf[256];
	char* argv = (char*)((char*)(&fmt) + 4);
	i = vsprintf(buf, fmt, argv);
	printl("%c !! panic !! %s", MAG_CH_PANIC,buf);
	__asm__ __volatile__("ud2");
}
