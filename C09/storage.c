#include "pm.h"
#include <minix/callnr.h>
#include <signal.h>
#include "mproc.h"
#include "param.h"

extern int errno;

int store (int n) {
	message msg;
	register message * ptr = &msg;
	
	ptr->m_type = 49;
	ptr->m1_i1 = n;
}

int retrieve(int *pn) {
	
}

int main(int argc, char * argv[]){
	
	int st_res = store(5);
	
	return 0;
}
