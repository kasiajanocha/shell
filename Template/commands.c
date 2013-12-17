#define _POSIX_SOURCE 1

#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "commands.h"

int echo(char*[]);

extern char **environ;
extern int *errno;

command_pair dispatch_table[]={
	{"echo", &echo},
	{NULL,NULL}
};

int m_exit(argv)
char * argv[];
{
}


int m_cd(argv)
char * argv[];
{
}

int m_kill(argv)
char * argv[];
{

}


int m_lenv(argv)
char * argv[];
{
	//wypisz wszystkie rzeczy z environ
}

int m_ls(argv)
char * argv[];
{
	char folder_name[1024];
	getcwd(folder_name, 1024);
	DIR *my_dir = opendir(folder_name);
	if(my_dir != NULL) {
		struct dirent * cur_f;
		cur_f = readdir(my_dir);
		while(cur_f != NULL) {
			if(cur_f -> d_name[0] != '.') {
				write_string(cur_f -> d_name);
				write_string("\n");
			}
		}		
	}
}

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
