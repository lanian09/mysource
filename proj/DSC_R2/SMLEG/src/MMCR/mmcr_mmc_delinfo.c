#include "mmcr.h"

extern char             trcBuf[TRCBUF_LEN], trcTmp[TRCTMP_LEN];

int doDelCommProcess(void *, IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,void *,MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE], char fname[FILESIZE] );

int doDelCommProcess( void * tbl, IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,void *pft,MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE], char fname[FILESIZE] )
{
     DLinkedList *node;
     char    bodyMag[MMCMSGSIZE], totalBuffer[BUFSIZE], titleMag[BUFSIZE],tmpMag[BUFSIZE], txBuf[MMCMSGSIZE];
    pMmcMemberTable mmcHdlr;
     pMmcFuncTable ft;
    ft=(pMmcFuncTable)pft;

//// jjinri 0629	char RealTime_RDR_command[128];

    mmcHdlr=(pMmcMemberTable)tbl;
    memset(txBuf, 0, sizeof(txBuf));
    memset(tmpMag, 0, sizeof(tmpMag));
    memset(bodyMag, 0, sizeof(bodyMag));
    memset(totalBuffer, 0, sizeof(totalBuffer));
    memset(titleMag, 0, sizeof(titleMag));

     //이미 존재하는지 체크
        if ( ( node = search_list_node(columns,ft->startIndex, ft->keyCnt, head) ) != NULL ) {
            //node삭제
            addHeadMessage( mmcHdlr, titleMag );
            if( ft->frmMode == COMM_FRM_MODE )
                addBodyLineMessage( mmcHdlr, node->item.columns, tmpMag, 1 );
            else if ( ft->frmMode == AAA_FRM_MODE )
                addBodyLineAaaMessage( mmcHdlr, node->item.columns, tmpMag, 1 );
            else if ( ft->frmMode == URL_FRM_MODE )
                addBodyLineUrlMessage( mmcHdlr, node->item.columns, tmpMag, 1 );
            else if ( ft->frmMode == TRC_FRM_MODE )
                addBodyLineTrcMessage( mmcHdlr, node->item.columns, tmpMag, 1 );
           
            strcat(bodyMag, tmpMag );
            addTailMessage( mmcHdlr, totalBuffer );

            delete_list_node( node );
            //file insert
            if ( flush_list_to_file( head, fname ) < 0 ) {
                fprintf (stdout, "file save fail[%s]; err=%d(%s)\n", fname, errno, strerror(errno));
                sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = INFOMATION DEL FAIL\n");
                MMCResSnd(rxIxpcMsg, txBuf, 1, 0);
                sprintf(trcBuf, "[doDelCommProcess]COMMAND:%s Process Fail\n", ft->cmdName);
                trclib_writeLogErr(FL, trcBuf);
                return -1;
            } else {
                sprintf(txBuf, "\n    RESULT = SUCCESS\n    REASON = INFOMATION DEL SUCCESS");
                strcat(txBuf, titleMag );       //title
                strcat(txBuf, bodyMag);          // body
                strcat(txBuf, totalBuffer);
                MMCResSnd(rxIxpcMsg, txBuf, 0, 0);
            }
        } else {
            fprintf( stdout, "data not found! \n" );
            sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = NOT FOUND INFOMATION DEL FAIL\n");
            MMCResSnd(rxIxpcMsg, txBuf, 1, 0);
            sprintf(trcBuf, "[doDelCommProcess]COMMAND:%s Process Fail\n", ft->cmdName);
            trclib_writeLogErr(FL, trcBuf);
            return -1;
        }


    genQMsgType.mtype = MTYPE_BSD_CONFIG;
    memcpy ((void*)genQMsgType.body, mcc, sizeof(MpConfigCmd) );
    if ( ft->mode == 0 )
        SendAppNoty(&genQMsgType);
    else if ( ft->mode == 1 ){
        oldSendAppNoty(1, DEF_SYS, SID_CHG_INFO, mcc->cmdType);
        if(!strcmp(ft->cmdName, "canc-call-trc"))
            oldSendAppNoty(2, DEF_SYS, SID_CHG_INFO, mcc->cmdType);
            oldSendAppNoty(3, DEF_SVC, SID_CHG_INFO, mcc->cmdType);

			/* jjinri 06.29
			// Trace 지우라고  SCE에 전송. 
			sprintf (RealTime_RDR_command, "sudo -u pcube /SM/pcube/sm/server/bin/p3subs --set --subscriber=%s --property=monitor=0 \n", columns[1]);
    		trclib_writeLogErr(FL, RealTime_RDR_command);
			if (system(RealTime_RDR_command) < 0) {
                sprintf(trcBuf, "[doAddCommProcess]COMMAND:%s. Fail\n", RealTime_RDR_command);
                trclib_writeLogErr(FL, trcBuf);
			}
			*/
    }

//    conf_file_sync(ft->confFile);
    sprintf(trcBuf, "[doDelCommProcess]COMMAND:%s Process Success\n", ft->cmdName);
    trclib_writeLogErr(FL, trcBuf);

    return 1;
}

#if 0 //// 0625
int doDelIpProcess( void * tbl, IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,void *pft,MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE], char fname[FILESIZE] )
{
     DLinkedList *node;
    char    txBuf[MMCMSGSIZE];
     pMmcFuncTable ft;
    ft=(pMmcFuncTable)pft;

     //이미 존재하는지 체크
        if ( ( node = search_list_node(columns,ft->startIndex, ft->keyCnt, head) ) != NULL ) {
            //strcpy( columns[2] , node->item.columns[2]);
            //node삭제
            memcpy( columns, node->item.columns, sizeof( node->item.columns ) );
            delete_list_node( node );
            //file insert
            if ( flush_list_to_file( head, fname ) < 0 ) {
                fprintf (stdout, "file save fail[%s]; err=%d(%s)\n", fname, errno, strerror(errno));
                sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = INFOMATION DEL FAIL\n");
                MMCResSnd(rxIxpcMsg, txBuf, 1, 0);
                return -1;
            }
        } else {
            fprintf( stdout, "data not found! \n" );
            sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = NOT FOUND INFOMATION DEL FAIL\n");
            MMCResSnd(rxIxpcMsg, txBuf, 1, 0);
            return -1;
        }

    return 1;
}
#endif


int doDelTrcProcess( void * tbl, IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,void *pft,MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE], char fname[FILESIZE] )
{
    //int     i;
    //char  txBuf[MMCMSGSIZE];
    //char tmpArray[MAX_COL_SIZE];
	int j = 0, dotCnt = 0;
	char *dot = NULL;
	const int _IMSI_SIZE_ = 15;


    pMmcFuncTable ft;
    ft=(pMmcFuncTable)pft;

    strcpy(mcc->cmdPara[1], mcc->cmdPara[0]);
    strcpy(columns[1], columns[0]);

	if( (strlen(mcc->cmdPara[1]) == _IMSI_SIZE_) && (dot = strchr(mcc->cmdPara[1], '.') == NULL ) )
	{
		strcpy(mcc->cmdPara[0], "IMSI");
		strcpy(columns[0], "1");
	}
	else
	{
		for(j = 0; j < strlen(mcc->cmdPara[1]); j++ )
		{
			if( mcc->cmdPara[1][j] == '.' )
				dotCnt++;
		}
		if( strlen(mcc->cmdPara[1]) >= 7 && strlen(mcc->cmdPara[1]) <= 15 && dotCnt == 3 )
		{
			strcpy(mcc->cmdPara[0], "IP");
			strcpy(columns[0], "2");
		}
	}          

	/*
    strcpy(mcc->cmdPara[0], "IMSI");
    strcpy(columns[0], "1");
	*/

/*
    memset( tmpArray, 0, sizeof( tmpArray ) );
    sprintf( tmpArray, "%s", ( !strcasecmp( columns[0], "IMSI" ) == 1 ? "1" : "2" ) );
    strcpy( columns[0],tmpArray );
*/

    if( doDelCommProcess( tbl, rxIxpcMsg, genQMsgType, ft, mcc, head, tail, columns, fname)>0 )
        return 1;
    else
        return -1;
}

#if 0 //// 0625
int doDelPdsnProcess( void * tbl, IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,void *pft,MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE], char fname[FILESIZE] )
{
    char  txBuf[MMCMSGSIZE],inFileName[FILESIZE];
    pMmcFuncTable ft;
    DLinkedList inHead, inTail;
    DLinkedList *innode;
    DLinkedList *node;
    ft=(pMmcFuncTable)pft;

    char    *env;
    char inColumns[MAX_COL_COUNT][MAX_COL_SIZE];
    ft=(pMmcFuncTable)pft;
   
    if ((env = getenv(IV_HOME)) == NULL){
       fprintf( stdout, "getenv error! \n" );
    }

#if 0 // jjinri

    //search ip-pool
    sprintf (inFileName, "%s/%s", env, IP_POOL_FILE);

    set_head_tail(&inHead, &inTail);
    strcpy( inColumns[0], columns[0] );
    if(make_conf_list(inFileName, &inHead) < 0){
         fprintf (stdout, "make linked list fail; err=%d(%s)\n", errno, strerror(errno));
         //trclib_writeLogErr (FL,trcBuf);
         sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = FILE NOT FOUND\n");
        MMCResSnd(rxIxpcMsg, txBuf, 1, 0);
        return -1;

    } else {

#endif
        if ( ( innode = search_list_node(inColumns,0, 1, &inHead) ) != NULL ) {
            //node change
            sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = IP POOL INFOMATION EXIST\n");
            MMCResSnd(rxIxpcMsg, txBuf, 1, 0);
            return -1;
        } else {
//         delete_all_list_node(&inHead, &inTail);
             //이미 존재하는지 체크
            if ( ( node = search_list_node(columns,ft->startIndex, ft->keyCnt, head) ) != NULL ) {
                //node삭제
                memcpy( columns, node->item.columns, sizeof( node->item.columns ) );
                delete_list_node( node );
                //file insert
                if ( flush_list_to_file( head, fname ) < 0 ) {
                    fprintf (stdout, "file save fail[%s]; err=%d(%s)\n", fname, errno, strerror(errno));
                    sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = INFOMATION DEL FAIL\n");
                    MMCResSnd(rxIxpcMsg, txBuf, 1, 0);
                    return -1;
                }
            } else {
                fprintf( stdout, "data not found! \n" );
                sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = NOT FOUND INFOMATION DEL FAIL\n");
                MMCResSnd(rxIxpcMsg, txBuf, 1, 0);
                return -1;
            }

        }

// jjinri    }
   return 1; 
}

int doDelSvcProcess( void * tbl, IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,void *pft,MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE], char fname[FILESIZE] )
{
     DLinkedList *node;
    char    txBuf[MMCMSGSIZE];
     pMmcFuncTable ft;
    ft=(pMmcFuncTable)pft;
    char tmpcolumns[MAX_COL_COUNT][MAX_COL_SIZE];

    strcpy( tmpcolumns[ft->startIndex], columns[0] );
    //fprintf( stderr, "strcpy : %s , columns : %s \n", tmpcolumns[9], columns[0] );
     //이미 존재하는지 체크
        if ( ( node = search_list_node(tmpcolumns,ft->startIndex, ft->keyCnt, head) ) != NULL ) {
            //node삭제
            memcpy( columns, node->item.columns, sizeof( node->item.columns ) );
            delete_list_node( node );
            //file insert
            if ( flush_list_to_file( head, fname ) < 0 ) {
                fprintf (stdout, "file save fail[%s]; err=%d(%s)\n", fname, errno, strerror(errno));
                sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = INFOMATION DEL FAIL\n");
                MMCResSnd(rxIxpcMsg, txBuf, 1, 0);
                return -1;
            }
        } else {
            sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = NOT FOUND INFOMATION DEL FAIL\n");
            MMCResSnd(rxIxpcMsg, txBuf, 1, 0);
            return -1;
        }


    return 1;
}


int doDelSvcOptProcess( void * tbl, IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,
						void *pft,MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail,
						char columns[MAX_COL_COUNT][MAX_COL_SIZE], char fname[FILESIZE] )
{
     DLinkedList *node;
    char    txBuf[MMCMSGSIZE];
     pMmcFuncTable ft;
    ft=(pMmcFuncTable)pft;

     //이미 존재하는지 체크
        if ( ( node = search_list_node(columns,ft->startIndex, ft->keyCnt, head) ) != NULL ) {
            //strcpy( columns[2] , node->item.columns[2]);
            //node삭제
            memcpy( columns, node->item.columns, sizeof( node->item.columns ) );
            delete_list_node( node );
            //file insert
            if ( flush_list_to_file( head, fname ) < 0 ) {
                fprintf (stdout, "file save fail[%s]; err=%d(%s)\n", fname, errno, strerror(errno));
                sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = INFOMATION DEL FAIL\n");
                MMCResSnd(rxIxpcMsg, txBuf, 1, 0);
                return -1;
            }
        } else {
            fprintf( stdout, "data not found! \n" );
            sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = NOT FOUND INFOMATION DEL FAIL\n");
            MMCResSnd(rxIxpcMsg, txBuf, 1, 0);
            return -1;
        }

    return 1;
}

#endif
