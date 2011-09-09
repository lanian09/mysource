
#include "rppi_func.h"

#include "common_stg.h"
#include "typedef.h"
#include "loglib.h"


void dSwitchMsg(S32 type, U8* data)
{
	switch(type)
	{
		/* RP A11 Start */
		case START_CALL_NUM:
			dCallStartInfo(type, data);
          	log_print( LOGN_INFO, "CALL START MESSAGE END");
			break;
		case STOP_CALL_NUM:
			dCallStopInfo(type, data);
			log_print(LOGN_INFO, "CALL STOP MESSAGE END");
			break;
		case LOG_SIGNAL_DEF_NUM:
			dSignalInfo(data);
			log_print( LOGN_INFO, "SIGNAL DATA MESSAGE END");
			break;

		/* RECALL Start*/
		case START_PI_DATA_RECALL_NUM:
			dCallStartInfo(type, data);
          	log_print( LOGN_INFO, "RECALL START MESSAGE END");
			break;
		case START_RP_DATA_RECALL_NUM:
			dReCallSignalInfo(type, data);
          	log_print( LOGN_INFO, "RECALL RP_DATA MESSAGE END");
			break;
		case START_RP_SIG_RECALL_NUM:
			dReCallSignalInfo(type, data);
          	log_print( LOGN_INFO, "RECALL RP_SIG MESSAGE END");
			break;
		case START_PI_SIG_RECALL_NUM:
			dReCallSignalInfo(type, data);
          	log_print( LOGN_INFO, "RECALL PI_SIG MESSAGE END");
			break;
		case STOP_RP_RECALL_NUM:
			dCallStopInfo(type, data);
          	log_print( LOGN_INFO, "RECALL STOP_RP MESSAGE END");
			break;
		case STOP_PI_RECALL_NUM:
			dCallStopInfo(type, data);
          	log_print( LOGN_INFO, "RECALL STOP_PI MESSAGE END");
			break;
		/* RECALL End*/

		case LOG_PAGE_TRANS_DEF_NUM:
			dPAGESessInfo(data);
//			LOG_PAGE_TRANS_Prt("PRINT LOG_PAGE", (LOG_PAGE_TRANS*)data);
			log_print( LOGN_INFO, "PAGE DATA MESSAGE END");
			break;
		case LOG_VOD_SESS_DEF_NUM:
			dVODSessInfo(data);
//			LOG_VOD_SESS_Prt("PRINT LOG_VOD", (LOG_VOD_SESS*)data);
			log_print( LOGN_INFO, "VOD DATA MESSAGE END");
			break;
		case LOG_HTTP_TRANS_DEF_NUM:
			dHTTPSessInfo(data);
//			LOG_HTTP_TRANS_Prt("PRINT LOG_HTTP", (LOG_HTTP_TRANS*)data);
			log_print( LOGN_INFO, "HTTP DATA MESSAGE END");
			break;
		case LOG_TCP_SESS_DEF_NUM:
			dTCPSessInfo(data);
//			LOG_TCP_SESS_Prt("PRINT LOG_TCP", (LOG_TCP_SESS*)data);
			log_print( LOGN_INFO, "TCP DATA MESSAGE END");
			break;

		case LOG_SIP_TRANS_DEF_NUM:
			dSIPSessInfo(data);
//			LOG_SIP_TRANS_Prt("PRINT LOG_SIP", (LOG_SIP_TRANS*)data);
			log_print( LOGN_INFO, "SIP DATA MESSAGE END");
			break;
		case LOG_MSRP_TRANS_DEF_NUM:
			dMSRPSessInfo(data);
//			LOG_MSRP_TRANS_Prt("PRINT LOG_MSRP", (LOG_MSRP_TRANS*)data);
			log_print( LOGN_INFO, "MSRP DATA MESSAGE END");
			break;
		case LOG_VT_SESS_DEF_NUM:
			dVTSessInfo(data);
//			LOG_VT_SESS_Prt("PRINT LOG_VT", (LOG_VT_SESS*)data);
			log_print( LOGN_INFO, "VT DATA MESSAGE END");
			break;
		case LOG_IM_SESS_DEF_NUM:
			dIMSessInfo(data);
//			LOG_IM_SESS_Prt("PRINT LOG_IM", (LOG_IM_SESS*)data);
			log_print( LOGN_INFO, "IM DATA MESSAGE END");
			break;
		case LOG_FTP_DEF_NUM:
			dFTPSessInfo(data);
//			LOG_FTP_Prt("PRINT LOG_FTP", (LOG_FTP*)data);
			log_print( LOGN_INFO, "FTP DATA MESSAGE END");
			break;
		case LOG_DIALUP_SESS_DEF_NUM:
			dDIALUPSessInfo(data);
//			LOG_DIALUP_SESS_Prt("PRINT LOG_DIALUP", (LOG_DIALUP_SESS*)data);
			log_print( LOGN_INFO, "DIALUP DATA MESSAGE END");
			break;
		case NOTIFY_SIG_DEF_NUM:
			dNotiSigInfo(data);
			log_print(LOGN_INFO, "FILETER MESSAGE END");
			break;	
		case START_SERVICE_DEF_NUM:
			dStartServiceInfo(data);
			log_print(LOGN_INFO, "START SERVICE MESSAGE END");
			break;
		case LOG_DNS_DEF_NUM:
			dDNSSessInfo(data);
//			LOG_DNS_Prt("PRINT LOG_DNS", (LOG_DNS *)data);
			log_print(LOGN_INFO, "DNS DATA MESSAGE END");
			break;	
		case LOG_INET_DEF_NUM:
			dINETSessInfo(data);
//			LOG_INET_Prt("PRINT LOG_INET", (LOG_INET*)data);
			log_print( LOGN_INFO, "INET DATA MESSAGE END");
			break;
		case LOG_ITCP_SESS_DEF_NUM:
			dITCPSessInfo(data);
//			LOG_TCP_SESS_Prt("PRINT LOG_ITCP", (LOG_ITCP_SESS*)data);
			log_print( LOGN_INFO, "ITCP DATA MESSAGE END");
			break;
		case LOG_IHTTP_TRANS_DEF_NUM:
			dIHTTPSessInfo(data);
//			LOG_TCP_SESS_Prt("PRINT LOG_IHTTP", (LOG_IHTTP_TRANS*)data);
			log_print( LOGN_INFO, "IHTTP DATA MESSAGE END");
			break;
		default:
			log_print(LOGN_WARN, "TYPE SWITCH : UNKNOWN TYPE[%d]", type);
			break;
	}
}
