#include <ansi.h>
#include <minix/ipc.h>
#include <stdio.h>

extern int errno;

int msysc(int who, int syscallnr,message* msgptr) {
	int status;

  msgptr->m_type = syscallnr;
  status = _sendrec(who, msgptr);
  if (status != 0) {
	msgptr->m_type = status;
  }
  if (msgptr->m_type < 0) {
	errno = -msgptr->m_type;
	return(-1);
  }
  return(msgptr->m_type);	
}


int mfork(void){
	message m;
  	return(msysc(0, 2, &m));
}

int main(int argc, char * argv[]){
	int result= mfork();

	printf("fork result:%d\n",result);
	return 0;
}
