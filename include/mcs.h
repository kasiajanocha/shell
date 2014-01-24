#ifndef _MCS_H
#define _MCS_H
#ifndef _TYPES_H
#include <sys/types.h>
#endif
#include <lib.h>
#include <unistd.h>
/* Function Prototypes. */
_PROTOTYPE( int my_lock, (int mutex_id)                );
_PROTOTYPE( int my_unlock, (int mutex_id)              );
_PROTOTYPE( int my_wait, (int con_var_id, int mutex_id));
_PROTOTYPE( int my_broadcast, (int con_var_id)         );
#endif