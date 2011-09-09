#include "mmcr.h"

extern char             trcBuf[TRCBUF_LEN], trcTmp[TRCTMP_LEN];

char origincolumns[MAX_COL_COUNT][MAX_COL_SIZE];

int doChgCommProcess( void *,IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,
                      void *,MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail,
                      char columns[MAX_COL_COUNT][MAX_COL_SIZE], char fname[FILESIZE] );

int doChgCommProcess( void *tbl,IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,
                      void *pft, MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail,
                       char columns[MAX_COL_COUNT][MAX_COL_SIZE], char fname[FILESIZE] )
{
    DLinkedList *node;
    char    bodyMag[MMCMSGSIZE], totalBuffer[BUFSIZE], titleMag[BUFSIZE],tmpMag[BUFSIZE], txBuf[MMCMSGSIZE];
    pMmcFuncTable ft;
    pMmcMemberTable mmcHdlr;
    ft=(pMmcFuncTable)pft;

	char RealTime_RDR_command[128];

    mmcHdlr=(pMmcMemberTable)tbl;
    memset(txBuf, 0, sizeof(txBuf));
    memset(tmpMag, 0, sizeof(tmpMag));
    memset(bodyMag, 0, sizeof(bodyMag));
    memset(totalBuffer, 0, sizeof(totalBuffer));
    memset(titleMag, 0, sizeof(titleMag));
    
    int i=0;
     //이미 존재하는지 체크
        if ( ( node = search_list_node(columns,ft->startIndex, ft->keyCnt, head) ) != NULL ) {
            //node change
            addHeadMessage( mmcHdlr, titleMag );
            if( ft->frmMode == COMM_FRM_MODE )
                addBodyLineMessage( mmcHdlr, node->item.columns, tmpMag, 1 );
            else if ( ft->frmMode == AAA_FRM_MODE )
                addBodyLineAaaMessage( mmcHdlr, node->item.columns, tmpMag, 1 );
            else if ( ft->frmMode == URL_FRM_MODE )
                addBodyLineUrlMessage( mmcHdlr, node->item.columns, tmpMag, 1 );
            else if ( ft->frmMode == TRC_FRM_MODE )
                addBodyLineTrcMessage( mmcHdlr, node->item.columns, tmpMag, 1 );
            else if ( ft->frmMode == PDSN_FRM_MODE )
                addBodyLinePdsnMessage( mmcHdlr, node->item.columns, tmpMag, 1 );

            strcat(bodyMag, tmpMag );

            memset(node->item.line, 0x00, sizeof(node->item.line));
            for(i = 0; i < ft->paraCnt; i++){
                if( strlen( columns[i]) > 0  )
                strncpy(node->item.columns[i], columns[i], MAX_COL_SIZE);
            }
           	if (!strcasecmp(ft->cmdName, "chg-wap-gw")) ft->paraCnt = 5;
			else if (!strcasecmp(ft->cmdName, "add-wap-gw")) ft->paraCnt = 5;
			fill_line_buff(node->item.columns, node->item.line, ft->paraCnt);
            //file insert
            if ( flush_list_to_file( head, fname ) < 0 ) {
                fprintf (stdout, "file update fail[%s]; err=%d(%s)\n", fname, errno, strerror(errno));
                sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = INFOMATION CHANGE FAIL\n");
                MMCResSnd(rxIxpcMsg, txBuf, 1, 0);
                sprintf(trcBuf, "[doChgCommProcess]COMMAND:%s Process Fail\n", ft->cmdName);
                trclib_writeLogErr(FL, trcBuf);
                return -1;
            } else {
                sprintf(txBuf, "\n    RESULT = SUCCESS\n    REASON = INFOMATION CHANGE SUCCESS");
                memset(tmpMag, 0, sizeof(tmpMag));
                if( ft->frmMode == COMM_FRM_MODE )
                    addBodyLineMessage( mmcHdlr, node->item.columns, tmpMag, 2 );
                else if ( ft->frmMode == AAA_FRM_MODE )
                    addBodyLineAaaMessage( mmcHdlr, node->item.columns, tmpMag, 2 );
                else if ( ft->frmMode == URL_FRM_MODE )
                    addBodyLineUrlMessage( mmcHdlr, node->item.columns, tmpMag, 2 );
                else if ( ft->frmMode == TRC_FRM_MODE )
                    addBodyLineTrcMessage( mmcHdlr, node->item.columns, tmpMag, 2 );
                else if ( ft->frmMode == PDSN_FRM_MODE )
                    addBodyLinePdsnMessage( mmcHdlr, node->item.columns, tmpMag, 2 );

                strcat(bodyMag, tmpMag );
                addTailMessage( mmcHdlr, totalBuffer );

                strcat(txBuf, titleMag );       //title
                strcat(txBuf, bodyMag);          // body
                strcat(txBuf, totalBuffer);    //------------
                MMCResSnd(rxIxpcMsg, txBuf, 0, 0);
            }
        } else {
            sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = NOT FOUND INFOMATION CHANGE FAIL\n");
            MMCResSnd(rxIxpcMsg, txBuf, 1, 0);
            sprintf(trcBuf, "[doChgCommProcess]COMMAND:%s Process Fail\n", ft->cmdName);
            trclib_writeLogErr(FL, trcBuf);
            return -1;
        }

    genQMsgType.mtype = MTYPE_BSD_CONFIG;

    memcpy ((void*)genQMsgType.body, mcc, sizeof(MpConfigCmd) );
    if ( ft->mode == 0 )
        SendAppNoty(&genQMsgType);
    else if ( ft->mode == 1 ){
        oldSendAppNoty(1, DEF_SYS, SID_CHG_INFO, mcc->cmdType);
        if(!strcmp(ft->cmdName, "chg-call-trc")) {
            oldSendAppNoty(2, DEF_SYS, SID_CHG_INFO, mcc->cmdType);
            oldSendAppNoty(3, DEF_SVC, SID_CHG_INFO, mcc->cmdType);

			// Trace 남기라고 SCE에 전송. 
			sprintf (RealTime_RDR_command, "sudo -u pcube /SM/pcube/sm/server/bin/p3subs --set --subscriber=%s --property=monitor=1 \n", columns[1]);
    		trclib_writeLogErr(FL, RealTime_RDR_command);
			if (system(RealTime_RDR_command) < 0) {
                sprintf(trcBuf, "[doAddCommProcess]COMMAND:%s. Fail\n", RealTime_RDR_command);
                trclib_writeLogErr(FL, trcBuf);
			}
		}
    }

    conf_file_sync(ft->confFile);
    sprintf(trcBuf, "[doChgCommProcess]COMMAND:%s Process Success\n", ft->cmdName);
    trclib_writeLogErr(FL, trcBuf);

    return 1;
}

#if 0 //// jjinri 0625
int doChgSvcProcess( void *tbl,IxpcQMsgType *rxIxpcMsg,
                       GeneralQMsgType genQMsgType,void *pft,
                       MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail,
                       char columns[MAX_COL_COUNT][MAX_COL_SIZE], char fname[FILESIZE] )
{
    DLinkedList *node;
    pMmcFuncTable ft;
    //DESTIP_KEY ipKey;
    //DESTPORT_KEY portKey;
    //int         SvcType;

    char tmpcolumns[MAX_COL_COUNT][MAX_COL_SIZE];
	char first[MAX_COL_SIZE];
	char alias[MAX_COL_SIZE];

	strncpy(first, columns[8], MAX_COL_SIZE);
	strncpy(alias, columns[9], MAX_COL_SIZE);

	if(!strcasecmp(columns[6], "CDR"))
		strcpy(columns[8], "on");
	else
		strcpy(columns[8], "off");

	strncpy(columns[9], first, MAX_COL_SIZE);
	strncpy(columns[10], alias, MAX_COL_SIZE);
	
	ft=(pMmcFuncTable)pft;

    char    txBuf[MMCMSGSIZE];

    strcpy( tmpcolumns[ft->startIndex], columns[0] );
    int i=0;
     //이미 존재하는지 체크
    if ( ( node = search_list_node(tmpcolumns,ft->startIndex, ft->keyCnt, head) ) != NULL ) {
        //node change
        memset( origincolumns, 0, sizeof( origincolumns ) );
        memset(node->item.line, 0x00, sizeof(node->item.line));
        memcpy( origincolumns, node->item.columns, sizeof( origincolumns ) );
        for(i = 1; i < ft->paraCnt; i++){
            //fprintf( stderr, "origincolumns : %s \n ", origincolumns[i] ); 
            if( strlen( columns[i]) > 0  )
                strncpy(node->item.columns[i], columns[i], MAX_COL_SIZE);

            strncpy(columns[i],node->item.columns[i], MAX_COL_SIZE);
        }

        strncpy( node->item.columns[ft->startIndex], columns[0], MAX_COL_SIZE );
        strncpy( columns[0],node->item.columns[0],MAX_COL_SIZE );
        strncpy( columns[ft->startIndex],node->item.columns[ft->startIndex],MAX_COL_SIZE );
        //fprintf( stdout, "origincolumns0 :%s, origincolumns9 : %s \n" , origincolumns[0], origincolumns[9] );
        fill_line_buff(node->item.columns, node->item.line, ft->paraCnt+1);
        //file insert
        if ( flush_list_to_file( head, fname ) < 0 ) {
            fprintf (stdout, "file update fail[%s]; err=%d(%s)\n", fname, errno, strerror(errno));
            sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = INFOMATION CHANGE FAIL\n");
            MMCResSnd(rxIxpcMsg, txBuf, 1, 0);
            return -1;
        }
    } else {
        sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = NOT FOUND INFOMATION CHANGE FAIL\n");
        MMCResSnd(rxIxpcMsg, txBuf, 1, 0);
        return -1;
    }

    if(!strcasecmp(columns[6], "WAP1ANA") && strcasecmp(columns[1], "UDP")){
        sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = WAP1ANA HAS ONLY UDP LAYER\n");
        MMCResSnd(rxIxpcMsg, txBuf, 1, 0);
        return -1;
    }
/*
    // All blocks but CDR have to only have "ON" as the value of CDRIP_FLAG.
    if(strcasecmp(columns[6], "CDR") && !strcasecmp(columns[8], "ON")){
        sprintf(txBuf, "\n    RESULT = FAIL\n"
        "    REASON = VALUE OF %s's CDRIP_FLAG HAVE TO BE OFF\n", strtoupper2(columns[6]));
        MMCResSnd(rxIxpcMsg, txBuf, 1, 0);
        return -1;
    }
*/
    return 1;
}
#endif

int doChgTrcProcess(  void *tbl,IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,void *pft, MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE],char  fname[FILESIZE])
{
    int     i;
    //char    txBuf[MMCMSGSIZE];
    //char tmpArray[MAX_COL_SIZE];
    time_t  now;
    pMmcFuncTable ft;
    ft=(pMmcFuncTable)pft;

    for(i = ft->paraCnt-2; i > 0; i--){
        strcpy(mcc->cmdPara[i], mcc->cmdPara[i-1]);
        strcpy(columns[i], columns[i-1]);
    }

    strcpy(mcc->cmdPara[0], "IMSI");
    strcpy(columns[0], "1");
/*
    memset( tmpArray, 0, sizeof( MAX_COL_SIZE ) );
    sprintf( tmpArray, "%s", ( !strcasecmp( columns[0], "IMSI" ) == 1 ? "1" : "2" ) );
    strcpy( columns[0],tmpArray );
*/
    now = time(0);
    sprintf(columns[ft->paraCnt-1], "%ld", now);
    if( doChgCommProcess( tbl, rxIxpcMsg, genQMsgType, ft, mcc, head, tail, columns, fname)>0 )
        return 1;
    else
        return -1;

}
