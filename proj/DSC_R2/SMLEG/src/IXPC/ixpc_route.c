#include "ixpc_proto.h"

extern IXPC_MsgQRoutTable	msgQRoutTbl[SYSCONF_MAX_APPL_NUM];
extern IXPC_SockRoutTable	sockRoutTbl[SYSCONF_MAX_ASSO_SYS_NUM];
extern char	mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN];
extern time_t	currentTime;
extern char		trcBuf[4096], trcTmp[1024];
extern int		trcFlag, trcLogFlag;



/*------------------------------------------------------------------------------
* 자신의 msgQ에서 수신한 메시지를 처리한다.
* - 내부 routing이면 ixpc_intRoute(), 외부 routing이면 ixpc_extRoute()를 호출한다.
* - destination이 ixpc 자신인 경우, ixpc_exeMyMsg() 호출 후 return된다.
------------------------------------------------------------------------------*/
int ixpc_exeRxQMsg (GeneralQMsgType *rxGenQMsg)
{
	IxpcQMsgType	*rxIxpcQMsg;

	/* 자신에게 전달되는 메시지인지 확인하여 자신이 처리해야 하는 메시지이면
	*	ixpc_exeMyMsg()를 호출된다.
	* - 자신이 처리할 메시지인 경우 1이 return된다.
	*/
	if (ixpc_isMyMsg(rxGenQMsg)) {
		return 1;
	}
#if 0
	time_t now;
    now = time(0);	
	if (rxGenQMsg->mtype == MTYPE_STATISTICS_REPORT)
		fprintf(stderr, "STATISTIC time: %d\n", now);


#endif
	rxIxpcQMsg = (IxpcQMsgType*)rxGenQMsg->body;

	/* 원칙적으로 메시지를 보내는 놈에 byteOrderFlag를 setting해야 하지만,
	*	시스템 내부 routing, 외부 routing에 관계없이 ixpc에서 message queue를 통해
	*	수신한 메시지에 byteOrderFlag를 다시 한번 기록한다.
	*/
	rxIxpcQMsg->head.byteOrderFlag = BYTE_ORDER_TAG;

	if (!strcasecmp(rxIxpcQMsg->head.dstSysName, mySysName)) {
		return (ixpc_intRoute(rxGenQMsg));
	} else {
		return (ixpc_extRoute(rxGenQMsg));
	}

} /** End of ixpc_exeRxQMsg **/



/*------------------------------------------------------------------------------
* msgQRoutTbl을 검색하여 해당 application의 msgQ로 메시지를 보낸다.
* - dstAppName이 등록되지 않은 경우 log 메시지를 남기고 메시지는 폐기한다.
------------------------------------------------------------------------------*/
int ixpc_intRoute (GeneralQMsgType *rxGenQMsg)
{
	int		i,qid,txLen,ret;
	IxpcQMsgType	*rxIxpcQMsg;
	
	rxIxpcQMsg = (IxpcQMsgType*)rxGenQMsg->body;
#if 1
	//if((rxIxpcQMsg->head.dstSysName == "SCMB") && (rxIxpcQMsg->head.dstAppName == "RLEG")){
		sprintf(trcBuf,"int_ROUTE info (src=%s-%s)(dst=%s-%s)(rxGenQMsg->mtype=%ld)\n",
				rxIxpcQMsg->head.srcSysName, rxIxpcQMsg->head.srcAppName,
				rxIxpcQMsg->head.dstSysName, rxIxpcQMsg->head.dstAppName,rxGenQMsg->mtype);
		trclib_writeLogErr (FL,trcBuf);
	//}
#endif
	for (i=0; i<SYSCONF_MAX_APPL_NUM; i++)
	{
		if (!msgQRoutTbl[i].pres)
			continue;
		if (!strcasecmp(rxIxpcQMsg->head.dstAppName, msgQRoutTbl[i].appName))
		{
			/* msgQ로 보내는 메시지의 크기 계산 시 mtype의 크기는 포함되지 않는다.
			*/
			txLen = rxIxpcQMsg->head.bodyLen + sizeof(rxIxpcQMsg->head);

			/* get msgQid
			*/
			if ((qid = msgget(msgQRoutTbl[i].msgQkey, 0)) < 0) {
			//if ((qid = msgget(msgQRoutTbl[i].msgQkey, IPC_CREAT|0666)) < 0) {
				sprintf(trcBuf,"int_ROUTE FAIL; (src=%s-%s)(dst=%s-%s); msgget fail; err=%d(%s)\n",
						rxIxpcQMsg->head.srcSysName, rxIxpcQMsg->head.srcAppName,
						rxIxpcQMsg->head.dstSysName, rxIxpcQMsg->head.dstAppName,
						errno, strerror(errno));
				trclib_writeLogErr (FL,trcBuf);
				return -1;
			}

			/* send to destination
			*/
			if ((ret = msgsnd(qid, rxGenQMsg, txLen, IPC_NOWAIT)) < 0) {
				sprintf(trcBuf,"int_ROUTE FAIL; (src=%s-%s)(dst=%s-%s); msgsnd fail; err=%d(%s)\n",
						rxIxpcQMsg->head.srcSysName, rxIxpcQMsg->head.srcAppName,
						rxIxpcQMsg->head.dstSysName, rxIxpcQMsg->head.dstAppName,
						errno, strerror(errno));
				trclib_writeLogErr (FL,trcBuf);
			} else {
				if (trcFlag || trcLogFlag) {
					sprintf(trcBuf,"int_route; (src=%s-%s)(dst=%s-%s)\n",
							rxIxpcQMsg->head.srcSysName, rxIxpcQMsg->head.srcAppName,
							rxIxpcQMsg->head.dstSysName, rxIxpcQMsg->head.dstAppName);
					trclib_writeLog (FL,trcBuf);
				}
			}
			return ret;
		}
	} /* end of for() */

	sprintf(trcBuf,"int_ROUTE FAIL; (src=%s-%s)(dst=%s-%s); unknown appName;\n",
			rxIxpcQMsg->head.srcSysName, rxIxpcQMsg->head.srcAppName,
			rxIxpcQMsg->head.dstSysName, rxIxpcQMsg->head.dstAppName);
	trclib_writeLogErr (FL,trcBuf);
	return -1;

} /** End of ixpc_intRoute **/



/*------------------------------------------------------------------------------
* sockRoutTbl을 검색하여 해당 system으로 접속된 socket port로 메시지를 보낸다.
* - msgQ에서 수신한 데이터를 sockMsg의 body field에 그대로 복사해 보낸다.
* - dstSysName이 등록되지 않았거나 접속이 되지 않은 경우 log 메시지를 남기고
*	메시지는 폐기한다.
* - 주기적인 connection check를 위해 lastTxTime을 update한다.
------------------------------------------------------------------------------*/
int ixpc_extRoute (GeneralQMsgType *rxGenQMsg)
{
	int				i,ret;
	char			sysType[32];
	IxpcQMsgType	*rxIxpcQMsg;

	rxIxpcQMsg = (IxpcQMsgType*)rxGenQMsg->body;

#if 0 /*jhpark*/
		printf("\n sSysname=%s sAppname=%s dSysname=%s dAppname=%s \n", 
				rxIxpcQMsg->head.srcSysName, rxIxpcQMsg->head.srcAppName,
				rxIxpcQMsg->head.dstSysName, rxIxpcQMsg->head.dstAppName);
#endif

	// dstSysName에 system_name이 들어있지 않고 system_type이 들어 있는지 확인한다.
	//
	if (ixpc_isSysTypeRoute(rxIxpcQMsg->head.dstSysName))
	{
		// 같은 type중 임의의 한군데로 보낸다.
		//
		strcpy (sysType, rxIxpcQMsg->head.dstSysName);
		for (i=0; i<SYSCONF_MAX_ASSO_SYS_NUM; i++) // type_name과 같은 놈을 찾는다.
		{
			if (sockRoutTbl[i].pres &&
				sockRoutTbl[i].sockTxFd &&
				!strcasecmp(sysType, sockRoutTbl[i].sysType)) 
			{
				 // type_name 대신 system_name으로 바꿔넣는다.
				strcpy (rxIxpcQMsg->head.dstSysName, sockRoutTbl[i].sysName);
				if ((ret = ixpc_extRouteSend (rxGenQMsg, sockRoutTbl[i].sockTxFd)) < 0) {
					sockRoutTbl[i].sockTxFd = 0; /* clear tx_port */
					strcpy (rxIxpcQMsg->head.dstSysName, sysType);
				} else {
					sockRoutTbl[i].lastTxTime = currentTime; /* update last transmitted time */
				}
				return ret;
			}
		} /* end of for() */
		 // type_name과 같고 접속된 놈이 없는 경우
		sprintf(trcBuf,"ext_ROUTE FAIL#1; (src=%s-%s)(dst=%s-%s); not connected\n",
				rxIxpcQMsg->head.srcSysName, rxIxpcQMsg->head.srcAppName,
				rxIxpcQMsg->head.dstSysName, rxIxpcQMsg->head.dstAppName);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	else // type_name으로 routing하지 않고 system_name으로 routing하는 경우
	{
		for (i=0; i<SYSCONF_MAX_ASSO_SYS_NUM; i++) // system_name과 같은 놈을 찾는다.
		{
			if (sockRoutTbl[i].pres &&
				!strcasecmp(rxIxpcQMsg->head.dstSysName, sockRoutTbl[i].sysName)) 
			{
				if (sockRoutTbl[i].sockTxFd == 0) { // 접속되어 있지 않은 경우
					sprintf(trcBuf,"ext_ROUTE FAIL#2; (src=%s-%s)(dst=%s-%s); not connected\n",
							rxIxpcQMsg->head.srcSysName, rxIxpcQMsg->head.srcAppName,
							rxIxpcQMsg->head.dstSysName, rxIxpcQMsg->head.dstAppName);
					trclib_writeLogErr (FL,trcBuf);
					return -1;
				}
				if ((ret = ixpc_extRouteSend (rxGenQMsg, sockRoutTbl[i].sockTxFd)) < 0) {
					sockRoutTbl[i].sockTxFd = 0; /* clear tx_port */
				} else {
					sockRoutTbl[i].lastTxTime = currentTime; /* update last transmitted time */
				}
				return ret;
			}
		} /* end of for() */
		 // system_name과 같은 놈이 없는 경우
		sprintf(trcBuf,"ext_ROUTE FAIL; (src=%s-%s)(dst=%s-%s); unknown sysName;\n",
				rxIxpcQMsg->head.srcSysName, rxIxpcQMsg->head.srcAppName,
				rxIxpcQMsg->head.dstSysName, rxIxpcQMsg->head.dstAppName);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	return -1;

} /** End of ixpc_extRoute **/



/*------------------------------------------------------------------------------
------------------------------------------------------------------------------*/
int ixpc_extRouteSend (GeneralQMsgType *rxGenQMsg, int sockTxFd)
{
	int		sockBodyLen,txLen,ret;
	IxpcQMsgType	*rxIxpcQMsg;
	SockLibMsgType	txSockMsg;

	rxIxpcQMsg = (IxpcQMsgType*)rxGenQMsg->body;

	/* socket으로 보내는 메시지 body 크기 계산 시에는 msgQ에서 받은 메시지의
	*	mtype의 크기까지 반드시 포함되어야 한다.
	* socket으로 보내는 메시지 body에는 msgQ에서 받은 메시지의 mtype을 제외한 body만
	*	(즉, ixpc메시지 부분만)넣고, mtype은 무조건 4byte로 변환하여 header에 포함시킨다.
	*/
	sockBodyLen = rxIxpcQMsg->head.bodyLen + sizeof(rxIxpcQMsg->head);


	/* 외부 시스템과 통신하므로 byte ordering 문제가 발생할 수 있다.
	* - IXPC에서는 socket_header와 ixpc_header의 field 중 short/int를 송신 시 network
	*   byte로 변환하고, 수신 시 host byte로 다시 변환한다.
	* - 나머지 body 부분은 Application에서 직접해야 한다.
	*/
	txSockMsg.head.bodyLen = htonl(sockBodyLen);
	txSockMsg.head.mtype   = htonl((int)rxGenQMsg->mtype);
	rxIxpcQMsg->head.msgId   = htonl(rxIxpcQMsg->head.msgId);
	rxIxpcQMsg->head.bodyLen = htons(rxIxpcQMsg->head.bodyLen);


	/* msgQ에서 수신한 데이터를 sockMsg의 body field에 그대로 복사한다.(mtype제외)
	*/
	memcpy (txSockMsg.body, rxIxpcQMsg, sockBodyLen);
	txLen = sockBodyLen + sizeof(txSockMsg.head);

	/* 해당 system으로 접속된 socket port로 메시지를 보낸다.
	* - 전송 실패시 fd (송신port)를 clear한다. (실제 disconnect 동작은 socklib에서 이미 수행되었다.)
	*/
	if ((ret = socklib_sndMsg(sockTxFd, (char*)&txSockMsg, txLen)) < 0) {
		sprintf(trcBuf,"ext_ROUTE FAIL; (src=%s-%s)(dst=%s-%s); socklib_sndMsg fail\n",
				rxIxpcQMsg->head.srcSysName, rxIxpcQMsg->head.srcAppName,
				rxIxpcQMsg->head.dstSysName, rxIxpcQMsg->head.dstAppName);
		trclib_writeLogErr (FL,trcBuf);
	} else {
		if (trcFlag || trcLogFlag) {
			sprintf(trcBuf,"ext_route; (src=%s-%s)(dst=%s-%s)\n",
					rxIxpcQMsg->head.srcSysName, rxIxpcQMsg->head.srcAppName,
					rxIxpcQMsg->head.dstSysName, rxIxpcQMsg->head.dstAppName);
			trclib_writeLog (FL,trcBuf);
		}
	}

	return ret;

} /** End of ixpc_extRouteSend **/



/*------------------------------------------------------------------------------
------------------------------------------------------------------------------*/
int ixpc_isMyMsg (GeneralQMsgType *rxGenQMsg)
{
	IxpcQMsgType	*rxIxpcQMsg;

	rxIxpcQMsg = (IxpcQMsgType*)rxGenQMsg->body;

	/* mtype을 먼저 확인하여 dest가 자신인지 확인해야 한다.
	* - ixpc header를 갖지 않는 메시지일 수도 있고 ixpc header를 갖는 메시지일 수도 있다.
	*/
	if ((rxGenQMsg->mtype == MTYPE_SETPRINT) ||
		(!strcasecmp(rxIxpcQMsg->head.dstSysName, mySysName) &&
		 !strcasecmp(rxIxpcQMsg->head.dstAppName, myAppName)))
	{
		ixpc_exeMyMsg (rxGenQMsg);
		return 1;
	}
	return 0;
} /** End of ixpc_isMyMsg **/
