#define GLOBAL_VARIABLES_HERE 
#include "type.h"
#include "const.h"
#include "tty.h"
#include "console.h"
#include "protect.h"
#include "proc.h"
#include "proto.h"

#include "global.h"

PUBLIC TASK task_table[NR_TASKS] = {
	{task_tty,STACK_SIZE_TTY,"tty"},
	{task_sys,STACK_SIZE_SYS_TASK,"task_sys"},
	{task_hd,STACK_SIZE_HD,"task_hd"},
	{task_fs,STACK_SIZE_FS,"task_fs"}
};

PUBLIC TASK user_proc_table[NR_PROCS] = {
	{TestA,STACK_SIZE_TESTA,"TestA"},
    {TestB,STACK_SIZE_TESTB,"TestB"},
    {TestC,STACK_SIZE_TESTC,"TestC"}
};

PUBLIC irq_handler irq_table[NR_IRQ];
PUBLIC system_call sys_call_table[NR_SYS_CALL] = {sys_get_ticks, sys_write, sys_sendrec, sys_printx};

PUBLIC TTY tty_table[NR_CONSOLES];
PUBLIC CONSOLE console_table[NR_CONSOLES];
PUBLIC struct dev_drv_map dd[] = {					/*顺序必须与pid一致*/
	{TASK_TTY},								//TTY任务
	{UNUSED},								//预留
	{TASK_HD}								//硬盘驱动
};

/*FS缓冲区  6M-7M   */
PUBLIC u8*	fsbuf			=	(u8*)0x600000;
PUBLIC const int FSBUF_SIZE	= 0x100000;