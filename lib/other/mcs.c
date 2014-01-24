#include "mcs.h"

#define MUT1 m_u.m_m1.m1i1
#define COND1 m_u.m_m1.m1i2

#define LOCK 91
#define UNLOCK 92
#define WAIT 93
#define BROADCAST 94

int lock(mutex_id)
int mutex_id;
{
	int sysc_res;
	message m;
	m.MUT1 = mutex_id;
	sysc_res = _syscall(MCS_PROC_NR, LOCK, &m);
	if(sysc_res == -1) {
		errno = EINTR;
	}
	return sysc_res;
}

int unlock(mutex_id)
int mutex_id;
{
	int sysc_res;
	message m;
	m.MUT1 = mutex_id;
	sysc_res = _syscall(MCS_PROC_NR, UNLOCK, &m);
	if(sysc_res == -1) {
		errno = EPERM;
	}
	return sysc_res;	
}

int wait(con_var_id, mutex_id)
int con_var_id;
int mutex_id;
{
	int sysc_res;
	message m;
	m.MUT1 = mutex_id;
	m.COND1 = con_var_id;
	sysc_res = _syscall(MCS_PROC_NR, WAIT, &m);
	if(sysc_res == -1) {
		errno = EINVAL;
	}
	return sysc_res;
}

int broadcast(con_var_id)
int con_var_id;
{
	message m;
	m.COND1 = con_var_id;
	return _syscall(MCS_PROC_NR, BROADCAST, &m);
}