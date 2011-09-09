
/***************************************************************
 * UPRESTO EPOLL LIBRORY
 *
 * AUTHOR  : JUNE.
 * VERSION : Revision: 1.0
 * DATE    : 2009-01-28
 * DESC    : epoll utility function
 ***************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <signal.h>

#include "uepoll.h"


int setNonBlock (int sock)
{
	int flags = fcntl (sock, F_GETFL);

	flags |= O_NONBLOCK;
	if (fcntl(sock, F_SETFL, flags)<0){
		perror ("setNonBlock, executing nonblock error");
		return -1;
	}
	return 0;
}


int setTcpNoDelay (int sock)
{
	int flag = 1;
	return (setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof (int)));
}


int setReuseAddr (int sock)
{
	int flag = 1;
	return (setsockopt (sock, SOL_SOCKET, SO_REUSEADDR, (char *)&flag, sizeof (int)));
}


int initAcceptSock (int port)
{
	struct sockaddr_in addr;

	int sfd = socket (AF_INET, SOCK_STREAM, 0);
	if (sfd == -1){
		perror ("initAcceptSock, socket error");
		close (sfd);
		return -1;
	}
	setReuseAddr (sfd);

	addr.sin_family = AF_INET;
	addr.sin_port = htons (port);
	addr.sin_addr.s_addr = htonl (INADDR_ANY);
	if (bind(sfd, (struct sockaddr *) &addr, sizeof (addr)) == -1){
		close (sfd);
		return -2;
	}
	listen (sfd, 5);

	if(setNonBlock (sfd)==-1) {
		perror ("doAccept, setNonBlock error");
		return -1;
	}

	return sfd;
}


int doAccept (int efd, int sfd)
{
	int cfd;
	struct sockaddr_in cliAddr;
	socklen_t cliLen = sizeof (cliAddr);

	bzero (&cliAddr, sizeof (cliAddr));

	cfd = accept (sfd, (struct sockaddr *)&cliAddr, &cliLen);
	if (cfd < 0){
		perror ("doAccept, accept error");
		return -1;
	}
	printf("\naccept ok fd(%d)\n", cfd);
	if(setNonBlock (cfd)==-1) {
		perror ("doAccept, setNonBlock error");
		return -1;
	}
	if(setTcpNoDelay (cfd)==-1) {
		perror ("doAccept, setTcpNoDelay error");
		return -1;
	}

	epoll_in_add (efd, cfd);
	return cfd;
}


int epoll_init (int event_size)
{
	int efd;
	if ((efd = epoll_create (event_size)) < 0){
		perror ("initEpoll, epoll_create error");
		return -1;
	}
	return efd;
}


int epoll_in_add (int efd, int cfd)
{
	struct epoll_event ev;

	ev.events = EPOLLIN;
	ev.data.fd = cfd;
	return epoll_ctl (efd, EPOLL_CTL_ADD, cfd, &ev);
}


int epollET_in_add (int efd, int cfd)
{
	struct epoll_event ev;

	ev.events = EPOLLIN|EPOLLET;
	ev.data.fd = cfd;
	return epoll_ctl (efd, EPOLL_CTL_ADD, cfd, &ev);
}


int epoll_out_add (int efd, int cfd)
{
	struct epoll_event ev;

	ev.events = EPOLLOUT|EPOLLIN;
	ev.data.fd = cfd;
	return epoll_ctl (efd, EPOLL_CTL_ADD, cfd, &ev);
}


int epoll_oneshot_add (int efd, int cfd)
{
	struct epoll_event ev;

	ev.events = EPOLLIN|EPOLLOUT|EPOLLET|EPOLLONESHOT;
	ev.data.fd = cfd;
	return epoll_ctl (efd, EPOLL_CTL_ADD, cfd, &ev);
}


int epoll_in_mod (int efd, int cfd)
{
	struct epoll_event ev;

	ev.events = EPOLLIN;
	ev.data.fd = cfd;
	return epoll_ctl (efd, EPOLL_CTL_MOD, cfd, &ev);
}


int epoll_inout_mod (int efd, int cfd)
{
	struct epoll_event ev;

	ev.events = EPOLLOUT|EPOLLIN;
	ev.data.fd = cfd;
	return epoll_ctl (efd, EPOLL_CTL_MOD, cfd, &ev);
}


int epoll_out_mod (int efd, int cfd)
{
	struct epoll_event ev;

	ev.events = EPOLLOUT;
	ev.data.fd = cfd;
	return epoll_ctl (efd, EPOLL_CTL_MOD, cfd, &ev);
}


int epoll_oneshot_mod (int efd, int cfd)
{
	struct epoll_event ev;

	ev.events = EPOLLIN|EPOLLOUT|EPOLLET|EPOLLONESHOT;
	ev.data.fd = cfd;
	return epoll_ctl (efd, EPOLL_CTL_MOD, cfd, &ev);
}


int epoll_del (int efd, int cfd)
{
	struct epoll_event ev;

	ev.events = 0;
	ev.data.fd = cfd;
	return epoll_ctl (efd, EPOLL_CTL_DEL, cfd, &ev);
}

int ignoreErrno(int ierrno)
{
	switch (ierrno) {
		case EINPROGRESS:
		case EAGAIN:
		case EINTR:
			return 1;
		default:
			return 0;
	}
}


#if 0
#define EPOLL_SIZE 4096
#define EVENT_SIZE 4096
#define MAX_RECV_BUF 1024*10


int doEcho (int efd, int cfd)
{
	char buf[MAX_RECV_BUF];
	char tmp[MAX_RECV_BUF];
	int n;
	int slen, tlen;

	memset(buf, 0x00, sizeof(buf)); 
	n = recv(cfd, buf, MAX_RECV_BUF, 0);
	if (n <= 0) 
	{ 
		epoll_del(efd, cfd); 
		close(cfd); 
		printf("Close fd(%d)\n", cfd); 
	} 
	else {
		//printf("read data(fd:%d): %s(%d)\n", cfd, buf, n);
		strcpy(tmp, buf+12);
		printf("read data(fd:%d): %s(%d)\n", cfd, tmp, n);
		tlen=slen=0;
		//slen = send (cfd, buf+slen, n-slen, 0);
#if 1
		while (tlen != n)
		{
			slen = send (cfd, buf+slen, n-slen, 0);
			tlen+= slen;
		}
#endif
	}
}


int main (int argc, char **argv)
{
	int port = 0;
	const int poolSize = EPOLL_SIZE;
	const int epollSize= EVENT_SIZE;
	struct epoll_event *events;
	struct epoll_event ev;
	int efd=0, sfd=0, cfd=0;
	int rst=0;
	int rvEvents=0;
	int i=0;

	printf ("WEB SERVER v1.0 - build date: %s\n", __DATE__);
	if (argc > 1){
		port = atoi (argv[1]);
	}
	else {
		printf ("usage: $webserver port\n");
		return 0;
	}

	signal (SIGPIPE, SIG_IGN);

	efd = epoll_init (epollSize);
	if (efd < 0) {
		perror (" init_epoll error");
		return 0;
	}

	events = (struct epoll_event *) malloc(sizeof(*events) * poolSize); 
	if (NULL == events) {
		perror (" epoll_event malloc error");
		close(efd);
		return 0;
	}

	sfd = initAcceptSock (port);
	if (sfd < 0) {
		perror (" init_acceptsock error");
		close(efd);
		close(sfd);
		free(events);
		return 0;
	}

	rst = epoll_in_add (efd, sfd);
	if (rst < 0) {
		perror ("epollf_in_add error");
		close(efd);
		close(sfd);
		free(events);
		return 0;
	}

	while (1) 
	{ 
		rvEvents = epoll_wait (efd, events, poolSize, -1); 
		for (i = 0; i < rvEvents; i++)
		{ 
			if (events[i].data.fd == sfd) { 
				cfd = doAccept (efd, sfd);
			} 
			else {
				doEcho (efd, events[i].data.fd);
			} 
		} 
	} 

}

#endif
