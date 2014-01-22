#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>

#define CHUNK_SIZE 4096

#define ROUNDS 100

volatile char * buffer;
int pipes[3][2];

void fillBuffer(volatile char * buf){
	int * p;
	for (p=(int *) buf; p< (int *)(buf+CHUNK_SIZE); p++){
		*p= rand();
	}
}

void consumeBuffer(volatile char * buf){
	write(1,(void *) buf, CHUNK_SIZE);
}

void consumer(int c){
	char A[2];

	close(pipes[3-c][STDOUT_FILENO]);
	close(pipes[3-c][STDIN_FILENO]);
	close(pipes[0][STDIN_FILENO]);
	close(pipes[c][STDOUT_FILENO]);

	while(read(pipes[c][STDIN_FILENO], A, 1)) {	
		consumeBuffer(buffer);
		write(pipes[0][STDOUT_FILENO], A, 1);
	}
	
	exit(0);
}

void producer(int c1, int c2){
	char A[2];
	close(1);
	if (open( "pr.out", O_CREAT|O_TRUNC|O_WRONLY,S_IWUSR|S_IRUSR)!=1) exit(1);

	close(pipes[1][STDIN_FILENO]);
	close(pipes[2][STDIN_FILENO]);
	close(pipes[0][STDOUT_FILENO]);

	for (int i=0; i<ROUNDS; i++) {
		fillBuffer(buffer);
		write(pipes[1][STDOUT_FILENO], A, 1);
		write(pipes[2][STDOUT_FILENO], A, 1);
		read(pipes[0][STDIN_FILENO], A, 1);
		consumeBuffer(buffer);
	}
}

void run_consumer(int c){
	char buf[20];
	int o =	sprintf(buf,"%d.out",c);
	close(1);
	if (open( buf, O_CREAT|O_TRUNC|O_WRONLY,S_IWUSR|S_IRUSR)!=1) exit(1);

	consumer(c);
	exit(0);
}

int main(){
	
	int c1, c2,b;
	srand(17);

	buffer = (char *) mmap(NULL, CHUNK_SIZE, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, -1, 0);

	pipe(pipes[0]);
	pipe(pipes[1]);
	pipe(pipes[2]);

	c1= fork();
	if (c1==0) run_consumer(1);

	c2= fork();
	if (c2==0) run_consumer(2);

	producer(c1,c2);
	return 0;
}


