
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>

// LIB
#include "typedef.h"
#include "mems.h"
#include "cifo.h"
#include "nifo.h"
#include "gifo.h"
#include "hasho.h"			/* stHASHOINFO */
#include "timerN.h"			/* stTIMERNINFO */
#include "loglib.h"
#include "verlib.h"
#include "commdef.h"

#include "path.h"
#include "sshmid.h"
#include "procid.h"

#include "common_stg.h"

//.
#include "rppi_init.h"		/* dInitRPPI() */
#include "rppi_util.h"		/* dMakePCFHash(), dMakeModelHash(), dMakeDefectHash() */
#include "rppi_switch.h"	/* dSwitchMsg() */
#include "rppi_func.h"		/* GET_ISRECALL, dProcCallStop() */
#include "rppi_msgq.h"		/* dSendLogRPPI() */

#ifdef _RPPI_MULTI_
int			gProcNum;
UINT		guiSeqProcKey;
char		gszMyProc[32];
#endif
char        vERSION[7] = "R4.0.0";


stMEMSINFO		*pMEMSINFO;
stCIFO			*gpCIFO;
stHASHOINFO 	*pHASHOINFO;        
stHASHOINFO		*pPCFINFO;
stHASHOINFO		*pDEFECTINFO;
stHASHOINFO		*pMODELINFO;
stTIMERNINFO	*pTIMERNINFO;

int				giStopFlag;

S32 main(int argc, char **argv)
{
	S32			dRet;           /**< 함수 Return 값 */
	OFFSET		offset;
	U8		*pNode;
	U8		*pNextNode;
	U8		*p, *data;
	S32			type, len, ismalloc;
	LOG_SIGNAL      *pstSIGNAL;

#ifdef _RPPI_MULTI_
    
	char					szLOGPATH[128];
	int						dLen;


	memcpy(&gszMyProc[0], argv[0], strlen(argv[0]));

	dLen = strlen(gszMyProc);
	gProcNum = atoi(&gszMyProc[dLen-1]);
	guiSeqProcKey = SEQ_PROC_A_RPPI + gProcNum;
	sprintf(szLOGPATH, LOG_PATH"/%s", gszMyProc);

	/* log_print 초기화 */
    log_init(S_SSHM_LOG_LEVEL, getpid(), guiSeqProcKey, szLOGPATH, gszMyProc);
#else
	log_print(S_SSHM_LOG_LEVEL, getpid(), SEQ_PROC_A_RPPI, LOG_PATH"/A_RPPI", "A_RPPI");
#endif

	/* A_RPPI 초기화 */
	if((dRet = dInitRPPI()) < 0)
	{
		log_print(LOGN_CRI, "[%s][%s.%d] dInitRPPI dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
		exit(0);
	}

	/* Hash 초기화 */	
	dMakePCFHash();
	dMakeDefectHash();
	dMakeModelHash();

	if( (dRet = set_version(S_SSHM_VERSION, SEQ_PROC_A_RPPI + gProcNum, vERSION)) < 0) {
		log_print(LOGN_WARN,"MAIN : Failed in Initialize Version Info [%d]", dRet );
		exit(1);
	}

	log_print(LOGN_CRI, "START RPPI");

	/* MAIN LOOP */
	while(giStopFlag)
	{
		timerN_invoke(pTIMERNINFO);
		if((offset = gifo_read(pMEMSINFO, gpCIFO, guiSeqProcKey)) > 0) {

			/* DB LOG 전송을 목적으로 하는 NODE (삭제 하지 않고 전송하기 위함 )*/
			pNode = nifo_ptr(pMEMSINFO, offset);
			pNextNode = pNode;

			do {
				p = pNextNode;

				while(p != NULL) {
					if((dRet = nifo_read_tlv_cont(pMEMSINFO, pNextNode, (U32*)&type, (U32*)&len, &data, &ismalloc, &p)) < 0)
						break;

					dSwitchMsg(type, data);

					if(ismalloc == DEF_READ_MALLOC){ free(data); }
				}

				pNextNode = (U8 *)nifo_entry(nifo_ptr(pMEMSINFO, ((NIFO *)pNextNode)->nont.offset_next), NIFO, nont);

			} while(pNode != pNextNode);

			switch(type)
			{
            case START_CALL_NUM:
            case NOTIFY_SIG_DEF_NUM:
            case START_SERVICE_DEF_NUM:
            case LOG_DIALUP_SESS_DEF_NUM:
            case STOP_CALL_NUM:
            case LOG_DNS_DEF_NUM:
                log_print(LOGN_INFO, "DELETE LOGTYPE[%d]", type);
                nifo_node_delete(pMEMSINFO, pNode);
                break;
            case LOG_SIGNAL_DEF_NUM:
                pstSIGNAL = (LOG_SIGNAL*)data;
                if( pstSIGNAL->szIMSI[0] == 0 || strlen((char*)pstSIGNAL->szIMSI) == 0 ){
                    log_print(LOGN_INFO, "DELETE LOG_SIGNAL[SIGNAL] IMSI NULL");
                    nifo_node_delete(pMEMSINFO, pNode);
                    break;
                }
            case LOG_PAGE_TRANS_DEF_NUM:
            case LOG_VOD_SESS_DEF_NUM:
            case LOG_HTTP_TRANS_DEF_NUM:
            case LOG_TCP_SESS_DEF_NUM:
            case LOG_SIP_TRANS_DEF_NUM:
            case LOG_MSRP_TRANS_DEF_NUM:
            case LOG_VT_SESS_DEF_NUM:
            case LOG_IM_SESS_DEF_NUM:
            case LOG_FTP_DEF_NUM:
            case LOG_INET_DEF_NUM:
            case LOG_ITCP_SESS_DEF_NUM:
            case LOG_IHTTP_TRANS_DEF_NUM:
                if((dRet = gifo_write(pMEMSINFO, gpCIFO, guiSeqProcKey, SEQ_PROC_M_LOG, nifo_offset(pMEMSINFO, pNode))) < 0)
                {
                    log_print(LOGN_CRI, "[%s][%s.%d] gifo_wrtie dRet:[%d] LOGTYPE[%d]",
                            __FILE__, __FUNCTION__, __LINE__, dRet, type);
                    nifo_node_delete(pMEMSINFO, pNode);
                }
                break;
            case START_PI_DATA_RECALL_NUM: 
            case START_RP_DATA_RECALL_NUM: 
            case START_PI_SIG_RECALL_NUM:     
            case START_RP_SIG_RECALL_NUM:     
            case STOP_PI_RECALL_NUM:          
            case STOP_RP_RECALL_NUM:
                log_print(LOGN_INFO, "DELETE LOGTYPE[%d]", type);
                nifo_node_delete(pMEMSINFO, pNode);
                break;
            default:
                log_print(LOGN_CRI, "LOG SEND LOOP:UNKNOWN LOGTYPE[%d]", type);
                nifo_node_delete(pMEMSINFO, pNode);
                break;
			}
		} 
		else 
		{
			usleep(0);
		}
	}

	FinishProgram();

	return 0;
}

