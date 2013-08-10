#include "mmcr.h"

extern char             trcBuf[TRCBUF_LEN], trcTmp[TRCTMP_LEN];
extern stLogLevel   *logLevel;
extern stTimer      *mpTimer;
//// jjinri 0625 extern st_PPS_CONF  *ppsconf;

stEnumTable     etLogLevel[LOGLVL_APPINX_NUM] = {
//    {"UAWAPANA", 	LOGLVL_APPINX_UAWAPANA},
//    {"AAAIF", 		LOGLVL_APPINX_AAAIF},
//    {"UDRGEN", 		LOGLVL_APPINX_UDRGEN},
    {"CAPD", 		LOGLVL_APPINX_CAPD},
//    {"CDR", 		LOGLVL_APPINX_CDR},
//    {"REANA", 		LOGLVL_APPINX_REANA},
	{"PANA", 		LOGLVL_APPINX_PANA},
    {"RLEG0", 		LOGLVL_APPINX_TRCDR0},
    {"RLEG1", 		LOGLVL_APPINX_TRCDR1},
    {"RLEG2", 		LOGLVL_APPINX_TRCDR2},
    {"RLEG3", 		LOGLVL_APPINX_TRCDR3},
    {"RLEG4", 		LOGLVL_APPINX_TRCDR4},
//    {"WAP1ANA", 	LOGLVL_APPINX_WAP1ANA},
//    {"WAP2ANA", 	LOGLVL_APPINX_WAP2ANA},
//    {"HTTPANA", 	LOGLVL_APPINX_HTTPANA},
//    {"SDMD", 		LOGLVL_APPINX_SDMD},
//	{"VODSANA", 	LOGLVL_APPINX_VODSANA},	/* ADDED BY LYH 2006.11.27 */
//	{"WIPINWANA", 	LOGLVL_APPINX_WIPINWANA},
//	{"JAVANWANA", 	LOGLVL_APPINX_JAVANWANA},
//	{"VTANA", 		LOGLVL_APPINX_VTANA},
//	{"FBANA", 		LOGLVL_APPINX_FBANA},
//	{"OZCDR", 		LOGLVL_APPINX_OZCDR},
	{"RANA", 		LOGLVL_APPINX_RANA},
//	{"LOGM", 		LOGLVL_APPINX_LOGM},
//	{"WVANA", 		LOGLVL_APPINX_WVANA},
//	{"MEMD", 		LOGLVL_APPINX_MEMD},
	{"RDRANA", 		LOGLVL_APPINX_RDRANA},	/* ADD BY jjinri 2009.04.24 */
	{"SMPP", 		LOGLVL_APPINX_SMPP},	/* by june, 2009.04.26 */
	{"RDRCAPD", 	LOGLVL_APPINX_RDRCAPD},	/* by yhshin, 2009.04.26 */
	{"MMCR", 		LOGLVL_APPINX_MMCR},	/* by jjinri, 2009.05.05 */
	{"IXPC", 		LOGLVL_APPINX_IXPC},	/* by sjjeon, 2009.05.15 */
	{"SAMD", 		LOGLVL_APPINX_SAMD}		/* by sjjeon, 2009.05.15 */
};

#if 0
stEnumTable     etLogLevelM[4] = {
    {"UAWAP", LOGLVL_APPINX_UAWAPANA},
    {"AAA", LOGLVL_APPINX_AAAIF},
    {"UDR", LOGLVL_APPINX_UDRGEN},
	{"LOGM", LOGLVL_APPINX_LOGM}
};
#endif


stEnumTable     etLogLevelKind[LOGLVL_KIND] = {
    {"MSG", LOGLVL_KIND_MSG},
    {"APP", LOGLVL_KIND_APP}
};

stEnumTable     etLogLevelType[LOGLVL_TYPE_NUM] = {
    {"NO", LOG_NOPRINT},
    {"CRITICAL", LOG_CRI},
    {"WARNING", LOG_WARN},
    {"DEBUG", LOG_DEBUG},
    {"ALL", LOG_INFO}
};

stEnumTable     etTimer[TIMER_KIND_NUM+1] = {
    {"SESS", TIMER_KIND_SESSION},
    {"MSG", TIMER_KIND_MESSAGE},
    {"CALL", TIMER_KIND_CALL}
};

#define TIMER_KIND_SESS_NUM     TIMER_KIND_GUBUN_NUM-3

stEnumTable     etTimerKindSess[TIMER_KIND_SESS_NUM] = {
    {"WAP1", TIMER_KIND_SESS_WAP1},
    {"WAP2", TIMER_KIND_SESS_WAP2},
    {"HTTP", TIMER_KIND_SESS_HTTP},
    {"VT", TIMER_KIND_SESS_VTSVC},
    {"FBCDR", TIMER_KIND_SESS_FBCDR}
};

#define TIMER_KIND_CALL_NUM     TIMER_KIND_GUBUN_NUM-5
stEnumTable      etTimerKindCall[TIMER_KIND_CALL_NUM] = {
    {"CALL", TIMER_KIND_SESS_CALL},
    {"LONGCALL", TIMER_KIND_SESS_LCALL}
};

#define TIMER_KIND_MSG_NUM     TIMER_KIND_GUBUN_NUM-6
stEnumTable     etTimerKindMsg[TIMER_KIND_MSG_NUM] = {
   // {"WAP1", TIMER_KIND_MSG_WAP1},
   // {"WAP2", TIMER_KIND_MSG_WAP2},
   // {"HTTP", TIMER_KIND_MSG_HTTP},
    {"AAA", TIMER_KIND_MSG_AAA}
};

int doSetLogLevel( void *tbl, IxpcQMsgType *rxIxpcMsg,
                       GeneralQMsgType genQMsgType,void *pft,
                       MpConfigCmd *mcc, DLinkedList *head,
                       DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE],
                       char fname[FILESIZE] )
{

    int appinx = -1, kindinx = -1, typeinx = -1;
    int i, result=0;
    char    txBuf[MMCMSGSIZE], failmsg[256];
    
    //shared memory process
    for(i = 0; i < LOGLVL_KIND; i++){
        if(!strcasecmp(mcc->cmdPara[0], etLogLevelKind[i].name)){
            kindinx = etLogLevelKind[i].value;
            result=1;
            break;
        }
    }


    if(kindinx == LOGLVL_KIND_APP){
        for(i = 0; i < LOGLVL_APPINX_NUM; i++){
            if(!strcasecmp(mcc->cmdPara[1], etLogLevel[i].name)){
                appinx = etLogLevel[i].value;
                result=1;
                break;
            }
        }
    }
	/*else if(kindinx == LOGLVL_KIND_MSG){
        for(i = 0; i < 4; i++){
            if(!strcasecmp(mcc->cmdPara[1], etLogLevelM[i].name)){
                appinx = etLogLevelM[i].value;
                result=1;
                break;
            }
        }
    }
	*/


    for(i = 0; i < LOGLVL_TYPE_NUM; i++){
        if(!strcasecmp(mcc->cmdPara[2], etLogLevelType[i].name)){
            typeinx = etLogLevelType[i].value;
            result=1;
            break;
        }
    }

    if( appinx>=0 && kindinx>=0 && typeinx>=0 ) {
        if( doSetCommProcess( tbl, rxIxpcMsg, genQMsgType, pft, mcc, head,tail, columns, fname )<0 )
			result=0;
    } else
        result=0;
    
    if( result == 0 ){
        if(kindinx == LOGLVL_KIND_APP){
			//sjjeon
            sprintf(failmsg, "WRONG LOG CLASS\n             APP's LOG CLASS ARE AS BELOWS\n"
							"CAPD MMCR PANA RANA RLEG REANA RDRANA RDRCAPD SMPP");
							//"CAPD IXPC MMCR PANA RANA RLEG REANA RDRANA RDRCAPD SAMD SMPP");
                            //    "             \"CAPD ANA CDR TRCDR WAP1ANA UAWAPANA WAP2ANA HTTPANA UDRGEN AAAIF SDMD\"\n"
                             //   "             \"VODSANA WIPINWANA JAVANWANA LOGM PTOPANA CDR2 VTANA\"\n");
        }else if(kindinx == LOGLVL_KIND_MSG){
			//sjjeon
            sprintf(failmsg, "WRONG LOG CLASS\n             MSG's LOG CLASS \n");
            //sprintf(failmsg, "WRONG LOG CLASS\n             MSG's LOG CLASS ARE AS BELOWS\n"
             //                   "             \"UAWAP UDR AAA LOGM\"\n");
        }else{
            sprintf(failmsg, "UNKNOWN PARAMETER\n");
        }
        sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = %s", failmsg);
        MMCResSnd(rxIxpcMsg, txBuf, 1, 0);
        return -1;
    }

    logLevel->loglevel[appinx][kindinx] = typeinx;
    return 1;
}

int doSetTimer( void *tbl, IxpcQMsgType *rxIxpcMsg,
                       GeneralQMsgType genQMsgType,void *pft,
                       MpConfigCmd *mcc, DLinkedList *head,
                       DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE],
                       char fname[FILESIZE] )
{
    int appinx = -1, kindinx = -1;
    int i, result=0;
    char    txBuf[MMCMSGSIZE];

    //shared memory process
    for(i = 0; i < TIMER_KIND_NUM+1; i++){
        if(!strcasecmp(mcc->cmdPara[0], etTimer[i].name)){
            appinx = etTimer[i].value;
            result=1;
            break;
        }
    }

    if(appinx == TIMER_KIND_SESSION){
        if(!strcasecmp(mcc->cmdPara[0], "SESS")){
            for(i = 0; i < TIMER_KIND_SESS_NUM; i++){
                if(!strcasecmp(mcc->cmdPara[1], etTimerKindSess[i].name)){
                    kindinx = etTimerKindSess[i].value;
                    result=1;
                    break;
                }
            }
        }
        if(!strcasecmp(mcc->cmdPara[0], "CALL")){
            for(i = 0; i < TIMER_KIND_CALL_NUM; i++){
                if(!strcasecmp(mcc->cmdPara[1], etTimerKindCall[i].name)){
                    kindinx = etTimerKindCall[i].value;
                    result=1;
                    break;
                }
            }
        }
    }else{
        for(i = 0; i < TIMER_KIND_MSG_NUM; i++){
            if(!strcasecmp(mcc->cmdPara[1], etTimerKindMsg[i].name)){
                kindinx = etTimerKindMsg[i].value;
                result=1;
                break;
            }
        }
    }

    if( appinx>=0 && kindinx>=0 ) {
        if( doSetCommProcess( tbl, rxIxpcMsg, genQMsgType, pft, mcc, head,tail, columns, fname )<0 )
                result=0;
    } else
        result=0;

    if( result == 0 ){
        sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = INVALID_PARAMETER FAIL\n");
        MMCResSnd(rxIxpcMsg, txBuf, 1, 0);
        return -1;
    }


    mpTimer->timer[appinx][kindinx] = atoi(mcc->cmdPara[2]);
    return 1;
}

void loadLogLevel(void)
{
    int i;
    int appinx = 0, kindinx = 0, typeinx = 0;
    char *env, fname[128];
    DLinkedList head, tail, *node;
    

    if ((env = getenv(IV_HOME)) == NULL){
        fprintf( stdout, "[loadLogLevel]getenv error! \n" );
        return;
    }

    sprintf (fname, "%s/%s", env, LOG_LEVEL_FILE);

    set_head_tail(&head, &tail);
     if(make_conf_list(fname, &head) < 0){
         fprintf (stdout, "[loadLogLevel] make linked list fail; err=%d(%s)\n", errno, strerror(errno));
         return;
    }

    node = &head;

    memset(logLevel, 0x00, sizeof(stLogLevel));

    while( (node=next_conf_node( node )) != NULL){
        if(node->item.gubun != TOK_CONF_LINE) continue;

        appinx = kindinx = typeinx = -1;

        for(i = 0; i < LOGLVL_KIND; i++){
            if(!strcasecmp(node->item.columns[0], etLogLevelKind[i].name)){
                kindinx = etLogLevelKind[i].value;
                break;
            }
        }
        if(kindinx == LOGLVL_KIND_APP){
            for(i = 0; i < LOGLVL_APPINX_NUM; i++){
                if(!strcasecmp(node->item.columns[1], etLogLevel[i].name)){
                    appinx = etLogLevel[i].value;
                    break;
                }
            }
        }/*else if(kindinx == LOGLVL_KIND_MSG){
            for(i = 0; i < 4; i++){
                if(!strcasecmp(node->item.columns[1], etLogLevelM[i].name)){
                    appinx = etLogLevelM[i].value;
                    break;
                }
            }
        }*/

        for(i = 0; i < LOGLVL_TYPE_NUM; i++){
            if(!strcasecmp(node->item.columns[2], etLogLevelType[i].name)){
                typeinx = etLogLevelType[i].value;
                break;
            }
        }

        if(!(kindinx < 0 || appinx < 0 || typeinx < 0))
            logLevel->loglevel[appinx][kindinx] = typeinx;
    }

    delete_all_list_node(&head, &tail);
}

void loadTimer(void)
{
    int i;
    int appinx, kindinx;
    char    *env, fname[128];
    DLinkedList head, tail, *node;
    

    if ((env = getenv(IV_HOME)) == NULL){
        fprintf( stdout, "[loadTimer]getenv error! \n" );
        return;
    }

    sprintf (fname, "%s/%s", env, TIMER_FILE);

    set_head_tail(&head, &tail);
     if(make_conf_list(fname, &head) < 0){
        fprintf (stdout, "[loadTimer] make linked list fail; err=%d(%s)\n", errno, strerror(errno));
        return;
    }

    node = &head;

    memset(mpTimer, 0x00, sizeof(stTimer));
    while( (node=next_conf_node( node )) != NULL){
        if(node->item.gubun != TOK_CONF_LINE) continue;
        for(i = 0; i < TIMER_KIND_NUM+1; i++){
            if(!strcasecmp(node->item.columns[0], etTimer[i].name)){
                appinx = etTimer[i].value;
                break;
            }
        }

        if(appinx == TIMER_KIND_SESSION){
            if(!strcasecmp(node->item.columns[0], "SESS")){
                for(i = 0; i < TIMER_KIND_SESS_NUM; i++){
                    if(!strcasecmp(node->item.columns[1], etTimerKindSess[i].name)){
                        kindinx = etTimerKindSess[i].value;
                        break;
                    }
                }
            }
            if(!strcasecmp(node->item.columns[0], "CALL")){
                for(i = 0; i < TIMER_KIND_CALL_NUM; i++){
                    if(!strcasecmp(node->item.columns[1], etTimerKindCall[i].name)){
                        kindinx = etTimerKindCall[i].value;
                        break;
                    }
                }
            }
        }else{
            for(i = 0; i < TIMER_KIND_MSG_NUM; i++){
                if(!strcasecmp(node->item.columns[1], etTimerKindMsg[i].name)){
                    kindinx = etTimerKindMsg[i].value;
                    break;
                }
            }
        }

        mpTimer->timer[appinx][kindinx] = atoi(node->item.columns[2]);
    }

    delete_all_list_node(&head, &tail);
}
#if 0 //// jjinri 0625
void loadPpsIcmpConf(void)
{
    //int i;
    //nt appinx, kindinx;
    char    *env, fname[128];
    DLinkedList ppshead, ppstail, icmphead, icmptail, *node, bcasthead, bcasttail;
    

    if ((env = getenv(IV_HOME)) == NULL){
        fprintf( stdout, "[loadPpsConf]getenv error! \n" );
        return;
    }

    sprintf (fname, "%s/%s", env, PPS_FILE);

    set_head_tail(&ppshead, &ppstail);
     if(make_conf_list(fname, &ppshead) < 0){
         fprintf (stdout, "[loadPpsIcmpConf] make linked list fail; err=%d(%s)\n", errno, strerror(errno));
         return;
    }

    sprintf (fname, "%s/%s", env, ICMP_FILE);

    set_head_tail(&icmphead, &icmptail);
     if(make_conf_list(fname, &icmphead) < 0){
         fprintf (stdout, "[loadPpsIcmpConf] make linked list fail; err=%d(%s)\n", errno, strerror(errno));
         return;
     }

#if 1	// 071011, poopee
	sprintf (fname, "%s/%s", env, BCAST_FILE);

	set_head_tail(&bcasthead, &bcasttail);
	if(make_conf_list(fname, &bcasthead) < 0){
		fprintf (stdout, "[loadPpsIcmpConf] make linked list fail; err=%d(%s)\n", errno, strerror(errno));
		return;
	}
#endif

    node = &ppshead;

    memset(ppsconf, 0x00, sizeof(st_PPS_CONF));
    if( (node=next_conf_node( node )) != NULL){
        ppsconf->uiInterimInterval = atoi(node->item.columns[0]);
        ppsconf->usMaxCnt = atoi(node->item.columns[1]);
        ppsconf->usOnOff = atoi(node->item.columns[2]);
    }

    delete_all_list_node(&ppshead, &ppstail);

    node = &icmphead;

    if( (node=next_conf_node( node )) != NULL){
        ppsconf->usFilterOut = atoi(node->item.columns[0]);
        ppsconf->usSvcType = atoi(node->item.columns[1]);
    }

    delete_all_list_node(&icmphead, &icmptail);

#if 1	// 071011, poopee
    node = &bcasthead;

    if( (node=next_conf_node( node )) != NULL){
        ppsconf->usBroadFilterOut = atoi(node->item.columns[0]);
        ppsconf->usBroadSvcType = atoi(node->item.columns[1]);
    }

    delete_all_list_node(&bcasthead, &bcasttail);
#endif
}

#endif

