#include "mmcr.h"
extern char         trcBuf[TRCBUF_LEN], trcTmp[TRCTMP_LEN];
extern int      msgqTable[MSGQ_MAX_SIZE];
extern char     mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN], sysLabel[COMM_MAX_NAME_LEN];

int oldSendAppNoty2(int appKind, USHORT sType, UCHAR sSvcID, UCHAR sMsgID)
{                                                                                        
	st_MsgQ     stmsg;
	st_MsgQSub  stsub;
	int         qid;
	int         txLen;

	GeneralQMsgType txGenQMsg;
	IxpcQMsgType    txIxpcMsg;

	txGenQMsg.mtype = MTYPE_PDSN_CONFIG;

	strcpy (txIxpcMsg.head.srcSysName, mySysName);
	strcpy (txIxpcMsg.head.srcAppName, myAppName);
	strcpy (txIxpcMsg.head.dstSysName, mySysName);
	strcpy (txIxpcMsg.head.dstAppName, "RLEG");
	txIxpcMsg.head.segFlag = 0;
	txIxpcMsg.head.seqNo = 1;

	qid = msgqTable[3]; // mmcr_init.c 

	stmsg.usBodyLen = 0;
	stmsg.usRetCode = 0;
	stmsg.dMsgQID = msgqTable[1];

	memset(&stmsg, 0, sizeof(st_MsgQ));
	memset(&stsub, 0, sizeof(st_MsgQSub));

	//stsub.uiType  = sType; // DEF_SVC or DEF_SYS
	stsub.usType  = sType; // DEF_SVC or DEF_SYS
	stsub.usSvcID = sSvcID; // SID_CHG_INFO 
	stsub.usMsgID = sMsgID; // MID_TRC = 100 

	if(appKind == 3)
	{
		memcpy(&stmsg.llMType, &stsub, sizeof(st_MsgQSub));
		txLen = sizeof(txIxpcMsg.head) + txIxpcMsg.head.bodyLen;
		if (memcpy ((void*)txGenQMsg.body, &stmsg, txLen) == NULL) {                     
			sprintf(trcBuf, "memcpy err = %s\n", strerror(errno));
			trclib_writeLogErr(FL,trcBuf);
			return -1;
		}
		if (msgsnd (qid, (void*)&txGenQMsg, txLen, IPC_NOWAIT) < 0) {
			sprintf(trcBuf, "[MMCReqBypassSnd] msgsnd error=%d(%s)\n", errno, strerror(errno));
			trclib_writeLogErr(FL,trcBuf);
			return -1;
		}

	}

	return 1;
}

int doAddPdsnInfoShm(void *tbl, IxpcQMsgType *rxIxpcMsg,
                  GeneralQMsgType genQMsgType, void *pft,
                  MpConfigCmd *mcc, DLinkedList *head, DLinkedList *tail,
                  char columns[MAX_COL_COUNT][MAX_COL_SIZE],char  fname[FILESIZE])
{
    int ret;
    //int found;

    //DLinkedList *node;

    st_IPPool   stIPPool;
    //DESTIP_KEY  DestIPKey1, DestIPKey2;
    DESTIP_DATA DestIP, *pDestIP;
    unsigned int    uiIP, uiNetMask;
    char command[FILESIZE];
    char tmpfname[FILESIZE];
    char    bodyMag[MMCMSGSIZE], totalBuffer[BUFSIZE], titleMag[BUFSIZE],tmpMag[BUFSIZE], txBuf[MMCMSGSIZE];
    pMmcFuncTable ft;
    pMmcMemberTable mmcHdlr;
    ft=(pMmcFuncTable)pft;

    mmcHdlr=(pMmcMemberTable)tbl;
    memset(txBuf, 0, sizeof(txBuf));
    memset(tmpMag, 0, sizeof(tmpMag));
    memset(bodyMag, 0, sizeof(bodyMag));
    memset(totalBuffer, 0, sizeof(totalBuffer));
    memset(titleMag, 0, sizeof(titleMag));

    //tmpfile create
    memset( command, 0, sizeof( command ) );
    memset( tmpfname, 0, sizeof( tmpfname ) );
    strcpy( tmpfname, fname );
    strcat( tmpfname, ".tmp" );
    sprintf (command, "cp -rf %s %s", fname, tmpfname );
    system (command);

    if( doAddPdsnProcess( tbl, rxIxpcMsg, genQMsgType, pft, mcc, head,tail, columns, fname )<0 ){
		sprintf (command, "mv -f %s %s", tmpfname, fname);
		system(command);
        sprintf(trcBuf, "[doAddPdsnInfoShm]COMMAND:%s Process Fail\n", ft->cmdName);
        trclib_writeLogErr(FL, trcBuf);
        return -1;
	}

#if 0 // jjinri
    memset( &stIPPool, 0x00, sizeof(st_IPPool) );
    strcpy(stIPPool.szSysIP, columns[0]);

    uiIP = inet_addr(stIPPool.szSysIP);
    if( uiIP == (in_addr_t)-1)
    {
        ret = -1;
    } else {

        // inet_addr converts alreay
        uiNetMask = 0xffffffff;

        // IP POOL
        memset( &DestIP, 0x00, sizeof(DESTIP_DATA) );

        DestIP.key.ucFlag = 'S';
        uiIP = htonl(uiIP) | ~uiNetMask;
        DestIP.key.uiIP = ntohl(uiIP);
        DestIP.uiNetmask = 0xffffffff;
        DestIP.ucIPType = IPTYPE_PDSN;
		//DestIP.ucPdsnType = 0;
        pDestIP = Search_DESTIP( &DestIP.key );

        if( pDestIP == 0 )  /* not exist */
        {
            ret = Insert_DESTIP( &DestIP );
        }
        else
        {
            ret = -1;
        }

    }
#endif
    if( ret < 0 )
    {
        //file delete
        sprintf (command, "mv -f %s %s", tmpfname, fname );
        system (command);
        sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = SHARED MEMORY ADD ERROR!\n");
        MMCResSnd(rxIxpcMsg, txBuf, 1, 0);
        sprintf(trcBuf, "[doAddPdsnInfoShm]COMMAND:%s Process Fail\n", ft->cmdName);
        trclib_writeLogErr(FL, trcBuf);
    }else {

        sprintf(txBuf, "\n    RESULT = SUCCESS\n    REASON = INFOMATION ADD SUCCESS");
        addHeadMessage( mmcHdlr, titleMag );
        addBodyLinePdsnMessage( mmcHdlr, columns, tmpMag, 2 );
        strcat(bodyMag, tmpMag );
        addTailMessage( mmcHdlr, totalBuffer );

        strcat(txBuf, titleMag );       //title
        strcat(txBuf, bodyMag);          // body
        strcat(txBuf, totalBuffer);    //------------
        MMCResSnd(rxIxpcMsg, txBuf, 0, 0);
		unlink(tmpfname);
        conf_file_sync(ft->confFile);
        sprintf(trcBuf, "[doAddPdsnInfoShm]COMMAND:%s Process Success\n", ft->cmdName);
        trclib_writeLogErr(FL, trcBuf);
		oldSendAppNoty2(3, DEF_SYS, SID_CHG_INFO, MID_CHG_PDSN);
    }
 
	return 1;
}

int doDelPdsnInfoShm(void *tbl, IxpcQMsgType *rxIxpcMsg,
                  GeneralQMsgType genQMsgType, void *pft,
                  MpConfigCmd *mcc, DLinkedList *head, DLinkedList *tail,
                  char columns[MAX_COL_COUNT][MAX_COL_SIZE],char  fname[FILESIZE])
{
    int ret;
    //int found;

    //DLinkedList *node;

    st_IPPool   stIPPool;
    //DESTIP_KEY  DestIPKey1, DestIPKey2;
    DESTIP_DATA DestIP, *pDestIP;
    unsigned int    uiIP, uiNetMask;
    char command[FILESIZE];
    char tmpfname[FILESIZE];
     char    bodyMag[MMCMSGSIZE], totalBuffer[BUFSIZE], titleMag[BUFSIZE],tmpMag[BUFSIZE], txBuf[MMCMSGSIZE];
    pMmcMemberTable mmcHdlr;
     pMmcFuncTable ft;
    ft=(pMmcFuncTable)pft;

    mmcHdlr=(pMmcMemberTable)tbl;
    memset(txBuf, 0, sizeof(txBuf));
    memset(tmpMag, 0, sizeof(tmpMag));
    memset(bodyMag, 0, sizeof(bodyMag));
    memset(totalBuffer, 0, sizeof(totalBuffer));
    memset(titleMag, 0, sizeof(titleMag));

    //tmpfile create
    memset( command, 0, sizeof( command ) );
    memset( tmpfname, 0, sizeof( tmpfname ) );
    strcpy( tmpfname, fname );
    strcat( tmpfname, ".tmp" );
    sprintf (command, "cp -rf %s %s", fname, tmpfname );
    system (command);

    if( doDelPdsnProcess( tbl, rxIxpcMsg, genQMsgType, pft, mcc, head,tail, columns, fname )<0 ){
		sprintf (command, "mv -f %s %s", tmpfname, fname);
		system(command);
        sprintf(trcBuf, "[doDelPdsnInfoShm]COMMAND:%s Process Fail\n", ft->cmdName);
        trclib_writeLogErr(FL, trcBuf);
        return -1;
	}

#if 0 // jjinri
    memset( &stIPPool, 0x00, sizeof(st_IPPool) );
    strcpy(stIPPool.szSysIP, columns[0]);

    uiIP = inet_addr(stIPPool.szSysIP);
    if( uiIP == (in_addr_t)-1)
    {
        ret = -1;
    } else {

        // inet_addr converts alreay
        uiNetMask = 0xffffffff;

        // IP POOL
        memset( &DestIP, 0x00, sizeof(DESTIP_DATA) );

        DestIP.key.ucFlag = 'S';
        uiIP = htonl(uiIP) | ~uiNetMask;
        DestIP.key.uiIP = ntohl(uiIP);
        DestIP.uiNetmask = 0xffffffff;
        DestIP.ucIPType = IPTYPE_PDSN;

        pDestIP = Search_DESTIP( &DestIP.key );

        if( pDestIP == 0 )  /* not exist */
        {
            ret = -1;
        }
        else
        {
            ret = Delete_DESTIP( &DestIP.key );
        }

    }
#endif 

    if( ret < 0 )
    {
        //file delete
        sprintf (command, "mv -f %s %s", tmpfname, fname );
        system (command);
        sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = SHARED MEMORY DEL ERROR!\n");
        MMCResSnd(rxIxpcMsg, txBuf, 1, 0);
        sprintf(trcBuf, "[doDelPdsnInfoShm]COMMAND:%s Process Fail\n", ft->cmdName);
        trclib_writeLogErr(FL, trcBuf);
    }else {

        sprintf(txBuf, "\n    RESULT = SUCCESS\n    REASON = INFOMATION DEL SUCCESS");
        addHeadMessage( mmcHdlr, titleMag );
        addBodyLinePdsnMessage( mmcHdlr, columns, tmpMag, 1 );
        strcat(bodyMag, tmpMag );
        addTailMessage( mmcHdlr, totalBuffer );
        strcat(txBuf, titleMag );       //title
        strcat(txBuf, bodyMag);          // body
        strcat(txBuf, totalBuffer);
        MMCResSnd(rxIxpcMsg, txBuf, 0, 0);
		unlink(tmpfname);
        conf_file_sync(ft->confFile);
        sprintf(trcBuf, "[doDelPdsnInfoShm]COMMAND:%s Process Success\n", ft->cmdName);
        trclib_writeLogErr(FL, trcBuf);
		oldSendAppNoty2(3, DEF_SYS, SID_CHG_INFO, MID_CHG_PDSN);
    }

	return 1;
}

