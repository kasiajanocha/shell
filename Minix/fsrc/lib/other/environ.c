/*
 * environ.c - define the variable environ
 */
/* $Header: /cvsup/minix/src/lib/other/environ.c,v 1.1.1.1 2005/04/21 14:56:26 beng Exp $ */
/*
 * This file defines the variable environ and initializes it with a magic
 * value.  The C run-time start-off routine tests whether the variable
 * environ is initialized with this value.  If it is not, it is assumed
 * that it is defined by the user.  Only two bytes are tested, since we
 * don't know the endian-ness and alignment restrictions of the machine.
 * This means that the low-order two-bytes should be equal to the
 * high-order two-bytes on machines with four-byte pointers.  In fact, all
 * the bytes in the pointer are the same, just in case.
 */

#if _EM_PSIZE==2
char **environ = (char **) 0x5353;
#else
char **environ = (char **) 0x53535353;
#endif
