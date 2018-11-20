#define GLOBAL_VARIABLES_HERE 
#include "type.h"
#include "const.h"
#include "protect.h"
#include "proc.h"
#include "proto.h"
#include "global.h"

PUBLIC TASK task_table[NR_TASKS]={
    {TestA,STACK_SIZE_TESTA,"TestA"},
    {TestB,STACK_SIZE_TESTB,"TestB"},
    {TestC,STACK_SIZE_TESTC,"TestC"}
};

PUBLIC irq_handler irq_table[NR_IRQ];