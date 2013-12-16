#define _POSIX_SOURCE 1

#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "commands.h"

int echo(char*[]);


command_pair dispatch_table[]={
	{"echo", &echo},
	{NULL,NULL}
};

int echo(argv)
char * argv[];
{
	int i =1;
	if (argv[i]) printf("%s", argv[i++]);
	while  (argv[i])
		printf(" %s", argv[i++]);

	fflush(stdout);
	return 0;
}
