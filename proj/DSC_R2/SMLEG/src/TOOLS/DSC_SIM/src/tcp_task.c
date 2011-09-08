#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <fcntl.h>

#include "logutil.h"
#include "session.h"
#include "tcp_task.h"


#define MAX_EVENTS   10000


struct epoll_event  g_events[MAX_EVENTS];
int		g_sock[MAX_EVENTS];
int		g_sock_cnt=0;

#ifdef SHOW_TRAFFIC

    time_t          rold_time, rnew_time;
    unsigned int    rcurr_traffic = 0;
    unsigned int    rtotal_cnt = 0;
    unsigned int    rcurr_cnt = 0;

#endif

int
create_tcp_sock ()
{
    int         flag;
    int         sockfd;
    int         dBufSize;
    socklen_t   slen;

    if ((sockfd = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
        dAppLog (LOG_DEBUG, "Can't open tcp socket! errno=%d[%s]\n", errno, strerror(errno));
        return -1;
    }
    if ((flag = fcntl (sockfd, F_GETFL, 0)) < 0) {
        dAppLog (LOG_DEBUG, "Fail in get socket option! errno=%d[%s]\n", errno, strerror(errno));
        close (sockfd);
        return -1;
    }

    flag |= O_NONBLOCK;
    if (fcntl (sockfd, F_SETFL, flag) < 0) {
        dAppLog (LOG_DEBUG, "Fail in Change Nonblcok option!  errno=%d[%s]\n", errno, strerror(errno));
        close (sockfd);
        return -1;
    }   
            
    return sockfd;
}


int 
start_connect (hs_key *pksid, hs_body *pbsid)
{   
    int         n, dret;
    struct      sockaddr_in stServerAddr;

    stServerAddr.sin_family         = AF_INET;
    stServerAddr.sin_addr.s_addr    = pbsid->dest_ipaddr;
    stServerAddr.sin_port           = pbsid->dest_port;

    if ( (n = connect (pbsid->cfd, (struct sockaddr *)&stServerAddr, sizeof(stServerAddr))) < 0) {

        if (errno != EINPROGRESS) {         
            dAppLog (LOG_DEBUG, "Nonblocking connect error! %d.%s-%s", errno, strerror(errno), __FUNCTION__);
            return -1;
        }

#if 0
        epollOne_cli_add (pbsid->tcpsock.sockfd);
        dAppLog(LOG_DEBUG, "--->] Nonblocking connect succss!! Dst:%d.%d.%d.%d : %d",
                                            NIPADDR(pbsid->dest_ipaddr), ntohs(pbsid->dest_port));
#endif
    
    }
    
    return 0;        
}



#if 0
void
rcv_tcpsock (struct epoll_event *event)
{
	int				flags, event_fd;
	int             n, error, dret;
	char            rbuf[1024*4];
	int				tcp_readlen;
	_hkey_fd		hfd_key;
	_hbody_fd		*phfd_body;

	hfd_key.sockfd = event_fd = event->data.fd;

	phfd_body = find_fd_session (&hfd_key);
	if (phfd_body == NULL) {

		dAppLog(LOG_DEBUG, "<--%d] Not found FD Session! %s-%d", hfd_key.sockfd, __FUNCTION__, __LINE__);
		/** clean **/
		del_fd_session(event_fd);
		return -1;
	}


	flags = phfd_body->tcpsock.f_flags;
	
	if ((flags & F_CONNECTING) && (event->events == EPOLLIN || event->events == EPOLLOUT)) {

        n = sizeof (error);
        if ((getsockopt(event_fd, SOL_SOCKET, SO_ERROR, &error, &n) < 0) || (error != 0)) {
                    
            dAppLog(LOG_WARN, "---]  nonblocking connect failed (reset)aaa %d-%s..%s-line:%d",
                    error, strerror(error),
                    __FUNCTION__, __LINE__);
		}

		phfd_body->tcpsock.f_flags = F_READING;
		epollin_cli_mod (event_fd);
		phfd_body->tcpsock.sock_idx = g_sock_cnt;
		g_sock [g_sock_cnt++] = event_fd;

		// write 
		if (g_auto_send == 1)
		{
			send_http_num_only (event_fd);
		}
	} else if (flags & F_READING) {
		tcp_readlen = read (event_fd, rbuf, sizeof (rbuf));
		dAppLog (LOG_DEBUG, "tcp_readlen: %d", tcp_readlen);

#ifdef SHOW_TRAFFIC
        rcurr_traffic += tcp_readlen;
        rtotal_cnt ++;
        rcurr_cnt ++;

		rnew_time = time(NULL);

        if ((rnew_time - rold_time) >= 5) {
            dAppLog (LOG_CRI, "WCLI TCP Traffic: %6.3f Kbps, Count: total=%d, %6.3f/sec",
                    (float)rcurr_traffic*8 / ((float)(rnew_time-rold_time)*1024), rtotal_cnt, (float)rcurr_cnt/(rnew_time-rold_time));
            
            rold_time = rnew_time;
            rcurr_traffic = 0;
            rcurr_cnt = 0;
        }
#endif

#ifdef TCP_DUMP
	    dAppLog (LOG_DEBUG,"[TCP RECV] ==============================>>>=================================");
		dAppDump((char*)rbuf, tcpreadlen);
#endif

		if (tcp_readlen == 0) { 	// tcp close 

			timerN_del(pReTimerInfo, phfd_body->send_timer);
			del_fd_session(event_fd);
			event_fd = -1;
		} else if (tcp_readlen > 0) {
//			dAppDump((char*)rbuf, tcp_readlen);
//			dAppLog (LOG_DEBUG, "%s", rbuf);
//	        phfd_body->send_timer = timerN_update (pReTimerInfo, phfd_body->send_timer, time(0) + g_wait_timeout);
//			dAppLog (LOG_CRI, "phfd_body->send_timer :%d", phfd_body->send_timer);
		} else {
			dAppLog (LOG_DEBUG, "read fail %d-%s", errno, strerror(errno));
		}

	} else {
		tcp_readlen = read (event_fd, rbuf, sizeof (rbuf));
		dAppLog (LOG_CRI, "??? read %d-", tcp_readlen);
		if (tcp_readlen <= 0) {

            dAppLog(LOG_WARN, "---] nonblocking connect refuse (reset)eeee %d-%s..",
                                            errno, strerror(errno));
			dAppLog(LOG_CRI, "---%d] Proxy off2! DestIP: %d.%d.%d.%d : %d connect failed", event_fd,
                   		NIPADDR(phfd_body->dest_ipaddr), ntohs(phfd_body->dest_port));

			del_fd_session(event_fd);
		}
	}
}
#endif

#if 0
void *
tcp_task (void *p_arg)
{

	int		i, nfds, dret;
	int		event_fd;

#ifdef SHOW_TRAFFIC
    rold_time = time (NULL);
#endif

	while (1) {


		timerN_invoke (pTimerInfo);
		timerN_invoke (pReTimerInfo);
		nfds = epoll_wait (g_epoll_fd, g_events, MAX_EVENTS, 100);  /**< timeout 100ms */
		if (nfds == 0) {
			timerN_invoke (pTimerInfo);
			timerN_invoke (pReTimerInfo);
			continue;       /**< no event **/
		}

		if (nfds < 0) {
			if( errno != EINTR ) {
				dAppLog (LOG_CRI, "epoll wait error! %d-%s. EPOLL_FD[%d]", errno, strerror(errno), g_epoll_fd);
				cmd_quit ("epoll wait error");
				exit(1);
			}
		}


		for (i=0; i < nfds; i++) {
			rcv_tcpsock (&g_events[i]);
		}
	}


	return p_arg;
}
#endif
