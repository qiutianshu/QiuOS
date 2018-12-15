#include "stdio.h"

int main(int argc, char const *argv[])
{
	int i;
	for(i = 1; i < argc; i++)
		printl("%s ", argv[i]);
	printl("\n");
	return 1;
}