/**A.1*	FILE INCLUSION ********************************************************/
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stropts.h>
#include <fcntl.h>
#include <sys/errno.h>

#include <eth_capd.h>
#include <ipaf_define.h>
#include <ipaf_svc.h>
#include <ipaf_sem.h>
#include <ipaf_shm.h>
#include <ipam_ipaf.h>
#include <init_shm.h>
#include <shmutil.h>
#include <Num_Proto.h>
#include <Analyze_Ext_Abs.h>
#include <Ethernet_header.h>
#include <define.h>
#include <utillib.h>
#include <Errcode.h>
#include <packet_def.h>
#include <hash_pdsn.h>
#include <ippool_bitarray.h>

#include "mems.h"
#include "nifo.h"
#include "comm_util.h"
#include "func_time_check.h"
#include "pana_stat.h"
#include "pana.h"
#include "conflib.h"
#include "pana_bfr.h"


/**B.1*	DEFINITION OF NEW CONSTANTS *******************************************/
#define	UDP_HEADER_LEN	8

/**B.2*	DEFINITION OF NEW TYPE ************************************************/
#define TOUSHORT(x) (USHORT)(*(x)<<8|*(x+1))
#define TOULONG(x)  (ULONG)(*(x)<<24|*(x+1)<<16|*(x+2)<<8|*(x+3))

/**C.1*	DECLARATION OF VARIABLES **********************************************/

stMEMSINFO			*pstMEMSINFO;
T_CAPHDR			*pstCAPHead;
st_IPTCPHeader 		*pstIPTCPHeader;
UCHAR 				*pNodeData;

PDSN_LIST        	*gpPdsnList[DEF_SET_CNT];
PDSN_LIST        	*gpCurPdsn; // CURRENT OPERATION pointer
stHASHOINFO         *gpPdsnHash[DEF_SET_CNT];
stHASHOINFO         *gpCurPdsnHash; // CURRENT OPERATION pointer

st_NOTI				gstIdx;
st_NOTI				*gpIdx = &gstIdx;

/* Function Time Check */
st_FuncTimeCheckList    stFuncTimeCheckList;
st_FuncTimeCheckList    *pFUNC = &stFuncTimeCheckList;

UINT				Diff_Node_Cnt = 0;
UINT				dPreCollection_Cnt= 0;
int     			JiSTOPFlag;
int     			FinishFlag;
char				szLogBuf[1024];

/* FOR ANALYZING */
int					g_ThruIndex = 0;
int					g_StatIndex = 0;
int					g_CheckIndex = 0;
int					g_timeindex = 0;

/* MSG QID */
int					dRADIUSQid;
int					dANAQid;
int					dREANAQid;

char				vERSION[7] = "R2.0.0";	// R1.0.0 -> R2.0.0


/**D.1*  DEFINITION OF FUNCTIONS  *********************************************/
//extern int 		set_version(int prc_idx, char *ver);	// 040127,poopee
extern int	Init_msgq( key_t q_key );
extern int 	dSend_PANA_Data(stMEMSINFO *pstMEMSINFO, S32 dSndMsgQ, U8 *pNode, U32 sec);
extern int	check_my_run_status (char *procname);
extern int 	dReadFLTIDXFile(void);
extern int 	InitSHM_PDSN_HASH(void);
extern int 	InitSHM_PDSN_LIST(void);
extern void	dSetCurPdsn(NOTIFY_SIG *pNOTISIG);

int 		dAppLog(int dIndex, char *fmt, ...);
void 		FinishProgram();
void 		SetUpSignal();
//int 		dLoad_PDSN_IP(void);
//void 		dLog_PDSN_IP(int level);
int 		FilterPDSNIP(st_IPHeader *pIPHdr);
int 		FilterRADIUSPort(st_TCPHeader * pTCPHdr);

UINT 		CVT_UINT( UINT value );
INT64 		CVT_INT64( INT64 value );
INT      	CVT_INT( INT value );
int			keepalivelib_init(char *processName);
void		keepalivelib_increase();
char 		*CVT_ipaddr(UINT uiIP);


/*******************************************************************************

*******************************************************************************/
static void sig_user( int signo )
{
	int i=0, j;

	dAppLog( LOG_CRI, "SIGNAL [EXIT:%d]", signo );

	while( i < pstCAPHead->datalen )
	{
		memset( szLogBuf, 0, 1024 );
		for( j=0; (j < 16 && i < pstCAPHead->datalen); j++, i++ )
			sprintf(szLogBuf+(j*3), "%02x ", *(pNodeData+1) );

		log_write( szLogBuf );
	} 	

	signal(SIGSEGV, NULL);
	raise(SIGSEGV);

	exit(1);
}


/*******************************************************************************

*******************************************************************************/
int dINIT_PANA_IPC()
{
	int 	dRet;
	int  	key;
    char 	tmp[64], fname[256];

	sprintf(fname,"%s", DEF_SYSCONF_FILE );

	/* INIT SHM : GENINFO */
    if (conflib_getNthTokenInFileSection (fname, "[SHARED_MEMORY_KEY]", "S_SSHM_GENINFO", 1, tmp) < 0) {
        sprintf( szLogBuf, "CAN'T GET SHM KEY OF S_SSHM_GENINFO err=%s", strerror(errno));
        dAppWrite( LOG_CRI, szLogBuf );
        return -1;
    }
    else
        key = strtol(tmp, 0, 0);
 
    dRet = Init_GEN_INFO( key );
    if( dRet < 0 ) {
        sprintf( szLogBuf, "ERROR IN Init_MMDBSESS [RET:%d]", dRet );
        dAppWrite( LOG_CRI, szLogBuf );
        return -2;
    }

	if( InitSHM_PDSN_LIST() < 0 ) {
		dAppLog( LOG_CRI, "ERROR InitSHM_PDSN_LIST() FAIL");
		return -3;
	}

	if( InitSHM_PDSN_HASH() < 0 ) {
		dAppLog( LOG_CRI, "ERROR InitSHM_PDSN_HASH() FAIL");
		return -4;
	}

    if (conflib_getNthTokenInFileSection (fname, "[APPLICATIONS]", "PANA", 1, tmp) < 0) {
        dAppLog (LOG_CRI, "[cdr_init] can't get queue key of PANA err=%s\n", strerror(errno));
        return -5;
    }
    else
        key = strtol(tmp, 0, 0);
 
    dANAQid = Init_msgq( key );
    if( dANAQid < 0 ) {
        dAppLog(LOG_CRI, "ERROR IN INITIAL MSGQ PANA [RET:%d]", dANAQid);
		return -6;
    }

    if (conflib_getNthTokenInFileSection (fname, "[APPLICATIONS]", "RANA", 1, tmp) < 0) {
        dAppLog (LOG_CRI, "[cdr_init] can't get queue key of RANA err=%s\n", strerror(errno));
        return -5;
    }
    else
        key = strtol(tmp, 0, 0);
 
    dRADIUSQid = Init_msgq( key );
    if( dRADIUSQid < 0 ) {
        dAppLog(LOG_CRI, "ERROR IN INITIAL MSGQ RANA [RET:%d]", dRADIUSQid);
		return -6;
    }

#ifdef _REANA_
	/* REANA */
    if (conflib_getNthTokenInFileSection (fname, "[APPLICATIONS]", "REANA", 1, tmp) < 0) {
        dAppLog (LOG_CRI, "[cdr_init] can't get queue key of REANA err=%s\n", strerror(errno));
        return -7;
    }
    else
        key = strtol(tmp, 0, 0);
 
    dREANAQid = Init_msgq( key );
    if( dREANAQid < 0 ) {
        dAppLog(LOG_CRI, "ERROR IN INITIAL MSGQ REANA [RET:%d]", dREANAQid);
		return -8;
    }
#endif

	if( (pstMEMSINFO = nifo_init_zone("PANA", SEQ_PROC_ANA, DEF_NIFO_ZONE_CONF_FILE)) == NULL )
	{
		dAppLog( LOG_CRI, "ERROR nifo_init_zone NULL");
		return -10;
	}

#if 0
	if( (dRet = dLoad_PDSN_IP()) < 0 )
	{
		dAppLog(LOG_CRI, "dLoad_PDSNIP FAIL");
		return -11;
	}
#endif

	return 1;
}

/*******************************************************************************
*******************************************************************************/
int main (void)
{
#ifdef BUFFERING
	time_t 			oldTime = 0;
#endif /* BUFFERING */
    int 			ret, dUDPLen = 0;
	time_t 			now;
	OFFSET			offset, sub_offset;
	UCHAR			*pNode, *pCurrNode, *pNextNode;
	NOTIFY_SIG		*pNOTISIG;

    /* INITIALZE LOG */
    Init_logdebug (getpid(), "PANA", "/DSC/APPLOG");
    Init_logbeacon("/DSC/CDR/LOG");

    /* SIGNAL */
    signal(SIGSEGV, sig_user);

    SetUpSignal();

    /* INIT SHM */
    ret = dINIT_PANA_IPC();
    if (ret < 0) {
		dAppLog(LOG_DEBUG, "Failed dINIT_IPC ret[%d]", ret);
        exit(1);
    }

	if (check_my_run_status("PANA") < 0) {
		dAppLog(LOG_DEBUG, "Failed check_my_run_status");
		exit(0);
	}

    /* INIT PROC VERSION */
	if((ret=set_version(SEQ_PROC_ANA,vERSION)) < 0) {
        dAppLog(LOG_CRI,"SET_VERSION ERROR(RET=%d,IDX=%d,VER=%s)", ret,SEQ_PROC_ANA, vERSION);
    }

	/* INIT PROC KEEPALIVE */
    if (keepalivelib_init("PANA") < 0) {
        dAppLog( LOG_CRI, "FAIL IN keepalivelib_init!!" );
        exit(1);
    }

	ret = dReadFLTIDXFile();
	if (ret < 0) {
		dAppLog(LOG_CRI, "dReadFLTIDXFile() FAIL RET:%d", ret);
	}

	dAppLog(LOG_CRI, "PANA %s %d] [PROCESS INIT SUCCESS", vERSION, getpid());
    dAppLog(LOG_CRI, "PANA %s %d] [PROCESS STARTED", vERSION, getpid());

    while (JiSTOPFlag)
    {
        keepalivelib_increase();

		now = time(0);
#ifdef BUFFERING
		if (oldTime + COLLECTION_TIME < now) {
			//if (Send_Node_Cnt && Send_Node_Cnt == Diff_Node_Cnt) {
			if (Send_Node_Cnt && Send_Node_Cnt <= Collection_Cnt) {
				// Send Buffring Packet 
				//dPreCollection_Cnt = Collection_Cnt;
				if((ret = dSend_PANA_Data(pstMEMSINFO, dRADIUSQid, NULL, 0)) < 0) {
					dAppLog(LOG_CRI, "[%s.%d] dSend_PANA_Data [%d][%s]", __FUNCTION__, __LINE__, ret, strerror(-ret));
				}
				//Collection_Cnt = dPreCollection_Cnt/2;    // 이전 Collection_Cnt 의 절반으로 줄인다.
				Collection_Cnt = COLLECTION_MIN;    // 이전 Collection_Cnt 의 절반으로 줄인다.
			}
			//Diff_Node_Cnt = Send_Node_Cnt;
			oldTime = now;
		}
#endif /* BUFFERING */
		g_timeindex = (now/300) % 12;
        //Check_StatTimer(now);

#ifdef FUNC_TIME_CHECK
		time_t 	chkTime;
		chkTime = time(NULL);

		if(((chkTime/60) % 60) != g_CheckIndex) {
			g_CheckIndex = (now/60) % 60;
			PRINT_FUNC_TIME_CHECK(pFUNC);
		}
#endif
		/* READ MSGQ */
		if((offset = nifo_msg_read(pstMEMSINFO, dANAQid, NULL)) <= 0) {
			usleep(1);
			continue;
		}		

		pNextNode = nifo_ptr(pstMEMSINFO, offset);
		pCurrNode = NULL;

		//dAppLog(LOG_CRI, "DEBUG COUNT [%lld]", ++debug_cnt);
		while( pCurrNode != pNextNode )
		{
			pNode = pNextNode;
			pCurrNode = pNextNode;
			sub_offset = nifo_offset(pstMEMSINFO, pNode);

			pNextNode  = (UCHAR *)nifo_entry(nifo_ptr(pstMEMSINFO, ((NIFO *)pNextNode)->nont.offset_next), NIFO, nont);
			pstCAPHead = (T_CAPHDR *)nifo_get_value(pstMEMSINFO, CAP_HEADER_NUM, sub_offset);
			pNodeData  = nifo_get_value(pstMEMSINFO, ETH_DATA_NUM, sub_offset);
			if (pstCAPHead == NULL || pNodeData == NULL) {
				pNOTISIG = (NOTIFY_SIG *)nifo_get_value(pstMEMSINFO, NOTIFY_SIG_DEF_NUM, sub_offset);
				if (pNOTISIG != NULL) {
					switch (pNOTISIG->dFltType) 
					{
						case NOTI_PDSN_TYPE :
						case NOTI_ALL : // 초기화 시에만 쓰임. 
							dSetCurPdsn(pNOTISIG);
							break;
						default :
							dAppLog( LOG_CRI, "INVALID NOTI SIG TYPE:%u", pNOTISIG->dFltType);
							break;
					}
				}
				nifo_node_unlink_nont(pstMEMSINFO, pNode);
				nifo_node_delete(pstMEMSINFO, pNode);
				continue;
			}

			if( (pstIPTCPHeader = (pst_IPTCPHeader)nifo_tlv_alloc(pstMEMSINFO, pNode, INFO_ANA_NUM, DEF_PACKHDR_SIZE, DEF_MEMSET_OFF)) == NULL ) {
				dAppLog(LOG_CRI, "[%s][%s.%d] nifo_tlv_alloc pstIPTCPHeader NULL"
						, __FILE__, __FUNCTION__, __LINE__);
				nifo_node_unlink_nont(pstMEMSINFO, pNode);
				nifo_node_delete(pstMEMSINFO, pNode);
				continue;
			}

			/* IP ANALYZE */
			ret = Analyze_IP(pNodeData, pstCAPHead->datalen, pstIPTCPHeader);
			if (ret < 0) {
				/* 다음 블럭으로 전송하지 않는 노드는 지운다. */
				nifo_node_unlink_nont(pstMEMSINFO, pNode);
				nifo_node_delete(pstMEMSINFO, pNode);
				dAppLog( LOG_CRI, "FAIL Analyze_IP dRet:%d", ret );
				continue;
			}
			else if (ret != DEF_PROTO_UDP) { 	// Not UDP
				dAppLog(LOG_CRI, "SUCCESS Analyze_IP but Not UDP Packet(1)");
				nifo_node_unlink_nont(pstMEMSINFO, pNode);
				nifo_node_delete(pstMEMSINFO, pNode);
				continue;
			}

			/* PDSN FILTERING */
			if((ret = FilterPDSNIP (&pstIPTCPHeader->stIPHeader)) < 0) {
				nifo_node_unlink_nont (pstMEMSINFO, pNode);
				nifo_node_delete (pstMEMSINFO, pNode);
				continue;
			}

			/* TCP/UDP ANALYZE */
			dUDPLen   = pstIPTCPHeader->stIPHeader.usTotLen-pstIPTCPHeader->stIPHeader.usIPHeaderLen;
			pNodeData = pNodeData + pstIPTCPHeader->stIPHeader.usIPHeaderLen;
			if((ret = Analyze_UDP(pNodeData, dUDPLen, pstIPTCPHeader)) < 0) {
				dAppLog (LOG_CRI, "SUCCESS Analyze_IP but Not UDP Packet(2)");
				nifo_node_unlink_nont (pstMEMSINFO, pNode);
				nifo_node_delete (pstMEMSINFO, pNode);
				continue;
			}

			dAppLog(LOG_INFO, "SRC=%u:%u DST=%u:%u"
					, pstIPTCPHeader->stIPHeader.dSrcIP, pstIPTCPHeader->stTCPHeader.usSrcPort
					, pstIPTCPHeader->stIPHeader.dDestIP, pstIPTCPHeader->stTCPHeader.usDestPort);

#ifdef MEM_TEST
			nifo_node_unlink_nont (pstMEMSINFO, pNode);
			nifo_node_delete (pstMEMSINFO, pNode);

			continue;
#endif
			/* PORT FILTERING - JUST RADIUS*/
			if((ret = FilterRADIUSPort (&pstIPTCPHeader->stTCPHeader)) < 0) {
				/* NODE DELETE */
				dAppLog(LOG_CRI, "FilterRADIUSPort FilterOut");
				nifo_node_unlink_nont (pstMEMSINFO, pNode);
				nifo_node_delete (pstMEMSINFO, pNode);
				continue;
			}

			nifo_node_unlink_nont (pstMEMSINFO, pNode);
			if((ret = dSend_PANA_Data (pstMEMSINFO, dRADIUSQid, pNode, pstCAPHead->curtime)) < 0)
			{
				dAppLog(LOG_CRI, "[%s.%d] dSend_PANA_Data [%d][%s]"
						, __FUNCTION__, __LINE__, ret, strerror(-ret));
				nifo_node_delete(pstMEMSINFO, pNode);
			} else {
				dAppLog(LOG_DEBUG, "#### dSend_PANA_Data SRCIP=%d DSTIP=%d TO=%d"
						, pstIPTCPHeader->stIPHeader.dSrcIP, pstIPTCPHeader->stIPHeader.dDestIP, dANAQid);
			}
		}
	}

    FinishProgram();
    return 1;
}


/*******************************************************************************

*******************************************************************************/
char *CVT_ipaddr(UINT uiIP)
{
    struct in_addr  inaddr;

    inaddr.s_addr = htonl(uiIP);

    return inet_ntoa(inaddr);
}


/*******************************************************************************

*******************************************************************************/
int LogTCPIPHeader( pst_IPTCPHeader pstHeader )
{
    int     dLog;

    dLog = LOG_INFO;

    dAppLog( dLog, "##### PACKET INFORMATION ############################################");

    dAppLog( dLog, "[IP : SRC IP    ] : [%s]", CVT_ipaddr(pstHeader->stIPHeader.dSrcIP) );
    dAppLog( dLog, "[IP : DEST IP   ] : [%s]", CVT_ipaddr(pstHeader->stIPHeader.dDestIP) );
    dAppLog( dLog, "[IP : IPHeadLen ] : [%d]", pstHeader->stIPHeader.usIPHeaderLen );
    dAppLog( dLog, "[IP : Total Len ] : [%d]", pstHeader->stIPHeader.usTotLen );
    dAppLog( dLog, "[IP : Timelive  ] : [%d]", pstHeader->stIPHeader.ucTimelive );
    dAppLog( dLog, "[IP : PROTOCAL  ] : [%d]", pstHeader->stIPHeader.ucProtocol );
    dAppLog( dLog, "[IP : IDENTI    ] : [%d]", pstHeader->stIPHeader.usIdentification );

    dAppLog( dLog, "[TCP : SEQ      ] : [%lu]", pstHeader->stTCPHeader.dSeq );
    dAppLog( dLog, "[TCP : ACK      ] : [%lu]", pstHeader->stTCPHeader.dAck );
    dAppLog( dLog, "[TCP : WINDOW   ] : [%d]", pstHeader->stTCPHeader.dWindow );
    dAppLog( dLog, "[TCP : SRC PORT ] : [%d]", pstHeader->stTCPHeader.usSrcPort );
    dAppLog( dLog, "[TCP : DEST PORT] : [%d]", pstHeader->stTCPHeader.usDestPort );
    dAppLog( dLog, "[TCP : RTX FLAG ] : [%d]", pstHeader->stTCPHeader.usRtxType );
    dAppLog( dLog, "[TCP : TCP HEAD ] : [%d]", pstHeader->stTCPHeader.usTCPHeaderLen );
    dAppLog( dLog, "[TCP : DATA LEN ] : [%d]", pstHeader->stTCPHeader.usDataLen );
    dAppLog( dLog, "[TCP : CONTROL  ] : [%d]", pstHeader->stTCPHeader.ucControlType );
    dAppLog( dLog, "[TCP : IP FRAG  ] : [%d]", pstHeader->stTCPHeader.ucIPFrag );

    return 1;
}

int FilterPDSNIP(st_IPHeader *pIPHdr)
{
	st_Pdsn_HashKey	stPdsnHashKey;
	st_Pdsn_HashKey	*pPdsnHashKey = &stPdsnHashKey;

	pPdsnHashKey->uiIP = pIPHdr->dSrcIP;
	if( hasho_find(gpCurPdsnHash, (U8 *)pPdsnHashKey) == NULL ) {
		pPdsnHashKey->uiIP = pIPHdr->dDestIP;
		if( hasho_find(gpCurPdsnHash, (U8 *)pPdsnHashKey) == NULL ) 
		{
			dAppLog( LOG_DEBUG, "CUR PDSN IDX=%d NOT MY PDSN LIST IP srcip=%u dstip=%u",
						gpIdx->dPdsnIdx, pIPHdr->dSrcIP, pIPHdr->dDestIP);
			return -1;
		}
	}
	return 0;
}

int FilterRADIUSPort(st_TCPHeader * pTCPHdr)
{
	if( (pTCPHdr->usSrcPort)  == (1812) || (pTCPHdr->usSrcPort)  == (1813) || 
		(pTCPHdr->usSrcPort)  == (1814) || (pTCPHdr->usSrcPort)  == (3799) || 
		(pTCPHdr->usDestPort) == (1812) || (pTCPHdr->usDestPort) == (1813) || 
		(pTCPHdr->usDestPort) == (1814) || (pTCPHdr->usDestPort) == (3799)) 
		return 0;
	else
		return -1;	
}

