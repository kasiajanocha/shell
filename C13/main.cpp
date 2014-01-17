#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>

#define CHUNK_SIZE 4096

#define ROUNDS 100

volatile char * buffer;

void fillBuffer(volatile char * buf){
	int * p;
	for (p=(int *) buf; p< (int *)(buf+CHUNK_SIZE); p++){
		*p= rand();
	}
}

void consumeBuffer(volatile char * buf){
	write(1,(void *) buf, CHUNK_SIZE);
}

void consumer(){
	// ...
	// consumeBuffer(buffer);
	// ...
	exit(0);
}

void producer(int c1, int c2){
	close(1);
	if (open( "pr.out", O_CREAT|O_TRUNC|O_WRONLY,S_IWUSR|S_IRUSR)!=1) exit(1);

	for (int i=0; i< ROUNDS; i++) 
			fillBuffer(buffer);
}

void run_consumer(int c){
	char buf[20];
	int o=	sprintf(buf,"%d.out",c);
	close(1);
	if (open( buf, O_CREAT|O_TRUNC|O_WRONLY,S_IWUSR|S_IRUSR)!=1) exit(1);

	consumer();
	exit(0);
}

int main(){
	
	int c1, c2,b;
	srand(17);

	buffer = (char *) mmap(...);

	c1= fork();
	if (c1==0) run_consumer(1);

	c2= fork();
	if (c2==0) run_consumer(2);

	producer(c1,c2);
	return 0;
}


