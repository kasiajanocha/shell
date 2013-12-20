#define _POSIX_SOURCE 1

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>

#include "config.h"
#include "commands.h"
#include "cparse.h"

int main(int, char**);

char prompt[10]=PROMPT;
char input_buf[2*BUFF_SIZE+1];
int beg;
int end;

/* pidy dzieci nie w tle */
int chld_pids[MAX_COMMANDS+2];
int chld_pids_size;
int chld_active;

/* ukonczone dzieci z backgroundu do wypisania */
int ended_pid[MAX_COMMANDS+2];
int ended_sig[MAX_COMMANDS+2];
int ended_killed[MAX_COMMANDS+2];
int ended_processes;


struct sigaction default_sigint_action;
struct sigaction default_sigchld_action;

sigset_t wait_for_us;
sigset_t block_sig;

void write_ended() {
	int i;
	char number[10];

	sigprocmask(SIG_BLOCK,&block_sig,NULL);
	for(i = 0; i < ended_processes; i++) {
		my_printf("Process ");
		to_string(ended_pid[i], number);
		my_printf(number);
		if(ended_killed[i] >= 0) {
			my_printf(" killed by signal ");
			to_string(ended_killed[i], number);
			my_printf(number);
			ended_killed[i] = -1;
		} else {
			my_printf(" ended with status ");
			to_string(ended_sig[i], number);
			my_printf(number);
		}
		my_printf(".\n");
	}
	ended_processes = 0;
	sigprocmask(SIG_UNBLOCK,&block_sig,NULL);
}

void save_process(int pid, int status) {
	ended_pid[ended_processes] = pid;
	ended_sig[ended_processes] = status;
	if(WIFSIGNALED(status)) ended_killed[ended_processes] = WTERMSIG(status);
	else ended_killed[ended_processes] = -1;
	ended_processes++;
}

/* handlery do SIGINT i SIGCHLD */

void handle_sigint(int s) {
	my_printf("\n");
}

void handle_sigchld(int s) {
	int i;
	int pid;
	int bcg;
	int status;
	/* pobieram child process id */
	while((pid = waitpid(-1, &status, WNOHANG)) > 0) {
		bcg = 1;
		fflush(stdout);
		for(i = 0; i < chld_pids_size; i++) {
			if(chld_pids[i] == pid) {
				chld_active--;
				bcg = 0;
				break;
			}
		}

		/* zapamietujemy do wypisania ukonczone dziecko z backgroundu */
		if(bcg && ended_processes < MAX_COMMANDS) {
			save_process(pid,status);
		}
	}
}

void handle_handlers() {
	struct sigaction sigint_action;
	struct sigaction sigchld_action;

	sigint_action.sa_handler = handle_sigint;
	sigfillset(&sigint_action.sa_mask);
	sigaction(SIGINT, &sigint_action, &default_sigint_action);

	sigchld_action.sa_handler = handle_sigchld;
	sigfillset(&sigchld_action.sa_mask);
	sigaction(SIGCHLD, &sigchld_action, &default_sigchld_action);
}

/* przekierowanie wyjscia */
void output_forwarding(command_s cmd) {
	mode_t mode;
	if(cmd.out_file_name == NULL) return;
	mode = S_IRWXU | S_IRUSR | S_IWUSR | S_IXUSR | S_IRWXG | S_IRGRP | S_IWGRP | S_IXGRP;
	
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
	int old_stdout;
	int lenv;

	for(i = 0; dispatch_table[i].name != NULL; i++) {
		if(strcmp(dispatch_table[i].name, cmd.argv[0]) == 0) {
			lenv = 0;
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

void exec_one(command_s cmd){
	if(cmd.in_file_name != NULL) {
		close(STDIN_FILENO);
		open(cmd.in_file_name, O_RDONLY);
	}
	output_forwarding(cmd);
	if(execvp(cmd.argv[0], cmd.argv) == -1) exit(EXIT_FAILURE);
}

/*exec dla pojedynczej linii*/

int exec() {
	int pid;
	int pipe_sdf[2];
	int there_was_pipe;
	int new_input;
	int i;
	int bcg;
	int status;
	command_s* cmds;
	there_was_pipe = 0;
	new_input = -1;

	bcg = in_background(input_buf+beg);
	cmds = split_commands(input_buf+beg);
	if(cmds[0].argv==NULL) {
		return 0;
	}

	sigprocmask(SIG_BLOCK, &block_sig, NULL);

	if(shell_commands(cmds[0])) {
		sigprocmask(SIG_UNBLOCK, &block_sig, NULL);
		return 1;
	}

	there_was_pipe = 0;
	chld_pids_size = 0;
	chld_active = 0;
	new_input = -1;

	for(i = 0; cmds[i].argv != NULL; i++) {
		if(cmds[i+1].argv != NULL)  { /* mamy pipe'a */ 
			pipe(pipe_sdf);
			there_was_pipe = 1;
		}
		chld_pids_size++;
		chld_active++;
		pid = fork();
		if(pid == 0) {
			sigaction(SIGINT, &default_sigint_action, NULL);
			sigaction(SIGCHLD, &default_sigchld_action, NULL);

			/* tworzenie nowej sesji */
			if(bcg) setsid();
			/* wpp odblokowujemy SIGCHLD */
			sigprocmask(SIG_UNBLOCK, &block_sig, NULL);

			/*trzeba przekierowac wyjscie*/
			if(there_was_pipe) {
				dup2(pipe_sdf[1], STDOUT_FILENO);
				close(pipe_sdf[1]);
				close(pipe_sdf[0]);
			}
			/*przek wyjscia*/
			if(new_input != -1) {
				dup2(new_input, STDIN_FILENO);
				close(new_input);
			}
			/*wywolywanie pojedynczej komendy*/
			exec_one(cmds[i]);
		} else if(pid > 0) {
			if(!bcg) chld_pids[i] = pid;
			if(there_was_pipe) {
				close(new_input);
				close(pipe_sdf[1]);
			}
		}
		if(there_was_pipe) {
			new_input = pipe_sdf[0];
		}
		there_was_pipe = 0;
	}
	if(!bcg) {
		while(chld_active > 0) {
			/* sigsuspend(&wait_for_us);*/
			pid = waitpid(-1,&status,0); 
			bcg = 1;
			if (pid <= 0) {
				continue;
			}
			for (i=0;i<chld_pids_size;i++) if(chld_pids[i] == pid) bcg = 0;
			if (!bcg) {
				chld_active--;
			} else if (ended_processes < MAX_COMMANDS) {
				save_process(pid,status);
			}
		}
	}
	chld_pids_size = 0;
	sigprocmask(SIG_UNBLOCK, &block_sig, NULL);
	return 1;
}

/* wywoluje execa */

int travel_and_exec() {
	int beg_prim = beg;
	while(beg_prim < end) {
		while(beg_prim < end && input_buf[beg_prim] != '\n') {
			beg_prim++;
		}
		if (input_buf[beg_prim]=='\n') {
			input_buf[beg_prim] = '\0';
			exec();
			beg = beg_prim+1;
		}
	}
	return 1;
}

/* handler wejscia */
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
	struct stat myStat;
	int write_prompt;
	size_t howmany;
	fstat(0, &myStat);
	write_prompt = (S_ISCHR(myStat.st_mode));
	/* sigemptyset(&wait_for_us);
	sigaddset(&wait_for_us, SIGCHLD); nie mozna bo trzeba lapac*/
	sigprocmask(SIG_BLOCK, NULL, &wait_for_us);
	sigemptyset(&block_sig);
	sigaddset(&block_sig, SIGCHLD);
	beg = end = 0;
	handle_handlers();
	while(1) {
		if(write_prompt) {
			write_ended();
			write(1, prompt, strlen(prompt));
		}

		while((howmany = read(0,input_buf+end,BUFF_SIZE))<0) {
			if (errno != EINTR) {
				exit(EXIT_FAILURE);
			} 
		}
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