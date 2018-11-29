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
	{task_sys,STACK_SIZE_SYS_TASK,"task_sys"}
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