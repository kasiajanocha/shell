#define _POSIX_SOURCE 1

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>

#include <mcs.h>

#include "utils.h"


#define MAX_DELAY 30

int mutex,cv1,cv2;

int main(int argc, char* argv[]){

	int res,turns,i;
	int pp[2];
	char * text;

	if (argc!=2){
		fail("one argument needed");
	}

	turns=atoi(argv[1]);
	if (turns<0 || turns > MAX_DELAY){
		fail("wrong number of turns");
	}
	pipe(pp);

	setsid();
	mutex= getpid();
	cv1= mutex<<1;
	cv2= (mutex<<1)+1;
	if (fork()){
		/* parent */
		int r;
		char buf[10];

		for (i=0;i< turns; i++){
			mcs_wait(cv1,mutex);

			r= read(pp[0],buf,2);
			if (r!=2){
				if (r!=1){
					fprintf(stderr,"FAIL: pipe read failed.\n");
					kill(0, SIGTERM);
				}
				mcs_wait(cv1,mutex);
				r= read(pp[0],buf,2);
				if (r!=1){
					fprintf(stderr,"FAIL: unexpected number of bytes in pipe.\n");
					kill(0, SIGTERM);
				}
			}
			write(1,"TI",2);
			sleep(1);
			write(1,"C\n",2);
			mcs_broadcast(cv2);
		}
		mcs_unlock(mutex);
	} else {
		fork();

		mcs_lock(mutex);
		mcs_broadcast(cv1);
		write(pp[1],"0",1);
		for (i=0;i< turns; i++){
			mcs_wait(cv2,mutex);
			write(1,"TA",2);
			sleep(1);
			write(1,"C\n",2);
			mcs_broadcast(cv1);
			write(pp[1],"0",1);
		}
		mcs_unlock(mutex);
	}

	wait(NULL);
	exit(0);
}
