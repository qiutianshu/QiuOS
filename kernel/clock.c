#include "type.h"
#include "protect.h"
#include "proc.h"
#include "const.h"
#include "proto.h"
#include "global.h"


PUBLIC void clock_handler(){
    p_proc_ready++;
    if(p_proc_ready >= proc_table+NR_TASKS)
        p_proc_ready = proc_table;
}

