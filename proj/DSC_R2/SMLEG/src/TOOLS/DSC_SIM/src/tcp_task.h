
#ifndef __TCP_TASK__
#define __TCP_TASK__

#include "session.h"

#define F_SID_INIT          1
#define F_CONNECTING        2
#define F_READING           4
#define F_DONE              8


extern int     g_sock[];
extern int     g_sock_cnt;

extern struct epoll_event g_events[];



extern void * tcp_task (void *p_arg);
extern int start_connect (hs_key *pksid, hs_body *pbsid);
extern int create_tcp_sock ();


#endif
