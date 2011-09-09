/*
 * re_main.c
 * Redirect Msg 처리 
 */

#include <string.h>
#include <ctype.h>
#include "re_define.h"
#include "comm_util.h"
#include "nifo.h"
#include "mems.h"


/**C.1*	DECLARATION OF VARIABLES **********************************************/

/* ADD BY YOON 2008.10.16 */
stMEMSINFO 				*pstMEMSINFO;

/* Function Time Check */
st_FuncTimeCheckList    stFuncTimeCheckList;
st_FuncTimeCheckList    *pFUNC = &stFuncTimeCheckList;

int	lasterror = 1;
int gTRCDRcnt = 0;
int JiSTOPFlag;
int FinishFlag;

/* FOR LOG */
char			szLogBuf[1024];
unsigned char	szTempIP[5];
unsigned char	szTempIP2[5];

/* FOR CDR */
int				g_Direction;	/* UP or DOWN */	

/* FOR STAT */
int				g_ThruIndex = 0;
int				g_StatIndex = 0;
int				g_timeindex = 0;
int				g_CheckIndex = 0;

/* SEMAPHORE ID FOR MMDB */
int 			semid_mif = -1;

/* 20041019 MSG QID */
int 			dREANAQid;
int				dRLEGQid;

/* 20080718 PDSN TYPE INFO */
unsigned int	uiModular;
unsigned int	uiSysModular;
unsigned int	uiRsltFlag;		// 080721, poopee

void print_timegap(struct timeval *before, struct timeval *after, char *funcname, int linenum)
{
    time_t diff = (after->tv_sec*1000000+after->tv_usec) - (before->tv_sec*1000000+before->tv_usec);
    dAppLog(LOG_CRI, "FUNC[%s %d]: BEFORE[%ld.%06ld] AFTER[%ld.%06ld] DIFF[%ld]",
            funcname, linenum, before->tv_sec, before->tv_usec, after->tv_sec, after->tv_usec, diff);
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
int dINIT_REANA_IPC()
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
	} else
		key = strtol(tmp, 0, 0);

	dRet = Init_GEN_INFO( key );
	if( dRet < 0 ) {
		sprintf( szLogBuf, "ERROR IN Init_MMDBSESS [RET:%d]", dRet );
		dAppWrite( LOG_CRI, szLogBuf );
		return -1;
	}

	/* ADD BY YOON 2008/09/17 */
	if (conflib_getNthTokenInFileSection (fname, "[APPLICATIONS]", "REANA", 1, tmp) < 0) {
		dAppLog (LOG_CRI, "[cdr_init] can't get queue key of RANA err=%s\n", strerror(errno));
		return -1;
	} else
		key = strtol(tmp, 0, 0);

	dREANAQid = Init_msgq( key );
	if( dREANAQid < 0 ) {
		dAppLog(LOG_CRI, "ERROR IN INITIAL MSGQ_REANA [RET:%d]", dREANAQid);
		exit(1);
	}

	if (conflib_getNthTokenInFileSection (fname, "[APPLICATIONS]", "RLEG", 1, tmp) < 0) {
		dAppLog (LOG_CRI, "[cdr_init] can't get queue key of RLEG err=%s\n", strerror(errno));
		return -1;
	} else
		key = strtol(tmp, 0, 0);

	dRLEGQid = Init_msgq( key );
	if( dRLEGQid < 0 ) {
		dAppLog(LOG_CRI, "ERROR IN INITIAL MSGQ_RLEG [RET:%d]", dRLEGQid);
		exit(1);
	}

	if( (pstMEMSINFO = nifo_init_zone("PANA", SEQ_PROC_ANA, DEF_NIFO_ZONE_CONF_FILE)) == NULL )
	{
		dAppLog( LOG_CRI, "ERROR nifo_init_zone NULL");
		return -10;
	}

	return 1;
}

/*******************************************************************************
 * 20080718 GET PDSN TYPE INFORMATION
*******************************************************************************/
int dGetPDSNTypeInfo()
{
    FILE            *fa;
    unsigned char   szBuffer[256];
        
        
    if( (fa = fopen( DEF_PDSNTYPEINFO_FILE, "r" )) == NULL ) {
        dAppLog( LOG_CRI, "FAIL OPEN %s", DEF_PDSNTYPEINFO_FILE );
        return -1;
    }   

    while( fgets( szBuffer, 256, fa ) != NULL ) {
    
        if( szBuffer[0] != '#' ) {
            dAppLog( LOG_CRI, "INVALID FILE FORMAT %s", DEF_PDSNTYPEINFO_FILE );
            fclose( fa );
            return -2;
        }

        if( szBuffer[1] == '#' )
            continue;
        else if( szBuffer[1] == 'E' )
            break;

#if 0   // 080721, poopee
        if( sscanf( &szBuffer[2], "%d %d", &uiModular, &uiSysModular ) == 2 ) {
            dAppLog( LOG_CRI, "PDSN TYPE INFO MODULAR:%u SYS_MODULAR:%u", 
                              uiModular, uiSysModular );
#else
        if( sscanf( &szBuffer[2], "%d %d %d", &uiModular, &uiSysModular, &uiRsltFlag ) == 3 ) {
            dAppLog( LOG_CRI, "PDSN TYPE INFO MODULAR:%u SYS_MODULAR:%u RSLT_FLAG:%u",
                              uiModular, uiSysModular, uiRsltFlag );
#endif
        }
    }

    fclose( fa );

    return 1;
}

/*******************************************************************************

*******************************************************************************/
int main (void)
{

	int			dRet;
	int			check_Index;
	time_t 		now;

	st_MsgQ		*pstMSGQHead;       // MsgQ Node

	/* ADD BY YOON 2008.10.16 */
	OFFSET 				offset, sub_offset;
	UCHAR 				*pNode, *pCurrNode, *pNextNode, *pNodeData;
	st_IPTCPHeader 		*pstIPTCPHeader;

    /* INITIALZE LOG */
    dRet = Init_logdebug( getpid(), "REANA", "/DSC/APPLOG" );

    SetUpSignal();

	/* SEMAPORE INITIALIZE */
	semid_mif = Init_sem( S_SEMA_MIF);
	if( semid_mif < 0  ) {
        dAppLog(LOG_CRI, "ERROR IN INITIAL SEM[SESS:%d]", semid_mif);
        exit(1);
    }

    /* 
     * INITIALIZE SHM 
     */
    dRet = dINIT_REANA_IPC();
    if( dRet < 0 ) {
        exit(1);
    }

#if 1
	if((check_Index = check_my_run_status("REANA")) < 0)
		exit(0);
#endif

	/*** DISPLAY VERSION : POOPEE */
	char 	vERSION[7] = "R1.0.0";

	if( (dRet=set_version(SEQ_PROC_REANA, vERSION)) < 0)
	{
		dAppLog( LOG_CRI, "SET_VERSION ERROR(RET=%d,IDX=%d,VER=%s)\n", dRet, SEQ_PROC_REANA, vERSION);
	}

	if (keepalivelib_init("REANA") < 0)   
        exit(0);
	
	dAppLog(LOG_CRI,"[REANA %s] [PROCESS INIT SUCCESS]", vERSION);

    while ( JiSTOPFlag )
	{
		keepalivelib_increase();

		now = time(0);
		g_timeindex = (now/300)%12;

#ifdef FUNC_TIME_CHECK
		time_t 	chkTime;
		chkTime = time(NULL);

		if( ((chkTime/60)%60) != g_CheckIndex ) {
			g_CheckIndex = (now/60)%60;
			PRINT_FUNC_TIME_CHECK(pFUNC);
		}
#endif
		// READ MSGQ 
		if((offset = nifo_msg_read(pstMEMSINFO, dREANAQid, NULL)) <= 0) {
			//usleep(0);
			commlib_microSleep(20000);
			continue;
		}

		pNextNode = nifo_ptr(pstMEMSINFO, offset);
		pCurrNode = NULL;

		while( pCurrNode != pNextNode ) {
			pNode = pNextNode;
			pCurrNode = pNextNode;
			sub_offset = nifo_offset(pstMEMSINFO, pNode);

			pNextNode = (UCHAR *)nifo_entry(nifo_ptr(pstMEMSINFO, ((NIFO *)pNextNode)->nont.offset_next), NIFO, nont);
			pNodeData = nifo_get_value(pstMEMSINFO, ETH_DATA_NUM, sub_offset);
			pstIPTCPHeader = (st_IPTCPHeader *)nifo_get_value(pstMEMSINFO, INFO_ANA_NUM, sub_offset);
			pstMSGQHead = (st_MsgQ *)nifo_get_value(pstMEMSINFO, MSGQ_HEADER_NUM, sub_offset);

			if( (pstIPTCPHeader == NULL) || (pNode == NULL)) {
				nifo_node_unlink_nont(pstMEMSINFO, pNode);
				nifo_node_delete(pstMEMSINFO, pNode);
			} else {
				nifo_node_unlink_nont(pstMEMSINFO, pNode);
#ifdef MEM_TEST
				nifo_node_delete(pstMEMSINFO, pNode);
				continue;
#endif
				switch( pstMSGQHead->uiReserved ) {
					case QID_REANA:

						dAppLog( LOG_DEBUG, "RECEIVED PACKET TYPE[%d] SRC[%s:%d] DEST[%s:%d]", 
								pstIPTCPHeader->stIPHeader.ucProtocol,
								CVT_ipaddr(pstIPTCPHeader->stIPHeader.dSrcIP), pstIPTCPHeader->stTCPHeader.usSrcPort,
								CVT_ipaddr(pstIPTCPHeader->stIPHeader.dDestIP), pstIPTCPHeader->stTCPHeader.usDestPort);

						/* TODO: 필터링은 필요 없나?  */
						dRet = dProcessRedirect(pNode, pNodeData, pstIPTCPHeader, pstMSGQHead);
						break;
					default:
						dAppLog( LOG_CRI, "RECEIVED UNKNOWN QID[%d] TYPE[%d] SRC[%u:%d] DEST[%u:%d]", 
								pstMSGQHead->uiReserved,
								pstIPTCPHeader->stIPHeader.ucProtocol,
								pstIPTCPHeader->stIPHeader.dSrcIP, pstIPTCPHeader->stTCPHeader.usSrcPort,
								pstIPTCPHeader->stIPHeader.dDestIP, pstIPTCPHeader->stTCPHeader.usDestPort);

						break;
				}
			}
			/* Delete All Node */
			nifo_node_delete(pstMEMSINFO, pNode);

		} /* end while */

	} /* end main loop */

    FinishProgram();

    return 1;
}

/* MODIFIED BY YOON 2008.10.16 */

#define WIDTH   16
int hexa_dump(char *debug_str, char *s, int len)
{
	char printStr[2048], buf[BUFSIZ],lbuf[BUFSIZ],rbuf[BUFSIZ];                                                                           
	unsigned char *p;                                                                                                     
	int line,i;
	
	bzero(printStr, 2048);
	dAppLog(LOG_DEBUG, "######### %s ##########", debug_str);
	p =(unsigned char *) s;
	for(line = 1; len > 0; len -= WIDTH,line++) {
		memset(lbuf,0,BUFSIZ);
		memset(rbuf,0,BUFSIZ);

		for(i = 0; i < WIDTH && len > i; i++,p++) {
			sprintf(buf,"%02x ",(unsigned char) *p);
			strcat(lbuf,buf);
			sprintf(buf,"%c",(!iscntrl(*p) && *p <= 0x7f) ? *p : '.');
			strcat(rbuf,buf);
		}
		sprintf(buf, "%04x: %-*s    %s\n", line - 1, WIDTH * 3, lbuf, rbuf);
		strcat(printStr, buf);
	}
	dAppLog(LOG_DEBUG, "\n%s", printStr);
	return line;
}

int dProcessRedirect(UCHAR *pNode, UCHAR *pNodeData, st_IPTCPHeader *pstIPTCPHeader, pst_MsgQ pstMsgQ)
{
	int 	dAgreeFlag=0;
	int 	*dDataLen;

	dDataLen = (U32 *)(&pNodeData[0]-4);    /* TLV 구조체의 len의 위치 */
	
	if(*dDataLen==0) {
		dAppLog( LOG_CRI, "[ERROR] DATA LENGTH[%d] SRCIP[%s] DSTIP[%s]", 
				*dDataLen, CVT_ipaddr(pstIPTCPHeader->stIPHeader.dSrcIP), CVT_ipaddr(pstIPTCPHeader->stIPHeader.dDestIP));
		return -1;
	}

#if HEXADUMP
#if 0
	FILE 	*tmpfp;
	int  	size=0, len=0;
	char 	tmpStr[256];
	char	*str = &tmpStr[0];
	
	len = *dDataLen;
	if (pNodeData != NULL) {
		if ((tmpfp = fopen("/DSC/NEW/DATA/nifo_node.dat", "wb+"))==NULL){
			return 0;
		}
		memcpy(tmpStr, &len, 4); 
		memcpy(&tmpStr[4], pNodeData, len); 
		write(tmpfp, tmpStr, len+ 4);
		fclose(tmpfp);
	}
#endif

//	hexa_dump("DATA", pNodeData, *dDataLen);

#endif

	dAgreeFlag = parsedata(pNodeData, *dDataLen);
#if 0
	dAppLog( LOG_DEBUG, "dAgreeFlag: %d", dAgreeFlag);
#endif
	if(dAgreeFlag) {
		dAppLog( LOG_DEBUG, "[AGREE] SUBSCRIBER SRCIP[%s] DSTIP[%s]", 
				CVT_ipaddr(pstIPTCPHeader->stIPHeader.dSrcIP), CVT_ipaddr(pstIPTCPHeader->stIPHeader.dDestIP));

		dSendToMsg2 (dRLEGQid, pstMsgQ, pstIPTCPHeader);
	}
	
	return 0;
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


