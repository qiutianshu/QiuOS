#include "stdio.h"
#include "system.h"


int main(int argc, char const *argv[])
{
	char name[32];
	int i;
	MESSAGE msg;
	printf("pid          name\n\n");
	for(i = 0; i < 21; i++){
		msg.type = FIND_PROCS;
		msg.BUF = name;
		msg.PID = i;								//要查询的进程号
		send_recv(BOTH, TASK_SYS, &msg);
		if(msg.FLAGS){
			printf("%d          ", i);
			printf("%s\n", name);
		}
	}
	return 0;
}