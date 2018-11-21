#include "type.h"
#include "const.h"
#include "protect.h"
#include "proc.h"
#include "global.h"

/*
	系统调用处理例程
*/
PUBLIC int sys_get_ticks(){
	return ticks;
}