#include "mmcr.h"
extern char     trcBuf[TRCBUF_LEN], trcTmp[TRCTMP_LEN];
extern char     mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN], sysLabel[COMM_MAX_NAME_LEN];
extern int      trcFlag, trcLogFlag;

MmcMemberTable mmcMemberTable[MMCR_MAX_MMC_HANDLER] = {
{"dis-aaa-info", {"prefix", "1st_AAA_IP", "1st_secret", "1st_alias", "2nd_AAA_IP", "2nd_secret", "2nd_alias"}, {17,20,14,12,20,14,10}, 7},
{"add-aaa-info", {"prefix", "1st_AAA_IP", "1st_secret", "1st_alias", "2nd_AAA_IP", "2nd_secret", "2nd_alias"}, {17,20,14,12,20,14,10}, 7},
{"chg-aaa-info", {"prefix", "1st_AAA_IP", "1st_secret", "1st_alias", "2nd_AAA_IP", "2nd_secret", "2nd_alias"}, {17,20,14,12,20,14,10}, 7},
{"del-aaa-info", {"prefix", "1st_AAA_IP", "1st_secret", "1st_alias", "2nd_AAA_IP", "2nd_secret", "2nd_alias"}, {17,20,14,12,20,14,10}, 7},
{"dis-anaaa-info", {"prefix", "1st_AAA_IP", "1st_secret", "1st_alias", "2nd_AAA_IP", "2nd_secret", "2nd_alias"}, {17,20,14,12,20,14,10}, 7},
{"add-anaaa-info", {"prefix", "1st_AAA_IP", "1st_secret", "1st_alias", "2nd_AAA_IP", "2nd_secret", "2nd_alias"}, {17,20,14,12,20,14,10}, 7},
{"chg-anaaa-info", {"prefix", "1st_AAA_IP", "1st_secret", "1st_alias", "2nd_AAA_IP", "2nd_secret", "2nd_alias"}, {17,20,14,12,20,14,10}, 7},
{"del-anaaa-info", {"prefix", "1st_AAA_IP", "1st_secret", "1st_alias", "2nd_AAA_IP", "2nd_secret", "2nd_alias"}, {17,20,14,12,20,14,10}, 7},
{"dis-pdsn-info", {"ip", "alias"},{20,20}, 2},
{"add-pdsn-info", {"ip", "alias"},{20,20}, 2},
{"chg-pdsn-info", {"ip", "alias"},{20,20}, 2},
{"del-pdsn-info", {"ip", "alias"},{20,20}, 2},
{"dis-ip-pool", {"pdsn_ip", "ip", "netmask"}, {20,20,15}, 3},
{"add-ip-pool", {"pdsn_ip", "ip", "netmask"}, {20,20,15}, 3},
{"del-ip-pool", {"pdsn_ip", "ip", "netmask"}, {20,20,15}, 3},
//{"dis-ip-pool", {"pdsn_ip", "pdsn_type", "ip", "netmask"}, {20,20,20,15}, 4},
//{"add-ip-pool", {"pdsn_ip", "pdsn_type", "ip", "netmask"}, {20,20,20,15}, 4},
//{"del-ip-pool", {"pdsn_ip", "pdsn_type", "ip", "netmask"}, {20,20,20,15}, 4},
{"add-wap-gw", {"ip", "port", "uid", "passwd", "alias" }, {17,8,15,15,15}, 5},
{"chg-wap-gw", {"ip", "port", "uid", "passwd", "alias" }, {17,8,15,15,15}, 5},
{"del-wap-gw", {"ip", "port", "uid", "passwd", "alias" }, {17,8,15,15,15}, 5},
{"dis-url-cha", {"svctype", "url", "newsvctype", "alias"}, {9,70,12,15}, 4},
{"add-url-cha", {"svctype", "url", "newsvctype", "alias"}, {9,70,12,15}, 4},
{"chg-url-cha", {"svctype", "url", "newsvctype", "alias"}, {9,70,12,15}, 4},
{"del-url-cha", {"svctype", "url", "newsvctype", "alias"}, {9,70,12,15}, 4},
{"dis-svc-type", {"svctype", "layer", "fout","recordid", "ip", "netmask", "port", "block", "url_cha", "cdrip", "first_udr", "alias"}, {9,7,6,11,17,9,7,10,9,12,12,15}, 12},
{"add-svc-type", {"svctype", "layer", "fout","recordid", "ip", "netmask", "port", "block", "url_cha", "cdrip", "first_udr", "alias"}, {9,7,6,11,17,9,7,10,9,12,12,15}, 12},
{"chg-svc-type", {"svctype", "layer", "fout","recordid", "ip", "netmask", "port", "block", "url_cha", "cdrip", "first_udr", "alias"}, {9,7,6,11,17,9,7,10,9,12,12,15}, 12},
{"del-svc-type", {"svctype",  "layer", "fout","recordid", "ip", "netmask", "port", "block", "url_cha", "cdrip", "first_udr", "alias"}, {9,7,6,11,17,9,7,10,9,12,12,15}, 12},
{"dis-txn-ext", {"extraction_period(sec)", "extraction_count", "past_count"}, {25,18,12}, 3},
{"set-txn-ext", {"extraction_period(sec)", "extraction_count", "past_count"}, {25,18,12}, 3},
{"dis-udr-conf", {"parameter_type", "flag", "alias"}, {17,10,30}, 3},
{"set-udr-conf", {"parameter_type", "flag", "alias"}, {17,10,30}, 3},
{"dis-udr-txc", {"svctype", "interim","url_count"}, {10,10,10}, 3},
{"add-udr-txc", {"svctype", "interim","url_count"}, {10,10,10}, 3},
{"chg-udr-txc", {"svctype", "interim","url_count"}, {10,10,10}, 3},
{"del-udr-txc", {"svctype", "interim","url_count"}, {10,10,10}, 3},
{"dis-dup-conf", {"hb_int(msec)", "hb_rcv(msec)","req_to(sec)","req_retry","app_delay(sec)","wait_so(sec)","auto_so"}, {14,14,13,12,16,14,9}, 7},
{"set-dup-conf", {"hb_int(msec)", "hb_rcv(msec)","req_to(sec)","req_retry","app_delay(sec)","wait_so(sec)","auto_so"}, {14,14,13,12,16,14,9}, 7},
{"dis-swt-cond", {"resource_type", "resource_index", "flag"}, {17,17,10}, 3},
{"set-swt-cond", {"resource_type", "resource_index", "flag"}, {17,17,10}, 3},
{"dis-tmr", {"timer_type","timer_class","timer_value(sec)"}, {16,16,16}, 3},
{"set-tmr", {"timer_type","timer_class","timer_value(sec)"}, {16,16,16}, 3},
{"dis-log-level", {"logtype","logclass", "logvalue"}, {15,15,15}, 3},
{"set-log-level", {"logtype","logclass", "logvalue"}, {15,15,15}, 3},
{"set-udr-dump", {"max_time(sec)","max_cnt"}, {15,15}, 2},
{"dis-pps-conf", {"interim_interval", "pps_max_count","pps_flag"}, {18, 15,8}, 3},
{"set-pps-conf", {"interim_interval", "pps_max_count","pps_flag"}, {18, 15,8}, 3},
{"dis-call-trc", {"trace_type", "trace_value", "registeration_date"}, {12,20,30}, 3},
{"reg-call-trc", {"trace_type", "trace_value", "registeration_date"}, {12,20,30}, 3},
{"chg-call-trc", {"trace_type", "trace_value", "registeration_date"}, {12,20,30}, 3},
{"canc-call-trc", {"trace_type", "trace_value", "registeration_date"}, {12,20,30}, 3},
{"dis-svc-opt", {"svc_opt", "svctype",  "alias"}, {10,10,15}, 3},
{"add-svc-opt", {"svc_opt", "svctype",  "alias"}, {10,10,15}, 3},
{"chg-svc-opt", {"svc_opt", "svctype",  "alias"}, {10,10,15}, 3},
{"del-svc-opt", {"svc_opt", "svctype",  "alias"}, {10,10,15}, 3},
{"dis-icmp-conf", {"fout", "service_type"}, {6, 15}, 2},
{"set-icmp-conf", {"fout", "service_type"}, {6, 15}, 2},
{"dis-bcast-conf", {"fout", "service_type"}, {6, 15}, 2},
{"set-bcast-conf", {"fout", "service_type"}, {6, 15}, 2},
{"dis-uawap-info", {"service-type", "SERVER-IP", "SERVER-PORT", "MY-NUM", "MAX-NUM", "URL-CHAR-FLAG"},
                   {14, 17, 13, 8, 9, 15}, 6},
{"set-uawap-info", {"service-type", "SERVER-IP", "SERVER-PORT", "MY-NUM", "MAX-NUM", "URL-CHAR-FLAG"},
                   {14, 17, 13, 8, 9, 15}, 6}


};
//{"dis-dup-sts", {"dup_sts"}, {15}, 1},
//{"dis-svc-opt", {"svc_opt", "svctype", "layer", "fout", "block", "url_cha", "alias"}, {10,10,8,7,10,10,15}, 7},
//{"add-svc-opt", {"svc_opt", "svctype", "layer", "fout", "block", "url_cha", "alias"}, {10,10,8,7,10,10,15}, 7},
//{"chg-svc-opt", {"svc_opt", "svctype", "layer", "fout", "block", "url_cha", "alias"}, {10,10,8,7,10,10,15}, 7},
//{"del-svc-opt", {"svc_opt", "svctype", "layer", "fout", "block", "url_cha", "alias"}, {10,10,8,7,10,10,15}, 7},

// TRACE_LIST.conf jjinri backup 
//{"dis-call-trc", {"trace_type", "trace_value", "trace_level", "registeration_date"}, {12,20,14,30}, 4},
//{"reg-call-trc", {"trace_type", "trace_value", "trace_level", "registeration_date"}, {12,20,14,30}, 4},
//{"chg-call-trc", {"trace_type", "trace_value", "trace_level", "registeration_date"}, {12,20,14,30}, 4},
//{"canc-call-trc", {"trace_type", "trace_value", "trace_level", "registeration_date"}, {12,20,14,30}, 4},


int numMmcMemberHdlr=59;


void doDisInfo(IxpcQMsgType *rxIxpcMsg, void *pt);
void doAddInfo(IxpcQMsgType *rxIxpcMsg, void *pt);
void doDelInfo(IxpcQMsgType *rxIxpcMsg, void *pt);
void doChgInfo(IxpcQMsgType *rxIxpcMsg, void *pt);
void doSetInfo(IxpcQMsgType *rxIxpcMsg, void *pt);
void set_dup_conf(IxpcQMsgType *rxIxpcMsg, void *pt);


void doDisInfo(IxpcQMsgType *rxIxpcMsg, void *pt)
{
	GeneralQMsgType genQMsgType;
    MpConfigCmd mcc;
    MMLReqMsgType   *rxReqMsg;
    pMmcFuncTable ft;
    pMmcMemberTable   mmcHdlr;
    int count=0, i=0, j=0, len=0;
    DLinkedList head, tail;
	//DLinkedList *node, *cur_node;
    char    *env,fname[FILESIZE], bodyMag[MMCMSGSIZE], totalBuffer[BUFSIZE];
    char    titleMag[BUFSIZE],tmpMag[BUFSIZE], txBuf[MMCMSGSIZE];
    char    columns[MAX_COL_COUNT][MAX_COL_SIZE];
    char    tempParaVal[MAX_COL_SIZE];

    memset( columns, 0, sizeof( columns ) );
    // func init
    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
    ft = (pMmcFuncTable)pt;

    memset(txBuf, 0, sizeof(txBuf));
    memset(tmpMag, 0, sizeof(tmpMag));
    memset(bodyMag, 0, sizeof(bodyMag));
    memset(totalBuffer, 0, sizeof(totalBuffer));
    memset(titleMag, 0, sizeof(titleMag));

    //printf("%s, %s, %d %d\n", ft->cmdName, ft->confFile, ft->cmdType, ft->paraCnt);

    //MpConfigCmd set
    mcc.cmdType = ft->cmdType;
    for( i=0; i<ft->paraCnt; i++ ){
        if(strchr(rxReqMsg->head.para[i].paraVal, '=')){
            for(j=0; (rxReqMsg->head.para[i].paraVal[j] != '(') && (j < MAX_COL_SIZE); j++);
            if(j < MAX_COL_SIZE){
                j++;
            }

            strcpy(tempParaVal, &rxReqMsg->head.para[i].paraVal[j]);
            len = strlen(tempParaVal);
            if(tempParaVal[len-1] == ')'){
                tempParaVal[len-1] = 0x00;
            }
        }else{
            strcpy(tempParaVal, rxReqMsg->head.para[i].paraVal);
        }

        strcpy(mcc.cmdPara[i], tempParaVal);
        strcpy(columns[i],convertNulltoDollar(tempParaVal));
        if( strlen(tempParaVal) > 0 )
            count++;

    }
    mcc.cmdParaCnt = count;
    if ((mmcHdlr = (pMmcMemberTable) bsearch (
                    ft->cmdName,
                    mmcMemberTable,
                    numMmcMemberHdlr,
                    sizeof(MmcMemberTable),
                    mmcr_mmcMemberHdlrVector_bsrchCmp)) == NULL) {
                    sprintf(trcBuf,"[fimd_doDisInfo] received unknown mml_cmd(%s)\n", rxReqMsg->head.cmdName);
                    trclib_writeLogErr (FL,trcBuf);
                }

    if ((env = getenv(IV_HOME)) == NULL){
        fprintf( stdout, "getenv error! \n" );
    }


    sprintf (fname, "%s/%s", env, ft->confFile);

    set_head_tail(&head, &tail);
	if(make_conf_list(fname, &head) < 0){
         fprintf (stdout, "make linked list fail; err=%d(%s)\n", errno, strerror(errno));
         //trclib_writeLogErr (FL,trcBuf);
		sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = FILE NOT FOUND\n");
        MMCResSnd(rxIxpcMsg, txBuf, 1, 0);

    } else {
        
        //doDisCommProcess( mmcHdlr, &head, rxIxpcMsg );
        ((ft->mmcPrc))( mmcHdlr,rxIxpcMsg, genQMsgType, ft, &mcc, &head, NULL, columns, NULL );


    }

//  print_list_node(&head);
    delete_all_list_node(&head, &tail);
	
}

void doAddInfo(IxpcQMsgType *rxIxpcMsg, void *pt)
{
	GeneralQMsgType genQMsgType;
    MpConfigCmd mcc;
    MMLReqMsgType   *rxReqMsg;
    pMmcFuncTable ft;
    pMmcMemberTable   mmcHdlr;
    int count=0, i=0, j = 0, len = 0;

	DLinkedList head, tail;
	char    *env, fname[FILESIZE], columns[MAX_COL_COUNT][MAX_COL_SIZE], txBuf[MMCMSGSIZE]; 
    char    tempParaVal[MAX_COL_SIZE];


    memset( columns, 0, sizeof( columns ) );
    // func init
    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
    ft = (pMmcFuncTable)pt;


    mcc.cmdType = ft->cmdType;

    sprintf(trcBuf, "[doAddInfo]COMMAND:%s, PARA->", rxReqMsg->head.cmdName);
    for( i=0; i<ft->paraCnt; i++ ){
        if(strchr(rxReqMsg->head.para[i].paraVal, '=')){
            for(j=0; (rxReqMsg->head.para[i].paraVal[j] != '(') && (j < MAX_COL_SIZE); j++);
            if(j < MAX_COL_SIZE){
                j++;
            }

            strcpy(tempParaVal, &rxReqMsg->head.para[i].paraVal[j]);
            len = strlen(tempParaVal);
            if(tempParaVal[len-1] == ')'){
                tempParaVal[len-1] = 0x00;
            }
        }else{
            strcpy(tempParaVal, rxReqMsg->head.para[i].paraVal);
        }

        strcpy(mcc.cmdPara[i], tempParaVal);
        strcpy(columns[i],convertNulltoDollar(tempParaVal));
        if( strlen(tempParaVal) > 0 )
            count++;

        sprintf(trcTmp, "[%d]%s, ", i, rxReqMsg->head.para[i].paraVal);
        strcat(trcBuf, trcTmp);
    }
    strcat(trcBuf, "\n");
    trclib_writeLogErr(FL, trcBuf);

    mcc.cmdParaCnt = count;
    if ((mmcHdlr = (pMmcMemberTable) bsearch (
                    ft->cmdName,
                    mmcMemberTable,
                    numMmcMemberHdlr,
                    sizeof(MmcMemberTable),
                    mmcr_mmcMemberHdlrVector_bsrchCmp)) == NULL) {
                    sprintf(trcBuf,"[fimd_doDisInfo] received unknown mml_cmd(%s)\n", rxReqMsg->head.cmdName);
                    trclib_writeLogErr (FL,trcBuf);
                }

	if ((env = getenv(IV_HOME)) == NULL){
		fprintf( stdout, "getenv error! \n" );
	}
	        
    sprintf (fname, "%s/%s", env, ft->confFile);

    set_head_tail(&head, &tail);

    if(make_conf_list(fname, &head) < 0){
		 fprintf (stdout, "[doAddAaaInfo] make linked list fail; err=%d(%s)\n", errno, strerror(errno));
		 //trclib_writeLogErr (FL,trcBuf);
		 sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = FILE NOT FOUND\n");
        MMCResSnd(rxIxpcMsg, txBuf, 1, 0);

    } else {

        ((ft->mmcPrc))(mmcHdlr,rxIxpcMsg, genQMsgType,ft,&mcc, &head, &tail, columns, fname);

	}

    //print_list_node(&head);
    delete_all_list_node(&head, &tail);

}

void doDelInfo(IxpcQMsgType *rxIxpcMsg, void *pt)
{
    GeneralQMsgType genQMsgType;
    MpConfigCmd mcc;
    MMLReqMsgType   *rxReqMsg;
    pMmcFuncTable ft;
    pMmcMemberTable   mmcHdlr;
    int count=0, i=0, j = 0, len=0;

    DLinkedList head, tail;
	//DLinkedList *node;
    char    *env, fname[FILESIZE], columns[MAX_COL_COUNT][MAX_COL_SIZE], txBuf[MMCMSGSIZE];
    char    tempParaVal[MAX_COL_SIZE];

    memset( columns, 0, sizeof( columns ) );

    // func init
    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
    ft = (pMmcFuncTable)pt;

    //printf("%s, %s, %d %d\n", ft->cmdName, ft->confFile, ft->cmdType, ft->paraCnt);

    //MpConfigCmd set
    mcc.cmdType = ft->cmdType;
    //count = ntohs(rxReqMsg->head.paraCnt);
    //mcc.cmdParaCnt = count;
    sprintf(trcBuf, "[doDelInfo]COMMAND:%s, PARA->", rxReqMsg->head.cmdName);
    for( i=0; i<ft->paraCnt; i++ ){
        if(strchr(rxReqMsg->head.para[i].paraVal, '=')){
            for(j=0; (rxReqMsg->head.para[i].paraVal[j] != '(') && (j < MAX_COL_SIZE); j++);
            if(j < MAX_COL_SIZE){
                j++;
            }

            strcpy(tempParaVal, &rxReqMsg->head.para[i].paraVal[j]);
            len = strlen(tempParaVal);
            if(tempParaVal[len-1] == ')'){
                tempParaVal[len-1] = 0x00;
            }
        }else{
            strcpy(tempParaVal, rxReqMsg->head.para[i].paraVal);
        }

        strcpy(mcc.cmdPara[i], tempParaVal);
        strcpy(columns[i],convertNulltoDollar(tempParaVal));
        if( strlen(tempParaVal) > 0 )
            count++;

        sprintf(trcTmp, "[%d]%s, ", i, rxReqMsg->head.para[i].paraVal);
        strcat(trcBuf, trcTmp);
    }
    strcat(trcBuf, "\n");
    trclib_writeLogErr(FL, trcBuf);
    mcc.cmdParaCnt = count;

    if ((mmcHdlr = (pMmcMemberTable) bsearch (
                    ft->cmdName,
                    mmcMemberTable,
                    numMmcMemberHdlr,
                    sizeof(MmcMemberTable),
                    mmcr_mmcMemberHdlrVector_bsrchCmp)) == NULL) {
                    sprintf(trcBuf,"[fimd_doDisInfo] received unknown mml_cmd(%s)\n", rxReqMsg->head.cmdName);
                    trclib_writeLogErr (FL,trcBuf);
                }

    if ((env = getenv(IV_HOME)) == NULL){
        fprintf( stdout, "getenv error! \n" );
    }


    sprintf (fname, "%s/%s", env, ft->confFile);

    set_head_tail(&head, &tail);

    if(make_conf_list(fname, &head) < 0){
         fprintf (stdout, "make linked list fail; err=%d(%s)\n", errno, strerror(errno));
         //trclib_writeLogErr (FL,trcBuf);
		 sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = FILE NOT FOUND\n");
        MMCResSnd(rxIxpcMsg, txBuf, 1, 0);

    } else {

        ((ft->mmcPrc))( mmcHdlr, rxIxpcMsg, genQMsgType, ft, &mcc, &head, &tail, columns, fname );

	}

    //print_list_node(&head);
    delete_all_list_node(&head, &tail);

}

void doChgInfo(IxpcQMsgType *rxIxpcMsg, void *pt)
{
	GeneralQMsgType genQMsgType;
    MpConfigCmd mcc;
    MMLReqMsgType   *rxReqMsg;
    pMmcFuncTable ft;
    pMmcMemberTable   mmcHdlr;
    int count=0, i=0, j = 0, len=0;

    DLinkedList head, tail;
    //DLinkedList *node;
    char    *env, fname[FILESIZE], columns[MAX_COL_COUNT][MAX_COL_SIZE], txBuf[MMCMSGSIZE];
    char    tempParaVal[MAX_COL_SIZE];
    
    memset( columns, 0, sizeof( columns ) );
    // func init
    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
    ft = (pMmcFuncTable)pt;

    //printf("%s, %s, %d %d\n", ft->cmdName, ft->confFile, ft->cmdType, ft->paraCnt);

    //MpConfigCmd set
    memset(&mcc, 0x00, sizeof(mcc));
    mcc.cmdType = ft->cmdType;
    //count = ntohs(rxReqMsg->head.paraCnt);
    //mcc.cmdParaCnt = count;
    sprintf(trcBuf, "[doChgInfo]COMMAND:%s, PARA->", rxReqMsg->head.cmdName);
    for( i=0; i<ft->paraCnt; i++ ){
        if(strchr(rxReqMsg->head.para[i].paraVal, '=')){
            for(j=0; (rxReqMsg->head.para[i].paraVal[j] != '(') && (j < MAX_COL_SIZE); j++);
            if(j < MAX_COL_SIZE){
                j++;
            }

            strcpy(tempParaVal, &rxReqMsg->head.para[i].paraVal[j]);
            len = strlen(tempParaVal);
            if(tempParaVal[len-1] == ')'){
                tempParaVal[len-1] = 0x00;
            }
        }else{
            strcpy(tempParaVal, rxReqMsg->head.para[i].paraVal);
        }
        strcpy(mcc.cmdPara[i], tempParaVal);
        strcpy(columns[i],convertNulltoDollar(tempParaVal));
        if( strlen(tempParaVal) > 0 )
            count++;
        sprintf(trcTmp, "[%d]%s, ", i, rxReqMsg->head.para[i].paraVal);
        strcat(trcBuf, trcTmp);
    }

    strcat(trcBuf, "\n");
    trclib_writeLogErr(FL, trcBuf);

    mcc.cmdParaCnt = count;
    if ((mmcHdlr = (pMmcMemberTable) bsearch (
                    ft->cmdName,
                    mmcMemberTable,
                    numMmcMemberHdlr,
                    sizeof(MmcMemberTable),
                    mmcr_mmcMemberHdlrVector_bsrchCmp)) == NULL) {
                    sprintf(trcBuf,"[fimd_doDisInfo] received unknown mml_cmd(%s)\n", rxReqMsg->head.cmdName);
                    trclib_writeLogErr (FL,trcBuf);
                }
    
    if ((env = getenv(IV_HOME)) == NULL){ 
        fprintf( stdout, "getenv error! \n" );
    }

    
    sprintf (fname, "%s/%s", env, ft->confFile);
    
    set_head_tail(&head, &tail);
        
    if(make_conf_list(fname, &head) < 0){
         fprintf (stdout, "make linked list fail; err=%d(%s)\n", errno, strerror(errno));
         //trclib_writeLogErr (FL,trcBuf);
		 sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = FILE NOT FOUND\n");
        MMCResSnd(rxIxpcMsg, txBuf, 1, 0);

    } else { 

        ((ft->mmcPrc))( mmcHdlr,rxIxpcMsg, genQMsgType, ft, &mcc, &head, &tail, columns, fname );   

	}
    
//    print_list_node(&head);
    delete_all_list_node(&head, &tail);
    

}

void doSetInfo(IxpcQMsgType *rxIxpcMsg, void *pt)
{
    GeneralQMsgType genQMsgType;
    MpConfigCmd mcc;
    MMLReqMsgType   *rxReqMsg;
    pMmcFuncTable ft;
    pMmcMemberTable   mmcHdlr;
    int count=0, i=0;

    DLinkedList head, tail;
    //DLinkedList *node;
    char    *env, fname[FILESIZE], columns[MAX_COL_COUNT][MAX_COL_SIZE], txBuf[MMCMSGSIZE];

    memset( columns, 0, sizeof( columns ) );

    // func init
    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
    ft = (pMmcFuncTable)pt;

//    printf("%s, %s, %d %d\n", ft->cmdName, ft->confFile, ft->cmdType, ft->paraCnt);

    //MpConfigCmd set
    memset(&mcc, 0x00, sizeof(mcc));
    mcc.cmdType = ft->cmdType;

    sprintf(trcBuf, "[doSetInfo]COMMAND:%s, PARA->", rxReqMsg->head.cmdName);
    for( i=0; i<ft->paraCnt; i++ ){
        strcpy(mcc.cmdPara[i],rxReqMsg->head.para[i].paraVal);
        strcpy(columns[i],convertNulltoDollar(rxReqMsg->head.para[i].paraVal) );
        if( strlen(rxReqMsg->head.para[i].paraVal) > 0 )
             count++;

        sprintf(trcTmp, "[%d]%s, ", i, rxReqMsg->head.para[i].paraVal);
        strcat(trcBuf, trcTmp);
    }
    strcat(trcBuf, "\n");
    trclib_writeLogErr(FL, trcBuf);

    mcc.cmdParaCnt = count;
    if ((mmcHdlr = (pMmcMemberTable) bsearch (
                    ft->cmdName,
                    mmcMemberTable,
                    numMmcMemberHdlr,
                    sizeof(MmcMemberTable),
                    mmcr_mmcMemberHdlrVector_bsrchCmp)) == NULL) {
                    sprintf(trcBuf,"[fimd_doDisInfo] received unknown mml_cmd(%s)\n", rxReqMsg->head.cmdName);
                    trclib_writeLogErr (FL,trcBuf);
                }

    if ((env = getenv(IV_HOME)) == NULL){
        fprintf( stdout, "getenv error! \n" );
    }


    sprintf (fname, "%s/%s", env, ft->confFile);

    set_head_tail(&head, &tail);

    if(make_conf_list(fname, &head) < 0){
         fprintf (stdout, "make linked list fail; err=%d(%s)\n", errno, strerror(errno));
         //trclib_writeLogErr (FL,trcBuf);
		 sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = FILE NOT FOUND\n");
        MMCResSnd(rxIxpcMsg, txBuf, 1, 0);

    } else { 
        
        ((ft->mmcPrc))( mmcHdlr,rxIxpcMsg, genQMsgType, ft, &mcc, &head, &tail, columns, fname );

	}
   
    //print_list_node(&head);
    delete_all_list_node(&head, &tail);
   
}

int mmcr_mmcMemberHdlrVector_qsortCmp (const void *a, const void *b)
{
    return (strcasecmp (((pMmcMemberTable)a)->cmdName, ((pMmcMemberTable)b)->cmdName));
} //----- End of mmcr_mmcMemberHdlrVector_qsortCmp -----//



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int mmcr_mmcMemberHdlrVector_bsrchCmp (const void *a, const void *b)
{
    return (strcasecmp(((pMmcMemberTable)a)->cmdName, ((pMmcMemberTable)b)->cmdName) );
} //----- End of mmcr_mmcMemberHdlrVector_bsrchCmp -----//
