#include <time.h>
#include <unistd.h>
#include "socklib.h"
#include <strings.h>

int 	maxSockFdNum=0;
fd_set	readSockFdSet, writeSockFdSet, exceptSockFdSet;
fd_set	rdSockFdSet, wrSockFdSet, exSockFdSet;
BindSockFdContext	bindSockFdTbl[SOCKLIB_MAX_BIND_CNT];
ServerSockFdContext	serverSockFdTbl[SOCKLIB_MAX_SERVER_CNT];
ClientSockFdContext	clientSockFdTbl[SOCKLIB_MAX_CLIENT_CNT];
in_addr_t	allowClientAddrTbl[SOCKLIB_MAX_CLIENT_CNT];



/*------------------------------------------------------------------------------
* socklib에서 선언된 global variable을 초기화 한다.
------------------------------------------------------------------------------*/
void socklib_initial (char *fname)
{
	FD_ZERO(&readSockFdSet);
	FD_ZERO(&writeSockFdSet);
	FD_ZERO(&exceptSockFdSet);
	memset ((void*)bindSockFdTbl, 0, sizeof(bindSockFdTbl));
	memset ((void*)serverSockFdTbl, 0, sizeof(serverSockFdTbl));
	memset ((void*)clientSockFdTbl, 0, sizeof(clientSockFdTbl));
	memset ((void*)allowClientAddrTbl, 0, sizeof(allowClientAddrTbl));

	return;

} /** End of socklib_initial **/


/*------------------------------------------------------------------------------
* 지정된 file을 읽어 allowClientAddrTbl을 setting한다.
* - client로부터 connect 요구가 수신 되었을때 여기에 기록된 address만 허용한다.
------------------------------------------------------------------------------*/
int socklib_setAllowClientAddrTbl (char *fname)
{
	FILE	*fp;
	char	getBuf[32];
	int		cliCnt=0;

	if ((fp = fopen(fname,"r")) == NULL) {
		fprintf(stderr,"[socklib_setAllowClientAddrTbl] fopen fail[%s]; errno=%d(%s)\n",fname,errno,strerror(errno));
		return -1;
	}

	while (fgets(getBuf,sizeof(getBuf),fp) != NULL) {
		if (getBuf[0]=='#' || getBuf[0]=='\n')
			continue;
		allowClientAddrTbl[cliCnt++] = inet_addr(getBuf);
		if (cliCnt >= SOCKLIB_MAX_CLIENT_CNT)
			break;
	}
	fclose(fp);

	return cliCnt;

} /** End of socklib_setAllowClientAddrTbl **/



/*------------------------------------------------------------------------------
* 지정된 port로 socket fd를 생성하고, bind/listen한다.
* - 생성된 fd를 read/except fd_set에 추가한다.
* - 어떤 binding port에 connect 요구가 감지되었는지 확인하기 위해 생성된 fd를
*	bindSockFdTbl에 추가한다.
*	-> 즉, 여러개의 bindin port를 생성/관리할 수 있다.
* - binding을 INADDR_ANY로 함으로써, 시스템에 설정된 모든 ip_address에 대해
*	binding 된다.
* - 생성된 fd가 return된다.
------------------------------------------------------------------------------*/
int socklib_initTcpBind (int port)
{
	int		fd;
	struct sockaddr_in	myAddr;

	/* create socket fd
	*/
	if ((fd = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr,"[socklib_initTcpBind] socket fail; errno=%d(%s)\n",errno,strerror(errno));
		return -1;
	}

	/*
	*/
	if (socklib_setSockOption(fd) < 0) {
		close(fd);
		return -1;
	}
	if (socklib_setNonBlockMode(fd) < 0) {
		close(fd);
		return -1;
	}

	/* bind fd
	*/
	memset ((void*)&myAddr, 0, sizeof(myAddr));
	myAddr.sin_family = AF_INET;
	myAddr.sin_port = htons(port);
	myAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(fd, (struct sockaddr*)&myAddr, sizeof(myAddr)) < 0 ) {
		fprintf(stderr,"[socklib_initTcpBind] bind port(0x%x:D'%d) fail; errno=%d(%s)\n",port,port,errno,strerror(errno));
		close(fd);
		return -1;
	}

	/* listen
	*/
	if (listen(fd, SOMAXCONN) < 0) {
		fprintf(stderr,"[socklib_initTcpBind] listen fail; errno=%d(%s)\n",errno,strerror(errno));
		close(fd);
		return -1;
	}

	/* bindSockFdTbl에 port와 fd를 저장한다.
	*/
	if (socklib_addBindSockFdTbl (port, fd) < 0) {
		close(fd);
		return -1;
	}

	/* add socket fd to readSockFdSet, exceptSockFdSet
	*/
	socklib_addSockFdSet(fd); 

	return fd;

} /** End of socklib_initTcpBind **/



void socklib_dumHdlr (int signo) { }

/*------------------------------------------------------------------------------
* 지정된 ipAddr,port로 socket fd를 생성하여 접속한다.
* - 생성된 fd를 read/except fd_set에 추가한다.
* - 생성된 fd가 return된다.
------------------------------------------------------------------------------*/
int socklib_connect (char *ipAddr, int port)
{
	int		fd;
	struct sockaddr_in	dstAddr;

	if (!strcmp(ipAddr,""))
		return -1;

	/* create socket fd
	*/
	if ((fd = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr,"[socklib_connect] socket fail; errno=%d(%s)\n",errno,strerror(errno));
		return -1;
	}

	/*
	*/
	if (socklib_setSockOption(fd) < 0) {
		close(fd);
		return -1;
	}

	/* connect to server
	*/
	memset ((void*)&dstAddr, 0, sizeof(dstAddr));
	dstAddr.sin_family = AF_INET;
	dstAddr.sin_port = htons(port);
	dstAddr.sin_addr.s_addr = inet_addr(ipAddr);

	signal (SIGALRM, socklib_dumHdlr);
	ualarm(100000,0); /* multi-thread인 경우 문제가 될수 있다.*/
	if (connect(fd, (struct sockaddr*)&dstAddr, sizeof(dstAddr)) < 0 ) {
#ifdef DEBUG
		fprintf(stdout,"[socklib_connect] connect fail[%s]; errno=%d(%s)\n",ipAddr,errno,strerror(errno));
#endif
		ualarm(0,0);
		close(fd);
		return -1;
	}
	ualarm(0,0);

	if (socklib_setNonBlockMode(fd) < 0) {
		close(fd);
		return -1;
	}

	/* serverSockFdTbl에 address, port, fd를 저장한다.
	*/
	if (socklib_addServerSockFdTbl (&dstAddr, port, fd) < 0) {
		close(fd);
		return -1;
	}

	/* add socket fd to readSockFdSet, exceptSockFdSet
	*/
	socklib_addSockFdSet(fd);

	return fd;

} /** End of socklib_connect **/



/*------------------------------------------------------------------------------
* 새로운 접속요구가 감지된 경우 호출되어 accept하고 새로이 생성된 client_fd를 return
* - allowClientAddrTbl에 등록된 address에서 접속된 경우만 허용한다.
------------------------------------------------------------------------------*/
int socklib_acceptNewConnection (int srvPort, int srvFd)
{
	int		fd, len;
	struct sockaddr_in	cliAddr;

	len = sizeof(cliAddr);
	memset ((void*)&cliAddr, 0, sizeof(cliAddr));

	if ((fd = accept(srvFd, (struct sockaddr*)&cliAddr, &len)) < 0) {
		fprintf(stderr,"[socklib_acceptNewConnection] accept fail; errno=%d(%s)\n",errno,strerror(errno));
		return -1;
	}

#ifdef RESTRICTED_CLIENT
	/* allowClientAddrTbl에 등록된 address에서 요청되었는지 확인한다.
	*/
	for (i=0; i<SOCKLIB_MAX_CLIENT_CNT; i++) {
		if (cliAddr.sin_addr.s_addr == allowClientAddrTbl[i])
			break;
	}
	if (i >= SOCKLIB_MAX_CLIENT_CNT) {
#ifdef DEBUG
		fprintf(stdout,"[socklib_acceptNewConnection] isn't allow address(%s)\n", inet_ntoa(cliAddr.sin_addr));
#endif
		close(fd);
		return -1;
	}
#endif /*RESTRICTED_CLIENT*/

	if (socklib_setSockOption(fd) < 0) {
		fprintf(stderr,"[socklib_acceptNewConnection] socklib_setSockOption fail\n");
		close(fd);
		return -1;
	}
	if (socklib_setNonBlockMode(fd) < 0) {
		close(fd);
		return -1;
	}

	/* clientSockFdTbl에 address, port, fd를 저장한다.
	*/
	if (socklib_addClientSockFdTbl (&cliAddr, fd, srvPort) < 0) {
		close(fd);
		return -1;
	}

	/* add socket fd to readSockFdSet, exceptSockFdSet
	*/
	socklib_addSockFdSet(fd);

	return fd;


} /** End of socklib_acceptNewConnection **/



/*------------------------------------------------------------------------------
* 지정된 fd가 server로 접속된 fd이면 socklib_disconnectServerFd를 호출하고,
*	client가 접속한 fd이면 socklib_disconnectClientFd를 호출하여 접속을 해제한다.
------------------------------------------------------------------------------*/
int socklib_disconnectSockFd (int fd)
{
	int		i;

	for (i=0; i<SOCKLIB_MAX_SERVER_CNT; i++) {
		if (serverSockFdTbl[i].fd == fd) {
			return (socklib_disconnectServerFd(fd));
		}
	}

	for (i=0; i<SOCKLIB_MAX_CLIENT_CNT; i++) {
		if (clientSockFdTbl[i].fd == fd) {
			return (socklib_disconnectClientFd(fd));
		}
	}

#ifdef DEBUG
	fprintf(stdout,"[socklib_disconnectSockFd] not found fd \n");
#endif
	return -1;

} /** End of socklib_disconnectSockFd **/



/*------------------------------------------------------------------------------
* - 해당 fd를 close하고,
* - serverSockFdTbl에서 삭제하고,
* - read/except fd_set에서 삭제한다.
------------------------------------------------------------------------------*/
int socklib_disconnectServerFd (int fd)
{
	close(fd);

	/* serverSockFdTbl에서 fd에 대한 정보를 삭제한다.
	*/
	socklib_delServerSockFdTbl (fd);

	/* delete socket fd from readSockFdSet, exceptSockFdSet
	*/
	socklib_delSockFdSet(fd);

	return 1;

} /** End of socklib_disconnectServerFd **/



/*------------------------------------------------------------------------------
* - 해당 fd를 close하고,
* - clientSockFdTbl에서 삭제하고,
* - read/except fd_set에서 삭제한다.
------------------------------------------------------------------------------*/
int socklib_disconnectClientFd (int fd)
{
	close(fd);

	/* clientSockFdTbl에서 fd에 대한 정보를 삭제한다.
	*/
	socklib_delClientSockFdTbl (fd);

	/* delete socket fd from readSockFdSet, exceptSockFdSet
	*/
	socklib_delSockFdSet(fd);

	return 1;

} /** End of socklib_disconnectClientFd **/



/*------------------------------------------------------------------------------
* 등록된 fd들을 확인하여 event 발생여부를 확인한다.
* - event가 없으면 NO_EVENT를 return한다.
* - binding port에서 발생한 경우이면 새로운 client가 접속을 요구한 경우이므로
*	새로운 client_fd를 생성하여 accept한 후 actFd에 새로운 client_fd를 넘기고,
*	NEW_CONNECTION을 return한다.
* - client_fd 또는 server_fd에서 발생한 경우이면 메시지를 수신한 경우이거나 접속이
*	끊어진 경우이다.
*	-> 접속이 끊어진 경우 actFd에 disconnect된 fd를 넘기고, DISCONNECTED를 return한다.
*	-> 메시지를 수신한 경우 actFd에 메시지를 수신한 fd를 넘기고, buff에 수신된
*		메시지를 기록하고, SOCKLIB_MSG_RECEIVED를 return한다.
* - 기타 내부 error인 경우 SOCKLIB_INTERNAL_ERROR를 return한다.
------------------------------------------------------------------------------*/
int socklib_action (char *buff, int *actFd)
{
	int		port,fd,newFd;

	/* readSockFdSet/exceptSockFdSet를 확인하여 event 발생 여부를 확인한다.
	*/
	if (socklib_pollFdSet() <= 0)
		return SOCKLIB_NO_EVENT;

	/* binding port에서 event가 감지되었는지 확인한다.
	** - binding port 번호와 binding port의 socket fd가 return된다.
	*/
	if (socklib_lookupBindSockFdTbl (&port, &fd) > 0) {
		/* 새로운 client_fd를 생성한다.
		*/
		if ((newFd = socklib_acceptNewConnection (port, fd)) < 0) {
			return SOCKLIB_INTERNAL_ERROR;
		}
		*actFd = newFd;
		return SOCKLIB_NEW_CONNECTION;
	}

	/* server로 연결된 fd에서 event가 감지되었는지 확인한다.
	*/
	if ((fd = socklib_lookupServerSockFdTbl()) > 0) {
		/* 읽을 메시지가 있거나 disconnect된 경우인데, read를 시도해서 읽이지면
		**	메시지를 수신하고, read fail이면 disconnect된 것이다.
		*/
		*actFd = fd;
		if (socklib_readSockFd (buff, fd) < 0) {
			/* read fail이면 접속이 끊어진 것으로 판단하여 disconnect한다.
			*/
			socklib_disconnectServerFd (fd);
			return SOCKLIB_SERVER_DISCONNECTED;
		}
		return SOCKLIB_SERVER_MSG_RECEIVED;
	}

	/* client_fd에서 event가 감지되었는지 확인한다.
	*/
	if ((fd = socklib_lookupClientSockFdTbl()) > 0) {
		/* 읽을 메시지가 있거나 disconnect된 경우인데, read를 시도해서 읽이지면
		**	메시지를 수신하고, read fail이면 disconnect된 것이다.
		*/
		*actFd = fd;
		if (socklib_readSockFd (buff, fd) < 0) {
			/* read fail이면 접속이 끊어진 것으로 판단하여 disconnect한다.
			*/
			socklib_disconnectClientFd (fd);
			return SOCKLIB_CLIENT_DISCONNECTED;
		}
		return SOCKLIB_CLIENT_MSG_RECEIVED;
	}

	return SOCKLIB_INTERNAL_ERROR;

} /** End of socklib_action **/



/*------------------------------------------------------------------------------
* event가 감지된 fd에서 메시지를 read하여 buff에 넣는다.
* - header의 크기만큼 먼저 읽고 header에 있는 length field를 보고 나머지 body를
*	읽는다.
------------------------------------------------------------------------------*/
int socklib_readSockFd (char *buff, int fd)
{
	int		i, len, rLen, bodyLen;
	char	*ptr;
	SockLibHeadType	*head;
	struct timeval	waitTmr;

	waitTmr.tv_sec  = 0;
	waitTmr.tv_usec = SOCKLIB_POLL_TIMER;

	ptr = buff;

	/* header 부분만 먼저 읽는다.
	*/
	for (i=0, rLen=0;
		(i<5) && (rLen < SOCKLIB_HEAD_LEN);
		i++)
	{
		len = read(fd, ptr, SOCKLIB_HEAD_LEN-rLen);

		if (len == 0) {
#ifdef DEBUG
			fprintf(stdout,"[socklib_readSockFd] read fail(head,fd=%d); errno=%d(%s)\n",fd,errno,strerror(errno));
#endif
			return -1;
		} else if (len < 0) {
			if (errno==EAGAIN || errno==EINTR) {
#ifdef DEBUG
				fprintf(stdout,"[socklib_readSockFd] read would be blocked(head,fd=%d); errno=%d(%s)\n",fd,errno,strerror(errno));
#endif
				select (0,0,0,0,&waitTmr);
				continue;
			}
#ifdef DEBUG
			fprintf(stdout,"[socklib_readSockFd] read fail(head,fd=%d); errno=%d(%s)\n",fd,errno,strerror(errno));
#endif
			return -1;
		}

		ptr += len;
		rLen += len;
	}
	if (i==5 && rLen < SOCKLIB_HEAD_LEN) {
		/* header length만큼 읽어내지 못한 경우 garbage데이터가 들어 있는 것으로
		**	생각할 수 있는데, 이 경우 -1을 return하여 disconnect하도록 한다.
		*/
#ifdef DEBUG
		fprintf(stdout,"[socklib_readSockFd] can't read data_head \n");
#endif
		return -1;
	}

	/* header에 있는 length field를 꺼낸다.
	*/
	head = (SockLibHeadType*)buff;
#ifdef aaaaaa
	memcpy (tmp, head->bodyLen, sizeof(head->bodyLen));
	tmp[sizeof(head->bodyLen)] = 0;
	bodyLen = atoi(tmp);
#else
	bodyLen = head->bodyLen = ntohl(head->bodyLen);
#endif

	/* bodyLen만큼 읽어낸다.
	*/
	for (i=0, rLen=0;
		(i<5) && (rLen < bodyLen);
		i++)
	{
		len = read(fd, ptr, bodyLen-rLen);

		if (len == 0) {
#ifdef DEBUG
			fprintf(stdout,"[socklib_readSockFd] read fail(body,fd=%d); errno=%d(%s)\n",fd,errno,strerror(errno));
#endif
			return -1;
		} else if (len < 0) {
			if (errno==EAGAIN || errno==EINTR) {
#ifdef DEBUG
				fprintf(stdout,"[socklib_readSockFd] read would be blocked(body,fd=%d); errno=%d(%s)\n",fd,errno,strerror(errno));
#endif
				select (0,0,0,0,&waitTmr);
				continue;
			}
#ifdef DEBUG
			fprintf(stdout,"[socklib_readSockFd] read fail(body,fd=%d); errno=%d(%s)\n",fd,errno,strerror(errno));
#endif
			return -1;
		}

		ptr += len;
		rLen += len;
	}
	if (i==5 && rLen < bodyLen) {
		/* body length만큼 읽어내지 못한 경우 garbage데이터가 들어 있는 것으로
		**	생각할 수 있는데, 이 경우 -1을 return하여 disconnect하도록 한다.
		*/
#ifdef DEBUG
		fprintf(stdout,"[socklib_readSockFd] can't read data_body \n");
#endif
		return -1;
	}

	// 맨끝에 NULL을 채운다.
	*ptr = 0;

#ifdef DEBUG
	//fprintf(stdout,"[socklib_readSockFd] read len=%d\n", rLen+SOCKLIB_HEAD_LEN);
#endif
	return (rLen+SOCKLIB_HEAD_LEN);

} /** End of socklib_readSockFd **/



/*------------------------------------------------------------------------------
* 지정된 fd에 데이터를 length만큼 write한다.
* - head.bodyLen를 network byte order로 바꾼다.
* - write하기전에 write 가능한지 확인한다.
*	- select로 확인하는데 select fail 시 fd를 강제로 close한다.
------------------------------------------------------------------------------*/
int socklib_sndMsg (int fd, char *buff, int buffLen)
{
	int		i,len,wLen,ret;
	struct timeval	pollTmr;
	fd_set	wFdSet;
	char	*ptr;

	/* head.bodyLen를 network byte order로 바꾼다.
       but, block by sukhee :: because GUI can not receive 
	head = (SockLibHeadType*)buff;
	head->bodyLen = htonl(head->bodyLen);
	*/
	pollTmr.tv_sec  = 0;
	pollTmr.tv_usec = SOCKLIB_POLL_TIMER;

	FD_ZERO(&wFdSet);
	FD_SET (fd, &wFdSet);

	ptr = buff;

	for (i=0, wLen=0;
		(i<5) && (wLen < buffLen);
		i++)
	{
		/* write할 수 있는지 확인한다.
		*/
		ret = select (fd+1, NULL, &wFdSet, NULL, &pollTmr);
		if (ret == 0) {
#ifdef DEBUG
			fprintf(stdout,"[socklib_sndMsg] write would be blocked;\n");
#endif
			continue;
		} else if (ret < 0){
#ifdef DEBUG
			fprintf(stdout,"[socklib_sndMsg] select fail; errno=%d(%s)\n",errno,strerror(errno));
#endif
			/* serverSockFdTbl, clientSockFdTbl를 검색해 server로 접속된 fd인지,
			**	client가 접속한 fd인지 찾은 후 접속을 끊는다.
			*/
			socklib_disconnectSockFd (fd);
			return -1;
		}

		/* 해당 fd에 data를 write한다.
		*/
		len = write(fd, ptr, buffLen-wLen);
		if (len < 0) {
			if (errno==EAGAIN || errno==EINTR) {
#ifdef DEBUG
				fprintf(stdout,"[socklib_sndMsg] write would be blocked; errno=%d(%s)\n",errno,strerror(errno));
#endif
				select (0,0,0,0,&pollTmr);
				continue;
			}
			/* write fail이면 접속을 강제로 끊는다.
			*/
#ifdef DEBUG
			fprintf(stdout,"[socklib_sndMsg] write fail; errno=%d(%s)\n",errno,strerror(errno));
#endif
			socklib_disconnectSockFd (fd);
			return -1;
		}
		ptr += len;
		wLen += len;
	}
	if (wLen < buffLen) {
		/* 지정된 length만큼 write하지 못한 경우 접속을 강제로 끊는다.
		*/
#ifdef DEBUG
		fprintf(stdout,"[socklib_sndMsg] can't write msg \n");
#endif
		socklib_disconnectSockFd (fd);
		return -1;
	}

#ifdef DEBUG
	fprintf(stdout,"[socklib_sndMsg] write msg len = %d, fd=%d\n", wLen, fd);
#endif
	return wLen;

} /** End of socklib_sndMsg **/



/*------------------------------------------------------------------------------
------------------------------------------------------------------------------*/
int socklib_broadcast (char *buff, int buffLen)
{
	socklib_broadcast2Servers (buff, buffLen);
	socklib_broadcast2Clients (buff, buffLen);
	return 1;
} /** End of socklib_broadcast **/



/*------------------------------------------------------------------------------
* server로 연결된 모든 fd에 메시지를 write한다.
------------------------------------------------------------------------------*/
int socklib_broadcast2Servers (char *buff, int buffLen)
{
	int	i;

	for (i=0; i<SOCKLIB_MAX_SERVER_CNT; i++) {
		if (serverSockFdTbl[i].fd) {
			socklib_sndMsg (serverSockFdTbl[i].fd, buff, buffLen);
		}
	}
	return 1;
} /** End of socklib_broadcast2Servers **/



/*------------------------------------------------------------------------------
* client가 접속한 모든 fd에 메시지를 write한다.
------------------------------------------------------------------------------*/
int socklib_broadcast2Clients (char *buff, int buffLen)
{
	int	i;

	for (i=0; i<SOCKLIB_MAX_CLIENT_CNT; i++) {
		if (clientSockFdTbl[i].fd) {
			socklib_sndMsg (clientSockFdTbl[i].fd, buff, buffLen);
		}
	}
	return 1;
} /** End of socklib_broadcast2Clients **/



/*------------------------------------------------------------------------------
------------------------------------------------------------------------------*/
int socklib_setSockOption (int fd)
{
	int		sockOpt=1;
	int		reUseOpt=1;
	int		buffSize;
	struct linger	lin;

	if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (char*)&sockOpt, sizeof(sockOpt)) < 0) {
		fprintf(stderr,"[socklib_setSockOption] setsockopt(SO_KEEPALIVE) fail; errno=%d(%s)\n",errno,strerror(errno));
		return -1;
	}

	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&reUseOpt, sizeof(reUseOpt)) < 0) {
		fprintf(stderr,"[socklib_setSockOption] setsockopt(SO_REUSEADDR) fail; errno=%d(%s)\n",errno,strerror(errno));
		return -1;
	}

#ifdef TRU64
	if (setsockopt(fd, SOL_SOCKET, SO_CLUA_IN_NOALIAS, (char*)&sockOpt, sizeof(sockOpt)) < 0) {
		fprintf(stderr,"[socklib_setSockOption] setsockopt(SO_CLUA_IN_NOALIAS) fail; errno=%d(%s)\n",errno,strerror(errno));
		return -1;
	}
#endif

#ifdef SOREUSEPORT
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEPOER, (char*)&reUseOpt, sizeof(reUseOpt)) < 0) {
		fprintf(stderr,"[socklib_setSockOption] setsockopt(SO_REUSEPOER) fail; errno=%d(%s)\n",errno,strerror(errno));
		return -1;
	}
#endif /*SOREUSEPORT*/

	lin.l_onoff  = 1;
	lin.l_linger = 0;
	if (setsockopt(fd, SOL_SOCKET, SO_LINGER, (char*)&lin, sizeof(lin)) < 0) {
		fprintf(stderr,"[socklib_setSockOption] setsockopt(SO_LINGER) fail; errno=%d(%s)\n",errno,strerror(errno));
		return -1;
	}

#ifdef TCPNODELAY
	if ((proto = getprotobyname("tcp")) == NULL) {
		fprintf(stderr,"[socklib_setSockOption] getprotobyname() fail\n");
		return -1;
	}
	if (setsockopt(fd, proto->p_proto, TCP_NODELAY, (char*)&sockOpt, sizeof(sockOpt)) < 0) {
		fprintf(stderr,"[socklib_setSockOption] setsockopt(TCP_NODELAY) fail; errno=%d(%s)\n",errno,strerror(errno));
		return -1;
	}
#endif /*TCPNODELAY*/

	/* set segment buffer size
	*/
	buffSize = SOCKLIB_SEG_BUF_SIZE;
	if (setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (void*)&buffSize, sizeof(buffSize)) < 0) {
		fprintf(stderr,"[socklib_setSockOption] setsockopt(SO_RCVBUFF) fail; errno=%d(%s)\n",errno,strerror(errno));
		return -1;
	}
	if (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (void*)&buffSize, sizeof(buffSize)) < 0) {
		fprintf(stderr,"[socklib_setSockOption] setsockopt(SO_SNDBUFF) fail; errno=%d(%s)\n",errno,strerror(errno));
		return -1;
	}

	return 1;

} /** End of socklib_setSockOption **/



/*------------------------------------------------------------------------------
------------------------------------------------------------------------------*/
int socklib_setNonBlockMode (int fd)
{
	int		flag;

	/* set Non-Blocking Mode
	*/
	if ((flag = fcntl(fd, F_GETFL, 0)) < 0) {
		fprintf(stderr,"[socklib_setNonBlockMode] fcntl(F_GETFL) fail; errno=%d(%s)\n",errno,strerror(errno));
		return -1;
	}
	flag |= O_NONBLOCK;
	if (fcntl(fd, F_SETFL, flag) < 0) {
		fprintf(stderr,"[socklib_setNonBlockMode] fcntl(F_SETFL) fail; errno=%d(%s)\n",errno,strerror(errno));
		return -1;
	}

	return 1;

} /** End of socklib_setNonBlockMode **/



/*------------------------------------------------------------------------------
* bindSockFdTbl에 binding port와 socket_fd를 저장한다.
* - event 감지시 bindSockFdTbl에 등록된 fd에서 event가 발생한 경우이면
*	client로부터의 새로운 connection 요구가 수신되었음을 알 수 있다.
------------------------------------------------------------------------------*/
int socklib_addBindSockFdTbl (int port, int fd)
{
	int		i;

	for (i=0; i<SOCKLIB_MAX_BIND_CNT; i++) {
		if (bindSockFdTbl[i].fd == 0) {
			bindSockFdTbl[i].fd   = fd;
			bindSockFdTbl[i].port = port;
			break;
		}
	}
	if (i == SOCKLIB_MAX_BIND_CNT) {
		fprintf(stderr,"[socklib_addBindSockFdTbl] bindSockFdTbl full \n");
		return -1;
	}

	return i;

} /** End of socklib_addBindSockFdTbl **/



/*------------------------------------------------------------------------------
* serverSockFdTbl에 server로 connect된 fd와 server의 address, port를 저장한다.
------------------------------------------------------------------------------*/
int socklib_addServerSockFdTbl (struct sockaddr_in *srvAddr, int port, int fd)
{
	int		i;

	for (i=0; i<SOCKLIB_MAX_SERVER_CNT; i++) {
		if (serverSockFdTbl[i].fd == 0) {
			serverSockFdTbl[i].fd   = fd;
			serverSockFdTbl[i].port = port;
			memcpy (&serverSockFdTbl[i].srvAddr, srvAddr, sizeof(struct sockaddr_in));
			serverSockFdTbl[i].tStamp = time(NULL);
			break;
		}
	}
	if (i == SOCKLIB_MAX_SERVER_CNT) {
		fprintf(stderr,"[socklib_addServerSockFdTbl] serverSockFdTbl full \n");
		return -1;
	}

	return i;

} /** End of socklib_addServerSockFdTbl **/



/*------------------------------------------------------------------------------
* clientSockFdTbl에 client_fd와 client의 address를 저장한다.
* - 해당 client가 어떤 binding port로 접속했던 놈인지 구분할 수 있도록 binding port
*	번호를 함께 저장한다.
------------------------------------------------------------------------------*/
int socklib_addClientSockFdTbl (struct sockaddr_in *cliAddr, int fd, int srvPort)
{
	int		i;

	for (i=0; i<SOCKLIB_MAX_CLIENT_CNT; i++) {
		if (clientSockFdTbl[i].fd == 0) {
			clientSockFdTbl[i].fd   = fd;
			clientSockFdTbl[i].port = srvPort;
			memcpy (&clientSockFdTbl[i].cliAddr, cliAddr, sizeof(struct sockaddr_in));
			clientSockFdTbl[i].tStamp = time(NULL);
			break;
		}
	}
	if (i == SOCKLIB_MAX_CLIENT_CNT) {
		fprintf(stderr,"[socklib_addClientSockFdTbl] clientSockFdTbl full \n");
		return -1;
	}

	return i;

} /** End of socklib_addClientSockFdTbl **/



/*------------------------------------------------------------------------------
* serverSockFdTbl에서 fd에 대한 정보를 삭제한다.
------------------------------------------------------------------------------*/
int socklib_delServerSockFdTbl (int fd)
{
	int		i;

	for (i=0; i<SOCKLIB_MAX_SERVER_CNT; i++) {
		if (serverSockFdTbl[i].fd == fd) {
			memset ((void*)&serverSockFdTbl[i], 0, sizeof(ServerSockFdContext));
			break;
		}
	}
	if (i == SOCKLIB_MAX_SERVER_CNT) {
#ifdef DEBUG
		fprintf(stdout,"[socklib_delServerSockFdTbl] not found fd in serverSockFdTbl \n");
#endif
		return -1;
	}

	return i;

} /** End of socklib_delServerSockFdTbl **/



/*------------------------------------------------------------------------------
* clientSockFdTbl에서 fd에 대한 정보를 삭제한다.
------------------------------------------------------------------------------*/
int socklib_delClientSockFdTbl (int fd)
{
	int		i;

	for (i=0; i<SOCKLIB_MAX_CLIENT_CNT; i++) {
		if (clientSockFdTbl[i].fd == fd) {
			memset ((void*)&clientSockFdTbl[i], 0, sizeof(ClientSockFdContext));
			break;
		}
	}
	if (i == SOCKLIB_MAX_CLIENT_CNT) {
#ifdef DEBUG
		fprintf(stdout,"[socklib_delClientSockFdTbl] not found fd in clientSockFdTbl \n");
#endif
		return -1;
	}

	return i;

} /** End of socklib_delClientSockFdTbl **/



/*------------------------------------------------------------------------------
* read/except fd_set에 추가한다.
* - 새로운 fd가 생성되면 readSockFdSet/exceptSockFdSet에 setting하고,
*	이를 확인하면 event를 감지할 수 있다.
------------------------------------------------------------------------------*/
int socklib_addSockFdSet (int fd)
{
	FD_SET (fd, &readSockFdSet);
	FD_SET (fd, &exceptSockFdSet);

	if (fd >= maxSockFdNum)
		maxSockFdNum = fd + 1;

	return 1;

} /** End of socklib_addSockFdSet **/



/*------------------------------------------------------------------------------
* read/except fd_set에서 해당 fd를 삭제한다.
------------------------------------------------------------------------------*/
int socklib_delSockFdSet (int fd)
{
	FD_CLR (fd, &readSockFdSet);
	FD_CLR (fd, &exceptSockFdSet);

	if ((fd+1) == maxSockFdNum)
		maxSockFdNum--;

	return 1;

} /** End of socklib_delSockFdSet **/



/*------------------------------------------------------------------------------
* readSockFdSet/exceptSockFdSet를 확인하여 event 발생 여부를 확인한다.
------------------------------------------------------------------------------*/
int socklib_pollFdSet (void)
{
	struct timeval	pollTmr;
	int		ret;

	memcpy (&rdSockFdSet, &readSockFdSet, sizeof(fd_set));
	memcpy (&exSockFdSet, &exceptSockFdSet, sizeof(fd_set));

	pollTmr.tv_sec  = 0;
	pollTmr.tv_usec = SOCKLIB_POLL_TIMER;

	//ret = select(maxSockFdNum, &readSockFdSet, NULL, &exceptSockFdSet, &pollTmr);
	ret = select(maxSockFdNum, &rdSockFdSet, NULL, &exSockFdSet, &pollTmr);

	return ret;

} /** End of socklib_pollFdSet **/



/*------------------------------------------------------------------------------
* event가 감지된 fd가 binding port인지 확인한다.
* - 어떤 binding port에서 감지되었는지 해당 port번호와 port의 socket fd가 return한다.
------------------------------------------------------------------------------*/
int socklib_lookupBindSockFdTbl (int *srvPort, int *srvFd)
{
	int		i;

	for (i=0; i<SOCKLIB_MAX_BIND_CNT; i++) {
		if (!bindSockFdTbl[i].fd)
			continue;
		if (FD_ISSET(bindSockFdTbl[i].fd, &rdSockFdSet)) {
			*srvPort = bindSockFdTbl[i].port;
			*srvFd   = bindSockFdTbl[i].fd;
			return 1;
		}
		if (FD_ISSET(bindSockFdTbl[i].fd, &exSockFdSet)) {
			*srvPort = bindSockFdTbl[i].port;
			*srvFd   = bindSockFdTbl[i].fd;
			return 1;
		}
	}
	return -1;

} /** End of socklib_lookupBindSockFdTbl **/



/*------------------------------------------------------------------------------
* event가 감지된 fd가 server로 연결된 fd인지 확인한다.
------------------------------------------------------------------------------*/
int socklib_lookupServerSockFdTbl (void)
{
	int		i;

	for (i=0; i<SOCKLIB_MAX_SERVER_CNT; i++) {
		if (!serverSockFdTbl[i].fd)
			continue;
		if (FD_ISSET(serverSockFdTbl[i].fd, &rdSockFdSet)) {
			return serverSockFdTbl[i].fd;
		}
		if (FD_ISSET(serverSockFdTbl[i].fd, &exSockFdSet)) {
			return serverSockFdTbl[i].fd;
		}
	}
	return -1;

} /** End of socklib_lookupServerSockFdTbl **/



/*------------------------------------------------------------------------------
* event가 감지된 fd가 client_fd인지 확인한다.
------------------------------------------------------------------------------*/
int socklib_lookupClientSockFdTbl (void)
{
	int		i;

	for (i=0; i<SOCKLIB_MAX_CLIENT_CNT; i++) {
		if (!clientSockFdTbl[i].fd)
			continue;
		if (FD_ISSET(clientSockFdTbl[i].fd, &rdSockFdSet)) {
			return clientSockFdTbl[i].fd;
		}
		if (FD_ISSET(clientSockFdTbl[i].fd, &exSockFdSet)) {
			return clientSockFdTbl[i].fd;
		}
	}
	return -1;

} /** End of socklib_lookupClientSockFdTbl **/



/*------------------------------------------------------------------------------
* well-known port로 tcp 접속을 시도함으로써 ping test기능을 수행한다.
------------------------------------------------------------------------------*/
int socklib_ping (char *ipAddr)
{
	int		i,fd,port[4]={7,13,21,23};

	if (!strcmp(ipAddr,""))
		return -1;

	for (i=0; i<4; i++) {
		if ((fd = socklib_connect (ipAddr, port[i])) > 0) {
			socklib_disconnectSockFd(fd);
			return 1;
		}
	}
	return -1;
} /** End of socklib_ping **/





int
icmpsock()
{
    struct protoent* proto;
    int fd,opt=1;

    if((proto=getprotobyname("icmp"))==NULL) {
        return -1;
    }
    if((fd=socket(AF_INET,SOCK_RAW,proto->p_proto))<0) {
        return -1;
    }
    setsockopt(fd,SOL_SOCKET,SO_DEBUG,(char*)&opt,sizeof(opt));
    opt=0;
    setsockopt(fd,SOL_SOCKET,SO_DONTROUTE,(char*)&opt,sizeof(opt));
    return fd;
}

#define BACKLOG 5

int
tcp_listen(unsigned short port)
{
    int fd,opt=1;
    struct protoent* proto;
    struct sockaddr_in snd;

    if((proto=getprotobyname("tcp"))==NULL) {
        return -1;
    }
    if((fd=socket(AF_INET,SOCK_STREAM,proto->p_proto))<0) {
        return -1;
    }
    setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,(char*)&opt,sizeof(opt));
    bzero((char*)&snd,sizeof(snd));
    snd.sin_family      = AF_INET;
    snd.sin_addr.s_addr = htonl(INADDR_ANY);
    snd.sin_port        = htons(port);
    if(bind(fd,(struct sockaddr*)&snd,sizeof(snd))<0) {
        close(fd);
        return -1;
    }
    if(listen(fd,BACKLOG)<0) {
        close(fd);
        return -1;
    }
    return fd;
}

int
tcp_connect(unsigned int addr,unsigned short port)
{
    int fd,opt=1;
    struct protoent* proto;
    struct sockaddr_in snd;

    if((proto=getprotobyname("tcp"))==NULL) {
        return -1;
    }
    if((fd=socket(AF_INET,SOCK_STREAM,proto->p_proto))<0) {
        return -1;
    }
    setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,(char*)&opt,sizeof(opt));
    bzero((char*)&snd,sizeof(snd));
    snd.sin_family      = AF_INET;
    snd.sin_addr.s_addr = addr;
    snd.sin_port        = htons(port);
    if(connect(fd,(struct sockaddr*)&snd,sizeof(snd))<0) {
        close(fd);
        return -1;
    }
    return fd;
}

int
udpsock(unsigned short port)
{
    int fd,opt=1;
    struct protoent* proto;
    struct sockaddr_in snd;

    if((proto=getprotobyname("udp"))==NULL) {
        return -1;
    }
    if((fd=socket(AF_INET,SOCK_DGRAM,proto->p_proto))<0) {
        return -1;
    }
    setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,(char*)&opt,sizeof(opt));
    bzero((char*)&snd,sizeof(snd));
    snd.sin_family      = AF_INET;
    snd.sin_addr.s_addr = htonl(INADDR_ANY);
    snd.sin_port        = htons(port);
    if(bind(fd,(struct sockaddr*)&snd,sizeof(snd))<0) {
        close(fd);
        return -1;
    }
    return fd;
}

int
udpbsock(unsigned short port)
{
    int fd,opt=1;
    struct protoent* proto;
    struct sockaddr_in snd;

    if((proto=getprotobyname("udp"))==NULL) {
        return -1;
    }
    if((fd=socket(AF_INET,SOCK_DGRAM,proto->p_proto))<0) {
        return -1;
    }
    setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,(char*)&opt,sizeof(opt));
    bzero((char*)&snd,sizeof(snd));
    snd.sin_family      = AF_INET;
    snd.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    snd.sin_port        = htons(port);
    if(bind(fd,(struct sockaddr*)&snd,sizeof(snd))<0) {
        close(fd);
        return -1;
    }
    return fd;
}

int
udpaddrsock(unsigned char* ipaddr,unsigned short port)
{
    int fd,opt=1;
    struct protoent* proto;
    struct sockaddr_in snd;

    if((proto=getprotobyname("udp"))==NULL) {
        return -1;
    }
    if((fd=socket(AF_INET,SOCK_DGRAM,proto->p_proto))<0) {
        return -1;
    }
    setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,(char*)&opt,sizeof(opt));
    bzero((char*)&snd,sizeof(snd));
    snd.sin_family      = AF_INET;
    snd.sin_addr.s_addr = inet_addr((char*)ipaddr);
    snd.sin_port        = htons(port);
    if(bind(fd,(struct sockaddr*)&snd,sizeof(snd))<0) {
        close(fd);
        return -1;
    }
    return fd;
}

//다른 '어떤' 시스템에서는 define이 안되어 있을지도 모르기 때문에 주석처리함.
//Solaris 에서는 없이도 잘됨. 
//uname -a 결과 :: "SunOS SCMA 5.10 Generic_138888-07 sun4v sparc SUNW,SPARC-Enterprise-T2000"
//by uamyd 20110508
//#define INADDR_NONE 0

int
getsaddrbyhost(char* h,int port,struct sockaddr_in* snd)
{
    struct hostent* host;
    //struct in_addr  inaddr;

    snd->sin_family = AF_INET;
    snd->sin_port   = htons(port);
    if((snd->sin_addr.s_addr=inet_addr(h))!=INADDR_NONE) {
    }
    else {
        if((host = gethostbyname(h))==NULL) {
            return -1;
        }
        snd->sin_family = host->h_addrtype;
        bcopy(host->h_addr,(caddr_t)&snd->sin_addr,host->h_length);
    }
    return 1;
}

int
in_chksum(unsigned short* s,int n)
{
    unsigned int sum = 0;
    unsigned short answer;

    while(n>1) {
        sum += htons(*s);
        s   += 1;
        n   -= 2;
    }
    if(n==1) {
        sum += *(unsigned char*)s;
    }
    sum     = (sum>>16)+(sum&0xffff);
    sum     += (sum>>16);
    answer  = ~sum;

    return htons(answer);
}

int
sendnwait(char *s,int len,char *addr,unsigned short port,unsigned int wait)
{    
	struct sockaddr_in snd;
//	struct timeval tm;
//	int ret, nint;
    fd_set fds;
    int sec,msec,nbyte;
    int fd = udpsock(0);

    if(fd<=0) return -1;

    bzero((char*)&snd,sizeof(snd));
    snd.sin_family      = AF_INET;
    snd.sin_addr.s_addr = inet_addr(addr);
    snd.sin_port        = htons(port);

    FD_ZERO(&fds);
    FD_SET(fd,&fds);
    if((nbyte=sendto(fd,s,len,0,(struct sockaddr*)&snd,sizeof(snd)))<=0) {
        return -1;
    }
    sec = wait / 1000;
    msec= (wait % 1000)*1000;

////    settimeout(&tm,sec,msec);
////    if((ret=select(fd+1,&fds,NULL,NULL,&tm))<=0) return ret;
////    if(FD_ISSET(fd,&fds)) {
////        nint = sizeof(snd);
////        nbyte = recvfrom(fd,s,len,0,(struct sockaddr*)&snd,&nint);
////    }
    return nbyte;
}


#if defined(EXAMPLECODE)

#undef OPEN_MAX
#define OPEN_MAX    1024

int main()
{
    int fd;

    while((fd=udpsock(0))>0) printf("sockfd %d\n",fd);
    perror("sock");
}

#endif




