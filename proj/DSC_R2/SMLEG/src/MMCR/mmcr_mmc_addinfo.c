#include "mmcr.h"

extern char     trcBuf[TRCBUF_LEN], trcTmp[TRCTMP_LEN];

int doAddCommProcess(  void *,IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,void *, MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE],char  fname[FILESIZE]);
int doAddLimitProcess(  void *,IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,void *, MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE],char  fname[FILESIZE]);
int doAddTrcProcess(  void *tbl,IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,void *pft, MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE],char  fname[FILESIZE]);
int doAddUdrConfProcess(  void *tbl,IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,void *pft, MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE],char  fname[FILESIZE]);

int doAddCommProcess(void *tbl,IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,void *pft, MpConfigCmd *mcc, 
		DLinkedList *head,DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE],char  fname[FILESIZE])
{
    DLinkedList *node;
    char    bodyMag[MMCMSGSIZE], totalBuffer[BUFSIZE], titleMag[BUFSIZE],tmpMag[BUFSIZE], txBuf[MMCMSGSIZE];
    pMmcFuncTable ft;
    pMmcMemberTable mmcHdlr;
    ft=(pMmcFuncTable)pft;

//// jjinri 0629	char RealTime_RDR_command[128];

    mmcHdlr=(pMmcMemberTable)tbl;
    memset(txBuf, 0, sizeof(txBuf));
    memset(tmpMag, 0, sizeof(tmpMag));
    memset(bodyMag, 0, sizeof(bodyMag));
    memset(totalBuffer, 0, sizeof(totalBuffer));
    memset(titleMag, 0, sizeof(titleMag));

     //이미 존재하는지 체크
        if ( (node = search_list_node(columns, ft->startIndex, ft->keyCnt, head)) != NULL ) {
        	sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = ALREADY EXIST INFOMATION ADD FAIL\n");
        	MMCResSnd(rxIxpcMsg, txBuf, 1, 0);
        	sprintf(trcBuf, "[doAddCommProcess]COMMAND:%s Process Fail\n", ft->cmdName);
        	trclib_writeLogErr(FL, trcBuf);
        	return -1;
        } else {

            //node insert
           	if (!strcasecmp(ft->cmdName, "add-wap-gw")) 
				insert_list_node(columns, 5 , tail);   		
			else insert_list_node(columns, 10 , tail);

            //file insert
            if ( flush_list_to_file( head, fname ) < 0 ) {
                fprintf (stdout, "[doAddAaaInfo] file save fail[%s]; err=%d(%s)\n", fname, errno, strerror(errno));
                sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = INFOMATION ADD FAIL\n");
                MMCResSnd(rxIxpcMsg, txBuf, 1, 0);
                sprintf(trcBuf, "[doAddCommProcess]COMMAND:%s Process Fail\n", ft->cmdName);
                trclib_writeLogErr(FL, trcBuf);
                return -1;
            } else {
                sprintf(txBuf, "\n    RESULT = SUCCESS\n    REASON = INFOMATION ADD SUCCESS");
                addHeadMessage( mmcHdlr, titleMag );

                if( ft->frmMode == COMM_FRM_MODE )
                    addBodyLineMessage( mmcHdlr, columns, tmpMag, 2 );
                else if ( ft->frmMode == AAA_FRM_MODE )
                    addBodyLineAaaMessage( mmcHdlr, columns, tmpMag, 2 );
                else if ( ft->frmMode == URL_FRM_MODE )
                    addBodyLineUrlMessage( mmcHdlr, columns, tmpMag, 2 );
                else if ( ft->frmMode == TRC_FRM_MODE )
                    addBodyLineTrcMessage( mmcHdlr, columns, tmpMag, 2 );

                strcat(bodyMag, tmpMag );
                addTailMessage( mmcHdlr, totalBuffer );
        
                strcat(txBuf, titleMag );       //title
                strcat(txBuf, bodyMag);          // body
                strcat(txBuf, totalBuffer);    //------------

                MMCResSnd(rxIxpcMsg, txBuf, 0, 0);
            }
        }
        
	genQMsgType.mtype = MTYPE_BSD_CONFIG; 
    memcpy ((void*)genQMsgType.body, mcc, sizeof(MpConfigCmd) );
    if ( ft->mode == 0 )
        SendAppNoty(&genQMsgType);
    else if ( ft->mode == 1 ){
        oldSendAppNoty(1, DEF_SYS, SID_CHG_INFO, mcc->cmdType);
        if(!strcmp(ft->cmdName, "reg-call-trc")) {
			// RDRANA에서 전송ㅎ고.
            oldSendAppNoty(2, DEF_SYS, SID_CHG_INFO, mcc->cmdType);
            oldSendAppNoty(3, DEF_SVC, SID_CHG_INFO, mcc->cmdType);

			// Trace 남기라고 SCE에 전송. 

			/* jjinri 06.29 RealTime RDR 제거 
			sprintf (RealTime_RDR_command, "sudo -u pcube /SM/pcube/sm/server/bin/p3subs --set --subscriber=%s --property=monitor=1 \n",  columns[1]);
    		trclib_writeLogErr(FL, RealTime_RDR_command);
			if (system(RealTime_RDR_command) < 0) {
                sprintf(trcBuf, "[doAddCommProcess]COMMAND:%s. Fail\n", RealTime_RDR_command);
                trclib_writeLogErr(FL, trcBuf);
			}
			*/
		}
    }

////    conf_file_sync(ft->confFile);
    sprintf(trcBuf, "[doAddCommProcess]COMMAND:%s Process Success\n", ft->cmdName);
    trclib_writeLogErr(FL, trcBuf);

    return 1;
}
#if 0 //// jjinri 0625
int doAddIpProcess(  void *tbl,IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,void *pft, MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE],char  fname[FILESIZE])
{
    
    char    txBuf[MMCMSGSIZE];
    pMmcFuncTable ft;
    ft=(pMmcFuncTable)pft;
    unsigned int    uiIP, uiNetMask;
    //struct in_addr intIp;
    //char * pIp;
    //char * pNetMask;
   	
    //uiIP = inet_addr(columns[2]);
    uiIP = inet_addr(columns[1]);
    //uiNetMask = atoi( columns[3] );
    uiNetMask = atoi( columns[2] );
    if( uiIP == (in_addr_t)-1 || uiNetMask < 0 || uiNetMask > 32)
    {
        sprintf(txBuf, "\n    RESULT = FAIL\n    REASON =  INVALID PARAMETER FAIL\n");
        MMCResSnd(rxIxpcMsg, txBuf, 1, 0);
        return -1;
    }

     //이미 존재하는지 체크
        if ( search_list_node(columns, ft->startIndex, ft->keyCnt, head) != NULL ) {
        sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = ALREADY EXIST INFOMATION ADD FAIL\n");
        MMCResSnd(rxIxpcMsg, txBuf, 1, 0);
        return -1;
        } else {
            //node insert
            insert_list_node(columns, ft->paraCnt , tail);

            //file insert
            if ( flush_list_to_file( head, fname ) < 0 ) {
                sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = INFOMATION ADD FAIL\n");
                MMCResSnd(rxIxpcMsg, txBuf, 1, 0);
                return -1;
            }

        }
    trclib_writeLogErr(FL, trcBuf);


    return 1;
}

int doAddLimitProcess(  void *tbl,IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,void *pft, MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE],char  fname[FILESIZE])
{
    	char    txBuf[MMCMSGSIZE];
    	int totalCount=0;
    	pMmcFuncTable ft;
    	ft=(pMmcFuncTable)pft;


     	//이미 존재하는지 체크
        if ( search_list_node(columns, ft->startIndex, ft->keyCnt, head) != NULL ) {
        	sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = ALREADY EXIST INFOMATION ADD FAIL\n");
        	MMCResSnd(rxIxpcMsg, txBuf, 1, 0);
        	sprintf(trcBuf, "[doAddLimitProcess]COMMAND:%s Process Fail(Not Found)\n", ft->cmdName);
        	trclib_writeLogErr(FL, trcBuf);
        	return -1;
        } else {

           if ( (totalCount=getTotalNodeCnt()) < 3 ){
               if(  doAddCommProcess( tbl, rxIxpcMsg, genQMsgType, ft, mcc, head, tail, columns, fname) > 0 )
                    return 1;
                else
                    return -1;
            }else {
                sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = INFOMATION ADD LIMIT FAIL\n");
                MMCResSnd(rxIxpcMsg, txBuf, 1, 0);
                delete_all_list_node(head, tail);
                sprintf(trcBuf, "[doAddLimitProcess]COMMAND:%s Process Fail(Excess Limit)\n", ft->cmdName);
                trclib_writeLogErr(FL, trcBuf);
                return -1;
            }
 
        }
}
#endif

int doAddTrcProcess(  void *tbl,IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,void *pft, MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE],char  fname[FILESIZE])
{
    int     i, j = 0;
    //char    txBuf[MMCMSGSIZE];
    //char tmpArray[MAX_COL_SIZE];
    time_t  now;
    pMmcFuncTable ft;
    ft=(pMmcFuncTable)pft;
    DLinkedList *node, *result_node; 
	const int _IMSI_SIZE_ = 15;
	char *dot = NULL;
	int  dotCnt = 0;
 	
    node = head;
    if ( getTotalNodeCnt() >= 10 ){
	result_node = delete_search_node(ft->startIndex, node);
	delete_list_node(result_node);	
	//conf_file_sync(ft->confFile);
	//sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = INFOMATION ADD LIMIT FAIL\n");
        //MMCResSnd(rxIxpcMsg, txBuf, 1, 0);
        //sprintf(trcBuf, "[doAddTrcProcess]COMMAND:%s Process Fail(Excess Limit)\n", ft->cmdName);
        //trclib_writeLogErr(FL, trcBuf);
        //return -1;
    }

    for(i = ft->paraCnt-2; i > 0; i--){
        strcpy(mcc->cmdPara[i], mcc->cmdPara[i-1]);
        strcpy(columns[i], columns[i-1]);
    }

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
    memset( tmpArray, 0, sizeof( MAX_COL_SIZE ) );
    sprintf( tmpArray, "%s", ( !strcasecmp( columns[0], "IMSI" ) == 1 ? "1" : "2" ) );
    strcpy( columns[0],tmpArray );
*/
    now = time(0);
    sprintf(columns[ft->paraCnt-1], "%ld", now);
    if( doAddCommProcess( tbl, rxIxpcMsg, genQMsgType, ft, mcc, head, tail, columns, fname)>0 )
        return 1;
    else
        return -1;

}

#if 0 //// jjinri 0625
int doAddPdsnProcess(  void *tbl,IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,void *pft, MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE],char  fname[FILESIZE])
{
    char    txBuf[MMCMSGSIZE];
    pMmcFuncTable ft;
    ft=(pMmcFuncTable)pft;

     //이미 존재하는지 체크
        if ( search_list_node(columns, ft->startIndex, ft->keyCnt, head) != NULL ) {
        sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = ALREADY EXIST INFOMATION ADD FAIL\n");
        MMCResSnd(rxIxpcMsg, txBuf, 1, 0);
        return -1;
        } else {

            //node insert
            insert_list_node(columns, ft->paraCnt , tail);

            //file insert
            if ( flush_list_to_file( head, fname ) < 0 ) {
                sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = INFOMATION ADD FAIL\n");
                MMCResSnd(rxIxpcMsg, txBuf, 1, 0);
                return -1;
            }

        }
    return 1;
}

int doAddUdrConfProcess(  void *tbl,IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,void *pft, MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE],char  fname[FILESIZE])
{
    char    txBuf[MMCMSGSIZE];
    int     i=0;
    pMmcFuncTable ft;
    ft=(pMmcFuncTable)pft;
    char tmpcolumns[MAX_COL_SIZE];
    
        strcpy( tmpcolumns, columns[0] );
        strcat( tmpcolumns, "/" );
        strcat( tmpcolumns, columns[1] );

//        fprintf( stderr, "colspan : %d \n", tmpcolumns );

        strcpy( columns[0], tmpcolumns );
        for( i=1; i< ft->paraCnt; i++ ) {
           strcpy( columns[i], !strcasecmp(columns[i+1],"$") == 1 ? "" : columns[i+1] ); 
        }
     //이미 존재하는지 체크
        if ( search_list_node(columns, ft->startIndex, ft->keyCnt, head) != NULL ) {
        sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = ALREADY EXIST INFOMATION SET FAIL\n");
        MMCResSnd(rxIxpcMsg, txBuf, 1, 0);
        return -1;
        } else {

            //node insert
            insert_list_node(columns, ft->paraCnt-1 , tail);

            //file insert
            if ( flush_list_to_file( head, fname ) < 0 ) {
                fprintf (stdout, "[doAddAaaInfo] file save fail[%s]; err=%d(%s)\n", fname, errno, strerror(errno));
                sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = INFOMATION SET FAIL\n");
                MMCResSnd(rxIxpcMsg, txBuf, 1, 0);
                return -1;
            } else {
                sprintf(txBuf, "\n    RESULT = SUCCESS\n    REASON = INFOMATION SET SUCCESS\n");
                MMCResSnd(rxIxpcMsg, txBuf, 0, 0);
            }

        }

    genQMsgType.mtype = MTYPE_BSD_CONFIG;
    memcpy ((void*)genQMsgType.body, mcc, sizeof(MpConfigCmd) );
    if ( ft->mode == 0 )
        SendAppNoty(&genQMsgType);
    else if ( ft->mode == 1 )
        oldSendAppNoty(1, DEF_SYS, SID_CHG_INFO, mcc->cmdType);

    return 1;
}


int doAddSvcOptProcess(  void *tbl,IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,void *pft, MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE],char  fname[FILESIZE])
{
    char    txBuf[MMCMSGSIZE];
    pMmcFuncTable ft;
    ft=(pMmcFuncTable)pft;

     //이미 존재하는지 체크
        if ( search_list_node(columns, ft->startIndex, ft->keyCnt, head) != NULL ) {
        sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = ALREADY EXIST INFOMATION ADD FAIL\n");
        MMCResSnd(rxIxpcMsg, txBuf, 1, 0);
        return -1;
        } else {

            //node insert
            insert_list_node(columns, ft->paraCnt , tail);

            //file insert
            if ( flush_list_to_file( head, fname ) < 0 ) {
                fprintf (stdout, "[doAddSvcOptProcess] file save fail[%s]; err=%d(%s)\n", fname, errno, strerror(errno));
                sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = INFOMATION ADD FAIL\n");
                MMCResSnd(rxIxpcMsg, txBuf, 1, 0);
                return -1;
            }

        }
    return 1;
}
#endif
