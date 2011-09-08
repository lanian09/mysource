/*******************************************************************************
			DQMS Project

	Author   : Lee Dong-Hwan
	Section  : QMON
	SCCS ID  : @(#)qmon_main.c	1.1
	Date     : 03/22/04
	Revision History :
		'04.    03. 03. initial

	Description :

	Copyright (c) uPRESTO 2005
*******************************************************************************/

/***** A.1 * File Include *******************************/

/* SYS HEADER */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <sys/msg.h>
#include <string.h>
/* LIB HEADER */
#include "commdef.h"
#include "filelib.h"                                                                           
#include "filedb.h"                                                                        
#include "loglib.h"                                                                        
#include "ipclib.h"                                                                        
#include "verlib.h"
#include "mems.h"
#include "cifo.h"
#include "gifo.h"
#include "nifo.h"
/* PRO HEADER */
#include "path.h"
#include "sockio.h"
#include "msgdef.h"                                                                        
#include "mmcdef.h" 
#include "procid.h"
#include "sshmid.h"
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
#include "almstat.h"                                                                       
/* LOC HEADER */
#include "qmon_restart.h"
/** B. DEFINITION OF NEW CONSTANTS ********************************************/
#define MAX_QUEUE	300
#define MSG_MAX_FILE 		"/proc/sys/kernel/msgmax"
#define MSG_MAX_SIZE_FILE	"/proc/sys/kernel/msgmnb"

/** C. DEFINITION OF NEW TYPES ************************************************/

/** D. DECLARATION OF VARIABLES ***********************************************/
st_WNTAM *fidb;
extern int 	    errno;
extern stMEMSINFO    *gpRECVMEMS;
extern stCIFO        *gpCIFO;

st_QUEUE_INFO   *pstQueInfo;
st_SHM_INFO     *pstShmInfo;

int             gdQueCount = MAX_QUEUE;
int             *pdQInfo;

int 	        dStopFlag = 1;
int 	        Finishflag = 0;
int		        dQId[MAX_QUEUE];
int		        gdMsgMaxSize = 0;
int  			g_dCondQid;

char	        vERSION[7] = "R4.0.0";
char            gsQName[MAX_QUEUE][10];
unsigned char	gucSysNo;
/** E.1 DEFINITION OF FUNCTIONS ***********************************************/
//void InitMsgQ(void);
void FinishProgram(void);
void print_func(stMEMSINFO *gpRECVMEMS, stMEMSNODEHDR *pMEMSNODEHDR);
void InitMQ(void);
int SetUpSignal(void);
//int dCheckNIFOZONEStatus(void);
//int dCheckCIFOStatus(void);
//int dPrintNIFOZONEStatus(void);
//int dPrintCIFOStatus(void);

extern int log_write(char *fmt, ...);
extern int dAppWrite(int dLevel, char *szMsg);
extern int Init_GEN_INFO(void);
extern int dCheckCHSMDStatus(void);

/** E.2 DEFINITION OF FUNCTIONS ***********************************************/

/*******************************************************************************
 * MAIN FUNCTION
 *******************************************************************************/
int main(void)
{
	int		dRet, mems_check;
	long long   lldNifoMax, lldChanMax;
	time_t	tCheckTime, tPrintTime, tMaxTime, tCurTime, tCycleTime;

	log_init(S_SSHM_LOG_LEVEL, getpid(), SEQ_PROC_QMON, LOG_PATH"/QMON", "QMON");

	if((dRet = SetUpSignal()) < 0)
	{
		log_print( LOGN_CRI, "[ERROR] SetSignal");
		exit(0);
	}

	gpRECVMEMS = nifo_init_zone((unsigned char *)"QMON", SEQ_PROC_QMON, FILE_NIFO_ZONE);
	if( gpRECVMEMS == NULL ){
		log_print(LOGN_CRI, LH" FAILED IN nifo_init, NULL",  LT);
		exit(0);
	}

	gpCIFO = gifo_init_group(FILE_CIFO_CONF, FILE_GIFO_CONF);
	if( gpCIFO == NULL ){
		log_print(LOGN_CRI, LH" FAILED IN gifo_init_group. cifo=%s, gifo=%s",
				LT, FILE_CIFO_CONF, FILE_GIFO_CONF);
		exit(0);
	}

	if( (dRet = shm_init(S_SSHM_FIDB, DEF_WNTAM_SIZE, (void**)&fidb)) < 0 ){
		log_print(LOGN_CRI,LH" FAILED IN shm_init(S_SSHM_FIDB[0x%04X], dRet=%d)" EH,
				LT,S_SSHM_FIDB,dRet,ET);
		exit(0);
	}

	if( (dRet = set_version(S_SSHM_VERSION, SEQ_PROC_QMON, vERSION)) != 0)
		log_print( LOGN_CRI, "set_version error(ret=%d,idx=%d,ver=%s)", dRet,SEQ_PROC_QMON,vERSION);

	log_print(LOGN_CRI, "QMONITOR %s Started", vERSION);

	tCurTime = time(NULL);
	tCheckTime = tCurTime;
	tPrintTime = tCurTime;
	tCycleTime  = tCurTime;
	tMaxTime = 0;	
	mems_check = 0;

	lldNifoMax = 0;
	lldChanMax = 0;

	fidb->stNTAM.nifoCnt = gpRECVMEMS->uiZoneCnt + 1;
	fidb->stNTAM.chanCnt = gpCIFO->uiChCnt + 1;

	while(dStopFlag)
	{
		tCurTime = time(NULL);

		if(tCurTime >= tCheckTime + 2)
		{
			if( (dRet = dCheckCHSMDStatus()) < 0)
				log_print(LOGN_CRI,"FAILED IN RESTART CHSMD[%d]",dRet);
			else if(dRet == 1)
				log_print(LOGN_DEBUG, "SUCCESS RESTART CHSMD");
#if 0
			if( (dRet = dCheckNIFOZONEStatus()) < 0)
				log_print(LOGN_CRI, LH" ERROR IN dCheckNIFOStatus() dRet[%d]", LT, dRet);

			if((dRet = dCheckCIFOStatus()) < 0)
				log_print(LOGN_CRI, LH" ERROR IN dCheckCIFOStatus() dRet[%d]", LT, dRet);
#endif

			tCheckTime = tCurTime;
		}

		if(tCurTime >= (tCycleTime+10))
		{
#if 0
			if( (dRet = dPrintNIFOZONEStatus()) < 0)
				log_print(LOGN_CRI, LH" ERROR IN dPrintNIFOStatus() dRet[%d]", LT, dRet);

			if( (dRet = dPrintCIFOStatus()) < 0)
				log_print(LOGN_CRI, LH" ERROR IN dPrintCIFOStatus() dRet[%d]", LT, dRet);
			tCycleTime = tCurTime;
#endif
		}

		if(tCurTime >= tPrintTime + 60)
		{
			if( fidb->stNTAM.nifosts.llCur > lldNifoMax)
			{
				lldNifoMax  = fidb->stNTAM.nifosts.llCur;
				tMaxTime    = tCurTime;
			}
#if 0
			if( fidb->stNTAM.chansts.llCur > lldChanMax)
			{
				lldChanMax  = fidb->stNTAM.chansts.llCur;
				tCMaxTime   = tCurTime;
			}
#endif

			tPrintTime = tCurTime;
		}

		usleep(0);
	}
	FinishProgram();

	return 0;
}

/*******************************************************************************
 * SIGNAL FUNCTION
*******************************************************************************/
void UserControlledSignal(int sign)
{
    Finishflag = sign;
    dStopFlag = 0;
}

/*******************************************************************************
 * SIGNAL FUNCTION
*******************************************************************************/
void IgnoreSignal(int sign)
{
    if(sign != SIGALRM) {
    	log_print( LOGN_CRI, "UNWANTED SIGNAL IS RECEIVED, signal = %d", sign );
    }

    signal(sign, IgnoreSignal);
}

/*******************************************************************************
 * FINISH PROGRAM
*******************************************************************************/
void FinishProgram()
{
    log_print( LOGN_CRI, "PROGRAM IS NORMALLY TERMINATED, Cause = %d", Finishflag);

    exit(0);
}

int SetUpSignal(void)
{
	FILE	*fp;
	char	szBuffer[128];

	/* WANTED SIGNALS   */
	signal(SIGTERM, UserControlledSignal);
	signal(SIGINT,  UserControlledSignal);
	signal(SIGQUIT, UserControlledSignal);

	/* UNWANTED SIGNALS */
	signal(SIGHUP,  IgnoreSignal);
	signal(SIGALRM, IgnoreSignal);
	signal(SIGPIPE, IgnoreSignal);
	signal(SIGPOLL, IgnoreSignal);
	signal(SIGPROF, IgnoreSignal);
	signal(SIGUSR1, IgnoreSignal);
	signal(SIGUSR2, IgnoreSignal);
	signal(SIGVTALRM, IgnoreSignal);
	signal(SIGCLD, SIG_IGN);

	if( (fp = fopen(MSG_MAX_SIZE_FILE, "r")) == NULL)
	{
		log_print( LOGN_CRI, "[QUEUE ERROR] FILE OPEN [%s]", MSG_MAX_SIZE_FILE );
		return -1;
	}

	while(fgets(szBuffer, 128, fp) != NULL)
		sscanf( szBuffer, "%d", &gdMsgMaxSize );

	fclose( fp );
	return 1;
}

/*******************************************************************************
 *
*******************************************************************************/
void InitMQ(void)
{
	int		i, dID;

	for(i = 0; i < pstQueInfo->dQueCnt; i++)
	{
		if( ( dID = msgget(*((pstQueInfo->dQueList)+i), 0666|IPC_CREAT)) < 0)
		{
			log_print(LOGN_CRI, "[ERROR] QUEUE MSGGET FAIL [%s] [0x%x] [%s]", pstQueInfo->sQueName[i], *((pstQueInfo->dQueList)+i), strerror(errno));
			continue;
		}
		else
		{
			dQId[i] = dID;
			log_print(LOGN_DEBUG, "[%10s]: QID[%d][0x%x]", pstQueInfo->sQueName[i], dID, *((pstQueInfo->dQueList)+i));
		}
	}
}

/*******************************************************************************
 *
*******************************************************************************/

#if 0
void print_func(stMEMSINFO *gpRECVMEMS, stMEMSNODEHDR *pMEMSNODEHDR)
{
    unsigned char *pNode;
    unsigned int  dRet;
    char *pNextNode;
    char *p;
	char *data;
	int  type, len;
	int ismalloc;
	LOG_RPPI	*pLOGRPPI;

    pNode = (unsigned char *)pMEMSNODEHDR + stMEMSNODEHDR_SIZE;

    pNextNode = (char *)pNode;

    do {
        p = pNextNode;

        while(p != NULL) {
            if((dRet = nifo_read_tlv_cont(gpRECVMEMS, pNextNode, &type, &len, &data, &ismalloc, &p)) < 0)
                break;

            log_print(LOGN_CRI, "####################################################################");
            log_print(LOGN_CRI, "TYPE[%d][%s] LEN[%d] ISMALLOC[%s]", type,
                ((type==START_CALL_NUM || type==STOP_CALL_NUM || type==RADIUS_START_NUM ||
                type==LOG_PISIGNAL_DEF_NUM) ? PRINT_TAG_DEF_ALL_CALL_INPUT(type) :
                PRINT_DEF_NUM_table_log(type)), len, (ismalloc == DEF_READ_MALLOC) ? "MALLOC MEM" : "ORIGIN MEM");
            switch(type)
            {
			case LOG_RPPI_DEF_NUM:
				pLOGRPPI = (LOG_RPPI *)data;
				log_print(LOGN_CRI, "IMSI=%s CallTime=%u.%u", pLOGRPPI->szIMSI, pLOGRPPI->uiCallTime, pLOGRPPI->uiCallMTime);
				break;
            default:
                break;
            }
        }

        pNextNode = (unsigned char *)nifo_entry(nifo_ptr(gpRECVMEMS, ((NIFO *)pNextNode)->nont.offset_next), NIFO, nont);

    } while(pNode != pNextNode);

}
#endif
#if 0
int dCheckNIFOZONEStatus(void)
{
	int             i;
	float           fPercent;
	long long       llCurNIFO, llMaxNIFO;
	unsigned int    uiAllocCnt = 0, uiTotCnt = 0;

	for(i=0;i<gpRECVMEMS->uiZoneCnt;i++)
	{
		llCurNIFO   = mems_alloced_cnt(gpRECVMEMS, i);
		llMaxNIFO   = gpRECVMEMS->stMEMSZONE[i].uiMemNodeTotCnt;

		if( (fPercent = ((float)llCurNIFO/(float)llMaxNIFO) * 100.0) >= 90.0)
		{
			log_print(LOGN_CRI, "ZoneID[%d] fPercent[%f] llCurNIFO[%lld] llMaxNIFO[%lld]",
					i, fPercent, llCurNIFO, llMaxNIFO);
		}
		fidb->stNTAM.nifosts[i+1].llCur = llCurNIFO;
		fidb->stNTAM.nifosts[i+1].lMax = llMaxNIFO;
		uiAllocCnt += llCurNIFO;
		uiTotCnt += llMaxNIFO;
	}
	fidb->stNTAM.nifosts[0].llCur  = uiAllocCnt;
	fidb->stNTAM.nifosts[0].lMax   = uiTotCnt;

	return 0;
}
#endif
#if 0
int dCheckCIFOStatus(void)
{
	int     i;
	float   fPercent;
	U32     uiUsedCnt, uiMaxCnt, uiAllUsed = 0, uiAllMax = 0;
	for(i=0;i<gpCIFO->uiChCnt;i++)
	{
		uiUsedCnt = cifo_used_count(gpCIFO, i);
		uiMaxCnt = cifo_max_count(gpCIFO, i);
		if( (fPercent = ((float)uiUsedCnt/(float)uiMaxCnt) * 100.0) >= 90.0)
		{
			log_print(LOGN_CRI, "ChannelID[%d] fPercent[%f] usedCellCnt[%u] maxCellCnt[%u]",
					i, fPercent, uiUsedCnt, uiMaxCnt);
		}

		fidb->stNTAM.chansts[i+1].llCur = uiUsedCnt;
		fidb->stNTAM.chansts[i+1].lMax = uiMaxCnt;
		uiAllUsed += uiUsedCnt;
		uiAllMax += uiMaxCnt;
	}

	fidb->stNTAM.chansts[0].llCur = uiAllUsed;
	fidb->stNTAM.chansts[0].lMax = uiAllMax;

	return 0;
}
#endif


#if 0 
int dPrintNIFOZONEStatus(void)
{
	int             i;
	float           fPercent;
	long long       llCurNIFO, llMaxNIFO;
	unsigned int    uiAllocCnt = 0, uiTotCnt = 0;
	for(i=0;i<gpRECVMEMS->uiZoneCnt;i++)
	{
		llCurNIFO   = mems_alloced_cnt(gpRECVMEMS, i);
		llMaxNIFO   = gpRECVMEMS->stMEMSZONE[i].uiMemNodeTotCnt;

		fPercent = ((float)llCurNIFO/(float)llMaxNIFO) * 100.0;
		log_print(LOGN_CRI, "ZoneID[%d] fPercent[%f] llCurNIFO[%lld] llMaxNIFO[%lld]",
				i, fPercent, llCurNIFO, llMaxNIFO);
		uiAllocCnt += llCurNIFO;
		uiTotCnt += llMaxNIFO;
	}

	fPercent = ((float)uiAllocCnt/(float)uiTotCnt) * 100.0;
	log_print(LOGN_CRI, "NIFO_ALL fPercent[%f] CurNIFO[%lld] MaxNIFO[%lld]",
			fPercent, uiAllocCnt, uiTotCnt);
	return 0;
}
#endif
#if 0
int dPrintCIFOStatus(void)
{
	int     i;
	float   fPercent;
	U32     uiUsedCnt, uiMaxCnt, uiAllUsed = 0, uiAllMax = 0;

	for(i=0;i<gpCIFO->uiChCnt;i++)
	{
		uiUsedCnt = cifo_used_count(gpCIFO, i);
		uiMaxCnt = cifo_max_count(gpCIFO, i);
		fPercent = ((float)uiUsedCnt/(float)uiMaxCnt) * 100.0;
		log_print(LOGN_CRI, "ChannelID[%d] fPercent[%f] usedCellCnt[%u] maxCellCnt[%u]",
				i, fPercent, uiUsedCnt, uiMaxCnt);

		uiAllUsed += uiUsedCnt;
		uiAllMax += uiMaxCnt;
	}
	fPercent = ((float)uiAllUsed/(float)uiAllMax) * 100.0;

	log_print(LOGN_CRI, "CIFO_ALL fPercent[%f] usedCellCnt[%lld] maxCellCnt[%lld]",
			fPercent, uiAllUsed, uiAllMax);

	return 0;
}
#endif
