#include "mmcr.h"

int doDisCommProcess( void * ,IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,void *,MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE], char fname[FILESIZE] );
int doDisTrcProcess( void *tbl ,IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,void *pft,MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE], char fname[FILESIZE] );

int doDisCommProcess( void *tbl ,IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,void *pft,MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE], char fname[FILESIZE] )
{
    DLinkedList *node, *snode;
    pMmcMemberTable mmcHdlr;
    char    bodyMag[MMCMSGSIZE], totalBuffer[BUFSIZE], titleMag[BUFSIZE],tmpMag[BUFSIZE], txBuf[MMCMSGSIZE];
    int cnt=0, len=0, totalCnt=0;
     pMmcFuncTable ft;
    ft=(pMmcFuncTable)pft;


    mmcHdlr=(pMmcMemberTable)tbl;
    node = head;
    memset(txBuf, 0, sizeof(txBuf));
    memset(tmpMag, 0, sizeof(tmpMag));
    memset(bodyMag, 0, sizeof(bodyMag));
    memset(totalBuffer, 0, sizeof(totalBuffer));
    memset(titleMag, 0, sizeof(titleMag));

    if(strlen(columns[0]) > 0){
        snode = search_node(columns[0], ft->startIndex, node);
        if(snode){
            headMessage( mmcHdlr, titleMag );
            sprintf( txBuf, "\n    RESULT = SUCCESS   " );
            bodyLineMessage( mmcHdlr, snode, tmpMag );
            strcat(bodyMag, tmpMag );
            totalCnt++;

            while((snode = search_node(columns[0], ft->startIndex, NULL)) != NULL){
                bodyLineMessage( mmcHdlr, snode, tmpMag );
                strcat(bodyMag, tmpMag );
                totalCnt++;
                len = strlen(bodyMag);
                  if(  len >= 3072 ){
                    conTailMessage( mmcHdlr, totalBuffer);

                    //message create
                    strcat(txBuf, titleMag );       //title
                    strcat(txBuf, bodyMag);          // body
                    strcat(txBuf, totalBuffer);    //------------
                    //fprintf( stderr, "titleMag : %s", titleMag);

                    MMCResSnd(rxIxpcMsg, txBuf, 0, 1);
                    memset( bodyMag, 0, sizeof( bodyMag ) );
                    memset( totalBuffer, 0, sizeof( totalBuffer ) );
                    memset( txBuf, 0, sizeof( txBuf ) );
                }
            }

            tailMessage( mmcHdlr, totalBuffer, totalCnt );

            //message create
            strcat(txBuf, titleMag );       //title
            strcat(txBuf, bodyMag);          // body
            strcat(txBuf, totalBuffer);    //------------
        }else{
            sprintf( txBuf, "\n    RESULT = FAIL   \n    REASON = SEARCH KEY NOT FOUND\n" );
        }
    }else{
        headMessage( mmcHdlr, titleMag );

        while( (node=next_conf_node_compare( node,columns,ft->startIndex,ft->keyCnt, mcc->cmdParaCnt )) != NULL ) { 
            sprintf( txBuf, "\n    RESULT = SUCCESS   " );
            bodyLineMessage( mmcHdlr, node, tmpMag );
            strcat(bodyMag, tmpMag );
            cnt++;
            totalCnt++;
            len = strlen(bodyMag);
              if(  len >= 3072 ){
                conTailMessage( mmcHdlr, totalBuffer);

                //message create
                strcat(txBuf, titleMag );       //title
                strcat(txBuf, bodyMag);          // body
                strcat(txBuf, totalBuffer);    //------------
                //fprintf( stderr, "titleMag : %s", titleMag);

                MMCResSnd(rxIxpcMsg, txBuf, 0, 1);
                memset( bodyMag, 0, sizeof( bodyMag ) );
                memset( totalBuffer, 0, sizeof( totalBuffer ) );
                memset( txBuf, 0, sizeof( txBuf ) );
                cnt = 0;
            }
        }

        tailMessage( mmcHdlr, totalBuffer, totalCnt );

        //message create
        strcat(txBuf, titleMag );       //title
        strcat(txBuf, bodyMag);          // body
        strcat(txBuf, totalBuffer);    //------------

//        fprintf( stderr, "dismmc : %s", txBuf);

    }

    MMCResSnd(rxIxpcMsg, txBuf, 0, 0);

    return 1;
}

int doDisUrlProcess( void *tbl ,IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,void *pft,MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE], char fname[FILESIZE] )
{
    DLinkedList *node, *snode;
    pMmcMemberTable mmcHdlr;
    char    bodyMag[MMCMSGSIZE], totalBuffer[BUFSIZE], titleMag[BUFSIZE],tmpMag[BUFSIZE], txBuf[MMCMSGSIZE];
    int cnt=0, len=0, totalCnt=0;
     pMmcFuncTable ft;
    ft=(pMmcFuncTable)pft;


    mmcHdlr=(pMmcMemberTable)tbl;
    node = head;
    memset(txBuf, 0, sizeof(txBuf));
    memset(tmpMag, 0, sizeof(tmpMag));
    memset(bodyMag, 0, sizeof(bodyMag));
    memset(totalBuffer, 0, sizeof(totalBuffer));
    memset(titleMag, 0, sizeof(titleMag));

    if(strlen(columns[0]) > 0){
        snode = search_node(columns[0], ft->startIndex, node);
        if(snode){
            headMessage( mmcHdlr, titleMag );
            sprintf( txBuf, "\n    RESULT = SUCCESS   " );
            bodyLineUrlMessage( mmcHdlr, snode, tmpMag );
            strcat(bodyMag, tmpMag );
            totalCnt++;

            while((snode = search_node(columns[0], ft->startIndex, NULL)) != NULL){
                bodyLineUrlMessage( mmcHdlr, snode, tmpMag );
                strcat(bodyMag, tmpMag );
                totalCnt++;
                len = strlen(bodyMag);
                  if(  len >= 3072 ){
                    conTailMessage( mmcHdlr, totalBuffer);

                    //message create
                    strcat(txBuf, titleMag );       //title
                    strcat(txBuf, bodyMag);          // body
                    strcat(txBuf, totalBuffer);    //------------
                    //fprintf( stderr, "titleMag : %s", titleMag);

                    MMCResSnd(rxIxpcMsg, txBuf, 0, 1);
                    memset( bodyMag, 0, sizeof( bodyMag ) );
                    memset( totalBuffer, 0, sizeof( totalBuffer ) );
                    memset( txBuf, 0, sizeof( txBuf ) );
                }
            }

            tailMessage( mmcHdlr, totalBuffer, totalCnt );

            //message create
            strcat(txBuf, titleMag );       //title
            strcat(txBuf, bodyMag);          // body
            strcat(txBuf, totalBuffer);    //------------
        }else{
            sprintf( txBuf, "\n    RESULT = FAIL   \n    REASON = SEARCH KEY NOT FOUND\n" );
        }
    }else{
        headMessage( mmcHdlr, titleMag );
        while( (node=next_conf_node_compare( node,columns,ft->startIndex,ft->keyCnt, mcc->cmdParaCnt )) != NULL ) {
            sprintf( txBuf, "\n    RESULT = SUCCESS   " );
            bodyLineUrlMessage( mmcHdlr, node, tmpMag );
            strcat(bodyMag, tmpMag );
            cnt++;
            totalCnt++;
            len = strlen(bodyMag);
            if( cnt>=20 || len >= 3072 ){
                conTailMessage( mmcHdlr, totalBuffer);

                //message create
                strcat(txBuf, titleMag );       //title
                strcat(txBuf, bodyMag);          // body
                strcat(txBuf, totalBuffer);    //------------
                //fprintf( stderr, "titleMag : %s", titleMag);

                MMCResSnd(rxIxpcMsg, txBuf, 0, 1);
                memset( bodyMag, 0, sizeof( bodyMag ) );
                memset( totalBuffer, 0, sizeof( totalBuffer ) );
                memset( txBuf, 0, sizeof( txBuf ) );
                cnt = 0;
            }
        }

        tailMessage( mmcHdlr, totalBuffer, totalCnt );

        //message create
        strcat(txBuf, titleMag );       //title
        strcat(txBuf, bodyMag);          // body
        strcat(txBuf, totalBuffer);    //------------

    }

    MMCResSnd(rxIxpcMsg, txBuf, 0, 0);
    return 1;
}

int doDisAaaProcess( void *tbl ,IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,void *pft,MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE], char fname[FILESIZE] )
{
    DLinkedList *node, *snode;
    pMmcMemberTable mmcHdlr;
    char    bodyMag[MMCMSGSIZE], totalBuffer[BUFSIZE], titleMag[BUFSIZE],tmpMag[BUFSIZE], txBuf[MMCMSGSIZE];
    int cnt=0, len=0, totalCnt=0;
     pMmcFuncTable ft;
    ft=(pMmcFuncTable)pft;


    mmcHdlr=(pMmcMemberTable)tbl;
    node = head;
    memset(txBuf, 0, sizeof(txBuf));
    memset(tmpMag, 0, sizeof(tmpMag));
    memset(bodyMag, 0, sizeof(bodyMag));
    memset(totalBuffer, 0, sizeof(totalBuffer));
    memset(titleMag, 0, sizeof(titleMag));

    if(strlen(columns[0]) > 0){
        snode = search_node(columns[0], ft->startIndex, node);
        if(snode){
            headMessage( mmcHdlr, titleMag );
            sprintf( txBuf, "\n    RESULT = SUCCESS   " );
            bodyLineAaaMessage( mmcHdlr, snode, tmpMag );
            strcat(bodyMag, tmpMag );
            totalCnt++;

            while((snode = search_node(columns[0], ft->startIndex, NULL)) != NULL){
                bodyLineAaaMessage( mmcHdlr, snode, tmpMag );
                strcat(bodyMag, tmpMag );
                totalCnt++;
                len = strlen(bodyMag);
                  if(  len >= 3072 ){
                    conTailMessage( mmcHdlr, totalBuffer);

                    //message create
                    strcat(txBuf, titleMag );       //title
                    strcat(txBuf, bodyMag);          // body
                    strcat(txBuf, totalBuffer);    //------------
                    //fprintf( stderr, "titleMag : %s", titleMag);

                    MMCResSnd(rxIxpcMsg, txBuf, 0, 1);
                    memset( bodyMag, 0, sizeof( bodyMag ) );
                    memset( totalBuffer, 0, sizeof( totalBuffer ) );
                    memset( txBuf, 0, sizeof( txBuf ) );
                }
            }

            tailMessage( mmcHdlr, totalBuffer, totalCnt );

            //message create
            strcat(txBuf, titleMag );       //title
            strcat(txBuf, bodyMag);          // body
            strcat(txBuf, totalBuffer);    //------------
        }else{
            sprintf( txBuf, "\n    RESULT = FAIL   \n    REASON = SEARCH KEY NOT FOUND\n" );
        }
    }else{

        headMessage( mmcHdlr, titleMag );

        while( (node=next_conf_node_compare( node,columns,ft->startIndex,ft->keyCnt, mcc->cmdParaCnt )) != NULL ) {
            sprintf( txBuf, "\n    RESULT = SUCCESS   " );
            bodyLineAaaMessage( mmcHdlr, node, tmpMag );
            strcat(bodyMag, tmpMag );
            cnt++;
            totalCnt++;
            len = strlen(bodyMag);
            if( cnt>=20 || len >= 3072 ){
                conTailMessage( mmcHdlr, totalBuffer);

                //message create
                strcat(txBuf, titleMag );       //title
                strcat(txBuf, bodyMag);          // body
                strcat(txBuf, totalBuffer);    //------------
                //fprintf( stderr, "titleMag : %s", titleMag);

                MMCResSnd(rxIxpcMsg, txBuf, 0, 1);
                memset( bodyMag, 0, sizeof( bodyMag ) );
                memset( totalBuffer, 0, sizeof( totalBuffer ) );
                memset( txBuf, 0, sizeof( txBuf ) );
                cnt = 0;
            }
        }

        tailMessage( mmcHdlr, totalBuffer, totalCnt );

        //message create
        strcat(txBuf, titleMag );       //title
        strcat(txBuf, bodyMag);          // body
        strcat(txBuf, totalBuffer);    //------------
    }

    MMCResSnd(rxIxpcMsg, txBuf, 0, 0);
    return 1;
}


int doDisTrcProcess( void *tbl ,IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,void *pft,MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE], char fname[FILESIZE] )
{
  DLinkedList *node;
    pMmcMemberTable mmcHdlr;
    char    bodyMag[MMCMSGSIZE], totalBuffer[BUFSIZE], titleMag[BUFSIZE],tmpMag[BUFSIZE], txBuf[MMCMSGSIZE];
    int cnt=0, len=0, totalCnt=0;
    pMmcFuncTable ft;
    ft=(pMmcFuncTable)pft;


    mmcHdlr=(pMmcMemberTable)tbl;
    node = head;
    memset(txBuf, 0, sizeof(txBuf));
    memset(tmpMag, 0, sizeof(tmpMag));
    memset(bodyMag, 0, sizeof(bodyMag));
    memset(totalBuffer, 0, sizeof(totalBuffer));
    memset(titleMag, 0, sizeof(titleMag));

        //header message format
        //sprintf(tmpMag, "\n    %s %s\n    M3301 DISPLAY AAA INFORMATION\n",
        //    mySysName,
        //    commlib_printTStamp());
        //strcat(txBuf, tmpMag);

        headMessage( mmcHdlr, titleMag );
        while( (node=next_conf_node( node )) != NULL){
            sprintf( txBuf, "\n    RESULT = SUCCESS   " );
            bodyLineTrcMessage( mmcHdlr, node, tmpMag );
            strcat(bodyMag, tmpMag );
            cnt++;
            totalCnt++;
            len = strlen(bodyMag);
            if( cnt>=20 || len >= 3072 ){
                conTailMessage( mmcHdlr, totalBuffer );

                //message create
                strcat(txBuf, titleMag );       //title
                strcat(txBuf, bodyMag);          // body
                strcat(txBuf, totalBuffer);    //------------
                //fprintf( stderr, "titleMag : %s", titleMag);

                MMCResSnd(rxIxpcMsg, txBuf, 0, 1);
                memset( bodyMag, 0, sizeof( bodyMag ) );
                memset( totalBuffer, 0, sizeof( totalBuffer ) );
                memset( txBuf, 0, sizeof( txBuf ) );
                cnt=0;
            }
        }

        tailMessage( mmcHdlr, totalBuffer, totalCnt );

        //message create
        strcat(txBuf, titleMag );       //title
        strcat(txBuf, bodyMag);          // body
        strcat(txBuf, totalBuffer);    //------------

//        fprintf( stderr, "dismmc : %s", txBuf);

        MMCResSnd(rxIxpcMsg, txBuf, 0, 0);
    return 1; 

}


int doDisPdsnProcess( void *tbl ,IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,void *pft,MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE], char fname[FILESIZE] )
{
  DLinkedList *node;
    pMmcMemberTable mmcHdlr;
    char    bodyMag[MMCMSGSIZE], totalBuffer[BUFSIZE], titleMag[BUFSIZE],tmpMag[BUFSIZE], txBuf[MMCMSGSIZE];
    int cnt=0, len=0, totalCnt=0;
    pMmcFuncTable ft;
    ft=(pMmcFuncTable)pft;


    mmcHdlr=(pMmcMemberTable)tbl;
    node = head;
    memset(txBuf, 0, sizeof(txBuf));
    memset(tmpMag, 0, sizeof(tmpMag));
    memset(bodyMag, 0, sizeof(bodyMag));
    memset(totalBuffer, 0, sizeof(totalBuffer));
    memset(titleMag, 0, sizeof(titleMag));

        //header message format
        //sprintf(tmpMag, "\n    %s %s\n    M3301 DISPLAY AAA INFORMATION\n",
        //    mySysName,
        //    commlib_printTStamp());
        //strcat(txBuf, tmpMag);

        headMessage( mmcHdlr, titleMag );
        while( (node=next_conf_node( node )) != NULL){
            sprintf( txBuf, "\n    RESULT = SUCCESS   " );
            bodyLinePdsnMessage( mmcHdlr, node, tmpMag );
            strcat(bodyMag, tmpMag );
            cnt++;
            totalCnt++;
            len = strlen(bodyMag);
            if( cnt>=20 || len >= 3072 ){
                conTailMessage( mmcHdlr, totalBuffer );

                //message create
                strcat(txBuf, titleMag );       //title
                strcat(txBuf, bodyMag);          // body
                strcat(txBuf, totalBuffer);    //------------
                //fprintf( stderr, "titleMag : %s", titleMag);

                MMCResSnd(rxIxpcMsg, txBuf, 0, 1);
                memset( bodyMag, 0, sizeof( bodyMag ) );
                memset( totalBuffer, 0, sizeof( totalBuffer ) );
                memset( txBuf, 0, sizeof( txBuf ) );
                cnt=0;
            }
        }

        tailMessage( mmcHdlr, totalBuffer, totalCnt );

        //message create
        strcat(txBuf, titleMag );       //title
        strcat(txBuf, bodyMag);          // body
        strcat(txBuf, totalBuffer);    //------------

//        fprintf( stderr, "dismmc : %s", txBuf);

        MMCResSnd(rxIxpcMsg, txBuf, 0, 0);

    return 1;
}

int doDisSvcProcess( void *tbl ,IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,void *pft,MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE], char fname[FILESIZE] )
{
    DLinkedList *node, *snode, *anode, *bnode;
    pMmcMemberTable mmcHdlr;
    char    bodyMag[MMCMSGSIZE], totalBuffer[BUFSIZE], titleMag[BUFSIZE],tmpMag[BUFSIZE], txBuf[MMCMSGSIZE];
    int cnt=0, len=0, totalCnt=0, srchCnt=0;
     
	pMmcFuncTable ft;
    ft=(pMmcFuncTable)pft;

	//int testCnt=0;

    mmcHdlr=(pMmcMemberTable)tbl;
    node = head;
    memset(txBuf, 0, sizeof(txBuf));
    memset(tmpMag, 0, sizeof(tmpMag));
    memset(bodyMag, 0, sizeof(bodyMag));
    memset(totalBuffer, 0, sizeof(totalBuffer));
    memset(titleMag, 0, sizeof(titleMag));

	//fprintf(stderr, "colunms[0]: %s, columns[1]: %s, columns[2]: %s\n", columns[0], columns[1], columns[2]);
    // search with service id
    if( (strlen(columns[0]) > 0) && (strlen(columns[1]) > 0) && (strlen(columns[2]) > 0) ){
		snode = search_node(columns[0], ft->startIndex, node);
        if(snode){
            headMessage( mmcHdlr, titleMag );
            sprintf( txBuf, "\n    RESULT = SUCCESS   " );

			while((snode = search_node(columns[0], ft->startIndex, node)) != NULL){	
				//fprintf(stderr, "test point01\n");	
				if((anode = search_node2(columns[1], 3, snode)) != NULL) {
					//fprintf(stderr, "test point02\n");
					if((bnode = search_node2(columns[2], 6, anode)) != NULL){
						//fprintf(stderr, "test point03\n");
						node = anode;
						bodyLineSvcMessage( mmcHdlr, bnode, tmpMag );
						strcat(bodyMag, tmpMag );
                		totalCnt++;
						srchCnt++;
						len = strlen(bodyMag);
                  		if(  len >= 3072 ){
                    		conTailMessage( mmcHdlr, totalBuffer);
                    		//message create
                    		strcat(txBuf, titleMag );       //title
                    		strcat(txBuf, bodyMag);          // body
                    		strcat(txBuf, totalBuffer);    //------------
                    		//fprintf( stderr, "titleMag : %s", titleMag);

                    		MMCResSnd(rxIxpcMsg, txBuf, 0, 1);
                    		memset( bodyMag, 0, sizeof( bodyMag ) );
                    		memset( totalBuffer, 0, sizeof( totalBuffer ) );
                    		memset( txBuf, 0, sizeof( txBuf ) );
                		}
						//continue;
           			}else
					   node = anode;	
				}else 
				   node = snode;	
			}

            tailMessage( mmcHdlr, totalBuffer, totalCnt );

            //message create
            strcat(txBuf, titleMag );       //title
            strcat(txBuf, bodyMag);          // body
            strcat(txBuf, totalBuffer);    //------------
        }else{
            sprintf( txBuf, "\n    RESULT = FAIL   \n    REASON = SEARCH KEY NOT FOUND\n" );
        }

		if(srchCnt == 0){
			sprintf( txBuf, "\n    RESULT = FAIL   \n    REASON = SEARCH KEY NOT FOUND\n" );
		}
    }

    else if( (strlen(columns[0]) <= 0) && (strlen(columns[1]) > 0) && (strlen(columns[2]) > 0) ){
    	snode = search_node(columns[1], 3, node);
    	if(snode){
        	headMessage( mmcHdlr, titleMag );
        	sprintf( txBuf, "\n    RESULT = SUCCESS   " );

   	    	while((snode = search_node(columns[1], 3, node)) != NULL){
            	if((anode = search_node2(columns[2], 6, snode)) != NULL) {
                    node = snode;
                    bodyLineSvcMessage( mmcHdlr, anode, tmpMag );
                    strcat(bodyMag, tmpMag );
                    totalCnt++;
					srchCnt++;
                    len = strlen(bodyMag);
                    if(  len >= 3072 ){
                        conTailMessage( mmcHdlr, totalBuffer);
                        //message create
                        strcat(txBuf, titleMag );       //title
                        strcat(txBuf, bodyMag);          // body
                        strcat(txBuf, totalBuffer);    //------------
                        //fprintf( stderr, "titleMag : %s", titleMag);

                        MMCResSnd(rxIxpcMsg, txBuf, 0, 1);
                        memset( bodyMag, 0, sizeof( bodyMag ) );
                        memset( totalBuffer, 0, sizeof( totalBuffer ) );
                        memset( txBuf, 0, sizeof( txBuf ) );
                    }
                    //continue;
            	}else
               		node = snode;
        	}

        	tailMessage( mmcHdlr, totalBuffer, totalCnt );

        	//message create
        	strcat(txBuf, titleMag );       //title
        	strcat(txBuf, bodyMag);          // body
        	strcat(txBuf, totalBuffer);    //------------
    	}else{
        	sprintf( txBuf, "\n    RESULT = FAIL   \n    REASON = SEARCH KEY NOT FOUND\n" );
    	}
		if(srchCnt == 0){
			sprintf( txBuf, "\n    RESULT = FAIL   \n    REASON = SEARCH KEY NOT FOUND\n" );
		}
	}

	else if( (strlen(columns[0]) <= 0) && (strlen(columns[1]) <= 0) && (strlen(columns[2]) > 0) ){
    	snode = search_node(columns[2], 6, node);
    	if(snode){
        	headMessage( mmcHdlr, titleMag );
        	sprintf( txBuf, "\n    RESULT = SUCCESS   " );
			bodyLineSvcMessage( mmcHdlr, snode, tmpMag );
			strcat(bodyMag, tmpMag );	
			srchCnt++;	
			totalCnt++;
        	while((snode = search_node(columns[2], 6, NULL)) != NULL){
				bodyLineSvcMessage( mmcHdlr, snode, tmpMag );
            	strcat(bodyMag, tmpMag );
            	totalCnt++;
				srchCnt++;
            	len = strlen(bodyMag);
            	if(  len >= 3072 ){
                	conTailMessage( mmcHdlr, totalBuffer);
                	//message create
                	strcat(txBuf, titleMag );       //title
                	strcat(txBuf, bodyMag);          // body
                	strcat(txBuf, totalBuffer);    //------------
                	//fprintf( stderr, "titleMag : %s", titleMag);

                	MMCResSnd(rxIxpcMsg, txBuf, 0, 1);
                	memset( bodyMag, 0, sizeof( bodyMag ) );
                	memset( totalBuffer, 0, sizeof( totalBuffer ) );
                	memset( txBuf, 0, sizeof( txBuf ) );
            	}
        	}

        	tailMessage( mmcHdlr, totalBuffer, totalCnt );

        	//message create
        	strcat(txBuf, titleMag );       //title
        	strcat(txBuf, bodyMag);          // body
        	strcat(txBuf, totalBuffer);    //------------
    	}else{
        	sprintf( txBuf, "\n    RESULT = FAIL   \n    REASON = SEARCH KEY NOT FOUND\n" );
    	}
		if(srchCnt == 0){
			sprintf( txBuf, "\n    RESULT = FAIL   \n    REASON = SEARCH KEY NOT FOUND\n" );
		}
	}
	else if( (strlen(columns[0]) <= 0) && (strlen(columns[1]) > 0) && (strlen(columns[2]) <= 0) ){
    	snode = search_node(columns[1], 3, node);
    	if(snode){
        	headMessage( mmcHdlr, titleMag );
        	sprintf( txBuf, "\n    RESULT = SUCCESS   " );
			bodyLineSvcMessage( mmcHdlr, snode, tmpMag );
			strcat(bodyMag, tmpMag );	
			srchCnt++;	
			totalCnt++;
        	while((snode = search_node(columns[1], 3, NULL)) != NULL){
            	bodyLineSvcMessage( mmcHdlr, snode, tmpMag );
                strcat(bodyMag, tmpMag );
                totalCnt++;
				srchCnt++;
                len = strlen(bodyMag);
                if(  len >= 3072 ){
                    conTailMessage( mmcHdlr, totalBuffer);
                    //message create
                    strcat(txBuf, titleMag );       //title
                    strcat(txBuf, bodyMag);          // body
                    strcat(txBuf, totalBuffer);    //------------
                    //fprintf( stderr, "titleMag : %s", titleMag);

                    MMCResSnd(rxIxpcMsg, txBuf, 0, 1);
                    memset( bodyMag, 0, sizeof( bodyMag ) );
                    memset( totalBuffer, 0, sizeof( totalBuffer ) );
                    memset( txBuf, 0, sizeof( txBuf ) );
                }
        	}

        	tailMessage( mmcHdlr, totalBuffer, totalCnt );

        	//message create
        	strcat(txBuf, titleMag );       //title
        	strcat(txBuf, bodyMag);          // body
        	strcat(txBuf, totalBuffer);    //------------
    	}else{
        	sprintf( txBuf, "\n    RESULT = FAIL   \n    REASON = SEARCH KEY NOT FOUND\n" );
    	}
		if(srchCnt == 0){
			sprintf( txBuf, "\n    RESULT = FAIL   \n    REASON = SEARCH KEY NOT FOUND\n" );
		}
	}
	
	else if( (strlen(columns[0]) > 0) && (strlen(columns[1]) <= 0) && (strlen(columns[2]) <= 0) ){
    	snode = search_node(columns[0], ft->startIndex, node);
    	//fprintf(stderr, "test helca snode: %d\n", snode);	
		if(snode){
        	headMessage( mmcHdlr, titleMag );
        	sprintf( txBuf, "\n    RESULT = SUCCESS   " );
			bodyLineSvcMessage( mmcHdlr, snode, tmpMag );
			strcat(bodyMag, tmpMag );	
			srchCnt++;	
			totalCnt++;
        	while((snode = search_node(columns[0], ft->startIndex, NULL)) != NULL){
        		bodyLineSvcMessage( mmcHdlr, snode, tmpMag );
            	strcat(bodyMag, tmpMag );
            	totalCnt++;
				srchCnt++;
            	len = strlen(bodyMag);
            	if(  len >= 3072 ){
            		conTailMessage( mmcHdlr, totalBuffer);
                	//message create
                	strcat(txBuf, titleMag );       //title
                	strcat(txBuf, bodyMag);          // body
                	strcat(txBuf, totalBuffer);    //------------
                	//fprintf( stderr, "titleMag : %s", titleMag);

                	MMCResSnd(rxIxpcMsg, txBuf, 0, 1);
                	memset( bodyMag, 0, sizeof( bodyMag ) );
                	memset( totalBuffer, 0, sizeof( totalBuffer ) );
                	memset( txBuf, 0, sizeof( txBuf ) );
            	}

        	}

        	tailMessage( mmcHdlr, totalBuffer, totalCnt );

        	//message create
        	strcat(txBuf, titleMag );       //title
        	strcat(txBuf, bodyMag);          // body
        	strcat(txBuf, totalBuffer);    //------------
    	}else{
        	sprintf( txBuf, "\n    RESULT = FAIL   \n    REASON = SEARCH KEY NOT FOUND\n" );
    	}
		if(srchCnt == 0){
			sprintf( txBuf, "\n    RESULT = FAIL   \n    REASON = SEARCH KEY NOT FOUND\n" );
		}
	}
	
	else if( (strlen(columns[0]) > 0) && (strlen(columns[1]) > 0) && (strlen(columns[2]) <= 0) ){
    	snode = search_node(columns[0], ft->startIndex, node);
    	if(snode){
        	headMessage( mmcHdlr, titleMag );
        	sprintf( txBuf, "\n    RESULT = SUCCESS   " );

        	while((snode = search_node(columns[0], ft->startIndex, node)) != NULL){
            	if((anode = search_node2(columns[1], 3, snode)) != NULL) {
                    node = snode;
                    bodyLineSvcMessage( mmcHdlr, anode, tmpMag );
                    strcat(bodyMag, tmpMag );
                    totalCnt++;
					srchCnt++;
                    len = strlen(bodyMag);
                    if(  len >= 3072 ){
                        conTailMessage( mmcHdlr, totalBuffer);
                        //message create
                        strcat(txBuf, titleMag );       //title
                        strcat(txBuf, bodyMag);          // body
                        strcat(txBuf, totalBuffer);    //------------
                        //fprintf( stderr, "titleMag : %s", titleMag);

                        MMCResSnd(rxIxpcMsg, txBuf, 0, 1);
                        memset( bodyMag, 0, sizeof( bodyMag ) );
                        memset( totalBuffer, 0, sizeof( totalBuffer ) );
                        memset( txBuf, 0, sizeof( txBuf ) );
                    }
                    //continue;

            	}else
               		node = snode;
        	}

        	tailMessage( mmcHdlr, totalBuffer, totalCnt );

        	//message create
        	strcat(txBuf, titleMag );       //title
        	strcat(txBuf, bodyMag);          // body
        	strcat(txBuf, totalBuffer);    //------------
    	}else{
        	sprintf( txBuf, "\n    RESULT = FAIL   \n    REASON = SEARCH KEY NOT FOUND\n" );
    	}
		if(srchCnt == 0){
			sprintf( txBuf, "\n    RESULT = FAIL   \n    REASON = SEARCH KEY NOT FOUND\n" );		
		}
	}
	
	else if( (strlen(columns[0]) > 0) && (strlen(columns[1]) <= 0) && (strlen(columns[2]) > 0) ){
    	snode = search_node(columns[0], ft->startIndex, node);
    	if(snode){
        	headMessage( mmcHdlr, titleMag );
        	sprintf( txBuf, "\n    RESULT = SUCCESS   " );

        	while((snode = search_node(columns[0], ft->startIndex, node)) != NULL){
                if((bnode = search_node2(columns[2], 6, snode)) != NULL){
                    node = snode;
                    bodyLineSvcMessage( mmcHdlr, bnode, tmpMag );
                    strcat(bodyMag, tmpMag );
                    totalCnt++;
					srchCnt++;
                    len = strlen(bodyMag);
                    if(  len >= 3072 ){
                        conTailMessage( mmcHdlr, totalBuffer);
                        //message create
                        strcat(txBuf, titleMag );       //title
                        strcat(txBuf, bodyMag);          // body
                        strcat(txBuf, totalBuffer);    //------------
                        //fprintf( stderr, "titleMag : %s", titleMag);

                        MMCResSnd(rxIxpcMsg, txBuf, 0, 1);
                        memset( bodyMag, 0, sizeof( bodyMag ) );
                        memset( totalBuffer, 0, sizeof( totalBuffer ) );
                        memset( txBuf, 0, sizeof( txBuf ) );
                    }
                    //continue;
                }else
                   node = snode;

        	}

        	tailMessage( mmcHdlr, totalBuffer, totalCnt );

        	//message create
        	strcat(txBuf, titleMag );       //title
        	strcat(txBuf, bodyMag);          // body
        	strcat(txBuf, totalBuffer);    //------------
	    }else{
	        sprintf( txBuf, "\n    RESULT = FAIL   \n    REASON = SEARCH KEY NOT FOUND\n" );
	    }
		if(srchCnt == 0){
			sprintf( txBuf, "\n    RESULT = FAIL   \n    REASON = SEARCH KEY NOT FOUND\n" );

		}
	}
	// all list up	
	else{
        headMessage( mmcHdlr, titleMag );
        sprintf( txBuf, "\n    RESULT = SUCCESS   " );
        while( (node=next_conf_node( node )) != NULL){

            bodyLineSvcMessage( mmcHdlr, node, tmpMag );
            strcat(bodyMag, tmpMag );
            cnt++;
            totalCnt++;
            len = strlen(bodyMag);

            if( cnt>=20 || len >= 2500 ){

                conTailMessage( mmcHdlr, totalBuffer );

                //message create
                strcat(txBuf, titleMag );       //title
                strcat(txBuf, bodyMag);          // body
                strcat(txBuf, totalBuffer);    //------------
                //fprintf( stderr, "titleMag : %s", titleMag);
                MMCResSnd(rxIxpcMsg, txBuf, 0, 1);
                memset( bodyMag, 0, sizeof( bodyMag ) );
                memset( totalBuffer, 0, sizeof( totalBuffer ) );
                memset( txBuf, 0, sizeof( txBuf ) );
                sprintf( txBuf, "\n    RESULT = SUCCESS   " );
                cnt=0;
            }
        }
        tailMessage( mmcHdlr, totalBuffer, totalCnt );

        //message create
        strcat(txBuf, titleMag );       //title
        strcat(txBuf, bodyMag);          // body
        strcat(txBuf, totalBuffer);    //------------

    }


    MMCResSnd(rxIxpcMsg, txBuf, 0, 0);

	memset(columns, 0x00, sizeof(columns));
    return 1;
}

int doDisPpsProcess( void *tbl ,IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,
                     void *pft,MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail,
                     char columns[MAX_COL_COUNT][MAX_COL_SIZE], char fname[FILESIZE] )
{
  DLinkedList *node;
    pMmcMemberTable mmcHdlr;
    char    bodyMag[MMCMSGSIZE], totalBuffer[BUFSIZE], titleMag[BUFSIZE],tmpMag[BUFSIZE], txBuf[MMCMSGSIZE];
    int cnt=0, totalCnt=0;
    pMmcFuncTable ft;
    ft=(pMmcFuncTable)pft;


    mmcHdlr=(pMmcMemberTable)tbl;
    node = head;
    memset(txBuf, 0, sizeof(txBuf));
    memset(tmpMag, 0, sizeof(tmpMag));
    memset(bodyMag, 0, sizeof(bodyMag));
    memset(totalBuffer, 0, sizeof(totalBuffer));
    memset(titleMag, 0, sizeof(titleMag));

    headMessage( mmcHdlr, titleMag );
    if((node=next_conf_node( node )) != NULL){
        sprintf( txBuf, "\n    RESULT = SUCCESS   " );
        addBodyLinePpsMessage( mmcHdlr, node->item.columns, tmpMag , 0);
        strcat(bodyMag, tmpMag );
        cnt++;
        totalCnt++;
    }

    tailMessage( mmcHdlr, totalBuffer, totalCnt );

    //message create
    strcat(txBuf, titleMag );       //title
    strcat(txBuf, bodyMag);          // body
    strcat(txBuf, totalBuffer);    //------------

    MMCResSnd(rxIxpcMsg, txBuf, 0, 0);

    return 1;
}

int doDisIcmpProcess( void *tbl ,IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,
                      void *pft,MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail,
                      char columns[MAX_COL_COUNT][MAX_COL_SIZE], char fname[FILESIZE] )
{
  DLinkedList *node;
    pMmcMemberTable mmcHdlr;
    char    bodyMag[MMCMSGSIZE], totalBuffer[BUFSIZE], titleMag[BUFSIZE],tmpMag[BUFSIZE], txBuf[MMCMSGSIZE];
    int cnt=0, totalCnt=0;
    pMmcFuncTable ft;
    ft=(pMmcFuncTable)pft;


    mmcHdlr=(pMmcMemberTable)tbl;
    node = head;
    memset(txBuf, 0, sizeof(txBuf));
    memset(tmpMag, 0, sizeof(tmpMag));
    memset(bodyMag, 0, sizeof(bodyMag));
    memset(totalBuffer, 0, sizeof(totalBuffer));
    memset(titleMag, 0, sizeof(titleMag));

    headMessage( mmcHdlr, titleMag );
    if((node=next_conf_node( node )) != NULL){
        sprintf( txBuf, "\n    RESULT = SUCCESS   " );
        addBodyLineIcmpMessage( mmcHdlr, node->item.columns, tmpMag , 0);
        strcat(bodyMag, tmpMag );
        cnt++;
        totalCnt++;
    }

    tailMessage( mmcHdlr, totalBuffer, totalCnt );

    //message create
    strcat(txBuf, titleMag );       //title
    strcat(txBuf, bodyMag);          // body
    strcat(txBuf, totalBuffer);    //------------

    MMCResSnd(rxIxpcMsg, txBuf, 0, 0);

    return 1;
}

int doDisBcastProcess( void *tbl ,IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,
		              void *pft,MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail,
					  char columns[MAX_COL_COUNT][MAX_COL_SIZE], char fname[FILESIZE] )
{
	DLinkedList *node;
    pMmcMemberTable mmcHdlr;
    char    bodyMag[MMCMSGSIZE], totalBuffer[BUFSIZE], titleMag[BUFSIZE],tmpMag[BUFSIZE], txBuf[MMCMSGSIZE];
    int cnt=0, totalCnt=0;
    pMmcFuncTable ft;
    ft=(pMmcFuncTable)pft;


    mmcHdlr=(pMmcMemberTable)tbl;
    node = head;
    memset(txBuf, 0, sizeof(txBuf));
    memset(tmpMag, 0, sizeof(tmpMag));
    memset(bodyMag, 0, sizeof(bodyMag));
    memset(totalBuffer, 0, sizeof(totalBuffer));
    memset(titleMag, 0, sizeof(titleMag));

    headMessage( mmcHdlr, titleMag );
    if((node=next_conf_node( node )) != NULL){
    	sprintf( txBuf, "\n    RESULT = SUCCESS   " );
	    addBodyLineIcmpMessage( mmcHdlr, node->item.columns, tmpMag , 0);
	    strcat(bodyMag, tmpMag );
	    cnt++;
	    totalCnt++;
	}

    tailMessage( mmcHdlr, totalBuffer, totalCnt );

    //message create
    strcat(txBuf, titleMag );       //title
    strcat(txBuf, bodyMag);          // body
    strcat(txBuf, totalBuffer);    //------------

    MMCResSnd(rxIxpcMsg, txBuf, 0, 0);

    return 1;
} /* End of doDisBcastProcess */


