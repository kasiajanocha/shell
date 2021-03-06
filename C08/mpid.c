#include <ansi.h>
#include <minix/ipc.h>
#include <stdio.h>

extern int errno;

int mgetpid(void){
	//return(_syscall(0, 20, &m)); who, syscallnr, msgptr
	message m;
	
	int status;

  	msgptr->m_type = 20;
	status = _sendrec(0, &m);
	if (status != 0) {
		msgptr->m_type = status;
  	}
  	
	if (msgptr->m_type < 0) {
		errno = -msgptr->m_type;
		return -1;
	}
	return(msgptr->m_type);
}

int main(int argc, char * argv[]){
	int pid= mgetpid();

	printf("my pid:%d\n",pid);
	return 0;
}
