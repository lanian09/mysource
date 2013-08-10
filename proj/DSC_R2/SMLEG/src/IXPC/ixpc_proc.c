#include "ixpc_proto.h"

extern IXPC_MsgQRoutTable	msgQRoutTbl[SYSCONF_MAX_APPL_NUM];
extern IXPC_SockRoutTable	sockRoutTbl[SYSCONF_MAX_ASSO_SYS_NUM];
extern int					ixpcPortNum;
extern char					mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN];
extern char					trcBuf[4096], trcTmp[1024];
extern ClientSockFdContext	clientSockFdTbl[SOCKLIB_MAX_CLIENT_CNT];
extern time_t				currentTime, lastPingTestTime;
extern int					trcFlag, trcLogFlag;
extern IxpcConSts			*ixpcCON;


/*------------------------------------------------------------------------------
* remote ixpc가 접속해온 경우 호출된다.
* - socklib에서 관리하는 clientSockFdTbl을 검색하여 ip address를 찾아내고, 이를 key값으로
*	sockRoutTbl을 다시 검색하여 어디에서 접속되었는지 알아낸다.
------------------------------------------------------------------------------*/
int ixpc_newConnEvent (int fd)
{
	int		i,j;
	char	remoteAddr[32];

	/* socklib에서 관리하는 clientSockFdTbl을 검색한다.
	*/
	for (i=0; i<SOCKLIB_MAX_CLIENT_CNT; i++) {
		if (clientSockFdTbl[i].fd == fd) {
			break;
		}
	}
	if (i >= SOCKLIB_MAX_CLIENT_CNT) {
		sprintf(trcBuf,"[ixpc_newConnEvent] not found fd[%d](rx_port) in clientSockFdTbl;\n", fd);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	/* 접속해온 remote 시스템의 ip address를 얻는다.
	*/
	sprintf (remoteAddr, "%s", inet_ntoa(clientSockFdTbl[i].cliAddr.sin_addr));

	/* remote system name과 일치하는 곳에 fd(수신port)를 저장한다.
	*/
	for (i=0; i<SYSCONF_MAX_ASSO_SYS_NUM; i++)
	{
		if (!sockRoutTbl[i].pres)
			continue;
		if (!strcmp(remoteAddr, sockRoutTbl[i].ipAddrPri) ||
			!strcmp(remoteAddr, sockRoutTbl[i].ipAddrSec)) {
			/* LAN cable 장애 등으로 인한 network 장애발생시 장애감지 event가 real-time으로 발생하지 않는 경우가 있다.
			*  rx_port가 끊어지지 않은 상태에서 같은곳에서 새로운 접속이 생긴 경우에는
			*  network 장애로 인한 절체가 일어난 것으로 판단하여 기존 connection을 끊어야 한다.
			*/
			if (sockRoutTbl[i].sockRxFd) {
				sprintf(trcBuf,">>> already established rx_port(may be change-over); remote=%s; close old_fd(%d)\n", sockRoutTbl[i].sysName, sockRoutTbl[i].sockRxFd);
				trclib_writeLogErr (FL,trcBuf);
				socklib_disconnectSockFd (sockRoutTbl[i].sockRxFd); /* close old rx_port */
				sockRoutTbl[i].sockRxFd = 0; /* clear rx_port */
				if (sockRoutTbl[i].sockTxFd) {
					sprintf(trcBuf,"      force disconnect tx_port; close fd(%d)\n", sockRoutTbl[i].sockTxFd);
					trclib_writeLogErr (FL,trcBuf);
					for (j=0; j<SYSCONF_MAX_GROUP_MEMBER; j++){
						if ( ixpcCON->ixpcCon[j].name[0] == 0 ) continue;
						fprintf (stderr," %s disconnect\n", sockRoutTbl[i].sysName );
						if ( !strcasecmp ( sockRoutTbl[i].sysName,ixpcCON->ixpcCon[j].name ) ){
							ixpcCON->ixpcCon[j].connSts = SFM_LAN_DISCONNECTED;
							break;
						}
					}
					socklib_disconnectSockFd (sockRoutTbl[i].sockTxFd); /* disconnect old tx_port */
					sockRoutTbl[i].sockTxFd = 0; /* clear tx_port */
				}
			}
			sockRoutTbl[i].sockRxFd = fd;
			sockRoutTbl[i].lastRxTime = currentTime;
			sprintf(trcBuf,"[ixpc_newConnEvent] accept rx_port; remote=%s(%s)(fd=%d)\n", sockRoutTbl[i].sysName, remoteAddr, fd);
			trclib_writeLogErr (FL,trcBuf);
			return 1;
		}
	}

	/* 등록되지 않은 시스템에서 접속된 경우 강제로 끊는다.
	*/
	socklib_disconnectSockFd (fd);
	sprintf(trcBuf,"[ixpc_newConnEvent] accept rx_port; unknown ipAddr=%s,fd=%d\n", remoteAddr, fd);
	trclib_writeLogErr (FL,trcBuf);

	return -1;

} /** End of ixpc_newConnEvent **/



/*------------------------------------------------------------------------------
* 수신 port에서 remote로부터 메시지를 수신한 경우 호출된다.
* - 주기적인 connection check를 위해 lastRxTime을 update하고,
* - 내부 routing에 필요한 SockLibMsgType의 body 부분만 잘라내 ixpc_intRoute()를
*	호출한다. -> socket으로 전달할때 application에서 수신한 메시지를 body에 그대로
*	복사해서 전달하므로...
* - checktion check 메시지는 ixpc_head에 destination이 ixpc 자기 자신이 들어있고,
*	body는 없다. -> 
------------------------------------------------------------------------------*/
int ixpc_recvEventRxPort (int fd, SockLibMsgType *rxSockMsg)
{
	int		i,sockBodyLen;
	GeneralQMsgType	rxGenQMsg;
	IxpcQMsgType	*rxIxpcQMsg;

	// 어느 port에서 메시지가 수신되었는지 찾는다.
	for (i=0; i<SYSCONF_MAX_ASSO_SYS_NUM; i++)
	{
		if (sockRoutTbl[i].pres && sockRoutTbl[i].sockRxFd == fd) {
			sockRoutTbl[i].lastRxTime = currentTime; /* update last received time */
			break;
		}
	}
	if (i >= SYSCONF_MAX_ASSO_SYS_NUM) {
		socklib_disconnectSockFd (fd); /* disconnect */
		sprintf(trcBuf,"[ixpc_recvEventRxPort] msg received rx_port; unknown fd=%d\n", fd);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}


	/* socket으로 받은 메시지를 msgQ 메시지로 변환한다.
	* 외부 시스템과 통신하므로 byte ordering 문제가 발생할 수 있다.
	* - IXPC에서는 socket_header와 ixpc_header의 field 중 short/int를 송신 시 network
	*	byte로 변환하고, 수신 시 host byte로 다시 변환한다.
	* - 나머지 body 부분은 Application에서 직접해야 한다.
	* 송신측에서 msgQ의 mtype을 socket 메시지 header에 4byte로 변환하여 보냈으므로,
	*	이을 다시 꺼내 long형의 mtype으로 msgQ 메시지에 다시 변환하여 넣는다.
	*/
	sockBodyLen = rxSockMsg->head.bodyLen;
	rxGenQMsg.mtype = (long)ntohl(rxSockMsg->head.mtype);
	memcpy ((void*)rxGenQMsg.body, rxSockMsg->body, sockBodyLen);

	rxIxpcQMsg = (IxpcQMsgType*)rxGenQMsg.body;
	rxIxpcQMsg->head.msgId   = ntohl(rxIxpcQMsg->head.msgId);
	rxIxpcQMsg->head.bodyLen = ntohs(rxIxpcQMsg->head.bodyLen);

    /* 자신에게 전달되는 메시지인지 확인하여 자신이 처리해야 하는 메시지이면
    *   ixpc_exeMyMsg()가 호출된다.
    * - 자신이 처리할 메시지인 경우 1이 return된다.
    */
	if (ixpc_isMyMsg((GeneralQMsgType*)&rxGenQMsg)) {
		return 1;
	}

	return (ixpc_intRoute((GeneralQMsgType*)&rxGenQMsg));

} /** End of ixpc_recvEventRxPort **/



/*------------------------------------------------------------------------------
* remote ixpc로  접속한 송신 port로 메시지가 들어온 경우 호출된다.
* - 자신이 송신 port로 일정시간동안 메시지를 보내지 않은 경우, remote에서 connection
*	check를 위해 dummy 메시지를 보내온 경우이다. 
* - 그러나, 송신 port에 대해서 일정시간동안 메시지를 보내지 않았으면 자신도 connection
*	check 메시지를 보낸다.
* - 수신 port에 대한 connection check timer 보다 송신 port에 대한 connection check
*	timer가 짧기 때문에, remote의 ixpc가 동작중이면 자신의 수신 port로 check 메시지가
*	먼저 수신된다.
* - 즉, 정상적인 경우 자신의 송신 port로 check 메시지가 들어오지 않는다.
------------------------------------------------------------------------------*/
int ixpc_recvEventTxPort (int fd)
{
	int		i;

	for (i=0; i<SYSCONF_MAX_ASSO_SYS_NUM; i++)
	{
		if (sockRoutTbl[i].pres && sockRoutTbl[i].sockTxFd == fd) {
			sprintf(trcBuf,"[ixpc_recvEventTxPort] unexpected msg received tx_port; dst=%s\n", sockRoutTbl[i].sysName);
			trclib_writeLogErr (FL,trcBuf);
			return 1;
		}
	}
	socklib_disconnectSockFd (fd); /* disconnect */
	sprintf(trcBuf,"[ixpc_recvEventTxPort] unexpected msg received tx_port; unknown fd=%d\n", fd);
	trclib_writeLogErr (FL,trcBuf);

	return -1;

} /** End of ixpc_recvEventTxPort **/



/*------------------------------------------------------------------------------
* remote ixpc가 접속해온 socket port가 끊어진 경우 호출된다.
* - network 장애 발생시 rx_port, tx_port에 대한 event가 제대로 감지되지 않는 경우가 있으므로
*   rx_port, tx_port 중 disconnect event가 감지되면 나머지 port도 무조건 해제한다.
------------------------------------------------------------------------------*/
int ixpc_disconnEventRxPort (int fd)
{
	int		i,j;

	for (i=0; i<SYSCONF_MAX_ASSO_SYS_NUM; i++)
	{
		if (sockRoutTbl[i].pres && sockRoutTbl[i].sockRxFd == fd) {
			sockRoutTbl[i].sockRxFd = 0;
			sprintf(trcBuf,"[ixpc_disconnEventRxPort] disconnected rx_port; dst=%s(fd=%d)\n", sockRoutTbl[i].sysName, fd);
			trclib_writeLogErr (FL,trcBuf);
			if (sockRoutTbl[i].sockTxFd) {
				for (j=0; j<SYSCONF_MAX_GROUP_MEMBER; j++){
					if ( ixpcCON->ixpcCon[j].name[0] == 0 ) continue;
					if ( !strcasecmp ( sockRoutTbl[i].sysName,ixpcCON->ixpcCon[j].name ) ){
						ixpcCON->ixpcCon[j].connSts = SFM_LAN_DISCONNECTED;
						break;
					}
				}
				socklib_disconnectSockFd (sockRoutTbl[i].sockTxFd); /* disconnect tx_port also */
				sprintf(trcBuf,"      force disconnect tx_port; close fd(%d)\n", sockRoutTbl[i].sockTxFd);
				trclib_writeLogErr (FL,trcBuf);
				sockRoutTbl[i].sockTxFd = 0; /* clear tx_port */
			}
			return 1;
		}
	}
	socklib_disconnectSockFd (fd); /* disconnect */
	sprintf(trcBuf,"disconnected rx_port; unknown fd=%d\n", fd);
	trclib_writeLogErr (FL,trcBuf);

	return -1;

} /** End of ixpc_disconnEventRxPort **/



/*------------------------------------------------------------------------------
* remote ixpc로 접속한 socket port(송신fd) 끊어진 경우 호출된다.
* - sockRoutTbl을 검색하여 disconnect된 fd가 저장된 곳에 0으로 clear하고
*   disconnect된 시각을 기록한다.
* - 주기적인 재접속 절차에 의해 일정시간 경과 후 재접속을 시도한다.
* - network 장애 발생시 rx_port, tx_port에 대한 event가 제대로 감지되지 않는 경우가 있으므로
*   rx_port, tx_port 중 disconnect event가 감지되면 나머지 port도 무조건 해제한다.
------------------------------------------------------------------------------*/
int ixpc_disconnEventTxPort (int fd)
{
	int		i,j;

	for (i=0; i<SYSCONF_MAX_ASSO_SYS_NUM; i++)
	{
		if (sockRoutTbl[i].pres && sockRoutTbl[i].sockTxFd == fd) {
			sockRoutTbl[i].sockTxFd = 0;
			sockRoutTbl[i].disconnTime = currentTime;
			sprintf(trcBuf,"[ixpc_disconnEventTxPort] disconnected tx_port; dst=%s(fd=%d)\n", sockRoutTbl[i].sysName, fd);
			trclib_writeLogErr (FL,trcBuf);
			if (sockRoutTbl[i].sockRxFd) {
				for (j=0; j<SYSCONF_MAX_GROUP_MEMBER; j++){
					if ( ixpcCON->ixpcCon[j].name[0] == 0 ) continue;
					if ( !strcasecmp ( sockRoutTbl[i].sysName,ixpcCON->ixpcCon[j].name ) ){
						ixpcCON->ixpcCon[j].connSts = SFM_LAN_DISCONNECTED;
						break;
					}
				}
				socklib_disconnectSockFd (sockRoutTbl[i].sockRxFd); /* disconnect rx_port also */
				sprintf(trcBuf,"      force disconnect rx_port; close fd(%d)\n", sockRoutTbl[i].sockRxFd);
				trclib_writeLogErr (FL,trcBuf);
				sockRoutTbl[i].sockRxFd = 0; /* clear rx_port */
			}
			return 1;
		}
	}
	sprintf(trcBuf,"[ixpc_disconnEventTxPort] disconnected tx_port; unknown fd=%d\n", fd);
	trclib_writeLogErr (FL,trcBuf);

	return -1;

} /** End of ixpc_disconnEventTxPort **/



/*------------------------------------------------------------------------------
* destination이 자신인 메시지 수신 시 호출된다.
* - 
------------------------------------------------------------------------------*/
int ixpc_exeMyMsg (GeneralQMsgType *rxGenQMsg)
{
    IxpcQMsgType	*rxIxpcQMsg;

    rxIxpcQMsg = (IxpcQMsgType*)rxGenQMsg->body;

	switch (rxGenQMsg->mtype) {
		case MTYPE_SETPRINT:
			trclib_exeSetPrintMsg ((TrcLibSetPrintMsgType*)rxGenQMsg);
			break;

		case MTYPE_IXPC_CONNECTION_CHECK:
			/* connection check 메시지는 body가 없고, socket port에서 수신하자마자
			*	lastRxTime을 update했으므로 다른 추가적인 동작이 필요없다.
			*/
			if (trcFlag || trcLogFlag) {
				sprintf(trcBuf,"[ixpc_exeMyMsg] received conn_check msg; src=%s-%s\n", rxIxpcQMsg->head.srcSysName, rxIxpcQMsg->head.srcAppName);
				trclib_writeLog (FL,trcBuf);
			}
			break;

		default:
			break;
	}

	return 1;

} /** End of ixpc_exeMyMsg **/



/*------------------------------------------------------------------------------
* 1. sockRoutTbl을 확인하여 아직 접속되지 않은 remote로 접속한다.
* - 재접속을 시도할때 interval을 주기 위해 disconnect된 시각을 기록해 두고,
*	이를 확인하여 일정 시간이 지난 경우에만 재접속을 시도한다.
* - 접속되지 않는 경우 disconnect_time을 현재시각으로 update하여 정해진 interval
*	경과 후 재시도 하도록 한다.
* 2. 일정시간동안 송신 메시지가 없으면 connection check 메시지를 보낸다.
* 3. 일정시간동안 수신 메시지가 없으면 송수신 port를 모두 강제로 끊는다.
* 4. 주기적으로 ping test하여 접속실패시 송수신 port를 모두 강제로 끊는다.
------------------------------------------------------------------------------*/
int ixpc_checkConnections (void)
{
	int		i;
#ifdef DELELTED
	int 	j;
#endif
	for (i=0; i<SYSCONF_MAX_ASSO_SYS_NUM; i++)
	{
		if (!sockRoutTbl[i].pres)
			continue;
		/* 접속되어 있지 않고, 재접속 interval이 지났으면 재접속 시도
		*/
		if ((!sockRoutTbl[i].sockTxFd) &&
			((currentTime - sockRoutTbl[i].disconnTime) > IXPC_RECONNECT_INTERVAL))
		{
            sprintf(trcBuf, "Try to connect to [%s], primary ip [%s], second ip [%s]\n",
                    sockRoutTbl[i].sysName, sockRoutTbl[i].ipAddrPri, sockRoutTbl[i].ipAddrSec);
			trclib_writeLogErr (FL,trcBuf);
			if(ixpc_connect2Remote(i) > 0)
                sprintf(trcBuf, "connection success on [%s], primary ip [%s], second ip [%s]\n",
                        sockRoutTbl[i].sysName, sockRoutTbl[i].ipAddrPri, sockRoutTbl[i].ipAddrSec);
            else
                sprintf(trcBuf, "connection fail on [%s], primary ip [%s], second ip [%s]\n",
                        sockRoutTbl[i].sysName, sockRoutTbl[i].ipAddrPri, sockRoutTbl[i].ipAddrSec);

			trclib_writeLogErr (FL,trcBuf);
		}

#ifdef DELELTED
		/* 일정시간동안 수신 메시지가 없으면 송수신 port를 모두 강제로 끊는다.
		* - 초기 접속 시 remote에서도 IXPC_RECONNECT_INTERVAL을 주기로 재접속을
		*	시도하므로 최대 IXPC_CONN_CHECK_PERIOD + IXPC_RECONNECT_INTERVAL 동안
		*	수신 port에서 메시지를 받지 못할 수 있다.
		*/
		if ((sockRoutTbl[i].sockRxFd) &&
			((currentTime - sockRoutTbl[i].lastRxTime) > IXPC_AUDIT_TIMER))
		{
			socklib_disconnectSockFd (sockRoutTbl[i].sockRxFd); /* disconnect rx_port */
			sockRoutTbl[i].sockRxFd = 0; /* clear rx_port */
			if (sockRoutTbl[i].sockTxFd) {
				for (j=0; j<SYSCONF_MAX_GROUP_MEMBER; j++){
					if ( ixpcCON->ixpcCon[j].name[0] == NULL ) continue;
					if ( !strcasecmp ( sockRoutTbl[i].sysName,ixpcCON->ixpcCon[j].name ) ){
						ixpcCON->ixpcCon[j].connSts = SFM_LAN_DISCONNECTED;
						break;
					}
				}
				socklib_disconnectSockFd (sockRoutTbl[i].sockTxFd); /* disconnect tx_port */
				sockRoutTbl[i].sockTxFd = 0; /* clear tx_port */
			}
			sprintf(trcBuf,">>> not received any msg from %s; force disconnect\n", sockRoutTbl[i].sysName);
			trclib_writeLogErr (FL,trcBuf);
		}
#endif /*DELELTED*/

		/* 일정시간동안 송신 메시지가 없으면 connection check 메시지를 보낸다.
		*/
		if ((sockRoutTbl[i].sockTxFd) &&
			((currentTime - sockRoutTbl[i].lastTxTime) >= IXPC_CONN_CHECK_PERIOD))
		{
			ixpc_txConnCheckMsg(i);
		}

#ifdef DELELTED
		/* 주기적으로 ping test하여 접속실패시 송수신 port를 모두 강제로 끊는다.
		*/
		if ((sockRoutTbl[i].sockTxFd || sockRoutTbl[i].sockRxFd) &&
			((currentTime - lastPingTestTime) >= IXPC_PING_TEST_PERIOD))
		{
			if ((socklib_ping(sockRoutTbl[i].ipAddrPri) < 0) &&
				(socklib_ping(sockRoutTbl[i].ipAddrSec) < 0)) {
				sprintf(trcBuf,"ping_test fail to %s; force disconnect\n", sockRoutTbl[i].sysName);
				trclib_writeLogErr (FL,trcBuf);
				if (sockRoutTbl[i].sockRxFd) {
					socklib_disconnectSockFd (sockRoutTbl[i].sockRxFd); /* disconnect rx_port */
					sockRoutTbl[i].sockRxFd = 0; /* clear rx_port */
				}
				if (sockRoutTbl[i].sockTxFd) {
					for (j=0; j<SYSCONF_MAX_GROUP_MEMBER; j++){
						if ( ixpcCON->ixpcCon[j].name[0] == NULL ) continue;
						if ( !strcasecmp ( sockRoutTbl[i].sysName,ixpcCON->ixpcCon[j].name ) ){
							ixpcCON->ixpcCon[j].connSts = SFM_LAN_DISCONNECTED;
							break;
						}
					}
					socklib_disconnectSockFd (sockRoutTbl[i].sockTxFd); /* disconnect tx_port */
					sockRoutTbl[i].sockTxFd = 0; /* clear tx_port */
				}
			}
			lastPingTestTime = currentTime;
		}
#endif /*DELELTED*/
	}

	return 1;

} /** End of ixpc_checkConnections **/



/*------------------------------------------------------------------------------
* 등록된 primary address로 먼저 접속 시도하고 실패시 secondary로 접속시도한다.
* - 접속되면 송신port를 저장하고, connection check를 위한 lastTxTime에 현재시각을
*	기록한다.
* - 접속실패시 일정시간(IXPC_RECONNECT_INTERVAL) 경과 후 재시도하도록 하기위해
*	disconnTime에 현재시각을 기록한다.
------------------------------------------------------------------------------*/
extern char myip1[BUFSIZ];
extern char myip2[BUFSIZ];

int ixpc_connect2Remote (int index)
{
#if 1 // jjinri
	int		fd,i;

	if ((fd = socklib_connect2 (myip1, sockRoutTbl[index].ipAddrPri, ixpcPortNum)) > 0) {
		sockRoutTbl[index].sockTxFd = fd;
		sockRoutTbl[index].lastTxTime = currentTime;
		for (i=0; i<SYSCONF_MAX_GROUP_MEMBER; i++){
			if ( ixpcCON->ixpcCon[i].name[0] == 0 ) continue;
			if ( !strcasecmp ( sockRoutTbl[index].sysName,ixpcCON->ixpcCon[i].name ) ){
				ixpcCON->ixpcCon[i].connSts = SFM_LAN_CONNECTED;
				break;
			}
		}
		sprintf(trcBuf,"[ixpc_connect2Remote] connect to %s(%s)(fd=%d)\n", sockRoutTbl[index].sysName, sockRoutTbl[index].ipAddrPri, fd);
		trclib_writeLogErr (FL,trcBuf);
		return 1;
	}

	if ((fd = socklib_connect2 (myip2, sockRoutTbl[index].ipAddrSec, ixpcPortNum)) > 0) {
		sockRoutTbl[index].sockTxFd = fd;
		sockRoutTbl[index].lastTxTime = currentTime;
		for (i=0; i<SYSCONF_MAX_GROUP_MEMBER; i++){
			if ( ixpcCON->ixpcCon[i].name[0] == 0 ) continue;
			if ( !strcasecmp ( sockRoutTbl[index].sysName,ixpcCON->ixpcCon[i].name ) ){
				ixpcCON->ixpcCon[i].connSts = SFM_LAN_CONNECTED;
				break;
			}
		}
		sprintf(trcBuf,"[ixpc_connect2Remote] connect to %s(%s)(fd=%d)\n", sockRoutTbl[index].sysName, sockRoutTbl[index].ipAddrSec, fd);
		trclib_writeLogErr (FL,trcBuf);
		return 1;
	}

	/* update disconnect time stamp -> 재접속 interval을 위해
	*/
	sockRoutTbl[index].disconnTime = currentTime;
	if (trcFlag || trcLogFlag) {
		sprintf(trcBuf,"connect fail to %s\n", sockRoutTbl[index].sysName);
		trclib_writeLog (FL,trcBuf);
	}

	return -1;
#endif
} /** End of ixpc_connect2Remote **/



/*------------------------------------------------------------------------------
* 일정시간동안 송신 메시지가 없을때 호출되어 송신 port로 connection check 메시지를
*	보낸다.
* - 이를 수신한 remote의 ixpc는 lastRxTime을 update한다.
------------------------------------------------------------------------------*/
int ixpc_txConnCheckMsg (int index)
{
	int		ret,txLen,j;
	GeneralQMsgType	txGenQMsg;
	IxpcQMsgType	*txIxpcQMsg;
	SockLibMsgType	txSockMsg;

	txIxpcQMsg = (IxpcQMsgType*)txGenQMsg.body;

	strcpy (txIxpcQMsg->head.srcSysName,  mySysName);
	strcpy (txIxpcQMsg->head.srcAppName,  myAppName); /* ixpc */
	strcpy (txIxpcQMsg->head.dstSysName,  sockRoutTbl[index].sysName);
	strcpy (txIxpcQMsg->head.dstAppName,  myAppName); /* ixpc */
	txIxpcQMsg->head.bodyLen = htonl(0);

	txSockMsg.head.bodyLen = htonl(sizeof(txIxpcQMsg->head));
	txSockMsg.head.mtype   = htonl(MTYPE_IXPC_CONNECTION_CHECK);
	memcpy ((void*)txSockMsg.body, txIxpcQMsg, sizeof(txIxpcQMsg->head));

	txLen = sizeof(txIxpcQMsg->head) + sizeof(txSockMsg.head);

	if ((ret = socklib_sndMsg(sockRoutTbl[index].sockTxFd, (char*)&txSockMsg, txLen)) < 0) {
		/* 전송 실패시 해당 fd (송신port)를 clear한다. (실제 disconnect 동작은 socklib에서 이미 수행되었다.)
		*/
		sockRoutTbl[index].sockTxFd = 0; /* clear tx_port */
		for (j=0; j<SYSCONF_MAX_GROUP_MEMBER; j++){
			if ( ixpcCON->ixpcCon[j].name[0] == 0 ) continue;
			if ( !strcasecmp ( sockRoutTbl[index].sysName,ixpcCON->ixpcCon[j].name ) ){
				ixpcCON->ixpcCon[j].connSts = SFM_LAN_DISCONNECTED;
				break;
			}
		}
		sprintf(trcBuf,"conn_check msg send fail to %s(fd=%d)\n", sockRoutTbl[index].sysName, sockRoutTbl[index].sockTxFd);
		trclib_writeLogErr (FL,trcBuf);
	}

	sockRoutTbl[index].lastTxTime = currentTime; /* update last transmitted time */
	if (trcFlag || trcLogFlag) {
		sprintf(trcBuf,"send conn_check msg to %s(fd=%d)\n", sockRoutTbl[index].sysName, sockRoutTbl[index].sockTxFd);
		trclib_writeLog (FL,trcBuf);
	}

	return ret;

} /** End of ixpc_txConnCheckMsg **/




/*------------------------------------------------------------------------------
* dstSysName에 system_name이 들어있지 않고 system_type이 들어 있는지 확인한다.
* - type_name과 system_name이 같을 수도 있으므로 system_name과 다른지 함께 확인한다.
------------------------------------------------------------------------------*/
int ixpc_isSysTypeRoute (char *name)
{
	int	i;

	for (i=0; i<SYSCONF_MAX_ASSO_SYS_NUM; i++) {
		if (sockRoutTbl[i].pres &&
			!strcasecmp (name, sockRoutTbl[i].sysType) &&
			strcasecmp (name, sockRoutTbl[i].sysName))
			return 1;
	}
	return 0;

} /** End of ixpc_isSysTypeRoute **/
