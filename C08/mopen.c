#include <ansi.h>
#include <minix/ipc.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

#define BSIZE 1024
#define open	_open

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


#if _ANSI
PUBLIC int open(const char *name, int flags, ...)
#else
PUBLIC int open(name, flags)
_CONST char *name;
int flags;
#endif
{
  va_list argp;
  message m;

  va_start(argp, flags);
  if (flags & O_CREAT) {
	m.m1_i1 = strlen(name) + 1;
	m.m1_i2 = flags;
	m.m1_i3 = va_arg(argp, _mnx_Mode_t);
	m.m1_p1 = (char *) name;
  } else {
	_loadname(name, &m);
	m.m3_i2 = flags;
  }
  va_end(argp);
  return (msysc(1, 5, &m));
}

int main(int argc, char * argv[]){
	char buf[BSIZE];
	int n;

	int fd;

	if (argc!=2){
		fprintf(stderr, "Wrong number of arguments.");
		exit(1);
	}

	fd= mopen(argv[1], 0);

	if (fd<0){
		fprintf(stderr, "Cannot open:%s\n",argv[1]);
		exit(1);
	}

	while (n=read(fd, buf,BSIZE)){	
		if ((n==-1) && (errno==EINTR || errno==EAGAIN)) continue;
		if (n==-1) exit(1);
		write(1, buf, n);
	}
	return 0;
}
