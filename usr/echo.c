#include "stdio.h"

int main(int argc, char const *argv[])
{
	int i;
	for(i = 1; i < argc; i++)
		printf("%s ", argv[i]);
	printf("\n");
	return 1;
}