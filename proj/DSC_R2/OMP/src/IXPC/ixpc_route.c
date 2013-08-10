#include "ixpc_proto.h"

extern IXPC_MsgQRoutTable	msgQRoutTbl[SYSCONF_MAX_APPL_NUM];
extern IXPC_SockRoutTable	sockRoutTbl[SYSCONF_MAX_ASSO_SYS_NUM];
extern char	mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN];
extern time_t	currentTime;
extern char		trcBuf[4096], trcTmp[1024];
extern int		trcFlag, trcLogFlag;


/*------------------------------------------------------------------------------
* �ڽ��� msgQ���� ������ �޽����� ó���Ѵ�.
* - ���� routing�̸� ixpc_intRoute(), �ܺ� routing�̸� ixpc_extRoute()�� ȣ���Ѵ�.
* - destination�� ixpc �ڽ��� ���, ixpc_exeMyMsg() ȣ�� �� return�ȴ�.
------------------------------------------------------------------------------*/
int ixpc_exeRxQMsg (GeneralQMsgType *rxGenQMsg)
{
	IxpcQMsgType	*rxIxpcQMsg;

	/* �ڽſ��� ���޵Ǵ� �޽������� Ȯ���Ͽ� �ڽ��� ó���ؾ� �ϴ� �޽����̸�
	*	ixpc_exeMyMsg()�� ȣ��ȴ�.
	* - �ڽ��� ó���� �޽����� ��� 1�� return�ȴ�.
	*/
	if (ixpc_isMyMsg(rxGenQMsg)) {
		return 1;
	}
	
	rxIxpcQMsg = (IxpcQMsgType*)rxGenQMsg->body;

#if 0
    time_t now;
    now = time(0);
    if (rxGenQMsg->mtype == MTYPE_STATISTICS_REPORT){
        fprintf(stderr, "AppName: %s\n", rxIxpcQMsg->head.srcAppName);
        fprintf(stderr, "STATISTIC time: %d\n", now);
    }
    if (rxIxpcQMsg->head.msgId == MSGID_VT_STATISTICS_REPORT){
        fprintf(stderr, "AppName: %s\n", rxIxpcQMsg->head.srcAppName);
        fprintf(stderr, "VT STATISTICS: %d\n", now);
    } 
    if (rxIxpcQMsg->head.msgId == MSGID_CDR2_STATISTICS_REPORT){
        fprintf(stderr, "AppName: %s\n", rxIxpcQMsg->head.srcAppName);
        fprintf(stderr, "CDR2 STATISTICS: %d\n", now);
    }
#endif

#if 0
	if(rxGenQMsg->mtype == MTYPE_MMC_REQUEST)
	{
		sprintf(trcBuf,"[ixpc_exeRxQMsg] MTYPE_MMC_REQUEST, srcSysName: %s, srcAppName: %s, dstSysName: %s, dsAppName: %s\n",
				 rxIxpcQMsg->head.srcSysName, rxIxpcQMsg->head.srcAppName, rxIxpcQMsg->head.dstSysName,rxIxpcQMsg->head.dstAppName);
		trclib_writeLogErr (FL,trcBuf);	

	} else if (rxGenQMsg->mtype == MTYPE_DUP_STATUS_REQUEST){
		sprintf(trcBuf,"[ixpc_exeRxQMsg] recved MTYPE_DUP_STATUS_REQUEST\n");
		trclib_writeLogErr (FL,trcBuf);	
	} else if (rxGenQMsg->mtype == MTYPE_DUP_STATUS_RESPONSE){
		sprintf(trcBuf,"[ixpc_exeRxQMsg] recved MTYPE_DUP_STATUS_RESPONSE\n");
		trclib_writeLogErr (FL,trcBuf);
	}

#endif
	keepalivelib_increase ();

	/* ��Ģ������ �޽����� ������ �� byteOrderFlag�� setting�ؾ� ������,
	*	�ý��� ���� routing, �ܺ� routing�� ������� ixpc���� message queue�� ����
	*	������ �޽����� byteOrderFlag�� �ٽ� �ѹ� ����Ѵ�.
	*/
	rxIxpcQMsg->head.byteOrderFlag = BYTE_ORDER_TAG;

	if (!strcasecmp(rxIxpcQMsg->head.dstSysName, mySysName)) {
		
		return (ixpc_intRoute(rxGenQMsg));
	} else {

		return (ixpc_extRoute(rxGenQMsg));
	}

} /** End of ixpc_exeRxQMsg **/



/*------------------------------------------------------------------------------
* msgQRoutTbl�� �˻��Ͽ� �ش� application�� msgQ�� �޽����� ������.
* - dstAppName�� ��ϵ��� ���� ��� log �޽����� ����� �޽����� ����Ѵ�.
------------------------------------------------------------------------------*/
int ixpc_intRoute (GeneralQMsgType *rxGenQMsg)
{
	int		i,qid,txLen,ret;
	IxpcQMsgType	*rxIxpcQMsg;
	
	rxIxpcQMsg = (IxpcQMsgType*)rxGenQMsg->body;

	for (i=0; i<SYSCONF_MAX_APPL_NUM; i++)
	{
		keepalivelib_increase ();	

		if (!msgQRoutTbl[i].pres)
			continue;
		if (!strcasecmp(rxIxpcQMsg->head.dstAppName, msgQRoutTbl[i].appName))
		{
			/* msgQ�� ������ �޽����� ũ�� ��� �� mtype�� ũ��� ���Ե��� �ʴ´�.
			*/
			txLen = rxIxpcQMsg->head.bodyLen + sizeof(rxIxpcQMsg->head);

			/* get msgQid
			*/
			if ((qid = msgget(msgQRoutTbl[i].msgQkey, 0)) < 0) {
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

				if (rxGenQMsg->mtype == MTYPE_DUP_STATUS_REQUEST){
					sprintf(trcBuf,"[ixpc_intRoute] msgsnd fail MTYPE_DUP_STATUS_REQUEST\n");
					trclib_writeLogErr (FL,trcBuf);
				} else if (rxGenQMsg->mtype == MTYPE_DUP_STATUS_RESPONSE){
					sprintf(trcBuf,"[ixpc_intRoute] msgsnd fail MTYPE_DUP_STATUS_RESPONSE\n");
					trclib_writeLogErr (FL,trcBuf);
				}
			} else {
				sprintf(trcBuf,"int_route; %d (src=%s-%s)(dst=%s-%s)\n",
						rxIxpcQMsg->head.bodyLen ,
						rxIxpcQMsg->head.srcSysName, rxIxpcQMsg->head.srcAppName,
						rxIxpcQMsg->head.dstSysName, rxIxpcQMsg->head.dstAppName);
				trclib_writeLogErr (FL,trcBuf);
				trclib_writeLog (FL,trcBuf);

				if (rxGenQMsg->mtype == MTYPE_DUP_STATUS_REQUEST){
                                        sprintf(trcBuf,"[ixpc_intRoute] msgsnd success MTYPE_DUP_STATUS_REQUEST\n");
                                        trclib_writeLogErr (FL,trcBuf);
				} else if (rxGenQMsg->mtype == MTYPE_DUP_STATUS_RESPONSE){
					sprintf(trcBuf,"[ixpc_intRoute] msgsnd success MTYPE_DUP_STATUS_RESPONSE\n");
					trclib_writeLogErr (FL,trcBuf);
				}
#if 0 /* by june, debugging */
				{
				LEG_STAT *pStat;
				if (rxGenQMsg->mtype == MTYPE_STATISTICS_REPORT){
					//if (rxIxpcQMsg->head.msgId == MSGID_LEG_STATISTICS_REPORT) {
						pStat = (LEG_STAT*)rxIxpcQMsg->body;
						sprintf(trcBuf,"[MTYPE_STATISTICS_REPORT]\n");
					trclib_writeLogErr (FL,trcBuf);
						sprintf(trcBuf,"[uiRadStartCnt		:%d]\n", ntohl(pStat->uiRadStartCnt));
					trclib_writeLogErr (FL,trcBuf);
						sprintf(trcBuf,"[uiRadInterimCnt	:%d]\n", ntohl(pStat->uiRadInterimCnt));
					trclib_writeLogErr (FL,trcBuf);
						sprintf(trcBuf,"[uiRadStopCnt		:%d]\n", ntohl(pStat->uiRadStopCnt));
					trclib_writeLogErr (FL,trcBuf);
						sprintf(trcBuf,"[uiLoginCnt			:%d]\n", ntohl(pStat->uiLoginCnt));
					trclib_writeLogErr (FL,trcBuf);
						sprintf(trcBuf,"[uiLoginSuccCnt		:%d]\n", ntohl(pStat->uiLoginSuccCnt));
					trclib_writeLogErr (FL,trcBuf);
						sprintf(trcBuf,"[uiLogoutCnt		:%d]\n", ntohl(pStat->uiLogoutCnt));
					trclib_writeLogErr (FL,trcBuf);
						sprintf(trcBuf,"[uiLogoutSuccCnt	:%d]\n", ntohl(pStat->uiLogoutSuccCnt));
					trclib_writeLogErr (FL,trcBuf);
					//}
					sprintf(trcBuf,"[ixpc_intRoute] msgsnd success MTYPE_STATISTICS_REPORT#########\n");
					trclib_writeLogErr (FL,trcBuf);
				}
#endif
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
* sockRoutTbl�� �˻��Ͽ� �ش� system���� ���ӵ� socket port�� �޽����� ������.
* - msgQ���� ������ �����͸� sockMsg�� body field�� �״�� ������ ������.
* - dstSysName�� ��ϵ��� �ʾҰų� ������ ���� ���� ��� log �޽����� �����
*	�޽����� ����Ѵ�.
* - �ֱ����� connection check�� ���� lastTxTime�� update�Ѵ�.
------------------------------------------------------------------------------*/
int ixpc_extRoute (GeneralQMsgType *rxGenQMsg)
{
	int		i,ret;
	char		sysType[32];
	IxpcQMsgType	*rxIxpcQMsg;

	rxIxpcQMsg = (IxpcQMsgType*)rxGenQMsg->body;

#if 0 /*jhpark*/
	printf("\n sSysname=%s sAppname=%s dSysname=%s dAppname=%s \n", 
		rxIxpcQMsg->head.srcSysName, rxIxpcQMsg->head.srcAppName,
		rxIxpcQMsg->head.dstSysName, rxIxpcQMsg->head.dstAppName);
#endif

	// dstSysName�� system_name�� ������� �ʰ� system_type�� ��� �ִ��� Ȯ���Ѵ�.
	//
	if (ixpc_isSysTypeRoute(rxIxpcQMsg->head.dstSysName))
	{
		// ���� type�� ������ �ѱ����� ������.
		//
		strcpy (sysType, rxIxpcQMsg->head.dstSysName);
		for (i=0; i<SYSCONF_MAX_ASSO_SYS_NUM; i++) // type_name�� ���� ���� ã�´�.
		{
			if (sockRoutTbl[i].pres && sockRoutTbl[i].sockTxFd &&
				!strcasecmp(sysType, sockRoutTbl[i].sysType)) 
			{
				// type_name ��� system_name���� �ٲ�ִ´�.
				strcpy (rxIxpcQMsg->head.dstSysName, sockRoutTbl[i].sysName);
				if ((ret = ixpc_extRouteSend (rxGenQMsg, sockRoutTbl[i].sockTxFd)) < 0) {
					sockRoutTbl[i].sockTxFd = 0; /* clear tx_port */
					strcpy (rxIxpcQMsg->head.dstSysName, sysType);
					
					if (rxGenQMsg->mtype == MTYPE_DUP_STATUS_REQUEST){
                                        	sprintf(trcBuf,"[ixpc_extRoute] ixpc_extRouteSend fail MTYPE_DUP_STATUS_REQUEST\n");
                                        	trclib_writeLogErr (FL,trcBuf);
                  	} else if (rxGenQMsg->mtype == MTYPE_DUP_STATUS_RESPONSE){
                                        	sprintf(trcBuf,"[ixpc_extRoute] ixpc_extRouteSend fail MTYPE_DUP_STATUS_RESPONSE\n");
											trclib_writeLogErr (FL,trcBuf);
                  	}
				} else {
					sockRoutTbl[i].lastTxTime = currentTime; /* update last transmitted time */

					if (rxGenQMsg->mtype == MTYPE_DUP_STATUS_REQUEST){
                                                sprintf(trcBuf,"[ixpc_extRoute] ixpc_extRouteSend success MTYPE_DUP_STATUS_REQUEST\n");
                                                trclib_writeLogErr (FL,trcBuf);
                    } else if (rxGenQMsg->mtype == MTYPE_DUP_STATUS_RESPONSE){
                                                sprintf(trcBuf,"[ixpc_extRoute] ixpc_extRouteSend success MTYPE_DUP_STATUS_RESPONSE\n");
                                                trclib_writeLogErr (FL,trcBuf);
                    }	
				}
				return ret;
			}
		} /* end of for() */
		// type_name�� ���� ���ӵ� ���� ���� ���
		sprintf(trcBuf,"ext_ROUTE FAIL; (src=%s-%s)(dst=%s-%s); not connected\n",
				rxIxpcQMsg->head.srcSysName, rxIxpcQMsg->head.srcAppName,
				rxIxpcQMsg->head.dstSysName, rxIxpcQMsg->head.dstAppName);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	else // type_name���� routing���� �ʰ� system_name���� routing�ϴ� ���
	{
		for (i=0; i<SYSCONF_MAX_ASSO_SYS_NUM; i++) // system_name�� ���� ���� ã�´�.
		{
			if (sockRoutTbl[i].pres &&
				!strcasecmp(rxIxpcQMsg->head.dstSysName, sockRoutTbl[i].sysName)) 
			{
				if (sockRoutTbl[i].sockTxFd == 0) { // ���ӵǾ� ���� ���� ���
					sprintf(trcBuf,"ext_ROUTE FAIL; (src=%s-%s)(dst=%s-%s); not connected\n",
							rxIxpcQMsg->head.srcSysName, rxIxpcQMsg->head.srcAppName,
							rxIxpcQMsg->head.dstSysName, rxIxpcQMsg->head.dstAppName);
					trclib_writeLogErr (FL,trcBuf);
					return -1;
				}
				if ((ret = ixpc_extRouteSend (rxGenQMsg, sockRoutTbl[i].sockTxFd)) < 0) {
					sockRoutTbl[i].sockTxFd = 0; /* clear tx_port */

					if (rxGenQMsg->mtype == MTYPE_DUP_STATUS_REQUEST){
                                                sprintf(trcBuf,"[ixpc_extRoute] ixpc_extRouteSend fail MTYPE_DUP_STATUS_REQUEST\n");
                                                trclib_writeLogErr (FL,trcBuf);
                                        } else if (rxGenQMsg->mtype == MTYPE_DUP_STATUS_RESPONSE){
                                                sprintf(trcBuf,"[ixpc_extRoute] ixpc_extRouteSend fail MTYPE_DUP_STATUS_RESPONSE\n");
                                                trclib_writeLogErr (FL,trcBuf);
                                        }
				} else {
					sockRoutTbl[i].lastTxTime = currentTime; /* update last transmitted time */

					if (rxGenQMsg->mtype == MTYPE_DUP_STATUS_REQUEST){
                                                sprintf(trcBuf,"[ixpc_extRoute] ixpc_extRouteSend success MTYPE_DUP_STATUS_REQUEST\n");
                                                trclib_writeLogErr (FL,trcBuf);
                                        } else if (rxGenQMsg->mtype == MTYPE_DUP_STATUS_RESPONSE){
                                                sprintf(trcBuf,"[ixpc_extRoute] ixpc_extRouteSend success MTYPE_DUP_STATUS_RESPONSE\n");
                                                trclib_writeLogErr (FL,trcBuf);
                                        }
				}
				return ret;
			}
		} /* end of for() */
		 // system_name�� ���� ���� ���� ���
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

	/* socket���� ������ �޽��� body ũ�� ��� �ÿ��� msgQ���� ���� �޽�����
	*	mtype�� ũ����� �ݵ�� ���ԵǾ�� �Ѵ�.
	* socket���� ������ �޽��� body���� msgQ���� ���� �޽����� mtype�� ������ body��
	*	(��, ixpc�޽��� �κи�)�ְ�, mtype�� ������ 4byte�� ��ȯ�Ͽ� header�� ���Խ�Ų��.
	*/
	sockBodyLen = rxIxpcQMsg->head.bodyLen + sizeof(rxIxpcQMsg->head);


	/* �ܺ� �ý��۰� ����ϹǷ� byte ordering ������ �߻��� �� �ִ�.
	* - IXPC������ socket_header�� ixpc_header�� field �� short/int�� �۽� �� network
	*   byte�� ��ȯ�ϰ�, ���� �� host byte�� �ٽ� ��ȯ�Ѵ�.
	* - ������ body �κ��� Application���� �����ؾ� �Ѵ�.
	*/
	txSockMsg.head.bodyLen = htonl(sockBodyLen);
	txSockMsg.head.mtype   = htonl((int)rxGenQMsg->mtype);
	rxIxpcQMsg->head.msgId   = htons(rxIxpcQMsg->head.msgId);
	rxIxpcQMsg->head.bodyLen = htons(rxIxpcQMsg->head.bodyLen);

	/* msgQ���� ������ �����͸� sockMsg�� body field�� �״�� �����Ѵ�.(mtype����)
	*/
	memcpy (txSockMsg.body, rxIxpcQMsg, sockBodyLen);
	txLen = sockBodyLen + sizeof(txSockMsg.head);


	/* �ش� system���� ���ӵ� socket port�� �޽����� ������.
	* - ���� ���н� fd (�۽�port)�� clear�Ѵ�. (���� disconnect ������ socklib���� �̹� ����Ǿ���.)
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

	/* mtype�� ���� Ȯ���Ͽ� dest�� �ڽ����� Ȯ���ؾ� �Ѵ�.
	* - ixpc header�� ���� �ʴ� �޽����� ���� �ְ� ixpc header�� ���� �޽����� ���� �ִ�.
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
