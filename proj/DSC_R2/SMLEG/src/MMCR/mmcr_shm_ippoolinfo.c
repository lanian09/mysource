#include "mmcr.h"
extern char     trcBuf[TRCBUF_LEN], trcTmp[TRCTMP_LEN];
extern pst_IPPOOLLIST     pstIPPOOLBIT;

int doAddIpPoolInfoShm(void *tbl, IxpcQMsgType *rxIxpcMsg,
                  GeneralQMsgType genQMsgType, void *pft,
                  MpConfigCmd *mcc, DLinkedList *head, DLinkedList *tail,
                  char columns[MAX_COL_COUNT][MAX_COL_SIZE],char  fname[FILESIZE])
{
    int ret, rst=0;
    //int found, i, PdsnType=0;

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

    if( doAddIpProcess( tbl, rxIxpcMsg, genQMsgType, pft, mcc, head,tail, columns, fname )<0 ){
    	sprintf (command, "mv -f %s %s", tmpfname, fname);
    	system (command);
        sprintf(trcBuf, "[doAddIpPoolInfoShm]COMMAND:%s Process Fail\n", ft->cmdName);
        trclib_writeLogErr(FL, trcBuf);
        return -1;
	}

#if 0
    memset( &stIPPool, 0x00, sizeof(st_IPPool) );
    strcpy(stIPPool.szSysIP, columns[2]);
	stIPPool.iPrefix = atoi( columns[3]);
	
	for (i=0; i<strlen(columns[1]); i++) columns[1][i] = toupper(columns[1][i]);
   	strcpy(stIPPool.pdsn_type, columns[1]);
   
   	if( !strcasecmp(columns[1], "1X")) {
		PdsnType = 1;
	}else if ( !strcasecmp(columns[1], "1XEV_DO")) {
		PdsnType = 2;
	}else {
		;
	}
#endif
#if 1
	memset( &stIPPool, 0x00, sizeof(st_IPPool) );
	strcpy(stIPPool.szSysIP, columns[1]);
	stIPPool.iPrefix = atoi( columns[2]);
#endif	

	uiIP = inet_addr(stIPPool.szSysIP);
    
	if( uiIP == (in_addr_t)-1 || stIPPool.iPrefix < 0 || stIPPool.iPrefix > 32)
    {
        ret = -1;
            
    } else {

        // inet_addr converts alreay
        uiNetMask = 0xffffffff << (32 - stIPPool.iPrefix);

        // IP POOL
        memset( &DestIP, 0x00, sizeof(DESTIP_DATA) );

        DestIP.key.ucFlag = 'S';
        uiIP = htonl(uiIP) | ~uiNetMask;
        DestIP.key.uiIP = ntohl(uiIP);
        DestIP.uiNetmask = uiNetMask;
        DestIP.ucIPType = IPTYPE_IPPOOL;

		//DestIP.ucPdsnType = PdsnType; // 1:1x, 2:1xEV-DO  add by helca 2008.07.21

        pDestIP = Search_DESTIP( &DestIP.key );

        if( pDestIP == 0 )  /* not exist */
        {
            ret = Insert_DESTIP( &DestIP );
        }
        else
        {
            ret = -1;
        }
	
		/* IPPOOLBIT SET */
		/* add by helca */
		if ((rst = dSetIPPOOLList(htonl(DestIP.key.uiIP), DestIP.uiNetmask, 1, pstIPPOOLBIT)) < 0) {
			sprintf(trcBuf, "[doAddIpPoolInfoShm] IpPool bit set fail \n");
			trclib_writeLogErr(FL, trcBuf);
		}

    }

    if( ret < 0 )
    {
        //file delete
        sprintf (command, "mv -f %s %s", tmpfname, fname );
        system (command);
        sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = SHARED MEMORY ADD ERROR!\n");
        MMCResSnd(rxIxpcMsg, txBuf, 1, 0);
        sprintf(trcBuf, "[doAddIpPoolInfoShm]COMMAND:%s Process Fail\n", ft->cmdName);
        trclib_writeLogErr(FL, trcBuf);

    } else {

        sprintf(txBuf, "\n    RESULT = SUCCESS\n    REASON = INFOMATION ADD SUCCESS");
        addHeadMessage( mmcHdlr, titleMag );
        //strcat(bodyMag, tmpMag );
        //memset(tmpMag, 0, sizeof(tmpMag));
        addBodyLineMessage( mmcHdlr, columns, tmpMag, 2 );
        strcat(bodyMag, tmpMag );
        addTailMessage( mmcHdlr, totalBuffer );

        strcat(txBuf, titleMag );       //title
        strcat(txBuf, bodyMag);          // body
        strcat(txBuf, totalBuffer);    //------------

        MMCResSnd(rxIxpcMsg, txBuf, 0, 0);
		unlink(tmpfname);
        conf_file_sync(ft->confFile);
        sprintf(trcBuf, "[doAddIpPoolInfoShm]COMMAND:%s Process Success\n", ft->cmdName);
        trclib_writeLogErr(FL, trcBuf);
    }
	
	return 1;

}

int doDelIpPoolInfoShm(void *tbl, IxpcQMsgType *rxIxpcMsg,
                  GeneralQMsgType genQMsgType, void *pft,
                  MpConfigCmd *mcc, DLinkedList *head, DLinkedList *tail,
                  char columns[MAX_COL_COUNT][MAX_COL_SIZE],char  fname[FILESIZE])
{

    int ret, rst=0;
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
    
	if( doDelIpProcess( tbl, rxIxpcMsg, genQMsgType, pft, mcc, head,tail, columns[1], fname )<0 ){
    	sprintf (command, "mv -f %s %s", tmpfname, fname);
    	system (command);
        sprintf(trcBuf, "[doDelIpPoolInfoShm]COMMAND:%s Process Fail\n", ft->cmdName);
        trclib_writeLogErr(FL, trcBuf);
        return -1;
	}

    memset( &stIPPool, 0x00, sizeof(st_IPPool) );
#if 0    
	strcpy(stIPPool.szSysIP, columns[2]);
    stIPPool.iPrefix = atoi( columns[3]);
#endif
#if 1
	strcpy(stIPPool.szSysIP, columns[1]);
	stIPPool.iPrefix = atoi( columns[2]);
#endif
	//fprintf(stderr, "columns[2]: %s, columns[3]: %s \n", columns[2], columns[3]);

    //fprintf( stderr, "delip iprefix : %d, column : %s \n ", stIPPool.iPrefix, columns[2] );

    uiIP = inet_addr(stIPPool.szSysIP);
    if( uiIP == (in_addr_t)-1 )
    {
        ret = -1;

    } else {

        uiNetMask = 0xffffffff << (32 - stIPPool.iPrefix);
        // IP POOL
        memset( &DestIP, 0x00, sizeof(DESTIP_DATA) );

        DestIP.key.ucFlag = 'S';
        uiIP = htonl(uiIP) | ~uiNetMask;
        DestIP.key.uiIP = ntohl(uiIP);
        DestIP.ucIPType = IPTYPE_IPPOOL;

        pDestIP = Search_DESTIP( &DestIP.key );

        if( pDestIP == 0 )  /* not exist */
        {
            ret = -2;
        }
        else
        {
            ret = Delete_DESTIP( &DestIP.key );;
        }

		/* IPPOOLBIT SET */
		/* add by helca */
		if ((rst = dSetIPPOOLList(htonl(DestIP.key.uiIP), uiNetMask, 0, pstIPPOOLBIT)) < 0) {
		    sprintf(trcBuf, "[doAddIpPoolInfoShm] IpPool bit set fail \n");
		    trclib_writeLogErr(FL, trcBuf);
		}

    }

    if( ret < 0 )
    {
        //file delete
        sprintf (command, "mv -f %s %s", tmpfname, fname );
        system (command);
        sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = SHARED MEMORY DEL ERROR!\n");
        MMCResSnd(rxIxpcMsg, txBuf, 1, 0);
        sprintf(trcBuf, "[doDelIpPoolInfoShm]COMMAND:%s Process Fail\n", ft->cmdName);
        trclib_writeLogErr(FL, trcBuf);
    }else {

        sprintf(txBuf, "\n    RESULT = SUCCESS\n    REASON = INFOMATION DEL SUCCESS");
        addHeadMessage( mmcHdlr, titleMag );
        addBodyLineMessage( mmcHdlr, columns, tmpMag, 1 );
        strcat(bodyMag, tmpMag );
        addTailMessage( mmcHdlr, totalBuffer );
        strcat(txBuf, titleMag );       //title
        strcat(txBuf, bodyMag);          // body
        strcat(txBuf, totalBuffer);
        MMCResSnd(rxIxpcMsg, txBuf, 0, 0);
		unlink(tmpfname);
        conf_file_sync(ft->confFile);
        sprintf(trcBuf, "[doDelIpPoolInfoShm]COMMAND:%s Process Success\n", ft->cmdName);
        trclib_writeLogErr(FL, trcBuf);
    }
	
	return 1;
}
