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

PUBLIC char* atoi(char* str,int num){
	char *p = str;
	int i = 0;
	char ch;

	*p++ = '0';
	*p++ = 'x';

	for(i=28;i>=0;i-=4){
		ch = (num>>i) & 0xf;
		if(ch<0xa)
			ch += '0';
		else
			ch = ch-0xa+'a';

		*p++ = ch; 
	}
	*p = 0;
	return str;

}

PUBLIC int strlen(char* str){
	int i = 0;
	char* p = str;
	while(*p++)	i++;

	return i;
}