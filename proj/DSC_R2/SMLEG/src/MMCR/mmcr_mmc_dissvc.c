#include "mmcr.h"

#define SVC_LINE        0
#define URL_LINE        1
#define UDR_LINE        2
#define MAX_SVC_PTR     30000

static char     msgbuff[4096];

void doDisSvcInfo(IxpcQMsgType *rxIxpcMsg, void *pt)
{

    int             srchCondFlag = 0;
    char            srchCond[16];
    MMLReqMsgType   *rxReqMsg;

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

    if(strlen(rxReqMsg->head.para[0].paraVal) > 0){
        strcpy(srchCond, rxReqMsg->head.para[0].paraVal);
        srchCondFlag = 1;
    }

    memset(msgbuff, 0x00, sizeof(msgbuff));
    if(srchCondFlag){
        dis_partly_svc_info(rxIxpcMsg, srchCond);
    }else{
        dis_all_svc_info(rxIxpcMsg);
    }

}

void dis_all_svc_info(IxpcQMsgType *rxIxpcMsg)
{
    int             i, count = 0;
    char            conffile[512], tmpbuff[1024];
    char            *iv_home;
    char            *svcidArr[MAX_SVC_PTR] = { NULL };
    //GeneralQMsgType genQMsgType;
    
	DLinkedList     svctypeH, svctypeT, urlcatH, urlcatT;
    DLinkedList     udrinfoH, udrinfoT, udrtxcH, udrtxcT;
    //DLinkedList     *node, *cur_node;


    set_head_tail(&svctypeH, &svctypeT);
    set_head_tail(&urlcatH,  &urlcatT);
    set_head_tail(&udrinfoH, &udrinfoT);
    set_head_tail(&udrtxcH,  &udrtxcT);

    iv_home = getenv(IV_HOME);
    sprintf(conffile, "%s/%s", iv_home, SERVICE_TYPE_FILE);
    make_conf_list(conffile, &svctypeH);
    sprintf(conffile, "%s/%s", iv_home, UDR_CATEGORY_FILE);
    make_conf_list(conffile, &urlcatH);
    sprintf(conffile, "%s/%s", iv_home, UDR_CDR_INFO_FILE);
    make_conf_list(conffile, &udrinfoH);
    sprintf(conffile, "%s/%s", iv_home, UDR_TXC_FILE);
    make_conf_list(conffile, &udrtxcH);

    uniq_svc_id(&svctypeH, svcidArr, 0);

    compose_svc_info_header(msgbuff);
    for(i = 0; svcidArr[i] != NULL; i++){
        count += make_svc_body(svcidArr[i], &svctypeH, &urlcatH, &udrtxcH, rxIxpcMsg);
        svcidArr[i] = NULL;
    }
    strcat(msgbuff, "    -----------------------------------------------------------"
                        "--------------------------------------------------------------------------------\n");
    uniq_svc_id(&udrinfoH, svcidArr, 1);
    for(i = 0; svcidArr[i] != NULL; i++)
        count += make_udr_body(svcidArr[i], &udrinfoH, &svctypeH, &udrtxcH, rxIxpcMsg);

    strcat(msgbuff, "    -----------------------------------------------------------"
                        "--------------------------------------------------------------------------------\n");
    sprintf(tmpbuff, "      TOTAL COUNT = %d\n", count);
    strcat(msgbuff, tmpbuff);
    strcat(msgbuff, "    =============================================================="
                        "=============================================================================\n");
    MMCResSnd(rxIxpcMsg, msgbuff, 0, 0);

    delete_all_list_node(&svctypeH, &svctypeT);
    delete_all_list_node(&urlcatH,  &urlcatT);
    delete_all_list_node(&udrinfoH, &udrinfoT);
    delete_all_list_node(&udrtxcH,  &udrtxcT);
}

void dis_partly_svc_info(IxpcQMsgType *rxIxpcMsg, char *srchStr)
{
    int             count;
    char            conffile[512], tmpbuff[1024];
    char            *iv_home;
    //GeneralQMsgType genQMsgType;
    DLinkedList     svctypeH, svctypeT, urlcatH, urlcatT;
    DLinkedList     udrinfoH, udrinfoT, udrtxcH, udrtxcT;
    //DLinkedList     *svcnode, *txcnode, *catnode;


    memset(msgbuff, 0x00, sizeof(msgbuff));
    set_head_tail(&svctypeH, &svctypeT);
    set_head_tail(&urlcatH,  &urlcatT);
    set_head_tail(&udrinfoH, &udrinfoT);
    set_head_tail(&udrtxcH,  &udrtxcT);

    iv_home = getenv(IV_HOME);
    sprintf(conffile, "%s/%s", iv_home, SERVICE_TYPE_FILE);
    make_conf_list(conffile, &svctypeH);
    sprintf(conffile, "%s/%s", iv_home, UDR_CATEGORY_FILE);
    make_conf_list(conffile, &urlcatH);
    sprintf(conffile, "%s/%s", iv_home, UDR_CDR_INFO_FILE);
    make_conf_list(conffile, &udrinfoH);
    sprintf(conffile, "%s/%s", iv_home, UDR_TXC_FILE);
    make_conf_list(conffile, &udrtxcH);


    compose_svc_info_header(msgbuff);

    count = 0;
    count = make_svc_body(srchStr, &svctypeH, &urlcatH, &udrtxcH, rxIxpcMsg);
    strcat(msgbuff, "    -----------------------------------------------------------"
                        "--------------------------------------------------------------------------------\n");
    count += make_udr_body(srchStr, &udrinfoH, &svctypeH, &udrtxcH, rxIxpcMsg);
    if(count){
        strcat(msgbuff, "    -----------------------------------------------------------"
                        "--------------------------------------------------------------------------------\n");
        sprintf(tmpbuff, "      TOTAL COUNT = %d\n", count);
        strcat(msgbuff, tmpbuff);
        strcat(msgbuff, "    ============================================================="
                            "==============================================================================\n");
    }else{
        sprintf(msgbuff, "\n    RESULT = FAIL\n    REASON = NOT EXIST\n");
    }
    MMCResSnd(rxIxpcMsg, msgbuff, 0, 0);
    
    delete_all_list_node(&svctypeH, &svctypeT);
    delete_all_list_node(&urlcatH,  &urlcatT);
    delete_all_list_node(&udrinfoH, &udrinfoT);
    delete_all_list_node(&udrtxcH,  &udrtxcT);
}

void compose_svc_info_header(char *buff)
{

    strcpy(buff, "\n    RESULT = SUCCESS\n    ============================================================="
                     "==============================================================================\n"
                 "      SVCTYPE LAVER FOUT RECORDID  IP              NETMASK PORT  BLOCK    URLCHA  CDRIP   FIRST_UDR     INTERIM URLCNT ALIAS\n"
                 "                                   URL                                                     PARENT\n"
                 "    ============================================================="
                     "==============================================================================\n");

}

void compose_svc_info_line(char *buff, char bodyPara[MAX_COL_COUNT][MAX_COL_SIZE], 
                           char txcPara[MAX_COL_COUNT][MAX_COL_SIZE], int  type, void *vptr)
{

    char    tmpcol[10][30], *cdrid;
    int     parent;

    switch(type){
        case SVC_LINE:
            strcpy(tmpcol[0], strtoupper2(bodyPara[1]));
            strcpy(tmpcol[1], strtoupper2(bodyPara[2]));
            strcpy(tmpcol[2], strtoupper2(bodyPara[6]));
            strcpy(tmpcol[3], strtoupper2(bodyPara[7]));
            strcpy(tmpcol[4], strtoupper2(bodyPara[8])); 
            strcpy(tmpcol[5], strtoupper2(bodyPara[9]));
            strcpy(tmpcol[6], strtoupper2(bodyPara[10]));
            memcpy(&parent, vptr, sizeof(int));
            sprintf(buff, "      %-7s %-5s %-4s %-9s %-15s %-7s %-5s %-8s %-6s  %-6s  %-6s        %-7s %-6s %s\n",
                    bodyPara[0], tmpcol[0], tmpcol[1], bodyPara[11], bodyPara[3],
                    bodyPara[4], bodyPara[5], tmpcol[2], tmpcol[3], tmpcol[4], tmpcol[5], txcPara[1],
                    txcPara[2], tmpcol[6]);
            break;
        case URL_LINE:
            strcpy(tmpcol[0], strtoupper2(bodyPara[3]));
            sprintf(buff, "      %-7s %-5s %-4s %-9s %-45s %-6s %-7s %-6s %s\n", 
                    bodyPara[2], "-", "-", "-", bodyPara[1],
                    bodyPara[0], txcPara[1], txcPara[2],  ((tmpcol[0][0] == '$')?"-":tmpcol[0]));
            break;
        case UDR_LINE:
            strcpy(tmpcol[0], strtoupper2(bodyPara[1]));
            strcpy(tmpcol[1], strtoupper2(bodyPara[2]));
            strcpy(tmpcol[2], strtoupper2(bodyPara[6]));
            strcpy(tmpcol[3], strtoupper2(bodyPara[7]));
            strcpy(tmpcol[4], strtoupper2(bodyPara[8])); 
            strcpy(tmpcol[5], strtoupper2(bodyPara[9])); 
            strcpy(tmpcol[6], strtoupper2(bodyPara[10]));
            cdrid = (char *)vptr;
            sprintf(buff, "      %-7s %-5s %-4s %-9s %-15s %-7s %-5s %-8s %-6s  %-6s  %-6s        %-7s %-6s %s\n",
                    cdrid, tmpcol[0], tmpcol[1], bodyPara[11], bodyPara[3],
                    bodyPara[4], bodyPara[5], tmpcol[2], tmpcol[3], tmpcol[4], tmpcol[5], txcPara[1],
                    txcPara[2], tmpcol[6]);
            break;
    }

}

int make_svc_body(char *svcid, DLinkedList *svctype,
                   DLinkedList *urlcat, DLinkedList *udrtxc,
                   IxpcQMsgType *rxIxpcMsg)
{
    char        dummyPara[MAX_COL_COUNT][MAX_COL_SIZE];
    char        tmpbuff[1024];
    int         count = 0, parent = 1;
    DLinkedList *svcnode, *catnode, *txcnode;

    memset(dummyPara, 0x00, sizeof(dummyPara));
    svcnode = search_node(svcid, 0, svctype);
    if(svcnode){
        txcnode = search_node(svcnode->item.columns[0], 0, udrtxc);
        if(!txcnode){
            txcnode = search_node("default", 0, udrtxc);
        }

        memset(tmpbuff, 0x00, sizeof(tmpbuff));
        if(txcnode)
        {
            if(strcasecmp(svcnode->item.columns[6], "CDR")){ // the 6th index is a block name.
                compose_svc_info_line(tmpbuff, svcnode->item.columns, txcnode->item.columns, SVC_LINE, (void *)&parent);
                count++;
            }
        }else{
            if(strcasecmp(svcnode->item.columns[6], "CDR")){ // the 6th index is a block name.
                compose_svc_info_line(tmpbuff, svcnode->item.columns, dummyPara, SVC_LINE, (void *)&parent);
                count++;
            }
        }

        strcat(msgbuff, tmpbuff);

        while((svcnode = search_node(svcid, 0, svcnode)) != NULL){
            memset(tmpbuff, 0x00, sizeof(tmpbuff));
            if(txcnode)
            {
                if(strcasecmp(svcnode->item.columns[6], "CDR")){ // the 6th index is a block name.
                    compose_svc_info_line(tmpbuff, svcnode->item.columns, txcnode->item.columns, SVC_LINE, (void *)&parent);
                    count++;
                }
            }else{
                if(strcasecmp(svcnode->item.columns[6], "CDR")){ // the 6th index is a block name.
                    compose_svc_info_line(tmpbuff, svcnode->item.columns, dummyPara, SVC_LINE, (void *)&parent);
                    count++;
                }
            }

            strcat(msgbuff, tmpbuff);
            if(strlen(msgbuff) > 3072){
                strcat(msgbuff, "    -----------------------------------------------------------"
                                    "--------------------------------------------------------------------------------\n");
                MMCResSnd(rxIxpcMsg, msgbuff, 0, 1);
                memset(msgbuff, 0x00, sizeof(msgbuff));
                compose_svc_info_header(msgbuff);
            }
        }

        catnode = search_node(svcid, 0, urlcat);
        parent = 0;
        if(catnode){
            count++;
            if(txcnode)
            {
                compose_svc_info_line(tmpbuff, catnode->item.columns, txcnode->item.columns, URL_LINE, (void *)&parent);
            }else{
                compose_svc_info_line(tmpbuff, catnode->item.columns, dummyPara, URL_LINE, (void *)&parent);
            }

            strcat(msgbuff, tmpbuff);
            if(strlen(msgbuff) > 3072){
                strcat(msgbuff, "    -----------------------------------------------------------"
                                    "--------------------------------------------------------------------------------\n");
                MMCResSnd(rxIxpcMsg, msgbuff, 0, 1);
                memset(msgbuff, 0x00, sizeof(msgbuff));
                compose_svc_info_header(msgbuff);
            }

            while((catnode = search_node(svcid, 0, catnode)) != NULL){
                count++;
                if(txcnode)
                {
                    compose_svc_info_line(tmpbuff, catnode->item.columns, txcnode->item.columns, URL_LINE, (void *)&parent);
                }else{
                    compose_svc_info_line(tmpbuff, catnode->item.columns, dummyPara, URL_LINE, (void *)&parent);
                }

                strcat(msgbuff, tmpbuff);
                if(strlen(msgbuff) > 3072){
                strcat(msgbuff, "    -----------------------------------------------------------"
                                    "--------------------------------------------------------------------------------\n");
                    MMCResSnd(rxIxpcMsg, msgbuff, 0, 1);
                    memset(msgbuff, 0x00, sizeof(msgbuff));
                    compose_svc_info_header(msgbuff);
                }
            }
        }

    }
    return count;
}

int  make_udr_body(char *cdrsvcid, DLinkedList *udrinfo,
                   DLinkedList *svctype, DLinkedList *udrtxc,
                   IxpcQMsgType *rxIxpcMsg)
{
    char        dummyPara[MAX_COL_COUNT][MAX_COL_SIZE];
    //char        tmpbuff[1024];
    int         count = 0;
    DLinkedList *udrnode;

    memset(dummyPara, 0x00, sizeof(dummyPara));
    udrnode = search_node(cdrsvcid, 1, udrinfo);
    if(udrnode){
/*
        txcnode = search_node(udrnode->item.columns[0], 0, udrtxc);
        if(!txcnode){
            txcnode = search_node("default", 0, udrtxc);
        }
        if(txcnode)
        {
            compose_svc_info_line(tmpbuff, udrnode->item.columns, txcnode->item.columns, UDR_LINE, 1);
        }else{
            compose_svc_info_line(tmpbuff, udrnode->item.columns, dummyPara, UDR_LINE, 1);
        }
        strcat(msgbuff, tmpbuff);
        if(strlen(msgbuff) > 3072){
            strcat(msgbuff, "    -----------------------------------------------------------"
                                "-------------------------------------------------------\n");
            MMCResSnd(rxIxpcMsg, msgbuff, 0, 1);
            memset(msgbuff, 0x00, sizeof(msgbuff));
            compose_svc_info_header(msgbuff);
        }
*/
        count += make_cdr_body(cdrsvcid, udrnode->item.columns[0], svctype, udrtxc, rxIxpcMsg);

        while((udrnode = search_node(cdrsvcid, 1, udrnode)) != NULL){
/*            if(txcnode)
            {
                compose_svc_info_line(tmpbuff, udrnode->item.columns, txcnode->item.columns, UDR_LINE, 1);
            }else{
                compose_svc_info_line(tmpbuff, udrnode->item.columns, dummyPara, UDR_LINE, 1);
            }

            strcat(msgbuff, tmpbuff);
            if(strlen(msgbuff) > 3072){
                strcat(msgbuff, "    -----------------------------------------------------------"
                                    "-------------------------------------------------------\n");
                MMCResSnd(rxIxpcMsg, msgbuff, 0, 1);
                memset(msgbuff, 0x00, sizeof(msgbuff));
                compose_svc_info_header(msgbuff);
            }
*/
            count += make_cdr_body(cdrsvcid, udrnode->item.columns[0], svctype, udrtxc, rxIxpcMsg);
        }

    }

    return count;
}

int  make_cdr_body(char *cdrsvcid, char *svcid, DLinkedList *svctype,
                   DLinkedList *udrtxc, IxpcQMsgType *rxIxpcMsg)
{
    int         count = 0;
    char        dummyPara[MAX_COL_COUNT][MAX_COL_SIZE];
    char        tmpbuff[1024];
    DLinkedList *svcnode, *txcnode;

    memset(dummyPara, 0x00, sizeof(dummyPara));
    svcnode = search_node(svcid, 0, svctype);
    if(svcnode){
        txcnode = search_node(svcnode->item.columns[0], 0, udrtxc);
        if(!txcnode){
            txcnode = search_node("default", 0, udrtxc);
        }

        memset(tmpbuff, 0x00, sizeof(tmpbuff));
        if(txcnode)
        {
            if(!strcasecmp(svcnode->item.columns[6], "CDR")){ // the 6th index is a block name.
                compose_svc_info_line(tmpbuff, svcnode->item.columns, txcnode->item.columns, UDR_LINE, (void *)cdrsvcid);
                count++;
            }
        }else{
            if(!strcasecmp(svcnode->item.columns[6], "CDR")){
                compose_svc_info_line(tmpbuff, svcnode->item.columns, dummyPara, UDR_LINE, (void *)cdrsvcid);
                count++;
            }
        }

        strcat(msgbuff, tmpbuff);

        while((svcnode = search_node(svcid, 0, svcnode)) != NULL){
            memset(tmpbuff, 0x00, sizeof(tmpbuff));
            if(txcnode)
            {
                if(!strcasecmp(svcnode->item.columns[6], "CDR")){
                    compose_svc_info_line(tmpbuff, svcnode->item.columns, txcnode->item.columns, UDR_LINE, (void *)cdrsvcid);
                    count++;
                }
            }else{
                if(!strcasecmp(svcnode->item.columns[6], "CDR")){
                    compose_svc_info_line(tmpbuff, svcnode->item.columns, dummyPara, UDR_LINE, (void *)cdrsvcid);
                    count++;
                }
            }

            strcat(msgbuff, tmpbuff);
            if(strlen(msgbuff) > 3072){
                strcat(msgbuff, "    -----------------------------------------------------------"
                                    "------------------------------------------------------------------\n");
                MMCResSnd(rxIxpcMsg, msgbuff, 0, 1);
                memset(msgbuff, 0x00, sizeof(msgbuff));
                compose_svc_info_header(msgbuff);
            }
        }
    }
    return count;
}

void uniq_svc_id(DLinkedList *head, char *svcidarr[], int pos)
{
    int     i, exist;
    DLinkedList *node = head;

    while(node->next){
        node = node->next;

        if(node->item.gubun != TOK_CONF_LINE){
            continue;
        }

        exist = 0;
        for(i = 0; svcidarr[i] != NULL; i++){
            if(!strcasecmp(svcidarr[i], node->item.columns[pos])){
                exist = 1;
                break;
            }
        }
        if(!exist)
            svcidarr[i] = node->item.columns[pos];
    }

}
