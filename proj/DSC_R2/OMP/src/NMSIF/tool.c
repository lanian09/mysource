#include <nmsif_proto.h>
#include <nmsif.h>

char		*int2dot (int);
struct tm	*now ();

HoldFList	hold_list[FILE_NUM_5MIN+FILE_NUM_HOUR];

extern	ListenInfo	ne_info;
extern	SFM_sfdb	*sfdb;

extern	MYSQL	*conn;
extern	char	traceBuf[];
extern	int		trcFlag, trcLogFlag;

int get_alm_code (int atype)
{
    switch (atype)
	{
		case SFM_ALM_TYPE_CPU_USAGE				:	return ALMCODE_SFM_CPU_USAGE;
		case SFM_ALM_TYPE_MEMORY_USAGE			:	return ALMCODE_SFM_MEMORY_USAGE;
		case SFM_ALM_TYPE_DISK_USAGE			:	return ALMCODE_SFM_DISK_USAGE;
		case SFM_ALM_TYPE_LAN					:	return ALMCODE_SFM_LAN;
		case SFM_ALM_TYPE_PROC					:	return ALMCODE_SFM_PROCESS;
		case SFM_ALM_TYPE_LINK					:	return ALMCODE_SFM_LINK;
		case SFM_ALM_TYPE_MP_HW					:	return ALMCODE_SFM_HPUX_HW;
	//	case SFM_ALM_TYPE_CONN_SERVER			:	return ALMCODE_SFM_SERVER_CONN;
		case SFM_ALM_TYPE_CALL_INFO				:	return ALMCODE_SFM_CALL_INFO;
		case SFM_ALM_TYPE_DUP_STS				:	return ALMCODE_SFM_DUPLICATION;
		case SFM_ALM_TYPE_DUP_HEARTBEAT			:	return ALMCODE_SFM_HEARTBEAT;
		case SFM_ALM_TYPE_DUP_OOS				:	return ALMCODE_SFM_OOS;
		case SFM_ALM_TYPE_SUCC_RATE				:	return ALMCODE_SFM_SUCCESS_RATE;
		case SFM_ALM_TYPE_SESS_LOAD				:	return ALMCODE_SFM_SESS_LOAD;
		case SFM_ALM_TYPE_DBCON_STST			:	return ALMCODE_SFM_RMT_DB_STS;
		case SFM_ALM_TYPE_RMT_LAN				:	return ALMCODE_SFM_RMT_LAN;
		case SFM_ALM_TYPE_OPT_LAN				:	return ALMCODE_SFM_OPT_LAN;
		case SFM_ALM_TYPE_HWNTP					:	return ALMCODE_SFM_HWNTP;
		//case SFM_ALM_TYPE_PD_CPU_USAGE			:	return ALMCODE_SFM_PD_CPU_USAGE;
		//case SFM_ALM_TYPE_PD_MEMORY_USAGE		:	return ALMCODE_SFM_PD_MEMORY_USAGE;
		//case SFM_ALM_TYPE_PD_FAN_STS			:	return ALMCODE_SFM_PD_FAN_USAGE;
		//case SFM_ALM_TYPE_PD_GIGA_LAN			:	return ALMCODE_SFM_PD_GIGA_USAGE;
		case SFM_ALM_TYPE_TAP_CPU_USAGE			:	return ALMCODE_SFM_TAP_CPU_USAGE;
		case SFM_ALM_TYPE_TAP_MEMORY_USAGE		:	return ALMCODE_SFM_TAP_MEMORY_USAGE;
		case SFM_ALM_TYPE_TAP_FAN_STS			:	return ALMCODE_SFM_TAP_FAN_USAGE;
		case SFM_ALM_TYPE_TAP_PORT_STS			:	return ALMCODE_SFM_TAP_PORT_STS;
		case SFM_ALM_TYPE_TAP_POWER_STS         :   return ALMCODE_SFM_TAP_POWER_STS; // 20110422 by dcham
		case SFM_ALM_TYPE_RSRC_LOAD				:	return ALMCODE_SFM_RSRC_LOAD;
		case SFM_ALM_TYPE_QUEUE_LOAD			:	return ALMCODE_SFM_QUEUE_LOAD;
		case SFM_ALM_TYPE_NMSIF_CONNECT			:	return ALMCODE_NMS_CONNECT;
		case SFM_ALM_TYPE_DUAL_ACT				:	return ALMCODE_SFM_DUAL_ACT;
		case SFM_ALM_TYPE_DUAL_STD				:	return ALMCODE_SFM_DUAL_STD;
		case SFM_ALM_TYPE_DUAL_STS_QRY_TIME_OUT	:	return ALMCODE_SFM_DUAL_STS_QRY_TIME_OUT;
		case SFM_ALM_TYPE_SCE_CPU				:	return ALMCODE_SFM_SCE_CPU_USAGE;
		case SFM_ALM_TYPE_SCE_MEM				:	return ALMCODE_SFM_SCE_MEM_USAGE;
		case SFM_ALM_TYPE_SCE_DISK				:	return ALMCODE_SFM_SCE_DISK_USAGE;
		case SFM_ALM_TYPE_SCE_PWR				:	return ALMCODE_SFM_SCE_PWR_USAGE;
		case SFM_ALM_TYPE_SCE_FAN				:	return ALMCODE_SFM_SCE_FAN_USAGE;
		case SFM_ALM_TYPE_SCE_TEMP				:	return ALMCODE_SFM_SCE_TEMP_STS;
		//case SFM_ALM_TYPE_SCE_VOLT			:	return ALMCODE_SFM_SCE_UNKNOWN;
		case SFM_ALM_TYPE_SCE_VOLT				:	return ALMCODE_SFM_UNKNOWN;
		case SFM_ALM_TYPE_SCE_PORT_MGMT			:	return ALMCODE_SFM_SCE_PORT_USAGE;
		case SFM_ALM_TYPE_SCE_PORT_LINK			:	return ALMCODE_SFM_SCE_PORT_USAGE;
		//case SFM_ALM_TYPE_SCE_RDR				:	return ALMCODE_SFM_SCE_UNKNOWN;
		case SFM_ALM_TYPE_SCE_RDR				:	return ALMCODE_SFM_UNKNOWN;
		case SFM_ALM_TYPE_SCE_RDR_CONN			:	return ALMCODE_SFM_SCE_RDR_CONN_STS;
		case SFM_ALM_TYPE_SCE_STATUS			:	return ALMCODE_SFM_SCE_SYS_STS;
		case SFM_ALM_TYPE_L2_CPU				:	return ALMCODE_SFM_L2SW_CPU_USAGE;
		case SFM_ALM_TYPE_L2_MEM				:	return ALMCODE_SFM_L2SW_MEMORY_USAGE;
		case SFM_ALM_TYPE_L2_LAN				:	return ALMCODE_SFM_L2SW_LAN_STS;
		case SFM_ALM_TYPE_CPS_OVER				:	return ALMCODE_SFM_CPS_OVER_INFO;
		case SFM_ALM_TYPE_PROCESS_SAMD			:	return ALMCODE_SFM_PROCESS_SAMD;
		case SFM_ALM_TYPE_PROCESS_IXPC			:	return ALMCODE_SFM_PROCESS_IXPC;
		case SFM_ALM_TYPE_PROCESS_FIMD			:	return ALMCODE_SFM_PROCESS_FIMD;
		case SFM_ALM_TYPE_PROCESS_COND			:	return ALMCODE_SFM_PROCESS_COND;
		case SFM_ALM_TYPE_PROCESS_STMD			:	return ALMCODE_SFM_PROCESS_STMD;
		case SFM_ALM_TYPE_PROCESS_MMCD			:	return ALMCODE_SFM_PROCESS_MMCD;
		case SFM_ALM_TYPE_PROCESS_MCDM			:	return ALMCODE_SFM_PROCESS_MCMD;
		case SFM_ALM_TYPE_PROCESS_NMSIF			:	return ALMCODE_SFM_PROCESS_NMSIF;
		case SFM_ALM_TYPE_PROCESS_CDELAY		:	return ALMCODE_SFM_PROCESS_CDELAY;
		case SFM_ALM_TYPE_PROCESS_HAMON			:	return ALMCODE_SFM_PROCESS_HAMON;
		case SFM_ALM_TYPE_PROCESS_MMCR			:	return ALMCODE_SFM_PROCESS_MMCR;
		case SFM_ALM_TYPE_PROCESS_RDRANA		:	return ALMCODE_SFM_PROCESS_RDRANA;
		case SFM_ALM_TYPE_PROCESS_RLEG0			:	return ALMCODE_SFM_PROCESS_RLEG0;
		case SFM_ALM_TYPE_PROCESS_RLEG1			:	return ALMCODE_SFM_PROCESS_RLEG1;
		case SFM_ALM_TYPE_PROCESS_RLEG2			:	return ALMCODE_SFM_PROCESS_RLEG2;
		case SFM_ALM_TYPE_PROCESS_RLEG3			:	return ALMCODE_SFM_PROCESS_RLEG3;
		case SFM_ALM_TYPE_PROCESS_RLEG4			:	return ALMCODE_SFM_PROCESS_RLEG4;
		case SFM_ALM_TYPE_PROCESS_SMPP			:	return ALMCODE_SFM_PROCESS_SMPP;
		case SFM_ALM_TYPE_PROCESS_PANA			:	return ALMCODE_SFM_PROCESS_PANA;
		case SFM_ALM_TYPE_PROCESS_RANA			:	return ALMCODE_SFM_PROCESS_RANA;
		case SFM_ALM_TYPE_PROCESS_RDRCAPD		:	return ALMCODE_SFM_PROCESS_RDRCAPD;
		case SFM_ALM_TYPE_PROCESS_CAPD			:	return ALMCODE_SFM_PROCESS_CAPD;
		case SFM_ALM_TYPE_PROCESS_SCEM          :	return ALMCODE_SFM_PROCESS_SCEM;
		case SFM_ALM_TYPE_PROCESS_CSCM          :	return ALMCODE_SFM_PROCESS_CSCM;
		case SFM_ALM_TYPE_PROCESS_DIRM          :	return ALMCODE_SFM_PROCESS_DIRM;
		case SFM_ALM_TYPE_HW_MIRROR				:	return ALMCODE_SFM_MIRROR_PORT;
		case SFM_ALM_TYPE_LEG_SESSION           :	return ALMCODE_SFM_LEG_SESSION_USAGE;
		case SFM_ALM_TYPE_SCE_USER              :	return ALMCODE_SFM_SCE_USER_USAGE;
		case SFM_ALM_TYPE_TPS:	       return ALMCODE_SFM_TPS_INFO; /* TPS addes by dcham 2011.05.25 */
		
        //default									:	return 0;
        default									:	return ALMCODE_SFM_UNKNOWN;
    }
} /* End of get_alm_code () */

/*
	by sjjeon 
	alarm level에 따라 alarm code가 달라진다.
	nmsif의 class 값은 6(minor clear), 7(major clear)
	8 (critical clear) 임. 
	alarm code 에 추가적인 값을 더해준다.
	minor (+100), major (+200), critical (+300)
 */
#define ALM_MINOR_CLEAR			6
#define ALM_MAJOR_CLEAR			7
#define ALM_CRITICAL_CLEAR		8

int get_alm_code_ex (int atype, int almClass)
{
	int almCode, levelVal=0;

	switch(almClass){
		case SFM_ALM_MINOR:
		case ALM_MINOR_CLEAR:
        	levelVal = ALMCODE_STS_MINOR;   // 100
			break;
		case SFM_ALM_MAJOR:
		case ALM_MAJOR_CLEAR:
        	levelVal = ALMCODE_STS_MAJOR;   // 200
			break;
		case SFM_ALM_CRITICAL:
		case ALM_CRITICAL_CLEAR:
        	levelVal = ALMCODE_STS_CRITICAL; // 300
			break;
		default:
			break;
	}

    switch (atype)
	{
		case SFM_ALM_TYPE_CPU_USAGE				:	almCode = ALMCODE_SFM_CPU_USAGE; break;
		case SFM_ALM_TYPE_MEMORY_USAGE			:	almCode = ALMCODE_SFM_MEMORY_USAGE;	break;
		case SFM_ALM_TYPE_DISK_USAGE			:	almCode = ALMCODE_SFM_DISK_USAGE; break;
		case SFM_ALM_TYPE_LAN					:	almCode = ALMCODE_SFM_LAN; break;
		case SFM_ALM_TYPE_PROC					:	almCode = ALMCODE_SFM_PROCESS; break;
		case SFM_ALM_TYPE_LINK					:	almCode = ALMCODE_SFM_LINK; break;
		case SFM_ALM_TYPE_MP_HW					:	almCode = ALMCODE_SFM_HPUX_HW; break;
	//	case SFM_ALM_TYPE_CONN_SERVER			:	almCode = ALMCODE_SFM_SERVER_CONN; break;
		case SFM_ALM_TYPE_CALL_INFO				:	almCode = ALMCODE_SFM_CALL_INFO; break;
		case SFM_ALM_TYPE_DUP_STS				:	almCode = ALMCODE_SFM_DUPLICATION; break;
		case SFM_ALM_TYPE_DUP_HEARTBEAT			:	almCode = ALMCODE_SFM_HEARTBEAT; break;
		case SFM_ALM_TYPE_DUP_OOS				:	almCode = ALMCODE_SFM_OOS; break;
		case SFM_ALM_TYPE_SUCC_RATE				:	almCode = ALMCODE_SFM_SUCCESS_RATE; break;
		case SFM_ALM_TYPE_SESS_LOAD				:	almCode = ALMCODE_SFM_SESS_LOAD; break;
		case SFM_ALM_TYPE_DBCON_STST			:	almCode = ALMCODE_SFM_RMT_DB_STS; break;
		case SFM_ALM_TYPE_RMT_LAN				:	almCode = ALMCODE_SFM_RMT_LAN; break;
		case SFM_ALM_TYPE_OPT_LAN				:	almCode = ALMCODE_SFM_OPT_LAN; break;
		case SFM_ALM_TYPE_HWNTP					:	almCode = ALMCODE_SFM_HWNTP; break;
		//case SFM_ALM_TYPE_PD_CPU_USAGE		:	almCode = ALMCODE_SFM_PD_CPU_USAGE; break;
		//case SFM_ALM_TYPE_PD_MEMORY_USAGE		:	almCode = ALMCODE_SFM_PD_MEMORY_USAGE; break;
		//case SFM_ALM_TYPE_PD_FAN_STS			:	almCode = ALMCODE_SFM_PD_FAN_USAGE; break;
		//case SFM_ALM_TYPE_PD_GIGA_LAN			:	almCode = ALMCODE_SFM_PD_GIGA_USAGE;
		case SFM_ALM_TYPE_TAP_CPU_USAGE			:	almCode = ALMCODE_SFM_TAP_CPU_USAGE; break;
		case SFM_ALM_TYPE_TAP_MEMORY_USAGE		:	almCode = ALMCODE_SFM_TAP_MEMORY_USAGE; break;
		case SFM_ALM_TYPE_TAP_FAN_STS			:	almCode = ALMCODE_SFM_TAP_FAN_USAGE; break;
		case SFM_ALM_TYPE_TAP_PORT_STS			:	almCode = ALMCODE_SFM_TAP_PORT_STS; break;
		case SFM_ALM_TYPE_RSRC_LOAD				:	almCode = ALMCODE_SFM_RSRC_LOAD; break;
		case SFM_ALM_TYPE_QUEUE_LOAD			:	almCode = ALMCODE_SFM_QUEUE_LOAD; break;
		//case SFM_ALM_TYPE_NMSIF_CONNECT:			almCode = ALMCODE_NMS_CONNECT; break;
		case SFM_ALM_TYPE_NMSIF_CONNECT:			return 	ALMCODE_NMS_CONNECT; // NMS CONNECTION ALARM은 프로세스별 지정 되지 않는다.
		case SFM_ALM_TYPE_DUAL_ACT				:	almCode = ALMCODE_SFM_DUAL_ACT; break;
		case SFM_ALM_TYPE_DUAL_STD				:	almCode = ALMCODE_SFM_DUAL_STD; break;
		case SFM_ALM_TYPE_DUAL_STS_QRY_TIME_OUT	:	almCode = ALMCODE_SFM_DUAL_STS_QRY_TIME_OUT; break;
		case SFM_ALM_TYPE_SCE_CPU				:	almCode = ALMCODE_SFM_SCE_CPU_USAGE; break;
		case SFM_ALM_TYPE_SCE_MEM				:	almCode = ALMCODE_SFM_SCE_MEM_USAGE; break;
		case SFM_ALM_TYPE_SCE_DISK				:	almCode = ALMCODE_SFM_SCE_DISK_USAGE; break;
		case SFM_ALM_TYPE_SCE_PWR				:	almCode = ALMCODE_SFM_SCE_PWR_USAGE; break;
		case SFM_ALM_TYPE_SCE_FAN				:	almCode = ALMCODE_SFM_SCE_FAN_USAGE; break;
		case SFM_ALM_TYPE_SCE_TEMP				:	almCode = ALMCODE_SFM_SCE_TEMP_STS; break;
		//case SFM_ALM_TYPE_SCE_VOLT			:	almCode = ALMCODE_SFM_SCE_UNKNOWN; break;
		case SFM_ALM_TYPE_SCE_VOLT				:	almCode = ALMCODE_SFM_UNKNOWN; break;
		case SFM_ALM_TYPE_SCE_PORT_MGMT			:	almCode = ALMCODE_SFM_SCE_PORT_USAGE; break;break;
		case SFM_ALM_TYPE_SCE_PORT_LINK			:	almCode = ALMCODE_SFM_SCE_PORT_USAGE; break;
		//case SFM_ALM_TYPE_SCE_RDR				:	almCode = ALMCODE_SFM_SCE_UNKNOWN; break;
		case SFM_ALM_TYPE_SCE_RDR				:	almCode = ALMCODE_SFM_UNKNOWN; break;
		case SFM_ALM_TYPE_SCE_RDR_CONN			:	almCode = ALMCODE_SFM_SCE_RDR_CONN_STS; break;
		case SFM_ALM_TYPE_SCE_STATUS			:	almCode = ALMCODE_SFM_SCE_SYS_STS; break;
		case SFM_ALM_TYPE_L2_CPU				:	almCode = ALMCODE_SFM_L2SW_CPU_USAGE; break;
		case SFM_ALM_TYPE_L2_MEM				:	almCode = ALMCODE_SFM_L2SW_MEMORY_USAGE; break;
		case SFM_ALM_TYPE_L2_LAN				:	almCode = ALMCODE_SFM_L2SW_LAN_STS; break;
		case SFM_ALM_TYPE_CPS_OVER				:	almCode = ALMCODE_SFM_CPS_OVER_INFO; break;
		case SFM_ALM_TYPE_PROCESS_SAMD			:	almCode = ALMCODE_SFM_PROCESS_SAMD; break;
		case SFM_ALM_TYPE_PROCESS_IXPC			:	almCode = ALMCODE_SFM_PROCESS_IXPC; break;
		case SFM_ALM_TYPE_PROCESS_FIMD			:	almCode = ALMCODE_SFM_PROCESS_FIMD; break;
		case SFM_ALM_TYPE_PROCESS_COND			:	almCode = ALMCODE_SFM_PROCESS_COND; break;
		case SFM_ALM_TYPE_PROCESS_STMD			:	almCode = ALMCODE_SFM_PROCESS_STMD; break;
		case SFM_ALM_TYPE_PROCESS_MMCD			:	almCode = ALMCODE_SFM_PROCESS_MMCD; break;
		case SFM_ALM_TYPE_PROCESS_MCDM			:	almCode = ALMCODE_SFM_PROCESS_MCMD; break;
		case SFM_ALM_TYPE_PROCESS_NMSIF			:	almCode = ALMCODE_SFM_PROCESS_NMSIF; break;
		case SFM_ALM_TYPE_PROCESS_CDELAY		:	almCode = ALMCODE_SFM_PROCESS_CDELAY; break;
		case SFM_ALM_TYPE_PROCESS_HAMON			:	almCode = ALMCODE_SFM_PROCESS_HAMON; break;
		case SFM_ALM_TYPE_PROCESS_MMCR			:	almCode = ALMCODE_SFM_PROCESS_MMCR; break;
		case SFM_ALM_TYPE_PROCESS_RDRANA		:	almCode = ALMCODE_SFM_PROCESS_RDRANA; break;
		case SFM_ALM_TYPE_PROCESS_SMPP			:	almCode = ALMCODE_SFM_PROCESS_SMPP; break;
		case SFM_ALM_TYPE_PROCESS_PANA			:	almCode = ALMCODE_SFM_PROCESS_PANA; break;
		case SFM_ALM_TYPE_PROCESS_RANA			:	almCode = ALMCODE_SFM_PROCESS_RANA; break;
		case SFM_ALM_TYPE_PROCESS_RDRCAPD		:	almCode = ALMCODE_SFM_PROCESS_RDRCAPD; break;
		case SFM_ALM_TYPE_PROCESS_CAPD			:	almCode = ALMCODE_SFM_PROCESS_CAPD; break;
		case SFM_ALM_TYPE_PROCESS_SCEM			:	almCode = ALMCODE_SFM_PROCESS_SCEM; break;
		case SFM_ALM_TYPE_PROCESS_CSCM			:	almCode = ALMCODE_SFM_PROCESS_CSCM; break;
		case SFM_ALM_TYPE_PROCESS_DIRM			:	almCode = ALMCODE_SFM_PROCESS_DIRM; break;
		case SFM_ALM_TYPE_HW_MIRROR				:	almCode = ALMCODE_SFM_MIRROR_PORT; break;
		case SFM_ALM_TYPE_SCE_USER				:	almCode = ALMCODE_SFM_SCE_USER_USAGE; break;
		case SFM_ALM_TYPE_LEG_SESSION			:	almCode = ALMCODE_SFM_LEG_SESSION_USAGE; break;
		case SFM_ALM_TYPE_SCM_FAULTED			:	almCode = ALMCODE_SFM_SCM_FAULTED; break;
		case SFM_ALM_TYPE_LOGON_SUCCESS_RATE    :   almCode = ALMCODE_SFM_LOGON_SUCCESS_RATE; break;
		case SFM_ALM_TYPE_LOGOUT_SUCCESS_RATE   :   almCode = ALMCODE_SFM_LOGOUT_SUCCESS_RATE; break;
		case SFM_ALM_TYPE_SM_CONN_STS           :   almCode = ALMCODE_SFM_SM_CONN_STS; break;
		case SFM_ALM_TYPE_PROCESS_RLEG0			:	almCode = ALMCODE_SFM_PROCESS_RLEG0; break; /* RLEG0~4 added by dcham 2011.04.26 */
		case SFM_ALM_TYPE_PROCESS_RLEG1			:	almCode = ALMCODE_SFM_PROCESS_RLEG1; break;
		case SFM_ALM_TYPE_PROCESS_RLEG2			:	almCode = ALMCODE_SFM_PROCESS_RLEG2; break;
		case SFM_ALM_TYPE_PROCESS_RLEG3			:	almCode = ALMCODE_SFM_PROCESS_RLEG3; break;
		case SFM_ALM_TYPE_PROCESS_RLEG4			:	almCode = ALMCODE_SFM_PROCESS_RLEG4; break;
		case SFM_ALM_TYPE_TPS               :	almCode = ALMCODE_SFM_TPS_INFO; break; /* TPS addes by dcham 2011.05.25 */

        //default									:	return 0; break;
        default									:	return ALMCODE_SFM_UNKNOWN;
    }

	almCode += levelVal;
	return almCode;
	
} /* End of get_alm_code_ex () */

/*
   modified by sjjeon
	aclass의 level을 6,7,8 추가한다. 
	6 : minor clear
	7 : major clear
	8 : critical clear
 */
get_alm_class (int aclass)
{
	switch (aclass) {
		case 8 : return 5; 		
		case 7 : return 5;
		case 6 : return 5;
		case 5 : return 5;
		case 3 : return 1;
		case 2 : return 2;
		case 1 : return 3;
		default : return 0;
	}

} /* End of get_alm_class () */

char *get_supple_info (char *node, int type, char *rsc)
{
        int             blen=0;
        static char     resbuf[1024];

        memset (resbuf, 0, 1024);

        strcpy (resbuf, node);
        blen = strlen (resbuf);

        if (type == SFM_ALM_TYPE_CPU_USAGE)
                sprintf (&resbuf[blen], ", %s (%s)", "CPU LOAD ALARM", rsc);
        else if (type == SFM_ALM_TYPE_MEMORY_USAGE)
                sprintf (&resbuf[blen], ", %s (%s)", "MEMORY LOAD ALARM", rsc);
        else if (type == SFM_ALM_TYPE_DISK_USAGE)
                sprintf (&resbuf[blen], ", %s (%s)", "DISK LOAD ALARM", rsc);
        else if (type == SFM_ALM_TYPE_LAN)
                sprintf (&resbuf[blen], ", %s (%s)", "MANAGEMENT NETWORK LINK ALARM", rsc);
        else if (type == SFM_ALM_TYPE_PROC)
                sprintf (&resbuf[blen], ", %s (%s)", "SW PROCESS ALARM", rsc);
        else if (type == SFM_ALM_TYPE_MP_HW)
                sprintf (&resbuf[blen], ", %s (%s)", "DSC HW ALARM", rsc);
        else if (type == SFM_ALM_TYPE_CALL_INFO)
                sprintf (&resbuf[blen], ", %s (%s)", "CALL INFO ALARM", rsc);
        else if (type == SFM_ALM_TYPE_DUP_HEARTBEAT)
                sprintf (&resbuf[blen], ", %s (%s)", "DUPLICATION HEARTBEAT ALARM", rsc);
        else if (type == SFM_ALM_TYPE_SUCC_RATE)
                sprintf (&resbuf[blen], ", %s (%s)", "SUCCESS RATE ALARM", rsc);
        else if (type == SFM_ALM_TYPE_SESS_LOAD)
                sprintf (&resbuf[blen], ", %s (%s)", "SESSION LOAD ALARM", rsc);
		else if (type == SFM_ALM_TYPE_DBCON_STST)
                sprintf (&resbuf[blen], ", %s (%s)", "UAWAP DB CONNECTION STATUS ALARM", rsc);
        else if (type == SFM_ALM_TYPE_RMT_LAN)
                sprintf (&resbuf[blen], ", %s (%s)", "REMOTE NETWORK LINK ALARM", rsc);
        else if (type == SFM_ALM_TYPE_OPT_LAN)
                sprintf (&resbuf[blen], ", %s (%s)", "MIRRORING PORT ALARM", rsc);
        else if (type == SFM_ALM_TYPE_HWNTP)
                sprintf (&resbuf[blen], ", %s (%s)", "NETWORK TIME PROTOCOL ALARM", rsc);
        //else if (type == SFM_ALM_TYPE_PD_CPU_USAGE)
                //sprintf (&resbuf[blen], ", %s (%s)", "PROBING DEVICE CPU USAGE ALARM", rsc);
        else if (type == SFM_ALM_TYPE_TAP_CPU_USAGE)
                sprintf (&resbuf[blen], ", %s (%s)", "TAP CPU USAGE ALARM", rsc);
        //else if (type == SFM_ALM_TYPE_PD_MEMORY_USAGE)
                //sprintf (&resbuf[blen], ", %s (%s)", "PROBING DEVICE MEMORY USAGE ALARM", rsc);
        else if (type == SFM_ALM_TYPE_TAP_MEMORY_USAGE)
                sprintf (&resbuf[blen], ", %s (%s)", "TAP MEMORY USAGE ALARM", rsc);
        //else if (type == SFM_ALM_TYPE_PD_FAN_STS)
                //sprintf (&resbuf[blen], ", %s (%s)", "PROBING DEVICE FAN ALARM", rsc);
		else if (type == SFM_ALM_TYPE_TAP_FAN_STS)
			sprintf (&resbuf[blen], ", %s (%s)", "TAP FAN ALARM", rsc);
		//else if (type == SFM_ALM_TYPE_PD_GIGA_LAN)
		//sprintf (&resbuf[blen], ", %s (%s)", "PROBING DEVICE NETWORK ALARM", rsc);
		else if (type == SFM_ALM_TYPE_TAP_PORT_STS)
			sprintf (&resbuf[blen], ", %s (%s)", "TAP NETWORK LINK ALARM", rsc);
		else if (type == SFM_ALM_TYPE_TAP_POWER_STS) // 20110422 by dcham
			sprintf (&resbuf[blen], ", %s (%s)", "TAP POWER ALARM", rsc);
		else if (type == SFM_ALM_TYPE_RSRC_LOAD)
			sprintf (&resbuf[blen], ", %s (%s)", "RESOURCE LOAD ALARM", rsc);
		else if (type == SFM_ALM_TYPE_QUEUE_LOAD)
                sprintf (&resbuf[blen], ", %s (%s)", "QUEUE LOAD ALARM", rsc);
        else if (type == SFM_ALM_TYPE_NMSIF_CONNECT)
                sprintf (&resbuf[blen], ", %s (%s)", "NMSIF STATUS ALARM", rsc);
        else if (type == SFM_ALM_TYPE_DUAL_ACT)
                sprintf (&resbuf[blen], ", %s (%s)", "DUPLICATION STATUS DUAL ACTIVE ALARM", rsc);
        else if (type == SFM_ALM_TYPE_DUAL_STD)
                sprintf (&resbuf[blen], ", %s (%s)", "DUPLICATION STATUS DUAL STANDBY ALARM", rsc);
        else if (type == SFM_ALM_TYPE_DUAL_STS_QRY_TIME_OUT)
                sprintf (&resbuf[blen], ", %s (%s)", "DUAL STATUS QUERY TIME OUT ALARM", rsc);
        else if (type == SFM_ALM_TYPE_SCE_CPU)
                sprintf (&resbuf[blen], ", %s (%s)", "SCE CPU LOAD ALARM", rsc);
        else if (type == SFM_ALM_TYPE_SCE_MEM)
                sprintf (&resbuf[blen], ", %s (%s)", "SCE MEMORY LOAD ALARM", rsc);
        else if (type == SFM_ALM_TYPE_SCE_DISK)
                sprintf (&resbuf[blen], ", %s (%s)", "SCE DISK LOAD ALARM", rsc);
        else if (type == SFM_ALM_TYPE_SCE_PWR)
                sprintf (&resbuf[blen], ", %s (%s)", "SCE POWER ALARM", rsc);
        else if (type == SFM_ALM_TYPE_SCE_FAN)
                sprintf (&resbuf[blen], ", %s (%s)", "SCE FAN ALARM", rsc);
        else if (type == SFM_ALM_TYPE_SCE_TEMP)
                sprintf (&resbuf[blen], ", %s (%s)", "SCE TEMPERATURE ALARM", rsc);
        else if (type == SFM_ALM_TYPE_SCE_VOLT)
                sprintf (&resbuf[blen], ", %s (%s)", "SCE EXCESS VOLTAGE ALARM", rsc);
        else if (type == SFM_ALM_TYPE_SCE_PORT_MGMT)
                sprintf (&resbuf[blen], ", %s (%s)", "SCE MGMT LINK ALARM", rsc);
        else if (type == SFM_ALM_TYPE_SCE_PORT_LINK)
                sprintf (&resbuf[blen], ", %s (%s)", "SCE LINK ALARM", rsc);
        else if (type == SFM_ALM_TYPE_SCE_RDR)
                sprintf (&resbuf[blen], ", %s (%s)", "SCE RDR STATUS ALARM", rsc);
        else if (type == SFM_ALM_TYPE_SCE_RDR_CONN)
                sprintf (&resbuf[blen], ", %s (%s)", "SCE RDR CONNECTION ALARM", rsc);
        else if (type == SFM_ALM_TYPE_SCE_STATUS)
                sprintf (&resbuf[blen], ", %s (%s)", "SCE STATUS ALARM", rsc);
        else if (type == SFM_ALM_TYPE_L2_CPU)
                sprintf (&resbuf[blen], ", %s (%s)", "L2 SWITCH CPU LOAD ALARM", rsc);
        else if (type == SFM_ALM_TYPE_L2_MEM)
                sprintf (&resbuf[blen], ", %s (%s)", "L2 SWITCH MEMORY LOAD ALARM", rsc);
        else if (type == SFM_ALM_TYPE_L2_LAN)
                sprintf (&resbuf[blen], ", %s (%s)", "L2 SWITCH LINK ALARM", rsc);
        else if (type == SFM_ALM_TYPE_CPS_OVER)
                sprintf (&resbuf[blen], ", %s (%s)", "SCM CPS OVER ALARM", rsc);
        else if (type == SFM_ALM_TYPE_PROCESS_SAMD)
                sprintf (&resbuf[blen], ", %s (%s)", "SW PROCESS ALARM(SAMD)", rsc);
        else if (type == SFM_ALM_TYPE_PROCESS_IXPC)
                sprintf (&resbuf[blen], ", %s (%s)", "SW PROCESS ALARM(IXPC)", rsc);
        else if (type == SFM_ALM_TYPE_PROCESS_FIMD)
                sprintf (&resbuf[blen], ", %s (%s)", "SW PROCESS ALARM(FIMD)", rsc);
        else if (type == SFM_ALM_TYPE_PROCESS_COND)
                sprintf (&resbuf[blen], ", %s (%s)", "SW PROCESS ALARM(COND)", rsc);
        else if (type == SFM_ALM_TYPE_PROCESS_STMD)
                sprintf (&resbuf[blen], ", %s (%s)", "SW PROCESS ALARM(STMD)", rsc);
        else if (type == SFM_ALM_TYPE_PROCESS_MMCD)
                sprintf (&resbuf[blen], ", %s (%s)", "SW PROCESS ALARM(MMCD)", rsc);
        else if (type == SFM_ALM_TYPE_PROCESS_MCDM)
                sprintf (&resbuf[blen], ", %s (%s)", "SW PROCESS ALARM(MCDM)", rsc);
        else if (type == SFM_ALM_TYPE_PROCESS_NMSIF)
                sprintf (&resbuf[blen], ", %s (%s)", "SW PROCESS ALARM(NMSIF)", rsc);
        else if (type == SFM_ALM_TYPE_PROCESS_CDELAY)
                sprintf (&resbuf[blen], ", %s (%s)", "SW PROCESS ALARM(CDELAY)", rsc);
        else if (type == SFM_ALM_TYPE_PROCESS_HAMON)
                sprintf (&resbuf[blen], ", %s (%s)", "SW PROCESS ALARM(HAMON)", rsc);
        else if (type == SFM_ALM_TYPE_PROCESS_MMCR)
                sprintf (&resbuf[blen], ", %s (%s)", "SW PROCESS ALARM(MMCR)", rsc);
        else if (type == SFM_ALM_TYPE_PROCESS_RDRANA)
                sprintf (&resbuf[blen], ", %s (%s)", "SW PROCESS ALARM(RDRANA)", rsc);
//        else if (type == SFM_ALM_TYPE_PROCESS_RLEG)
 //               sprintf (&resbuf[blen], ", %s (%s)", "SW PROCESS ALARM(RLEG)", rsc);
        else if (type == SFM_ALM_TYPE_PROCESS_SMPP)
                sprintf (&resbuf[blen], ", %s (%s)", "SW PROCESS ALARM(SMPP)", rsc);
        else if (type == SFM_ALM_TYPE_PROCESS_PANA)
                sprintf (&resbuf[blen], ", %s (%s)", "SW PROCESS ALARM(PANA)", rsc);
        else if (type == SFM_ALM_TYPE_PROCESS_RANA)
                sprintf (&resbuf[blen], ", %s (%s)", "SW PROCESS ALARM(RANA)", rsc);
        else if (type == SFM_ALM_TYPE_PROCESS_RDRCAPD)
                sprintf (&resbuf[blen], ", %s (%s)", "SW PROCESS ALARM(RDRCAPD)", rsc);
        else if (type == SFM_ALM_TYPE_PROCESS_CAPD)
                sprintf (&resbuf[blen], ", %s (%s)", "SW PROCESS ALARM(CAPD)", rsc);
        else if (type == SFM_ALM_TYPE_PROCESS_SCEM)
                sprintf (&resbuf[blen], ", %s (%s)", "SW PROCESS ALARM(SCEM)", rsc);
        else if (type == SFM_ALM_TYPE_PROCESS_CSCM)
                sprintf (&resbuf[blen], ", %s (%s)", "SW PROCESS ALARM(CSCM)", rsc);
        else if (type == SFM_ALM_TYPE_PROCESS_DIRM)
                sprintf (&resbuf[blen], ", %s (%s)", "SW PROCESS ALARM(DIRM)", rsc);
        else if (type == SFM_ALM_TYPE_PROCESS_RLEG0)
                sprintf (&resbuf[blen], ", %s (%s)", "SW PROCESS ALARM(RLEG0)", rsc);
        else if (type == SFM_ALM_TYPE_PROCESS_RLEG1)  /* RLEG0~4 added by dcham 2011.04.26 */
                sprintf (&resbuf[blen], ", %s (%s)", "SW PROCESS ALARM(RLEG1)", rsc);
        else if (type == SFM_ALM_TYPE_PROCESS_RLEG2)
                sprintf (&resbuf[blen], ", %s (%s)", "SW PROCESS ALARM(RLEG2)", rsc);
        else if (type == SFM_ALM_TYPE_PROCESS_RLEG3)
                sprintf (&resbuf[blen], ", %s (%s)", "SW PROCESS ALARM(RLEG3)", rsc);
        else if (type == SFM_ALM_TYPE_PROCESS_RLEG4)
                sprintf (&resbuf[blen], ", %s (%s)", "SW PROCESS ALARM(RLEG4)", rsc);
        else if (type == SFM_ALM_TYPE_HW_MIRROR)
                sprintf (&resbuf[blen], ", %s (%s)", "SCM H/W MIRROR LINK ALARM", rsc);
        else if (type == SFM_ALM_TYPE_TPS)
                sprintf (&resbuf[blen], ", %s (%s)", "TPS LOAD ALARM", rsc); /* TPS alarm description added by dcham 2011.05.25 */
		else if (type == SFM_ALM_TYPE_LOGON_SUCCESS_RATE)
                sprintf (&resbuf[blen], ", (%s)","LOGON SUCCESS RATE ALARM",  rsc);
		else if (type == SFM_ALM_TYPE_LOGOUT_SUCCESS_RATE)
                sprintf (&resbuf[blen], ", %s","LOGOUT_SUCCESS RATE ALARM",  rsc);
        else if (type == SFM_ALM_TYPE_LEG_SESSION)
                sprintf (&resbuf[blen], ", %s (%s)", "LEG SESSION USAGE ALARM", rsc); /* added by dcham 2011.06.08 */
        else if (type == SFM_ALM_TYPE_SCE_USER)
                sprintf (&resbuf[blen], ", %s (%s)", "SCE USER ALARM", rsc); /* added by dcham 2011.06.08 */
        else
        	sprintf (&resbuf[blen], ", UNKNOWN ALARM (%s)", rsc);

        return resbuf;

} /* End of get_supple_info () */

char *get_oidtime_with_para(StatQueryInfo *qry)
{
	int         i,j;
	static char tm_oid[1024];
	memset(tm_oid, 0x00, sizeof(tm_oid));

	//time  e.g) "2011-02-27 12:45:00" @qry->stime
	//retur e.g) "20110227124500"

	i = j = 0;
	//year
	tm_oid[i++] = qry->stime[j++];
	tm_oid[i++] = qry->stime[j++];
	tm_oid[i++] = qry->stime[j++];
	tm_oid[i++] = qry->stime[j++];

	//month
	j++;
	tm_oid[i++] = qry->stime[j++];
	tm_oid[i++] = qry->stime[j++];

	//date
	j++;
	tm_oid[i++] = qry->stime[j++];
	tm_oid[i++] = qry->stime[j++];

	if( STAT_PERIOD_HOUR == qry->period || STAT_PERIOD_5MIN == qry->period ){
		j++;
		tm_oid[i++] = qry->stime[j++];
		tm_oid[i++] = qry->stime[j++];
		if( STAT_PERIOD_5MIN == qry->period ){
			j++;
			tm_oid[i++] = qry->stime[j++];
			tm_oid[i++] = qry->stime[j++];
		}
	}
	tm_oid[i] = 0x00;

	return tm_oid;
}

char *get_oidtime (int prd)
{
	static char	tm_oid[1024];
	struct tm	*n_time;

	n_time = (struct tm *)now ();

	memset(tm_oid, 0x00, sizeof(tm_oid));

	if (prd == STAT_PERIOD_5MIN)
		sprintf (tm_oid, "%04d%02d%02d%02d%02d", n_time->tm_year+1900, n_time->tm_mon+1, n_time->tm_mday, n_time->tm_hour, n_time->tm_min);
	else if (prd == STAT_PERIOD_HOUR)
		sprintf (tm_oid, "%04d%02d%02d%02d", n_time->tm_year+1900, n_time->tm_mon+1, n_time->tm_mday, n_time->tm_hour);
	else
		sprintf (tm_oid, "%04d%02d%02d", n_time->tm_year+1900, n_time->tm_mon+1, n_time->tm_mday);

	return tm_oid;
}


get_abs_min (int r_min)
{
	static int	ret_min=0;

	if (!(r_min%5))
		ret_min = r_min;
	else ret_min = r_min - (r_min%5);

	return ret_min;
}



char *get_time_str (int ntm)
{
	static char	ttime[40];
	struct tm	*ntime;

	memset (ttime, 0, 40);

	ntime = (struct tm *)now ();

	sprintf (ttime, "%04d%02d%02d%02d%02d%02d", ntime->tm_year+1900,
			ntime->tm_mon+1, ntime->tm_mday, ntime->tm_hour,
			ntime->tm_min, ntime->tm_sec);

	return ttime;
}


mysql_select_query (char *qry)
{
	int		rval=0;
	MYSQL_RES	*rsql;
	MYSQL_ROW	rrow;

	if(nmsif_mysql_query(conn, qry) < 0 ){
		printf ("[*] mysql_query fail : %s\n", mysql_error (conn));
		return -1;
	}

	rval = 0;
	rsql = mysql_store_result (conn);
	while( (rrow = mysql_fetch_row(rsql)) != NULL)
	{
		rval = 1;
		break;
	}
	mysql_free_result (rsql);

	return rval;
} /* End of mysql_select_query () */


char *int2dot (int ipaddr)
{
	static char		dot_buf[20];
	struct in_addr	n_info;

	memset (dot_buf, 0, 20);
	n_info.s_addr = ipaddr;
	sprintf (dot_buf, "%s", inet_ntoa (n_info));

	return dot_buf;
}


struct tm *now ()
{
	struct tm	*tim;

	time_t	oclock = time (NULL);
	tim = localtime (&oclock);

	return tim;
}


save_fname (char *file, int sfd, int prd)
{
	int		k;

	if (prd == STAT_PERIOD_5MIN) {
		for (k=0; k<FILE_NUM_5MIN; k++) {

			if (hold_list[k].flag == 0) {

				if (sfd > 0) {
					hold_list[k].sfd 	= sfd;
					hold_list[k].flag	= FLAG_NEED_RCV_OK;
				}
				else {
					hold_list[k].sfd 	= 0;
					hold_list[k].flag	= FLAG_NO_NEED_RCV_OK;
				}

				hold_list[k].time = time ((time_t *)0);
				strcpy (hold_list[k].name, file);

				if (trcFlag || trcLogFlag) {
					sprintf (traceBuf, "insert file(%s)/fd(%d)/index(%d)\n", file, sfd, k);
					trclib_writeLog (FL, traceBuf);
				}
				break;
			}
		}
		if (k == FILE_NUM_5MIN) {
			sprintf (traceBuf, "not insert file(%s)\n", file);
			trclib_writeLogErr (FL, traceBuf);
		}
	}
	else {
		for (k=FILE_NUM_5MIN; k<FILE_NUM_5MIN+FILE_NUM_HOUR; k++) {
			if (hold_list[k].flag == 0) {

				if (sfd > 0) {
					hold_list[k].sfd 	= sfd;
					hold_list[k].flag 	= FLAG_NEED_RCV_OK;
				}
				else {
					hold_list[k].sfd 	= 0;
					hold_list[k].flag	= FLAG_NO_NEED_RCV_OK;
				}
				strcpy (hold_list[k].name, file);
				hold_list[k].time = time ((time_t *)0);

				if (trcFlag || trcLogFlag) {
					sprintf (traceBuf, "insert file(%s)/fd(%d)/index(%d)\n", file, sfd, k);
					trclib_writeLog (FL, traceBuf);
				}
				break;
			}
		}
		if (k == FILE_NUM_5MIN+FILE_NUM_HOUR) {
			sprintf (traceBuf, "not insert file(%s)\n", file);
			trclib_writeLogErr (FL, traceBuf);
		}
	}
	display_fname ();
	return 1;

} /* End of save_fname () */


reset_fname (char *file, int sfd)
{
	int		k, rst_cnt=0;

//lala
printf ("[*] reset fname=(%s), sfd=(%d)\n", file, sfd);

	for (k=0; k<FILE_NUM_5MIN+FILE_NUM_HOUR; k++) {
		if (!strcmp (file, hold_list[k].name) &&
			hold_list[k].sfd == sfd) {
			hold_list[k].sfd 	= 0;
			hold_list[k].flag 	= 0;
			hold_list[k].time 	= 0;
			memset (&hold_list[k].name, 0, FILE_NAME_LEN);

			if (trcFlag || trcLogFlag) {
				sprintf (traceBuf, "reset file(%s)/sfd(%d)/index(%d)\n", file, sfd, k);
				trclib_writeLog (FL, traceBuf);
			}
			return 1;
		}
	}
	sprintf (traceBuf, "not reset file(%s)\n", file, k);
	trclib_writeLogErr (FL, traceBuf);

	display_fname ();

	return -1;

} /* End of clear_fname () */


display_fname ()
{
	int		k;

	for (k=0; k<FILE_NUM_5MIN+FILE_NUM_HOUR; k++) {
		if (strlen (hold_list[k].name) > 0) {
			printf ("[%02d] sfd=%d, file=%s, flag=%d\n", k,
					hold_list[k].sfd, hold_list[k].name, hold_list[k].flag);
		}
	}
	puts ("______________________________________________________\n");
}


refresh_flist ()
{
	int		k;
	time_t	nt;

	nt = time ((time_t *)0);

	for (k=0; k<FILE_NUM_5MIN+FILE_NUM_HOUR; k++) {
		if (strlen (hold_list[k].name) > 0 &&
			hold_list[k].flag == FLAG_NEED_RCV_OK) {
			if (nt - hold_list[k].time > 60) {
				reset_fname (hold_list[k].name, hold_list[k].sfd);
			}
		}
	}
}


expired_stat_file ()
{
	char	bpath[200];
	char	tpath[200];
	char	*env;
	time_t	ttm;
	DIR		*dirp;
	struct dirent	*direntp;
	struct stat		tstat;


	if ((env = getenv (IV_HOME)) == NULL) {
		sprintf (traceBuf, "fail getenv () : %s\n", strerror (errno));
		trclib_writeLogErr (FL, traceBuf);
		return -1;
	}

	memset (bpath, 0, 200);
	sprintf (bpath, "%s/%s5MIN/", env, NMS_STAT_DIR);

	if ((dirp = opendir (bpath)) == (DIR *)NULL) {
		sprintf (traceBuf, "fail opendir (%s) : %s\n", bpath, strerror (errno));
		trclib_writeLogErr (FL, traceBuf);
		return -1;
	}

	ttm = time ((time_t *)0);

	while ((direntp = readdir (dirp)) != NULL) {
		if (!strcmp (direntp->d_name, "..") ||
			!strcmp (direntp->d_name, "."))
			continue;

		sprintf (tpath, "%s%s", bpath, direntp->d_name);

		memset (&tstat, 0, sizeof (struct stat));
		if (stat (tpath, &tstat) != 0)
			continue;

		// 화일 생성 이후 3시간 이상 초과하면 삭제 (5분 화일)
		// -> 48시간으로 연장 : 2006.08.29
		if ((ttm - tstat.st_ctime) >= 172800) {

			if (trcFlag || trcLogFlag) {
				sprintf (traceBuf, "remove file (%s)\n", tpath);
				trclib_writeLog (FL, traceBuf);
			}
			unlink (tpath);
			reset_fname (direntp->d_name, 0);
		}
	}
	closedir (dirp);

	memset (bpath, 0, 200);
	sprintf (bpath, "%s/%sHOUR/", env, NMS_STAT_DIR);

	if ((dirp = opendir (bpath)) == (DIR *)NULL) {
		sprintf (traceBuf, "fail opendir (%s) : %s\n", bpath, strerror (errno));
		trclib_writeLogErr (FL, traceBuf);
		return -1;
	}

	while ((direntp = readdir (dirp)) != NULL) {
		if (!strcmp (direntp->d_name, "..") ||
			!strcmp (direntp->d_name, "."))
			continue;

		sprintf (tpath, "%s%s", bpath, direntp->d_name);

		memset (&tstat, 0, sizeof (struct stat));
		if (stat (tpath, &tstat) != 0)
			continue;

		// 화일 생성 이후 48시간 이상 초과하면 삭제 (1시간 화일)
		// -> 1주일로 연장 : 2006.08.29
		if ((ttm - tstat.st_ctime) >= 604800) {

			if (trcFlag || trcLogFlag) {
				sprintf (traceBuf, "remove file (%s)\n", tpath);
				trclib_writeLog (FL, traceBuf);
			}
			unlink (tpath);
			reset_fname (direntp->d_name, 0);
		}
	}
	closedir (dirp);

	return 1;

} /* End of expired_stat_file () */


retrans_file_name ()
{
	int		i, j;
	int		lcnt=0;
	char	fn_buf[1024];

	lcnt = FILE_NUM_5MIN+FILE_NUM_HOUR;

	for (i=0; i<lcnt; i++) {

		if ((strlen (hold_list[i].name) > 0) &&
			(hold_list[i].flag == FLAG_NO_NEED_RCV_OK)) {

			memset (fn_buf, 0, 1024);
			sprintf (fn_buf, "F%s", hold_list[i].name);

			for (j=0; j<MAX_NMS_CON; j++) {
				if ((sfdb->nmsInfo.fd[j] > 0) &&
					(sfdb->nmsInfo.ptype[j] == FD_TYPE_DATA) &&
					(sfdb->nmsInfo.port[j] == ne_info.port[PORT_IDX_STAT])) {

//lala
if (trcFlag || trcLogFlag) {
	sprintf (traceBuf, "retrans file = [%d] (%s)\n", i, hold_list[i].name);
	trclib_writeLog (FL, traceBuf);
}

					send_packet (sfdb->nmsInfo.fd[j], fn_buf, 129);

					reset_fname (hold_list[i].name, 0);
				}
			}
		}
	}
	return 1;

} /* End of retrans_file_name () */
