#include "mcs.h"

/* Set debugging level to 0, 1, or 2 to see no, some, all debug output. */
#define DEBUG_LEVEL	1
#define DPRINTF		if (DEBUG_LEVEL > 0) printf
#define LOCK LOCK_SIGNAL_NMB
#define UNLOCK LOCK_SIGNAL_NMB + 1

/* Allocate space for the global variables. */
message m_in;		/* the input message itself */
message m_out;		/* the output message used for reply */
int who;		/* caller's proc number */
int callnr;		/* system call number */

struct mutex
{
  int id;
  int process;
};

int mutexes_nmb;
struct mutex locked_mutexes[MAX_MUT];

extern int errno;	/* error number set by system library */

/* Declare some local functions. */
FORWARD _PROTOTYPE(void init_server, (int argc, char **argv)		);
FORWARD _PROTOTYPE(void exit_server, (void)				);
FORWARD _PROTOTYPE(void get_work, (void)				);
FORWARD _PROTOTYPE(void reply, (int whom, int result)			);
// FORWARD _PROTOTYPE(void lock, (int mut, int who)      );
// FORWARD _PROTOTYPE(void unlock, (int mut, int who)      );



/*===========================================================================*
 *				main                                         *
 *===========================================================================*/
PUBLIC int main(int argc, char **argv)
{
/* This is the main routine of this service. The main loop consists of 
 * three major activities: getting new work, processing the work, and
 * sending the reply. The loop never terminates, unless a panic occurs.
 */
  int result;                 
  sigset_t sigset;

  /* Initialize the server, then go to work. */
  init_server(argc, argv);

  /* Main loop - get work and do it, forever. */         
  while (TRUE) {              

      /* Wait for incoming message, sets 'callnr' and 'who'. */
      get_work();

      switch (callnr) {
      case SYS_SIG:
          sigset = (sigset_t) m_in.NOTIFY_ARG;
          if (sigismember(&sigset,SIGTERM) || sigismember(&sigset,SIGKSTOP)) {
              exit_server();
          }
          continue;
      case LOCK
          
      default: 
          report("MCS","warning, got illegal request from %d\n", m_in.m_source);
          result = EINVAL;
      }

      /* Finally send reply message, unless disabled. */
      if (result != EDONTREPLY) {
	  reply(who, result);
      }
  }
  return(OK);				/* shouldn't come here */
}

/*===========================================================================*
 *				 init_server                                 *
 *===========================================================================*/
PRIVATE void init_server(int argc, char **argv)
{
/* Initialize the information service. */
  int fkeys, sfkeys;
  int i, s;
#if DEAD_CODE
  struct sigaction sigact;

  /* Install signal handler. Ask PM to transform signal into message. */
  sigact.sa_handler = SIG_MESS;
  sigact.sa_mask = ~0;			/* block all other signals */
  sigact.sa_flags = 0;			/* default behaviour */
  if (sigaction(SIGTERM, &sigact, NULL) < 0) 
      report("MCS","warning, sigaction() failed", errno);
#endif
}

/*===========================================================================*
 *				exit_server                                  *
 *===========================================================================*/
PRIVATE void exit_server()
{
  exit(0);
}

/*===========================================================================*
 *				get_work                                     *
 *===========================================================================*/
PRIVATE void get_work()
{
    int status = 0;
    status = receive(ANY, &m_in);   /* this blocks until message arrives */
    if (OK != status)
        panic("MCS","failed to receive message!", status);
    who = m_in.m_source;        /* message arrived! set sender */
    callnr = m_in.m_type;       /* set function call number */
}

/*===========================================================================*
 *				reply					     *
 *===========================================================================*/
PRIVATE void reply(who, result)
int who;                           	/* destination */
int result;                           	/* report result to replyee */
{
    int send_status;
    m_out.m_type = result;  		/* build reply message */
    send_status = send(who, &m_out);    /* send the message */
    if (OK != send_status)
        panic("IS", "unable to send reply!", send_status);
}

