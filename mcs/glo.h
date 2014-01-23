/* Global variables. */

/* Parameters needed to keep diagnostics at MCS. */
#define MAX_MUT 1024
#define LOCK_SIGNAL_NMB 91
extern char diag_buf[MAX_MUT];	/* buffer for messages */
extern int diag_next;			/* next index to be written */
extern int diag_size;			/* size of all messages */

/* The parameters of the call are kept here. */
extern message m_in;		/* the input message itself */
extern message m_out;		/* the output message used for reply */
extern int who;			/* caller's proc number */
extern int callnr;		/* system call number */
extern int dont_reply;		/* normally 0; set to 1 to inhibit reply */

