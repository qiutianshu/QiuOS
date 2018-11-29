#include "type.h"
#include "const.h"
#include "tty.h"
#include "console.h"
#include "protect.h"
#include "proc.h"
#include "proto.h"
#include "global.h"

PUBLIC void spin(char * func_name)
{
	printl("\nspinning in %s ...\n", func_name);
	while (1) {}
}

PUBLIC void assertion_failure(char* exp, char* file, char* base_file, int line){
	printl("%c  assert(%s) failed: file: %s, base_file: %s, line: %d", MAG_CH_ASSERT, exp, file, base_file, line);

	spin("assertion_failure()");	    			

	__asm__ __volatile__("ud2");
}

PUBLIC void panic(char* fmt, ...){
	int i;
	char buf[256];
	char* argv = (char*)((char*)(&fmt) + 4);
	i = vsprintf(buf, fmt, argv);
	printl("%c !! panic !! %s", MAG_CH_PANIC,buf);
	__asm__ __volatile__("ud2");
}
