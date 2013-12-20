#define _POSIX_SOURCE 1

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <signal.h>

#include "commands.h"
#include "cparse.h"
#include "config.h"

int echo(char*[]);
int my_exit(char*[]);
int my_cd(char*[]);
int my_kill(char*[]);
int my_lenv(char*[]);
int my_ls(char*[]);


extern char **environ;
extern int *errno;

void to_string(int arg, char * c) {
	int argcp;
	int digits;
	int i;
	argcp = arg;
	digits = 0;
	if(argcp == 0) {
		digits = 1;
	} else while(argcp) {
		argcp /= 10;
		digits++;
	}
	argcp = arg;
	c[digits] = '\0';
	for(i = digits-1; i >=0; i--) {
		c[i] = argcp%10 + '0';
		argcp /= 10;
	}
}

void my_printf(char * arg) {
	char *s = arg;
	int to_pr = strlen(s);
	int pr = 0; 
	while( pr < to_pr) {
		pr += write(1, s, to_pr-pr);
	}
}	

command_pair dispatch_table[]={
	{"exit", &my_exit},
	{"cd", &my_cd},
	{"kill", &my_kill},
	{"lenv", &my_lenv},
	{"lls", &my_ls},
	{"echo", &echo},
	{NULL,NULL}
};

int my_exit(argv)
char * argv[];
{
	if(argv[1] == NULL) {
		my_printf("exit.\n");
		exit(EXIT_SUCCESS);	
	}
	my_printf("exit: ");
	my_printf(argv[1]);
	my_printf("\n");
	exit(atoi(argv[1]));
	return 1;
}

int my_cd(argv)
char * argv[];
{
	if(argv[1] != NULL) {
		chdir(argv[1]);
	} else {
		chdir(getenv("HOME"));
	}
	return 1;
}

int my_kill(argv)
char * argv[];
{
	if(!argv[1]) {
		my_printf("kill: usage: kill [-s sigspec | -n signum | -sigspec] pid | jobspec ... or kill -l [sigspec]\n");
	} else if(!argv[2]) {
			kill(atoi(argv[1]), 15);
	} else {
		kill(atoi(argv[2]), (-1)*atoi(argv[1]));
	}
	fflush(stdout);
	return 1;
}


int my_lenv(argv)
char * argv[];
{
	/*wypisz wszystkie rzeczy z environ*/
	char **cur;
	for(cur = environ; *cur != NULL; cur++) {
		my_printf(*cur);
		my_printf("\n");
	}
	fflush(stdout);
	return 1;
}

int my_ls(argv)
char * argv[];
{
	DIR *my_dir;
	char dir[BUFF_SIZE];
	getcwd(dir, BUFF_SIZE);
	my_dir = opendir(dir);
	if(my_dir != NULL) {
		struct dirent * cur_f;
		cur_f = readdir(my_dir);
		while(cur_f != NULL) {
			if(cur_f -> d_name[0] != '.') {
				my_printf(cur_f -> d_name);
				my_printf("\n");
			}
			cur_f = readdir(my_dir);
		}		
	}
	return 1;
}

int echo(argv)
char * argv[];
{
	int i = 1;
	if (argv[i]) my_printf(argv[i++]);
	while  (argv[i]) {
		my_printf(" ");
		my_printf(argv[i++]);
	}
	my_printf("\n");	
	fflush(stdout);
	return 0;
}