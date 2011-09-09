/*
   $Id: session.c,v 1.1.1.1 2011/04/19 14:13:43 june Exp $
 */

#include <errno.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <assert.h>
#include "session.h"

stHASHOINFO		*pHashInfo;
stTIMERNINFO 	*pTmrNInfo;
int sess_gSessionCnt;


hs_body *find_fd_session (hs_key *key);
hs_body *add_fd_session (hs_key *key, hs_body *body);
void del_fd_session (int sockfd);
hs_body *get_fd_session (hs_key *key);


hs_body *find_fd_session (hs_key *key)
{
	stHASHONODE		*phs_node;
	hs_body			*phs_body;

	phs_node = hasho_find (pHashInfo, (U8 *)key);
	if (phs_node==NULL) return NULL;

	phs_body = (hs_body *)FD_HASHO_PTR (phs_node->offset_Data);
	return phs_body;
}


hs_body *add_fd_session (hs_key *key, hs_body *body)
{
	stHASHONODE		*phs_node;
	hs_body			*phs_body;

	phs_node = hasho_add (pHashInfo, (U8 *)key, (U8 *)body);
	if (phs_node==NULL) return NULL;

	phs_body = ((hs_body *)FD_HASHO_PTR (phs_node->offset_Data));
	sess_gSessionCnt++;
	return phs_body;
}


void del_fd_session (int sockfd)
{
	if ((find_fd_session ((hs_key *)&sockfd)) != NULL ) {
		hasho_del (pHashInfo, (U8 *)&sockfd);
	}

	sess_gSessionCnt--;
	close(sockfd);
}


hs_body *get_fd_session (hs_key *key)
{
	hs_body	*phs_body, hs_body;

	phs_body = find_fd_session (key);
	if (phs_body == NULL) {
		phs_body = add_fd_session (key, &hs_body);
		if (phs_body == NULL)
			return NULL;
	} else {
		del_fd_session (key->sockfd);
		phs_body = get_fd_session (key);
		return NULL;
	}

	return phs_body;
}

#if 0
void writeHandler(int fd, void *data)
{
	int rlen = 0;
	hs_body *phs_body = (hs_body *)data;

	assert(fd);

	rlen = send (fd, phs_body->rbuf, phs_body->rlen, 0);
	debug(5, 5) ("writeHandler: write() returns %d\n", rlen);
	if (rlen == 0) {
		sessRelease();
	}
	else if (rlen < 0) {
		/* An error */
		if (ignoreErrno(errno)) {
			fprintf(stderr, "writeHandler: FD %d: write failure: %s.\n",
					fd, xstrerror());
			epoll_out_add (phs_body->efd, key->sockfd);
		} else {
			debug(5, 2) ("writeHandler: FD %d: write failure: %s.\n",
					fd, xstrerror());
			sessRelease();
		}
	} else {
		/* A successful write, continue */
		if (rlen < phs_body->rlen) {
			memmove(phs_body->rbuf+rlen, phs_body->rbuf, phs_body->rlen-rlen);
			phs_body->rlen = phs_body->rlen-rlen;
			phs_body->rbuf[phs_body->rlen]="\0";

			epoll_out_add (phs_body->efd, key->sockfd);
		} else {
			sessRelease();
		}
	}
}
#endif

void sessionRelease (int efd, int cfd, long tid)
{
	fprintf(stderr, " @ client session release(fd:%d)\n", cfd);
	if (efd>=0 && cfd >=0)
		epoll_del (efd, cfd);
	if (cfd>=0)
		del_fd_session (cfd);
	if (tid>=0)
		timerN_del(pTmrNInfo, tid);
}


void
invoke_send_wait_timeout (hs_key *pkey)
{  

	fprintf(stderr, " @ Time out(fd:%d)\n", pkey->sockfd);
	del_fd_session(pkey->sockfd);
}

/*
   $Log: session.c,v $
   Revision 1.1.1.1  2011/04/19 14:13:43  june
   성능 패키지

   Revision 1.1.1.1  2011/01/20 12:18:51  june
   DSC CVS RECOVERY

   Revision 1.1  2009/05/09 09:41:02  dsc
   init

   Revision 1.2  2009/03/03 12:06:07  june
   socket i/o event 방식으로 처리

   Revision 1.1.1.1  2009/02/17 13:35:34  june
   client_server simulator start

 */

