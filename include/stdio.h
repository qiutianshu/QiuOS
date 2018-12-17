#ifndef _QIUOS_STDIO_H_
#define _QIUOS_STDIO_H_

#define UNUSED						-10
#define TASK_TTY					0		//TTY任务
#define TASK_SYS					1		//系统进程号
#define TASK_HD						2		//硬盘驱动进程号
#define TASK_FS						3		//文件系统
#define TASK_MM 					4		//内存管理
#define PROC_INIT					5		//Init进程号

#define PUBLIC 
#define PRIVATE static
#define EXTERN	extern 

void assertion_failure(char* exp,char* file,char* base_file,int line);
#define assert(exp)		if(exp);\
else assertion_failure(#exp, __FILE__, __BASE_FILE__, __LINE__)
		

#define min(a,b)		(a < b) ? a : b
#define printf 			print


PUBLIC int printf(char* fmt, ...);

PUBLIC char* atoi(char* str,int num);

PUBLIC void memcpy(void *Dest,void *Src,int size);
PUBLIC void memset(void* dest,char chr,int size);
PUBLIC void strcpy(void* dest, void* src);
PUBLIC int strcmp(char* str1, char* str2);

PUBLIC int print(char* fmt, ...);
PUBLIC int printl(char* fmt, ...);
PUBLIC int vsprintf(char* buf, char* fmt, char* argv);
PUBLIC int sprintf(char *buf, const char *fmt, ...);
PUBLIC int printx(char* buf);

PUBLIC void panic(char* fmt, ...);

PUBLIC int open(char* filepath, int flag);
PUBLIC int close(int fd);
PUBLIC int write(int fd, void* buf, int count);
PUBLIC int read(int fd, void* buf, int count);
PUBLIC int unlink(char* filepath);

PUBLIC int fork();												//创建子进程
PUBLIC void exit(int status);									//退出进程
PUBLIC int wait(int* status);									//等待子进程返回状态码


#endif