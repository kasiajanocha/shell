#define _POSIX_SOURCE 1

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "config.h"
#include "commands.h"
#include "cparse.h"

int main(int, char**);

char prompt[10]=PROMPT;

char input_buf[2*BUFF_SIZE+1];
int beg;
int end;

int shell_commands(command_s command) {
	int i;
	for(i = 0; dispatch_table[i].name != NULL; i++) {
		if(strcmp(dispatch_table[i].name, command.argv[0]) == 0) {
			dispatch_table[i].fun(command.argv);
			return 1;
		}
	}
	return 0;
}

int exec_one(command_s s){
	// if(s.in_file_name != NULL) {
	// 		open(s.in_file_name, O_RDONLY);
	// }
	// mode_t mode = S_IRWXU | S_IRUSR | S_IWUSR | S_IXUSR | S_IRWXG | S_IRGRP | S_IWGRP | S_IXGRP;
	// if(s.out_file_name != NULL) {
	// 	if(s.append_mode != 0) {
	// 		open(s.out_file_name, O_WRONLY | O_CREAT | O_APPEND, mode);
	// 	} else {
	// 		open(s.out_file_name, O_WRONLY | O_CREAT | O_TRUNC, mode);
	// 	}
	// }
	if(execvp(s.argv[0], s.argv) == -1) exit(0);
}

int exec() {
	int pid;
	command_s* cmds = split_commands(input_buf+beg);
	if(cmds==NULL) {
		return 0;
	}
	if(shell_commands(cmds[0])) {
		return 1;
	}

	int pipe_sdf[2];
		
	int i;
	for(i = 0; cmds[i].argv != NULL; i++) {
		if(cmds[i].argv != NULL)  { //mamy pipe'a 
			pipe(pipe_sdf[2]); 
		}
		pid = fork();
		//printf("%d\n", pid);
		if(pid == 0) {
			exec_one(cmds[i]);
		} else if(pid > 0) {
			waitpid(pid, NULL, 0);
		}
	}
	return 1;
}

int travel_and_exec() {
	int beg_prim = beg;
	while(beg_prim < end) {
		while(beg_prim < end && input_buf[beg_prim] != '\n') {
			beg_prim++;
		}
		if (input_buf[beg_prim]=='\n')
		{
			input_buf[beg_prim] = '\0';
			exec();
			beg = beg_prim+1;
		}
	}
	return 1;
}

void rewrite() {
	int len = end - beg;
	int i;
	for(i = 0; i < len; i++) {
		input_buf[i] = input_buf[beg+i];
	}
	beg = 0;
	end = len;
}

int main(argc, argv)
int argc;
char* argv[];
{
	beg = end = 0;
	struct stat myStat;
	fstat(0, &myStat);
	int write_prompt = (S_ISCHR(myStat.st_mode));
	while(1) {
		if(write_prompt) write(1, prompt, strlen(prompt));
		size_t howmany = read(0, input_buf + end, BUFF_SIZE);
		if(howmany==0) {
			break;
		}
		end += howmany;
		if(!travel_and_exec()) break;
		if(end > BUFF_SIZE) {
			rewrite();
		}
	}
	return 0;
}