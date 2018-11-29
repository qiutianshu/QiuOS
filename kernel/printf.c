#include "type.h"
#include "const.h"
#include "tty.h"
#include "console.h"
#include "protect.h"
#include "proc.h"
#include "proto.h"
#include "global.h"

PUBLIC int  printf(char* fmt, ...){
	int i;
	char buf[256];
	char* argv = (char*)((char*)(&fmt) + 4);			//指向第一个参数
	i = vsprintf(buf, fmt, argv);						//格式化字符串
	buf[i] = 0;
	printx(buf);
	return i;
}

PUBLIC int vsprintf(char* buf, char* fmt, char* argv){
	char* p;
	char tmp[256];
	char* q = tmp;
	char* p_next_arg = argv;
	int tmp_int;
	int length = 0;

	for(p = buf; *fmt; fmt++){
		if(*fmt != '%'){
			*p++ = *fmt;
			continue;
		}

		fmt++;						//跳过%
		switch(*fmt){
			case 'x':
				atoi(tmp, *(int*)(p_next_arg));
				strcpy(p,tmp);
				p_next_arg += 4;
				p += strlen(tmp);
				break;
			case 'd':
				tmp_int = *((int*)p_next_arg);
				while(tmp_int){
					*q++ = '0' + tmp_int % 10;
					tmp_int = (tmp_int - tmp_int % 10) / 10;
					length++;
				}
				while(length)
					*p++ = tmp[--length];
				
				q = tmp;
				p_next_arg += 4;
				break;
			case 'c':
				*p = *(char*)p_next_arg;
				p++;
				p_next_arg += 4;
				break;
			case 's':
				strcpy(p, (*(char**)p_next_arg));
				p += strlen(*((char**)p_next_arg));
				p_next_arg += 4;
				break;
			default:
				break;
		}
	}
	return (p - buf);
}
