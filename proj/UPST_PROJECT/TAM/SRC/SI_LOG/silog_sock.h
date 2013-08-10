#ifndef __SI_LOG_SOCK_H__
#define __SI_LOG_SOCK_H__

#include "nsocklib.h"	/* stNetTuple */

extern int Check_ClientEvent(stNetTuple *stNet, int *dSrvSfd, fd_set *Rfds, int *Numfds);
extern int dSendCheck(stNetTuple *stNet, int dIdx, fd_set *fdSet, int *NumFds, int dSrvSfd);

#endif /* __SI_LOG_SOCK_H__ */
