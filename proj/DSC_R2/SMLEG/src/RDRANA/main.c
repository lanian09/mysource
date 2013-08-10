/**********************************************************
                 KTF IPAF Project

   Author   : Park Si Woo
   Modified : Yoon Jin Seok
   Section  : ABLEX Develpment
   SCCS ID  : %W%
   Date     : %G%
   Revision History :
	'03.  1. 6	Revised by Jiyoon Chung

   Description:

   Copyright (c) Infravalley 2003
***********************************************************/

/**A.1*  File Inclusion ***********************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <time.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <unistd.h>
#include <pcap.h>
//#include <mysql_db_tables.h>
#include <mysql.h>
#include <ctype.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/stat.h>

//#include "VJ.h"
#include <sys/ethernet.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <Analyze_Ext_Abs.h>
#include <Ethernet_header.h>
#include <packet_def.h>

#include <eth_capd.h>
#include <shmutil.h>
#include <utillib.h>
#include "Errcode.h"
#include "capd_global.h"
#include <init_shm.h>
#include "comm_define.h"
#include "sfm_msgtypes.h"
#include "rdrheader.h"
#include "capd_msgtypes.h"
#include "comm_msgtypes.h"
#include "comm_smpp_if.h"
#include "trclib.h"

//#include "mems.h"
#include "nifo.h"
#include "keepalivelib.h"
#include "comm_util.h"
#include "comm_trace.h"
#include "sysconf.h"
#include "ipaf_svc.h"
#include "ipam_sys.h" // MID_TRC 100 정의 
//#include "packet_def.h"
#include "capd_mml.h"
#include "comm_session.h"
#include "rdrheader.h"

#define TOUSHORT(x) (USHORT)(*(x)<<8|*(x+1))
#define TOULONG(x)  (ULONG)(*(x)<<24|*(x+1)<<16|*(x+2)<<8|*(x+3)) 
#if 0
// 20110208 by dcham, for delete RDR
#define RDRANA_DEL_TIME 60*10
#endif
/* added by dcham(2011.03.15) for */ 
#define RDR_DEL_POS 60*60
#define STAT_UNIT 5

int 	dReadFLTIDXFile(void);
int 	InitSHM_TRACE(void);
void 	dSetCurTrace(NOTIFY_SIG *pNOTISIG);

extern int      check_my_run_status (char *procname);
extern int 		INIT_CAPD_IPCS(void);
extern void 	SetupSignal(void);

int Analyze_Block(const char *rdrfld, RDR_BLOCK *pstRdr, int fldTotLen, int num_of_fld);
int Analyze_TR(const char *payload, RDR_TR *pstTR, int fldTotLen, int num_of_fld);
int Analyze_TLV(const char *payload, int type, PARA_VAL *tlv, int *pos);
int Insert_Block(RDR_BLOCK *pstBlock);
int Check_SmsSend(RDR_BLOCK *pstBlock);
int Check_Package(int pkgId);
int SendSmsMsgQ(RDR_BLOCK *pstBlock); // SMPP 프로세스로 메시지 큐를 보낸다. 
int rdrana_RDRDelete(void); // 2010.02.07 by dcham, 특정 주기로 RDR data delete

extern int Send_CondTrcMsg_BLOCK(RDR_BLOCK stBlock, int traceType, char *ip);
extern int Send_CondTrcMsg_TR(RDR_TR stTR, int traceType, char *ip);

extern int dLoad_TrcConf(pst_SESSInfo pstInfo);
extern void dLog_TrcConf(st_SESSInfo  *pstCallTrcList);

extern int rdrana_mmc_sync_rule_file2 (IxpcQMsgType *rxIxpcMsg);
extern int rdrana_mmc_sync_sms_msg (IxpcQMsgType *rxIxpcMsg);
extern int rdrana_mmcHdlrVector_qsortCmp (const void *a, const void *b);
extern void FinishProgram(void);
/* DEL : by june, 2010-10-03
 * DESC: TRACE 에 사용되는 SESSION 정보 참조 파트 주석 처리
extern char * find_imsi_rad_sess (rad_sess_key *pkey);
 */

int goBlock(char *payload, int totLen);
int goTrans(char *payload, int totLen);
int isTimeToWork(void);

static char vERSION[7] = "R2.0.0";	// BEFORE: R1.2.0 (2011-03-02) -> R2.0.0 (2011-05-09)

extern int		keepalivelib_init(char *processName);

char			sysLable[COMM_MAX_NAME_LEN], mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN];

int 				dMyQid;
int 				dCAPDQid;
int 				dSmppQid;
int 				dIxpcQid;
int 				dMmcrQid;
SFM_SysCommMsgType  *loc_sadb;

char			trcBuf[TRCBUF_LEN], trcTmp[TRCTMP_LEN];

unsigned char		ruleSetList[MAX_PKG_ID_NUMBER];
RuleSetList			g_stRule[MAX_PKG_ID_NUMBER];
RuleEntryList   	g_stEntry[MAX_RULE_ENTRY_LIST];

// TRACE_INFO.conf 구조체 
st_SESSInfo			*gpTrcList[DEF_SET_CNT];
st_SESSInfo			*gpCurTrc; // CURRENT OPERATION pointer

st_NOTI				gstIdx;
st_NOTI				*gpIdx = &gstIdx;


char smsBody[SMS_MAX_MSG_LEN] = {0,}; // SMS MSG 

SMS_INFO stSms;

SCE_LIST	g_stSce[MAX_SCE_NUM];

int numMmcHdlr = 2;

RDRMmcHdlrVector mmcHdlrVector[MAX_MMC_HANDLER] =
{
	{"sync-rule-file2", 		rdrana_mmc_sync_rule_file2},
	{"sync-sms-msg", 			rdrana_mmc_sync_sms_msg},
};

MYSQL			sql, *conn;

struct in_addr cli_ip ;
char	g_CliIP[24];

struct in_addr serv_ip ;
char	g_ServIP[24];

/* added by dcham(2011.03.15) for job flag */
int     workFLAG = 0; 

/* ethernet headers are always exactly 14 bytes [1] */
#define SIZE_ETHERNET 14
#define SIZE_RDR_SOME_HEADER 15 

int                 JiSTOPFlag;
int                 FinishFlag;

/* FOR LOG */
char                szLogBuf[1024];

int got_packet(char *payload, int totLen);

/*******************************************************************************

*******************************************************************************/
static void sig_user( int signo )
{
   // int i=0, j;

    dAppLog( LOG_CRI, "SIGNAL [EXIT:%d]", signo );

    signal(SIGSEGV, NULL);
    raise(SIGSEGV);

    exit(1);
}

int main(int argc, char *argv[])
{
	int 	ret;
	int		check_Index;
	char 	logbuffer[4096];
	time_t  now_t, pre_t;    

    GeneralQMsgType     rxGenQMsg;
    IxpcQMsgType        *rxIxpcMsg;
	NOTIFY_SIG 			*pNOTIFY;

	memset(&rxGenQMsg, 0x00, sizeof(GeneralQMsgType));

	Init_logdebug( getpid(), "RDRANA", "/DSC/APPLOG" );

	/* SIGNAL */
	signal( SIGSEGV, sig_user );
	SetupSignal();

	ret = INIT_CAPD_IPCS();
	if(ret < 0) {
		sprintf(logbuffer, "FAIL[init_ipc] [%s] %d.", strerror(errno), ret);
		dAppWrite(LOG_CRI, logbuffer);
		exit(0);
	}

	if ((ret=set_version(SEQ_PROC_RDRANA, vERSION)) < 0 ) {
		dAppLog( LOG_CRI, "SET_VERSION ERROR(RET=%d,IDX=%d,VER=%s)\n",
				ret,SEQ_PROC_RDRANA,vERSION);
	}

#if 1 
	if((check_Index = check_my_run_status("RDRANA")) < 0)
		exit(0);
#endif

	if( keepalivelib_init("RDRANA") < 0 )
		exit(1);

	ret = dReadFLTIDXFile();
	if( ret < 0 ) {
		dAppLog( LOG_CRI, "dReadFLTIDXFile() FAIL RET:%d", ret);
	}

	dAppLog(LOG_CRI, "==RDRANA START [VERSION:%s]", vERSION);
	time(&pre_t);

    while ( JiSTOPFlag )
    {
        keepalivelib_increase();

		while ((ret = msgrcv(dMyQid, &rxGenQMsg, sizeof(GeneralQMsgType), 0, IPC_NOWAIT)) > 0 )
		{
			switch (rxGenQMsg.mtype)
			{
				case MTYPE_TRC_CONFIG: // 42
					// TODO TrcInfo.conf 파일 reloading.
					//memset(&g_stTrcInfo, 0x00, sizeof(st_SESSInfo));
					//dLoad_TrcConf(&g_stTrcInfo);
				    //dLog_TrcConf(&g_stTrcInfo);
    				rxIxpcMsg = (IxpcQMsgType*)rxGenQMsg.body;
					pNOTIFY = (NOTIFY_SIG *)rxIxpcMsg->body;	
					dSetCurTrace(pNOTIFY);
					dAppLog(LOG_DEBUG, "Trace Msg Get....");
					break;

				case MTYPE_MMC_REQUEST :
					ret = getMMCMsg ((IxpcQMsgType*)rxGenQMsg.body);
					break;

				case MTYPE_BLOCK_REPORT : // 60
    				rxIxpcMsg = (IxpcQMsgType*)rxGenQMsg.body;
					ret = goBlock ((char *)rxIxpcMsg->body, rxIxpcMsg->head.bodyLen);
					memset(rxIxpcMsg->body, 0x00, rxIxpcMsg->head.bodyLen);
					break;

				case MTYPE_TRANS_REPORT : // 60
    				rxIxpcMsg = (IxpcQMsgType*)rxGenQMsg.body;
					ret = goTrans (rxIxpcMsg->body, rxIxpcMsg->head.bodyLen);
					memset(rxIxpcMsg->body, 0x00, rxIxpcMsg->head.bodyLen);
					break;

				default:
					dAppLog(LOG_DEBUG, "unexpected mtype[%d]", rxGenQMsg.mtype);
					break;
			}
			keepalivelib_increase();
		}

		time(&now_t);
		/* added by dcham(2011.03.15) for 5의 배수 +2,3분 마다 delete 로직 수행 */
		if(isTimeToWork())
		{
			if(rdrana_RDRDelete() == -1) 
				dAppLog(LOG_CRI, "RDR Delete Fail, pre_t[%ld] now_t[%ld]", pre_t, now_t);
		}

		usleep(1);
	}
	/*
	   FILE *fp = NULL;
	   char *fname = "/CM/scmscm/cm/adapters/CSVAdapter/csvfiles/4042321984/18.csv.tmp";
	   char *fname2 = "/CM/scmscm/cm/adapters/CSVAdapter/csvfiles/4042321936/165.csv";
	char getBuf[1024];

	fp = fopen(fname, "r");
	memset(getBuf, 0x00, sizeof(getBuf));
	while( (fgets (getBuf, sizeof(getBuf), fp) != NULL ))
	{
		ret = goBlock (getBuf, strlen(getBuf));
		if( ret >= 0 )
			dAppLog(LOG_CRI, "Success Block Rdr end");
		memset(getBuf, 0x00, sizeof(getBuf));
	}
	fclose(fp);

	fp = fopen(fname2, "r");
	memset(getBuf, 0x00, sizeof(getBuf));
	while( (fgets( getBuf, sizeof(getBuf), fp) != NULL ))
	{
		ret = goTrans (getBuf, strlen(getBuf));
		if( ret >= 0 )
			dAppLog(LOG_CRI, "Success Transaction Rdr end");
		memset(getBuf, 0x00, sizeof(getBuf));
	}

	*/
	FinishProgram();

	return 0;
}

void fillBlk(char *payload, RDR_BLOCK *pstBlk, int fldIdx, int fldLen) 
{
	char Buff[257];
	int	 _msec_len = 3;

	struct in_addr addr_ip ;

	memset(Buff, 0x00, sizeof(Buff));
	if( fldLen > 0 && fldLen <= 256 )
	{
		strncpy(Buff, payload, fldLen);
		Buff[strlen(Buff)] = 0;
		dAppLog(LOG_CRI, "[%d] VAL[%s]", fldIdx, Buff);
	}
	else
		return ;

	switch( fldIdx )
	{
		case 0:
			Buff[strlen(Buff)-_msec_len] = 0; // 뒤에 msec 부분을 잘라낸다. 
			pstBlk->dTimeStamp = atoi(Buff);
			break;

		case 1:
			sprintf(pstBlk->szTag, "%s", Buff);
			pstBlk->szTag[strlen(pstBlk->szTag)] = 0;
			break;

		case 2:
			sprintf(pstBlk->szSceIp, "%s", Buff);
			pstBlk->szSceIp[strlen(pstBlk->szSceIp)] = 0;
			break;

		case 3:
			sprintf(pstBlk->ucSubscriberID, "%s", Buff);
			pstBlk->ucSubscriberID[strlen(pstBlk->ucSubscriberID)] = 0;
			break;

		case 4:
			pstBlk->usPackageID = atoi(Buff);
			break;

		case 5:
			pstBlk->dServiceID = atoi(Buff);
			break;

		case 6:
//			pstBlk->dProtocolID = atoi(Buff);
			break;

		case 7:
			pstBlk->uiClientIP = atoi(Buff);

			addr_ip.s_addr = pstBlk->uiClientIP;
			strcpy(pstBlk->szCliIP, inet_ntoa(addr_ip));
			pstBlk->szCliIP[strlen(pstBlk->szCliIP)] = 0;
			break;

		case 8:
			pstBlk->usClientPort = atoi(Buff);
			break;

		case 9:
			pstBlk->uiSrcIP = atoi(Buff);

			addr_ip.s_addr = pstBlk->uiSrcIP;
			strcpy(pstBlk->szSrcIP, inet_ntoa(addr_ip));
			pstBlk->szSrcIP[strlen(pstBlk->szSrcIP)] = 0;
			break;

		case 10:
			pstBlk->usSrcPort = atoi(Buff);
			break;

		case 11:
//			pstBlk->ucInitSide = atoi(Buff);
			break;

		case 12:
//			sprintf(pstBlk->ucAccString, "%s", Buff);
//			pstBlk->ucAccString[strlen(pstBlk->ucAccString)] = 0;
			break;

		case 13:
//			sprintf(pstBlk->ucInfoString, "%s", Buff);
//			pstBlk->ucInfoString[strlen(pstBlk->ucInfoString)] = 0;
			break;

		case 14:
			pstBlk->ucBlkReason = atoi(Buff);
			break;

		case 15:
			pstBlk->uiBlkRdrCnt = atoi(Buff);
			break;

		case 16: // REDIRECTED
			break;

		case 17:
			pstBlk->uiTime = atoi(Buff);
			dAppLog(LOG_CRI, "uiTime : %d", pstBlk->uiTime);
			break;

		default :
			break;
	}
}

void fillTrans(char *payload, RDR_TR *pstTr, int fldIdx, int fldLen) 
{
	char Buff[257];
	int	 _msec_len = 3;

	struct in_addr addr_ip ;

	if( fldIdx > 19 )
		return ;

	memset(Buff, 0x00, sizeof(Buff));
	if( fldLen > 0 && fldLen <= 256 )
	{
		strncpy(Buff, payload, fldLen);
		Buff[strlen(Buff)] = 0;
	}
	else
		return ;

	switch( fldIdx )
	{
		case 0:
			Buff[strlen(Buff)-_msec_len] = 0; // 뒤에 msec 부분을 잘라낸다. 
			pstTr->dTimeStamp = atoi(Buff);
			break;

		case 1:
			sprintf(pstTr->szTag, "%s", Buff);
			pstTr->szTag[strlen(pstTr->szTag)] = 0;
			break;

		case 2:
			sprintf(pstTr->szSceIp, "%s", Buff);
			pstTr->szSceIp[strlen(pstTr->szSceIp)] = 0;
			break;

		case 3:
			sprintf(pstTr->ucSubscriberID, "%s", Buff);
			pstTr->ucSubscriberID[strlen(pstTr->ucSubscriberID)] = 0;
			break;

		case 4:
			pstTr->usPackageID = atoi(Buff);
			break;

		case 5:
			pstTr->dServiceID = atoi(Buff);
			break;

		case 6:
//			pstTr->usProtocolID = atoi(Buff);
			break;

		case 7:
			pstTr->dSampleSize = atoi(Buff);
			break;

		case 8:
			pstTr->uiServerIP = atoi(Buff);

			addr_ip.s_addr = pstTr->uiServerIP;
			strcpy(pstTr->szSvrIP, inet_ntoa(addr_ip));
			pstTr->szSvrIP[strlen(pstTr->szSvrIP)] = 0;
			break;

		case 9:
			pstTr->usServerPort = atoi(Buff);
			break;

		case 10:
//			sprintf(pstTr->ucAccString, "%s", Buff);
//			pstTr->ucAccString[strlen(pstTr->ucAccString)] = 0;
			break;

		case 11:
//			sprintf(pstTr->ucInfoString, "%s", Buff);
//			pstTr->ucInfoString[strlen(pstTr->ucInfoString)] = 0;
			break;

		case 12:
			pstTr->uiClientIP = atoi(Buff);

			addr_ip.s_addr = pstTr->uiClientIP;
			strcpy(pstTr->szCliIP, inet_ntoa(addr_ip));
			pstTr->szCliIP[strlen(pstTr->szCliIP)] = 0;
			break;

		case 13:
			pstTr->usClientPort = atoi(Buff);
			break;

		case 14:
//			pstTr->ucInitSide = atoi(Buff);
			break;

		case 15:
			pstTr->uiTime = atoi(Buff);
			break;

		case 16:
//			pstTr->uiMilDura = atoi(Buff);
			break;

		case 17:
//			pstTr->cTimeFrame = atoi(Buff);
			break;

		case 18:
			pstTr->uiUpVol = atoi(Buff);
			break;

		case 19:
			pstTr->uiDnVol = atoi(Buff);
			break;

		// 이하 필드는 스킵한다. 

		default :
			break;
	}
}

void printCsvBlk(RDR_BLOCK *pstBlk)
{
	dAppLog(LOG_DEBUG, "TIME STAMP:[%d]", pstBlk->dTimeStamp);
	dAppLog(LOG_DEBUG, "TAG:[%s]", pstBlk->szTag);
	dAppLog(LOG_DEBUG, "SCE IP:[%s]", pstBlk->szSceIp);
	dAppLog(LOG_DEBUG, "SUBS ID:[%s]", pstBlk->ucSubscriberID);
	dAppLog(LOG_DEBUG, "PKG ID:[%d]", pstBlk->usPackageID);
	dAppLog(LOG_DEBUG, "SVC ID:[%d]", pstBlk->dServiceID);
	dAppLog(LOG_DEBUG, "PROTO ID:[%d]", pstBlk->dProtocolID);
	dAppLog(LOG_DEBUG, "CLI IP:[%d][%s]", pstBlk->uiClientIP, pstBlk->szCliIP);
	dAppLog(LOG_DEBUG, "CLI PORT:[%d]", pstBlk->usClientPort);
	dAppLog(LOG_DEBUG, "SVR IP:[%d][%s]", pstBlk->uiSrcIP, pstBlk->szSrcIP);
	dAppLog(LOG_DEBUG, "SVR PORT:[%d]", pstBlk->usSrcPort);
	dAppLog(LOG_CRI, "uiTime:[%d]", pstBlk->uiTime);

}

int csvBlkParse(char *payload, int totLen, RDR_BLOCK *pstBlk)
{
	int cIdx = 0;
	int	csvIdx = 0, nIdx = 0;
	int	csvLen = 0;

	while( nIdx <= totLen )
	{
		if( payload[nIdx] == ',' || payload[nIdx] == '\0' || payload[nIdx] == '\n')
		{
			// nIdx == cIdx : 연속된 ',,' 가 나오는 경우다. 
			if( nIdx == cIdx )
			{
				csvIdx++;
				cIdx++;
				nIdx++;
				continue;
			}
			else
			{
				csvLen = nIdx - cIdx;
				if( csvLen > 0 )
					fillBlk(&payload[cIdx], pstBlk, csvIdx, csvLen);
				cIdx = nIdx + 1;
				csvIdx++;
				nIdx++;
			}
		}
		else
			nIdx++;
	}
	printCsvBlk(pstBlk);

	return 0;
}

int csvTransParse(char *payload, int totLen, RDR_TR *pstTr)
{
	int cIdx = 0;
	int	csvIdx = 0, nIdx = 0;
	int	csvLen = 0;

	while( nIdx <= totLen )
	{
		if( payload[nIdx] == ',' || payload[nIdx] == '\0' )
		{
			// nIdx == cIdx : 연속된 ',,' 가 나오는 경우다. 
			if( nIdx == cIdx )
			{
				csvIdx++;
				cIdx++;
				nIdx++;
				continue;
			}
			else
			{
				csvLen = nIdx - cIdx;
				if( csvLen > 0 )
					fillTrans(&payload[cIdx], pstTr, csvIdx, csvLen);
				cIdx = nIdx + 1;
				csvIdx++;
				nIdx++;
			}
		}
		else
			nIdx++;
	}

	return 0;
}


// Block RDR
int goBlock(char *payload, int totLen)
{
	int ret = 0, j = 0;
/* DEL : by june, 2010-10-03
    rad_sess_key stRadKey;
	char *radIMSI;
 */

	RDR_BLOCK		stBlock;
    
	payload[strlen(payload)] = 0;
	dAppLog(LOG_CRI, "START ANALYZE BLOCK[%d] LEN[%d] PAYLOAD[%s]", time(0), totLen, payload);

	memset(&stBlock, 0x00, sizeof(stBlock));
	ret = csvBlkParse(payload, totLen, &stBlock);
	if( ret >= 0 )
		dAppLog(LOG_CRI, "SUCCESS BLOCK Parsing Time[%d] SCE[%s]", stBlock.dTimeStamp, stBlock.szSceIp);

	dAppLog(LOG_CRI, "END ANALYZE BLOCK RDR : [IMSI:%s],[P/H:%02d %02d],[SID:%d],[CIP:%s],[SIP:%s]",
						stBlock.ucSubscriberID, 
						g_stRule[stBlock.usPackageID].pBit,
						g_stRule[stBlock.usPackageID].hBit, 
						stBlock.dServiceID, 
						stBlock.szCliIP, 
						stBlock.szSrcIP);

	if (loc_sadb->loc_system_dup.myLocalDupStatus == SYS_STATE_ACTIVE /** ACTIVE **/) 
	{
		dAppLog(LOG_WARN, "SYSTEM MODE : ACTIVE");

		if ( gpCurTrc->dTCount > 0 && ret >= 0 )
		{
			dAppLog(LOG_WARN, "TRACE EXIST");
			// TODO Trace message 전송 From MMCR To COND
			for( j = 0; j < MAX_TRACE_NUM; j++ )
			{
				if(gpCurTrc->stTrc[j].dType == TYPE_IMSI )
				{
					if(!strcmp(gpCurTrc->stTrc[j].szImsi, stBlock.ucSubscriberID) ) 
					{
						Send_CondTrcMsg_BLOCK(stBlock, gpCurTrc->stTrc[j].dType, stBlock.szCliIP);
						continue;
					}
					else if( !strncmp(stBlock.ucSubscriberID, "N/A", 3) )
					{

/* DEL : by june, 2010-10-03
 * DESC: TRACE 에 사용되는 SESSION 정보 참조 파트 주석 처리
 */
#if 0
						stRadKey.mobIP = stBlock.uiClientIP;
						radIMSI = find_imsi_rad_sess(&stRadKey);
						if( radIMSI == NULL )
						{
							dAppLog(LOG_CRI, "UNKNOWN IMSI: NOT FOUND SESSION IMSI. IP");
							stRadKey.mobIP = 0;
							continue;
						}
						else
						{
							dAppLog(LOG_DEBUG, "UNKNOWN IMSI: FIND SESSION->IMSI[%s]", radIMSI);
							strncpy(stBlock.ucSubscriberID, radIMSI, strlen(radIMSI));
							stBlock.ucSubscriberID[strlen(stBlock.ucSubscriberID)] = 0;

							if(!strcmp(g_stTrcInfo.stTrc[j].szImsi, stBlock.ucSubscriberID) ) 
							{
								dAppLog(LOG_DEBUG, "FIND SESSION->IMSI[%s] -> Tracing Block", radIMSI);
								Send_CondTrcMsg_BLOCK(stBlock, g_stTrcInfo.stTrc[j].dType, stBlock.szCliIP);
								stRadKey.mobIP = 0;
								radIMSI = NULL;
								continue;
							}
						}
#else
						dAppLog(LOG_CRI, "UNKNOWN IMSI: N/A");
#endif
					}
				}
				else // TYPE_IP
				{
					if(!strcmp(gpCurTrc->stTrc[j].szImsi, stBlock.szCliIP)) 
					{
						Send_CondTrcMsg_BLOCK(stBlock, gpCurTrc->stTrc[j].dType, stBlock.szCliIP);
						continue;
					}
					else if(!strcmp(gpCurTrc->stTrc[j].szImsi, stBlock.szSrcIP)) 
					{
						// 내부적으로 trace type 에 + 1을 부여하여 client IP와 server IP 를 구분한다. 
						Send_CondTrcMsg_BLOCK(stBlock, gpCurTrc->stTrc[j].dType + 1, stBlock.szSrcIP);
						continue;
					}
				}
			}// end-of-for
		} // trace 가 있을 때 
		else
			dAppLog(LOG_WARN, "No Register Trace ");

		Check_SmsSend(&stBlock); // pkg check, 가입자 check(조건미정):select->(update/send)
	} // ACTIVE 일 때 

	Insert_Block(&stBlock);

	return 1;
}


// Transaction RDR
int goTrans(char *payload, int totLen)
{
	int ret = 0, j = 0;

/* DEL : by june, 2010-10-03
    rad_sess_key stRadKey;
	char *radIMSI;
 */

	RDR_TR		stTR;

	if (loc_sadb->loc_system_dup.myLocalDupStatus == SYS_STATE_ACTIVE /** ACTIVE **/) 
	{
		dAppLog(LOG_DEBUG, "SYSTEM MODE : ACTIVE");


		if ( gpCurTrc->dTCount > 0 )
		{
			dAppLog(LOG_CRI, "TRACE EXIST. START TRANSACTION LEN[%d] PAYLOAD[%s]", totLen, payload);

			memset(&stTR, 0x00, sizeof(stTR));
			dAppLog(LOG_CRI,"START ANALYZE Trans RDR TIME[%d]", time(0));

			payload[strlen(payload)] = 0;
			ret = csvTransParse(payload, totLen, &stTR);
			if( ret >= 0 )
				dAppLog(LOG_CRI, "SUCCESS TRANSACTION Parsing Time[%d] SCE[%s]", stTR.dTimeStamp, stTR.szSceIp);

			dAppLog(LOG_CRI, "END ANALYZE TRANSACTION RDR[%d] : [IMSI:%s],[P/H:%02d %02d],[SID:%d],[CIP:%s],[SIP:%s]",
					time(0),
					stTR.ucSubscriberID, 
					g_stRule[stTR.usPackageID].pBit,
					g_stRule[stTR.usPackageID].hBit, 
					stTR.dServiceID, 
					stTR.szCliIP, 
					stTR.szSvrIP);

			for( j = 0; j < MAX_TRACE_NUM; j++ )
			{
				if(gpCurTrc->stTrc[j].dType == TYPE_IMSI )
				{
					if(!strcmp(gpCurTrc->stTrc[j].szImsi, stTR.ucSubscriberID) ) 
					{
						Send_CondTrcMsg_TR(stTR, gpCurTrc->stTrc[j].dType, stTR.szCliIP);
						continue;
					}
					else if( !strncmp(stTR.ucSubscriberID, "N/A", 3) )
					{
/* DEL : by june, 2010-10-03
 * DESC: TRACE 에 사용되는 SESSION 정보 참조 파트 주석 처리
 */
#if 0
						stRadKey.mobIP = stTR.uiClientIP;
						radIMSI = find_imsi_rad_sess(&stRadKey);
						if( radIMSI == NULL )
						{
							dAppLog(LOG_CRI, "UNKNOWN IMSI: NOT FOUND SESSION IMSI. IP");
							stRadKey.mobIP = 0;
							continue;
						}
						else
						{
							dAppLog(LOG_DEBUG, "UNKNOWN IMSI: FIND SESSION->IMSI[%s]", radIMSI);
							strncpy(stTR.ucSubscriberID, radIMSI, strlen(radIMSI));
							stTR.ucSubscriberID[strlen(stTR.ucSubscriberID)] = 0;

							if(!strcmp(g_stTrcInfo.stTrc[j].szImsi, stTR.ucSubscriberID) ) 
							{
								dAppLog(LOG_DEBUG, "FIND SESSION->IMSI[%s] -> Tracing Trans", radIMSI);
								Send_CondTrcMsg_TR(stTR,g_stTrcInfo.stTrc[j].dType, stTR.szCliIP);
								stRadKey.mobIP = 0;
								radIMSI = NULL;
								continue;
							}
						}
#else
						dAppLog(LOG_CRI, "UNKNOWN IMSI: NOT FOUND IMSI(N/A). IP");
#endif
					}
				}
				else // TYPE_IP
				{
					if(!strcmp(gpCurTrc->stTrc[j].szImsi, stTR.szCliIP)) 
					{
						Send_CondTrcMsg_TR(stTR, gpCurTrc->stTrc[j].dType, stTR.szCliIP);
						continue;
					}
					else if(!strcmp(gpCurTrc->stTrc[j].szImsi, stTR.szSvrIP)) 
					{
						// 내부적으로 trace type 에 + 1을 부여하여 client IP와 server IP 를 구분한다. 
						Send_CondTrcMsg_TR(stTR, gpCurTrc->stTrc[j].dType + 1, stTR.szSvrIP);
						continue;
					}
				}
			}// end-of-for
			if( j == MAX_TRACE_NUM )
			{
				dAppLog(LOG_DEBUG, "Not Matched Trace List. ");
			}
		} // Trace가 있을 때 
		else
			dAppLog(LOG_DEBUG, "Not Registered Trace List. ");
	} // ACTIVE 일 때 
	else
		dAppLog(LOG_CRI, "System is STANDBY BYE BYE.");

	return 1;
}


int Analyze_Block(const char *payload, RDR_BLOCK *pstBlock, int fldTotLen, int num_of_fld)
{
	int i;
	int pos = 0;
	PARA_VAL	tlv;
	int paraType = 0, paraLen = 0;
	int totLen = 0;

	for( i = 0; i < num_of_fld; i++ )
	{
		paraType = payload[pos++];
		memset(&tlv, 0x00, sizeof(tlv));
		paraLen = Analyze_TLV(payload, paraType, &tlv, &pos);
		totLen += paraLen;
		if( paraLen < 0 || totLen > fldTotLen )
		{
		//	printf("parameter [%d]th len Invalid...:%d\n",i, paraLen);
			return -1;
		}
		switch(i)
		{
			/*
			case 0 :
				memcpy(pstBlock->ucTimeStamp,tlv[i],paraLen);
				break;
			case 1 :
				pstBlock->dRecordSource = TOULONG(tlv[i]);
				break;
			*/
			case 0 :
				if( paraLen > 0 && paraLen < MAX_SUBS_ID_LEN)
				{
					memcpy(pstBlock->ucSubscriberID, tlv.ucVal, paraLen);
					pstBlock->ucSubscriberID[strlen(pstBlock->ucSubscriberID)] = 0;
				}
				break;
			case 1 :
				pstBlock->usPackageID = tlv.ushort_val;
				break;
			case 2 :
				pstBlock->dServiceID = tlv.int_val;
				break;
			case 3 :
				pstBlock->dProtocolID = tlv.short_val;
				break;
			case 4 :
				pstBlock->uiClientIP = tlv.uint_val;
				break;
			case 5 :
				pstBlock->usClientPort = tlv.ushort_val;
				break;
			case 6 :
				pstBlock->uiSrcIP = tlv.uint_val;
				break;
			case 7 :
				pstBlock->usSrcPort = tlv.ushort_val;
				break;
			case 8 :
				pstBlock->ucInitSide = tlv.uchar_val;
				break;
			case 9 :
				if ( paraLen > 0 )
					memcpy(pstBlock->ucAccString, tlv.ucVal, paraLen);
				else 
					pstBlock->ucAccString[0] = 0;
				break;
			case 10 :
				if ( paraLen > 0 )
					memcpy(pstBlock->ucInfoString, tlv.ucVal, paraLen);
				else
					pstBlock->ucInfoString[0] = 0;
				break;
			case 11 :
				pstBlock->ucBlkReason = tlv.uchar_val;
				break;
			case 12 :
				pstBlock->uiBlkRdrCnt = tlv.uint_val;
				break;
			case 13 :
				pstBlock->ucRedirect = tlv.uchar_val;
				break;
			case 14 :
				pstBlock->uiTime = tlv.uint_val;
				break;
			default :
				dAppLog(LOG_INFO, "%d th parameter is not unknown block field\n",i);
				break;
		}
	}

	return 0;
}

int Analyze_TR(const char *payload, RDR_TR *pstTR, int fldTotLen, int num_of_fld)
{
	int i;
	int pos = 0;
	PARA_VAL	tlv;
	int paraType = 0, paraLen = 0;
	int totLen = 0;

	for( i = 0; i < num_of_fld; i++ )
	{
		paraType = payload[pos++];
		memset(&tlv, 0x00, sizeof(tlv));
		paraLen = Analyze_TLV(payload, paraType, &tlv, &pos);
		totLen += paraLen;
		if( paraLen < 0 || totLen > fldTotLen )
		{
	//		printf("parameter [%d]th len Invalid...:%d\n",i, paraLen);
			return -i;
		}
		switch(i)
		{
			case 0 :
				if( paraLen > 0 )
				{
					memcpy(pstTR->ucSubscriberID, tlv.ucVal, paraLen);
					pstTR->ucSubscriberID[paraLen] = 0;
				}
				else
					pstTR->ucSubscriberID[0] = 0;
				break;
			case 1 :
				pstTR->usPackageID = tlv.ushort_val;
				break;
			case 2 :
				pstTR->dServiceID = tlv.int_val;
				break;
			case 3 :
				pstTR->usProtocolID = tlv.short_val;
				break;
			case 4 :
				pstTR->dSampleSize = tlv.int_val;
				break;
			case 5 :
				pstTR->uiServerIP = tlv.uint_val;
				break;
			case 6 :
				pstTR->usServerPort = tlv.ushort_val;
				break;
			case 7 :
				if( paraLen > 0 )
				{
					memcpy(pstTR->ucAccString, tlv.ucVal, paraLen);
					pstTR->ucAccString[paraLen] = 0;
				}
				else
					pstTR->ucAccString[0] = 0;
				break;
			case 8 :
				if( paraLen > 0 )
				{
					memcpy(pstTR->ucInfoString, tlv.ucVal, paraLen);
					pstTR->ucInfoString[paraLen] = 0;
				}
				else
					pstTR->ucInfoString[0] = 0;
				break;
			case 9 :
				pstTR->uiClientIP = tlv.uint_val;
				break;
			case 10 :
				pstTR->usClientPort = tlv.ushort_val;
				break;
			case 11 :
				pstTR->cInitSide = tlv.char_val;
				break;
			case 12 :
				pstTR->uiTime = tlv.uint_val;
				break;
			case 13 :
				pstTR->uiMilDura = tlv.uint_val;
				break;
			case 14 :
				pstTR->cTimeFrame = tlv.char_val;
				break;
			case 15 :
				pstTR->uiUpVol = tlv.uint_val;
				break;
			case 16 :
				pstTR->uiDnVol = tlv.uint_val;
				break;
			case 17 :
				pstTR->usSubCntId = tlv.ushort_val;
				break;
			case 18 :
				pstTR->usGblCntId = tlv.ushort_val;
				break;
			case 19 :
				pstTR->usPkgCntId = tlv.ushort_val;
				break;
			case 20 :
				pstTR->ucIpProto = tlv.uchar_val;
				break;
			case 21 :
				pstTR->uiProtoSig = tlv.uint_val;
				break;
			case 22 :
				pstTR->uiZoneID = tlv.uint_val;
				break;
			case 23 :
				pstTR->uiFlvID = tlv.uint_val;
				break;
			case 24 :
				pstTR->ucClose = tlv.uchar_val;
				break;
			default :
//				printf("%d th parameter is not unknown block field\n",i);
				break;
		}
	}

	return 0;
}

int Analyze_TLV(const char *payload, int type, PARA_VAL *tlv, int *pos)
{
	int paraLen = 0;
	char lenBuf[4] = {0,};

	switch(type)
	{
		case T_INT8:
		case T_UINT8:
		case T_BOOLEAN:
			paraLen = 1;
			break;
		case T_INT16:
		case T_UINT16:
			paraLen = 2;
			break;
		case T_INT32:
		case T_UINT32:
			paraLen = 4;
			break;
		case T_STRING:
				memset(lenBuf, 0x00, sizeof(lenBuf));
				memcpy(lenBuf, payload+*pos, 4);
				paraLen = TOULONG(lenBuf);
	//			printf("paraLen:%d\n", paraLen);
			break;
		default:
			break;
	}
	*pos += 4; // length field 4byte 건너뛰기. 
	if (paraLen > 0 && paraLen <= 256)
	{
		memcpy(tlv, payload+*pos, paraLen);
		*pos += paraLen;
	}
	else
	{
		if( type == T_STRING) 
		{
			dAppLog(LOG_WARN, " parameter string len Invalid...%d", paraLen);
			return 0;
		}
	}
#if 0 // jjinri 06.29
	switch(type)
	{
		case T_BOOLEAN:
//			printf("BOOLEAN Type:%d | len:%d | value:%d\n", type, paraLen, tlv->uchar_val);
			break;
		case T_INT8:
//			printf("INT8 Type:%d | len:%d | value:%d\n", type, paraLen, tlv->short_val);
			break;
		case T_UINT8:
//			printf("UINT8 Type:%d | len:%d | value:%u\n", type, paraLen, tlv->uchar_val);
			break;
		case T_INT16:
//			printf("INT8 Type:%d | len:%d | value:%d\n", type, paraLen, tlv->short_val);
			break;
		case T_UINT16:
//			printf("UINT16 Type:%d | len:%d | value:%u\n", type, paraLen, tlv->ushort_val);
			break;
		case T_INT32:
//			printf("INT32 Type:%d | len:%d | value:%d\n", type, paraLen, tlv->int_val);
			break;
		case T_UINT32:
//			printf("UINT32 Type:%d | len:%d | value:%u\n", type, paraLen, tlv->uint_val);
			break;
		case T_STRING:
//			printf("STRING Type:%d | len:%d | value:%s\n", type, paraLen, tlv->ucVal);
			break;
		default:
//			printf(">>>> INVALID VALUE TYPE : %d\n",type);
			break;
	}
#endif

	return paraLen;
}

int Insert_Block(RDR_BLOCK *pstBlock)
{
	char query[4096];

	dAppLog(LOG_INFO, "insert block start:%s\n", query);
	snprintf(query, sizeof(query), "INSERT INTO %s VALUES "
					"('%s', '%s', %d, %d,"
					"  %d,  %d, %d,"
					"  1,  %d, %d);", RPT_BLOCK_TBL, pstBlock->szSceIp, \
					pstBlock->ucSubscriberID, pstBlock->usPackageID, pstBlock->dServiceID, \
					pstBlock->dProtocolID, pstBlock->ucInitSide, pstBlock->ucBlkReason, \
					pstBlock->ucRedirect, pstBlock->uiTime);

	if (mysql_query (conn, query) != 0)
	{
//		printf("insert block fail:%s; err=%s\n", query, mysql_error(conn));
		dAppLog(LOG_INFO, "insert block fail:%s; err=%s\n", query, mysql_error(conn));
		return -1;
	}
	dAppLog(LOG_INFO, "insert block start:%s\n", query);
	return 0;
}

int Check_SmsSend(RDR_BLOCK *pstBlock)
{
	int ret = 0;
	ret = Check_Package(pstBlock->usPackageID);
	if (ret == 1) // 특정 패키지가 아니면 그대로 Insert
	{
		ret = SendSmsMsgQ(pstBlock); // SMPP 프로세스로 메시지 큐를 보낸다. 
		if (ret < 0 )
		{
			return -1;
		}
		else
			return 1;
	}

	return ret;
}

int Check_Package(int pkgId)
{
	if( ruleSetList[pkgId]	== 1 )
		return 1;
	else
	{
		dAppLog( LOG_DEBUG, "Check Package. Do not Send SMS For This Package.[%d]", pkgId);
		return 0;
	}
}

int readSmsFile()
{
	int fd = 0;
	int len = 0;
	char buf[160] = {0,};

	memset(stSms.smsMsg, 0x00, sizeof(stSms.smsMsg));
	if( (fd = open("/DSC/NEW/DATA/SMS_MSG.conf", O_RDONLY)) )
	{
		while( (len = read(fd, buf, sizeof(buf))) > 0 )
		{
			sprintf(stSms.smsMsg,"%s", buf);
			stSms.smsMsg[strlen(stSms.smsMsg)] = 0;
		}
	}
	else
	{
		dAppLog(LOG_INFO, "SMS_MSG.conf open fail");
		return -1;
	}
	close(fd);
	return 0;
}

int writeSmsFile()
{
	FILE *fp = NULL;

	if( (fp = fopen("/DSC/NEW/DATA/SMS_MSG.conf", "w")))
	{
		fprintf(fp, "%s", stSms.smsMsg);
	}
	else
	{
		dAppLog(LOG_INFO, "SMS_MSG.conf open fail");
		return -1;
	}
	fclose(fp);

	return 0;
}

int SendSmsMsgQ(RDR_BLOCK *pstBlock) // SMPP 프로세스로 메시지 큐를 보낸다. 
{
	GeneralQMsgType txGenQMsg;
	IxpcQMsgType    *txIxpcMsg;
    int         txLen;

/* DEL : by june, 2010-10-03
	rad_sess_key stRadKey;
	char *radIMSI;
 */

	memset(&txGenQMsg, 0x00, sizeof(GeneralQMsgType));

	txIxpcMsg = (IxpcQMsgType*)txGenQMsg.body;

	dAppLog(LOG_DEBUG,"SMS:%s, LEN:%d", stSms.smsMsg, strlen(stSms.smsMsg));	

	txGenQMsg.mtype = MTYPE_STATUS_REPORT;

	strcpy (txIxpcMsg->head.srcSysName, mySysName);
	strcpy (txIxpcMsg->head.srcAppName, myAppName);
	strcpy (txIxpcMsg->head.dstSysName, mySysName);
	strcpy (txIxpcMsg->head.dstAppName, "SMPP");

	stSms.sendFlag = 0;
	stSms.pkgID = pstBlock->usPackageID;
	stSms.sPBit	= g_stRule[stSms.pkgID].pBit;
	stSms.sHBit	= g_stRule[stSms.pkgID].hBit;
	stSms.blkTm = pstBlock->uiTime;
	memset(stSms.subsID, 0x00, SMS_MAX_SUBSID_LEN);
	if( !strncmp(pstBlock->ucSubscriberID, "N/A", 3) )
	{
/* DEL : by june, 2010-10-03
 * DESC: TRACE 에 사용되는 SESSION 정보 참조 파트 주석 처리
 */
#if 0
		stRadKey.mobIP = pstBlock->uiClientIP;
		radIMSI = find_imsi_rad_sess(&stRadKey);
		if( radIMSI == NULL )
		{
			dAppLog(LOG_CRI, "UNKNOWN IMSI: NOT FIND SESSION IMSI.");
			return -1;
		}
		else
		{
			dAppLog(LOG_DEBUG, "UNKNOWN IMSI: FIND SESSION->IMSI[%s]", radIMSI);
			strncpy(stSms.subsID, radIMSI, strlen(radIMSI));
		}
#else
		dAppLog(LOG_CRI, "UNKNOWN IMSI: N/A");
		return -1;
#endif
	}
	else
		strncpy(stSms.subsID, pstBlock->ucSubscriberID, strlen(pstBlock->ucSubscriberID));

	stSms.subsID[strlen(stSms.subsID)] = 0;

	sprintf(stSms.subsIP, "%s", g_CliIP);
	stSms.subsIP[strlen(stSms.subsIP)] = 0;

	txIxpcMsg->head.bodyLen = sizeof(stSms);

	txLen = sizeof(txIxpcMsg->head) + txIxpcMsg->head.bodyLen;

	dAppLog(LOG_DEBUG,"SMS:%s, LEN:%d", stSms.smsMsg, strlen(stSms.smsMsg), txLen);	
	if (memcpy ((void*)txIxpcMsg->body, &stSms, sizeof(stSms)) == NULL) {
		sprintf(trcBuf, "memcpy err = %s\n", strerror(errno));
		trclib_writeLogErr(FL,trcBuf);
		return -1;
	}

	if (msgsnd(dSmppQid, (void*)&txGenQMsg, txLen, IPC_NOWAIT) < 0)
	{
		dAppLog(LOG_CRI,"msgQ send fail...dSmppQid[%d], err[%d:%s]", dSmppQid, errno,strerror(errno));
		return -1;
	}
	else
	{
		dAppLog(LOG_DEBUG,"msgQ send success...To SMPP[%d]", dSmppQid);
		return 0;
	}
	return 0;
}

/* added by dcham(2011.03.15) for RDR data delete */
int rdrana_RDRDelete(void)
{
	char   query[2048] = {0,};
	time_t now_t,del_t, exc_t;

	del_t = time(&now_t) - RDR_DEL_POS;

	/* Delete Block RDR **/
	sprintf(query, "DELETE FROM %s WHERE END_TIME < %ld", "RPT_BLOCK", del_t);
	if (mysql_query (conn, query) != 0)
	{
		dAppLog(LOG_DEBUG, "Delete Block RDR Fail:%s; err=%s", query, mysql_error(conn));
		return -1;
	}
	time(&exc_t);
	dAppLog(LOG_DEBUG, "Delete Excute Time[%d], Block RDR Delete Time Condition : END_TIME < %ld", (exc_t-now_t), del_t);
	dAppLog(LOG_DEBUG, "Delete Block RDR block Success:%s", query);
	memset(query, 0x00, sizeof(query));

	del_t = time(&now_t) - RDR_DEL_POS;
	/* Delete Transaction RDR **/
	sprintf(query, "DELETE FROM %s WHERE END_TIME < %ld", "RPT_TR", del_t);
	if (mysql_query (conn, query) != 0)
	{
		dAppLog(LOG_DEBUG, "Delete Transaction RDR Fail:%s; err=%s", query, mysql_error(conn));
		return -1;
	}
	time(&exc_t);
	dAppLog(LOG_DEBUG, "Delete Excute Time[%d] Transaction RDR Delete Time Condition : END_TIME < %d", (exc_t-now_t), del_t);
	dAppLog(LOG_DEBUG, "Delete Transaction RDR block Success:%s", query);
	memset(query, 0x00, sizeof(query));

	/* Delete Link Usage RDR **/
	del_t = time(&now_t) - RDR_DEL_POS;
	sprintf(query, "DELETE FROM %s WHERE END_TIME < %ld", "RPT_LUR", del_t);
	if (mysql_query (conn, query) != 0)
	{
		dAppLog(LOG_DEBUG, "Delete Link Usage RDR Fail:%s; err=%s", query, mysql_error(conn));
		return -1;
	}   
	time(&exc_t);
	dAppLog(LOG_DEBUG, "Delete Excute Time[%d] Link Usage RDR Delete Time Condition : from_Time %d and To_Time %d", (exc_t-now_t), del_t);
	dAppLog(LOG_DEBUG, "Delete Link Usage RDR block Success:%s", query);

	return 0;
}

/* added by dcham(2011.03.15) for job period */
int isTimeToWork ()
{
	time_t      cur_time;
	struct tm  *cur_tMS;

	cur_time  = time (0);
	cur_tMS = (struct tm*)localtime((time_t*)&cur_time);

	// 현재의 분이 moduler 5연산으로 결과가 2,3이고, workFlag가 0일때 RDR delete작업 수행
	if ( ((cur_tMS->tm_min%STAT_UNIT) == 3 || (cur_tMS->tm_min%STAT_UNIT) == 4) && workFLAG == 0 )
	{
		workFLAG = 1;
		return 1;
	}
	else if ( ((cur_tMS->tm_min%STAT_UNIT) != 3 || (cur_tMS->tm_min%STAT_UNIT) != 4) )
	{
		workFLAG = 0;
		return -1;
	}
	else
	{
		return -1;
	}
	return -1;
}

int dReadFLTIDXFile(void)
{
	FILE *fa;
	char szBuf[1024];
	char szType[64];
	int i = 0, dIdx = 0;

	fa = fopen(DEF_NOTI_INDEX_FILE, "r");
	if(fa == NULL)
	{
		dAppLog(LOG_CRI,"dReadFLTIDXFile : %s FILE OPEN FAIL (%s)",
		DEF_NOTI_INDEX_FILE, strerror(errno));
		return -1;
	}

	while(fgets(szBuf,1024,fa) != NULL)
	{
		if(szBuf[0] != '#')
		{
			dAppLog(LOG_CRI,"dReadFLTIDXFile : %s File [%d] row format error",
			DEF_NOTI_INDEX_FILE, i);
			fclose(fa);
			return -1;
		}

		if(szBuf[1] == '#')
			continue;
		else if(szBuf[1] == 'E')
			break;
		else if(szBuf[1] == '@')
		{

			if(sscanf(&szBuf[2],"%s %d",szType, &dIdx) == 2)
			{
				if(strcmp(szType,"TRACE") == 0)
				{
					gpIdx->dTrcIdx = dIdx;
					
					gpCurTrc = (dIdx == 0) ? gpTrcList[0] : gpTrcList[1]; 
					dAppLog(LOG_CRI, "TRACE READ IDX[%d]", dIdx);
				}
				else if( strcmp(szType, "PDSN") == 0 )
				{
					gpIdx->dPdsnIdx = dIdx;
					dAppLog(LOG_CRI, "PDSN READ IDX[%d]", dIdx);
				}
				else if( strcmp(szType, "RULESET_LIST") == 0 )
				{
					gpIdx->dRsetListIdx = dIdx;
					dAppLog(LOG_CRI, "RULESET_LIST READ IDX[%d]", dIdx);
				}
				else if( strcmp(szType, "RULESET_USED") == 0 )
				{
					gpIdx->dRsetUsedIdx = dIdx;
					dAppLog(LOG_CRI, "RULESET_USED READ IDX[%d]", dIdx);
				}
				else if( strcmp(szType, "CPS") == 0 )
				{
					gpIdx->dCpsIdx = dIdx;
					dAppLog(LOG_CRI, "CPS OVLD CONTROL READ IDX[%d]", dIdx);
				}
				else if( strcmp(szType, "TIMEOUT") == 0 )
				{
					gpIdx->dTimeIdx = dIdx;
					dAppLog(LOG_CRI, "TIMEOUT READ IDX[%d]", dIdx);
				}
			}
		}
		dIdx = 0; i++;
	}

	fclose(fa);

	return i;
} 

void dSetCurTrace(NOTIFY_SIG *pNOTISIG)
{
	if( pNOTISIG->stNoti.dTrcIdx < 0 || pNOTISIG->stNoti.dTrcIdx >= DEF_SET_CNT )
	{
		dAppLog(LOG_CRI, "[%s.%d] Current Index Range ERROR dTrcIdx[%d]", __FUNCTION__, __LINE__, pNOTISIG->stNoti.dTrcIdx);
		return;
		//gpCurTrc = NULL;
	}

	gpIdx->dTrcIdx = pNOTISIG->stNoti.dTrcIdx;
	gpCurTrc = gpTrcList[gpIdx->dTrcIdx];

	dAppLog(LOG_CRI, "NOTI ->> TRACE ACTIVE IDX[%d]", pNOTISIG->stNoti.dTrcIdx);
}

