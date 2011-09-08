#include "mmcr.h"

extern char             trcBuf[TRCBUF_LEN], trcTmp[TRCTMP_LEN];
//// jjinri 0625extern st_PPS_CONF      *ppsconf;

int doSetOneProcess( void *, IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,void *,MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE], char fname[FILESIZE] );

int doSetOneProcess( void *tbl, IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,
                     void *pft,MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail,
                     char columns[MAX_COL_COUNT][MAX_COL_SIZE], char fname[FILESIZE] )
{
    DLinkedList *node;
    char    bodyMag[MMCMSGSIZE], totalBuffer[BUFSIZE], titleMag[BUFSIZE],tmpMag[BUFSIZE], txBuf[MMCMSGSIZE];
    int i=0;
    pMmcFuncTable ft;
    pMmcMemberTable mmcHdlr;
    ft=(pMmcFuncTable)pft;

    mmcHdlr=(pMmcMemberTable)tbl;
    memset(txBuf, 0, sizeof(txBuf));
    memset(tmpMag, 0, sizeof(tmpMag));
    memset(bodyMag, 0, sizeof(bodyMag));
    memset(totalBuffer, 0, sizeof(totalBuffer));
    memset(titleMag, 0, sizeof(titleMag));

    node = head;
    //이미 존재하는지 체크
        if( (node=next_conf_node(node)) != NULL){
            //node change
            addHeadMessage( mmcHdlr, titleMag );
            addBodyLineMessage( mmcHdlr, node->item.columns, tmpMag, 1 );
            strcat(bodyMag, tmpMag );

            for(i = 0; i < ft->paraCnt; i++){
                if( strlen( mcc->cmdPara[i]) > 0 )
                strncpy(node->item.columns[i], mcc->cmdPara[i], MAX_COL_SIZE);
            }
            fill_line_buff(node->item.columns, node->item.line, ft->paraCnt);
            //file insert
            if ( flush_list_to_file( head, fname ) < 0 ) {
                fprintf (stdout, "file update fail[%s]; err=%d(%s)\n", fname, errno, strerror(errno));
                sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = INFOMATION SET FAIL\n");
                MMCResSnd(rxIxpcMsg, txBuf, 1, 0);
                sprintf(trcBuf, "[doSetOneProcesss]COMMAND:%s Process Fail\n", ft->cmdName);
                trclib_writeLogErr(FL, trcBuf);
                return -1;
            } else {
                sprintf(txBuf, "\n    RESULT = SUCCESS\n    REASON = INFOMATION SET SUCCESS");
                memset(tmpMag, 0, sizeof(tmpMag));
                addBodyLineMessage( mmcHdlr, node->item.columns, tmpMag, 2 );
                strcat(bodyMag, tmpMag );
                addTailMessage( mmcHdlr, totalBuffer );

                strcat(txBuf, titleMag );       //title
                strcat(txBuf, bodyMag);          // body
                strcat(txBuf, totalBuffer);    //------------

                MMCResSnd(rxIxpcMsg, txBuf, 0, 0);
            }
        } else {
                sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = NOT FOUND INFOMATION FAIL\n");
                MMCResSnd(rxIxpcMsg, txBuf, 1, 0);
                sprintf(trcBuf, "[doSetOneProcesss]COMMAND:%s Process Fail\n", ft->cmdName);
                trclib_writeLogErr(FL, trcBuf);
                return -1;
        }


    genQMsgType.mtype = MTYPE_BSD_CONFIG;
    memcpy ((void*)genQMsgType.body, mcc, sizeof(MpConfigCmd) );
    if ( ft->mode == 0 )
        SendAppNoty(&genQMsgType);
    else if ( ft->mode == 1 )
        oldSendAppNoty(1, DEF_SYS, SID_CHG_INFO, mcc->cmdType);

    conf_file_sync(ft->confFile);
    sprintf(trcBuf, "[doSetOneProcesss]COMMAND:%s Process Success\n", ft->cmdName);
    trclib_writeLogErr(FL, trcBuf);

    return 1;
}

int doSetCommProcess( void *tbl, IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,void *pft,MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE], char fname[FILESIZE] )
{
    DLinkedList *node;
    char    bodyMag[MMCMSGSIZE], totalBuffer[BUFSIZE], titleMag[BUFSIZE],tmpMag[BUFSIZE], txBuf[MMCMSGSIZE];
    int i=0, j=0;
    pMmcFuncTable ft;
    pMmcMemberTable mmcHdlr;
    ft=(pMmcFuncTable)pft;

    mmcHdlr=(pMmcMemberTable)tbl;
    memset(txBuf, 0, sizeof(txBuf));
    memset(tmpMag, 0, sizeof(tmpMag));
    memset(bodyMag, 0, sizeof(bodyMag));
    memset(totalBuffer, 0, sizeof(totalBuffer));
    memset(titleMag, 0, sizeof(titleMag));

    node = head;
    //이미 존재하는지 체크
        if( (node=search_list_node(columns,ft->startIndex, ft->keyCnt, head)) != NULL){
            //node change
            addHeadMessage( mmcHdlr, titleMag );
            addBodyLineMessage( mmcHdlr, node->item.columns, tmpMag, 1 );
            strcat(bodyMag, tmpMag );

            for(i = 0; i < ft->paraCnt; i++){
                if( strlen( mcc->cmdPara[i]) > 0 )
                strncpy(node->item.columns[i], mcc->cmdPara[i], MAX_COL_SIZE);
           	    for(j=0; j<strlen(node->item.columns[i]); j++) {
					node->item.columns[i][j] = toupper(node->item.columns[i][j]);	
				    //fprintf(stderr, "node->item.columns[i]: %s\n", node->item.columns[i]);  // by helca 
				}	
			}
            fill_line_buff(node->item.columns, node->item.line, ft->paraCnt);
            //file insert
            if ( flush_list_to_file( head, fname ) < 0 ) {
                fprintf (stdout, "file update fail[%s]; err=%d(%s)\n", fname, errno, strerror(errno));
                sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = INFOMATION SET FAIL\n");
                MMCResSnd(rxIxpcMsg, txBuf, 1, 0);
                sprintf(trcBuf, "[doSetComProcesss]COMMAND:%s Process Fail\n", ft->cmdName);
                trclib_writeLogErr(FL, trcBuf);
                return -1;
            } else {
                sprintf(txBuf, "\n    RESULT = SUCCESS\n    REASON = INFOMATION SET SUCCESS");
                memset(tmpMag, 0, sizeof(tmpMag));
                addBodyLineMessage( mmcHdlr, node->item.columns, tmpMag, 2 );
                strcat(bodyMag, tmpMag );
                addTailMessage( mmcHdlr, totalBuffer );

                strcat(txBuf, titleMag );       //title
                strcat(txBuf, bodyMag);          // body
                strcat(txBuf, totalBuffer);    //------------

                MMCResSnd(rxIxpcMsg, txBuf, 0, 0);
            }
        } else {
                sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = NOT FOUND INFOMATION FAIL\n");
                MMCResSnd(rxIxpcMsg, txBuf, 1, 0);
                sprintf(trcBuf, "[doSetComProcesss]COMMAND:%s Process Fail\n", ft->cmdName);
                trclib_writeLogErr(FL, trcBuf);
                return -1;
        }


    genQMsgType.mtype = MTYPE_BSD_CONFIG;
    memcpy ((void*)genQMsgType.body, mcc, sizeof(MpConfigCmd) );
    if ( ft->mode == 0 )
        SendAppNoty(&genQMsgType);
    else if ( ft->mode == 1 )
        oldSendAppNoty(1, DEF_SYS, SID_CHG_INFO, mcc->cmdType);

    conf_file_sync(ft->confFile);
    sprintf(trcBuf, "[doSetComProcesss]COMMAND:%s Process Success\n", ft->cmdName);
    trclib_writeLogErr(FL, trcBuf);
    return 1;
}

#if 0 // jjinri 0625
int doSetDupProcesss( void *tbl, IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,void *pft,MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE], char fname[FILESIZE] )
{
    DLinkedList *node;
    char    bodyMag[MMCMSGSIZE], totalBuffer[BUFSIZE], titleMag[BUFSIZE],tmpMag[BUFSIZE], txBuf[MMCMSGSIZE];
    int i=0;
    pMmcFuncTable ft;
    pMmcMemberTable mmcHdlr;
    ft=(pMmcFuncTable)pft;

    mmcHdlr=(pMmcMemberTable)tbl;
    memset(txBuf, 0, sizeof(txBuf));
    memset(tmpMag, 0, sizeof(tmpMag));
    memset(bodyMag, 0, sizeof(bodyMag));
    memset(totalBuffer, 0, sizeof(totalBuffer));
    memset(titleMag, 0, sizeof(titleMag));

    node = head;
    //이미 존재하는지 체크
        if( (node=next_conf_node(node)) != NULL){
            //node change
            addHeadMessage( mmcHdlr, titleMag );
            addBodyLineMessage( mmcHdlr, node->item.columns, tmpMag, 1 );
            strcat(bodyMag, tmpMag );

            for(i = 0; i < ft->paraCnt; i++){
                if( strlen( mcc->cmdPara[i]) > 0 )
                strncpy(node->item.columns[i], mcc->cmdPara[i], MAX_COL_SIZE);
            }
            fill_line_buff(node->item.columns, node->item.line, ft->paraCnt+3);
            //file insert
            if ( flush_list_to_file( head, fname ) < 0 ) {
                fprintf (stdout, "file update fail[%s]; err=%d(%s)\n", fname, errno, strerror(errno));
                sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = INFOMATION SET FAIL\n");
                MMCResSnd(rxIxpcMsg, txBuf, 1, 0);
                sprintf(trcBuf, "[doSetDupProcesss]COMMAND:%s Process Fail\n", ft->cmdName);
                trclib_writeLogErr(FL, trcBuf);
                return -1;
            } else {
                sprintf(txBuf, "\n    RESULT = SUCCESS\n    REASON = INFOMATION SET SUCCESS");

                memset(tmpMag, 0, sizeof(tmpMag));
                addBodyLineMessage( mmcHdlr, node->item.columns, tmpMag, 2 );
                strcat(bodyMag, tmpMag );
                addTailMessage( mmcHdlr, totalBuffer );

                strcat(txBuf, titleMag );       //title
                strcat(txBuf, bodyMag);          // body
                strcat(txBuf, totalBuffer);    //------------

                MMCResSnd(rxIxpcMsg, txBuf, 0, 0);
            }

        } else {
                sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = NOT FOUND INFOMATION FAIL\n");
                MMCResSnd(rxIxpcMsg, txBuf, 1, 0);
                sprintf(trcBuf, "[doSetDupProcesss]COMMAND:%s Process Fail\n", ft->cmdName);
                trclib_writeLogErr(FL, trcBuf);
                return -1;
        }


    genQMsgType.mtype = MTYPE_BSD_CONFIG;
    memcpy ((void*)genQMsgType.body, mcc, sizeof(MpConfigCmd) );
    if ( ft->mode == 0 )
        SendAppNoty(&genQMsgType);
    else if ( ft->mode == 1 )
        oldSendAppNoty(1, DEF_SYS, SID_CHG_INFO, mcc->cmdType);

    conf_file_sync(ft->confFile);
    sprintf(trcBuf, "[doSetDupProcesss]COMMAND:%s Process Success\n", ft->cmdName);
    trclib_writeLogErr(FL, trcBuf);
    return 1;

}

int doSetSwtProcess( void *tbl, IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,void *pft,MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE], char fname[FILESIZE] )
{
    char    txBuf[MMCMSGSIZE];
    pMmcFuncTable ft;
    ft=(pMmcFuncTable)pft;
    int result = 0;
    // invalid check
    if( !strcasecmp( columns[0], "MIRROR") ){
        if( !strcasecmp( columns[1], "LINE_A" ) )
            result = 1;
        else if( !strcasecmp( columns[1], "LINE_B" ) )
            result = 1;
		else if( !strcasecmp( columns[1], "PACKET_A" ) )
			 result = 1;
		else if( !strcasecmp( columns[1], "PACKET_B" ) )
			 result = 1;
        else 
            result = 0;
    } else if( !strcasecmp( columns[0], "LAN") ){  
        if( !strcasecmp( columns[1], "WAP_GW" ) )
            result = 1;
        else if( !strcasecmp( columns[1], "AAA" ) )
            result = 1;
        else
            result = 0;
    } else if( !strcasecmp( columns[0], "APP" ) ){
        if( !strcasecmp( columns[1], "CAPD" ) )
            result = 1;
        else if( !strcasecmp( columns[1], "ANA" ) )
            result = 1;
        else if( !strcasecmp( columns[1], "CDR" ) )
            result = 1;
        else if( !strcasecmp( columns[1], "TRCDR" ) )
            result = 1;
        else if( !strcasecmp( columns[1], "WAP1ANA" ) )
            result = 1;
        else if( !strcasecmp( columns[1], "UAWAPANA" ) )
            result = 1;
        else if( !strcasecmp( columns[1], "WAP2ANA" ) )
            result = 1;
        else if( !strcasecmp( columns[1], "HTTPANA" ) )
            result = 1;
        else if( !strcasecmp( columns[1], "UDRGEN" ) )
            result = 1;
        else if( !strcasecmp( columns[1], "AAAIF" ) )
            result = 1;
        else if( !strcasecmp( columns[1], "SDMD" ) )
            result = 1;
        else if( !strcasecmp( columns[1], "VODSANA" ) )
            result = 1;
        else if( !strcasecmp( columns[1], "JAVANWANA" ) )
            result = 1;
        else if( !strcasecmp( columns[1], "WIPINWANA" ) )
            result = 1;
        else if( !strcasecmp( columns[1], "VTANA" ) )
		    result = 1;	
		else if( !strcasecmp( columns[1], "CDR2" ) )
			result = 1;
		else if( !strcasecmp( columns[1], "PTOPANA" ) )
			result = 1;
		else
            result = 0;

    }

    if( result == 0 ){
        sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = INVALID_PARAMETER FAIL\n");
        MMCResSnd(rxIxpcMsg, txBuf, 1, 0);
        sprintf(trcBuf, "[doSetSwtProcess]COMMAND:%s Process Fail\n", ft->cmdName);
        trclib_writeLogErr(FL, trcBuf);
        return -1;
    }

    // send
    if( doSetCommProcess( tbl, rxIxpcMsg, genQMsgType, ft, mcc, head, tail, columns, fname)>0 )
        return 1;
    else
        return -1;
  

}

int doSetUdrConfProcess( void *tbl, IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,void *pft,MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE], char fname[FILESIZE] )
{
    DLinkedList *node;
    char    bodyMag[MMCMSGSIZE], totalBuffer[BUFSIZE], titleMag[BUFSIZE],tmpMag[BUFSIZE], txBuf[MMCMSGSIZE];
    int i=0;
    pMmcFuncTable ft;
    pMmcMemberTable mmcHdlr;
    ft=(pMmcFuncTable)pft;
    char tmpcolumns[MAX_COL_SIZE];
    ft=(pMmcFuncTable)pft;

    mmcHdlr=(pMmcMemberTable)tbl;
    memset(txBuf, 0, sizeof(txBuf));
    memset(tmpMag, 0, sizeof(tmpMag));
    memset(bodyMag, 0, sizeof(bodyMag));
    memset(totalBuffer, 0, sizeof(totalBuffer));
    memset(titleMag, 0, sizeof(titleMag));

        strcpy( tmpcolumns, columns[0] );
        strcat( tmpcolumns, "/" );
        strcat( tmpcolumns, columns[1] );

//        fprintf( stderr, "colspan : %d \n", tmpcolumns );

        strcpy( columns[0], tmpcolumns );
        for( i=1; i< ft->paraCnt; i++ ) {
           strcpy( columns[i], !strcasecmp(columns[i+1],"$") == 1 ? "" : columns[i+1] );
        }


    //이미 존재하는지 체크
        if ( ( node = search_list_node(columns,ft->startIndex, ft->keyCnt, head) ) != NULL ) {
            //node change
            addHeadMessage( mmcHdlr, titleMag );
            addBodyLineMessage( mmcHdlr, node->item.columns, tmpMag, 1 );
            strcat(bodyMag, tmpMag );

            for(i = 0; i < ft->paraCnt-1; i++){
                if( strlen( columns[i] ) > 0 )
                strncpy(node->item.columns[i], columns[i], MAX_COL_SIZE);
                // In case of second columns, change all string to upper case
                if(i == 1)
                    strtoupper(node->item.columns[i]);
            }
            fill_line_buff(node->item.columns, node->item.line, ft->paraCnt-1);
            //file insert
            if ( flush_list_to_file( head, fname ) < 0 ) {
                fprintf (stdout, "file update fail[%s]; err=%d(%s)\n", fname, errno, strerror(errno));
                sprintf(txBuf, "\n      RESULT = FAIL\n    REASON = INFOMATION SET FAIL\n");
                MMCResSnd(rxIxpcMsg, txBuf, 1, 0);
                sprintf(trcBuf, "[doSetUdrConfProcess]COMMAND:%s Process Fail\n", ft->cmdName);
                trclib_writeLogErr(FL, trcBuf);
                return -1;
            } else {
                sprintf(txBuf, "\n      RESULT = SUCCESS\n    REASON = INFOMATION SET SUCCESS");
                memset(tmpMag, 0, sizeof(tmpMag));
                addBodyLineMessage( mmcHdlr, node->item.columns, tmpMag, 2 );
                strcat(bodyMag, tmpMag );
                addTailMessage( mmcHdlr, totalBuffer );

                strcat(txBuf, titleMag );       //title
                strcat(txBuf, bodyMag);          // body
                strcat(txBuf, totalBuffer);    //------------

                MMCResSnd(rxIxpcMsg, txBuf, 0, 0);
            }
        } else {
                sprintf(txBuf, "\n      RESULT = FAIL\n    REASON = NOT FOUND INFOMATION FAIL\n");
                MMCResSnd(rxIxpcMsg, txBuf, 1, 0);
                sprintf(trcBuf, "[doSetUdrConfProcess]COMMAND:%s Process Fail\n", ft->cmdName);
                trclib_writeLogErr(FL, trcBuf);
                return -1;

        }


    genQMsgType.mtype = MTYPE_BSD_CONFIG;
    memcpy ((void*)genQMsgType.body, mcc, sizeof(MpConfigCmd) );
    if ( ft->mode == 0 )
        SendAppNoty(&genQMsgType);
    else if ( ft->mode == 1 )
        oldSendAppNoty(1, DEF_SYS, SID_CHG_INFO, mcc->cmdType);

    conf_file_sync(ft->confFile);
    sprintf(trcBuf, "[doSetUdrConfProcess]COMMAND:%s Process Success\n", ft->cmdName);
    trclib_writeLogErr(FL, trcBuf);
    return 1;
}

int doSetPpsProcess( void *tbl, IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,
                     void *pft,MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail,
                     char columns[MAX_COL_COUNT][MAX_COL_SIZE], char fname[FILESIZE] )
{
    DLinkedList *node;
    char command[FILESIZE];
    char tmpfname[FILESIZE];
    char    bodyMag[MMCMSGSIZE], totalBuffer[BUFSIZE], titleMag[BUFSIZE],tmpMag[BUFSIZE], txBuf[MMCMSGSIZE];
    int i=0;
    pMmcFuncTable ft;
    pMmcMemberTable mmcHdlr;
    ft=(pMmcFuncTable)pft;

    mmcHdlr=(pMmcMemberTable)tbl;
    memset(txBuf, 0, sizeof(txBuf));
    memset(tmpMag, 0, sizeof(tmpMag));
    memset(bodyMag, 0, sizeof(bodyMag));
    memset(totalBuffer, 0, sizeof(totalBuffer));
    memset(titleMag, 0, sizeof(titleMag));

    if((strlen(mcc->cmdPara[1]) > 0) && (atoi(mcc->cmdPara[1]) > 3)){
        sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = INFOMATION ADD LIMIT FAIL\n");
        MMCResSnd(rxIxpcMsg, txBuf, 1, 0);
        sprintf(trcBuf, "[doSetPpsProcesss]COMMAND:%s Process Fail\n", ft->cmdName);
        trclib_writeLogErr(FL, trcBuf);
        return -1;
    }

    memset( command, 0, sizeof( command ) );
    memset( tmpfname, 0, sizeof( tmpfname ) );
    strcpy( tmpfname, fname );
    strcat( tmpfname, ".tmp" );
    sprintf (command, "cp -rf %s %s", fname, tmpfname );
    system (command);

    node = head;
    //이미 존재하는지 체크

    
    memset(tmpMag, 0, sizeof(tmpMag));
    if( (node=next_conf_node(node)) != NULL){
        //node change
        addHeadMessage( mmcHdlr, titleMag );
        addBodyLinePpsMessage( mmcHdlr, node->item.columns, tmpMag, 1 );
        strcat(bodyMag, tmpMag );
        for(i = 0; i < ft->paraCnt; i++){
            if( strlen( mcc->cmdPara[i]) > 0 ){
                switch(i){
                    case 0:
                    case 1:
                        strncpy(node->item.columns[i], mcc->cmdPara[i], MAX_COL_SIZE);
                        break;
                    case 2:
                        if(!strcasecmp(mcc->cmdPara[i], "ON")){
                            strcpy(node->item.columns[i], "1");
                            strcpy(mcc->cmdPara[i], "1");
                        }else{
                            strcpy(node->item.columns[i], "0");
                            strcpy(mcc->cmdPara[i], "0");
                        }
                        break;
                }
            }
        }
        memset(tmpMag, 0, sizeof(tmpMag));
        addBodyLinePpsMessage( mmcHdlr, node->item.columns, tmpMag, 2 );

        strcat(bodyMag, tmpMag );

        fill_line_buff(node->item.columns, node->item.line, ft->paraCnt);
        //file insert
        if ( flush_list_to_file( head, fname ) < 0 ) {
            fprintf (stdout, "file update fail[%s]; err=%d(%s)\n", fname, errno, strerror(errno));
            sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = INFOMATION SET FAIL\n");
            MMCResSnd(rxIxpcMsg, txBuf, 1, 0);
            sprintf(trcBuf, "[doSetPpsProcesss]COMMAND:%s Process Fail\n", ft->cmdName);
            trclib_writeLogErr(FL, trcBuf);
            sprintf (command, "mv -f %s %s", tmpfname, fname );
            system(command);
            return -1;
        } else {
            sprintf(txBuf, "\n    RESULT = SUCCESS\n    REASON = INFOMATION SET SUCCESS");
            addTailMessage( mmcHdlr, totalBuffer );

            strcat(txBuf, titleMag );       //title
            strcat(txBuf, bodyMag);          // body
            strcat(txBuf, totalBuffer);    //------------

            MMCResSnd(rxIxpcMsg, txBuf, 0, 0);
        }
    } else {
            sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = NOT FOUND INFOMATION FAIL\n");
            MMCResSnd(rxIxpcMsg, txBuf, 1, 0);
            sprintf(trcBuf, "[doSetPpsProcesss]COMMAND:%s Process Fail\n", ft->cmdName);
            trclib_writeLogErr(FL, trcBuf);
            sprintf (command, "mv -f %s %s", tmpfname, fname );
            system(command);
            return -1;
    }


    if(strlen(mcc->cmdPara[0]) > 0)
        ppsconf->uiInterimInterval = atoi(mcc->cmdPara[0]);
    if(strlen(mcc->cmdPara[1]) > 0)
        ppsconf->usMaxCnt = atoi(mcc->cmdPara[1]);
    if(strlen(mcc->cmdPara[2]) > 0)
        ppsconf->usOnOff  = atoi(mcc->cmdPara[2]);

    genQMsgType.mtype = MTYPE_BSD_CONFIG;
    memcpy ((void*)genQMsgType.body, mcc, sizeof(MpConfigCmd) );
    if ( ft->mode == 0 )
        SendAppNoty(&genQMsgType);
    else if ( ft->mode == 1 )
        oldSendAppNoty(1, DEF_SYS, SID_CHG_INFO, mcc->cmdType);

    unlink(tmpfname);

    conf_file_sync(ft->confFile);
    sprintf(trcBuf, "[doSetPpsProcesss]COMMAND:%s Process Success\n", ft->cmdName);
    trclib_writeLogErr(FL, trcBuf);

    return 1;
}

int doSetIcmpProcess( void *tbl, IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,
                     void *pft,MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail,
                     char columns[MAX_COL_COUNT][MAX_COL_SIZE], char fname[FILESIZE] )
{
    DLinkedList *node;
    char command[FILESIZE];
    char tmpfname[FILESIZE];
    char    bodyMag[MMCMSGSIZE], totalBuffer[BUFSIZE], titleMag[BUFSIZE],tmpMag[BUFSIZE], txBuf[MMCMSGSIZE];
    //int i=0, cnt=0, ret = 0;
    pMmcFuncTable ft;
    pMmcMemberTable mmcHdlr;
    ft=(pMmcFuncTable)pft;

    mmcHdlr=(pMmcMemberTable)tbl;
    memset(txBuf, 0, sizeof(txBuf));
    memset(tmpMag, 0, sizeof(tmpMag));
    memset(bodyMag, 0, sizeof(bodyMag));
    memset(totalBuffer, 0, sizeof(totalBuffer));
    memset(titleMag, 0, sizeof(titleMag));

    memset( command, 0, sizeof( command ) );
    memset( tmpfname, 0, sizeof( tmpfname ) );
    strcpy( tmpfname, fname );
    strcat( tmpfname, ".tmp" );
    sprintf (command, "cp -rf %s %s", fname, tmpfname );
    system (command);

    node = head;
    //이미 존재하는지 체크

    
    memset(tmpMag, 0, sizeof(tmpMag));
    if( (node=next_conf_node(node)) != NULL){
        //node change
        addHeadMessage( mmcHdlr, titleMag );
        addBodyLineIcmpMessage( mmcHdlr, node->item.columns, tmpMag, 1 );
        strcat(bodyMag, tmpMag );
        if(strlen(mcc->cmdPara[0]) > 0){
            if(!strcasecmp(mcc->cmdPara[0], "OFF")){
                strcpy(node->item.columns[0], "0");
                strcpy(mcc->cmdPara[0], "0");
            }else{
                strcpy(node->item.columns[0], "1");
                strcpy(mcc->cmdPara[0], "1");
            }
        }

        if(strlen(mcc->cmdPara[1]) > 0){
                strcpy(node->item.columns[1], mcc->cmdPara[1]);
        }

        memset(tmpMag, 0, sizeof(tmpMag));
        addBodyLineIcmpMessage( mmcHdlr, node->item.columns, tmpMag, 2 );
        strcat(bodyMag, tmpMag );

        fill_line_buff(node->item.columns, node->item.line, ft->paraCnt);
        //file insert
        if ( flush_list_to_file( head, fname ) < 0 ) {
            fprintf (stdout, "file update fail[%s]; err=%d(%s)\n", fname, errno, strerror(errno));
            sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = INFOMATION SET FAIL\n");
            MMCResSnd(rxIxpcMsg, txBuf, 1, 0);
            sprintf(trcBuf, "[doSetIcmpProcesss]COMMAND:%s Process Fail\n", ft->cmdName);
            trclib_writeLogErr(FL, trcBuf);
            sprintf (command, "mv -f %s %s", tmpfname, fname );
            system(command);
            return -1;
        } else {
            sprintf(txBuf, "\n    RESULT = SUCCESS\n    REASON = INFOMATION SET SUCCESS");
            addTailMessage( mmcHdlr, totalBuffer );

            strcat(txBuf, titleMag );       //title
            strcat(txBuf, bodyMag);          // body
            strcat(txBuf, totalBuffer);    //------------

            MMCResSnd(rxIxpcMsg, txBuf, 0, 0);
        }
    } else {
            sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = NOT FOUND INFOMATION FAIL\n");
            MMCResSnd(rxIxpcMsg, txBuf, 1, 0);
            sprintf(trcBuf, "[doSetIcmpProcesss]COMMAND:%s Process Fail\n", ft->cmdName);
            trclib_writeLogErr(FL, trcBuf);
            sprintf (command, "mv -f %s %s", tmpfname, fname );
            system(command);
            return -1;
    }


    if(strlen(mcc->cmdPara[0]) > 0)
        ppsconf->usFilterOut  = atoi(mcc->cmdPara[0]);
    if(strlen(mcc->cmdPara[1]) > 0)
        ppsconf->usSvcType  = atoi(mcc->cmdPara[1]);

    unlink(tmpfname);

    conf_file_sync(ft->confFile);
    sprintf(trcBuf, "[doSetIcmpProcesss]COMMAND:%s Process Success\n", ft->cmdName);
    trclib_writeLogErr(FL, trcBuf);

    return 1;
}

int doSetBcastProcess( void *tbl, IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,
		               void *pft,MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail,
		               char columns[MAX_COL_COUNT][MAX_COL_SIZE], char fname[FILESIZE] )
{
    DLinkedList *node;
	char command[FILESIZE];
	char tmpfname[FILESIZE];
	char    bodyMag[MMCMSGSIZE], totalBuffer[BUFSIZE], titleMag[BUFSIZE],tmpMag[BUFSIZE], txBuf[MMCMSGSIZE];
    //int i=0, cnt=0, ret = 0;
    pMmcFuncTable ft;
    pMmcMemberTable mmcHdlr;
    ft=(pMmcFuncTable)pft;

    mmcHdlr=(pMmcMemberTable)tbl;
    memset(txBuf, 0, sizeof(txBuf));
    memset(tmpMag, 0, sizeof(tmpMag));
    memset(bodyMag, 0, sizeof(bodyMag));
    memset(totalBuffer, 0, sizeof(totalBuffer));
    memset(titleMag, 0, sizeof(titleMag));

    memset( command, 0, sizeof( command ) );
    memset( tmpfname, 0, sizeof( tmpfname ) );
    strcpy( tmpfname, fname );
    strcat( tmpfname, ".tmp" );
    sprintf (command, "cp -rf %s %s", fname, tmpfname );
    system (command);

    node = head;
	//이미 존재하는지 체크

    memset(tmpMag, 0, sizeof(tmpMag));
    if( (node=next_conf_node(node)) != NULL){
    	//node change
	    addHeadMessage( mmcHdlr, titleMag );
	    addBodyLineIcmpMessage( mmcHdlr, node->item.columns, tmpMag, 1 );
	    strcat(bodyMag, tmpMag );
	    if(strlen(mcc->cmdPara[0]) > 0){
	    	if(!strcasecmp(mcc->cmdPara[0], "OFF")){
		        strcpy(node->item.columns[0], "0");
		        strcpy(mcc->cmdPara[0], "0");
		  	}else{
		        strcpy(node->item.columns[0], "1");
		        strcpy(mcc->cmdPara[0], "1");
		    }
		}

        if(strlen(mcc->cmdPara[1]) > 0){
            strcpy(node->item.columns[1], mcc->cmdPara[1]);
		}

		memset(tmpMag, 0, sizeof(tmpMag));
	    addBodyLineIcmpMessage( mmcHdlr, node->item.columns, tmpMag, 2 );
	    strcat(bodyMag, tmpMag );

		fill_line_buff(node->item.columns, node->item.line, ft->paraCnt);
		//file insert

		if ( flush_list_to_file( head, fname ) < 0 ) {
	        fprintf (stdout, "file update fail[%s]; err=%d(%s)\n", fname, errno, strerror(errno));
	        sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = INFOMATION SET FAIL\n");
	    	MMCResSnd(rxIxpcMsg, txBuf, 1, 0);
		    sprintf(trcBuf, "[doSetBcastProcess]COMMAND:%s Process Fail\n", ft->cmdName);
		    trclib_writeLogErr(FL, trcBuf);
		    sprintf (command, "mv -f %s %s", tmpfname, fname );
		    system(command);
		    return -1;
		} else {
			sprintf(txBuf, "\n    RESULT = SUCCESS\n    REASON = INFOMATION SET SUCCESS");
		    addTailMessage( mmcHdlr, totalBuffer );

			strcat(txBuf, titleMag );       //title
			strcat(txBuf, bodyMag);          // body
			strcat(txBuf, totalBuffer);    //------------

            MMCResSnd(rxIxpcMsg, txBuf, 0, 0);
        }
    } else {
		sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = NOT FOUND INFOMATION FAIL\n");
        MMCResSnd(rxIxpcMsg, txBuf, 1, 0);
	    sprintf(trcBuf, "[doSetBcastProcesss]COMMAND:%s Process Fail\n", ft->cmdName);
	    trclib_writeLogErr(FL, trcBuf);
	    sprintf (command, "mv -f %s %s", tmpfname, fname );
	    system(command);
	    return -1;
	}		

	if(strlen(mcc->cmdPara[0]) > 0)
        ppsconf->usBroadFilterOut  = atoi(mcc->cmdPara[0]);
    if(strlen(mcc->cmdPara[1]) > 0)
        ppsconf->usBroadSvcType  = atoi(mcc->cmdPara[1]);

    unlink(tmpfname);

    conf_file_sync(ft->confFile);
    sprintf(trcBuf, "[doSetBcastProcesss]COMMAND:%s Process Success\n", ft->cmdName);
    trclib_writeLogErr(FL, trcBuf);

    return 1;
} /* End of doSetBcastProcess */
#endif


