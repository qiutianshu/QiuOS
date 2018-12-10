#include "type.h"
#include "const.h"
#include "fs.h"
#include "tty.h"
#include "console.h"
#include "protect.h"
#include "proc.h"
#include "proto.h"
#include "hd.h" 
#include "global.h"

PUBLIC int strcmp(char* str1, char* str2){
	char* p1 = str1;
	char* p2 = str2;
	int ret = 0;
	while(*p1 == *p2){
		if(*p1++ == '\0' || *p2++ == '\0')
			return (*p1 - *p2);
	}
	return (*p1 - *p2);
}
