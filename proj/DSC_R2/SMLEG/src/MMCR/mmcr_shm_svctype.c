#include "mmcr.h"

extern char origincolumns[MAX_COL_COUNT][MAX_COL_SIZE];
extern char oldcolumns[MAX_COL_COUNT][MAX_COL_SIZE];
extern char newcolumns[MAX_COL_COUNT][MAX_COL_SIZE];
extern char delcolumns[MAX_COL_COUNT][MAX_COL_SIZE];

extern stHASHOINFO	*pstHashSvcType;

extern int load_svc_group(char columns[MAX_COL_COUNT][MAX_COL_SIZE], st_CategoryInfo *stCat );
extern unsigned char get_svc_group(char *layer);

int create_IPDATA_key(char columns[MAX_COL_COUNT][MAX_COL_SIZE],
                         DESTIP_KEY *key);
int create_PORTDATA_key(char columns[MAX_COL_COUNT][MAX_COL_SIZE],
                         DESTPORT_KEY *key);

int doAddSvcTypeShm(void *tbl, IxpcQMsgType *rxIxpcMsg,
                  GeneralQMsgType genQMsgType, void *pft,
                  MpConfigCmd *mcc, DLinkedList *head, DLinkedList *tail,
                  char columns[MAX_COL_COUNT][MAX_COL_SIZE],char  fname[FILESIZE])
{
    int ret;
    int iPrefix, svcGroup;

    //DLinkedList *node;
    st_CategoryInfo     stCat;
    //DESTIP_KEY  DestIPKey1, DestIPKey2;
    DESTIP_DATA DestIP, *pDestIP;
    DESTPORT_DATA   DestPort, *pDestPort;
    //DESTPORT_KEY    DestPortKey1, DestPortKey2;

	stHASHONODE             *pstHashNode;
	pst_SvcType_HashKey     pstKey;
	pst_SvcType_HashData    pstData;

    unsigned int    uiIP, uiNetMask;
    char command[FILESIZE];
    char tmpfname[FILESIZE];
    char trcBuf[BUFSIZE];
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

    memset( txBuf, 0, sizeof( txBuf ) );
    memset( trcBuf, 0, sizeof( trcBuf ) );
    //tmpfile create
    memset( command, 0, sizeof( command ) );
    memset( tmpfname, 0, sizeof( tmpfname ) );
    strcpy( tmpfname, fname );
    strcat( tmpfname, ".tmp" );
    sprintf (command, "cp -rf %s %s", fname, tmpfname );
    system (command);
    
   	// add by helca 2007.02.15
	// Layer=IP인 경우 Port=0이외의 값 입력시 에러처리
	if (!strcasecmp(columns[1], "IP") && atoi(columns[5]) != 0){
	     sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = PORT INPUT ERROR (The Number of Port must be 0 in case IP Layer)\n");
	     MMCResSnd(rxIxpcMsg, txBuf, 1, 0);
	     return -1; 
	}

	if ((!strcasecmp(columns[1], "TCP") || !strcasecmp(columns[1], "UDP")) && atoi(columns[5]) == 0 ) {
		sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = PORT INPUT ERROR (The Number of Port must not 0 in case TCP,UDP Layer)\n");
		MMCResSnd(rxIxpcMsg, txBuf, 1, 0);
	    return -1;
	}

    if( doAddSvcType( tbl, rxIxpcMsg, genQMsgType, pft, mcc, head,tail, columns, fname )<0 ){
		sprintf (command, "mv -f %s %s", tmpfname, fname );
		system(command);
        sprintf(trcBuf, "[doAddSvcTypeShm]COMMAND:%s Process Fail\n", ft->cmdName);
        trclib_writeLogErr(FL, trcBuf);
        return -1;
	}

    memset( &stCat, 0x00, sizeof(st_CategoryInfo) );

/*	
for(i=0; i<12; i++){
	fprintf(stderr, "columns[%d]:%s\n", i, columns[i]);
}
*/
    load_svc_group(columns, &stCat);

    svcGroup = stCat.ucGroup >> 4;
   
//fprintf(stderr, "%s -> %d\n", columns[1], svcGroup);
    switch( svcGroup ){
    case 1:     /* DESTIP */

        memset( &DestIP, 0x00, sizeof(DESTIP_DATA) );

        DestIP.key.ucFlag = 'D';
        uiIP = inet_addr( stCat.szCon[0] );
        iPrefix = atoi( stCat.szCon[1] );
        
        if( iPrefix < 0 || iPrefix > 32)
        {
            ret = -1;
            sprintf(trcBuf, "DESTIP iPrefix[%d] GroupID:%d",
                iPrefix, svcGroup);
            trclib_writeLogErr (FL,trcBuf);
            break;
        }
        if( strcmp(stCat.szCon[0], "255.255.255.255") !=0 &&
            uiIP == (in_addr_t)-1 )
        {
            ret = -1;
            sprintf(trcBuf, "DESTIP NET[%s:%d] GroupID:%d",
                stCat.szCon[0], uiIP, svcGroup);
            trclib_writeLogErr (FL,trcBuf);
            break;
        }
        uiNetMask = 0xffffffff << (32 - iPrefix);
        uiIP = htonl(uiIP) | ~uiNetMask;
        DestIP.key.uiIP = ntohl(uiIP);
        DestIP.uiNetmask = uiNetMask;
        DestIP.usCatID = stCat.usCategory;
        DestIP.ucGroupID = stCat.ucGroup;
        DestIP.ucSerial = stCat.szServiceID[0];
        DestIP.ucFilterOut = stCat.ucFilterOut;
        //Obsolete
        //DestIP.ucLayer = stCat.ucLayer;
        // 21 - including retransmission
        DestIP.ucLayer = 21;

        DestIP.ucURLChar = stCat.ucMode;       /* 040429 added by js */
        DestIP.ucSvcBlk = stCat.ucSvcBlk;
        DestIP.ucCDRIPFlag = stCat.szReserved[0]; // block 'CDR' must be '1'
		DestIP.ucUDRFlag = stCat.szReserved[1]; // add by helca

        pDestIP = Search_DESTIP( &DestIP.key );
        if( pDestIP == 0 )  /* not exist */
            ret = Insert_DESTIP( &DestIP );
        else
            ret = -1;
        if( ret < 0 )
        {
            ret = -1;
            sprintf(trcBuf, "pDestIP[%p] GroupID:%d ret[%d]",
                pDestIP, svcGroup, ret);
            trclib_writeLogErr (FL,trcBuf);
            break;
        }
        break;

    case 2:     /* DESTIP+TCP PORT */
    case 3:     /* DESTIP+UDP PORT */
        uiIP = inet_addr( stCat.szCon[0] );
        iPrefix = atoi( stCat.szCon[1] );
        if( iPrefix < 0 || iPrefix > 32)
        {
            ret = -1;
            sprintf(trcBuf, "DESTPORT iPrefix[%d] GroupID:%d",
                iPrefix, svcGroup);
            trclib_writeLogErr (FL,trcBuf);
            break;
        }

        if( strcmp(stCat.szCon[0], "255.255.255.255") !=0 &&
            uiIP == (in_addr_t)-1 )
        {
            ret = -1;
            sprintf(trcBuf, "DESTPORT NET[%s:%d] GroupID:%d",
                stCat.szCon[0], uiIP, svcGroup);
            trclib_writeLogErr (FL,trcBuf);
            break;
        }

        uiNetMask = 0xffffffff << (32 - iPrefix);

        memset( &DestPort, 0x00, sizeof(DESTPORT_DATA) );
        uiIP = htonl(uiIP) | ~uiNetMask;
        DestPort.key.uiDestIP = ntohl(uiIP);
        if( svcGroup == 2 )
            DestPort.key.ucProtocol = 6;    /* TCP */
        else
            DestPort.key.ucProtocol = 17;   /* UDP */

        DestPort.key.usDestPort = atoi( stCat.szCon[2] );

        DestPort.uiNetmask = uiNetMask;
        DestPort.usCatID = stCat.usCategory;
        DestPort.ucGroupID = stCat.ucGroup;
        DestPort.ucSerial = stCat.szServiceID[2];
        DestPort.ucFilterOut = stCat.ucFilterOut;
        //Obsolete
        //DestPort.ucLayer = stCat.ucLayer;
        // 21 means including retransmission.
        DestPort.ucLayer = 21;

        DestPort.ucURLChar = stCat.ucMode;       /* 040429 added by js */
        DestPort.ucSvcBlk = stCat.ucSvcBlk;
        DestPort.ucCDRIPFlag = stCat.szReserved[0];
		DestPort.ucUDRFlag = stCat.szReserved[1]; // add by helca

        pDestPort = Search_DESTPORT( &DestPort.key );
        if( pDestPort == 0 )    /* not exist */
            ret = Insert_DESTPORT( &DestPort );
        else
            ret = -1;
        if( ret < 0 )
        {
            ret = -1;
            sprintf(trcBuf, "pDestPort[%p] GroupID:%d ret[%d]\n",
                pDestPort, svcGroup, ret);
            trclib_writeLogErr (FL,trcBuf);
            break;
        }

		/* svcType Hash add */
		/* add by helca 2008.11.19 */
		pstHashNode = hasho_add(pstHashSvcType,(unsigned char *)&pstKey,(unsigned char *)&pstData);
		  
		if(pstHashNode == NULL) {
			sprintf(trcBuf, "[doAddSvcTypeShm] hasho_add fail\n");
			trclib_writeLogErr (FL,trcBuf);
	    	break;
		} else {
			pstData = (pst_SvcType_HashData)HASHO_PTR( pstHashSvcType, pstHashNode->offset_Data );
		
			pstData->uiNetMask = DestPort.uiNetmask;
			pstData->usCatID = DestPort.usCatID;
			pstData->ucGroupID = DestPort.ucGroupID;
			pstData->ucSerial = DestPort.ucSerial;
			pstData->ucFilterOut = DestPort.ucFilterOut;
			pstData->ucLayer = DestPort.ucLayer;
			pstData->ucMode = DestPort.ucMode;
			pstData->ucSvcBlk = DestPort.ucSvcBlk;
			pstData->ucIPType = DestPort.ucIPType;
			pstData->ucURLChar = DestPort.ucURLChar;
			pstData->ucCDRIPFlag = DestPort.ucCDRIPFlag;
			pstData->ucUDRFlag = DestPort.ucUDRFlag;

		}
		/****************************************************************************************/

        break;

    default:
        sprintf(trcBuf, "Undefined GroupID:%d", svcGroup );
        trclib_writeLogErr (FL,trcBuf);
        break;
    }

    if( ret < 0 )
    {
        //file delete
        sprintf (command, "mv -f %s %s", tmpfname, fname );
        system (command);
        //sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = SHARED MEMORY ADD ERROR!\n");
       	sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = ALREADY EXIST (DIFFRENT SVC_TYPE)\n"); 
		MMCResSnd(rxIxpcMsg, txBuf, 1, 0);
        sprintf(trcBuf, "[doAddSvcTypeShm]COMMAND:%s Process Fail\n", ft->cmdName);
        trclib_writeLogErr(FL, trcBuf);
    }else {

        sprintf(txBuf, "\n    RESULT = SUCCESS\n    REASON = INFOMATION ADD SUCCESS");
        addHeadMessage( mmcHdlr, titleMag );
        addBodyLineSvcMessage( mmcHdlr, columns, tmpMag, 2 );
        strcat(bodyMag, tmpMag );
        addTailMessage( mmcHdlr, totalBuffer );

        strcat(txBuf, titleMag );       //title
        strcat(txBuf, bodyMag);          // body
        strcat(txBuf, totalBuffer);    //------------

        MMCResSnd(rxIxpcMsg, txBuf, 0, 0);
		unlink(tmpfname);

        genQMsgType.mtype = MTYPE_BSD_CONFIG;
        memcpy ((void*)genQMsgType.body, mcc, sizeof(MpConfigCmd) );
        if ( ft->mode == 0 )
            SendAppNoty(&genQMsgType);
        else if ( ft->mode == 1 )
            oldSendAppNoty(1, DEF_SYS, SID_CHG_INFO, mcc->cmdType);

        conf_file_sync(ft->confFile);
        sprintf(trcBuf, "[doAddSvcTypeShm]COMMAND:%s Process Success\n", ft->cmdName);
        trclib_writeLogErr(FL, trcBuf);
	}
	    
	return 1;

}

int doChgSvcTypeShm(void *tbl, IxpcQMsgType *rxIxpcMsg,
                  GeneralQMsgType genQMsgType, void *pft,
                  MpConfigCmd *mcc, DLinkedList *head, DLinkedList *tail,
                  char columns[MAX_COL_COUNT][MAX_COL_SIZE],char  fname[FILESIZE])
{
    int ret, svcGroup;
    int iPrefix ;
   	//int error_index = 0; 
	//DLinkedList *node;
    st_CategoryInfo     stCat;
    DESTIP_KEY  DestIPKey1, DestIPKey2;
    DESTIP_DATA DestIP, *pDestIP;
    DESTPORT_DATA   DestPort, *pDestPort;
    DESTPORT_KEY    DestPortKey1, DestPortKey2;

    unsigned int    uiIP, uiNetMask;
    char command[FILESIZE];
    char tmpfname[FILESIZE];
    char trcBuf[BUFSIZE];
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

    memset( trcBuf, 0, sizeof( trcBuf ) );

    //tmpfile create
    memset( command, 0, sizeof( command ) );
    memset( tmpfname, 0, sizeof( tmpfname ) );
    strcpy( tmpfname, fname );
    strcat( tmpfname, ".tmp" );
    sprintf (command, "cp -rf %s %s", fname, tmpfname );
    system (command);

    // add by helca 2007.02.15
	// Layer=IP인 경우 Port=0이외의 값 입력시 에러처리
	if (!strcasecmp(columns[1], "IP") && atoi(columns[5]) != 0){
	     sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = PORT INPUT ERROR (The Number of Port must be 0 in case IP Layer)\n");
	     MMCResSnd(rxIxpcMsg, txBuf, 1, 0);
	     return -1;
	} 

	if ((!strcasecmp(columns[1], "TCP") || !strcasecmp(columns[1], "UDP")) && atoi(columns[5]) == 0 ) {
        sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = PORT INPUT ERROR (The Number of Port must not 0 in case TCP,UDP Layer)\n");
        MMCResSnd(rxIxpcMsg, txBuf, 1, 0);
        return -1;
    }	
	// change file
    if( doChgSvcProcess( tbl, rxIxpcMsg, genQMsgType, pft, mcc, head,tail, columns, fname )<0 ){
		sprintf (command, "mv -f %s %s", tmpfname, fname );
		system(command);
        sprintf(trcBuf, "[doChgSvcTypeShm]COMMAND:%s Process Fail\n", ft->cmdName);
        trclib_writeLogErr(FL, trcBuf);
        return -1;
	}


    memset( &stCat, 0x00, sizeof(st_CategoryInfo) );
    load_svc_group(origincolumns, &stCat);

    svcGroup = stCat.ucGroup >> 4;

    switch( svcGroup ){
    case 1:     /* DESTIP */

        create_IPDATA_key(origincolumns, &DestIPKey1);
        pDestIP = Search_DESTIP(&DestIPKey1);
                if(pDestIP)
                {
                    memcpy(&DestIPKey2, &(pDestIP->key), sizeof(DESTIP_KEY));
                    ret = Delete_DESTIP( &DestIPKey1 );

                }else{
                    // Handling the exception - Not found;
                    ret=-1;
                }
        break;
    case 2:
    case 3:
        create_PORTDATA_key(origincolumns, &DestPortKey1);
        pDestPort = Search_DESTPORT(&DestPortKey1);
                if(pDestPort)
                {
                    memcpy(&DestPortKey2, &(pDestPort->key), sizeof(DESTPORT_KEY));
                    ret = Delete_DESTPORT( &DestPortKey1 );

                }else{
                    // Handling the exception - Not found;
                    ret=-1;
                }
        break;

    default:
        sprintf(trcBuf, "Undefined GroupID:%d", svcGroup );
        trclib_writeLogErr (FL,trcBuf);
        break;
    }


   //insert
    memset( &stCat, 0x00, sizeof(st_CategoryInfo) );

    load_svc_group(columns, &stCat);

    svcGroup = stCat.ucGroup >> 4;
//fprintf(stderr, "%s -> %d\n", columns[1], svcGroup);
	switch( svcGroup ){
    case 1:     /* DESTIP */

        memset( &DestIP, 0x00, sizeof(DESTIP_DATA) );

        DestIP.key.ucFlag = 'D';
        uiIP = inet_addr( stCat.szCon[0] );
        iPrefix = atoi( stCat.szCon[1] );
        if( iPrefix < 0 || iPrefix > 32)
        {
            ret = -1;
            sprintf(trcBuf, "DESTIP iPrefix[%d] GroupID:%d",
                iPrefix, svcGroup);
            trclib_writeLogErr (FL,trcBuf);
            break;
        }
        if( strcmp(stCat.szCon[0], "255.255.255.255") !=0 &&
            uiIP == (in_addr_t)-1 )
        {
            ret = -1;
            sprintf(trcBuf, "DESTIP NET[%s:%d] GroupID:%d",
                stCat.szCon[0], uiIP, svcGroup);
            trclib_writeLogErr (FL,trcBuf);
            break;
        }
        uiNetMask = 0xffffffff << (32 - iPrefix);
        uiIP = htonl(uiIP) | ~uiNetMask;
        DestIP.key.uiIP = ntohl(uiIP);
        DestIP.uiNetmask = uiNetMask;
        DestIP.usCatID = stCat.usCategory;
        DestIP.ucGroupID = stCat.ucGroup;
        DestIP.ucSerial = stCat.szServiceID[0];
        DestIP.ucFilterOut = stCat.ucFilterOut;
        //Obsolete
        //DestIP.ucLayer = stCat.ucLayer;
        // 21 - including retransmission
        DestIP.ucLayer = 21;

        DestIP.ucURLChar = stCat.ucMode;       /* 040429 added by js */
        DestIP.ucSvcBlk = stCat.ucSvcBlk;
        DestIP.ucCDRIPFlag = stCat.szReserved[0];
		DestIP.ucUDRFlag = stCat.szReserved[1]; // add by helca

        pDestIP = Search_DESTIP( &DestIP.key );
        if( pDestIP == 0 )  /* not exist */
            ret = Insert_DESTIP( &DestIP );
        else
            ret = -1;
        if( ret < 0 )
        {
            ret = -1;
            sprintf(trcBuf, "pDestIP[%p] GroupID:%d ret[%d]",
                pDestIP, svcGroup, ret);
            trclib_writeLogErr (FL,trcBuf);
            break;
        }
        break;

    case 2:     /* DESTIP+TCP PORT */
    case 3:     /* DESTIP+UDP PORT */
        uiIP = inet_addr( stCat.szCon[0] );
        iPrefix = atoi( stCat.szCon[1] );
        if( iPrefix < 0 || iPrefix > 32)
        {
            ret = -1;
            sprintf(trcBuf, "DESTPORT iPrefix[%d] GroupID:%d",
                iPrefix, svcGroup);
            trclib_writeLogErr (FL,trcBuf);
            break;
        }

        if( strcmp(stCat.szCon[0], "255.255.255.255") !=0 &&
            uiIP == (in_addr_t)-1 )
        {
            ret = -1;
            sprintf(trcBuf, "DESTPORT NET[%s:%d] GroupID:%d",
                stCat.szCon[0], uiIP, svcGroup);
            trclib_writeLogErr (FL,trcBuf);
            break;
        }

        uiNetMask = 0xffffffff << (32 - iPrefix);

        memset( &DestPort, 0x00, sizeof(DESTPORT_DATA) );
        uiIP = htonl(uiIP) | ~uiNetMask;
        DestPort.key.uiDestIP = ntohl(uiIP);
        if( svcGroup == 2 )
            DestPort.key.ucProtocol = 6;    /* TCP */
        else
            DestPort.key.ucProtocol = 17;   /* UDP */

        DestPort.key.usDestPort = atoi( stCat.szCon[2] );

        DestPort.uiNetmask = uiNetMask;
        DestPort.usCatID = stCat.usCategory;
        DestPort.ucGroupID = stCat.ucGroup;
        DestPort.ucSerial = stCat.szServiceID[2];
        DestPort.ucFilterOut = stCat.ucFilterOut;
        //Obsolete
        //DestPort.ucLayer = stCat.ucLayer;
        // 21 means including retransmission.
        DestPort.ucLayer = 21;

        DestPort.ucURLChar = stCat.ucMode;       /* 040429 added by js */
        DestPort.ucSvcBlk = stCat.ucSvcBlk;
        DestPort.ucCDRIPFlag = stCat.szReserved[0];
		DestPort.ucUDRFlag = stCat.szReserved[1]; // add by helca

        pDestPort = Search_DESTPORT( &DestPort.key );
        if( pDestPort == 0 )    /* not exist */
            ret = Insert_DESTPORT( &DestPort );
        else
            ret = -1;
        if( ret < 0 )
        {
            ret = -1;
            sprintf(trcBuf, "pDestPort[%p] GroupID:%d ret[%d]",
                pDestPort, svcGroup, ret);
            trclib_writeLogErr (FL,trcBuf);
            break;
        }

        break;

    default:
        sprintf(trcBuf, "Undefined GroupID:%d", svcGroup );
        trclib_writeLogErr (FL,trcBuf);
        break;
    }

   
    if( ret < 0 )
    {
        //file rollback
        sprintf (command, "mv -f %s %s", tmpfname, fname );
        system (command);
        sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = SHARED MEMORY CHANGE ERROR!\n");
        MMCResSnd(rxIxpcMsg, txBuf, 1, 0);
        sprintf(trcBuf, "[doChgSvcTypeShm]COMMAND:%s Process Fail\n", ft->cmdName);
        trclib_writeLogErr(FL, trcBuf);
    }else {

        sprintf(txBuf, "\n    RESULT = SUCCESS\n    REASON = INFOMATION CHANGE SUCCESS");
        addHeadMessage( mmcHdlr, titleMag );
        addBodyLineSvcMessage( mmcHdlr, origincolumns, tmpMag, 1 );
        strcat(bodyMag, tmpMag );
        memset(tmpMag, 0, sizeof(tmpMag));
        addBodyLineSvcMessage( mmcHdlr, columns, tmpMag, 2 );
        strcat(bodyMag, tmpMag );
        addTailMessage( mmcHdlr, totalBuffer );

        strcat(txBuf, titleMag );       //title
        strcat(txBuf, bodyMag);          // body
        strcat(txBuf, totalBuffer);    //------------

        MMCResSnd(rxIxpcMsg, txBuf, 0, 0);
		unlink(tmpfname);

        genQMsgType.mtype = MTYPE_BSD_CONFIG;
        memcpy ((void*)genQMsgType.body, mcc, sizeof(MpConfigCmd) );
        if ( ft->mode == 0 )
            SendAppNoty(&genQMsgType);
        else if ( ft->mode == 1 )
            oldSendAppNoty(1, DEF_SYS, SID_CHG_INFO, mcc->cmdType);

        conf_file_sync(ft->confFile);
        sprintf(trcBuf, "[doChgSvcTypeShm]COMMAND:%s Process Success\n", ft->cmdName);
        trclib_writeLogErr(FL, trcBuf);
    }
    
	return 1;
}

int doDelSvcTypeShm(void *tbl, IxpcQMsgType *rxIxpcMsg,
                  GeneralQMsgType genQMsgType, void *pft,
                  MpConfigCmd *mcc, DLinkedList *head, DLinkedList *tail,
                  char columns[MAX_COL_COUNT][MAX_COL_SIZE],char  fname[FILESIZE])
{
    int ret, svcGroup;
    //int found,iPrefix;
    //short sequence = 0;

    //DLinkedList *node;
    st_CategoryInfo     stCat;
    DESTIP_KEY  DestIPKey1, DestIPKey2;
    DESTIP_DATA *pDestIP;
    DESTPORT_DATA   *pDestPort;
    DESTPORT_KEY    DestPortKey1, DestPortKey2;

	pst_SvcType_HashKey     pstKey;

    //unsigned int    uiIP, uiNetMask;
    char command[FILESIZE];
    char tmpfname[FILESIZE];
    char trcBuf[BUFSIZE];
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
    memset( trcBuf, 0, sizeof( trcBuf ) );

    //tmpfile create
    memset( command, 0, sizeof( command ) );
    memset( tmpfname, 0, sizeof( tmpfname ) );
    strcpy( tmpfname, fname );
    strcat( tmpfname, ".tmp" );
    sprintf (command, "cp -rf %s %s", fname, tmpfname );
    system (command);

    if( doDelSvcProcess( tbl, rxIxpcMsg, genQMsgType, pft, mcc, head,tail, columns, fname )<0 ){
		sprintf (command, "mv -f %s %s", tmpfname, fname );
		system(command);
        sprintf(trcBuf, "[doDelSvcTypeShm]COMMAND:%s Process Fail\n", ft->cmdName);
        trclib_writeLogErr(FL, trcBuf);
        return -1;
	}

    memset( &stCat, 0x00, sizeof(st_CategoryInfo) );
    load_svc_group(columns, &stCat);

    svcGroup = stCat.ucGroup >> 4;
	switch( svcGroup ){
		case 1:     /* DESTIP */

			create_IPDATA_key(columns, &DestIPKey1);
			pDestIP = Search_DESTIP(&DestIPKey1);
			if(pDestIP)
			{
				memcpy(&DestIPKey2, &(pDestIP->key), sizeof(DESTIP_KEY));
				ret = Delete_DESTIP( &DestIPKey1 );

			}else{
				// Handling the exception - Not found;
				ret=-1;
			}
			break;
		case 2:
		case 3:
			create_PORTDATA_key(columns, &DestPortKey1);
			pDestPort = Search_DESTPORT(&DestPortKey1);
			if(pDestPort)
			{
				memcpy(&DestPortKey2, &(pDestPort->key), sizeof(DESTPORT_KEY));
				ret = Delete_DESTPORT( &DestPortKey1 );

			}else{
				// Handling the exception - Not found;
				ret=-1;
			}

			/* svcType Hash del */
			/* add by helca 2008.11.19 */
			pstKey->uiDestIP = pDestPort->key.uiDestIP;
			pstKey->usDestPort = pDestPort->key.usDestPort;
			pstKey->ucProtocol = pDestPort->key.ucProtocol;
			hasho_del(pstHashSvcType,(unsigned char *)&pstKey);
			/**************************************************/

			break;

		default:
			sprintf(trcBuf, "Undefined GroupID:%d", svcGroup );
			trclib_writeLogErr (FL,trcBuf);
			break;
	}

    if( ret < 0 )
    {
        //file delete
        sprintf (command, "mv -f %s %s", tmpfname, fname );
        system (command);
        sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = SHARED MEMORY DEL ERROR!\n");
        sprintf(trcBuf, "[doDelSvcTypeShm]COMMAND:%s Process Fail\n", ft->cmdName);
        trclib_writeLogErr(FL, trcBuf);
        MMCResSnd(rxIxpcMsg, txBuf, 1, 0);
    }else {

        sprintf(txBuf, "\n    RESULT = SUCCESS\n    REASON = INFOMATION DEL SUCCESS");
        addHeadMessage( mmcHdlr, titleMag );
        addBodyLineSvcMessage( mmcHdlr, columns, tmpMag, 1 );
        strcat(bodyMag, tmpMag );
        addTailMessage( mmcHdlr, totalBuffer );
        strcat(txBuf, titleMag );       //title
        strcat(txBuf, bodyMag);          // body
        strcat(txBuf, totalBuffer);
        MMCResSnd(rxIxpcMsg, txBuf, 0, 0);
		unlink(tmpfname);

        genQMsgType.mtype = MTYPE_BSD_CONFIG;
        memcpy ((void*)genQMsgType.body, mcc, sizeof(MpConfigCmd) );
        if ( ft->mode == 0 )
            SendAppNoty(&genQMsgType);
        else if ( ft->mode == 1 )
            oldSendAppNoty(1, DEF_SYS, SID_CHG_INFO, mcc->cmdType);

        conf_file_sync(ft->confFile);
        sprintf(trcBuf, "[doDelSvcTypeShm]COMMAND:%s Process Success\n", ft->cmdName);
        trclib_writeLogErr(FL, trcBuf);
    }

	return 1;
}

int create_PORTDATA_key(char columns[MAX_COL_COUNT][MAX_COL_SIZE],
                         DESTPORT_KEY *key)
{
    //int     svctype;
    unsigned int uiIP, uiNetmask;

    memset(key, 0x00, sizeof(DESTPORT_DATA));

    switch(get_svc_group(columns[1])){
        case 2:
            key->ucProtocol = 6;
            break;
        case 3:
            key->ucProtocol = 17;
            break;
        default:
            // do exception handle
            break;
    }

    uiNetmask = 0xffffffff << (32 - atoi(columns[4]));
    uiIP = inet_addr(columns[3]);
    uiIP = htonl(uiIP) | ~uiNetmask;
    key->uiDestIP = ntohl(uiIP);

    key->usDestPort = atoi(columns[5]);
	
	return 1;
}

int create_IPDATA_key(char columns[MAX_COL_COUNT][MAX_COL_SIZE],
                         DESTIP_KEY *key)
{
    //int     svctype;
    unsigned int uiIP, uiNetmask;

    memset(key, 0x00, sizeof(DESTIP_DATA));

    key->ucFlag = 'D';

    uiNetmask = 0xffffffff << (32 - atoi(columns[4]));
    uiIP = inet_addr(columns[3]);
    uiIP = htonl(uiIP) | ~uiNetmask;
    key->uiIP = ntohl(uiIP);

	return 1;
}
