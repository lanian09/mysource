#include "mmcr.h"

int doAddSvcType(void *tbl, IxpcQMsgType *rxIxpcMsg,
                  GeneralQMsgType genQMsgType, void *pft,
                  MpConfigCmd *mcc, DLinkedList *head, DLinkedList *tail,
                  char columns[MAX_COL_COUNT][MAX_COL_SIZE],char  fname[FILESIZE])
{
    char    txBuf[MMCMSGSIZE];
    pMmcFuncTable ft;
    ft=(pMmcFuncTable)pft;
    unsigned int    uiIP, uiNetMask;
    //struct in_addr intIp;
    //char * pIp;
    //char * pNetMask;

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
    uiIP = inet_addr(columns[3]);
    uiNetMask = atoi( columns[4] );
    if( uiIP == (in_addr_t)-1 || uiNetMask < 0 || uiNetMask > 32)
    {
        sprintf(txBuf, "\n    RESULT = FAIL\n    REASON =  INVALID PARAMETER FAIL\n");
        MMCResSnd(rxIxpcMsg, txBuf, 1, 0);
        return -1;
    }

#if 0
    // inet_addr converts alreay
    // uiNetMask가 24였다면 8bit만큼 왼쪽으로 shift되는군..ffffff00 이렇게... 
	uiNetMask = 0xffffffff << (32 - uiNetMask);

    uiIP = htonl(uiIP) | ~uiNetMask; // '~' 는 보수에 대한 논리 연산자 ip:1.1.1.0 netmask:24이면 결과는 1.1.1.255
    intIp.s_addr = ntohl(uiIP);
    pIp = inet_ntoa(intIp);
    strcpy( columns[3], pIp );
#endif  


    //이미 존재하는지 체크
    // svc_type, IP address, netmask, port 등 비교
	if (find_same_svctype_key(columns, head) != NULL ) {
        sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = ALREADY EXIST (SAME SVC_TYPE)\n");
        MMCResSnd(rxIxpcMsg, txBuf, 1, 0);
        return -1;
    } else {
		
        //node insert
        insert_svctype_node(columns, ft->paraCnt , head);

        //file insert
        if ( flush_list_to_file( head, fname ) < 0 ) {
        	fprintf (stdout, "[doAddAaaInfo] file save fail[%s]; err=%d(%s)\n", fname, errno, strerror(errno));
            sprintf(txBuf, "\n    RESULT = FAIL\n    REASON = INFOMATION ADD FAIL\n");
            MMCResSnd(rxIxpcMsg, txBuf, 1, 0);
            return -1;
        }

    }
    return 1;
}

DLinkedList *find_same_svctype_key(char srchStr[MAX_COL_COUNT][MAX_COL_SIZE], DLinkedList *head)
{
    int         found = 0;
    DLinkedList *node = head;
    while(node->next){
        node = node->next;

        if(node->item.gubun != TOK_CONF_LINE){
            continue;
        }

        if(!strcmp(srchStr[0], node->item.columns[0]) && // Svc Type
           !compare_strIPAddr(srchStr[3], node->item.columns[3]) && // IP Address
           !strcmp(srchStr[4], node->item.columns[4]) && // Netmask
           !strcmp(srchStr[5], node->item.columns[5])){  // PORT
            found = 1;
            break;
        }
    }
    if(found)
        return node;
    else
        return NULL;

}

DLinkedList *insert_svctype_node(char insitem[MAX_COL_COUNT][MAX_COL_SIZE], int colsize, DLinkedList *head)
{
    int     i, seq;
   	char	*on = {"on"};
    char	*off = {"off"};	
	char    first[MAX_COL_SIZE], alias[MAX_COL_SIZE];
	DLinkedList *node, *prev, *confend;
    confend = search_jump_node(head, colsize, &seq);
    if ( confend==NULL )
        return NULL;

    
    prev = confend->prev;
    if( (node = new_node(prev)) == NULL){
        //fail to allocate new node
        return NULL;
    }

    node->item.gubun = TOK_CONF_LINE;
    for(i = 0; i < colsize; i++){ 
   		if( (i == 8) && (!strcasecmp(insitem[6], "CDR")) ){
        	strncpy(node->item.columns[i], on, MAX_COL_SIZE);
		}else if ( (i == 8) && (strcasecmp(insitem[6], "CDR")) ){
		    strncpy(node->item.columns[i], off, MAX_COL_SIZE);
		}else if (i > 8 ){
			strncpy(first, insitem[8], MAX_COL_SIZE);
			strncpy(alias, insitem[9], MAX_COL_SIZE);

			strncpy(insitem[8], node->item.columns[8], MAX_COL_SIZE);
			strncpy(insitem[9], first, MAX_COL_SIZE);
			strncpy(insitem[10], alias, MAX_COL_SIZE);
			
			strncpy(node->item.columns[9], insitem[9], MAX_COL_SIZE);
			strncpy(node->item.columns[10], insitem[10], MAX_COL_SIZE);
				
			break;
		}else
        	strncpy(node->item.columns[i], insitem[i], MAX_COL_SIZE);
//fprintf(stderr, "%d  columns[%d]:%s\n", i, i, &node->item.columns[i]);	
	}

    
	sprintf(node->item.columns[colsize], "%d", seq);
    sprintf( insitem[colsize], "%d", seq );

    fill_line_buff(node->item.columns, node->item.line, colsize+1);

    return node;

}


DLinkedList *search_jump_node(DLinkedList *head, int posSeq, int *seq)
{
    int     i;
    short   numArr[JMM_DESTPORT_RECORD], num;
    DLinkedList *node = head;

    for(i = 0; i < JMM_DESTPORT_RECORD; i++)
        numArr[i] = 0;

    i = 0;
    while(node->next){
        node = node->next;
        if(node->item.gubun == TOK_END_LINE) break;
        if(node->item.gubun != TOK_CONF_LINE) continue;
        num = (short)atoi(node->item.columns[posSeq]);
        numArr[num-1] = 1;
    }

    for(i = 0; i < JMM_DESTPORT_RECORD; i++){
        if(!numArr[i])
            break;
    }

    *seq = (i + 1);

    return node;
}


int compare_strIPAddr(char *addr1, char *addr2)
{
    in_addr_t   addrA, addrB;

    addrA = inet_addr(addr1);
    addrB = inet_addr(addr2);

    if(memcmp((void *)&addrA, (void *)&addrB, sizeof(in_addr_t))){
        return 1;
    }else{
        return 0;
    }
}

unsigned char get_protocol_enum(char *proto)
{
    unsigned char ret = 0x00;

    if(!strcasecmp(proto, "TCP")){
        ret = 6;
    }else if(!strcasecmp(proto, "UDP")){
        ret = 17;
    }

    return ret;
    
}
