#include "type.h"
#include "const.h"
#include "fs.h"
#include "tty.h"
#include "console.h"
#include "protect.h"
#include "proc.h"
#include "proto.h"
#include "global.h"

PUBLIC void ti_shell(const char* tty){
	int fd_stdin = open(tty, O_RW);
	assert(fd_stdin == 0);
	int fd_stdout = open(tty, O_RW);
	assert(fd_stdout == 1);
	char rdbuf[80];

	sprintf(proc_table[getpid()].p_name, "ti_shell@%s", tty);

	while(1){
		printf("tishell@tty%d:\\$", tty[8] - '0');
		int r = read(0, rdbuf, 70);
		rdbuf[r] = 0;
		int argc = 0;
		char* argv[0x400];
		char* p = rdbuf;
		char* s;
		int word = 0;
		char ch;
		do{
			ch = *p;
			if(*p != ' ' && *p != 0 && !word){
				s = p;
				word = 1;
			}
			if((*p == ' ' || *p == 0) && word){
				word = 0;
				argv[argc++] = s;
				*p = 0;
			}
			p++;
		}while(ch);
		argv[argc] = 0;
		int fd = open(argv[0], O_RW);
		if(fd == -1){
			if(rdbuf[0])
				printf("ti_shell: %s : cannot find command\n", argv[0]);
		}
		else{
			close(fd);
			int pid = fork();
			if(pid != 0){
				int s;
				wait(&s);
			}
			else{
				execv(argv[0],argv);
			}
		}
	}
	close(fd_stdin);
	close(fd_stdout);
}