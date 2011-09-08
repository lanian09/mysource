
/***************************************************************
 * UPRESTO EPOLL LIBRORY
 *
 * AUTHOR  : JUNE.
 * VERSION : Revision: 1.0
 * DATE    : 2009-01-28
 * DESC    : epoll utility function
 ***************************************************************/

#ifndef _UEPOLL_H_
#define _UEPOLL_H_

int setNonBlock (int sock);
int setTcpNoDelay (int sock);
int setReuseAddr (int sock);

int initAcceptSock (int port);
int doAccept (int efd, int sfd);

int epoll_init (int event_size);
int epoll_in_add (int efd, int cfd);
int epollET_in_add (int efd, int cfd);
int epoll_out_add (int efd, int cfd);
int epoll_oneshot_add (int efd, int cfd);
int epoll_in_mod (int efd, int cfd);
int epoll_inout_mod (int efd, int cfd);
int epoll_out_mod (int efd, int cfd);
int epoll_oneshot_mod (int efd, int cfd);
int epoll_del (int efd, int cfd);

int ignoreErrno(int ierrno);

#endif //#define _UEPOLL_H_ 
