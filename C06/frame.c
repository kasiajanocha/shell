#define _POSIX_SOURCE 1

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#define N 5
#define ROUNDS 30


volatile int interrupted=0;

void handler(int n){
	interrupted=1;
}

char encode(int i, int j);
void decode(char i, char * c1, char *c2);
int validate(char c1, char c2, int n);

void counter(pid_t producer);

int rounds;

int main(int argc, char* argv[])
{
	FILE* outF[N];

	int i,j, flags;
	char c, buf1[10], buf2[10],buf3[10];
	pid_t producer;

	int pipes[2*N], opipe[2];
	pid_t children[N];

	if (argc != 2 || ((rounds= atoi(argv[1]))<0)) {
		printf("%s NB_OF_ROUNDS\n", argv[0]);
		exit(1);
	}
	printf("rounds: %d\n", rounds);
	


	pipe(opipe);

	producer= fork();
	if (producer == -1) {
		fprintf(stderr,"Producer process fork failed.\n");
		exit(1);
	}
	if (producer) {
		dup2(opipe[0],0);
		close(opipe[1]);
		close(opipe[0]);
		counter(producer);
	}

/*	PRODUCER STARTS HERE */

	struct sigaction sa;
	sa.sa_handler=handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags=0;

	sigaction(SIGINT,&sa,NULL);

	for (i=0; i<N; i++) {
		if (pipe(&pipes[2*i])){
			fprintf(stderr,"Pipe creation failed.\n");
			exit(1);
		};
	}

	close(opipe[0]);

	sprintf(buf1, "%d", N);

	for (i=0; i< N; i++){
		children[i]= fork();
		if (children[i]==-1) exit(1);
		if (children[i]==0){
			close(0);
			dup2(opipe[1], 1);
			close(opipe[1]);
			dup2(pipes[(2*i+2)%(2*N)],4);
			dup2(pipes[2*i],3);
			for (j=0; j<= pipes[2*N-1]; j++) 
				if (pipes[j]>4) close(pipes[j]); 

			sprintf(buf2, "%d",i);
	
			if (!execl("./phil.x","phil.x", buf1, buf2, NULL)){
				fprintf(stderr,"Exec failed!\n");
				exit(1);
			}
		}
	}

	close(opipe[1]);
	for (i=0; i<N; i++){
		close(pipes[2*i]);
		sprintf(buf3, "%d", children[(i+1)%N]);
		write(pipes[2*i+1], buf3, strlen(buf3));
	}

	j=0;

	for (j=0; j< rounds; j++){
		sleep(1);
		for (i=0; i<N; i++){
			c=encode(j,i);
			write(pipes[2*i+1],&j,sizeof(int));	//	time stamp
			write(pipes[2*i+1],&c,1);	//	value
		}
	}

//	for (i=0; i<N; i++){
//		kill(children[i], SIGTERM);
//	}
}

void counter(pid_t producer){
	int valid[N], invalid[N];
	int i,n;
	char buf[7];

	for (i=0; i<N; i++) 
		valid[i]=invalid[i]=0;

	printf("Counter STARTED\n");
 	for (i=0; i< rounds; i++){
		n=read(0,buf,6);
		if ((n==-1) && (errno==EINTR)) continue;
		if (n!=6) {
			printf("Boundary error: %d\n",n);
			break;
		}
		if (buf[0]!=buf[0]%N) {
			printf("Invalid ID: %d\n",buf[0]);
			break;
		}
		else if (!validate(buf[1],buf[3],N)){
			printf("Invalid pair: [%d,%d] from %d\n",buf[1],buf[3], buf[0]);
			invalid[buf[0]]++;
		} else {
			printf("Valid pair: [%d,%d] from %d\n",buf[1],buf[3],buf[0]);
			valid[buf[0]]++;
		}
	}


	for (i=0; i<N; i++) 
		printf("[%d] valid:%d 	invalid:%d\n", i,valid[i],invalid[i]);

	kill(producer, SIGINT);

	exit(0);
}
