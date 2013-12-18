#define _POSIX_SOURCE 1

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

#include "config.h"
#include "commands.h"
#include "cparse.h"

int main(int, char**);

char prompt[10]=PROMPT;

char input_buf[2*BUFF_SIZE+1];
int beg;
int end;

void output_forwarding(command_s cmd) {
	if(cmd.out_file_name == NULL) return;
	mode_t mode = S_IRWXU | S_IRUSR | S_IWUSR | S_IXUSR | S_IRWXG | S_IRGRP | S_IWGRP | S_IXGRP;
	
	close(STDOUT_FILENO);

	if(cmd.out_file_name != NULL) {
		if(cmd.append_mode != 0) {
			open(cmd.out_file_name, O_WRONLY | O_CREAT | O_APPEND, mode);
		} else {
			open(cmd.out_file_name, O_WRONLY | O_CREAT | O_TRUNC, mode);
		}
	}
}

int shell_commands(command_s cmd) {
	int i;
	for(i = 0; dispatch_table[i].name != NULL; i++) {
		if(strcmp(dispatch_table[i].name, cmd.argv[0]) == 0) {
			int lenv = 0;
			int old_stdout;
			if(strcmp(dispatch_table[i].name, "lenv") == 0) {
				lenv = 1;
			}
			if(lenv && cmd.out_file_name != NULL) {
				old_stdout = dup(STDOUT_FILENO);
				output_forwarding(cmd);
			}
			dispatch_table[i].fun(cmd.argv);
			if(lenv && cmd.out_file_name) {
				dup2(old_stdout, STDOUT_FILENO);
				close(old_stdout);
			}
			return 1;
		}
	}
	return 0;
}

int exec_one(command_s cmd){
	if(cmd.in_file_name != NULL) {
		open(cmd.in_file_name, O_RDONLY);
	}
	output_forwarding(cmd);
	if(execvp(cmd.argv[0], cmd.argv) == -1) exit(0);
}

int exec() {
	int pid;
	command_s* cmds = split_commands(input_buf+beg);
	if(cmds[0].argv==NULL) {
		return 0;
	}
	if(shell_commands(cmds[0])) {
		return 1;
	}

	int pipe_sdf[2];
	int there_was_pipe = 0;
	int new_input = -1;

	int i;
	for(i = 0; cmds[i].argv != NULL; i++) {
		if(cmds[i+1].argv != NULL)  { //mamy pipe'a 
			pipe(pipe_sdf);
			there_was_pipe = 1;
		}
		pid = fork();
		if(pid == 0) {
			//trzeba przekierowac wyjscie
			if(there_was_pipe) {
				dup2(pipe_sdf[1], STDOUT_FILENO);
				close(pipe_sdf[0]);
				close(pipe_sdf[1]);
			}
			//przek wyjscia
			if(new_input != -1) {
				dup2(new_input, STDIN_FILENO);
				close(new_input);
				close(pipe_sdf[1]);
			}
			exec_one(cmds[i]);
		} else if(pid > 0) {
			waitpid(pid, NULL, 0);
			if(there_was_pipe) {
				close(pipe_sdf[0]);
				close(pipe_sdf[1]);
			}
		}
		if(there_was_pipe) {
			new_input = pipe_sdf[1];
		}
		there_was_pipe = 0;
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