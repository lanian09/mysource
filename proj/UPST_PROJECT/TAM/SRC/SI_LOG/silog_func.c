
/** A.1* FILE INCLUSION ***********************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/types.h>

#include "common_stg.h"
#include "procid.h"
#include "mems.h"
#include "cifo.h"
#include "nifo.h"
#include "gifo.h"

#include "msgdef.h"

#include "sockio.h"
#include "loglib.h"
#include "nsocklib.h"

/** B.1* DEFINITION OF NEW CONSTANTS **********************/
/** C.1* DEFINITION OF NEW TYPES **************************/
/** D.1* DECLARATION OF VARIABLES *************************/
st_subsys_mng			*stSubSys;
int						gARPPICnt;
/** D.1* DECLARATION OF EXTERN VARIABLES ******************/
extern stMEMSINFO		*pMEMSINFO;
extern stCIFO			*gpCIFO;

/** E.1* DEFINITION OF FUNCTIONS **************************/
/** E.2* DEFINITION OF FUNCTIONS **************************/


/** 
 * ci_log로 부터 받은 메시지를 처리한다.
 * 
 * @param dLen 
 * @param szBuf 
 * @param pstHeader 
 * @param dSubNo 
 * 
 * @return 
 */
int dHandle_RecvMsg(int dLen, char *szBuf, pst_NTAFTHeader pstHeader, int dSubNo)
{
    int                 	dRet, dProcID, type, size;
	U8						*pNODE, *pDATA;
	LOG_COMMON				*pLOG;

#ifdef _RPPI_MULTI_
    int						dIMSI; /**< IMSI의 뒤에 4자리가 들어갈 변수 */
#endif

    if(pstHeader->llMagicNumber != MAGIC_NUMBER) {
        log_print(LOGN_CRI, "dSendToProc : MAGIC NUMBER ERROR, Will be Reset %llu!=%llu",
			pstHeader->llMagicNumber, MAGIC_NUMBER);
        return -1;
    } /* end of if */

    switch(pstHeader->ucSvcID)
    {
    case SID_LOG:
		if(dSubNo != pstHeader->ucNTAFID && dSubNo != 0) {	
        	log_print(LOGN_WARN, "MSGRCV SVCID=%d MSGID=%d:%s SYSNO=%d:MEM=%d", 
				pstHeader->ucSvcID, pstHeader->ucMsgID, PRINT_DEF_NUM_table_log(pstHeader->ucMsgID), 
				pstHeader->ucNTAFID, dSubNo);
		} else {
        	log_print(LOGN_INFO, "MSGRCV SVCID=%d MSGID=%d:%s SYSNO=%d:MEM=%d", 
				pstHeader->ucSvcID, pstHeader->ucMsgID, 
				(pstHeader->ucMsgID == START_CALL_NUM || pstHeader->ucMsgID == STOP_CALL_NUM 
				|| pstHeader->ucMsgID == START_SERVICE_DEF_NUM) 
				? PRINT_TAG_DEF_ALL_CALL_INPUT(pstHeader->ucMsgID) : PRINT_DEF_NUM_table_log(pstHeader->ucMsgID), 
				pstHeader->ucNTAFID, dSubNo);
		}

		dProcID = SEQ_PROC_A_RPPI; 

		switch(pstHeader->ucMsgID) {
		case START_CALL_NUM:
			type = START_CALL_NUM;
			size = LOG_SIGNAL_SIZE;
			break;
		case START_SERVICE_DEF_NUM:
			type = START_SERVICE_DEF_NUM;
			size = LOG_COMMON_SIZE;
			break;
		case STOP_CALL_NUM:
			type = STOP_CALL_NUM;
			size = LOG_SIGNAL_SIZE;
			break;
		case LOG_DIALUP_SESS_DEF_NUM:
			type = LOG_DIALUP_SESS_DEF_NUM;
			size = LOG_DIALUP_SESS_SIZE;
			break;
		case LOG_SIGNAL_DEF_NUM:
			type = LOG_SIGNAL_DEF_NUM;
			size = LOG_SIGNAL_SIZE;
			break;
		case LOG_TCP_SESS_DEF_NUM:
			type = LOG_TCP_SESS_DEF_NUM;
			size = LOG_TCP_SESS_SIZE;
			break;
        case LOG_HTTP_TRANS_DEF_NUM:
			type = LOG_HTTP_TRANS_DEF_NUM;
			size = LOG_HTTP_TRANS_SIZE;
			break;
        case LOG_RTSP_TRANS_DEF_NUM:
			type = LOG_HTTP_TRANS_DEF_NUM;
			size = LOG_HTTP_TRANS_SIZE;
			break;
        case LOG_PAGE_TRANS_DEF_NUM:
			type = LOG_PAGE_TRANS_DEF_NUM;
			size = LOG_PAGE_TRANS_SIZE;
			break;
        case LOG_ONLINE_TRANS_DEF_NUM:
			type = LOG_ONLINE_TRANS_DEF_NUM;
			size = LOG_ONLINE_TRANS_SIZE;
			break;
        case LOG_JNC_TRANS_DEF_NUM:
			type = LOG_JNC_TRANS_DEF_NUM;
			size = LOG_JNC_TRANS_SIZE;
			break;
        case LOG_IV_DEF_NUM:
			type = LOG_IV_DEF_NUM;
			size = LOG_IV_SIZE;
			break;
        case LOG_VOD_SESS_DEF_NUM:
			type = LOG_VOD_SESS_DEF_NUM;
			size = LOG_VOD_SESS_SIZE;
			break;
		case LOG_SIP_TRANS_DEF_NUM:  		
			type = LOG_SIP_TRANS_DEF_NUM;
			size = LOG_SIP_TRANS_SIZE;
			break;
		case LOG_MSRP_TRANS_DEF_NUM:
			type = LOG_MSRP_TRANS_DEF_NUM;
			size = LOG_MSRP_TRANS_SIZE;
			break;
		case LOG_VT_SESS_DEF_NUM:
			type = LOG_VT_SESS_DEF_NUM;
			size = LOG_VT_SESS_SIZE;
			break;
		case LOG_IM_SESS_DEF_NUM:
			type = LOG_IM_SESS_DEF_NUM;
			size = LOG_IM_SESS_SIZE;
			break;
		case LOG_FTP_DEF_NUM: 
			type = LOG_FTP_DEF_NUM;
			size = LOG_FTP_SIZE;
			break;
		case st_TraceMsgHdr_DEF_NUM: 
			dProcID = SEQ_PROC_M_TRACE;
			type = st_TraceMsgHdr_DEF_NUM;
			size = pstHeader->usBodyLen;
			break;
		case LOG_DNS_DEF_NUM: 
			type = LOG_DNS_DEF_NUM;
			size = LOG_DNS_SIZE;
			break;
		case LOG_INET_DEF_NUM: 
			type = LOG_INET_DEF_NUM;
			size = LOG_INET_SIZE;
			break;
		case LOG_ITCP_SESS_DEF_NUM: 
			type = LOG_ITCP_SESS_DEF_NUM;
			size = LOG_ITCP_SESS_SIZE;
			break;
		case LOG_IHTTP_TRANS_DEF_NUM: 
			type = LOG_IHTTP_TRANS_DEF_NUM;
			size = LOG_IHTTP_TRANS_SIZE;
			break;
		case START_PI_DATA_RECALL_NUM:
			type = START_PI_DATA_RECALL_NUM;
			size = LOG_SIGNAL_SIZE;
			break;
		case START_RP_DATA_RECALL_NUM:
			type = START_RP_DATA_RECALL_NUM;
			size = LOG_SIGNAL_SIZE;
			break;
		case START_PI_SIG_RECALL_NUM:
			type = START_PI_SIG_RECALL_NUM;
			size = LOG_SIGNAL_SIZE;
			break;
		case START_RP_SIG_RECALL_NUM:
			type = START_RP_SIG_RECALL_NUM;
			size = LOG_SIGNAL_SIZE;
			break;
		case STOP_PI_RECALL_NUM:
			type = STOP_PI_RECALL_NUM;
			size = LOG_SIGNAL_SIZE;
			break;
		case STOP_RP_RECALL_NUM:
			type = STOP_RP_RECALL_NUM;
			size = LOG_SIGNAL_SIZE;
			break;
		default :
            log_print(LOGN_CRI, "dSendToProc ERROR : INVALID MSG ID [MID]:[%d] ", pstHeader->ucMsgID);
			return 0;
        }

		if(pstHeader->usBodyLen != size) {
			log_print(LOGN_CRI, "F=%s:%s.%d DIFF SIZE TYPE=%d:%s RCV=%d ORG=%d", 
					__FILE__, __FUNCTION__, __LINE__, type, PRINT_DEF_NUM_table_log(type), pstHeader->usBodyLen, size);
			return 0;
		}

		if((pNODE = nifo_node_alloc(pMEMSINFO)) == NULL) {
			log_print(LOGN_CRI, "F=%s:%s.%d nifo_node_alloc NULL TYPE=%d:%s", 
					__FILE__, __FUNCTION__, __LINE__, type, PRINT_DEF_NUM_table_log(type));
			return 0;
		}
		
		if((pDATA = nifo_tlv_alloc(pMEMSINFO, pNODE, type, size, DEF_MEMSET_OFF)) == NULL) {
			log_print(LOGN_CRI, "F=%s:%s.%d nifo_tlv_alloc NULL TYPE=%d:%s", 
					__FILE__, __FUNCTION__, __LINE__, type, PRINT_DEF_NUM_table_log(type));
			nifo_node_delete(pMEMSINFO, pNODE);
			return 0;
		}

		memcpy(pDATA, szBuf+NTAFT_HEADER_LEN, size);

		if(dProcID == SEQ_PROC_A_RPPI) {
            /**
             * A_ROAM으로 분기하는 부분
             * A_RPPI 멀티시 default 에서 qid를 변경하는 것으로 간단히 처리하겠다.
             */
			pLOG = (LOG_COMMON *)pDATA;
			switch(pLOG->ucBranchID)
			{
			case TYPE_LNS:
			case TYPE_LAC:
			case TYPE_CRX:
            case TYPE_PDIF:
				log_print(LOGN_INFO, "RCV ROAM DATA BRANCHID=%d", pLOG->ucBranchID);
				dProcID = SEQ_PROC_A_ROAM;
				break;
			default:
#ifdef _RPPI_MULTI_
                /**
                 * TODO
                 * IMSI의 뒤에 4자리를 숫자로 변경한후 RPPI의 숫자 만큼 mod한다.
                 * mod수를 배열의 index로 참조한 RPPI Queue ID로 node를 write 하겠다.
                 * IMSI 배열의 SIZE는 16이며 define명은 MAX_MIN_SIZE 이다.
                 * 실제 IMSI의 String size는 15이다.
                 * IMSI의 11번째 index부터 atoi로 넘겨서 IMSI를 Number로 변경하겠다.
                 */
                dIMSI = atoi((char*)&pLOG->szIMSI[11]);
                dProcID = SEQ_PROC_A_RPPI0 + (dIMSI % gARPPICnt);
				log_print(LOGN_INFO, "Send ProcID:%d Index:%d IMSI:%s dIMSI:%d gARPPICnt:%d", dProcID, (dIMSI%gARPPICnt), pLOG->szIMSI, dIMSI, gARPPICnt);
                if(dProcID == -1)
                {
                    log_print(LOGN_CRI, LH" *** CAN'T EVENT *** PROCID=%d, IMSI=%d, A_RPPICnt=%d",
						LT, dProcID, dIMSI, gARPPICnt);
                    nifo_node_delete(pMEMSINFO, pNODE);
                    return 0;
                }
#endif                
				log_print(LOGN_INFO, "RCV DATA BRANCHID=%d", pLOG->ucBranchID);
				break;
			}
		}
		
		if((dRet = gifo_write(pMEMSINFO, gpCIFO, SEQ_PROC_SI_LOG, dProcID, nifo_offset(pMEMSINFO, pNODE))) < 0) {
			log_print(LOGN_CRI, "F=%s:%s.%d gifo_write dRet=%d TYPE=%d:%s",
					__FILE__, __FUNCTION__, __LINE__, dRet, type, PRINT_DEF_NUM_table_log(type));
			nifo_node_delete(pMEMSINFO, pNODE);
			return 0;
		}

        break;
    default:
        log_print(LOGN_CRI, "dSendToProc : SVCID=%d MSGID=%d Not Support Msg", 
			pstHeader->ucSvcID, pstHeader->ucMsgID);
        break;
    } /* end of switch */

    return 0;
} /* end of dSendToProc */

int dGetNtafNo(stNetTuple *stNet, int dIdx)
{
    int     i;
    UINT    uiIP;

    uiIP = stNet[dIdx].uiIP;

    for(i = 0; i < MAX_RECORD; i++)
    {
        if(stSubSys->sys[i].uiIP == uiIP)
            return  stSubSys->sys[i].usSysNo;
    }

    return -1;
}
