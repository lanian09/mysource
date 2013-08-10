/**********************************************************
			LG BSD Project

   Author	:
   Section	: 
   SCCS ID	: %W%
   Date	: %G%
   Revision History :


   Description :

***********************************************************/

/**A.1*	File Inclusion ************************************/
#include "mmcr.h"

/**B.1*	Definition of New Constants *************************/

/**B.2*	Definition of New Type ******************************/

/**C.1*	Declaration of Variables ****************************/
int		semid_destip=-1;		/* semaphore id for MMDB_DESTIP */
int		semid_destport =-1;

// for log

/**D.1*  Definition of Functions  *************************/
// mmcr_mdbload.c
extern int load_svc_group(char columns[MAX_COL_COUNT][MAX_COL_SIZE], st_CategoryInfo *stCat );

extern  DESTPORT_TABLE  *destport_tbl;
extern  DESTIP_TABLE    *destip_tbl;
extern char              trcBuf[TRCBUF_LEN], trcTmp[TRCTMP_LEN];

// this file
int Add_SrcIP();
int Add_Pdsn();
int Add_Category();

/* for alone test */
#ifdef ALONE_TEST
#ifdef S_SEMA_DESTIP
#undef S_SEMA_DESTIP
#define S_SEMA_DESTIP 0x20001
#endif
#ifdef S_SEMA_DESTPORT
#undef S_SEMA_DESTPORT
#define S_SEMA_DESTPORT 0x20002
#endif
#endif /* ALONE_TEST */

int mdb_init(void)
{
	int ret=0;
	//time_t	curtime, dDelTime, pretime=0;

	//int				isize;

	DESTIP_DATA		DestIPData;
	DESTPORT_DATA	DestPortData;

    if(Init_MMDBDESTIP() < 0){
		fprintf(stderr, "Init_MMDBDESTIP Fail\n");
		return -1;
	}
    if(Init_MMDBDESTPORT() < 0){
		fprintf(stderr, "Init_MMDBDESTPORT Fail\n");
		return -1;
	}

    if(Init_MMDBSVCOPT() < 0){
		fprintf(stderr, "Init_MMDBSVCOPT Fail\n");
		return -1;
	}

	// semapore initialize
	semid_destip = Init_sem( S_SEMA_DESTIP );
	semid_destport = Init_sem( S_SEMA_DESTPORT );

	if( semid_destip < 0
	 	|| semid_destport < 0 )
	{
        fprintf(stderr, "Error Init_sem  [DESTIP:%d] [DESTPORT:%d]",
                semid_destip, semid_destport);
		return -1;
	}

    // insert first key into MMDB
	memset( &DestIPData, 0x00, sizeof(DESTIP_DATA) );
DEBUG_PRINT("\n\n============================== Insert_DESTIP - IP POOL ================================\n\n");
DEBUG_IP;
	ret = Insert_DESTIP( &DestIPData  );
DEBUG_IP;
	if( ret < 0 )
	{
        sprintf(trcBuf, "Error in Insert_DESTIP insert first KEY [RET:%d]", ret);
		trclib_writeLogErr(FL, trcBuf);
	    return -1;
	}

	// insert last key into MMDB
	memset( &DestIPData, 0xff, sizeof(DESTIP_DATA) );
DEBUG_PRINT("\n\n============================== Insert_DESTIP - PDSN ================================\n\n");
DEBUG_IP;
	ret = Insert_DESTIP( &DestIPData );
DEBUG_IP;
	if( ret < 0 )
	{
		sprintf(trcBuf, "Error in Insert_DESTIP insert last KEY [RET:%d]", ret);
		trclib_writeLogErr(FL, trcBuf);
	    return -1;
	}

    // insert first key into MMDB
	memset( &DestPortData, 0x00, sizeof(DESTPORT_DATA) );
DEBUG_PRINT("\n\n============================== Insert_DESTPORT - SVC ================================\n\n");
DEBUG_PORT;
	ret = Insert_DESTPORT( &DestPortData  );
DEBUG_PORT;
	if( ret < 0 )
	{
		sprintf(trcBuf, "Error in Insert_DESTPORT insert first KEY [RET:%d]", ret);
		trclib_writeLogErr(FL, trcBuf);
	    return -1;
	}

	// insert last key into MMDB
	memset( &DestPortData, 0xff, sizeof(DESTPORT_DATA) );
DEBUG_PORT;
	ret = Insert_DESTPORT( &DestPortData );
DEBUG_PORT;
	if( ret < 0 )
	{
		sprintf(trcBuf, "Error in Insert_DESTPORT insert last KEY [RET:%d]", ret);
		trclib_writeLogErr(FL, trcBuf);
		return -1;
	}


	// add Source IP block when process started
DEBUG_IP;
	ret = Add_SrcIP();
DEBUG_IP;
	if( ret < 0 )
	{
		sprintf(trcBuf, "Error in Add_SrcIP [RET:%d]", ret);
		trclib_writeLogErr(FL, trcBuf);
		return -1;
	}
	
	// add PDSN IP block when process started
	ret = Add_Pdsn();
	if( ret < 0 )
	{
		sprintf(trcBuf, "Error in Add_Pdsn [RET:%d]", ret);
		trclib_writeLogErr(FL, trcBuf);
		return -1;
	}

	// add Category when process started

	ret = Add_Category();
	if( ret < 0 )
	{
		sprintf(trcBuf, "Error in Add_Category [RET:%d]", ret);
		trclib_writeLogErr(FL, trcBuf);
		return -1;
	}

//	ret = Add_SvcOpt();
//	if( ret < 0 )
//	{
//		sprintf(trcBuf, "Error in Add_SvcOpt [RET:%d]", ret);
//		trclib_writeLogErr(FL, trcBuf);
//		return -1;
//	}

	return 1;
}

int Add_SrcIP()
{
	int ret, ret_val=0;
	int found;

	char        fname[FILESIZE], *env;
	DLinkedList head, tail, *node;

	st_IPPool	stIPPool;
	DESTIP_KEY	DestIPKey1, DestIPKey2;
	DESTIP_DATA	DestIP, *pDestIP;
	unsigned int	uiIP, uiNetMask;

	if ((env = getenv(IV_HOME)) == NULL){
		sprintf( trcBuf, "getenv error! \n" );
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	sprintf (fname, "%s/%s", env, IP_POOL_FILE);

	set_head_tail(&head, &tail);
	if(make_conf_list(fname, &head) < 0){
		sprintf (trcBuf, "make linked list fail; err=%d(%s)\n", errno, strerror(errno));
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}
	    // add & update
	node = &head;
	DEBUG_PRINT("First While -> start");
	while( node->next ) {
		node = node->next;
		if(node->item.gubun != TOK_CONF_LINE) continue;

DEBUG_PRINT_A(node->item.line);
		memset( &stIPPool, 0x00, sizeof(st_IPPool) );
#if 1
		strcpy(stIPPool.szSysIP, node->item.columns[1]);
		stIPPool.iPrefix = atoi( node->item.columns[2]);
#endif
#if 0
		strcpy(stIPPool.szSysIP, node->item.columns[2]);
		stIPPool.iPrefix = atoi( node->item.columns[3]);

		for (i=0; i<strlen(node->item.columns[1]); i++) node->item.columns[1][i] = toupper(node->item.columns[1][i]);
		strcpy(stIPPool.pdsn_type, node->item.columns[1]); // add by helca 080721
		
		if( !strcasecmp(stIPPool.pdsn_type, "1X")) {
	    	PdsnType = 1;
	    }else if ( !strcasecmp(stIPPool.pdsn_type, "1XEV_DO")) {
	        PdsnType = 2;
	    }else {
	        ;
	    }
#endif

		uiIP = inet_addr(stIPPool.szSysIP);
		
		if( uiIP == (in_addr_t)-1 || stIPPool.iPrefix < 0 || stIPPool.iPrefix > 32)
		{
			ret_val = -1;
			break;
		}

		// inet_addr converts alreay
		uiNetMask = 0xffffffff << (32 - stIPPool.iPrefix);

		// IP POOL 
		memset( &DestIP, 0x00, sizeof(DESTIP_DATA) );

		DestIP.key.ucFlag = 'S';
		uiIP = htonl(uiIP) | ~uiNetMask;
		DestIP.key.uiIP = ntohl(uiIP);
		DestIP.uiNetmask = uiNetMask;
//		DestIP.uiNetmask = htonl(uiNetMask);
		DestIP.ucIPType = IPTYPE_IPPOOL;
		//DestIP.ucPdsnType = PdsnType; // 1:1x, 2:1xEV-DO  add by helca 2008.07.21

		DEBUG_MEM( DestIP.key, (sizeof(DestIP.key)));
		pDestIP = Search_DESTIP( &DestIP.key );

		if( pDestIP == 0 )	/* not exist */
		{
DEBUG_PRINT("Insert_DESTIP");
DEBUG_IP;
			ret = Insert_DESTIP( &DestIP );
		}
		else
		{
DEBUG_PRINT("Update_DESTIP");
DEBUG_IP;
			ret = Update_DESTIP( pDestIP, &DestIP );
		}

		if( ret < 0 )
		{
			ret_val = -1;
			break;
		}
	}
DEBUG_PRINT("First While -> end");

	if( ret_val == 0 ){
		// delete
		memset( &DestIPKey1, 0x00, sizeof(DESTIP_KEY) );
		DestIPKey1.ucFlag = 'S';

		memset( &DestIPKey2, 0xff, sizeof(DESTIP_KEY) );
		DestIPKey2.ucFlag = 'S';

DEBUG_PRINT("Second While -> start");
		while( 1 )
		{

			pDestIP = Select_DESTIP( &DestIPKey1, &DestIPKey2 );
			if( pDestIP == 0 || pDestIP->key.ucFlag != 'S')
				break;

			memcpy( &DestIPKey1, &pDestIP->key, sizeof(DESTIP_KEY) );

			if(pDestIP->ucIPType != IPTYPE_IPPOOL) continue;


DEBUG_MEM(DestIPKey1, (sizeof(DestIPKey1)));
			// move data to temp buffer
			memcpy( &DestIP, pDestIP, sizeof(DESTIP_DATA) );
			DestIP.key.uiIP = htonl(DestIP.key.uiIP);
//			DestIP.uiNetmask = htonl(DestIP.IP);
			found = 0;
			node = &head;
DEBUG_PRINT("while in Second While -> start");
			while( node->next )
			{
				node = node->next;
				if(node->item.gubun != TOK_CONF_LINE) continue;

DEBUG_PRINT_A(node->item.line);
				memset( &stIPPool, 0x00, sizeof(st_IPPool) );
				//strcpy(stIPPool.szSysIP, node->item.columns[2]);
				strcpy(stIPPool.szSysIP, node->item.columns[1]);
				//stIPPool.iPrefix = atoi( node->item.columns[3]);
				stIPPool.iPrefix = atoi( node->item.columns[2]);

				uiIP = inet_addr(stIPPool.szSysIP);
				uiIP = htonl(uiIP);
				uiNetMask = 0xffffffff << (32 - stIPPool.iPrefix);
				
DEBUG_MEM(DestIP.key.uiIP, (sizeof(DestIP.key.uiIP)));
DEBUG_MEM(uiIP, (sizeof(uiIP)));
DEBUG_MEM(DestIP.uiNetmask, (sizeof(DestIP.uiNetmask)));
DEBUG_MEM(uiNetMask, (sizeof(uiNetMask)));
				if( (DestIP.key.uiIP & uiNetMask) == (uiIP & uiNetMask) 
						&& DestIP.uiNetmask == uiNetMask )
				{
					found = 1;
					break;
				}
			}
DEBUG_PRINT("while in Second While -> end");

			if( !found )
			{
				ret = Delete_DESTIP( &DestIPKey1 );
				if( ret < 0 )
					ret_val = -1;
DEBUG_PRINT("Delete_DESTIP");
DEBUG_IP;
			}
		}
DEBUG_PRINT("Second While -> end");
	}

	delete_all_list_node(&head, &tail);

	return ret_val;
}

int Add_Pdsn()
{
	int ret, ret_val=0;
	int found;

	char        fname[FILESIZE], *env;
	DLinkedList head, tail, *node;
    
	st_IPPool	stIPPool;
	DESTIP_KEY	DestIPKey1, DestIPKey2;
	DESTIP_DATA	DestIP, *pDestIP;
	unsigned int	uiIP, uiNetMask;

	if ((env = getenv(IV_HOME)) == NULL){
		sprintf( trcBuf, "getenv error! \n" );
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	sprintf (fname, "%s/%s", env, PSDN_FILE);

	set_head_tail(&head, &tail);
	if(make_conf_list(fname, &head) < 0){
		sprintf (trcBuf, "make linked list fail; err=%d(%s)\n", errno, strerror(errno));
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	    // add & update
	node = &head;
	while( node->next ) {
		node = node->next;
		if(node->item.gubun != TOK_CONF_LINE) continue;

DEBUG_PRINT_A(node->item.line);
		//memset( &stIPPool, 0x00, sizeof(st_IPPool) );
		strcpy(stIPPool.szSysIP, node->item.columns[0]);

		uiIP = inet_addr(stIPPool.szSysIP);
		if( uiIP == (in_addr_t)-1)
		{
                	ret_val = -1;
                	break;
		}

		// IP POOL 
		memset( &DestIP, 0x00, sizeof(DESTIP_DATA) );

		DestIP.key.ucFlag = 'S';
		// Not needed
		uiIP = htonl(uiIP) | ~(0xffffffff);
		DestIP.key.uiIP = ntohl(uiIP);
		DestIP.uiNetmask = 0xffffffff; // 255.255.255.255 fixed
		DestIP.ucIPType = IPTYPE_PDSN;

		pDestIP = Search_DESTIP( &DestIP.key );
DEBUG_MEM(DestIP.key, sizeof(DestIP.key));

		if( pDestIP == 0 )	//* not exist
		{
DEBUG_PRINT("Insert_DESTIP");
                	ret = Insert_DESTIP( &DestIP );
		}
		else
		{
DEBUG_PRINT("Update_DESTIP");
                	ret = Update_DESTIP( pDestIP, &DestIP );
		}

		if( ret < 0 )
		{
			ret_val = -1;
			break;
		}
	}

	if( ret_val == 0 ){
		// delete
		memset( &DestIPKey1, 0x00, sizeof(DESTIP_KEY) );
		DestIPKey1.ucFlag = 'S';

		memset( &DestIPKey2, 0xff, sizeof(DESTIP_KEY) );
		DestIPKey2.ucFlag = 'S';
DEBUG_PRINT("PDSN START=======================================");

		while( 1 )
		{
			pDestIP = Select_DESTIP( &DestIPKey1, &DestIPKey2 );
			if( pDestIP == 0 || pDestIP->key.ucFlag != 'S')
				break;
			memcpy( &DestIPKey1, &pDestIP->key, sizeof(DESTIP_KEY) );

DEBUG_MEM(DestIPKey1, sizeof(DestIPKey1));
			if(pDestIP->ucIPType != IPTYPE_PDSN) continue;

			// move data to temp buffer
			memcpy( &DestIP, pDestIP, sizeof(DESTIP_DATA) );
			DestIP.key.uiIP = htonl(DestIP.key.uiIP);
			found = 0;
			node = &head;
			while( node->next )
			{
				node = node->next;
				if(node->item.gubun != TOK_CONF_LINE) continue;
DEBUG_PRINT(node->item.line);

				memset( &stIPPool, 0x00, sizeof(st_IPPool) );
				strcpy(stIPPool.szSysIP, node->item.columns[0]);

				uiIP = inet_addr(stIPPool.szSysIP);
				uiIP = ntohl(uiIP);
				uiNetMask = 0xffffffff;
DEBUG_MEM(DestIP.key.uiIP, (sizeof(DestIP.key.uiIP)));
DEBUG_MEM(uiIP, (sizeof(uiIP)));
DEBUG_MEM(DestIP.uiNetmask, (sizeof(DestIP.uiNetmask)));
DEBUG_MEM(uiNetMask, (sizeof(uiNetMask)));
				if( (DestIP.key.uiIP & uiNetMask) == (uiIP & uiNetMask) 
						&& DestIP.uiNetmask == uiNetMask )
				{
					found = 1;
					break;
				}
			}

			if( !found )
			{
DEBUG_PRINT("Delete_DESTIP");
				ret = Delete_DESTIP( &DestIPKey1 );
				if( ret < 0 )
					ret_val = -1;
			}
		}
	}

	delete_all_list_node(&head, &tail);

	return ret_val;
}


int Add_Category()
{
	int ret, ret_val=0, svcGroup;

	char        fname[FILESIZE], *env;
	DLinkedList head, tail, *node;

	st_CategoryInfo		stCat;
	DESTIP_DATA	DestIP, *pDestIP;
	DESTIP_KEY	DestIPKey1, DestIPKey2;

	DESTPORT_DATA	DestPort, *pDestPort;
	DESTPORT_KEY	DestPortKey1, DestPortKey2;

	int	found, iPrefix;
	unsigned int	uiIP, uiNetMask;

	if ((env = getenv(IV_HOME)) == NULL){
		sprintf( trcBuf, "getenv error! \n" );
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	sprintf (fname, "%s/%s", env, SERVICE_TYPE_FILE);

	set_head_tail(&head, &tail);
	if(make_conf_list(fname, &head) < 0){
		sprintf (trcBuf, "make linked list fail; err=%d(%s)\n", errno, strerror(errno));
		trclib_writeLogErr (FL,trcBuf);
	 	return -1;
	 }


	// add & update
	node = &head;
	while( node->next ) {
		node = node->next;
		if(node->item.gubun != TOK_CONF_LINE) continue;

DEBUG_PRINT_A(node->item.line);
		memset( &stCat, 0x00, sizeof(st_CategoryInfo) );

		load_svc_group(node->item.columns, &stCat);

		svcGroup = stCat.ucGroup >> 4;
DEBUG_PRINT_I(svcGroup);
		switch( svcGroup ){
			case 1:		/* DESTIP */

				memset( &DestIP, 0x00, sizeof(DESTIP_DATA) );

				DestIP.key.ucFlag = 'D';
				uiIP = inet_addr( stCat.szCon[0] );
				iPrefix = atoi( stCat.szCon[1] );
				if( iPrefix < 0 || iPrefix > 32)
				{
					ret_val = -1;
					sprintf(trcBuf, "DESTIP iPrefix[%d] GroupID:%d", 
					iPrefix, svcGroup);
					trclib_writeLogErr (FL,trcBuf);
					break;
				}
				if( strcmp(stCat.szCon[0], "255.255.255.255") !=0 && 
				uiIP == (in_addr_t)-1 )
				{
					ret_val = -1;
					sprintf(trcBuf, "DESTIP NET[%s:%d] GroupID:%d", 
						stCat.szCon[0], uiIP, svcGroup);
					trclib_writeLogErr (FL,trcBuf);
					break;
				}
				uiNetMask = 0xffffffff << (32 - iPrefix);

				/* input error handling */
				uiIP = htonl(uiIP) | ~uiNetMask;
				DestIP.key.uiIP = ntohl(uiIP);

				DestIP.uiNetmask = uiNetMask;
				DestIP.usCatID = stCat.usCategory;
				DestIP.ucGroupID = stCat.ucGroup;
				/* Sequence */
				DestIP.ucSerial = stCat.szServiceID[0];
				DestIP.ucFilterOut = stCat.ucFilterOut;

				//Obsolete
				//DestIP.ucLayer = stCat.ucLayer;
				// 21 - including retransmission
				DestIP.ucLayer = 21;

				DestIP.ucURLChar = stCat.ucMode;
				DestIP.ucSvcBlk = stCat.ucSvcBlk;
				DestIP.ucCDRIPFlag = stCat.szReserved[0];
				DestIP.ucUDRFlag = stCat.szReserved[1]; // add by helca

DEBUG_MEM(DestIP.key, (sizeof(DestIP.key)));

				pDestIP = Search_DESTIP( &DestIP.key );
				if( pDestIP == 0 ){	/* not exist */
					ret = Insert_DESTIP( &DestIP );
DEBUG_PRINT("SVC type 1 Insert_DESTIP");
				}else{
					ret = Update_DESTIP( pDestIP, &DestIP );
DEBUG_PRINT("SVC type 1 Update_DESTIP");
                }
				if( ret < 0 )
				{
					ret_val = -1;
					sprintf(trcBuf, "pDestIP[%p] GroupID:%d ret[%d]", 
						pDestIP, svcGroup, ret);
					trclib_writeLogErr (FL,trcBuf);
					break;
				}
				break;

			case 2:		/* DESTIP+TCP PORT */
			case 3:		/* DESTIP+UDP PORT */
				uiIP = inet_addr( stCat.szCon[0] );
				iPrefix = atoi( stCat.szCon[1] );
				if( iPrefix < 0 || iPrefix > 32)
				{
					ret_val = -1;
					sprintf(trcBuf, "DESTPORT iPrefix[%d] GroupID:%d", 
					iPrefix, svcGroup);
					trclib_writeLogErr (FL,trcBuf);
					break;
				}

				if( strcmp(stCat.szCon[0], "255.255.255.255") !=0 && 
					uiIP == (in_addr_t)-1 )
				{
					ret_val = -1;
					sprintf(trcBuf, "DESTPORT NET[%s:%d] GroupID:%d", 
					stCat.szCon[0], uiIP, svcGroup);
					trclib_writeLogErr (FL,trcBuf);
					break;
				}

				uiNetMask = 0xffffffff << (32 - iPrefix);

				memset( &DestPort, 0x00, sizeof(DESTPORT_DATA) );
				uiIP = htonl(uiIP) | ~uiNetMask;
				DestPort.key.uiDestIP = ntohl(uiIP);
				if( svcGroup == 2 )
					DestPort.key.ucProtocol = 6;	/* TCP */
				else
					DestPort.key.ucProtocol = 17;	/* UDP */

				DestPort.key.usDestPort = atoi( stCat.szCon[2] );

				DestPort.uiNetmask = uiNetMask;
				DestPort.usCatID = stCat.usCategory;
				DestPort.ucGroupID = stCat.ucGroup;
				DestPort.ucSerial = stCat.szServiceID[0]; // sequence
				DestPort.ucFilterOut = stCat.ucFilterOut;
				//Obsolete
				//DestPort.ucLayer = stCat.ucLayer;
				// 21 means including retransmission.
				DestPort.ucLayer = 21;

				DestPort.ucURLChar = stCat.ucMode;       /* 040429 added by js */
				DestPort.ucSvcBlk = stCat.ucSvcBlk;
				DestPort.ucCDRIPFlag = stCat.szReserved[0];
				DestPort.ucUDRFlag = stCat.szReserved[1]; // add by helca

				pDestPort = Search_DESTPORT( &DestPort.key );

DEBUG_MEM(DestPort.key, (sizeof(DestPort.key)));
				if( pDestPort == 0 ){	/* not exist */
					ret = Insert_DESTPORT( &DestPort );
DEBUG_PRINT("SVC type 2(3) Insert_DESTIP");
				}else{
					ret = Update_DESTPORT( pDestPort, &DestPort );
DEBUG_PRINT("SVC type 2(3) Update_DESTIP");
                }
				if( ret < 0 )
				{
					ret_val = -1;
					sprintf(trcBuf, "pDestPort[%p] GroupID:%d ret[%d]", 
					pDestPort, svcGroup, ret);
					trclib_writeLogErr (FL,trcBuf);
					break;
				}

				break;

			default:
				sprintf(trcBuf, "Undefined GroupID:%d", svcGroup );
				trclib_writeLogErr (FL,trcBuf);
				break;
		}
	}

	// delete
	if( ret_val == 0 )
	{
		// destip delete
		memset( &DestIPKey1, 0x00, sizeof(DESTIP_KEY) );
		DestIPKey1.ucFlag = 'D';

		memset( &DestIPKey2, 0xff, sizeof(DESTIP_KEY) );
		DestIPKey2.ucFlag = 'D';

		while( 1 )
		{
			pDestIP = Select_DESTIP( &DestIPKey1, &DestIPKey2 );
			if( pDestIP == 0 || pDestIP->key.ucFlag != 'D')
				break;

			memcpy( &DestIPKey1, &pDestIP->key, sizeof(DESTIP_KEY) );

			// convert key if needed
			memcpy( &DestIP, pDestIP, sizeof(DESTIP_DATA) );
			DestIP.key.uiIP = htonl(DestIP.key.uiIP);
			found = 0;
			node = &head;
			while( node->next )
			{
				node = node->next;
				if(node->item.gubun != TOK_CONF_LINE) continue;

				memset( &stCat, 0x00, sizeof(st_CategoryInfo) );
				load_svc_group(node->item.columns, &stCat);

				svcGroup = stCat.ucGroup >> 4;
				if( svcGroup != 1 )
					continue;

				uiIP = inet_addr( stCat.szCon[0] );
// inet_addr return Network byter ordered IP Addr
				uiIP = htonl(uiIP);
				iPrefix = atoi( stCat.szCon[1] );
				uiNetMask = 0xffffffff << (32-iPrefix);
				
DEBUG_MEM(DestIP.key.uiIP, (sizeof(DestIP.key.uiIP)));
DEBUG_MEM(uiIP, (sizeof(uiIP)));
DEBUG_MEM(DestIP.uiNetmask, (sizeof(DestIP.uiNetmask)));
DEBUG_MEM(uiNetMask, (sizeof(uiNetMask)));

				if( (DestIP.key.uiIP & DestIP.uiNetmask) == (uiIP & DestIP.uiNetmask) 
						&& DestIP.uiNetmask == uiNetMask )
				{
					found = 1;
					break;
				}
			}

			if( !found )
			{
DEBUG_PRINT("Delete_DESTIP");
				ret = Delete_DESTIP( &DestIPKey1 );
				if( ret < 0 )
					ret_val = -1;
			}
		}

		// destport delete
		memset( &DestPortKey1, 0x00, sizeof(DESTPORT_KEY) );
		memset( &DestPortKey2, 0xff, sizeof(DESTPORT_KEY) );

		while( 1 )
		{
			pDestPort = Select_DESTPORT( &DestPortKey1, &DestPortKey2 );
			if( pDestPort == 0 ) 
				break;

			memcpy( &DestPortKey1, &pDestPort->key, sizeof(DESTPORT_KEY) );

			// convert key if needed
			memcpy( &DestPort, pDestPort, sizeof(DESTPORT_DATA) );
			DestPort.key.uiDestIP = htonl(DestPort.key.uiDestIP);
			found = 0;
			node = &head;
			while( node->next )
			{
				node = node->next;
				if(node->item.gubun != TOK_CONF_LINE) continue;

				memset( &stCat, 0x00, sizeof(st_CategoryInfo) );
				load_svc_group(node->item.columns, &stCat);

				svcGroup = stCat.ucGroup >> 4;

				switch( DestPort.key.ucProtocol )
				{
					case 6 : // TCP
						if( svcGroup == 2 )  // 
							break;
						continue;
					case 17 : // UDP
						if( svcGroup ==3 )   //
							break;
						continue;
					default :
							continue;
				}
						
				uiIP = inet_addr( stCat.szCon[0] );
//  inet_addr returns network byte orderd IP Address
				uiIP = htonl(uiIP);
				iPrefix = atoi( stCat.szCon[1] );
				uiNetMask = 0xffffffff << (32-iPrefix);

DEBUG_MEM(DestPort.key.uiDestIP, (sizeof(DestPort.key.uiDestIP)));
DEBUG_MEM(uiIP, (sizeof(uiIP)));
DEBUG_MEM(DestPort.uiNetmask, (sizeof(DestPort.uiNetmask)));
DEBUG_MEM(uiNetMask, (sizeof(uiNetMask)));
					
				// 2003/09/25 edit by hwh
				if( (DestPort.key.uiDestIP & DestPort.uiNetmask) == 
					(uiIP & DestPort.uiNetmask) && 
					DestPort.uiNetmask == uiNetMask && 
					DestPort.key.usDestPort == atoi(stCat.szCon[2]) )
				{
					found = 1;
					break;
				}
			}

			if( !found )
			{
DEBUG_PRINT("Delete_DESTPORT");
				ret = Delete_DESTPORT( &DestPortKey1 );
				if( ret < 0 )
					ret_val = -1;
			}
		}
	}

	delete_all_list_node(&head, &tail);

	return ret_val;
}

