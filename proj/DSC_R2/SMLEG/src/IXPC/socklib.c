#include "socklib.h"

int 	maxSockFdNum=0;
fd_set	readSockFdSet, writeSockFdSet, exceptSockFdSet;
fd_set	rdSockFdSet, wrSockFdSet, exSockFdSet;
BindSockFdContext	bindSockFdTbl[SOCKLIB_MAX_BIND_CNT];
ServerSockFdContext	serverSockFdTbl[SOCKLIB_MAX_SERVER_CNT];
ClientSockFdContext	clientSockFdTbl[SOCKLIB_MAX_CLIENT_CNT];
in_addr_t	allowClientAddrTbl[SOCKLIB_MAX_CLIENT_CNT];
char	sockErrBuf[4096], sockErrTmp[1024];



/*------------------------------------------------------------------------------
* socklib���� ����� global variable�� �ʱ�ȭ �Ѵ�.
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
* ������ file�� �о� allowClientAddrTbl�� setting�Ѵ�.
* - client�κ��� connect �䱸�� ���� �Ǿ����� ���⿡ ��ϵ� address�� ����Ѵ�.
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
* ������ port�� socket fd�� �����ϰ�, bind/listen�Ѵ�.
* - ������ fd�� read/except fd_set�� �߰��Ѵ�.
* - � binding port�� connect �䱸�� �����Ǿ����� Ȯ���ϱ� ���� ������ fd��
*	bindSockFdTbl�� �߰��Ѵ�.
*	-> ��, �������� bindin port�� ����/������ �� �ִ�.
* - binding�� INADDR_ANY�� �����ν�, �ý��ۿ� ������ ��� ip_address�� ����
*	binding �ȴ�.
* - ������ fd�� return�ȴ�.
------------------------------------------------------------------------------*/
int socklib_initTcpBind (int port)
{
	int		fd;
#	ifdef TRU64
	int		sockopt;
#endif
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
	if (socklib_setNonBlockMode(fd, 1) < 0) {
		close(fd);
		return -1;
	}

	/*
	**  for cluster to use local physical IP address 
	*/
#	ifdef TRU64
	sockopt=1;
	if ( setsockopt (fd, SOL_SOCKET, SO_CLUA_IN_NOALIAS,&sockopt, sizeof(sockopt) ) < 0 ){
		fprintf (stderr,"[socklib_initTcpBind] ERROR setsockopt SO_CLUA_IN_NOALIAS errno=%d(%s)\n", errno, strerror(errno) );
	}
#	endif

	/* bind fd
	*/
	memset ((void*)&myAddr, 0, sizeof(myAddr));
	myAddr.sin_family = AF_INET;
	myAddr.sin_port = htons(port);
	myAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(fd, (struct sockaddr*)&myAddr, sizeof(myAddr)) < 0 ) {
		fprintf(stderr,"[socklib_initTcpBind] bind fail; errno=%d(%s)\n",errno,strerror(errno));
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

	/* bindSockFdTbl�� port�� fd�� �����Ѵ�.
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
* ������ ipAddr,port�� socket fd�� �����Ͽ� �����Ѵ�.
* - ������ fd�� read/except fd_set�� �߰��Ѵ�.
* - ������ fd�� return�ȴ�.
------------------------------------------------------------------------------*/
int socklib_connect (char *ipAddr, int port)
{
	int		fd;
#	ifdef TRU64
	int		sockopt;
#endif
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

	/*
	**  for cluster to use local physical IP address 
	*/
#	ifdef TRU64
	sockopt=1;
	if ( setsockopt (fd, SOL_SOCKET, SO_CLUA_IN_NOALIAS,&sockopt, sizeof(sockopt) ) < 0 ){
		fprintf (stderr,"[socklib_initTcpBind] ERROR setsockopt SO_CLUA_IN_NOALIAS errno=%d(%s)\n", errno, strerror(errno) );
	}
#	endif

	/* connect to server
	*/
	memset ((void*)&dstAddr, 0, sizeof(dstAddr));
	dstAddr.sin_family = AF_INET;
	dstAddr.sin_port = htons(port);
	dstAddr.sin_addr.s_addr = inet_addr(ipAddr);

	signal (SIGALRM, socklib_dumHdlr);
	ualarm(100000,0); /* multi-thread�� ��� ������ �ɼ� �ִ�.*/
	if (connect(fd, (struct sockaddr*)&dstAddr, sizeof(dstAddr)) < 0 ) {
		ualarm(0,0);
		close(fd);
		sprintf(sockErrBuf,"[socklib_connect] connect fail[%s-%d]; errno=%d(%s)\n",ipAddr,port,errno,strerror(errno));
		//fprintf(SOCKLIB_PRINT_TO,"%s",sockErrBuf);
		return -1;
	}
	ualarm(0,0);
	sprintf(sockErrBuf,"[socklib_connect] connect to [%s-%d]; fd=%d\n", ipAddr, port, fd);
	//fprintf(SOCKLIB_PRINT_TO,"%s",sockErrBuf);

	if (socklib_setNonBlockMode(fd, 1) < 0) {
		close(fd);
		return -1;
	}

	/* serverSockFdTbl�� address, port, fd�� �����Ѵ�.
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

int socklib_connect2 (char *myip, char *ipAddr, int port)
{
	int		fd;
#	ifdef TRU64
	int		sockopt;
#endif
	struct sockaddr_in	dstAddr, myAddr;

	if (!strcmp(ipAddr,""))
		return -1;

	/* create socket fd
	*/
	if ((fd = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr,"[socklib_connect] socket fail; errno=%d(%s)\n",errno,strerror(errno));
		return -1;
	}

	memset ((void*)&myAddr, 0, sizeof(myAddr));
	myAddr.sin_family 	= AF_INET;
	myAddr.sin_port 	= htons(0);
	myAddr.sin_addr.s_addr = inet_addr(myip);

	if (bind(fd, (struct sockaddr*)&myAddr, sizeof(myAddr)) < 0 ) {
		fprintf(stderr,"[socklib_connect2] bind fail; errno=%d(%s)\n",errno,strerror(errno));
		close(fd);
		return -1;
	}
	/*
	*/
	if (socklib_setSockOption(fd) < 0) {
		close(fd);
		return -1;
	}

	/*
	**  for cluster to use local physical IP address 
	*/
#	ifdef TRU64
	sockopt=1;
	if ( setsockopt (fd, SOL_SOCKET, SO_CLUA_IN_NOALIAS,&sockopt, sizeof(sockopt) ) < 0 ){
		fprintf (stderr,"[socklib_initTcpBind] ERROR setsockopt SO_CLUA_IN_NOALIAS errno=%d(%s)\n", errno, strerror(errno) );
	}
#	endif

	/* connect to server
	*/
	memset ((void*)&dstAddr, 0, sizeof(dstAddr));
	dstAddr.sin_family = AF_INET;
	dstAddr.sin_port = htons(port);
	dstAddr.sin_addr.s_addr = inet_addr(ipAddr);

	signal (SIGALRM, socklib_dumHdlr);
	ualarm(100000,0); /* multi-thread�� ��� ������ �ɼ� �ִ�.*/
	if (connect(fd, (struct sockaddr*)&dstAddr, sizeof(dstAddr)) < 0 ) {
		ualarm(0,0);
		close(fd);
		sprintf(sockErrBuf,"[socklib_connect] connect fail[%s-%d]; errno=%d(%s)\n",ipAddr,port,errno,strerror(errno));
		//fprintf(SOCKLIB_PRINT_TO,"%s",sockErrBuf);
		return -1;
	}
	ualarm(0,0);
	sprintf(sockErrBuf,"[socklib_connect] connect to [%s-%d]; fd=%d\n", ipAddr, port, fd);
	//fprintf(SOCKLIB_PRINT_TO,"%s",sockErrBuf);

	if (socklib_setNonBlockMode(fd, 1) < 0) {
		close(fd);
		return -1;
	}

	/* serverSockFdTbl�� address, port, fd�� �����Ѵ�.
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
* ���ο� ���ӿ䱸�� ������ ��� ȣ��Ǿ� accept�ϰ� ������ ������ client_fd�� return
* - allowClientAddrTbl�� ��ϵ� address���� ���ӵ� ��츸 ����Ѵ�.
------------------------------------------------------------------------------*/
int socklib_acceptNewConnection (int srvPort, int srvFd)
{
#ifdef RESTRICTED_CLIENT
	int		i;
#endif
	int		fd, len;
	struct sockaddr_in	cliAddr;

	len = sizeof(cliAddr);
	memset ((void*)&cliAddr, 0, sizeof(cliAddr));

	if ((fd = accept(srvFd, (struct sockaddr*)&cliAddr, &len)) < 0) {
		fprintf(stderr,"[socklib_acceptNewConnection] accept fail; errno=%d(%s)\n",errno,strerror(errno));
		return -1;
	}

#ifdef RESTRICTED_CLIENT
	/* allowClientAddrTbl�� ��ϵ� address���� ��û�Ǿ����� Ȯ���Ѵ�.
	*/
	for (i=0; i<SOCKLIB_MAX_CLIENT_CNT; i++) {
		if (cliAddr.sin_addr.s_addr == allowClientAddrTbl[i])
			break;
	}
	if (i >= SOCKLIB_MAX_CLIENT_CNT) {
		sprintf(sockErrBuf,"[socklib_acceptNewConnection] isn't allow address(%s)\n", inet_ntoa(cliAddr.sin_addr));
		fprintf(SOCKLIB_PRINT_TO,"%s",sockErrBuf);
		close(fd);
		return -1;
	}
#endif /*RESTRICTED_CLIENT*/

	if (socklib_setSockOption(fd) < 0) {
		fprintf(stderr,"[socklib_acceptNewConnection] socklib_setSockOption fail\n");
		close(fd);
		return -1;
	}
	if (socklib_setNonBlockMode(fd, 0) < 0) {
		close(fd);
		return -1;
	}

	/* clientSockFdTbl�� address, port, fd�� �����Ѵ�.
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
* ������ fd�� server�� ���ӵ� fd�̸� socklib_disconnectServerFd�� ȣ���ϰ�,
*	client�� ������ fd�̸� socklib_disconnectClientFd�� ȣ���Ͽ� ������ �����Ѵ�.
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

	sprintf(sockErrBuf,"[socklib_disconnectSockFd] not found fd \n");
	//fprintf(SOCKLIB_PRINT_TO,"%s",sockErrBuf);
	return -1;

} /** End of socklib_disconnectSockFd **/



/*------------------------------------------------------------------------------
* - �ش� fd�� close�ϰ�,
* - serverSockFdTbl���� �����ϰ�,
* - read/except fd_set���� �����Ѵ�.
------------------------------------------------------------------------------*/
int socklib_disconnectServerFd (int fd)
{
	close(fd);

	/* serverSockFdTbl���� fd�� ���� ������ �����Ѵ�.
	*/
	socklib_delServerSockFdTbl (fd);

	/* delete socket fd from readSockFdSet, exceptSockFdSet
	*/
	socklib_delSockFdSet(fd);

	sprintf(sockErrBuf,"[socklib_disconnectServerFd] disconnect fd=%d\n", fd);
	//fprintf(SOCKLIB_PRINT_TO,"%s",sockErrBuf);
	return 1;

} /** End of socklib_disconnectServerFd **/



/*------------------------------------------------------------------------------
* - �ش� fd�� close�ϰ�,
* - clientSockFdTbl���� �����ϰ�,
* - read/except fd_set���� �����Ѵ�.
------------------------------------------------------------------------------*/
int socklib_disconnectClientFd (int fd)
{
	close(fd);

	/* clientSockFdTbl���� fd�� ���� ������ �����Ѵ�.
	*/
	socklib_delClientSockFdTbl (fd);

	/* delete socket fd from readSockFdSet, exceptSockFdSet
	*/
	socklib_delSockFdSet(fd);

	sprintf(sockErrBuf,"[socklib_disconnectClientFd] disconnect fd=%d\n", fd);
	//fprintf(SOCKLIB_PRINT_TO,"%s",sockErrBuf);
	return 1;

} /** End of socklib_disconnectClientFd **/



/*------------------------------------------------------------------------------
* ��ϵ� fd���� Ȯ���Ͽ� event �߻����θ� Ȯ���Ѵ�.
* - event�� ������ NO_EVENT�� return�Ѵ�.
* - binding port���� �߻��� ����̸� ���ο� client�� ������ �䱸�� ����̹Ƿ�
*	���ο� client_fd�� �����Ͽ� accept�� �� actFd�� ���ο� client_fd�� �ѱ��,
*	NEW_CONNECTION�� return�Ѵ�.
* - client_fd �Ǵ� server_fd���� �߻��� ����̸� �޽����� ������ ����̰ų� ������
*	������ ����̴�.
*	-> ������ ������ ��� actFd�� disconnect�� fd�� �ѱ��, DISCONNECTED�� return�Ѵ�.
*	-> �޽����� ������ ��� actFd�� �޽����� ������ fd�� �ѱ��, buff�� ���ŵ�
*		�޽����� ����ϰ�, SOCKLIB_MSG_RECEIVED�� return�Ѵ�.
* - ��Ÿ ���� error�� ��� SOCKLIB_INTERNAL_ERROR�� return�Ѵ�.
------------------------------------------------------------------------------*/
int socklib_action (char *buff, int *actFd)
{
	int		port,fd,newFd;
	char	remoteAddr[32];

	/* readSockFdSet/exceptSockFdSet�� Ȯ���Ͽ� event �߻� ���θ� Ȯ���Ѵ�.
	*/
	if (socklib_pollFdSet() <= 0)
		return SOCKLIB_NO_EVENT;

	/* binding port���� event�� �����Ǿ����� Ȯ���Ѵ�.
	** - binding port ��ȣ�� binding port�� socket fd�� return�ȴ�.
	*/
	if (socklib_lookupBindSockFdTbl (&port, &fd) > 0) {
		/* ���ο� client_fd�� �����Ѵ�.
		*/
		if ((newFd = socklib_acceptNewConnection (port, fd)) < 0) {
			return SOCKLIB_INTERNAL_ERROR;
		}
		*actFd = newFd;
		return SOCKLIB_NEW_CONNECTION;
	}

	/* server�� ����� fd���� event�� �����Ǿ����� Ȯ���Ѵ�.
	*/
	if ((fd = socklib_lookupServerSockFdTbl (remoteAddr)) > 0) {
		/* ���� �޽����� �ְų� disconnect�� ����ε�, read�� �õ��ؼ� ��������
		**	�޽����� �����ϰ�, read fail�̸� disconnect�� ���̴�.
		*/
		*actFd = fd;
		if (socklib_readSockFd (buff, fd, remoteAddr) < 0) {
			sprintf(sockErrBuf,"[socklib_action] to server socklib_readSockFd fail(fd=%d); remoteAddr=%s\n", fd, remoteAddr);
			fprintf(SOCKLIB_PRINT_TO,"%s",sockErrBuf);
			trclib_writeLogErr(__FILE__, __LINE__,sockErrBuf);
			/* read fail�̸� ������ ������ ������ �Ǵ��Ͽ� disconnect�Ѵ�.
			*/
			socklib_disconnectServerFd (fd);
			return SOCKLIB_SERVER_DISCONNECTED;
		}
		return SOCKLIB_SERVER_MSG_RECEIVED;
	}

	/* client_fd���� event�� �����Ǿ����� Ȯ���Ѵ�.
	*/
	if ((fd = socklib_lookupClientSockFdTbl (remoteAddr)) > 0) {
		/* ���� �޽����� �ְų� disconnect�� ����ε�, read�� �õ��ؼ� ��������
		**	�޽����� �����ϰ�, read fail�̸� disconnect�� ���̴�.
		*/
		*actFd = fd;
		if (socklib_readSockFd (buff, fd, remoteAddr) < 0) {
			sprintf(sockErrBuf,"[socklib_action] from client socklib_readSockFd fail(fd=%d); remoteAddr=%s\n", fd, remoteAddr);
			fprintf(SOCKLIB_PRINT_TO,"%s",sockErrBuf);
			trclib_writeLogErr(__FILE__, __LINE__,sockErrBuf);
			/* read fail�̸� ������ ������ ������ �Ǵ��Ͽ� disconnect�Ѵ�.
			*/
			socklib_disconnectClientFd (fd);
			return SOCKLIB_CLIENT_DISCONNECTED;
		}
		return SOCKLIB_CLIENT_MSG_RECEIVED;
	}

	return SOCKLIB_INTERNAL_ERROR;

} /** End of socklib_action **/



/*------------------------------------------------------------------------------
* event�� ������ fd���� �޽����� read�Ͽ� buff�� �ִ´�.
* - header�� ũ�⸸ŭ ���� �а� header�� �ִ� length field�� ���� ������ body��
*	�д´�.
------------------------------------------------------------------------------*/
int socklib_readSockFd (char *buff, int fd, char *remoteAddr)
{
#ifdef aaaaaa
	char	tmp[32];
#endif
	int		i, len, rLen, bodyLen;
	char	*ptr;
	SockLibHeadType	*head;
	struct timeval	waitTmr;

	waitTmr.tv_sec  = 0;
	waitTmr.tv_usec = SOCKLIB_POLL_TIMER;

	ptr = buff;

	/* header �κи� ���� �д´�.
	*/
	for (i=0, rLen=0;
		(i<SOCKLIB_RETRY_CNT) && (rLen < SOCKLIB_HEAD_LEN);
		i++)
	{
		len = read(fd, ptr, SOCKLIB_HEAD_LEN-rLen);

		if (len == 0) {
			sprintf(sockErrBuf,"[socklib_readSockFd] read fail(head,fd=%d,rmt=%s); errno=%d(%s); try=%d, tot=%d, rLen=%d, rem=%d\n",
					fd, remoteAddr, errno, strerror(errno), i+1, SOCKLIB_HEAD_LEN, rLen, (SOCKLIB_HEAD_LEN-rLen));
			fprintf(SOCKLIB_PRINT_TO,"%s",sockErrBuf);
			return -1;
		} else if (len < 0) {
			if (errno==EAGAIN || errno==EINTR) {
				sprintf(sockErrBuf,"[socklib_readSockFd] read would be blocked(head,fd=%d,rmt=%s); errno=%d(%s); try=%d, tot=%d, rLen=%d, rem=%d\n",
						fd, remoteAddr, errno, strerror(errno), i+1, SOCKLIB_HEAD_LEN, rLen, (SOCKLIB_HEAD_LEN-rLen));
				fprintf(SOCKLIB_PRINT_TO,"%s",sockErrBuf);
				select (0,0,0,0,&waitTmr);
				continue;
			}
			sprintf(sockErrBuf,"[socklib_readSockFd] read fail(head,fd=%d,rmt=%s); errno=%d(%s); try=%d, tot=%d, rLen=%d, rem=%d\n",
					fd, remoteAddr, errno, strerror(errno), i+1, SOCKLIB_HEAD_LEN, rLen, (SOCKLIB_HEAD_LEN-rLen));
			fprintf(SOCKLIB_PRINT_TO,"%s",sockErrBuf);
			return -1;
		}

		ptr += len;
		rLen += len;
	}
	if (i==SOCKLIB_RETRY_CNT && rLen < SOCKLIB_HEAD_LEN) {
		/* header length��ŭ �о�� ���� ��� garbage�����Ͱ� ��� �ִ� ������
		**	������ �� �ִµ�, �� ��� -1�� return�Ͽ� disconnect�ϵ��� �Ѵ�.
		*/
		sprintf(sockErrBuf,"[socklib_readSockFd] can't read data_head; fd=%d, rmt=%s, tot=%d, rLen=%d, rem=%d\n",
				fd, remoteAddr, SOCKLIB_HEAD_LEN, rLen, (SOCKLIB_HEAD_LEN-rLen));
		fprintf(SOCKLIB_PRINT_TO,"%s",sockErrBuf);
		return -1;
	}

	/* header�� �ִ� length field�� ������.
	*/
	head = (SockLibHeadType*)buff;
#ifdef aaaaaa
	memcpy (tmp, head->bodyLen, sizeof(head->bodyLen));
	tmp[sizeof(head->bodyLen)] = 0;
	bodyLen = atoi(tmp);
#else
	bodyLen = head->bodyLen = ntohl(head->bodyLen);
#endif

	/* bodyLen��ŭ �о��.
	*/
	for (i=0, rLen=0;
		(i<SOCKLIB_RETRY_CNT) && (rLen < bodyLen);
		i++)
	{
		len = read(fd, ptr, bodyLen-rLen);

		if (len == 0) {
			sprintf(sockErrBuf,"[socklib_readSockFd] read fail(body,fd=%d,rmt=%s); errno=%d(%s); try=%d, tot=%d, rLen=%d, rem=%d\n",
					fd, remoteAddr, errno, strerror(errno), i+1, bodyLen, rLen, (bodyLen-rLen));
			fprintf(SOCKLIB_PRINT_TO,"%s",sockErrBuf);
			return -1;
		} else if (len < 0) {
			if (errno==EAGAIN || errno==EINTR) {
				sprintf(sockErrBuf,"[socklib_readSockFd] read would be blocked(body,fd=%d,rmt=%s); errno=%d(%s); try=%d, tot=%d, rLen=%d, rem=%d\n",
						fd, remoteAddr, errno, strerror(errno), i+1, bodyLen, rLen, (bodyLen-rLen));
				fprintf(SOCKLIB_PRINT_TO,"%s",sockErrBuf);
				select (0,0,0,0,&waitTmr);
				continue;
			}
			sprintf(sockErrBuf,"[socklib_readSockFd] read fail(body,fd=%d,rmt=%s); errno=%d(%s); try=%d, tot=%d, rLen=%d, rem=%d\n",
					fd, remoteAddr, errno, strerror(errno), i+1, bodyLen, rLen, (bodyLen-rLen));
			fprintf(SOCKLIB_PRINT_TO,"%s",sockErrBuf);
			return -1;
		}

		ptr += len;
		rLen += len;
	}
	if (i==SOCKLIB_RETRY_CNT && rLen < bodyLen) {
		/* body length��ŭ �о�� ���� ��� garbage�����Ͱ� ��� �ִ� ������
		**	������ �� �ִµ�, �� ��� -1�� return�Ͽ� disconnect�ϵ��� �Ѵ�.
		*/
		sprintf(sockErrBuf,"[socklib_readSockFd] can't read data_body; fd=%d, rmt=%s, tot=%d, rLen=%d, rem=%d\n",
				fd, remoteAddr, bodyLen, rLen, (bodyLen-rLen));
		fprintf(SOCKLIB_PRINT_TO,"%s",sockErrBuf);
		return -1;
	}

	// �ǳ��� NULL�� ä���.
	*ptr = 0;

#ifdef DEBUG
	//fprintf(stdout,"[socklib_readSockFd] read len=%d\n", rLen+SOCKLIB_HEAD_LEN);
#endif
	return (rLen+SOCKLIB_HEAD_LEN);

} /** End of socklib_readSockFd **/



/*------------------------------------------------------------------------------
* ������ fd�� �����͸� length��ŭ write�Ѵ�.
* - head.bodyLen�� network byte order�� �ٲ۴�.
* - write�ϱ����� write �������� Ȯ���Ѵ�.
*	- select�� Ȯ���ϴµ� select fail �� fd�� ������ close�Ѵ�.
------------------------------------------------------------------------------*/
int socklib_sndMsg (int fd, char *buff, int buffLen)
{
	int		i,len,wLen,ret;
	struct timeval	pollTmr;
	fd_set	wFdSet;
	char	*ptr;

	pollTmr.tv_sec  = 0;
	pollTmr.tv_usec = SOCKLIB_POLL_TIMER;

	FD_ZERO(&wFdSet);
	FD_SET (fd, &wFdSet);

	ptr = buff;

	for (i=0, wLen=0;
		(i<SOCKLIB_RETRY_CNT) && (wLen < buffLen);
		i++)
	{
		/* write�� �� �ִ��� Ȯ���Ѵ�.
		*/
		ret = select (fd+1, NULL, &wFdSet, NULL, &pollTmr);
		if (ret == 0) {
			sprintf(sockErrBuf,"[socklib_sndMsg] (select=0)write would be blocked; try=%d, tot=%d, wLen=%d, rem=%d\n",
					i+1, buffLen, wLen, (buffLen-wLen));
			fprintf(SOCKLIB_PRINT_TO,"%s",sockErrBuf);
			continue;
		} else if (ret < 0){
			sprintf(sockErrBuf,"[socklib_sndMsg] select fail; errno=%d(%s), try=%d, tot=%d, wLen=%d, rem=%d\n",
					errno, strerror(errno), i+1, buffLen, wLen, (buffLen-wLen));
			fprintf(SOCKLIB_PRINT_TO,"%s",sockErrBuf);
			trclib_writeLogErr(__FILE__, __LINE__,sockErrBuf);
			/* serverSockFdTbl, clientSockFdTbl�� �˻��� server�� ���ӵ� fd����,
			**	client�� ������ fd���� ã�� �� ������ ���´�.
			*/
			socklib_disconnectSockFd (fd);
			return -1;
		}

		/* �ش� fd�� data�� write�Ѵ�.
		*/
		len = write(fd, ptr, buffLen-wLen);
		if (len < 0) {
			if (errno==EAGAIN || errno==EINTR) {
				sprintf(sockErrBuf,"[socklib_sndMsg] write would be blocked; errno=%d(%s), try=%d, tot=%d, wLen=%d, rem=%d\n",
						errno, strerror(errno), i+1, buffLen, wLen, (buffLen-wLen));
				fprintf(SOCKLIB_PRINT_TO,"%s",sockErrBuf);
				select (0,0,0,0,&pollTmr);
				continue;
			}
			/* write fail�̸� ������ ������ ���´�.
			*/
			sprintf(sockErrBuf,"[socklib_sndMsg] write fail; errno=%d(%s), try=%d, tot=%d, wLen=%d, rem=%d\n",
					errno, strerror(errno), i+1, buffLen, wLen, (buffLen-wLen));
			fprintf(SOCKLIB_PRINT_TO,"%s",sockErrBuf);
			trclib_writeLogErr(__FILE__, __LINE__,sockErrBuf);
			socklib_disconnectSockFd (fd);
			return -1;
		}
		ptr += len;
		wLen += len;

		if (wLen < buffLen) {
			sprintf(sockErrBuf,"[socklib_sndMsg] writing...; try=%d, tot=%d, wLen=%d, rem=%d\n",
					i+1, buffLen, wLen, (buffLen-wLen));
			fprintf(SOCKLIB_PRINT_TO,"%s",sockErrBuf);
		}
	}
	//if (i==SOCKLIB_RETRY_CNT && wLen < buffLen)
	if (wLen < buffLen)
	{
		/* ������ length��ŭ write���� ���� ��� ������ ������ ���´�.
		*/
		sprintf(sockErrBuf,"[socklib_sndMsg] can't write msg; tot=%d, wLen=%d, rem=%d\n",
				buffLen, wLen, (buffLen-wLen));
		fprintf(SOCKLIB_PRINT_TO,"%s",sockErrBuf);
		trclib_writeLogErr(__FILE__, __LINE__,sockErrBuf);
		socklib_disconnectSockFd (fd);
		return -1;
	}

#ifdef DEBUG
	//fprintf(stdout,"[socklib_sndMsg] write msg len = %d, fd=%d\n", wLen, fd);
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
* server�� ����� ��� fd�� �޽����� write�Ѵ�.
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
* client�� ������ ��� fd�� �޽����� write�Ѵ�.
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
#ifdef TCPNODELAY
	struct protoent	*proto;
#endif

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
int socklib_setNonBlockMode (int fd, int onoff)
{
	int		flag;

	/* set Non-Blocking Mode
	*/
	if ((flag = fcntl(fd, F_GETFL, 0)) < 0) {
		fprintf(stderr,"[socklib_setNonBlockMode] fcntl(F_GETFL) fail; errno=%d(%s)\n",errno,strerror(errno));
		return -1;
	}
	if (onoff)
		flag |= O_NONBLOCK;
	else
		flag &= ~O_NONBLOCK;
	if (fcntl(fd, F_SETFL, flag) < 0) {
		fprintf(stderr,"[socklib_setNonBlockMode] fcntl(F_SETFL) fail; errno=%d(%s)\n",errno,strerror(errno));
		return -1;
	}

	return 1;

} /** End of socklib_setNonBlockMode **/



/*------------------------------------------------------------------------------
* bindSockFdTbl�� binding port�� socket_fd�� �����Ѵ�.
* - event ������ bindSockFdTbl�� ��ϵ� fd���� event�� �߻��� ����̸�
*	client�κ����� ���ο� connection �䱸�� ���ŵǾ����� �� �� �ִ�.
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
* serverSockFdTbl�� server�� connect�� fd�� server�� address, port�� �����Ѵ�.
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
* clientSockFdTbl�� client_fd�� client�� address�� �����Ѵ�.
* - �ش� client�� � binding port�� �����ߴ� ������ ������ �� �ֵ��� binding port
*	��ȣ�� �Բ� �����Ѵ�.
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
* serverSockFdTbl���� fd�� ���� ������ �����Ѵ�.
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
		sprintf(sockErrBuf,"[socklib_delServerSockFdTbl] not found fd in serverSockFdTbl \n");
		fprintf(SOCKLIB_PRINT_TO,"%s",sockErrBuf);
		return -1;
	}

	return i;

} /** End of socklib_delServerSockFdTbl **/



/*------------------------------------------------------------------------------
* clientSockFdTbl���� fd�� ���� ������ �����Ѵ�.
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
		sprintf(sockErrBuf,"[socklib_delClientSockFdTbl] not found fd in clientSockFdTbl \n");
		fprintf(SOCKLIB_PRINT_TO,"%s",sockErrBuf);
		return -1;
	}

	return i;

} /** End of socklib_delClientSockFdTbl **/



/*------------------------------------------------------------------------------
* read/except fd_set�� �߰��Ѵ�.
* - ���ο� fd�� �����Ǹ� readSockFdSet/exceptSockFdSet�� setting�ϰ�,
*	�̸� Ȯ���ϸ� event�� ������ �� �ִ�.
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
* read/except fd_set���� �ش� fd�� �����Ѵ�.
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
* readSockFdSet/exceptSockFdSet�� Ȯ���Ͽ� event �߻� ���θ� Ȯ���Ѵ�.
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
* event�� ������ fd�� binding port���� Ȯ���Ѵ�.
* - � binding port���� �����Ǿ����� �ش� port��ȣ�� port�� socket fd�� return�Ѵ�.
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
* event�� ������ fd�� server�� ����� fd���� Ȯ���Ѵ�.
------------------------------------------------------------------------------*/
int socklib_lookupServerSockFdTbl (char *remoteAddr)
{
	int		i;

	for (i=0; i<SOCKLIB_MAX_SERVER_CNT; i++) {
		if (!serverSockFdTbl[i].fd)
			continue;
		if (FD_ISSET(serverSockFdTbl[i].fd, &rdSockFdSet)) {
			sprintf (remoteAddr, "%s", inet_ntoa(serverSockFdTbl[i].srvAddr.sin_addr));
			return serverSockFdTbl[i].fd;
		}
		if (FD_ISSET(serverSockFdTbl[i].fd, &exSockFdSet)) {
			sprintf (remoteAddr, "%s", inet_ntoa(serverSockFdTbl[i].srvAddr.sin_addr));
			return serverSockFdTbl[i].fd;
		}
	}
	return -1;

} /** End of socklib_lookupServerSockFdTbl **/



/*------------------------------------------------------------------------------
* event�� ������ fd�� client_fd���� Ȯ���Ѵ�.
------------------------------------------------------------------------------*/
int socklib_lookupClientSockFdTbl (char *remoteAddr)
{
	int		i;

	for (i=0; i<SOCKLIB_MAX_CLIENT_CNT; i++) {
		if (!clientSockFdTbl[i].fd)
			continue;
		if (FD_ISSET(clientSockFdTbl[i].fd, &rdSockFdSet)) {
			sprintf (remoteAddr, "%s", inet_ntoa(clientSockFdTbl[i].cliAddr.sin_addr));
			return clientSockFdTbl[i].fd;
		}
		if (FD_ISSET(clientSockFdTbl[i].fd, &exSockFdSet)) {
			sprintf (remoteAddr, "%s", inet_ntoa(clientSockFdTbl[i].cliAddr.sin_addr));
			return clientSockFdTbl[i].fd;
		}
	}
	return -1;

} /** End of socklib_lookupClientSockFdTbl **/



/*------------------------------------------------------------------------------
* well-known port�� tcp ������ �õ������ν� ping test����� �����Ѵ�.
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
