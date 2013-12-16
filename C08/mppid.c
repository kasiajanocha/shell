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

int mgetppid(void){
	message m;

	if (msysc(0, 20, &m) < 0) return ( (pid_t) -1);
	return( (pid_t) m.m2_i1);
}

int main(int argc, char * argv[]){
	int ppid= mgetpid();

	printf("parent pid:%d\n",pid);
	return 0;
}
