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
* remote ixpc�� �����ؿ� ��� ȣ��ȴ�.
* - socklib���� �����ϴ� clientSockFdTbl�� �˻��Ͽ� ip address�� ã�Ƴ���, �̸� key������
*	sockRoutTbl�� �ٽ� �˻��Ͽ� ��𿡼� ���ӵǾ����� �˾Ƴ���.
------------------------------------------------------------------------------*/
int ixpc_newConnEvent (int fd)
{
	int		i,j;
	char	remoteAddr[32];

	/* socklib���� �����ϴ� clientSockFdTbl�� �˻��Ѵ�.
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

	/* �����ؿ� remote �ý����� ip address�� ��´�.
	*/
	sprintf (remoteAddr, "%s", inet_ntoa(clientSockFdTbl[i].cliAddr.sin_addr));

	/* remote system name�� ��ġ�ϴ� ���� fd(����port)�� �����Ѵ�.
	*/
	for (i=0; i<SYSCONF_MAX_ASSO_SYS_NUM; i++)
	{
		if (!sockRoutTbl[i].pres)
			continue;
		if (!strcmp(remoteAddr, sockRoutTbl[i].ipAddrPri) ||
			!strcmp(remoteAddr, sockRoutTbl[i].ipAddrSec)) {
			/* LAN cable ��� ������ ���� network ��ֹ߻��� ��ְ��� event�� real-time���� �߻����� �ʴ� ��찡 �ִ�.
			*  rx_port�� �������� ���� ���¿��� ���������� ���ο� ������ ���� ��쿡��
			*  network ��ַ� ���� ��ü�� �Ͼ ������ �Ǵ��Ͽ� ���� connection�� ����� �Ѵ�.
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

	/* ��ϵ��� ���� �ý��ۿ��� ���ӵ� ��� ������ ���´�.
	*/
	socklib_disconnectSockFd (fd);
	sprintf(trcBuf,"[ixpc_newConnEvent] accept rx_port; unknown ipAddr=%s,fd=%d\n", remoteAddr, fd);
	trclib_writeLogErr (FL,trcBuf);

	return -1;

} /** End of ixpc_newConnEvent **/



/*------------------------------------------------------------------------------
* ���� port���� remote�κ��� �޽����� ������ ��� ȣ��ȴ�.
* - �ֱ����� connection check�� ���� lastRxTime�� update�ϰ�,
* - ���� routing�� �ʿ��� SockLibMsgType�� body �κи� �߶� ixpc_intRoute()��
*	ȣ���Ѵ�. -> socket���� �����Ҷ� application���� ������ �޽����� body�� �״��
*	�����ؼ� �����ϹǷ�...
* - checktion check �޽����� ixpc_head�� destination�� ixpc �ڱ� �ڽ��� ����ְ�,
*	body�� ����. -> 
------------------------------------------------------------------------------*/
int ixpc_recvEventRxPort (int fd, SockLibMsgType *rxSockMsg)
{
	int		i,sockBodyLen;
	GeneralQMsgType	rxGenQMsg;
	IxpcQMsgType	*rxIxpcQMsg;

	// ��� port���� �޽����� ���ŵǾ����� ã�´�.
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


	/* socket���� ���� �޽����� msgQ �޽����� ��ȯ�Ѵ�.
	* �ܺ� �ý��۰� ����ϹǷ� byte ordering ������ �߻��� �� �ִ�.
	* - IXPC������ socket_header�� ixpc_header�� field �� short/int�� �۽� �� network
	*	byte�� ��ȯ�ϰ�, ���� �� host byte�� �ٽ� ��ȯ�Ѵ�.
	* - ������ body �κ��� Application���� �����ؾ� �Ѵ�.
	* �۽������� msgQ�� mtype�� socket �޽��� header�� 4byte�� ��ȯ�Ͽ� �������Ƿ�,
	*	���� �ٽ� ���� long���� mtype���� msgQ �޽����� �ٽ� ��ȯ�Ͽ� �ִ´�.
	*/
	sockBodyLen = rxSockMsg->head.bodyLen;
	rxGenQMsg.mtype = (long)ntohl(rxSockMsg->head.mtype);
	memcpy ((void*)rxGenQMsg.body, rxSockMsg->body, sockBodyLen);

	rxIxpcQMsg = (IxpcQMsgType*)rxGenQMsg.body;
	rxIxpcQMsg->head.msgId   = ntohl(rxIxpcQMsg->head.msgId);
	rxIxpcQMsg->head.bodyLen = ntohs(rxIxpcQMsg->head.bodyLen);

    /* �ڽſ��� ���޵Ǵ� �޽������� Ȯ���Ͽ� �ڽ��� ó���ؾ� �ϴ� �޽����̸�
    *   ixpc_exeMyMsg()�� ȣ��ȴ�.
    * - �ڽ��� ó���� �޽����� ��� 1�� return�ȴ�.
    */
	if (ixpc_isMyMsg((GeneralQMsgType*)&rxGenQMsg)) {
		return 1;
	}

	return (ixpc_intRoute((GeneralQMsgType*)&rxGenQMsg));

} /** End of ixpc_recvEventRxPort **/



/*------------------------------------------------------------------------------
* remote ixpc��  ������ �۽� port�� �޽����� ���� ��� ȣ��ȴ�.
* - �ڽ��� �۽� port�� �����ð����� �޽����� ������ ���� ���, remote���� connection
*	check�� ���� dummy �޽����� ������ ����̴�. 
* - �׷���, �۽� port�� ���ؼ� �����ð����� �޽����� ������ �ʾ����� �ڽŵ� connection
*	check �޽����� ������.
* - ���� port�� ���� connection check timer ���� �۽� port�� ���� connection check
*	timer�� ª�� ������, remote�� ixpc�� �������̸� �ڽ��� ���� port�� check �޽�����
*	���� ���ŵȴ�.
* - ��, �������� ��� �ڽ��� �۽� port�� check �޽����� ������ �ʴ´�.
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
* remote ixpc�� �����ؿ� socket port�� ������ ��� ȣ��ȴ�.
* - network ��� �߻��� rx_port, tx_port�� ���� event�� ����� �������� �ʴ� ��찡 �����Ƿ�
*   rx_port, tx_port �� disconnect event�� �����Ǹ� ������ port�� ������ �����Ѵ�.
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
* remote ixpc�� ������ socket port(�۽�fd) ������ ��� ȣ��ȴ�.
* - sockRoutTbl�� �˻��Ͽ� disconnect�� fd�� ����� ���� 0���� clear�ϰ�
*   disconnect�� �ð��� ����Ѵ�.
* - �ֱ����� ������ ������ ���� �����ð� ��� �� �������� �õ��Ѵ�.
* - network ��� �߻��� rx_port, tx_port�� ���� event�� ����� �������� �ʴ� ��찡 �����Ƿ�
*   rx_port, tx_port �� disconnect event�� �����Ǹ� ������ port�� ������ �����Ѵ�.
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
* destination�� �ڽ��� �޽��� ���� �� ȣ��ȴ�.
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
			/* connection check �޽����� body�� ����, socket port���� �������ڸ���
			*	lastRxTime�� update�����Ƿ� �ٸ� �߰����� ������ �ʿ����.
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
* 1. sockRoutTbl�� Ȯ���Ͽ� ���� ���ӵ��� ���� remote�� �����Ѵ�.
* - �������� �õ��Ҷ� interval�� �ֱ� ���� disconnect�� �ð��� ����� �ΰ�,
*	�̸� Ȯ���Ͽ� ���� �ð��� ���� ��쿡�� �������� �õ��Ѵ�.
* - ���ӵ��� �ʴ� ��� disconnect_time�� ����ð����� update�Ͽ� ������ interval
*	��� �� ��õ� �ϵ��� �Ѵ�.
* 2. �����ð����� �۽� �޽����� ������ connection check �޽����� ������.
* 3. �����ð����� ���� �޽����� ������ �ۼ��� port�� ��� ������ ���´�.
* 4. �ֱ������� ping test�Ͽ� ���ӽ��н� �ۼ��� port�� ��� ������ ���´�.
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
		/* ���ӵǾ� ���� �ʰ�, ������ interval�� �������� ������ �õ�
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
		/* �����ð����� ���� �޽����� ������ �ۼ��� port�� ��� ������ ���´�.
		* - �ʱ� ���� �� remote������ IXPC_RECONNECT_INTERVAL�� �ֱ�� ��������
		*	�õ��ϹǷ� �ִ� IXPC_CONN_CHECK_PERIOD + IXPC_RECONNECT_INTERVAL ����
		*	���� port���� �޽����� ���� ���� �� �ִ�.
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

		/* �����ð����� �۽� �޽����� ������ connection check �޽����� ������.
		*/
		if ((sockRoutTbl[i].sockTxFd) &&
			((currentTime - sockRoutTbl[i].lastTxTime) >= IXPC_CONN_CHECK_PERIOD))
		{
			ixpc_txConnCheckMsg(i);
		}

#ifdef DELELTED
		/* �ֱ������� ping test�Ͽ� ���ӽ��н� �ۼ��� port�� ��� ������ ���´�.
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
* ��ϵ� primary address�� ���� ���� �õ��ϰ� ���н� secondary�� ���ӽõ��Ѵ�.
* - ���ӵǸ� �۽�port�� �����ϰ�, connection check�� ���� lastTxTime�� ����ð���
*	����Ѵ�.
* - ���ӽ��н� �����ð�(IXPC_RECONNECT_INTERVAL) ��� �� ��õ��ϵ��� �ϱ�����
*	disconnTime�� ����ð��� ����Ѵ�.
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

	/* update disconnect time stamp -> ������ interval�� ����
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
* �����ð����� �۽� �޽����� ������ ȣ��Ǿ� �۽� port�� connection check �޽�����
*	������.
* - �̸� ������ remote�� ixpc�� lastRxTime�� update�Ѵ�.
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
		/* ���� ���н� �ش� fd (�۽�port)�� clear�Ѵ�. (���� disconnect ������ socklib���� �̹� ����Ǿ���.)
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
* dstSysName�� system_name�� ������� �ʰ� system_type�� ��� �ִ��� Ȯ���Ѵ�.
* - type_name�� system_name�� ���� ���� �����Ƿ� system_name�� �ٸ��� �Բ� Ȯ���Ѵ�.
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
