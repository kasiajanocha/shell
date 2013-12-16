#define _POSIX_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

/* read from descriptors 3,4
   write to 1
   descriptor 0 closed
*/


int ph_num, my_num, next;




int main(int argc, char* argv[]){
	char buf[6]="nx,y \n", buf1[10];
	int n, ts1, ts2;

	ph_num= atoi(argv[1]);
	my_num= atoi(argv[2]);

	buf[0]=(char) my_num;

	n=read(3, buf1,10);
	buf1[n]=0;

	sleep(2);

	next= atoi(buf1);

	fprintf(stderr, "I am alive!\n");

	while(1){
		read(3,&ts1,sizeof(int));
		read(3,buf+1,1);
		read(4,&ts2,sizeof(int));
		read(4,buf+3,1);
		write(1,buf,6);
	}
}
