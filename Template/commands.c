#define _POSIX_SOURCE 1

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>

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

//parses string to int
int parse(char *s, int* error) {
	int result = 0;
	*error = 0;
	int neg = 1;
	int ten_pow = 1;
	int len = strlen(s);
	if(s[0] == '-') {
		neg = -1;
	}
	int i;
	for(i = len - 1; i >=0; i--) {
		if(s[i] >='0' && s[i] <='9') {
			result += s[i] - '0';
		} else {
			if(i==0 && s[i] == '-') continue;
			*error = 1;
		} 
		ten_pow *= 10;
	}
	return result;
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
	{"ls", &my_ls},
	{"echo", &echo},
	{NULL,NULL}
};

int my_exit(argv)
char * argv[];
{
	my_printf("exit: ");
	my_printf(argv[1]);
	int err;
	int arg = parse(argv[1], &err);
	if(err != 0) {
		my_printf("invalid argument: not an int!\n");
		exit(255);
	} else {
		exit(arg);
	}
}


int my_cd(argv)
char * argv[];
{
	if(argv[1] != NULL) {
		chdir(argv[1]);
	} else {
		chdir(getenv("HOME"));
	}
}

int my_kill(argv)
char * argv[];
{
	if(!argv[2]) {
		int err;
		kill(parse(argv[1], &err), 0);
	} else {
		int err1, err2;
		kill(parse(argv[2], &err1), parse(argv[1], &err2));
	}
	fflush(stdout);
}


int my_lenv(argv)
char * argv[];
{
	//wypisz wszystkie rzeczy z environ
	char **cur;
	for(cur = environ; *cur != NULL; cur++) {
		my_printf(*cur);
		my_printf("\n");
	}
	fflush(stdout);
}

int my_ls(argv)
char * argv[];
{
	char dir[BUFF_SIZE];
	getcwd(dir, BUFF_SIZE);
	DIR *my_dir = opendir(dir);
	if(my_dir != NULL) {
		struct dirent * cur_f;
		cur_f = readdir(my_dir);
		while(cur_f != NULL) {
			if(cur_f -> d_name[0] != '.') {
				my_printf(cur_f -> d_name);
				my_printf("\n");
			}
		}		
	}
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
	fflush(stdout);
	return 0;
}
