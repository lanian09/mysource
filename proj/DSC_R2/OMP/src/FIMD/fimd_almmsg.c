#include "fimd_proto.h"

#define OCCURED_OR_CLEARED(x)   (x==SFM_ALM_OCCURED ? "OCCURED" : "CLEARED")
#define TCPCON_STATUS(x)        (x==SFM_ALM_OCCURED ? "DISCONNECT" : ((x==SFM_ALM_REMOVED) ? "NORMAL (REMOVED_IP)" : "CONNECT"))

#define INTERFACE_IS(x) (x==1 ? "SMSC" : (x==2 ? "VAS" : (x==3 ? "LMSC" : (x==4 ? "IDR" : (x==5 ? "SCP" : (x==6 ? "WISE" : "UNKNOWN")))))) 
#define SYS_NAME_IS(x)          (x==0 ? "INBH1" : "INBH2")


extern char		trcBuf[4096], trcTmp[1024], sysLabel[COMM_MAX_NAME_LEN];
extern SFM_sfdb		*sfdb;
extern SFM_L3PD		*l3pd;
extern SFM_SCE		*g_pstSCEInfo;
extern SFM_L2Dev    *g_pstL2Dev;
extern SFM_LOGON    *g_pstLogonRate;
extern int	trcFlag, trcLogFlag;
extern char *rsrcName[SFM_MAX_RSRC_LOAD_CNT];
extern int  g_hwswType[SFM_MAX_HPUX_HW_COM];

LANIF_CONFIG	lanConf;

int getScePortName(int index, char * dev_name);


//------------------------------------------------------------------------------
// 시스템 전체 장애 등급이 변경된 경우 호출되어 상태 메시지를 만들어 cond로 보낸다.
//------------------------------------------------------------------------------
int	fimd_makeSysAlmLevelChgMsg (int sysIndex)
{
	char	condBuf[4096], tmpBuf[1024];

	sprintf(condBuf,"    %s %s\n    S%04d SYSTEM ALARM LEVEL CHANGED\n",
			sysLabel,	// system_name
			commlib_printTStamp(),	// 현재시각 time stamp (년,월,일,시,분,초,요일)
			STSCODE_SFM_SYS_LEVEL_CHANGE);

	sprintf(tmpBuf,"      SYSTEM   = %s\n      PREVIOUS = %s\n      CURRENT  = %s\n      COMPLETED\n\n\n",
			sfdb->sys[sysIndex].commInfo.name,
			fimd_printAlmLevel (sfdb->sys[sysIndex].almInfo.prevLevel),
			fimd_printAlmLevel (sfdb->sys[sysIndex].almInfo.level));
	strcat (condBuf, tmpBuf);

	// cond로 보낸다.
	// - cond가 로그파일에 기록할때, MMC/STATUS/ALARM 메시지별 구분할 수 있도록
	//	mtype을 표시해 보낸다.
	fimd_txMsg2Cond (condBuf, MTYPE_STATUS_REPORT, STSCODE_SFM_SYS_LEVEL_CHANGE);

	return 1;

} //----- End of fimd_makeSysAlmLevelChgMsg -----//

/* by helca */

#define ACTIVE  1
#define STANDBY  2
#define FAULTED 3 // by dcham

int fimd_hdlDupstsMsg_yh (int sysIndex, int srcStatus, int destStatus)
{

	char	condBuf[4096], tmpBuf[1024], actName[10], stdName[10];
	
	memset(condBuf, 0x00, sizeof(condBuf)); 
	memset(tmpBuf, 0x00, sizeof(tmpBuf)); 
	memset(actName, 0x00, sizeof(actName)); 
	memset(stdName, 0x00, sizeof(stdName)); 


	switch (srcStatus) {
	case ACTIVE:
		sprintf(stdName, "ACTIVE");
		break;
	case STANDBY:
		sprintf(stdName, "STANDBY");
		break;
	case FAULTED: // by dcham
		sprintf(stdName, "FAULTED");
		break;
	default:
		sprintf(stdName, "UNKNOWN");
		break;
	}

	switch (destStatus) {
	case ACTIVE:
		sprintf(actName, "ACTIVE");
		break;
	case STANDBY:
		sprintf(actName, "STANDBY");
		break;
	case FAULTED: // by dcham
		sprintf(actName, "FAULTED");
		break;
	default:
		sprintf(actName, "UNKNOWN");
		break;
	}

	sprintf(condBuf,"    %s %s\n    S%04d DUPLICATION STATUS CHANGED\n",
			sysLabel,       // system_name
			commlib_printTStamp(),  // 현재시각 time stamp (년,월,일,시,분,초,요일)
			STSCODE_SFM_SYS_DUPLICATION_STS);

	sprintf(tmpBuf,"      SYSTEM = %s\n      %s ==> %s\n      COMPLETED\n\n\n",
			sfdb->sys[sysIndex].commInfo.name,
			stdName,
			actName);
	strcat (condBuf, tmpBuf);

	// cond로 보낸다.
	// - cond가 로그파일에 기록할때, MMC/STATUS/ALARM 메시지별 구분할 수 있도록
	//	mtype을 표시해 보낸다.
	fimd_txMsg2Cond (condBuf, MTYPE_STATUS_REPORT, STSCODE_SFM_SYS_DUPLICATION_STS);
	fimd_DuplicationStsEvent2Client();
	return 1;

} //----- End of fimd_hdlDupstsAlm -----//


//------------------------------------------------------------------------------
// CPU 과부하 장애 발생/해지 시 호출되어 장애 메시지를 만들어 cond로 보낸다.
// - 장애 등급 및 발생/해지 여부는 이미 결정된 상태로 호출된다.
//------------------------------------------------------------------------------
int	fimd_makeCpuUsageAlmMsg (int sysIndex, int cpuIndex, int almLevel, int occurFlag)
{
	int	almCode;
	char	condBuf[4096], tmpBuf[1024];
	char	occur_or_clear[12], levelSymbol[4];

	//sjjeon almCode = ALMCODE_SFM_CPU_USAGE;
	almCode = getAlarmCodeFromType(SFM_ALM_TYPE_CPU_USAGE, almLevel);

	if (occurFlag) {
		sprintf (occur_or_clear, "OCCURED");
		switch (almLevel) {
			case SFM_ALM_MINOR:    sprintf (levelSymbol, "*"); break;
			case SFM_ALM_MAJOR:    sprintf (levelSymbol, "**"); break;
			case SFM_ALM_CRITICAL: sprintf (levelSymbol, "***"); break;
			default:               sprintf (levelSymbol, " "); break;
		}
	} else {
		sprintf (occur_or_clear, "CLEARED");
		switch (almLevel) {
			case SFM_ALM_MINOR:    sprintf (levelSymbol, "#"); break;
			case SFM_ALM_MAJOR:    sprintf (levelSymbol, "##"); break;
			case SFM_ALM_CRITICAL: sprintf (levelSymbol, "###"); break;
			default:               sprintf (levelSymbol, " "); break;
		}
	}

	sprintf(condBuf,"    %s %s\n%3s A%04d CPU LOAD ALARM %s\n",
			sysLabel,	// system_name
			commlib_printTStamp(),	// 현재시각 time stamp (년,월,일,시,분,초,요일)
			levelSymbol, almCode, occur_or_clear);

	sprintf(tmpBuf,"      SYSTEM   = %s\n      RESOURCE = CPU\n      LEVEL    = %s (%d%%)\n      COMPLETED\n\n\n",
			sfdb->sys[sysIndex].commInfo.name,
			fimd_printAlmLevel ((unsigned char)almLevel),
			sfdb->sys[sysIndex].commInfo.cpuInfo.usage[cpuIndex]/10);
	strcat (condBuf, tmpBuf);

	// cond로 보낸다.
	// - cond가 로그파일에 기록할때, MMC/STATUS/ALARM 메시지별 구분할 수 있도록
	//	mtype을 표시해 보낸다.
	fimd_txMsg2Cond (condBuf, MTYPE_ALARM_REPORT, almCode);

	return almLevel;

} //----- End of fimd_makeCpuUsageAlmMsg -----//

//------------------------------------------------------------------------------
// 20100916 by dcham 
// - SCM Fault 상태 cond로 전송  
//------------------------------------------------------------------------------
int fimd_makeSCMFaultStsAlmMsg (int sysIndex, int almLevel, int occurFlag)
{
	int almCode;
	char    condBuf[4096], tmpBuf[1024];
	// RESOURCE내용 추가 20101003 by dch
	char    occur_or_clear[12], levelSymbol[4], scmNormalYn[9];
	almCode = getAlarmCodeFromType(SFM_ALM_TYPE_SCM_FAULTED, almLevel);

    if (occurFlag) {
       sprintf (scmNormalYn, "ABNORMAL");
       sprintf (occur_or_clear, "OCCURED");
       sprintf (levelSymbol, "***");
     }
    else {
        sprintf (scmNormalYn, "NORMAL");
        sprintf (occur_or_clear, "CLEARED");
        sprintf (levelSymbol, "###");
    }

    sprintf(condBuf,"    %s %s\n%3s A%04d SCM FAULTED ALARM %s\n",
                    sfdb->sys[sysIndex].commInfo.name,   // system_name
                    commlib_printTStamp(),  // 현재시각 time stamp (년,월,일,시,분,초,요일)
                    levelSymbol, almCode, occur_or_clear);

    sprintf(tmpBuf,"      SYSTEM = %s\n      RESOURCE = %s\n      STATUS = %s\n      COMPLETED\n\n\n",
                    sfdb->sys[sysIndex].commInfo.name,
                    SQL_SCM_ALM_INFO, //SCM_FAULTED=>SCM_STATUS, 20101003 by dcham
                    scmNormalYn);
    strcat (condBuf, tmpBuf);

   // cond로 보낸다.
   // - cond가 로그파일에 기록할때, MMC/STATUS/ALARM 메시지별 구분할 수 있도록
   // mtype을 표시해 보낸다.                                                                                                                                                                
									                                                                                                                                                                                             
   fimd_txMsg2Cond (condBuf, MTYPE_ALARM_REPORT, almCode);                                                                                                                                  
											                                                                                                                                                                                             
   return almLevel;                                                                                                                                                                         
												                                                                                                                                                                                             
												                                                                                                                                                                                             
} //----- End of fimd_makeCpuUsageAlmMsg -----//

//------------------------------------------------------------------------------
// 메모리 점유율 장애 발생/해지 시 호출되어 장애 메시지를 만들어 cond로 보낸다.
// - 장애 등급 및 발생/해지 여부는 이미 결정된 상태로 호출된다.
//------------------------------------------------------------------------------
int	fimd_makeMemUsageAlmMsg (int sysIndex, int almLevel, int occurFlag)
{
	int		almCode;
	char	condBuf[4096], tmpBuf[1024];
	char	occur_or_clear[12], levelSymbol[4];

	//sjjeon almCode = ALMCODE_SFM_MEMORY_USAGE;
	almCode = getAlarmCodeFromType(SFM_ALM_TYPE_MEMORY_USAGE, almLevel);

	if (occurFlag) {
		sprintf (occur_or_clear, "OCCURED");
		switch (almLevel) {
			case SFM_ALM_MINOR:    sprintf (levelSymbol, "*"); break;
			case SFM_ALM_MAJOR:    sprintf (levelSymbol, "**"); break;
			case SFM_ALM_CRITICAL: sprintf (levelSymbol, "***"); break;
			default:               sprintf (levelSymbol, " "); break;
		}
	} else {
		sprintf (occur_or_clear, "CLEARED");
		switch (almLevel) {
			case SFM_ALM_MINOR:    sprintf (levelSymbol, "#"); break;
			case SFM_ALM_MAJOR:    sprintf (levelSymbol, "##"); break;
			case SFM_ALM_CRITICAL: sprintf (levelSymbol, "###"); break;
			default:               sprintf (levelSymbol, " "); break;
		}
	}

	sprintf(condBuf,"    %s %s\n%3s A%04d MEMORY LOAD ALARM %s\n",
			sysLabel,	// system_name
			commlib_printTStamp(),	// 현재시각 time stamp (년,월,일,시,분,초,요일)
			levelSymbol, almCode, occur_or_clear);

	sprintf(tmpBuf,"      SYSTEM   = %s\n      RESOURCE = MEMORY\n      LEVEL    = %s (%d%%)\n      COMPLETED\n\n\n",
			sfdb->sys[sysIndex].commInfo.name,
			fimd_printAlmLevel ((unsigned char)almLevel),
			sfdb->sys[sysIndex].commInfo.memInfo.usage/10);
	strcat (condBuf, tmpBuf);

	// cond로 보낸다.
	// - cond가 로그파일에 기록할때, MMC/STATUS/ALARM 메시지별 구분할 수 있도록
	//	mtype을 표시해 보낸다.
	fimd_txMsg2Cond (condBuf, MTYPE_ALARM_REPORT, almCode);

	return almLevel;

} //----- End of fimd_makeMemUsageAlmMsg -----//



//------------------------------------------------------------------------------
// 디스크 사용율 장애 발생/해지 시 호출되어 장애 메시지를 만들어 cond로 보낸다.
// - 장애 등급 및 발생/해지 여부는 이미 결정된 상태로 호출된다.
//------------------------------------------------------------------------------
int	fimd_makeDiskUsageAlmMsg (int sysIndex, int diskIndex, int almLevel, int occurFlag)
{
	int	almCode, i, usage=0;
	char	condBuf[4096], tmpBuf[1024];
	char	occur_or_clear[12], levelSymbol[4];

	//sjjeon almCode = ALMCODE_SFM_DISK_USAGE;
	almCode = getAlarmCodeFromType(SFM_ALM_TYPE_DISK_USAGE, almLevel);

	for (i=0; i<strlen(sfdb->sys[sysIndex].commInfo.diskInfo[diskIndex].name); i++)  
		sfdb->sys[sysIndex].commInfo.diskInfo[diskIndex].name[i]  = toupper(sfdb->sys[sysIndex].commInfo.diskInfo[diskIndex].name[i]);

	usage = sfdb->sys[sysIndex].commInfo.diskInfo[diskIndex].usage / 10;	
//	if(!strcasecmp(sfdb->sys[sysIndex].commInfo.name, "DSCM"))
//		usage = sfdb->sys[sysIndex].commInfo.diskInfo[diskIndex].usage / 10;	
//	else
//		usage = sfdb->sys[sysIndex].commInfo.diskInfo[diskIndex].usage;

	if (occurFlag) {
		sprintf (occur_or_clear, "OCCURED");
		switch (almLevel) {
			case SFM_ALM_MINOR:    sprintf (levelSymbol, "*"); break;
			case SFM_ALM_MAJOR:    sprintf (levelSymbol, "**"); break;
			case SFM_ALM_CRITICAL: sprintf (levelSymbol, "***"); break;
			default:               sprintf (levelSymbol, " "); break;
		}
	} else {
		sprintf (occur_or_clear, "CLEARED");
		switch (almLevel) {
			case SFM_ALM_MINOR:    sprintf (levelSymbol, "#"); break;
			case SFM_ALM_MAJOR:    sprintf (levelSymbol, "##"); break;
			case SFM_ALM_CRITICAL: sprintf (levelSymbol, "###"); break;
			default:               sprintf (levelSymbol, " "); break;
		}
	}

	sprintf(condBuf,"    %s %s\n%3s A%04d DISK LOAD ALARM %s\n",
			sysLabel,	// system_name
			commlib_printTStamp(),	// 현재시각 time stamp (년,월,일,시,분,초,요일)
			levelSymbol, almCode, occur_or_clear);

	sprintf(tmpBuf,"      SYSTEM   = %s\n      RESOURCE = %s\n      LEVEL    = %s (%d%%)\n      COMPLETED\n\n\n",
			sfdb->sys[sysIndex].commInfo.name,
			sfdb->sys[sysIndex].commInfo.diskInfo[diskIndex].name, // disk partition name
			fimd_printAlmLevel ((unsigned char)almLevel),
			usage);
	strcat (condBuf, tmpBuf);

	// cond로 보낸다.
	// - cond가 로그파일에 기록할때, MMC/STATUS/ALARM 메시지별 구분할 수 있도록
	//	mtype을 표시해 보낸다.
	fimd_txMsg2Cond (condBuf, MTYPE_ALARM_REPORT, almCode);

	return almLevel;

} //----- End of fimd_makeDiskUsageAlmMsg -----//



//------------------------------------------------------------------------------
// LAN 장애 발생/해지 시 호출되어 장애 메시지를 만들어 cond로 보낸다.
// - 상태 변경이 감지되었을때 호출되고, 장애 발생인지 해지인지와 등급을 결정해야한다.
//------------------------------------------------------------------------------
int	fimd_makeLanAlmMsg (int sysIndex, int lanIndex, int *almLevel, int *occurFlag1)
{
	int		almCode, i;
	char	condBuf[4096], tmpBuf[1024], lanSts[5];
	char	occur_or_clear[12], levelSymbol[4];

	*almLevel = SFM_ALM_CRITICAL; // LAN 장애는 무조건 critical로 설정한다.
	//sjjeon almCode = ALMCODE_SFM_LAN;
	almCode = getAlarmCodeFromType(SFM_ALM_TYPE_LAN, *almLevel);

	for (i=0; i<strlen(sfdb->sys[sysIndex].commInfo.lanInfo[lanIndex].name); i++){  
		sfdb->sys[sysIndex].commInfo.lanInfo[lanIndex].name[i]  = toupper(sfdb->sys[sysIndex].commInfo.lanInfo[lanIndex].name[i]);
	}
	if (sfdb->sys[sysIndex].commInfo.lanInfo[lanIndex].status == SFM_LAN_DISCONNECTED) {
		//printf("sts_D: %d\n", sfdb->sys[sysIndex].commInfo.lanInfo[lanIndex].status);
		sprintf (occur_or_clear, "OCCURED");
		sprintf (levelSymbol, "***");
		*occurFlag1 = 1;
		sprintf(lanSts, "DOWN"); // by helca 08.02
	} else if (sfdb->sys[sysIndex].commInfo.lanInfo[lanIndex].status == SFM_LAN_CONNECTED)
	{
		//printf("sts_c: %d\n", sfdb->sys[sysIndex].commInfo.lanInfo[lanIndex].status);
		sprintf (occur_or_clear, "CLEARED");
		sprintf (levelSymbol, "###");
		*occurFlag1 = 0;
		sprintf(lanSts, "UP"); // by helca 08.02
	}

	sprintf(condBuf,"    %s %s\n%3s A%04d MANAGEMENT NETWORK LINK ALARM %s\n",
			sysLabel,	// system_name
			commlib_printTStamp(),	// 현재시각 time stamp (년,월,일,시,분,초,요일)
			levelSymbol, almCode, occur_or_clear);

	sprintf(tmpBuf,"      SYSTEM = %s\n      TARGET = %s(%s)\n      STATUS = %s\n      COMPLETED\n\n\n",
			sfdb->sys[sysIndex].commInfo.name,
			sfdb->sys[sysIndex].commInfo.lanInfo[lanIndex].name,
			sfdb->sys[sysIndex].commInfo.lanInfo[lanIndex].targetIp,
			lanSts); // by helca 08.02
	//		fimd_printLanStatus (sfdb->sys[sysIndex].commInfo.lanInfo[lanIndex].status));
	strcat (condBuf, tmpBuf);

	// cond로 보낸다.
	// - cond가 로그파일에 기록할때, MMC/STATUS/ALARM 메시지별 구분할 수 있도록
	//	mtype을 표시해 보낸다.
	fimd_txMsg2Cond (condBuf, MTYPE_ALARM_REPORT, almCode);
	return 1;
} //----- End of fimd_makeLanAlmMsg -----//

//------------------------------------------------------------------------------
// CPS Over 장애 발생/해지 시 호출되어 장애 메시지를 만들어 cond로 보낸다.
// - 상태 변경이 감지되었을때 호출되고, 장애 발생인지 해지인지와 등급을 결정해야한다.
//------------------------------------------------------------------------------
int	fimd_makeCpsOverAlmMsg(int sysIndex, int lanIndex, int *almLevel, int *occurFlag1)
{
	int		almCode;
	char	condBuf[4096], tmpBuf[1024], cpsSts[5];
	char	occur_or_clear[12], levelSymbol[4];

	*almLevel = SFM_ALM_MINOR;  
	//almCode = ALMCODE_SFM_CPS_OVER_INFO;
	almCode = getAlarmCodeFromType(SFM_ALM_TYPE_CPS_OVER, *almLevel);

	if (sfdb->sys[sysIndex].commInfo.cpsOverSts.status == SFM_CPS_OVER) {
		//printf("cps over sts: %d\n", sys[sysIndex].commInfo.cpsOverSts.status);
		sprintf (occur_or_clear, "OCCURED");
		sprintf (levelSymbol, "*");
		*occurFlag1 = 1;
		sprintf(cpsSts, "OVER"); // by helca 08.02
	} else if (sfdb->sys[sysIndex].commInfo.lanInfo[lanIndex].status == SFM_LAN_CONNECTED)
	{
		//printf("sts_c: %d\n", sfdb->sys[sysIndex].commInfo.lanInfo[lanIndex].status);
		sprintf (occur_or_clear, "CLEARED");
		sprintf (levelSymbol, "#");
		*occurFlag1 = 0;
		sprintf(cpsSts, "NORMAL"); // by helca 08.02
	}

	sprintf(condBuf,"    %s %s\n%3s A%04d MANAGEMENT CPS STATUS ALARM %s\n",
			sysLabel,	// system_name
			commlib_printTStamp(),	// 현재시각 time stamp (년,월,일,시,분,초,요일)
			levelSymbol, almCode, occur_or_clear);

	sprintf(tmpBuf,"      SYSTEM = %s\n      CPS STATUS = %s\n      COMPLETED\n\n\n",
			sfdb->sys[sysIndex].commInfo.name,
			cpsSts); // by sjjeon

	strcat (condBuf, tmpBuf);

	// cond로 보낸다.
	// - cond가 로그파일에 기록할때, MMC/STATUS/ALARM 메시지별 구분할 수 있도록
	//	mtype을 표시해 보낸다.
	fimd_txMsg2Cond (condBuf, MTYPE_ALARM_REPORT, almCode);

	return 1;
} //----- End of fimd_makeLanAlmMsg -----//

//------------------------------------------------------------------------------
// S/W Process 장애 발생/해지 시 호출되어 장애 메시지를 만들어 cond로 보낸다.
// - 상태 변경이 감지되었을때 호출되고, 장애 발생인지 해지인지와 등급을 결정해야한다.
//------------------------------------------------------------------------------
int	fimd_makeProcAlmMsg (int sysIndex, int procIndex, int *almLevel, int *occurFlag, int watchDogFlag)
{
	int		almCode, i=0;
	char	condBuf[4096], tmpBuf[1024];
	char	occur_or_clear[12], levelSymbol[4];

#ifndef _NOT_USED_PROCESS_ALARM_TYPE_
	int almType;
	/* process 이름으로 Alarm type 획득*/
	almType = getAlarmTypeFromProcName(sysIndex, procIndex);
	/* Alarm type으로 Alarm code 획득 */
	//almCode = getAlarmCodeFromType(almType);
	almCode = getAlarmCodeFromType(almType, sfdb->sys[sysIndex].commInfo.procInfo[procIndex].level);
	if(almCode<0){
		sprintf(trcBuf,"[%s] Invalid alarm code.(%d)\n", __FUNCTION__, almCode);
		return -1;
	}
//	fprintf(stderr,"[%s] sysIndex: %d, procIndex: %d\n",__FUNCTION__, sysIndex, procIndex);
//	fprintf(stderr,"[%s] almType : %d, almCode : %d\n",__FUNCTION__, almType, almCode);
#else
	almCode = ALMCODE_SFM_PROCESS;
#endif
	for (i=0; i<strlen(sfdb->sys[sysIndex].commInfo.procInfo[procIndex].name); i++) {
		sfdb->sys[sysIndex].commInfo.procInfo[procIndex].name[i]  = toupper(sfdb->sys[sysIndex].commInfo.procInfo[procIndex].name[i]);
	}
	if (sfdb->sys[sysIndex].commInfo.procInfo[procIndex].status == SFM_STATUS_DEAD) {
		sprintf (occur_or_clear, "OCCURED");
		*occurFlag = 1;
		switch (sfdb->sys[sysIndex].commInfo.procInfo[procIndex].level) {
			case SFM_ALM_MINOR:    sprintf (levelSymbol, "*"); *almLevel = SFM_ALM_MINOR; break;
			case SFM_ALM_MAJOR:    sprintf (levelSymbol, "**"); *almLevel = SFM_ALM_MAJOR; break;
			case SFM_ALM_CRITICAL: sprintf (levelSymbol, "***"); *almLevel = SFM_ALM_CRITICAL; break;
			default:               sprintf (levelSymbol, " "); *almLevel = SFM_ALM_NORMAL; break;
		}
	} else {
		sprintf (occur_or_clear, "CLEARED");
		*occurFlag = 0;
		switch (sfdb->sys[sysIndex].commInfo.procInfo[procIndex].level) {
			case SFM_ALM_MINOR:    sprintf (levelSymbol, "#"); *almLevel = SFM_ALM_MINOR; break;
			case SFM_ALM_MAJOR:    sprintf (levelSymbol, "##"); *almLevel = SFM_ALM_MAJOR; break;
			case SFM_ALM_CRITICAL: sprintf (levelSymbol, "###"); *almLevel = SFM_ALM_CRITICAL; break;
			default:               sprintf (levelSymbol, " "); *almLevel = SFM_ALM_NORMAL; break;
		}
	}

	sprintf(condBuf,"    %s %s\n%3s A%04d S/W PROCESS ALARM %s\n",
			sysLabel,	// system_name
			commlib_printTStamp(),	// 현재시각 time stamp (년,월,일,시,분,초,요일)
			levelSymbol, almCode, occur_or_clear);

	if (watchDogFlag) {
		sprintf(tmpBuf,"      SYSTEM  = %s\n      PROCESS = %s\n      STATUS  = %s\n      INFORM  = APPLICATION NO RESPONSE\n      COMPLETED\n\n\n",
				sfdb->sys[sysIndex].commInfo.name,
				sfdb->sys[sysIndex].commInfo.procInfo[procIndex].name,
				fimd_printProcStatus (sfdb->sys[sysIndex].commInfo.procInfo[procIndex].status));
	} else {
		sprintf(tmpBuf,"      SYSTEM  = %s\n      PROCESS = %s\n      STATUS  = %s\n      COMPLETED\n\n\n",
				sfdb->sys[sysIndex].commInfo.name,
				sfdb->sys[sysIndex].commInfo.procInfo[procIndex].name,
				fimd_printProcStatus (sfdb->sys[sysIndex].commInfo.procInfo[procIndex].status));
	}
	strcat (condBuf, tmpBuf);

	// cond로 보낸다.
	// - cond가 로그파일에 기록할때, MMC/STATUS/ALARM 메시지별 구분할 수 있도록
	//	mtype을 표시해 보낸다.
	fimd_txMsg2Cond (condBuf, MTYPE_ALARM_REPORT, almCode);

	return 1;

} //----- End of fimd_makeProcAlmMsg -----//

int is_lan_dev(char *hwname)
{
	int i;

	for (i=0 ; i< lanConf.count; i++)
	{
		if( !strncasecmp(hwname, lanConf.lanif[i].name, lanConf.lanif[i].name_size) ) {
			return 1;
		}
	}
	return 0;
}

//------------------------------------------------------------------------------
// H/W 상태 변경 시 호출되어 단순 상태 변경인지 장애 발생,해지 인지 판단하고
//	장애 또는 상태 메시지를 만들어 cond로 보낸다.
//------------------------------------------------------------------------------
// by helca 08.03 
int	fimd_makeHwComAlmMsg (int sysIndex, SFM_HpUxHWInfo *hwInfo, int comIndex, int *almFlag, int *almLevel, int *occurFlag)
{
	int		almCode, len;
	char	condBuf[4096], tmpBuf[1024], hwnama[16];
	char	occur_or_clear[12], levelSymbol[4], hwSts[10];

	// 어느 상태로 변경되었는지 확인하여 단순 상태변경인지 장애 발생,해지 인지 판단한다.
	//
	// by helca 08.07 상태 변경의 경우만 판단한다.

	len = strlen(hwInfo->hwcom[comIndex].name);
	hw_name_mapping(hwInfo->hwcom[comIndex].name, len, hwnama);

	if (is_lan_dev(hwInfo->hwcom[comIndex].name)) {
		if (hwInfo->hwcom[comIndex].status == 0)
			sprintf(hwSts, "UP");
		else
			sprintf(hwSts, "DOWN");
		printf("hwInfo->hwcom[%d].name : %s, hwSts: %s \n",comIndex, hwInfo->hwcom[comIndex].name, hwSts);
	}
	else{
		sprintf(hwSts, "%s", fimd_printHWStatus(hwInfo->hwcom[comIndex].status));
	}

	switch (hwInfo->hwcom[comIndex].prevStatus)
	{
		case SFM_HW_UP:
			switch (hwInfo->hwcom[comIndex].status) {
				case SFM_HW_DOWN:
					*almFlag = 1;
					*occurFlag = 1;
					break;
			}
			break;
		case SFM_HW_DOWN:
			switch (hwInfo->hwcom[comIndex].status) {
				case SFM_HW_UP:
					*almFlag = 1;
					*occurFlag = 0;
			}
			break;
				
	}
	
	
	if (*almFlag) {
		// 장애 발생,해지 이면 장애 등급을 결정한다.
		//
		if (*occurFlag) {
			sprintf (occur_or_clear, "OCCURED");
			switch (hwInfo->hwcom[comIndex].level) {
				case SFM_ALM_MINOR:    sprintf (levelSymbol, "*"); *almLevel = SFM_ALM_MINOR; break;
				case SFM_ALM_MAJOR:    sprintf (levelSymbol, "**"); *almLevel = SFM_ALM_MAJOR; break;
				case SFM_ALM_CRITICAL: sprintf (levelSymbol, "***"); *almLevel = SFM_ALM_CRITICAL; break;
				default:               sprintf (levelSymbol, " "); *almLevel = SFM_ALM_NORMAL; break;
			}
		}
		else {
			sprintf (occur_or_clear, "CLEARED");
			switch (hwInfo->hwcom[comIndex].level) {
				case SFM_ALM_MINOR:    sprintf (levelSymbol, "#"); *almLevel = SFM_ALM_MINOR; break;
				case SFM_ALM_MAJOR:    sprintf (levelSymbol, "##"); *almLevel = SFM_ALM_MAJOR; break;
				case SFM_ALM_CRITICAL: sprintf (levelSymbol, "###"); *almLevel = SFM_ALM_CRITICAL; break;
				default:               sprintf (levelSymbol, " "); *almLevel = SFM_ALM_NORMAL; break;
			}
		}
	}

	if (*almFlag) {
		// HW MIRROR
		if(g_hwswType[comIndex] == LOC_HW_MIRROR_TYPE){
			//almCode = ALMCODE_SFM_MIRROR_PORT;
			almCode = getAlarmCodeFromType(SFM_ALM_TYPE_HW_MIRROR, *almLevel);
		}else{
			//almCode = ALMCODE_SFM_HPUX_HW;
			almCode = getAlarmCodeFromType(SFM_ALM_TYPE_MP_HW, *almLevel);
		}
		
	//	sprintf(condBuf,"    %s %s\n%3s A%04d DSC H/W ALARM %s\n",
		sprintf(condBuf,"    %s %s\n%3s A%04d DSC ALARM %s\n",
				sysLabel, commlib_printTStamp(),
				levelSymbol, almCode, occur_or_clear);
		
		sprintf(tmpBuf,"      SYSTEM   = %s\n      RESOURCE = %s\n      STATUS   = %s\n      COMPLETED\n\n\n",
				sfdb->sys[sysIndex].commInfo.name,
				hwnama,
				hwSts);
		strcat (condBuf, tmpBuf);
		// cond로 보낸다.
		fimd_txMsg2Cond (condBuf, MTYPE_ALARM_REPORT, almCode);
	}

	return 1;

} //----- End of fimd_makeHwComAlmMsg -----//


//------------------------------------------------------------------------------
// 20040921-by mnpark 
// DB Replication Fail로 MP Process를 Kill시키는 경우 Alarm message 전송 
//------------------------------------------------------------------------------
int	fimd_makeKillprcAlmMsg (int almReasonType, int sysIndex, int procIdx, int altibaseIdx)
{
	int		almCode;
	char	condBuf[4096], tmpBuf[1024];
	char	occur_or_clear[12], levelSymbol[4];
	char	reason[128];
	char*	procName;

	//sjjeon almCode = ALMCODE_SFM_PROCESS;
	almCode = getAlarmCodeFromType(SFM_ALM_TYPE_PROC, SFM_ALM_CRITICAL); //

	sprintf (occur_or_clear, "OCCURED");
	sprintf (levelSymbol, "***"); 

	sprintf(condBuf,"    %s %s\n%3s A%04d S/W PROCESS ALARM %s\n",
			sysLabel, 	// system_name
			commlib_printTStamp(),	// 현재시각 time stamp (년,월,일,시,분,초,요일)
			levelSymbol, almCode, occur_or_clear);

	/* alarm message reason */
	memset(reason, 0x00, sizeof(reason));
	switch(almReasonType) 
	{
		case ALMCODE_SFM_PROCESS:	/* ALTIBASE Shutdown */
			sprintf(reason, "%s IS %s",	/* ALTIBASE IS DEAD */
				sfdb->sys[sysIndex].commInfo.procInfo[altibaseIdx].name,
				fimd_printProcStatus (sfdb->sys[sysIndex].commInfo.procInfo[altibaseIdx].status));
			break;
		case ALMCODE_SFM_DB_REP:	/* replication 장애 */
			sprintf(reason, "DB REPLICATION FAIL");
			break;
	}

	procName = sfdb->sys[sysIndex].commInfo.procInfo[procIdx].name;
	// yhshin 
	// SM / CM은 process alram에서 제외 
	// 현재 CM/SM은 H/W Alram으로 등록되어 있음. 
	if (!strcmp(procName, "CM") || !strcmp(procName, "SM"))
		return 1;

	sprintf(tmpBuf,"      SYSTEM  = %s\n      PROCESS = %s\n      INFORM  = KILL-PRC BY FIMD\n      REASON  = %s\n      COMPLETED\n\n\n",
			sfdb->sys[sysIndex].commInfo.name, procName, reason);

	strcat (condBuf, tmpBuf);

	// cond로 보낸다.
	// - cond가 로그파일에 기록할때, MMC/STATUS/ALARM 메시지별 구분할 수 있도록
	//	mtype을 표시해 보낸다.
	fimd_txMsg2Cond (condBuf, MTYPE_ALARM_REPORT, almCode);

	return 1;

} //----- End of fimd_makeKillprcAlmMsg -----//

//------------------------------------------------------------------------------
// 20040921-by mnpark
// DB Replication Fail로 MP Process를 Kill시키는 경우 Alarm message 전송
//------------------------------------------------------------------------------
int fimd_makeActiveAlmMsg (char *preActStats, char *actStats, int occurFlag)
{
	int		almCode;
	char	preActName[12];
	char    condBuf[4096], tmpBuf[1024];
	char    occur_or_clear[12], levelSymbol[4];
	char	tmp[6];

// 공유메모리의 값을 받아 온다
//공유메모리 값이 언제 바뀔지 모르기 때문
	sprintf(preActName, preActStats);

	if(occurFlag){
		sprintf (occur_or_clear, "OCCURED");
		sprintf (levelSymbol, "***");
	}
	else {
		sprintf (occur_or_clear, "CLEARED");
		sprintf (levelSymbol, "###");
	}

	//sjjeon almCode = ALMCODE_SFM_ACTIVE;
	almCode = getAlarmCodeFromType(SFM_ALM_TYPE_ACTIVE_STS, SFM_ALM_CRITICAL);  // almCode 1700

	sprintf(condBuf,"    ACTIVE-SYS %s\n%3s A%04d ACTIVE/STANDBY ALARM %s\n",
				commlib_printTStamp(),  // 현재시각 time stamp (년,월,일,시,분,초,요일)
				levelSymbol, almCode, occur_or_clear);

	/* alarm message reason */
	memset(tmpBuf, 0x00, 1024);

	if(occurFlag){
		if(!strcasecmp(actStats, "ACTIVE"))
			if(!strcasecmp(preActName, "SCMA"))
				sprintf(tmp, "SCMB");
			else
				sprintf(tmp, "SCMA");
		else if(!strcasecmp(actStats, "STANDBY"))
			sprintf(tmp, preActName);

//		sprintf(tmpBuf, "\n  %s IS GOING TO %s\n  BOTH NODE ARE %s\n\nCOMPLETED\n\n\n", preActName, actStats, actStats);
		sprintf(tmpBuf, "\n      %s IS GOING TO %s\n      BOTH NODE ARE %s\n\n      COMPLETED\n\n\n", tmp, actStats, actStats);
	}else
		sprintf(tmpBuf, "\n      %s IS ACTIVE\n\nCOMPLETED\n\n\n", actStats);

	strcat (condBuf, tmpBuf);

	// cond로 보낸다.
	// - cond가 로그파일에 기록할때, MMC/STATUS/ALARM 메시지별 구분할 수 있도록
	//  mtype을 표시해 보낸다.
	fimd_txMsg2Cond (condBuf, MTYPE_ALARM_REPORT, almCode);

	return 1;

}

int fimd_makeSmsAlmMsg(int almLevel, char almMsg[512])
{
    int     almCode;
    char    condBuf[4096];
    char    occur_or_clear[12], levelSymbol[4];

	memset(condBuf, 0, 4096);

	if( almLevel != SFM_ALM_NORMAL ) {
		sprintf (occur_or_clear, "OCCURED");
		switch(almLevel) {
		case SFM_ALM_MINOR:    sprintf (levelSymbol, "*"); break;
		case SFM_ALM_MAJOR:    sprintf (levelSymbol, "**"); break;
		case SFM_ALM_CRITICAL: sprintf (levelSymbol, "***"); break;
		}
	}
	else {
		sprintf (occur_or_clear, "CLEARED");
		switch(almLevel) {
		case SFM_ALM_MINOR:    sprintf (levelSymbol, "#"); break;
		case SFM_ALM_MAJOR:    sprintf (levelSymbol, "##"); break;
		case SFM_ALM_CRITICAL: sprintf (levelSymbol, "###"); break;
		}
	}

    almCode = ALMCODE_SFM_BSD;

    sprintf(condBuf, "\n%s\n\n", almMsg);

    // cond로 보낸다.
    // - cond가 로그파일에 기록할때, MMC/STATUS/ALARM 메시지별 구분할 수 있도록
    //  mtype을 표시해 보낸다.

    fimd_txMsg2Cond (condBuf, MTYPE_ALARM_REPORT, almCode);

    return 1;
}

//------------------------------------------------------------------------------
// CallInfo 장애 발생/해지 시 호출되어 장애 메시지를 만들어 cond로 보낸다.
// - 장애 등급 및 발생/해지 여부는 이미 결정된 상태로 호출된다.
//------------------------------------------------------------------------------
int fimd_makeCallInfoAlmMsg(int sysIndex, int almLevel, int occurFlag)
{
    int     almCode;
    char    condBuf[4096], tmpBuf[1024];
    char    occur_or_clear[12], levelSymbol[4];

    //sjjeon almCode = ALMCODE_SFM_CALL_INFO;
	almCode = getAlarmCodeFromType(SFM_ALM_TYPE_CALL_INFO, almLevel);

    if (occurFlag) {
        sprintf (occur_or_clear, "OCCURED");
        sprintf (levelSymbol, "***");
    } else {
        sprintf (occur_or_clear, "CLEARED");
        sprintf (levelSymbol, " ");
    }

    sprintf(condBuf,"    %s %s\n%3s A%04d CALL INFO ALARM %s\n",
            sysLabel,    // system_name
            commlib_printTStamp(),  // 현재시각 time stamp (년,월,일,시,분,초,요일)
            levelSymbol, almCode, occur_or_clear);

    sprintf(tmpBuf,"      SYSTEM   = %s\n      RESOURCE = CALL\n      LEVEL    = %s\n    COMPLETED\n\n\n",
            sfdb->sys[sysIndex].commInfo.name,
            fimd_printAlmLevel ((unsigned char)almLevel));
    strcat (condBuf, tmpBuf);

    // cond로 보낸다.
    // - cond가 로그파일에 기록할때, MMC/STATUS/ALARM 메시지별 구분할 수 있도록
    //  mtype을 표시해 보낸다.
    fimd_txMsg2Cond (condBuf, MTYPE_ALARM_REPORT, almCode);

    return almLevel;

} //----- End of fimd_makeCalllnfoAlmMsg -----//

//---------------------------------------//
//---------------------------------------//
/* by helca */

int	fimd_makeRmtLanAlmMsg (int sysIndex, int lanIndex, int *almLevel, int *occurFlag)
{
	int	almCode, i;
	char	condBuf[4096], tmpBuf[1024];
	char	occur_or_clear[12], levelSymbol[4], rmtlanSts[5];

	*almLevel = SFM_ALM_CRITICAL; // LAN 장애는 무조건 critical로 설정한다. sjjeon
	almCode = getAlarmCodeFromType(SFM_ALM_TYPE_RMT_LAN, *almLevel);

	for (i=0; i<strlen(sfdb->sys[sysIndex].commInfo.rmtLanInfo[lanIndex].name); i++){  
		sfdb->sys[sysIndex].commInfo.rmtLanInfo[lanIndex].name[i]  = toupper(sfdb->sys[sysIndex].commInfo.rmtLanInfo[lanIndex].name[i]);
	}

	if (sfdb->sys[sysIndex].commInfo.rmtLanInfo[lanIndex].status == SFM_LAN_DISCONNECTED) {
		sprintf (occur_or_clear, "OCCURED");
		sprintf (levelSymbol, "***");
		*occurFlag = 1;
		sprintf(rmtlanSts, "DOWN"); // by helca 08.02
	} else {
		sprintf (occur_or_clear, "CLEARED");
		sprintf (levelSymbol, "###");
		*occurFlag = 0;
		sprintf(rmtlanSts, "UP"); // by helca 08.02
	}

	sprintf(condBuf,"    %s %s\n%3s A%04d REMOTE NETWORK LINK ALARM %s\n",
			sysLabel,	// system_name
			commlib_printTStamp(),	// 현재시각 time stamp (년,월,일,시,분,초,요일)
			levelSymbol, almCode, occur_or_clear);

	sprintf(tmpBuf,"      SYSTEM = %s\n      TARGET = %s(%s)\n      STATUS = %s\n      COMPLETED\n\n\n",
			sfdb->sys[sysIndex].commInfo.name,
			//&sfdb->sys[sysIndex].commInfo.rmtLanInfo[lanIndex].name[1],
			&sfdb->sys[sysIndex].commInfo.rmtLanInfo[lanIndex].name[0], //sjjoen
			sfdb->sys[sysIndex].commInfo.rmtLanInfo[lanIndex].targetIp,
			rmtlanSts); // by helca 08.02
	//		fimd_printLanStatus (sfdb->sys[sysIndex].commInfo.rmtLanInfo[lanIndex].status));
	strcat (condBuf, tmpBuf);

	// cond로 보낸다.
	// - cond가 로그파일에 기록할때, MMC/STATUS/ALARM 메시지별 구분할 수 있도록
	//	mtype을 표시해 보낸다.
	fimd_txMsg2Cond (condBuf, MTYPE_ALARM_REPORT, almCode);

	return 1;

} //----- End of fimd_makeRmtLanAlmMsg -----//

int	fimd_makeOptLanAlmMsg (int sysIndex, int lanIndex, int *almLevel, int *occurFlag)
{
	int		almCode;
	char	condBuf[4096], tmpBuf[1024];
	char	occur_or_clear[12], levelSymbol[4], optName[10], optlanSts[5];

	*almLevel = SFM_ALM_CRITICAL; // LAN 장애는 무조건 critical로 설정한다.
	//almCode = ALMCODE_SFM_OPT_LAN;
	almCode = getAlarmCodeFromType(SFM_ALM_TYPE_OPT_LAN, *almLevel);	

	if(!strcasecmp(sfdb->sys[sysIndex].commInfo.optLanInfo[lanIndex].name, "TAP-A")) sprintf(optName, "MIRROR_A");
	else if(!strcasecmp(sfdb->sys[sysIndex].commInfo.optLanInfo[lanIndex].name, "TAB-B")) sprintf(optName, "MIRROR_B");

	if (sfdb->sys[sysIndex].commInfo.optLanInfo[lanIndex].status == SFM_LAN_DISCONNECTED) {
		sprintf (occur_or_clear, "OCCURED");
		sprintf (levelSymbol, "***");
		*occurFlag = 1;
		sprintf(optlanSts, "DOWN"); // by helca 08.02
	} else {
		sprintf (occur_or_clear, "CLEARED");
		sprintf (levelSymbol, "###");
		*occurFlag = 0;
		sprintf(optlanSts, "UP");  // by helca 08.02
	}

	sprintf(condBuf,"    %s %s\n%3s A%04d MIRRORING PORT ALARM %s\n",
			sysLabel,	// system_name
			commlib_printTStamp(),	// 현재시각 time stamp (년,월,일,시,분,초,요일)
			levelSymbol, almCode, occur_or_clear);

	sprintf(tmpBuf,"      SYSTEM   = %s\n      RESOURCE = %s\n      STATUS   = %s\n      COMPLETED\n\n\n",
			sfdb->sys[sysIndex].commInfo.name,
			optName,
			optlanSts); // by helca 08.02		
	//		fimd_printLanStatus (sfdb->sys[sysIndex].commInfo.optLanInfo[lanIndex].status));
	strcat (condBuf, tmpBuf);

	// cond로 보낸다.
	// - cond가 로그파일에 기록할때, MMC/STATUS/ALARM 메시지별 구분할 수 있도록
	//	mtype을 표시해 보낸다.
	fimd_txMsg2Cond (condBuf, MTYPE_ALARM_REPORT, almCode);

	return 1;

} //----- End of fimd_makeOptLanAlmMsg -----//

int	fimd_makeDupHeartAlmMsg (int sysIndex, SFM_SysDuplicationSts *pDup, int occurFlag)
{
	int	almCode;
	char	condBuf[4096], tmpBuf[1024], heartSts[10];
	char	occur_or_clear[12], levelSymbol[4];

	//almCode = ALMCODE_SFM_HEARTBEAT;
	almCode = getAlarmCodeFromType(SFM_ALM_TYPE_DUP_HEARTBEAT, SFM_ALM_CRITICAL);

	if(occurFlag) {
		sprintf(heartSts, "ABNORMAL");
		sprintf (occur_or_clear, "OCCURED");
		sprintf (levelSymbol, "***");
	} else {
		sprintf(heartSts, "NORMAL");
		sprintf (occur_or_clear, "CLEARED");
		sprintf (levelSymbol, "###");
	}
	sprintf(condBuf,"    %s %s\n%3s A%04d DUPLICATION HEARTBEAT ALARM %s\n",
		sysLabel,	// system_name
		commlib_printTStamp(),	// 현재시각 time stamp (년,월,일,시,분,초,요일)
		levelSymbol, almCode, occur_or_clear);

	sprintf(tmpBuf,"      SYSTEM    = %s\n      HEARTBEAT = %s\n      COMPLETED\n\n\n",
		sfdb->sys[sysIndex].commInfo.name,
		heartSts
		);
	strcat (condBuf, tmpBuf);

	// cond로 보낸다.
	// - cond가 로그파일에 기록할때, MMC/STATUS/ALARM 메시지별 구분할 수 있도록
	//	mtype을 표시해 보낸다.
	fimd_txMsg2Cond (condBuf, MTYPE_ALARM_REPORT, almCode);
	fimd_broadcastAlmEvent2Client ();
	return 1;

} //----- End of fimd_makeDupHeartAlmMsg -----//

int	fimd_makeDupOosAlmMsg (int sysIndex, SFM_SysDuplicationSts *pDup, int *almLevel, int *occurFlag)
{
	int		almCode;
	char	condBuf[4096], tmpBuf[1024];
	char	occur_or_clear[12], levelSymbol[4], oosSts[10];

	*almLevel = SFM_ALM_CRITICAL; 
	//sjjeon almCode = ALMCODE_SFM_OOS;
	almCode = getAlarmCodeFromType(SFM_ALM_TYPE_DUP_OOS, *almLevel);

	if(pDup->OosAlarm == 1) sprintf(oosSts, "NORMAL");
	else if (pDup->OosAlarm == 2) sprintf(oosSts, "ABNORMAL");

	if(sfdb->sys[sysIndex].commInfo.systemDup.oosAlm != pDup->OosAlarm){		
		sprintf (occur_or_clear, "OCCURED");
		sprintf (levelSymbol, "***");
		*occurFlag = 1;
	} else {
		sprintf (occur_or_clear, "CLEARED");
		sprintf (levelSymbol, "###");
		*occurFlag = 0;
	}

	sprintf(condBuf,"    %s %s\n%3s A%04d DUPLICATION OOS ALARM %s\n",
			sysLabel, // system_name
			commlib_printTStamp(),	// 현재시각 time stamp (년,월,일,시,분,초,요일)
			levelSymbol, almCode, occur_or_clear);

	sprintf(tmpBuf,"      SYSTEM = %s\n      OosAlarm = %s\n      COMPLETED\n\n\n",
			sfdb->sys[sysIndex].commInfo.name,
			oosSts
			);
	strcat (condBuf, tmpBuf);

	// cond로 보낸다.
	// - cond가 로그파일에 기록할때, MMC/STATUS/ALARM 메시지별 구분할 수 있도록
	//	mtype을 표시해 보낸다.
	fimd_txMsg2Cond (condBuf, MTYPE_ALARM_REPORT, almCode);

	return 1;

} //----- End of fimd_makeDupOosAlmMsg -----//

int   fimd_makeSuccRateAlmMsg (int sysIndex, int succIndex, int almLevel, int count, int rate, int occurFlag) // by helca 10.20
{
	int	almCode;
	char	condBuf[4096], tmpBuf[1024];
	char	occur_or_clear[12], levelSymbol[4];
	

	//almCode = ALMCODE_SFM_SUCCESS_RATE;
	almCode = getAlarmCodeFromType(SFM_ALM_TYPE_SUCC_RATE, SFM_ALM_CRITICAL);

    	if(occurFlag){
		sprintf (occur_or_clear, "OCCURED");
		sprintf (levelSymbol, "***");
	} else {
		sprintf (occur_or_clear, "CLEARED");
		sprintf (levelSymbol, "###");
	}
	sprintf(condBuf,"    %s %s\n%3s A%04d SUCCESS RATE ALARM %s\n",
			sysLabel,	// system_name
			commlib_printTStamp(),	// 현재시각 time stamp (년,월,일,시,분,초,요일)
			levelSymbol, almCode, occur_or_clear);

	sprintf(tmpBuf, "      SYSTYPE  = %s\n      RESOURCE = %s\n"
			"      INFO     = CURRENT [%d:%d%%] / UNDER THRESHOLD [%d:%d%%]\n"
			"      COMPLETED\n\n\n",
                       	sfdb->sys[sysIndex].commInfo.name,
                       	sfdb->sys[sysIndex].commInfo.succRate[succIndex].name,
                      	count,rate,	
			sfdb->sys[sysIndex].commInfo.succRate[succIndex].cnt, 
			sfdb->sys[sysIndex].commInfo.succRate[succIndex].rate
                       	);
	strcat (condBuf, tmpBuf);

	// cond로 보낸다.
	// - cond가 로그파일에 기록할때, MMC/STATUS/ALARM 메시지별 구분할 수 있도록
	//	mtype을 표시해 보낸다.
	fimd_txMsg2Cond (condBuf, MTYPE_ALARM_REPORT, almCode);

	return 1;

} //----- End of fimd_makeSuccRateAlmMsg -----//

int fimd_makeSuccRateIpAlmMsg (int sysIndex, SuccRateIpInfo succRateIp, SFM_SysSuccRate *succRate, int almLevel, int occurFlag)
{
    int     almCode;
    char    condBuf[4096], tmpBuf[1024];
    char    occur_or_clear[12], levelSymbol[4];
    struct in_addr tIpAddr;

    //almCode = ALMCODE_SFM_SUCCESS_RATE;
	almCode = getAlarmCodeFromType(SFM_ALM_TYPE_SUCC_RATE, almLevel);

    if(occurFlag){
        sprintf (occur_or_clear, "OCCURED");
        sprintf (levelSymbol, "***");
    } else {
        sprintf (occur_or_clear, "CLEARED");
        sprintf (levelSymbol, "###");
    }
    sprintf(condBuf,"    %s %s\n%3s A%04d SUCCESS RATE IP ALARM %s\n",
            sysLabel,   // system_name
            commlib_printTStamp(),  // 현재시각 time stamp (년,월,일,시,분,초,요일)
            levelSymbol, almCode, occur_or_clear);

    tIpAddr.s_addr = succRateIp.ipAddr;
    sprintf(tmpBuf,"      SYSTYPE  = %s\n      RESOURCE = %s(%s)\n"
                   "      INFO     = CURRENT [%d:%d%%] / UNDER THRESHOLD [%d:%d%%]\n"
                   "      COMPLETED\n\n\n",
            sfdb->sys[sysIndex].commInfo.name,
   			succRate->name, inet_ntoa(tIpAddr),
			succRateIp.count, succRateIp.rate,	
			succRate->cnt, succRate->rate);
	strcat (condBuf, tmpBuf);

    // cond로 보낸다.
    // - cond가 로그파일에 기록할때, MMC/STATUS/ALARM 메시지별 구분할 수 있도록
    //  mtype을 표시해 보낸다.
    fimd_txMsg2Cond (condBuf, MTYPE_ALARM_REPORT, almCode);

    return 1;

} //----- End of fimd_makeSuccRateIpAlmMsg -----//
#if 0
int	fimd_makeSessLoadAlmMsg (int sysIndex, unsigned short sess_load, int *almLevel, int *occurFlag)
{
	int		almCode;
	char	condBuf[4096], tmpBuf[1024];
	char	occur_or_clear[12], levelSymbol[4];

	almCode = ALMCODE_SFM_SESS_LOAD;
	*almLevel = SFM_ALM_CRITICAL; 

	if(sess_load >= 80){
		sprintf (occur_or_clear, "OCCURED");
		sprintf (levelSymbol, "***");
		*occurFlag = 1;
	} else {
		sprintf (occur_or_clear, "CLEARED");
		sprintf (levelSymbol, "###");
		*occurFlag = 0;
	}

	sprintf(condBuf,"    %s %s\n%3s A%04d SESSION LOAD ALARM %s\n",
			sysLabel,	// system_name
			commlib_printTStamp(),	// 현재시각 time stamp (년,월,일,시,분,초,요일)
			levelSymbol, almCode, occur_or_clear);

	sprintf(tmpBuf,"      SYSTEM        = %s\n      SessLoadAlarm = %d\n      COMPLETED\n\n\n",
			sfdb->sys[sysIndex].commInfo.name,
			sfdb->sys[sysIndex].commInfo.sessLoad
			);
	strcat (condBuf, tmpBuf);

	// cond로 보낸다.
	// - cond가 로그파일에 기록할때, MMC/STATUS/ALARM 메시지별 구분할 수 있도록
	//	mtype을 표시해 보낸다.
	fimd_txMsg2Cond (condBuf, MTYPE_ALARM_REPORT, almCode);

	return 1;

} //----- End of fimd_makeSessLoadAlmMsg -----//
#endif

int	fimd_makeRmtDbStsAlmMsg (int sysIndex, int rmtDbIndex, int occurFlag)
{
	int		almCode, i;
	char	condBuf[4096], tmpBuf[1024];
	char	occur_or_clear[12], levelSymbol[4], /*status[15],*/ rmtDbSts[2][10];

	//almCode = ALMCODE_SFM_RMT_DB_STS;
	almCode = getAlarmCodeFromType(SFM_ALM_TYPE_DBCON_STST, SFM_ALM_CRITICAL);
	
	memset(rmtDbSts, 0x00, sizeof(rmtDbSts));
	for (i=0; i<strlen((char *)sfdb->sys[sysIndex].commInfo.rmtDbSts[rmtDbIndex].sDbAlias); i++){  
		sfdb->sys[sysIndex].commInfo.rmtDbSts[rmtDbIndex].sDbAlias[i]  =
			toupper(sfdb->sys[sysIndex].commInfo.rmtDbSts[rmtDbIndex].sDbAlias[i]);
	}
	
	if(occurFlag){
		sprintf(rmtDbSts[rmtDbIndex], "ABNORMAL");
		sprintf (occur_or_clear, "OCCURED");
		sprintf (levelSymbol, "***");
//		sprintf (status, "DISCONNECT");
	} else {
		sprintf(rmtDbSts[rmtDbIndex], "NORMAL");
		sprintf (occur_or_clear, "CLEARED");
		sprintf (levelSymbol, "###");
//		sprintf (status, "CONNECT");
	}

	sprintf(condBuf,"    %s %s\n%3s A%04d UAWAP DB CONNECTION STATUS ALARM %s\n",
			sysLabel,	// system_name
			commlib_printTStamp(),	// 현재시각 time stamp (년,월,일,시,분,초,요일)
			levelSymbol, almCode, occur_or_clear);

	sprintf(tmpBuf,"      SYSTEM  = %s\n      STATUS  = %s (%s)\n      COMPLETED\n\n\n",
			sfdb->sys[sysIndex].commInfo.name,
			sfdb->sys[sysIndex].commInfo.rmtDbSts[rmtDbIndex].sDbAlias,
			rmtDbSts[rmtDbIndex]
			);
	strcat (condBuf, tmpBuf);

	// cond로 보낸다.
	// - cond가 로그파일에 기록할때, MMC/STATUS/ALARM 메시지별 구분할 수 있도록
	//	mtype을 표시해 보낸다.
	fimd_txMsg2Cond (condBuf, MTYPE_ALARM_REPORT, almCode);

	return 1;

} //----- End of fimd_makeRmtDbStsAlmMsg -----//


int	fimd_makehwNTPAlmMsg (int sysIndex, int hwNTPIndex, int occurFlag)
{
	int		almCode;
	char	condBuf[4096], tmpBuf[1024];
	char	occur_or_clear[12], levelSymbol[4], ntpName[2][10], ntpSts[2][10];

	//sjjeon almCode = ALMCODE_SFM_HWNTP;
	almCode = getAlarmCodeFromType(SFM_ALM_TYPE_HWNTP, SFM_ALM_MINOR);

	if(hwNTPIndex == 0) { // by helca 08.07
		sprintf(ntpName[hwNTPIndex], "DAEMON");
		if(sfdb->sys[sysIndex].commInfo.ntpSts[hwNTPIndex].level == SFM_ALM_NORMAL) sprintf(ntpSts[hwNTPIndex], "ALIVE");
		else sprintf(ntpSts[hwNTPIndex], "DEAD");
	}else{
		sprintf(ntpName[hwNTPIndex], "CHANNEL");
		if(sfdb->sys[sysIndex].commInfo.ntpSts[hwNTPIndex].level == SFM_ALM_NORMAL) sprintf(ntpSts[hwNTPIndex], "NORMAL");
		else sprintf(ntpSts[hwNTPIndex], "ABNORMAL");
	}
	
	if(occurFlag){
		sprintf (occur_or_clear, "OCCURED");
		sprintf (levelSymbol, "*");
	} else {
		sprintf (occur_or_clear, "CLEARED");
		sprintf (levelSymbol, "#");
	}

	sprintf(condBuf,"    %s %s\n%3s A%04d NETWORK TIME PROTOCOL ALARM %s\n",
			sysLabel,	// system_name
			commlib_printTStamp(),	// 현재시각 time stamp (년,월,일,시,분,초,요일)
			levelSymbol, almCode, occur_or_clear);

	sprintf(tmpBuf,"      SYSTEM   = %s\n      NTP      = %s\n      STATUS   = %s\n      COMPLETED\n\n\n",
			sfdb->sys[sysIndex].commInfo.name,
			ntpName[hwNTPIndex],ntpSts[hwNTPIndex]);
	strcat (condBuf, tmpBuf);

	// cond로 보낸다.
	// - cond가 로그파일에 기록할때, MMC/STATUS/ALARM 메시지별 구분할 수 있도록
	//	mtype을 표시해 보낸다.
	fimd_txMsg2Cond (condBuf, MTYPE_ALARM_REPORT, almCode);

	return 1;

} //----- End of fimd_makehwNTPAlmMsg -----//


int	fimd_makePDCpuUsageAlmMsg (int devIndex, int almLevel, int occurFlag)
{
	int		almCode;
	char	condBuf[4096], tmpBuf[1024];
	char	occur_or_clear[12], levelSymbol[4], probeName[2][5];

	//almCode = ALMCODE_SFM_TAP_CPU_USAGE;
	almCode = getAlarmCodeFromType(SFM_ALM_TYPE_TAP_CPU_USAGE, almLevel);

	if(devIndex == 0) sprintf(probeName[0], "TAP_A");
	else if (devIndex == 1) sprintf(probeName[1], "TAP_B");
	
	if (occurFlag) {
		sprintf (occur_or_clear, "OCCURED");
		switch (almLevel) {
			case SFM_ALM_MINOR:    sprintf (levelSymbol, "*"); break;
			case SFM_ALM_MAJOR:    sprintf (levelSymbol, "**"); break;
			case SFM_ALM_CRITICAL: sprintf (levelSymbol, "***"); break;
			default:               sprintf (levelSymbol, " "); break;
		}
	} else {
		sprintf (occur_or_clear, "CLEARED");
		switch (almLevel) {
			case SFM_ALM_MINOR:    sprintf (levelSymbol, "#"); break;
			case SFM_ALM_MAJOR:    sprintf (levelSymbol, "##"); break;
			case SFM_ALM_CRITICAL: sprintf (levelSymbol, "###"); break;
			default:               sprintf (levelSymbol, " "); break;
		}
	}

	sprintf(condBuf,"    %s %s\n%3s A%04d TAP CPU USAGE ALARM %s\n",
			sysLabel,	// system_name
			commlib_printTStamp(),	// 현재시각 time stamp (년,월,일,시,분,초,요일)
			levelSymbol, almCode, occur_or_clear);

	sprintf(tmpBuf,"      SYSTEM   = %s\n      RESOURCE = TAP_CPU\n      LEVEL    = %s (%d%%)\n      COMPLETED\n\n\n",
			probeName[devIndex],
			fimd_printAlmLevel ((unsigned char)almLevel), 
			l3pd->l3ProbeDev[devIndex].cpuInfo.usage);
	strcat (condBuf, tmpBuf);

	// cond로 보낸다.
	// - cond가 로그파일에 기록할때, MMC/STATUS/ALARM 메시지별 구분할 수 있도록
	//	mtype을 표시해 보낸다.
	fimd_txMsg2Cond (condBuf, MTYPE_ALARM_REPORT, almCode);

	return almLevel;

} //----- End of fimd_makePDCpuUsageAlmMsg -----//

// sjjeon
int	fimd_makeL2swCpuUsageAlmMsg (int devIndex, int almLevel, int occurFlag)
{
	int		almCode;
	char	condBuf[4096], tmpBuf[1024];
	char	occur_or_clear[12], levelSymbol[4], L2Name[10];

	bzero(L2Name, 10);
	//almCode = ALMCODE_SFM_L2SW_CPU_USAGE;
	almCode = getAlarmCodeFromType(SFM_ALM_TYPE_L2_CPU, almLevel);

	if(devIndex == 0) sprintf(L2Name, "L2SWA");
	else sprintf(L2Name, "L2SWB");
	
	if (occurFlag) {
		sprintf (occur_or_clear, "OCCURED");
		switch (almLevel) {
			case SFM_ALM_MINOR:    sprintf (levelSymbol, "*"); break;
			case SFM_ALM_MAJOR:    sprintf (levelSymbol, "**"); break;
			case SFM_ALM_CRITICAL: sprintf (levelSymbol, "***"); break;
			default:               sprintf (levelSymbol, " "); break;
		}
	} else {
		sprintf (occur_or_clear, "CLEARED");
		switch (almLevel) {
			case SFM_ALM_MINOR:    sprintf (levelSymbol, "#"); break;
			case SFM_ALM_MAJOR:    sprintf (levelSymbol, "##"); break;
			case SFM_ALM_CRITICAL: sprintf (levelSymbol, "###"); break;
			default:               sprintf (levelSymbol, " "); break;
		}
	}

	sprintf(condBuf,"    %s %s\n%3s A%04d L2 SWITCH CPU USAGE ALARM %s\n",
			sysLabel,	// system_name
			commlib_printTStamp(),	// 현재시각 time stamp (년,월,일,시,분,초,요일)
			levelSymbol, almCode, occur_or_clear);

	sprintf(tmpBuf,"      SYSTEM   = %s\n      RESOURCE = L2_CPU\n      LEVEL    = %s (%d%%)\n      COMPLETED\n\n\n",
			L2Name,
			fimd_printAlmLevel ((unsigned char)almLevel), 
			g_pstL2Dev->l2Info[devIndex].cpuInfo.usage);
	strcat (condBuf, tmpBuf);

	// cond로 보낸다.
	// - cond가 로그파일에 기록할때, MMC/STATUS/ALARM 메시지별 구분할 수 있도록
	//	mtype을 표시해 보낸다.
	fimd_txMsg2Cond (condBuf, MTYPE_ALARM_REPORT, almCode);

	return almLevel;

} //----- End of fimd_makeL2swCpuUsageAlmMsg-----//

int	fimd_makePDMemUsageAlmMsg (int devIndex, int almLevel, int occurFlag)
{
	int		almCode;
	char	condBuf[4096], tmpBuf[1024];
	char	occur_or_clear[12], levelSymbol[4], probeName[2][5];

	//sjjeon almCode = ALMCODE_SFM_TAP_MEMORY_USAGE;
	almCode = getAlarmCodeFromType(SFM_ALM_TYPE_TAP_MEMORY_USAGE, almLevel);

	if(devIndex == 0) sprintf(probeName[0], "TAP_A");
	else if (devIndex == 1) sprintf(probeName[1], "TAP_B");

	if (occurFlag) {
		sprintf (occur_or_clear, "OCCURED");
		switch (almLevel) {
			case SFM_ALM_MINOR:    sprintf (levelSymbol, "*"); break;
			case SFM_ALM_MAJOR:    sprintf (levelSymbol, "**"); break;
			case SFM_ALM_CRITICAL: sprintf (levelSymbol, "***"); break;
			default:               sprintf (levelSymbol, " "); break;
		}
	} else {
		sprintf (occur_or_clear, "CLEARED");
		switch (almLevel) {
			case SFM_ALM_MINOR:    sprintf (levelSymbol, "#"); break;
			case SFM_ALM_MAJOR:    sprintf (levelSymbol, "##"); break;
			case SFM_ALM_CRITICAL: sprintf (levelSymbol, "###"); break;
			default:               sprintf (levelSymbol, " "); break;
		}
	}

	sprintf(condBuf,"    %s %s\n%3s A%04d TAP MEMORY USAGE ALARM %s\n",
			sysLabel, 	// system_name
			commlib_printTStamp(),	// 현재시각 time stamp (년,월,일,시,분,초,요일)
			levelSymbol, almCode, occur_or_clear);

	sprintf(tmpBuf,"      SYSTEM   = %s\n      RESOURCE = TAP_MEMORY\n      LEVEL    = %s (%d%%)\n      COMPLETED\n\n\n",
			probeName[devIndex],
			fimd_printAlmLevel ((unsigned char)almLevel),
			l3pd->l3ProbeDev[devIndex].memInfo.usage);
	strcat (condBuf, tmpBuf);

	// cond로 보낸다.
	// - cond가 로그파일에 기록할때, MMC/STATUS/ALARM 메시지별 구분할 수 있도록
	//	mtype을 표시해 보낸다.
	fimd_txMsg2Cond (condBuf, MTYPE_ALARM_REPORT, almCode);

	return almLevel;

} //----- End of fimd_makePDMemUsageAlmMsg -----//

/*
by sjjeon 
 **/
int	fimd_makeL2swMemUsageAlmMsg (int devIndex, int almLevel, int occurFlag)
{
	int		almCode;
	char	condBuf[4096], tmpBuf[1024];
	char	occur_or_clear[12], levelSymbol[4], L2Name[10];

	bzero(condBuf,sizeof(condBuf));
	bzero(tmpBuf,sizeof(tmpBuf));
	bzero(occur_or_clear,sizeof(occur_or_clear));
	bzero(levelSymbol,sizeof(levelSymbol));
	bzero(L2Name,sizeof(L2Name));

	//sjjeon almCode = ALMCODE_SFM_L2SW_MEMORY_USAGE;
	almCode = getAlarmCodeFromType(SFM_ALM_TYPE_L2_MEM, almLevel);

	if(devIndex == 0) sprintf(L2Name, "L2SWA");
	else if (devIndex == 1) sprintf(L2Name, "L2SWB");

	if (occurFlag) {
		sprintf (occur_or_clear, "OCCURED");
		switch (almLevel) {
			case SFM_ALM_MINOR:    sprintf (levelSymbol, "*"); break;
			case SFM_ALM_MAJOR:    sprintf (levelSymbol, "**"); break;
			case SFM_ALM_CRITICAL: sprintf (levelSymbol, "***"); break;
			default:               sprintf (levelSymbol, " "); break;
		}
	} else {
		sprintf (occur_or_clear, "CLEARED");
		switch (almLevel) {
			case SFM_ALM_MINOR:    sprintf (levelSymbol, "#"); break;
			case SFM_ALM_MAJOR:    sprintf (levelSymbol, "##"); break;
			case SFM_ALM_CRITICAL: sprintf (levelSymbol, "###"); break;
			default:               sprintf (levelSymbol, " "); break;
		}
	}

	sprintf(condBuf,"    %s %s\n%3s A%04d L2 SWITCH MEMORY USAGE ALARM %s\n",
			sysLabel, 	// system_name
			commlib_printTStamp(),	// 현재시각 time stamp (년,월,일,시,분,초,요일)
			levelSymbol, almCode, occur_or_clear);

	sprintf(tmpBuf,"      SYSTEM   = %s\n      RESOURCE = L2SW_MEM\n      LEVEL    = %s (%d%%)\n      COMPLETED\n\n\n",
			L2Name,
			fimd_printAlmLevel ((unsigned char)almLevel),
			g_pstL2Dev->l2Info[devIndex].memInfo.usage);
	strcat (condBuf, tmpBuf);

	// cond로 보낸다.
	// - cond가 로그파일에 기록할때, MMC/STATUS/ALARM 메시지별 구분할 수 있도록
	//	mtype을 표시해 보낸다.
	fimd_txMsg2Cond (condBuf, MTYPE_ALARM_REPORT, almCode);

	return almLevel;

} //----- End of fimd_makePDMemUsageAlmMsg -----//
#if 0
int	fimd_makeL2swMemUsageAlmMsg (int devIndex, int almLevel, int occurFlag)
{
	int		almCode;
	char	condBuf[4096], tmpBuf[1024];
	char	occur_or_clear[12], levelSymbol[4], L2Name[10];

	bzero(L2Name,10);
	almCode = ALMCODE_SFM_L2SW_MEMORY_USAGE;
	if(devIndex == 0) sprintf(L2Name, "L2SW-A");
	else if (devIndex == 1) sprintf(L2Name, "L2SW-B");

	if (occurFlag) {
		sprintf (occur_or_clear, "OCCURED");
		switch (almLevel) {
			case SFM_ALM_MINOR:    sprintf (levelSymbol, "*"); break;
			case SFM_ALM_MAJOR:    sprintf (levelSymbol, "**"); break;
			case SFM_ALM_CRITICAL: sprintf (levelSymbol, "***"); break;
			default:               sprintf (levelSymbol, " "); break;
		}
	} else {
		sprintf (occur_or_clear, "CLEARED");
		switch (almLevel) {
			case SFM_ALM_MINOR:    sprintf (levelSymbol, "#"); break;
			case SFM_ALM_MAJOR:    sprintf (levelSymbol, "##"); break;
			case SFM_ALM_CRITICAL: sprintf (levelSymbol, "###"); break;
			default:               sprintf (levelSymbol, " "); break;
		}
	}

	sprintf(condBuf,"    %s %s\n%3s A%04d L2SW MEMORY USAGE ALARM %s\n",
			sysLabel, 	// system_name
			commlib_printTStamp(),	// 현재시각 time stamp (년,월,일,시,분,초,요일)
			levelSymbol, almCode, occur_or_clear);

	sprintf(tmpBuf,"      SYSTEM   = %s\n      RESOURCE = L2SW MEMORY\n      LEVEL    = %s (%d%%)\n      COMPLETED\n\n\n",
			L2Name,
			fimd_printAlmLevel ((unsigned char)almLevel),
			g_pstL2Dev->l2Info[devIndex].memInfo.usage);
	strcat (condBuf, tmpBuf);

	// cond로 보낸다.
	// - cond가 로그파일에 기록할때, MMC/STATUS/ALARM 메시지별 구분할 수 있도록
	//	mtype을 표시해 보낸다.
	fimd_txMsg2Cond (condBuf, MTYPE_ALARM_REPORT, almCode);

	return almLevel;

} //----- End of fimd_makeL2swMemUsageAlmMsg-----//
#endif
int	fimd_makePDFanAlmMsg (int devIndex, int fanIndex, int *almLevel, int *occurFlag)
{
	int		almCode;
	char	condBuf[4096], tmpBuf[1024], probeName[2][5], fanSts[2][10];
	char	occur_or_clear[12], levelSymbol[4];

	*almLevel = SFM_ALM_CRITICAL; 
	//sjjeon almCode = ALMCODE_SFM_TAP_FAN_USAGE;
	almCode = getAlarmCodeFromType(SFM_ALM_TYPE_TAP_FAN_STS, *almLevel);

	if(devIndex == 0) sprintf(probeName[0], "TAP_A");
	else if (devIndex == 1) sprintf(probeName[1], "TAP_B");
	if(l3pd->l3ProbeDev[devIndex].fanInfo.status[fanIndex] == 0) sprintf(fanSts[fanIndex], "NORMAL");
	else sprintf(fanSts[fanIndex], "ABNORMAL");

	if (l3pd->l3ProbeDev[devIndex].fanInfo.status[fanIndex] == PD_FUNCTION_OFF) {
		sprintf (occur_or_clear, "OCCURED");
		sprintf (levelSymbol, "***");
		*occurFlag = 1;
	} else {
		sprintf (occur_or_clear, "CLEARED");
		sprintf (levelSymbol, "###");
		*occurFlag = 0;
	}

	sprintf(condBuf,"    %s-%s %s\n%3s A%04d TAP DEVICE FAN ALARM %s\n",
			sysLabel, "TAP",	// system_name
			commlib_printTStamp(),	// 현재시각 time stamp (년,월,일,시,분,초,요일)
			levelSymbol, almCode, occur_or_clear);

	sprintf(tmpBuf,"      SYSTEM   = %s\n      RESOURCE = %s\n      STATUS   = %s\n      COMPLETED\n\n\n",
			"TAP",
			probeName[devIndex],
			fanSts[fanIndex]);
	strcat (condBuf, tmpBuf);

	// cond로 보낸다.
	// - cond가 로그파일에 기록할때, MMC/STATUS/ALARM 메시지별 구분할 수 있도록
	//	mtype을 표시해 보낸다.
	fimd_txMsg2Cond (condBuf, MTYPE_ALARM_REPORT, almCode);

	return 1;

} //----- End of fimd_makePDFanAlmMsg -----//

int	fimd_makeGigaLanAlmMsg (int devIndex, int gigaIndex, int occurFlag)
{
	int		almCode;
	char	condBuf[4096], tmpBuf[1024], probeName[2][20], gigaSts[2][20], rscName[6];
	char	occur_or_clear[12], levelSymbol[4];

	//sjjeon almCode = ALMCODE_SFM_TAP_PORT_STS;
	almCode = getAlarmCodeFromType(SFM_ALM_TYPE_TAP_PORT_STS, SFM_ALM_CRITICAL);

	if(devIndex == 0)
		sprintf(probeName[0], "TAP DEVICE A");
	else if (devIndex == 1)
		sprintf(probeName[1], "TAP DEVICE B");

	if(l3pd->l3ProbeDev[devIndex].gigaLanInfo[gigaIndex].status == L3PD_RJ45_STS_PHYSIUP)
		sprintf(gigaSts[gigaIndex], "PHYSICAL UP");
	else if(l3pd->l3ProbeDev[devIndex].gigaLanInfo[gigaIndex].status == L3PD_RJ45_STS_PHYSIDOWN)
		sprintf(gigaSts[gigaIndex], "PHYSICAL DOWN");
	else if(l3pd->l3ProbeDev[devIndex].gigaLanInfo[gigaIndex].status == L3PD_RJ45_STS_PROTODOWN)
		sprintf(gigaSts[gigaIndex], "PROTOCOL DOWN");
	else
		sprintf(gigaSts[gigaIndex], "PROTOCOL DOWN");

	sprintf(rscName, "LINK%d", gigaIndex+1);

	if (occurFlag == 1) {
		sprintf (occur_or_clear, "OCCURED");
		sprintf (levelSymbol, "***");
	} else {
		sprintf (occur_or_clear, "CLEARED");
		sprintf (levelSymbol, "###");
	}
//yhshin	sprintf(condBuf,"    %s-%s %s\n%3s A%04d PROBING DEVICE NETWORK ALARM %s\n",
// PD --> TAP	
	sprintf(condBuf,"    %s-%s %s\n%3s A%04d TAP DEVICE NETWORK ALARM %s\n",
			sysLabel, "TAP",	// system_name
			commlib_printTStamp(),	// 현재시각 time stamp (년,월,일,시,분,초,요일)
			levelSymbol, almCode, occur_or_clear);

	sprintf(tmpBuf,"      SYSTEM   = %s\n      RESOURCE = %s\n      STATUS   = %s\n      COMPLETED\n\n\n",
			probeName[devIndex],
			rscName,
			gigaSts[gigaIndex]
			);
	strcat (condBuf, tmpBuf);
	// cond로 보낸다.
	// - cond가 로그파일에 기록할때, MMC/STATUS/ALARM 메시지별 구분할 수 있도록
	//	mtype을 표시해 보낸다.
	fimd_txMsg2Cond (condBuf, MTYPE_ALARM_REPORT, almCode);

	return 1;

} //----- End of fimd_makeGigaLanAlmMsg -----//

int	fimd_makeL2swLanAlmMsg (int devIndex, int gigaIndex, int occurFlag)
{
	// L2SW는 공통으로 관리 하는 부분에서 처리 하므로 사용하지 않음..
	// fimd_makeCommStsAlmMsg ()
	return 1;

} //----- End of fimd_makeGigaLanAlmMsg -----//

int	fimd_makeRsrcLoadAlmMsg (int sysIndex, int rsrcIndex, int usage, int almLevel, int occurFlag)
{
	int		almCode;
	char	condBuf[4096], tmpBuf[1024];
	char	occur_or_clear[12], levelSymbol[4];

	//sjjeon almCode = ALMCODE_SFM_RSRC_LOAD;
	almCode = getAlarmCodeFromType(SFM_ALM_TYPE_RSRC_LOAD, almLevel);

	if (occurFlag) {
		sprintf (occur_or_clear, "OCCURED");
		switch (almLevel) {
			case SFM_ALM_MINOR:    sprintf (levelSymbol, "*"); break;
			case SFM_ALM_MAJOR:    sprintf (levelSymbol, "**"); break;
			case SFM_ALM_CRITICAL: sprintf (levelSymbol, "***"); break;
			default:               sprintf (levelSymbol, " "); break;
		}
	} else {
		sprintf (occur_or_clear, "CLEARED");
		switch (almLevel) {
			case SFM_ALM_MINOR:    sprintf (levelSymbol, "#"); break;
			case SFM_ALM_MAJOR:    sprintf (levelSymbol, "##"); break;
			case SFM_ALM_CRITICAL: sprintf (levelSymbol, "###"); break;
			default:               sprintf (levelSymbol, " "); break;
		}
	}

	sprintf(condBuf,"    %s %s\n%3s A%04d RESOURCE LOAD ALARM %s\n",
			sysLabel,	// system_name
			commlib_printTStamp(),	// 현재시각 time stamp (년,월,일,시,분,초,요일)
			levelSymbol, almCode, occur_or_clear);

	sprintf(tmpBuf,"      SYSTEM   = %s\n      RESOURCE = %s\n      LOAD     = %d%%\n      LEVEL    = %s\n      COMPLETED\n\n\n",
			sfdb->sys[sysIndex].commInfo.name,
			rsrcName[rsrcIndex],
			usage,
			fimd_printAlmLevel ((unsigned char)almLevel));
	strcat (condBuf, tmpBuf);

	// cond로 보낸다.
	// - cond가 로그파일에 기록할때, MMC/STATUS/ALARM 메시지별 구분할 수 있도록
	//	mtype을 표시해 보낸다.
	fimd_txMsg2Cond (condBuf, MTYPE_ALARM_REPORT, almCode);
	return almLevel;

} 

int	fimd_makeQueLoadAlmMsg (int sysIndex, int queLoadIndex, int almLevel, int occurFlag)
{
	int		almCode, i;
	char	condBuf[4096], tmpBuf[1024];
	char	occur_or_clear[12], levelSymbol[4];

	//sjjeon almCode = ALMCODE_SFM_QUEUE_LOAD;
	almCode = getAlarmCodeFromType(SFM_ALM_TYPE_QUEUE_LOAD, almLevel);

	for (i=0; i<strlen(sfdb->sys[sysIndex].commInfo.queInfo[queLoadIndex].qNAME); i++){
		sfdb->sys[sysIndex].commInfo.queInfo[queLoadIndex].qNAME[i]  =
			toupper(sfdb->sys[sysIndex].commInfo.queInfo[queLoadIndex].qNAME[i]);
	}
	if (occurFlag) {
		sprintf (occur_or_clear, "OCCURED");
		switch (almLevel) {
			case SFM_ALM_MINOR:    sprintf (levelSymbol, "*"); break;
			case SFM_ALM_MAJOR:    sprintf (levelSymbol, "**"); break;
			case SFM_ALM_CRITICAL: sprintf (levelSymbol, "***"); break;
			default:               sprintf (levelSymbol, " "); break;
		}
	} else {
		sprintf (occur_or_clear, "CLEARED");
		switch (almLevel) {
			case SFM_ALM_MINOR:    sprintf (levelSymbol, "#"); break;
			case SFM_ALM_MAJOR:    sprintf (levelSymbol, "##"); break;
			case SFM_ALM_CRITICAL: sprintf (levelSymbol, "###"); break;
			default:               sprintf (levelSymbol, " "); break;
		}
	}

	sprintf(condBuf,"    %s %s\n%3s A%04d QUEUE LOAD ALARM %s\n",
			sysLabel,	// system_name
			commlib_printTStamp(),	// 현재시각 time stamp (년,월,일,시,분,초,요일)
			levelSymbol, almCode, occur_or_clear);

	sprintf(tmpBuf,"      SYSTEM   = %s\n      RESOURCE = %s\n      LEVEL    = %s (%d%%)\n      COMPLETED\n\n\n",
			sfdb->sys[sysIndex].commInfo.name,
			sfdb->sys[sysIndex].commInfo.queInfo[queLoadIndex].qNAME, // queue partition name
			fimd_printAlmLevel ((unsigned char)almLevel),
			sfdb->sys[sysIndex].commInfo.queInfo[queLoadIndex].load);
	strcat (condBuf, tmpBuf);

	// cond로 보낸다.
	// - cond가 로그파일에 기록할때, MMC/STATUS/ALARM 메시지별 구분할 수 있도록
	//	mtype을 표시해 보낸다.
	fimd_txMsg2Cond (condBuf, MTYPE_ALARM_REPORT, almCode);

	return almLevel;

} //----- End of fimd_makeQueLoadAlmMsg -----//

int fimd_makeNmsifstsAlmMsg (int sysIndex, int nmsIndex, int almLevel, int occurFlag)
{

    int almCode;
    char    condBuf[4096], tmpBuf[1024], nmsStsName[10], nmsPortName[16];
    char    occur_or_clear[12], levelSymbol[4];

    almCode = ALMCODE_NMS_CONNECT;

    if(sfdb->nmsInfo.ptype[nmsIndex] == FD_TYPE_LISTEN) 
        sprintf(nmsStsName, "LISTEN");
    else if(sfdb->nmsInfo.ptype[nmsIndex] == FD_TYPE_DATA)
        sprintf(nmsStsName, "DATA");
    else
        sprintf(nmsStsName, "ABNORMAL");

    if (nmsIndex == 0) sprintf(nmsPortName, "ALARM");
    else if (nmsIndex == 1) sprintf(nmsPortName, "CONSOLE");
    else if (nmsIndex == 2) sprintf(nmsPortName, "CONFIG");
    else if (nmsIndex == 3) sprintf(nmsPortName, "MMC");
    else if (nmsIndex == 4) sprintf(nmsPortName, "STATISTICS");

    if (occurFlag) {
        sprintf (occur_or_clear, "OCCURED");
        switch (almLevel) {
            case SFM_ALM_MINOR:    sprintf (levelSymbol, "*"); break;
            case SFM_ALM_MAJOR:    sprintf (levelSymbol, "**"); break;
            case SFM_ALM_CRITICAL: sprintf (levelSymbol, "***"); break;
            default:               sprintf (levelSymbol, " "); break;
        }
    } else {
        sprintf (occur_or_clear, "CLEARED");
        switch (almLevel) {
            case SFM_ALM_MINOR:    sprintf (levelSymbol, "#"); break;
            case SFM_ALM_MAJOR:    sprintf (levelSymbol, "##"); break;
            case SFM_ALM_CRITICAL: sprintf (levelSymbol, "###"); break;
            default:               sprintf (levelSymbol, " "); break;
        }
    }

    sprintf(condBuf,"    %s %s\n%3s A%04d NMSIF STATUS ALARM %s\n",
            sysLabel,   // system_name
            commlib_printTStamp(),  // 현재시각 time stamp (년,월,일,시,분,초,요일)
            levelSymbol, almCode, occur_or_clear);

    sprintf(tmpBuf,"      SYSTEM         = %s\n      RESOURCE       = NMSIF(%s)\n      LEVEL          = %s\n      STATUS         = %s\n      COMPLETED\n\n\n",
            sfdb->sys[0].commInfo.name,
	    nmsPortName,
            fimd_printAlmLevel ((unsigned char)almLevel),
            nmsStsName
    );
    
    strcat (condBuf, tmpBuf);

    // cond로 보낸다.
    // - cond가 로그파일에 기록할때, MMC/STATUS/ALARM 메시지별 구분할 수 있도록
    //  mtype을 표시해 보낸다.
    fimd_txMsg2Cond (condBuf, MTYPE_ALARM_REPORT, almCode);
    return 1;

} //----- End of fimd_makeNmsifstsAlmMsg -----//

int fimd_makeRadiusSuccMsg (int sysIndex, int radiusIndex, int result_count, int almLevel, int occurFlag)
{
    int     almCode;
    char    condBuf[2048], tmpBuf[1024];
    char    occur_or_clear[12], levelSymbol[4];
    
    memset(tmpBuf, 0x00, 1024);	
    //almCode = ALMCODE_SFM_SUCCESS_RATE;
	almCode = getAlarmCodeFromType(SFM_ALM_TYPE_SUCC_RATE, almLevel);


    if(occurFlag){
        sprintf (occur_or_clear, "OCCURED");
        sprintf (levelSymbol, "***");
    } else {
        sprintf (occur_or_clear, "CLEARED");
        sprintf (levelSymbol, "###");
    }
    sprintf(condBuf,"    %s %s\n%3s A%04d SUCCESS RATE IP ALARM %s\n",
            sfdb->sys[sysIndex].commInfo.name,   // system_name
            commlib_printTStamp(),  // 현재시각 time stamp (년,월,일,시,분,초,요일)
            levelSymbol, almCode, occur_or_clear);
    
    sprintf(tmpBuf,"      SYSTYPE  = %s\n"
		   "      RESOURCE = RADIUS(%s)\n"
		   "      INFO     = CURRENT [%d/0%%] / UNDER THRESHOLD [0/0%%]\n"
                   "      COMPLETED\n\n\n",
            	sfdb->sys[sysIndex].commInfo.name,
                sfdb->sys[sysIndex].succRateIpInfo.radius[radiusIndex].ipAddr,
		result_count);
    strcat (condBuf, tmpBuf);

    // cond로 보낸다.
    // - cond가 로그파일에 기록할때, MMC/STATUS/ALARM 메시지별 구분할 수 있도록
    //  mtype을 표시해 보낸다.
    fimd_txMsg2Cond (condBuf, MTYPE_ALARM_REPORT, almCode);

    return 1;

} //----- End of fimd_makeRadiusSuccMsg -----/

int fimd_hdlDupDualStsAlmMsg (int sysIndex, int dualActStdFlag, int dupDualOccured)
{
	int     almCode;
	char	condBuf[1024], tmpBuf[256];
	char    occur_or_clear[12], levelSymbol[4], dual_status[12];
	
	memset(condBuf, 0x00, sizeof(condBuf)); 
	memset(tmpBuf, 0x00, sizeof(tmpBuf)); 

	if(dualActStdFlag == 1){
		//sjjeon almCode = ALMCODE_SFM_DUAL_ACT;
		almCode = getAlarmCodeFromType(SFM_ALM_TYPE_DUAL_ACT, SFM_ALM_CRITICAL);
		sprintf(dual_status, "ACTIVE");
	}else if (dualActStdFlag == 2){
		//sjjeon almCode = ALMCODE_SFM_DUAL_STD;
		almCode = getAlarmCodeFromType(SFM_ALM_TYPE_DUAL_STD, SFM_ALM_CRITICAL);
		sprintf(dual_status, "STANDBY");
	}

	if(dupDualOccured){
        	sprintf (occur_or_clear, "OCCURED");
        	sprintf (levelSymbol, "***");
    	} else {
        	sprintf (occur_or_clear, "CLEARED");
        	sprintf (levelSymbol, "###");
    	}
    
	sprintf(condBuf,"    %s %s\n%3s A%04d DUPLICATION STATUS DUAL %s ALARM %s\n",
        	sysLabel,   // system_name
            	commlib_printTStamp(),  // 현재시각 time stamp (년,월,일,시,분,초,요일)
            	levelSymbol, almCode, dual_status, occur_or_clear);

    	sprintf(tmpBuf,"      SYSTEM   = %s\n      SCMA <==> SCMB\n      COMPLETED\n\n\n",
            	sfdb->sys[sysIndex].commInfo.name);
                        
    	strcat (condBuf, tmpBuf);
	
	// cond로 보낸다.
	// - cond가 로그파일에 기록할때, MMC/STATUS/ALARM 메시지별 구분할 수 있도록
	//   mtype을 표시해 보낸다.
	fimd_txMsg2Cond (condBuf, MTYPE_ALARM_REPORT, almCode);
	//fimd_DuplicationStsEvent2Client();
	return 1;

} //----- End of fimd_hdlDupDualStsAlmMsg -----//

int fimd_makeDupTimeOutAlmMsg (int sysIndex, int occurFlag)
{
        int     almCode;
        char    condBuf[1024], tmpBuf[256];
        char    occur_or_clear[12], levelSymbol[4];

        memset(condBuf, 0x00, sizeof(condBuf));
        memset(tmpBuf, 0x00, sizeof(tmpBuf));

        //sjjeon almCode = ALMCODE_SFM_DUAL_STS_QRY_TIME_OUT;
		almCode = getAlarmCodeFromType(SFM_ALM_TYPE_DUAL_STS_QRY_TIME_OUT, SFM_ALM_CRITICAL);

        if(occurFlag){
                sprintf (occur_or_clear, "OCCURED");
                sprintf (levelSymbol, "***");
        } else {
                sprintf (occur_or_clear, "CLEARED");
                sprintf (levelSymbol, "###");
        }

        sprintf(condBuf,"    %s %s\n%3s A%04d DUAL STATUS QUERY TIME OUT ALARM %s\n",
                sysLabel,   // system_name
                commlib_printTStamp(),  // 현재시각 time stamp (년,월,일,시,분,초,요일)
                levelSymbol, almCode, occur_or_clear);
                
        sprintf(tmpBuf,"      SYSTEM   = %s\n      COMPLETED\n\n\n",
                sfdb->sys[sysIndex].commInfo.name);

        strcat (condBuf, tmpBuf);

        // cond로 보낸다.
        // - cond가 로그파일에 기록할때, MMC/STATUS/ALARM 메시지별 구분할 수 있도록
        //   mtype을 표시해 보낸다.
        fimd_txMsg2Cond (condBuf, MTYPE_ALARM_REPORT, almCode);
        return 1;

} //----- End of fimd_makeDupTimeOutAlmMsg -----//        


int	fimd_makeSceCpuUsageAlmMsg (int devIndex, int cpuIndex, int almLevel, int occurFlag)
{
	int		almCode;
	char	condBuf[4096], tmpBuf[1024];
	char	occur_or_clear[12], levelSymbol[4], devName[2][5];

	//sjjeon almCode = ALMCODE_SFM_SCE_CPU_USAGE;
	almCode = getAlarmCodeFromType(SFM_ALM_TYPE_SCE_CPU, almLevel);


	if(devIndex == 0) sprintf(devName[0], "SCEA");
	else if (devIndex == 1) sprintf(devName[1], "SCEB");
	
	if (occurFlag) {
		sprintf (occur_or_clear, "OCCURED");
		switch (almLevel) {
			case SFM_ALM_MINOR:    sprintf (levelSymbol, "*"); break;
			case SFM_ALM_MAJOR:    sprintf (levelSymbol, "**"); break;
			case SFM_ALM_CRITICAL: sprintf (levelSymbol, "***"); break;
			default:               sprintf (levelSymbol, " "); break;
		}
	} else {
		sprintf (occur_or_clear, "CLEARED");
		switch (almLevel) {
			case SFM_ALM_MINOR:    sprintf (levelSymbol, "#"); break;
			case SFM_ALM_MAJOR:    sprintf (levelSymbol, "##"); break;
			case SFM_ALM_CRITICAL: sprintf (levelSymbol, "###"); break;
			default:               sprintf (levelSymbol, " "); break;
		}
	}

	sprintf(condBuf,"    %s %s\n%3s A%04d SCE DEVICE CPU%d USAGE ALARM %s\n",
			sysLabel,	// system_name
			commlib_printTStamp(),	// 현재시각 time stamp (년,월,일,시,분,초,요일)
			levelSymbol, almCode, cpuIndex, occur_or_clear);

	sprintf(tmpBuf,"      SYSTEM   = %s\n      RESOURCE = CPU%d\n      LEVEL    = %s (%d%%)\n      COMPLETED\n\n\n",
			devName[devIndex],
			cpuIndex,
			fimd_printAlmLevel ((unsigned char)almLevel), 
			g_pstSCEInfo->SCEDev[devIndex].cpuInfo[cpuIndex].usage);
			//l3pd->l3ProbeDev[devIndex].cpuInfo.usage);
	strcat (condBuf, tmpBuf);

	// cond로 보낸다.
	// - cond가 로그파일에 기록할때, MMC/STATUS/ALARM 메시지별 구분할 수 있도록
	//	mtype을 표시해 보낸다.
	fimd_txMsg2Cond (condBuf, MTYPE_ALARM_REPORT, almCode);

	return almLevel;

} //----- End of fimd_makePDCpuUsageAlmMsg -----//


int	fimd_makeSceDiskUsageAlmMsg (int devIndex, int almLevel, int occurFlag)
{
	int		almCode;
	char	condBuf[4096], tmpBuf[1024];
	char	occur_or_clear[12], levelSymbol[4], devName[2][5];

	//almCode = ALMCODE_SFM_SCE_DISK_USAGE;
	almCode = getAlarmCodeFromType(SFM_ALM_TYPE_SCE_DISK, almLevel);

	if(devIndex == 0) sprintf(devName[0], "SCEA");
	else if (devIndex == 1) sprintf(devName[1], "SCEB");
	
	if (occurFlag) {
		sprintf (occur_or_clear, "OCCURED");
		switch (almLevel) {
			case SFM_ALM_MINOR:    sprintf (levelSymbol, "*"); break;
			case SFM_ALM_MAJOR:    sprintf (levelSymbol, "**"); break;
			case SFM_ALM_CRITICAL: sprintf (levelSymbol, "***"); break;
			default:               sprintf (levelSymbol, " "); break;
		}
	} else {
		sprintf (occur_or_clear, "CLEARED");
		switch (almLevel) {
			case SFM_ALM_MINOR:    sprintf (levelSymbol, "#"); break;
			case SFM_ALM_MAJOR:    sprintf (levelSymbol, "##"); break;
			case SFM_ALM_CRITICAL: sprintf (levelSymbol, "###"); break;
			default:               sprintf (levelSymbol, " "); break;
		}
	}

	sprintf(condBuf,"    %s %s\n%3s A%04d SCE DEVICE DISK USAGE ALARM %s\n",
			sysLabel,	// system_name
			commlib_printTStamp(),	// 현재시각 time stamp (년,월,일,시,분,초,요일)
			levelSymbol, almCode, occur_or_clear);

	sprintf(tmpBuf,"      SYSTEM   = %s\n      RESOURCE = DISK(%d%%)\n      LEVEL    = %s\n      COMPLETED\n\n\n",
			devName[devIndex],
			g_pstSCEInfo->SCEDev[devIndex].diskInfo.usage,
			fimd_printAlmLevel ((unsigned char)almLevel));
	strcat (condBuf, tmpBuf);

	// cond로 보낸다.
	// - cond가 로그파일에 기록할때, MMC/STATUS/ALARM 메시지별 구분할 수 있도록
	//	mtype을 표시해 보낸다.
	fimd_txMsg2Cond (condBuf, MTYPE_ALARM_REPORT, almCode);

	return almLevel;

} //----- End of fimd_makePDCpuUsageAlmMsg -----//


// fimd_getSceAlarmCode -> fimd_getAlarmCode : sjjeon
int fimd_getAlarmCode (int dev_kind)
{
	int code=0;
	
	switch(dev_kind)
	{
	/*TAP*/
	case TAP_CPU:
		code = ALMCODE_SFM_TAP_CPU_USAGE;
		break;
	case TAP_MEM:
		code = ALMCODE_SFM_TAP_MEMORY_USAGE;
		break;
	case TAP_PORT:
		code = ALMCODE_SFM_TAP_PORT_STS;
		break;
	case TAP_POWER:  // 20110424 by dcham
		code = ALMCODE_SFM_TAP_POWER_STS;
		break;

		/*SCE*/
	case SCE_CPU:
		code = ALMCODE_SFM_SCE_CPU_USAGE;
		break;
	case SCE_MEM:
		code = ALMCODE_SFM_SCE_MEM_USAGE;
		break;
	case SCE_DISK:
		code = ALMCODE_SFM_SCE_DISK_USAGE;
		break;
	case SCE_PORT_MGMT:
	case SCE_PORT_LINK:
		code = ALMCODE_SFM_SCE_PORT_USAGE;
		break;
	case SCE_FAN:
		code = ALMCODE_SFM_SCE_FAN_USAGE;
		break;
	case SCE_POWER:
		code = ALMCODE_SFM_SCE_PWR_USAGE;
		break;
	case SCE_RDR_CONN:
		code = ALMCODE_SFM_SCE_RDR_CONN_STS;
		break;
	case SCE_STATUS:
		code = ALMCODE_SFM_SCE_SYS_STS;
		break;
	case SCE_TEMP:
		code = ALMCODE_SFM_SCE_TEMP_STS;
		break;

	/*L2 SWITCH */
	case L2SW_CPU:
		code = ALMCODE_SFM_L2SW_CPU_USAGE;
		break;
	case L2SW_MEM:
		code = ALMCODE_SFM_L2SW_MEMORY_USAGE;
		break;
	case L2SW_PORT:
		code = ALMCODE_SFM_L2SW_LAN_STS;
		break;

	/* hjjung_20100823 */
	case SCE_USER:
		code = ALMCODE_SFM_SCE_USER_USAGE;
		break;
	
	/* hjjung_20100823 */
	/*LEG*/
	case LEG_SESSION:
		code = ALMCODE_SFM_LEG_SESSION_USAGE;
		break;
	case TPS_LOAD: // added by dcham 2011.05.25 for TPS
		code = ALMCODE_SFM_TPS_INFO;
		break;
	default:
		//code = ALMCODE_SFM_SCE_UNKNOWN;
		code = ALMCODE_SFM_UNKNOWN;
		break;
	}
	return code;
}

char *fimd_getSceDevName (int dev_kind)
{
	switch(dev_kind)
	{
	case SCE_CPU:
		return SQL_SCE_CPU_ALM_INFO;
		break;
	case SCE_MEM:
		return SQL_SCE_MEM_ALM_INFO;
		break;
	case SCE_DISK:
		return SQL_SCE_DISK_ALM_INFO;
		break;
	//case SCE_PORT:  PORT정보는 따로 얻는다. 
	//	return "LINK";
	//	break;
	/* hjjung_20100823 */
	case SCE_USER:
		return SQL_SCE_USER_ALM_INFO;
		break;
	case SCE_FAN:
		return SQL_SCE_FAN_ALM_INFO;
		break;
	case SCE_POWER:
		return SQL_SCE_PWR_ALM_INFO;
		break;
	case SCE_TEMP:
		return SQL_SCE_TEMP_ALM_INFO;
		break;
	case SCE_RDR_CONN: 
		return SQL_SCE_RDR_CONN_ALM_INFO;
		break;
	case SCE_STATUS:
		return SQL_SCE_STATUS_ALM_INFO;
		break;
	default:
		return "UNKNOWN";
		break;
	}
}
/*End of fimd_getSceDevName()*/

/* hjjung_20100823 */
char *fimd_getLegDevName (int dev_kind)
{
	switch(dev_kind)
	{
	case LEG_SESSION:
		return SQL_LEG_SESSION_ALM_INFO;
		break;
	case TPS_LOAD: // added by dcham 20110525 for TPS
		return SQL_TPS_ALM_INFO;
		break;
	default:
		return "UNKNOWN";
		break;
	}
}
// SCE PORT NAME
char *fimd_getScePortName(int dev_num)
{
	switch (dev_num) {
		case 0:
			return (char*)"MGMT#1";
		case 1:
			return (char*)"MGMT#2";
		case 2:                                                                                          
			return (char*)"LINK#1-IN";
		case 3:
			return (char*)"LINK#1-OUT";
		case 4:
			return (char*)"LINK#2-IN";
		case 5:
			return (char*)"LINK#2-OUT";
		default:
			return (char*)"UNKNOWN";
	}                                 

}

int fimd_getAlarmType (int dev_kind)
{
	int type=0;

	switch(dev_kind)
	{
		/*TAP*/
		case TAP_CPU:
			type = SFM_ALM_TYPE_TAP_CPU_USAGE;
			break;
		case TAP_MEM:
			type = SFM_ALM_TYPE_TAP_MEMORY_USAGE;
			break;
		case TAP_PORT:
			type = SFM_ALM_TYPE_TAP_PORT_STS;
			break;
		case TAP_POWER: // 20110422 by dcham
			type = SFM_ALM_TYPE_TAP_POWER_STS;
			break;
				/*SCE*/
		case SCE_CPU:
			type = SFM_ALM_TYPE_SCE_CPU;
			break;
		case SCE_MEM:
			type = SFM_ALM_TYPE_SCE_MEM;
			break;
		case SCE_DISK:
			type = SFM_ALM_TYPE_SCE_DISK;
			break;
		case SCE_POWER:
			type = SFM_ALM_TYPE_SCE_PWR;
			break;
		case SCE_FAN:
			type = SFM_ALM_TYPE_SCE_FAN;
			break;
		case SCE_TEMP:
			type = SFM_ALM_TYPE_SCE_TEMP;
			break;
		case SCE_VOLT:
			type = SFM_ALM_TYPE_SCE_VOLT;
			break;
		case SCE_PORT_MGMT:
			type = SFM_ALM_TYPE_SCE_PORT_MGMT;
			break;
		case SCE_PORT_LINK:
			type = SFM_ALM_TYPE_SCE_PORT_LINK;
			break;
		case SCE_RDR:
			type = SFM_ALM_TYPE_SCE_RDR;
			break;
		case SCE_RDR_CONN:
			type = SFM_ALM_TYPE_SCE_RDR_CONN;
			break;
		case SCE_STATUS:
			type = SFM_ALM_TYPE_SCE_STATUS;
			break;
		/*L2SW*/
		case L2SW_CPU:
			type = SFM_ALM_TYPE_L2_CPU;
			break;
		case L2SW_MEM:
			type = SFM_ALM_TYPE_L2_MEM;
			break;
		case L2SW_PORT:
			type = SFM_ALM_TYPE_L2_LAN;
			break;
		/* hjjung_20100823 */
		case SCE_USER:
			type = SFM_ALM_TYPE_SCE_USER;
			break;
		/* hjjung_20100823 */
		/*LEG*/
		case LEG_SESSION:
			type = SFM_ALM_TYPE_LEG_SESSION;
			break;
		case TPS_LOAD: // added by dcham 20110525 for TPS alarm type
			type = SFM_ALM_TYPE_TPS;
			break;
	}
	return type;
}

int fimd_getAbnormalAlarmLevel_byDevName(COMM_STATUS_PARAM *pParam)
{
	int level=-1;
	int sysIndex = 0;

	// 상태 체크 하는 부분만 레벨 설정 (usage는 사용하지 않는다.)
	switch(pParam->devKind)
	{
		/**< SCE -----**/
		case SCE_PORT_MGMT:
			/**< pParam->devIndex  port number **/
			/**< 0 | 1
			  	if 0 or 1
					1/0 이 down 이면 Critical
			 **/
            if(pParam->sysType == SCEA) sysIndex = 0;
			else sysIndex = 1;

			// MGMT#0 상태 확인
			if ( pParam->devIndex == 0 ) {
				if (g_pstSCEInfo->SCEDev[sysIndex].portStatus[1].preStatus != 2 /*abnomal*/) {

					// MGMT#0 가 Major가 아니고 MGMG#1 이 abnormal 일때..
					if (g_pstSCEInfo->SCEDev[sysIndex].portStatus[pParam->devIndex].level != SFM_ALM_MAJOR)
						level = SFM_ALM_CRITICAL;
					else 
						level = SFM_ALM_MAJOR;

				} else  {
					// Citical --> major로 내려온 경우 한번만 
					if (g_pstSCEInfo->SCEDev[sysIndex].portStatus[pParam->devIndex].level == SFM_ALM_CRITICAL) {
//						pParam->preStatus = 2;
						level = SFM_ALM_CRITICAL;
					} else  {
						level = SFM_ALM_MAJOR;
					}
				}
			} else if ( pParam->devIndex == 1 ) {
				if (g_pstSCEInfo->SCEDev[sysIndex].portStatus[0].preStatus != 2 /*abnomal*/) {

					if (g_pstSCEInfo->SCEDev[sysIndex].portStatus[pParam->devIndex].level != SFM_ALM_MAJOR)
						level = SFM_ALM_CRITICAL;
					else 
						level = SFM_ALM_MAJOR;

				} else  {
					if (g_pstSCEInfo->SCEDev[sysIndex].portStatus[pParam->devIndex].level == SFM_ALM_CRITICAL) {
						//pParam->preStatus = 2;
						level = SFM_ALM_CRITICAL;
					} else  {
						level = SFM_ALM_MAJOR;
					}
				}
			}
			break;

		case SCE_PORT_LINK:
			/**<  2,3 | 4,5
			  	if 2,3 or 4,5
					4,5/2,3 이 down 이면 Critical
			 **/
            if(pParam->sysType == SCEA) sysIndex = 0;
			else sysIndex = 1;

			// 
			if ( (pParam->devIndex == 2) || (pParam->devIndex == 3)) {
				if ((g_pstSCEInfo->SCEDev[sysIndex].portStatus[4].preStatus != 2 /*abnomal*/)
					|| (g_pstSCEInfo->SCEDev[sysIndex].portStatus[5].preStatus != 2) ) {

					if (g_pstSCEInfo->SCEDev[sysIndex].portStatus[pParam->devIndex].level != SFM_ALM_MAJOR)
						level = SFM_ALM_CRITICAL;
					else 
						level = SFM_ALM_MAJOR;
				} else  {
					if (g_pstSCEInfo->SCEDev[sysIndex].portStatus[pParam->devIndex].level == SFM_ALM_CRITICAL) {
					//	pParam->preStatus = 2;
						level = SFM_ALM_CRITICAL;
					} else {
						level = SFM_ALM_MAJOR;
					}
				}
			} else if ( (pParam->devIndex == 4) || (pParam->devIndex == 5)) {
				if ((g_pstSCEInfo->SCEDev[sysIndex].portStatus[2].preStatus != 2 /*abnomal*/)
					|| (g_pstSCEInfo->SCEDev[sysIndex].portStatus[3].preStatus != 2) ) {

					if (g_pstSCEInfo->SCEDev[sysIndex].portStatus[pParam->devIndex].level != SFM_ALM_MAJOR)
						level = SFM_ALM_CRITICAL;
					else 
						level = SFM_ALM_MAJOR;
				} else {
					if (g_pstSCEInfo->SCEDev[sysIndex].portStatus[pParam->devIndex].level == SFM_ALM_CRITICAL) {
						//pParam->preStatus = 2;
						level = SFM_ALM_CRITICAL;
					} else {
						level = SFM_ALM_MAJOR;
					}
				}
			}
		break;

		case SCE_POWER:
		case SCE_FAN:
		case SCE_RDR:
		case SCE_RDR_CONN:
			level = SFM_ALM_MAJOR;
			break;

		case SCE_TEMP:
		case SCE_VOLT:
		case SCE_STATUS:
			level = SFM_ALM_MINOR;
			break;
		/**<==== SCE **/

		case SCM_FAN:  					/* scm h/w fan */
		case SCM_PWR:  					/* scm h/w power */
		case SCM_PORT:  				/* scm h/w link */
		case SCM_MYSQL:					/* scm h/w mysql*/
		case SCM_SM:					/* scm h/w sm*/
		case SCM_CM:					/* scm h/w cm*/
		case SCM_TIMESTEN:				/* scm timesten */
		case SCM_DISK:
		case TAP_PORT:
		//case TAP_POWER:                 // 20110424 by dcham
		case L2SW_PORT:
			level = SFM_ALM_CRITICAL;
			break;
		case TAP_POWER:                 // NOT EQUIP 처리, added by dcham 2011.05.12
           level = SFM_HW_NOT_EQUIP;
		   break;
			/**< port 별 처리는 indxxxx 로 한다. **/
//		case TAP_PORT:
//		case L2SW_PORT:
		case SCM_SM_STAT:				/* scm h/w sm status */
			level = SFM_ALM_MAJOR;
			break;
	}
	return level;
}


/*
	by sjjeon
	장비별 Normal 상태 값 리턴..
*/
int fimd_getNormalStatValue(int sysIndex, int unitIndex)
{
	int stat=-1;

		switch(unitIndex)
		{
			case SCE_POWER:
				stat = 2;  /* off(2) : the power supply to the chassis is normal */
				break;
			case SCE_FAN:
				stat = 2;  /* off(2) : all fans are functional */
				break;
			case SCE_TEMP:
				stat = 2;  /* off(2) : temperature is within acceptable */
				break;
			case SCE_VOLT:
				stat = 2;  /* off(2) : voltage level is within normal range */
				break;
			case SCE_PORT_MGMT:
			case SCE_PORT_LINK:
				stat = 2;  /* up(2) :  the port is up */
				break;
			case SCE_RDR:
				stat = 2;  /* active(2) : this destination is where the reports are sent */
				break;
			case SCE_RDR_CONN: 
				stat = 2;   /* up(2) : the TCP connection to this destination is up */
				break;
			case SCE_STATUS: 
				stat = 3;   /* operational(3) :  the system is operational */
				break;
			case TAP_PORT:
				stat = 0;  /* up(0) : link up */
				break;
			case TAP_POWER: // 20110424 by dcham
				stat = 0;  /* up(0) : link up */
				break;
			case L2SW_PORT: /* up(0) : link up */
				stat = 0;
				break;

			case SCM_PORT:
			case SCM_CM:
			case SCM_SM:
			case SCM_DISK:
			case SCM_SM_STAT:
			case SCM_MYSQL:
			case SCM_FAN:
			case SCM_PWR:
			case SCM_CPU:
			case SCM_TIMESTEN:
				stat = 0;
				break;
		}

	return stat;
}
/*  End of fimd_getNormalStatValue */

int	fimd_makeSceUsageAlmMsg (SCE_USAGE_PARAM *param)
{
	int		almCode, prtLevel=0;
	char	condBuf[4096], tmpBuf[1024];
	char	occur_or_clear[12], levelSymbol[4], devName[2][5];

	almCode = fimd_getAlarmCode(param->devKind);

	if(param->sysIndex == 0) sprintf(devName[0], "SCEA");
	else if (param->sysIndex == 1) sprintf(devName[1], "SCEB");
	
	if (param->occurFlag) {
		sprintf (occur_or_clear, "OCCURED");
		prtLevel = param->curStatus;

		// level별 alarm code : sjjeon
		if(param->curStatus == SFM_ALM_MINOR) almCode += ALMCODE_STS_MINOR;
		else if(param->curStatus == SFM_ALM_MAJOR) almCode += ALMCODE_STS_MAJOR;
		else if(param->curStatus == SFM_ALM_CRITICAL) almCode += ALMCODE_STS_CRITICAL;

		switch (param->curStatus) {
			case SFM_ALM_MINOR:    sprintf (levelSymbol, "*"); break;
			case SFM_ALM_MAJOR:    sprintf (levelSymbol, "**"); break;
			case SFM_ALM_CRITICAL: sprintf (levelSymbol, "***"); break;
			default:               sprintf (levelSymbol, " "); break;
		}
	} else {
		sprintf (occur_or_clear, "CLEARED");
		prtLevel = param->preStatus;

		// level별 alarm code : sjjeon
		if(param->preStatus == SFM_ALM_MINOR) almCode += ALMCODE_STS_MINOR;
		else if(param->preStatus == SFM_ALM_MAJOR) almCode += ALMCODE_STS_MAJOR;
		else if(param->preStatus == SFM_ALM_CRITICAL) almCode += ALMCODE_STS_CRITICAL;

		switch (param->preStatus) {
			case SFM_ALM_MINOR:    sprintf (levelSymbol, "#"); break;
			case SFM_ALM_MAJOR:    sprintf (levelSymbol, "##"); break;
			case SFM_ALM_CRITICAL: sprintf (levelSymbol, "###"); break;
			default:               sprintf (levelSymbol, " "); break;
		}
	}

	sprintf(condBuf,"    %s %s\n%3s A%04d SCE DEVICE %s USAGE ALARM %s\n",
			sysLabel, commlib_printTStamp(), // system_name, 현재시각 time stamp (년,월,일,시,분,초,요일)
			levelSymbol, almCode,
			fimd_getSceDevName(param->devKind),	
			occur_or_clear);

	if (param->devKind != SCE_USER){
		sprintf(tmpBuf,"      SYSTEM   = %s\n      RESOURCE = %s(%d)\n      LEVEL    = %s\n      COMPLETED\n\n\n",
				devName[param->sysIndex],
				fimd_getSceDevName(param->devKind),	param->usage,
				fimd_printAlmLevel (prtLevel));
		strcat (condBuf, tmpBuf);
	} else {
		sprintf(tmpBuf,"      SYSTEM   = %s\n      RESOURCE = %s(%d)\n      LEVEL    = %s\n      COMPLETED\n\n\n",
				devName[param->sysIndex],
				fimd_getSceDevName(param->devKind),	param->usage,
				fimd_printAlmLevel (prtLevel));
		strcat (condBuf, tmpBuf);
	}

	// cond로 보낸다.
	// - cond가 로그파일에 기록할때, MMC/STATUS/ALARM 메시지별 구분할 수 있도록
	//	mtype을 표시해 보낸다.
	fimd_txMsg2Cond (condBuf, MTYPE_ALARM_REPORT, almCode);

	return param->curStatus;

} //----- End of fimd_makeSceUsageAlmMsg -----//

/* hjjung_20100823 */
int	fimd_makeLegUsageAlmMsg (LEG_USAGE_PARAM *param)
{
	int		almCode, prtLevel=0;
	char	condBuf[4096], tmpBuf[1024];
	char	occur_or_clear[12], levelSymbol[4], devName[2][5];

	almCode = fimd_getAlarmCode(param->devKind);
 
	if(param->sysIndex == 0) sprintf(devName[0], "SCMA");
	else if (param->sysIndex == 1) sprintf(devName[1], "SCMB");
	
	if (param->occurFlag) {
		sprintf (occur_or_clear, "OCCURED");
		prtLevel = param->curStatus;

		// level별 alarm code : sjjeon
		if(param->curStatus == SFM_ALM_MINOR) almCode += ALMCODE_STS_MINOR;
		else if(param->curStatus == SFM_ALM_MAJOR) almCode += ALMCODE_STS_MAJOR;
		else if(param->curStatus == SFM_ALM_CRITICAL) almCode += ALMCODE_STS_CRITICAL;

		switch (param->curStatus) {
			case SFM_ALM_MINOR:    sprintf (levelSymbol, "*"); break;
			case SFM_ALM_MAJOR:    sprintf (levelSymbol, "**"); break;
			case SFM_ALM_CRITICAL: sprintf (levelSymbol, "***"); break;
			default:               sprintf (levelSymbol, " "); break;
		}
	} else {
		sprintf (occur_or_clear, "CLEARED");
		prtLevel = param->preStatus;

		// level별 alarm code : sjjeon
		if(param->preStatus == SFM_ALM_MINOR) almCode += ALMCODE_STS_MINOR;
		else if(param->preStatus == SFM_ALM_MAJOR) almCode += ALMCODE_STS_MAJOR;
		else if(param->preStatus == SFM_ALM_CRITICAL) almCode += ALMCODE_STS_CRITICAL;

		switch (param->preStatus) {
			case SFM_ALM_MINOR:    sprintf (levelSymbol, "#"); break;
			case SFM_ALM_MAJOR:    sprintf (levelSymbol, "##"); break;
			case SFM_ALM_CRITICAL: sprintf (levelSymbol, "###"); break;
			default:               sprintf (levelSymbol, " "); break;
		}
	}

	sprintf(condBuf,"    %s %s\n%3s A%04d SCM DEVICE %s USAGE ALARM %s\n",
			sysLabel, commlib_printTStamp(), // system_name, 현재시각 time stamp (년,월,일,시,분,초,요일)
			levelSymbol, almCode,
			fimd_getLegDevName(param->devKind),	
			occur_or_clear);

	sprintf(tmpBuf,"      SYSTEM   = %s\n      RESOURCE = %s(%d)\n      LEVEL    = %s\n      COMPLETED\n\n\n",
			devName[param->sysIndex],
			fimd_getLegDevName(param->devKind),	param->usage,
			fimd_printAlmLevel (prtLevel));
	strcat (condBuf, tmpBuf);

	// cond로 보낸다.
	// - cond가 로그파일에 기록할때, MMC/STATUS/ALARM 메시지별 구분할 수 있도록
	//	mtype을 표시해 보낸다.
	fimd_txMsg2Cond (condBuf, MTYPE_ALARM_REPORT, almCode);

	return param->curStatus;
} //----- End of fimd_makeLegUsageAlmMsg -----//


int	fimd_makeLegTpsAlmMsg (TPS_PARAM *param)
{
	int		almCode, prtLevel=0;
	char	condBuf[4096], tmpBuf[1024];
	char	occur_or_clear[12], levelSymbol[4], devName[2][5];

	almCode = fimd_getAlarmCode(param->devKind);
 
	if(param->sysIndex == 0) sprintf(devName[0], "SCMA");
	else if (param->sysIndex == 1) sprintf(devName[1], "SCMB");
	
	if (param->occurFlag) {
		sprintf (occur_or_clear, "OCCURED");
		prtLevel = param->curStatus;

		// level별 alarm code : sjjeon
		if(param->curStatus == SFM_ALM_MINOR) almCode += ALMCODE_STS_MINOR;
		else if(param->curStatus == SFM_ALM_MAJOR) almCode += ALMCODE_STS_MAJOR;
		else if(param->curStatus == SFM_ALM_CRITICAL) almCode += ALMCODE_STS_CRITICAL;

		switch (param->curStatus) {
			case SFM_ALM_MINOR:    sprintf (levelSymbol, "*"); break;
			case SFM_ALM_MAJOR:    sprintf (levelSymbol, "**"); break;
			case SFM_ALM_CRITICAL: sprintf (levelSymbol, "***"); break;
			default:               sprintf (levelSymbol, " "); break;
		}
	} else {
		sprintf (occur_or_clear, "CLEARED");
		prtLevel = param->preStatus;

		// level별 alarm code : sjjeon
		if(param->preStatus == SFM_ALM_MINOR) almCode += ALMCODE_STS_MINOR;
		else if(param->preStatus == SFM_ALM_MAJOR) almCode += ALMCODE_STS_MAJOR;
		else if(param->preStatus == SFM_ALM_CRITICAL) almCode += ALMCODE_STS_CRITICAL;

		switch (param->preStatus) {
			case SFM_ALM_MINOR:    sprintf (levelSymbol, "#"); break;
			case SFM_ALM_MAJOR:    sprintf (levelSymbol, "##"); break;
			case SFM_ALM_CRITICAL: sprintf (levelSymbol, "###"); break;
			default:               sprintf (levelSymbol, " "); break;
		}
	}

	sprintf(condBuf,"    %s %s\n%3s A%04d SCM DEVICE %s TPS ALARM %s\n",
			sysLabel, commlib_printTStamp(), // system_name, 현재시각 time stamp (년,월,일,시,분,초,요일)
			levelSymbol, almCode,
			fimd_getLegDevName(param->devKind),	
			occur_or_clear);

	sprintf(tmpBuf,"      SYSTEM   = %s\n      RESOURCE = %s(%d)\n      LEVEL    = %s\n      COMPLETED\n\n\n",
			devName[param->sysIndex],
			fimd_getLegDevName(param->devKind),	param->usage,
			fimd_printAlmLevel (prtLevel));
	strcat (condBuf, tmpBuf);

	// cond로 보낸다.
	// - cond가 로그파일에 기록할때, MMC/STATUS/ALARM 메시지별 구분할 수 있도록
	//	mtype을 표시해 보낸다.
	fimd_txMsg2Cond (condBuf, MTYPE_ALARM_REPORT, almCode);

	return param->curStatus;
} //----- End of fimd_makeLegUsageAlmMsg -----//
int	fimd_makeSCEPortalmMsg (SCE_USAGE_PARAM *param)
{
	int		almCode;
	char	condBuf[4096], tmpBuf[1024];
	char	occur_or_clear[12], levelSymbol[4], devName[2][5];

	almCode = fimd_getAlarmCode(param->devKind);
	if(param->sysIndex == 0) sprintf(devName[0], "SCEA");
	else if (param->sysIndex == 1) sprintf(devName[1], "SCEB");
	
	if (param->occurFlag) {
		sprintf (occur_or_clear, "OCCURED");

		// level별 alarm code : sjjeon
		if(param->curStatus == SFM_ALM_MINOR) almCode += ALMCODE_STS_MINOR;
		else if(param->curStatus == SFM_ALM_MAJOR) almCode += ALMCODE_STS_MAJOR;
		else if(param->curStatus == SFM_ALM_CRITICAL) almCode += ALMCODE_STS_CRITICAL;
	
		switch (param->curStatus) {
			case SFM_ALM_MINOR:    sprintf (levelSymbol, "*"); break;
			case SFM_ALM_MAJOR:    sprintf (levelSymbol, "**"); break;
			case SFM_ALM_CRITICAL: sprintf (levelSymbol, "***"); break;
			default:               sprintf (levelSymbol, " "); break;
		}
	} else {
		sprintf (occur_or_clear, "CLEARED");

		// level별 alarm code : sjjeon
		if(param->preStatus == SFM_ALM_MINOR) almCode += ALMCODE_STS_MINOR;
		else if(param->preStatus == SFM_ALM_MAJOR) almCode += ALMCODE_STS_MAJOR;
		else if(param->preStatus == SFM_ALM_CRITICAL) almCode += ALMCODE_STS_CRITICAL;

		switch (param->preStatus) {
			case SFM_ALM_MINOR:    sprintf (levelSymbol, "#"); break;
			case SFM_ALM_MAJOR:    sprintf (levelSymbol, "##"); break;
			case SFM_ALM_CRITICAL: sprintf (levelSymbol, "###"); break;
			default:               sprintf (levelSymbol, " "); break;
		}
	}

	sprintf(condBuf,"    %s %s\n%3s A%04d SCE DEVICE %s%d LINK ALARM %s\n",
			sysLabel, commlib_printTStamp(), // system_name, 현재시각 time stamp (년,월,일,시,분,초,요일)
			levelSymbol, almCode,
			fimd_getSceDevName(param->devKind),	
			param->devIndex, occur_or_clear);

	sprintf(tmpBuf,"      SYSTEM   = %s\n      RESOURCE = %s%d\n      LEVEL    = %s\n      COMPLETED\n\n\n",
			devName[param->sysIndex],
			fimd_getSceDevName(param->devKind),	param->devIndex,
			fimd_printAlmLevel ((unsigned char)param->curStatus));
	strcat (condBuf, tmpBuf);

	// cond로 보낸다.
	// - cond가 로그파일에 기록할때, MMC/STATUS/ALARM 메시지별 구분할 수 있도록
	//	mtype을 표시해 보낸다.
	fimd_txMsg2Cond (condBuf, MTYPE_ALARM_REPORT, almCode);

	return param->curStatus;

} //----- End of fimd_makePDCpuUsageAlmMsg -----//


int	fimd_makeSCELinkalmMsg (SCE_USAGE_PARAM *param, char *dev_name)
{
	int		almCode;
	char	condBuf[4096], tmpBuf[1024];
	char	occur_or_clear[12], levelSymbol[4], devName[2][5];

	//almCode = fimd_getSceAlarmCode(param->devKind);
	almCode = fimd_getAlarmCode(param->devKind);
	if(param->sysIndex == 0) sprintf(devName[0], "SCEA");
	else if (param->sysIndex == 1) sprintf(devName[1], "SCEB");
	
	if (param->occurFlag) {
		sprintf (occur_or_clear, "OCCURED");

		// level별 alarm code : sjjeon
		if(param->curStatus == SFM_ALM_MINOR) almCode += ALMCODE_STS_MINOR;
		else if(param->curStatus == SFM_ALM_MAJOR) almCode += ALMCODE_STS_MAJOR;
		else if(param->curStatus == SFM_ALM_CRITICAL) almCode += ALMCODE_STS_CRITICAL;

		switch (param->curStatus) {
			case SFM_ALM_MINOR:    sprintf (levelSymbol, "*"); break;
			case SFM_ALM_MAJOR:    sprintf (levelSymbol, "**"); break;
			case SFM_ALM_CRITICAL: sprintf (levelSymbol, "***"); break;
			default:               sprintf (levelSymbol, " "); break;
		}
	} else {
		sprintf (occur_or_clear, "CLEARED");

		if(param->preStatus == SFM_ALM_MINOR) almCode += ALMCODE_STS_MINOR;
		else if(param->preStatus == SFM_ALM_MAJOR) almCode += ALMCODE_STS_MAJOR;
		else if(param->preStatus == SFM_ALM_CRITICAL) almCode += ALMCODE_STS_CRITICAL;

		switch (param->preStatus) {
			case SFM_ALM_MINOR:    sprintf (levelSymbol, "#"); break;
			case SFM_ALM_MAJOR:    sprintf (levelSymbol, "##"); break;
			case SFM_ALM_CRITICAL: sprintf (levelSymbol, "###"); break;
			default:               sprintf (levelSymbol, " "); break;
		}
	}

	switch (param->devIndex) {
	case 0:
		sprintf(dev_name, "Mgmt#1");		
		  break;
	case 1:
		sprintf(dev_name, "Mgmt#2");		
		  break;
	case 2:
		sprintf(dev_name, "Link#1-In");		
		  break;
	case 3:
		sprintf(dev_name, "Link#1-Out");		
		  break;
	case 4:
		sprintf(dev_name, "Link#2-In");		
		  break;
	case 5:
		sprintf(dev_name, "Link#2-Out");		
		  break;
	}

//	sprintf(condBuf,"    %s %s\n%3s A%04d SCE DEVICE %s%d LINK ALARM %s\n",
	sprintf(condBuf,"    %s %s\n%3s A%04d SCE DEVICE %s LINK ALARM %s\n",
			sysLabel, commlib_printTStamp(), // system_name, 현재시각 time stamp (년,월,일,시,분,초,요일)
			levelSymbol, almCode,
			// fimd_getSceDevName(param->devKind),	param->devIndex, 
			dev_name,
			occur_or_clear);

//	sprintf(tmpBuf,"      SYSTEM   = %s\n      RESOURCE = %s%d\n      LEVEL    = %s\n      COMPLETED\n\n\n",
	sprintf(tmpBuf,"      SYSTEM   = %s\n      RESOURCE = %s\n      LEVEL    = %s\n      COMPLETED\n\n\n",
			devName[param->sysIndex],
//			fimd_getSceDevName(param->devKind),	param->devIndex,
			dev_name,
			fimd_printAlmLevel ((unsigned char)param->curStatus));
	strcat (condBuf, tmpBuf);

	// cond로 보낸다.
	// - cond가 로그파일에 기록할때, MMC/STATUS/ALARM 메시지별 구분할 수 있도록
	//	mtype을 표시해 보낸다.
	fimd_txMsg2Cond (condBuf, MTYPE_ALARM_REPORT, almCode);

	return param->curStatus;

} //----- End of fimd_makeSCELinkalmMsg-----//

/* hjjung */
int	fimd_makeLEGLinkalmMsg (LEG_USAGE_PARAM *param, char *dev_name)
{
	int		almCode;
	char	condBuf[4096], tmpBuf[1024];
	char	occur_or_clear[12], levelSymbol[4], devName[2][5];

	//almCode = fimd_getSceAlarmCode(param->devKind);
	almCode = fimd_getAlarmCode(param->devKind);
	if(param->sysIndex == 0) sprintf(devName[0], "SCMA");
	else if (param->sysIndex == 1) sprintf(devName[1], "SCMB");
	
	if (param->occurFlag) {
		sprintf (occur_or_clear, "OCCURED");

		// level별 alarm code : sjjeon
		if(param->curStatus == SFM_ALM_MINOR) almCode += ALMCODE_STS_MINOR;
		else if(param->curStatus == SFM_ALM_MAJOR) almCode += ALMCODE_STS_MAJOR;
		else if(param->curStatus == SFM_ALM_CRITICAL) almCode += ALMCODE_STS_CRITICAL;

		switch (param->curStatus) {
			case SFM_ALM_MINOR:    sprintf (levelSymbol, "*"); break;
			case SFM_ALM_MAJOR:    sprintf (levelSymbol, "**"); break;
			case SFM_ALM_CRITICAL: sprintf (levelSymbol, "***"); break;
			default:               sprintf (levelSymbol, " "); break;
		}
	} else {
		sprintf (occur_or_clear, "CLEARED");

		if(param->preStatus == SFM_ALM_MINOR) almCode += ALMCODE_STS_MINOR;
		else if(param->preStatus == SFM_ALM_MAJOR) almCode += ALMCODE_STS_MAJOR;
		else if(param->preStatus == SFM_ALM_CRITICAL) almCode += ALMCODE_STS_CRITICAL;

		switch (param->preStatus) {
			case SFM_ALM_MINOR:    sprintf (levelSymbol, "#"); break;
			case SFM_ALM_MAJOR:    sprintf (levelSymbol, "##"); break;
			case SFM_ALM_CRITICAL: sprintf (levelSymbol, "###"); break;
			default:               sprintf (levelSymbol, " "); break;
		}
	}

	switch (param->devIndex) {
	case 0:
		sprintf(dev_name, "Mgmt#1");		
		  break;
	case 1:
		sprintf(dev_name, "Mgmt#2");		
		  break;
	case 2:
		sprintf(dev_name, "Link#1-In");		
		  break;
	case 3:
		sprintf(dev_name, "Link#1-Out");		
		  break;
	case 4:
		sprintf(dev_name, "Link#2-In");		
		  break;
	case 5:
		sprintf(dev_name, "Link#2-Out");		
		  break;
	}

	sprintf(condBuf,"    %s %s\n%3s A%04d RLEG DEVICE %s LINK ALARM %s\n",
			sysLabel, commlib_printTStamp(), // system_name, 현재시각 time stamp (년,월,일,시,분,초,요일)
			levelSymbol, almCode,
			dev_name,
			occur_or_clear);

	sprintf(tmpBuf,"      SYSTEM   = %s\n      RESOURCE = %s\n      LEVEL    = %s\n      COMPLETED\n\n\n",
			devName[param->sysIndex],
			dev_name,
			fimd_printAlmLevel ((unsigned char)param->curStatus));
	strcat (condBuf, tmpBuf);

	// cond로 보낸다.
	// - cond가 로그파일에 기록할때, MMC/STATUS/ALARM 메시지별 구분할 수 있도록
	//	mtype을 표시해 보낸다.
	fimd_txMsg2Cond (condBuf, MTYPE_ALARM_REPORT, almCode);

	return param->curStatus;

} //----- End of fimd_makeLEGLinkalmMsg-----//


int	fimd_makeSceRDRConnAlmMsg(SCE_USAGE_PARAM *param, char *dev_name)
{
	int		almCode;
	char	condBuf[4096], tmpBuf[1024];
	char	occur_or_clear[12], levelSymbol[4], devName[5];

	//almCode = fimd_getSceAlarmCode(param->devKind);
	almCode = fimd_getAlarmCode(param->devKind);
	if(param->sysIndex == 0) sprintf(devName, "SCEA");
	else if (param->sysIndex == 1) sprintf(devName, "SCEB");
	
	if (param->occurFlag) {
		sprintf (occur_or_clear, "OCCURED");

		// level별 alarm code : sjjeon
		if(param->curStatus == SFM_ALM_MINOR) almCode += ALMCODE_STS_MINOR;
		else if(param->curStatus == SFM_ALM_MAJOR) almCode += ALMCODE_STS_MAJOR;
		else if(param->curStatus == SFM_ALM_CRITICAL) almCode += ALMCODE_STS_CRITICAL;

		switch (param->curStatus) {
			case SFM_ALM_MINOR:    sprintf (levelSymbol, "*"); break;
			case SFM_ALM_MAJOR:    sprintf (levelSymbol, "**"); break;
			case SFM_ALM_CRITICAL: sprintf (levelSymbol, "***"); break;
			default:               sprintf (levelSymbol, " "); break;
		}
	} else {
		sprintf (occur_or_clear, "CLEARED");

		// level별 alarm code : sjjeon
		if(param->preStatus == SFM_ALM_MINOR) almCode += ALMCODE_STS_MINOR;
		else if(param->preStatus == SFM_ALM_MAJOR) almCode += ALMCODE_STS_MAJOR;
		else if(param->preStatus == SFM_ALM_CRITICAL) almCode += ALMCODE_STS_CRITICAL;

		switch (param->preStatus) {
			case SFM_ALM_MINOR:    sprintf (levelSymbol, "#"); break;
			case SFM_ALM_MAJOR:    sprintf (levelSymbol, "##"); break;
			case SFM_ALM_CRITICAL: sprintf (levelSymbol, "###"); break;
			default:               sprintf (levelSymbol, " "); break;
		}
	}

	sprintf(dev_name,SQL_SCE_RDR_CONN_ALM_INFO);;

	sprintf(condBuf,"    %s %s\n%3s A%04d SCE DEVICE %s ALARM %s\n",
			sysLabel, commlib_printTStamp(), // system_name, 현재시각 time stamp (년,월,일,시,분,초,요일)
			levelSymbol, almCode,
			dev_name,
			occur_or_clear);

	sprintf(tmpBuf,"      SYSTEM   = %s\n      RESOURCE = %s\n      LEVEL    = %s\n      COMPLETED\n\n\n",
			devName,
			dev_name,
			fimd_printAlmLevel ((unsigned char)param->curStatus));
	strcat (condBuf, tmpBuf);

	// cond로 보낸다.
	// - cond가 로그파일에 기록할때, MMC/STATUS/ALARM 메시지별 구분할 수 있도록
	//	mtype을 표시해 보낸다.
	fimd_txMsg2Cond (condBuf, MTYPE_ALARM_REPORT, almCode);

	return param->curStatus;

} //----- End of fimd_makeSceRDRConnAlmMsg-----//


int	fimd_makeL2LanAlmMsg (int devIndex, int gigaIndex, int occurFlag)
{
	int		almCode;
	char	condBuf[4096], tmpBuf[1024], L2swName[20], gigaSts[20], rscName[6];
	char	occur_or_clear[12], levelSymbol[4];

	//sjjeon almCode = ALMCODE_SFM_L2SW_LAN_STS;
	almCode = getAlarmCodeFromType(SFM_ALM_TYPE_L2_LAN, SFM_ALM_CRITICAL);


	bzero(L2swName, 20);

	if(devIndex == 0)
		sprintf(L2swName, "L2SW DEVICE A");
	else if (devIndex == 1)
		sprintf(L2swName, "L2SW DEVICE B");

	if(g_pstL2Dev->l2Info[devIndex].portInfo[gigaIndex].status == L3PD_RJ45_STS_PHYSIUP)
		sprintf(gigaSts, "PHYSICAL UP");
	else 
		sprintf(gigaSts, "PHYSICAL DOWN");

	sprintf(rscName, "%s(%d)",SQL_L2SW_PORT_ALM_INFO, gigaIndex+1);

	if (occurFlag == 1) {
		sprintf (occur_or_clear, "OCCURED");
		sprintf (levelSymbol, "***");
	} else {
		sprintf (occur_or_clear, "CLEARED");
		sprintf (levelSymbol, "###");
	}
//sjjeon 	sprintf(condBuf,"    %s-%s %s\n%3s A%04d L2 SWITCH NETWORK ALARM %s\n",
	sprintf(condBuf,"    %s-%s %s\n%3s A%04d L2SW DEVICE NETWORK ALARM %s\n",
			sysLabel, "L2SW",	// system_name
			commlib_printTStamp(),	// 현재시각 time stamp (년,월,일,시,분,초,요일)
			levelSymbol, almCode, occur_or_clear);

	sprintf(tmpBuf,"      SYSTEM   = %s\n      RESOURCE = %s\n      STATUS   = %s\n      COMPLETED\n\n\n",
			L2swName,
			rscName,
			gigaSts
			);
	strcat (condBuf, tmpBuf);
	// cond로 보낸다.
	// - cond가 로그파일에 기록할때, MMC/STATUS/ALARM 메시지별 구분할 수 있도록
	//	mtype을 표시해 보낸다.
	fimd_txMsg2Cond (condBuf, MTYPE_ALARM_REPORT, almCode);
	return 1;

} //----- End of fimd_makeGigaLanAlmMsg -----//

// 20100924 by dcham
int	fimd_makescmFaultStsAlmMsg (int sysIndex, int occurFlag)
{
	int		almCode;
	char	condBuf[4096], tmpBuf[1024];
	char	occur_or_clear[12], levelSymbol[4], scmName[5];

	//sjjeon almCode = ALMCODE_SFM_HWNTP;
	almCode = getAlarmCodeFromType(SFM_ALM_TYPE_SCM_FAULTED, SFM_ALM_CRITICAL);

    if(sysIndex == 1) {
		sprintf(scmName, "SCMA");
	}
	else if(sysIndex == 2) {
		sprintf(scmName, "SCMB");
	}


	if(occurFlag){
		sprintf (occur_or_clear, "OCCURED");
		sprintf (levelSymbol, "***");
	} else {
		sprintf (occur_or_clear, "CLEARED");
		sprintf (levelSymbol, "###");
	}

	sprintf(condBuf,"    %s %s\n%3s A%04d SCM FAULTED STATUS ALARM %s\n",
        	sysLabel,   // system_name
            	commlib_printTStamp(),  // 현재시각 time stamp (년,월,일,시,분,초,요일)
            	levelSymbol, almCode, occur_or_clear);

    	sprintf(tmpBuf,"      SYSTEM   = %s\n      %s\n      FAULTED STATUS CHANGE\n      COMPLETED\n\n\n",
            	sfdb->sys[sysIndex].commInfo.name, scmName);
                        
    	strcat (condBuf, tmpBuf);
	
	// cond로 보낸다.
	// - cond가 로그파일에 기록할때, MMC/STATUS/ALARM 메시지별 구분할 수 있도록
	//   mtype을 표시해 보낸다.
	fimd_txMsg2Cond (condBuf, MTYPE_ALARM_REPORT, almCode);
	//fimd_DuplicationStsEvent2Client();
	return 1;

} //----- End of fimd_makehwNTPAlmMsg -----//

/*------------------------------------------------------------------------------
*  상태 변경 시 호출되어 단순 상태 변경인지 장애 발생,해지 인지 판단하고
*  장애 또는 상태 메시지를 만들어 cond로 보낸다.

*  Parameter
*  sysType : 
*  sysIndex : 
*  unitType : 
*  unitIndex : 
*  almFlag : 
*  almLevel : 
*  occurFlag : 
------------------------------------------------------------------------------*/

int fimd_makeCommStsAlmMsg (int sysType, 
							int unitType, 
							int unitIndex, 
							int curLevel, 
							int preLevel, 
							int occurFlag, 
							char *devName )
{
	int	almCode=0;
	char sysName[10], condBuf[4096], tmpBuf[1024];
	char occur_or_clear[12], levelSymbol[4];

	bzero(sysName,sizeof(sysName));
	bzero(condBuf,sizeof(condBuf));
	bzero(tmpBuf,sizeof(tmpBuf));
	bzero(occur_or_clear,sizeof(occur_or_clear));

	switch (sysType)
	{
		case DSCM:
		case SCMA:
		case SCMB:
			if(sysType == DSCM) sprintf(sysName, "DSCM");
			else if(sysType == SCMA) sprintf(sysName, "SCMA");
			else strcpy(sysName, "SCMB");
			almCode = fimd_getAlarmCode(unitType);
		break;

		case TAPA:
		case TAPB:
			if(sysType == TAPA) sprintf(sysName, "TAPA");
			else sprintf(sysName, "TAPB");
			almCode = fimd_getAlarmCode(unitType);
		break;

		case SCEA:
		case SCEB:
			if(sysType == SCEA) sprintf(sysName, "SCEA");
			else sprintf(sysName, "SCEB");
			almCode = fimd_getAlarmCode(unitType);
		break;

		case L2SWA:
		case L2SWB:
			if(sysType == L2SWA) sprintf(sysName, "L2SWA");
			else sprintf(sysName, "L2SWB");
			almCode = fimd_getAlarmCode(unitType);
		break;

		default:
		return -1;
	}

	if (occurFlag) {
		sprintf (occur_or_clear, "OCCURED");

		// level별 alarm code : sjjeon
		if(curLevel == SFM_ALM_MINOR) almCode += ALMCODE_STS_MINOR;
		else if(curLevel == SFM_ALM_MAJOR) almCode += ALMCODE_STS_MAJOR;
		else if(curLevel == SFM_ALM_CRITICAL) almCode += ALMCODE_STS_CRITICAL;

		switch (curLevel) {
			case SFM_ALM_MINOR:    sprintf (levelSymbol, "*"); break;
			case SFM_ALM_MAJOR:    sprintf (levelSymbol, "**"); break;
			case SFM_ALM_CRITICAL: sprintf (levelSymbol, "***"); break;
			default:               sprintf (levelSymbol, " "); break;
		}
	} else {
		sprintf (occur_or_clear, "CLEARED");

		// level별 alarm code : sjjeon
		if(preLevel == SFM_ALM_MINOR) almCode += ALMCODE_STS_MINOR;
		else if(preLevel == SFM_ALM_MAJOR) almCode += ALMCODE_STS_MAJOR;
		else if(preLevel == SFM_ALM_CRITICAL) almCode += ALMCODE_STS_CRITICAL;

		switch (preLevel) {
			case SFM_ALM_MINOR:    sprintf (levelSymbol, "#"); break;
			case SFM_ALM_MAJOR:    sprintf (levelSymbol, "##"); break;
			case SFM_ALM_CRITICAL: sprintf (levelSymbol, "###"); break;
			default:               sprintf (levelSymbol, "."); break;
		}
	}

	// unit type에 따라서 alarm info 지정..
	switch (unitType)
	{
		case TAP_PORT:
			sprintf(devName,"%s(%d)",SQL_TAP_PORT_ALM_INFO, unitIndex+1);
			break;
		case TAP_POWER:
			sprintf(devName,"%s(%d)",SQL_TAP_POWER_ALM_INFO, unitIndex+1);
			break;

		case L2SW_PORT:
			sprintf(devName,"%s(%d)",SQL_L2SW_PORT_ALM_INFO, unitIndex+1);
			break;

		case SCE_POWER:
			sprintf(devName,SQL_SCE_PWR_ALM_INFO);
			break;

		case SCE_FAN:
			sprintf(devName,SQL_SCE_FAN_ALM_INFO);
			break;

		case SCE_TEMP:
			sprintf(devName,SQL_SCE_TEMP_ALM_INFO);
			break;

		case SCE_VOLT:
			sprintf(devName,SQL_SCE_VOLT_ALM_INFO);
			break;

		case SCE_PORT_MGMT:
		case SCE_PORT_LINK:
			sprintf(devName,fimd_getScePortName(unitIndex));
			break;

		case SCE_RDR_CONN:
			sprintf(devName,"%s(%d)",SQL_SCE_RDR_CONN_ALM_INFO, unitIndex+1);
			break;

		case SCE_STATUS:
			sprintf(devName,SQL_SCE_STATUS_ALM_INFO);
			break;

		default:
			sprintf(trcBuf,"[%s:%d] INVALID UNIT TYPE(%d) \n", __FUNCTION__, __LINE__, unitType);
			trclib_writeLogErr (FL,trcBuf);
			return -1;
	}

	sprintf(condBuf,"    %s %s\n%3s A%04d %6s DEVICE %s ALARM %s\n",
			sysLabel, commlib_printTStamp(), // 현재시각 time stamp (년,월,일,시,분,초,요일)
			levelSymbol, almCode,
			sysName,
			devName,
			occur_or_clear);

	sprintf(tmpBuf,"      SYSTEM   = %s\n      RESOURCE = %s\n      LEVEL    = %s\n      COMPLETED\n\n\n",
			sysName,
			devName,
			fimd_printAlmLevel ((unsigned char)curLevel));
	strcat (condBuf, tmpBuf);

	// cond로 보낸다.
	// - cond가 로그파일에 기록할때, MMC/STATUS/ALARM 메시지별 구분할 수 있도록
	//	mtype을 표시해 보낸다.
	fimd_txMsg2Cond (condBuf, MTYPE_ALARM_REPORT, almCode);
	return 0;
}
/* End of fimd_makeCommStsAlmMsg */


/*------------------------------------------------------------------------------
*  USAGE 상태 변경 시 호출되어 단순 상태 변경인지 장애 발생,해지 인지 판단하고
*  장애 또는 상태 메시지를 만들어 cond로 보낸다.

*  Parameter
*  sysType : 
*  sysIndex : 
*  usage : 
*  unitType : 
*  unitIndex : 
*  almFlag : 
*  almLevel : 
*  occurFlag : 
------------------------------------------------------------------------------*/

int fimd_makeCommUsageAlmMsg (int sysType, 
							int unitType, 
							int unitIndex, 
							int usage,
							int curLevel, 
							int preLevel, 
							int occurFlag, 
							char *devName )
{
	int	almCode=0;
	char sysName[10], condBuf[4096], tmpBuf[1024];
	char occur_or_clear[12], levelSymbol[4];

	bzero(sysName,sizeof(sysName));
	bzero(condBuf,sizeof(condBuf));
	bzero(tmpBuf,sizeof(tmpBuf));
	bzero(occur_or_clear,sizeof(occur_or_clear));

	switch (sysType)
	{
		case DSCM:
		case SCMA:
		case SCMB:
			if(sysType == DSCM) sprintf(sysName, "DSCM");
			else if(sysType == SCMA) sprintf(sysName, "SCMA");
			else strcpy(sysName, "SCMB");
			almCode = fimd_getAlarmCode(unitType);
		break;

		case TAPA:
		case TAPB:
			if(sysType == TAPA) sprintf(sysName, "TAPA");
			else sprintf(sysName, "TAPB");
			almCode = fimd_getAlarmCode(unitType);
		break;

		case SCEA:
		case SCEB:
			if(sysType == SCEA) sprintf(sysName, "SCEA");
			else sprintf(sysName, "SCEB");
			almCode = fimd_getAlarmCode(unitType);
		break;

		case L2SWA:
		case L2SWB:
			if(sysType == L2SWA) sprintf(sysName, "L2SWA");
			else sprintf(sysName, "L2SWB");
			almCode = fimd_getAlarmCode(unitType);
		break;

		default:
		return -1;
	}

	if (occurFlag) {
		sprintf (occur_or_clear, "OCCURED");

		// level별 alarm code : sjjeon
		if(curLevel == SFM_ALM_MINOR) almCode += ALMCODE_STS_MINOR;
		else if(curLevel == SFM_ALM_MAJOR) almCode += ALMCODE_STS_MAJOR;
		else if(curLevel == SFM_ALM_CRITICAL) almCode += ALMCODE_STS_CRITICAL;

		switch (curLevel) {
			case SFM_ALM_MINOR:    sprintf (levelSymbol, "*"); break;
			case SFM_ALM_MAJOR:    sprintf (levelSymbol, "**"); break;
			case SFM_ALM_CRITICAL: sprintf (levelSymbol, "***"); break;
			default:               sprintf (levelSymbol, "."); break;
		}
	} else {
		sprintf (occur_or_clear, "CLEARED");

		// level별 alarm code : sjjeon
		if(preLevel == SFM_ALM_MINOR) almCode += ALMCODE_STS_MINOR;
		else if(preLevel == SFM_ALM_MAJOR) almCode += ALMCODE_STS_MAJOR;
		else if(preLevel == SFM_ALM_CRITICAL) almCode += ALMCODE_STS_CRITICAL;

		switch (preLevel) {
			case SFM_ALM_MINOR:    sprintf (levelSymbol, "#"); break;
			case SFM_ALM_MAJOR:    sprintf (levelSymbol, "##"); break;
			case SFM_ALM_CRITICAL: sprintf (levelSymbol, "###"); break;
			default:               sprintf (levelSymbol, " "); break;
		}
	}

	// unit type에 따라서 alarm info 지정..
	switch (unitType)
	{
		case SCE_CPU:
			if(unitIndex<0 || unitIndex>=MAX_SCE_CPU_CNT ) {
				sprintf(trcBuf,"[%s] UNKNOWN CPU UNIT INDEX[%d]\n",__FUNCTION__,unitIndex);
       			trclib_writeLogErr (FL,trcBuf);
				return -1;
			}
			sprintf(devName,"%s(%d)",SQL_SCE_CPU_ALM_INFO, unitIndex+1);
			break;

		case SCE_MEM:
			if(unitIndex<0 || unitIndex>=MAX_SCE_MEM_CNT) {
				sprintf(trcBuf,"[%s] UNKNOWN MEM UNIT INDEX[%d]\n",__FUNCTION__,unitIndex);
       			trclib_writeLogErr (FL,trcBuf);
				return -1;
			}
			sprintf(devName,"%s(%d)",SQL_SCE_MEM_ALM_INFO, unitIndex+1);
			break;

		case SCE_DISK:
			sprintf(devName,"%s",SQL_SCE_DISK_ALM_INFO);
			break;

		/* hjjung_20100823 */
		case SCE_USER:
			sprintf(devName,"%s",SQL_SCE_USER_ALM_INFO);
			break;

		default:
			return -1;
			break;
	}

	sprintf(condBuf,"    %s %s\n%3s A%04d %6s DEVICE %s ALARM %s\n",
			sysLabel, commlib_printTStamp(), // 현재시각 time stamp (년,월,일,시,분,초,요일)
			levelSymbol, almCode,
			sysName,
			devName,
			occur_or_clear);

	sprintf(tmpBuf,"      SYSTEM   = %s\n      RESOURCE = %s\n      LEVEL    = %s\n      COMPLETED\n\n\n",
			sysName,
			devName,
			occurFlag ? fimd_printAlmLevel ((unsigned char)curLevel): fimd_printAlmLevel ((unsigned char)preLevel) );
	strcat (condBuf, tmpBuf);

	// cond로 보낸다.
	// - cond가 로그파일에 기록할때, MMC/STATUS/ALARM 메시지별 구분할 수 있도록
	//	mtype을 표시해 보낸다.
	fimd_txMsg2Cond (condBuf, MTYPE_ALARM_REPORT, almCode);
	return 0;
}
/* End of fimd_makeCommStsAlmMsg */



/*
   by sjjeon
   Alarm type으로 Alarm code룰 획득한다.
   추가 : alarm level 별 code가 달라지므로 level에 따라서 
   		  alarm code 에 추가적인 값을 더해준다.
		  minor (+100), major (+200), critical (+300)
*/
//int getAlarmCodeFromType(int almType)
int getAlarmCodeFromType(int almType,  int almLevel)
{
	int almCode = -1, levelVal=0;

	if((almType > MAX_ALRAM_CNT)||!(almLevel >= SFM_ALM_MINOR && almLevel <= SFM_ALM_CRITICAL)){
		sprintf(trcBuf,"[%s] Invalid alarm type=%d, level=%d\n",__FUNCTION__,almType, almLevel);
		trclib_writeLogErr (FL,trcBuf);
		return almCode;
	}

	if(almLevel == SFM_ALM_MINOR)
		levelVal = ALMCODE_STS_MINOR; 	// 100
	else if(almLevel == SFM_ALM_MAJOR)
		levelVal = ALMCODE_STS_MAJOR;	// 200
	else if(almLevel == SFM_ALM_CRITICAL)
		levelVal = ALMCODE_STS_CRITICAL; // 300

	switch(almType)
	{
		case SFM_ALM_TYPE_CPU_USAGE:       /* 1  */
			almCode = ALMCODE_SFM_CPU_USAGE;          /* 1000 // CPU 사용률 알람  */
			break;

		case SFM_ALM_TYPE_MEMORY_USAGE:    /* 2  */
			almCode = ALMCODE_SFM_MEMORY_USAGE;       /* 1001 // 메모리 사용률 알람  */
			break;

		case SFM_ALM_TYPE_DISK_USAGE:      /* 3  */
			almCode = ALMCODE_SFM_DISK_USAGE;         /* 1002 // 디스크 사용률 알람  */
			break;

		case SFM_ALM_TYPE_LAN:             /* 4  */
			almCode = ALMCODE_SFM_LAN;                /* 1003 // LAN connection 상태 알람 */
			break;

		case SFM_ALM_TYPE_PROC:            /* 5  */
			almCode = ALMCODE_SFM_PROCESS ;           /* 1004 // 프로세스 상태 알람 */
			break;

		case SFM_ALM_TYPE_LINK:            /* 6  */
			almCode = ALMCODE_SFM_LINK ;              /* 1005 // LINK connection 상태 알람*/	
			break;

		case SFM_ALM_TYPE_MP_HW:           /* 7  */
			almCode = ALMCODE_SFM_HPUX_HW;            /* 1026 */
			break;

	//	case SFM_ALM_TYPE_STAT:            /* 8  */
		//	break;

	//	case SFM_ALM_TYPE_DB_REP:          /* 9  */
		//	break;

	//	case SFM_ALM_TYPE_DB_REP_GAP:      /* 10 */
		//	break;

//		case SFM_ALM_TYPE_CONN_SERVER:     /* 11 */
//			break;

		case SFM_ALM_TYPE_CALL_INFO:       /* 12 */
			almCode = ALMCODE_SFM_CALL_INFO;          /* 1011 // hardware 의 알람 발생시*/
			break;

		case SFM_ALM_TYPE_DUP_STS:         /* 13 */
			almCode = ALMCODE_SFM_DUPLICATION;        /* 1012 // Duplication 상태 알람*/
			break;

		case SFM_ALM_TYPE_DUP_HEARTBEAT:   /* 14 */
			almCode = ALMCODE_SFM_HEARTBEAT;          /* 1013 // HEARTBEAT 상태 알람*/
			break;

		case SFM_ALM_TYPE_DUP_OOS:         /* 15 */
			almCode = ALMCODE_SFM_OOS;                /* 1014 // OOS 상태 알람  	*/
			break;

		case SFM_ALM_TYPE_SUCC_RATE:       /* 16 */
			almCode = ALMCODE_SFM_SUCCESS_RATE;       /* 1015 // SuccessRate 상태 알람*/ 
			break;

		case SFM_ALM_TYPE_SESS_LOAD:       /* 17 */
			almCode = ALMCODE_SFM_SESS_LOAD;          /* 1016 // Session에 대한 Load상태 알람*/
			break;

		case SFM_ALM_TYPE_DBCON_STST:      /* 18 */ 
			almCode = ALMCODE_SFM_RMT_DB_STS;         /* 1017 // Remote DB 상태 알람*/
			break; 

		case SFM_ALM_TYPE_RMT_LAN:         /* 19 */ 
			almCode = ALMCODE_SFM_RMT_LAN;            /* 1018 // Remote LAN Connection 상태 알람*/
			break; 

		case SFM_ALM_TYPE_OPT_LAN:         /* 20 */ 
			almCode = ALMCODE_SFM_OPT_LAN;			 /*	1019 // Optical LAN Connection 상태 알람*/
			break;

		case SFM_ALM_TYPE_HWNTP:          /* 21 */
			almCode = ALMCODE_SFM_HWNTP;              /* 1020 // hwNTP 상태 알람*/
			break;

		case SFM_ALM_TYPE_TAP_CPU_USAGE :   /* 22 */
			almCode = ALMCODE_SFM_TAP_CPU_USAGE;       /* 1021 // TAP 사용률 알람*/
			break;

		case SFM_ALM_TYPE_TAP_MEMORY_USAGE :/* 23 */
			almCode = ALMCODE_SFM_TAP_MEMORY_USAGE;    /* 1022 // TAP 메모리 사용률 알람*/
			break;

		case SFM_ALM_TYPE_TAP_FAN_STS :     /* 24 */
			almCode = ALMCODE_SFM_TAP_FAN_USAGE;       /* 1023 // TAP 팬 사용률 알람*/
			break;

		case SFM_ALM_TYPE_TAP_PORT_STS :    /* 25 */
			almCode = ALMCODE_SFM_TAP_PORT_STS;      /* 1024  TAP Port 상태 알람*/
			break;

		case SFM_ALM_TYPE_RSRC_LOAD :      /* 26 */
			almCode = ALMCODE_SFM_RSRC_LOAD;          /* 1025 */
			break;

		case SFM_ALM_TYPE_QUEUE_LOAD :     /* 27 */
			almCode = ALMCODE_SFM_QUEUE_LOAD;         /* 1027 : Queue load 상태 알람*/
			break;

		case SFM_ALM_TYPE_NMSIF_CONNECT:      /* 28 */
			almCode = ALMCODE_NMS_CONNECT;      /* 3500 : OMP NMSIF Connecton 상태알람*/
			return almCode;
			break;

		case SFM_ALM_TYPE_DUAL_ACT :       /* 29 */
			almCode = ALMCODE_SFM_DUAL_ACT;           /* 1028 */
			break;

		case SFM_ALM_TYPE_DUAL_STD :       /* 30 */
			almCode = ALMCODE_SFM_DUAL_STD;           /* 1029 */
			break;

		case SFM_ALM_TYPE_DUAL_STS_QRY_TIME_OUT:  /* 31 */
			almCode = ALMCODE_SFM_DUAL_STS_QRY_TIME_OUT;    /* 1030 */
			break;

		case SFM_ALM_TYPE_SCE_CPU :          /* 32 */
			almCode = ALMCODE_SFM_SCE_CPU_USAGE;      /* 1051 */	
			break;

		case SFM_ALM_TYPE_SCE_MEM :          /* 33 */
			almCode = ALMCODE_SFM_SCE_MEM_USAGE;      /* 1052 */
			break;

		case SFM_ALM_TYPE_SCE_DISK :         /* 34 */
			almCode = ALMCODE_SFM_SCE_DISK_USAGE;     /* 1053 */
			break;


		case SFM_ALM_TYPE_SCE_PWR :          /* 35 */
			almCode = ALMCODE_SFM_SCE_PWR_USAGE;      /* 1056 */
			break;

		case SFM_ALM_TYPE_SCE_FAN :          /* 36 */
			almCode = ALMCODE_SFM_SCE_FAN_USAGE;     /* 1055 */
			break;

		case SFM_ALM_TYPE_SCE_TEMP:          /* 37 */
			almCode = ALMCODE_SFM_SCE_TEMP_STS;       /* 1063 */
			break;

		//case SFM_ALM_TYPE_SCE_VOLT:          /* 38 */
		//break;

		case SFM_ALM_TYPE_SCE_PORT_LINK:     /* 47 */
		case SFM_ALM_TYPE_SCE_PORT_MGMT :    /* 39 */ 
			almCode = ALMCODE_SFM_SCE_PORT_USAGE;     /* 1054 */
			break;

		//case SFM_ALM_TYPE_SCE_RDR:           /* 40 */
			//	break;

		case SFM_ALM_TYPE_SCE_RDR_CONN:    /* 41 */ 
			almCode = ALMCODE_SFM_SCE_RDR_CONN_STS;   /* 1061 */	
			break;

		case SFM_ALM_TYPE_SCE_STATUS:        /* 42 */ 
			almCode = ALMCODE_SFM_SCE_SYS_STS;        /* 1062 */
			break;


		case SFM_ALM_TYPE_L2_CPU:          /* 43 */ 
			almCode = ALMCODE_SFM_L2SW_CPU_USAGE;     /* 1057 */
			break;

		case SFM_ALM_TYPE_L2_MEM:          /* 44 */ 
			almCode = ALMCODE_SFM_L2SW_MEMORY_USAGE;  /* 1058 */
			break;

		case SFM_ALM_TYPE_L2_LAN:         /* 45 */ 
			almCode = ALMCODE_SFM_L2SW_LAN_STS;     /* 1059 */
			break;

		case SFM_ALM_TYPE_CPS_OVER:         /* 46 */ 
			almCode = ALMCODE_SFM_CPS_OVER_INFO;     /* 1060 // CPS OVER 상태 알람*/
			break;
													/* sjjeon : 프로세스 장애 메시지 코드 */
		case SFM_ALM_TYPE_PROCESS_SAMD:      /*  48 : OMP,MP SAMD 상태알람  */	
			almCode = ALMCODE_SFM_PROCESS_SAMD;       /* 1071 // OMP,MP SAMD 상태알람  */
			break;

		case SFM_ALM_TYPE_PROCESS_IXPC:      /*  49 : OMP,MP IXPC 상태알람  */
			almCode = ALMCODE_SFM_PROCESS_IXPC;       /* 1072 // OMP,MP IXPC 상태알람  */
			break;

		case SFM_ALM_TYPE_PROCESS_FIMD:      /*  50 : OMP FIMD 상태알람     */
			almCode = ALMCODE_SFM_PROCESS_FIMD;       /* 1073 // OMP FIMD 상태알람     */
			break;

		case SFM_ALM_TYPE_PROCESS_COND:      /*  51 : OMP COND 상태알람     */
			almCode = ALMCODE_SFM_PROCESS_COND;       /* 1074 // OMP COND 상태알람     */
			break;

		case SFM_ALM_TYPE_PROCESS_STMD:      /*  52 : OMP STMD 상태알람     */
			almCode = ALMCODE_SFM_PROCESS_STMD;       /* 1075 // OMP STMD 상태알람     */
			break;

		case SFM_ALM_TYPE_PROCESS_MMCD:      /*  53 : OMP MMCD 상태알람     */
			almCode = ALMCODE_SFM_PROCESS_MMCD;       /* 1076 // OMP MMCD 상태알람     */
			break;

		case SFM_ALM_TYPE_PROCESS_MCDM:      /*  54 : OMP MCMD 상태알람     */
			almCode = ALMCODE_SFM_PROCESS_MCMD;       /* 1077 // OMP MCMD 상태알람     */
			break;

		case SFM_ALM_TYPE_PROCESS_NMSIF:     /*  55 : OMP NMSIF 상태알람    */
			almCode = ALMCODE_SFM_PROCESS_NMSIF;      /* 1078 // OMP NMSIF 상태알람    */
			break;

		case SFM_ALM_TYPE_PROCESS_CDELAY:    /*  56 : OMP CDELAY 상태알람   */
			almCode = ALMCODE_SFM_PROCESS_CDELAY;     /* 1079 // OMP CDELAY 상태알람   */
			break;

		case SFM_ALM_TYPE_PROCESS_HAMON:     /*  57 : OMP HAMON 상태알람    */
			almCode = ALMCODE_SFM_PROCESS_HAMON;      /* 1080 // OMP HAMON 상태알람    */
			break;

		case SFM_ALM_TYPE_PROCESS_MMCR:      /*  58 : MP MMCR 상태알람      */
			almCode = ALMCODE_SFM_PROCESS_MMCR;       /* 1081 // MP MMCR 상태알람      */
			break;

		case SFM_ALM_TYPE_PROCESS_RDRANA:    /*  59 : MP RDRANA 상태알람    */
			almCode = ALMCODE_SFM_PROCESS_RDRANA;     /* 1082 // MP RDRANA 상태알람    */
			break;
		case SFM_ALM_TYPE_PROCESS_SMPP:      /*  61 : MP SMPP 상태알람      */
			almCode = ALMCODE_SFM_PROCESS_SMPP;       /* 1084 // MP SMPP 상태알람      */
			break;

		case SFM_ALM_TYPE_PROCESS_PANA:      /*  62 : MP PANA 상태알람      */
			almCode = ALMCODE_SFM_PROCESS_PANA;       /* 1085 // MP PANA 상태알람      */
			break;

		case SFM_ALM_TYPE_PROCESS_RANA:      /*  63 : MP RANA 상태알람      */
			almCode = ALMCODE_SFM_PROCESS_RANA;       /* 1086 // MP RANA 상태알람      */
			break;

		case SFM_ALM_TYPE_PROCESS_RDRCAPD:   /*  64 : MP RDRCAPD 상태알람   */
			almCode = ALMCODE_SFM_PROCESS_RDRCAPD;    /* 1087 // MP RDRCAPD 상태알람   */
			break;

		case SFM_ALM_TYPE_PROCESS_CAPD:      /*  65 : MP CAPD 상태알람      */
			almCode = ALMCODE_SFM_PROCESS_CAPD;       /* 1088 // MP CAPD 상태알람      */
			break;

		case SFM_ALM_TYPE_PROCESS_SCEM:      /*  65 : OMP SCEM 상태알람      */
			almCode = ALMCODE_SFM_PROCESS_SCEM;       /* 1088 // OMP  SCEM 상태알람      */
			break;

		case SFM_ALM_TYPE_PROCESS_CSCM:      /*  65 : OMP CSCM 상태알람      */
			almCode = ALMCODE_SFM_PROCESS_CSCM;       /* 1088 // OMP  CSCM상태알람      */
			break;

		case SFM_ALM_TYPE_PROCESS_DIRM:      /*  65 : OMP DIRM 상태알람      */
			almCode = ALMCODE_SFM_PROCESS_DIRM;       /* 1088 // OMP DIRM 상태알람      */
			break;

		case SFM_ALM_TYPE_HW_MIRROR:		 /* 66 : MP HW MIRROR 알람 */
			almCode = ALMCODE_SFM_MIRROR_PORT; 		  /* 1092 : MIRROR PORT	*/
			break;

		case SFM_ALM_TYPE_TAP_MGMT_STS:		/* 67 : TAP MGMT 상태알람*/
			almCode = ALMCODE_SFM_TAP_MGMT_STS; 	/* 1031 : TAP Management port */
			break;

		case SFM_ALM_TYPE_L2SW_MGMT_STS:	/* 68 : L2 Switch MGMT 상태알람 */
			almCode = ALMCODE_SFM_L2SW_MGMT_STS;	/* 1032 : L2 Switch Management port */
			break;

		case SFM_ALM_TYPE_ACTIVE_STS:		/* 69 : Dual standby 상태 알람 */
			almCode = ALMCODE_SFM_ACTIVE;
			break;
		/* hjjung_20100823 */
		case SFM_ALM_TYPE_SCE_USER: 		/* 70 : SCE SUBSCRIBER NUMBER */
			almCode = ALMCODE_SFM_SCE_USER_USAGE;     /* 1093 */
			break;

		/* hjjung_20100823 */
		case SFM_ALM_TYPE_LEG_SESSION : 	/* 72 : LEG SESSION NUMBER */
			almCode = ALMCODE_SFM_LEG_SESSION_USAGE;     /* 1094 */
			break;

			// 20100915 by dcham
		case SFM_ALM_TYPE_SCM_FAULTED:      /* 74 : SCM Fault 상태 알람 */
			almCode = ALMCODE_SFM_SCM_FAULTED; /* 미지정 : SCM Fault 상태 알람 */
			break;

		case SFM_ALM_TYPE_LOGON_SUCCESS_RATE:       /* 75 : LOGON 통계 감시 */
			almCode = ALMCODE_SFM_LOGON_SUCCESS_RATE;		/* 1096 : LOGON SUCCESS RATE */
			break;

		case SFM_ALM_TYPE_LOGOUT_SUCCESS_RATE:       /* 79 : LOGON 통계 감시 */
			almCode = ALMCODE_SFM_LOGOUT_SUCCESS_RATE;		/* 1042 : LOGOUT SUCCESS RATE */
			break;
		case SFM_ALM_TYPE_TAP_POWER_STS :    /* 80  20110422 by dcham */
			almCode = ALMCODE_SFM_TAP_POWER_STS;      /* 1101  TAP Power 상태 알람*/
			break;
		case SFM_ALM_TYPE_SM_CONN_STS :    /* 81  : SM Connection Link Status */
			almCode = ALMCODE_SFM_SM_CONN_STS;      /* 1102  */
			break;
		case SFM_ALM_TYPE_PROCESS_RLEG0:      /*  MP RLEG0 상태알람    */
			almCode = ALMCODE_SFM_PROCESS_RLEG0;  
			break;
		case SFM_ALM_TYPE_PROCESS_RLEG1:      /*  MP RLEG1 상태알람      */
			almCode = ALMCODE_SFM_PROCESS_RLEG1;      
			break;
		case SFM_ALM_TYPE_PROCESS_RLEG2:      /*  MP RLEG2 상태알람      */
			almCode = ALMCODE_SFM_PROCESS_RLEG2;      
			break;
		case SFM_ALM_TYPE_PROCESS_RLEG3:      /*  MP RLEG3 상태알람      */
			almCode = ALMCODE_SFM_PROCESS_RLEG3;      
			break;
		case SFM_ALM_TYPE_PROCESS_RLEG4:      /*  MP RLEG4 상태알람      */
			almCode = ALMCODE_SFM_PROCESS_RLEG4;      
			break;
		case SFM_ALM_TYPE_TPS: 	/* 87 : LEG TPS, added by dcham 2011.05.25 */
			almCode = ALMCODE_SFM_TPS_INFO;     /* 1064 */
			break;
		default:
			break;
	}

	// 상태별 알람 코드의 상태값을 반영한다.
	almCode += levelVal;

	return almCode;
}
/* End of getAlarmCodeFromType */

/*
	by sjjeon
	process name으로 alarm type을 획득한다.  
 */
int getAlarmTypeFromProcName(int sysIndex, int procIndex)
{
	SFM_ProcInfo *procInfo=&sfdb->sys[sysIndex].commInfo.procInfo[procIndex];
	int almType = SFM_ALM_TYPE_PROC; 		// default: 5

	//fprintf(stderr,"[%s]procname : %s\n",__FUNCTION__, procInfo->name);

	if(procInfo->name[0] == 'c'|| procInfo->name[0] == 'C'){
		if(!strcasecmp(procInfo->name,"cdelay")){
			almType = SFM_ALM_TYPE_PROCESS_CDELAY;
		}else if(!strcasecmp(procInfo->name,"capd")){
			almType = SFM_ALM_TYPE_PROCESS_CAPD;
		}else if(!strcasecmp(procInfo->name,"cscm")){
			almType = SFM_ALM_TYPE_PROCESS_CSCM;
		}else if(!strcasecmp(procInfo->name,"cond")){
			almType = SFM_ALM_TYPE_PROCESS_COND;
			goto GO_OUT;
		}
	}
	else if(procInfo->name[0] == 'i' || procInfo->name[0] == 'I'){
		if(!strcasecmp(procInfo->name,"ixpc")){
			almType = SFM_ALM_TYPE_PROCESS_IXPC;
		}
		goto GO_OUT;
	}
	else if(procInfo->name[0] == 'f' || procInfo->name[0] == 'F'){
		if(!strcasecmp(procInfo->name,"fimd")){
			almType = SFM_ALM_TYPE_PROCESS_FIMD;
		}
	}
	else if(procInfo->name[0] == 'm' || procInfo->name[0] == 'M')
	{
		if(!strcasecmp(procInfo->name,"mmcd")){
			almType = SFM_ALM_TYPE_PROCESS_MMCD;
		}else if(!strcasecmp(procInfo->name,"mcdm")){
			almType = SFM_ALM_TYPE_PROCESS_MCDM;
		}else if(!strcasecmp(procInfo->name,"mmcr")){
			almType = SFM_ALM_TYPE_PROCESS_MMCR;
		}
		goto GO_OUT;
	}
	else if(procInfo->name[0] == 'n'||procInfo->name[0] == 'N'){
		if(!strcasecmp(procInfo->name,"nmsif")){
			almType = SFM_ALM_TYPE_PROCESS_NMSIF;
		}
		goto GO_OUT;
	}
	else if(procInfo->name[0] == 'h' || procInfo->name[0] == 'H'){
		if(!strcasecmp(procInfo->name,"hamon")){
			almType = SFM_ALM_TYPE_PROCESS_HAMON;
		}
		goto GO_OUT;
	}
	else if(procInfo->name[0] == 'd' || procInfo->name[0] == 'D'){
		if(!strcasecmp(procInfo->name,"dirm")){
			almType = SFM_ALM_TYPE_PROCESS_DIRM;
		}
		goto GO_OUT;
	}
	else if(procInfo->name[0] == 'p' || procInfo->name[0] == 'P'){
		if(!strcasecmp(procInfo->name,"pana")){
			almType = SFM_ALM_TYPE_PROCESS_PANA;
		}
		goto GO_OUT;
	}
	else if(procInfo->name[0] == 'r' || procInfo->name[0] == 'R'){
		if(!strcasecmp(procInfo->name,"rdrana")){
			almType = SFM_ALM_TYPE_PROCESS_RDRANA;
		}else if(!strcasecmp(procInfo->name,"rleg0")){ // 20110426 added by dcham 
			almType = SFM_ALM_TYPE_PROCESS_RLEG0;      // rleg0~rleg4
		}else if(!strcasecmp(procInfo->name,"rleg1")){
			almType = SFM_ALM_TYPE_PROCESS_RLEG1;
		}else if(!strcasecmp(procInfo->name,"rleg2")){
			almType = SFM_ALM_TYPE_PROCESS_RLEG2;
		}else if(!strcasecmp(procInfo->name,"rleg3")){
			almType = SFM_ALM_TYPE_PROCESS_RLEG3;
		}else if(!strcasecmp(procInfo->name,"rleg4")){
			almType = SFM_ALM_TYPE_PROCESS_RLEG4;
		}else if(!strcasecmp(procInfo->name,"rana")){
			almType = SFM_ALM_TYPE_PROCESS_RANA;
		}else if(!strcasecmp(procInfo->name,"rdrcapd")){
			almType = SFM_ALM_TYPE_PROCESS_RDRCAPD;
		}
		goto GO_OUT;
	}
	else if(procInfo->name[0] == 's' || procInfo->name[0] == 'S'){
		if(!strcasecmp(procInfo->name,"samd")){
			almType = SFM_ALM_TYPE_PROCESS_SAMD;
		}else if(!strcasecmp(procInfo->name,"smpp")){
			almType = SFM_ALM_TYPE_PROCESS_SMPP;
		}else if(!strcasecmp(procInfo->name,"scem")){
			almType = SFM_ALM_TYPE_PROCESS_SCEM;
		}else if(!strcasecmp(procInfo->name,"stmd")){
			almType = SFM_ALM_TYPE_PROCESS_STMD;
			goto GO_OUT;
		}
	}
	
GO_OUT:
	return almType;
}
/*End of getAlarmTypeFromProcName*/

/*
* LOGON 통계 감시를 위한 COND MESSAGE 생성
* added by uamyd 02110209
*/
int	fimd_makeLogonSuccessRateAlmMsg (int sysIndex, int log_mod, int almLevel, int occurFlag)
{
	int		almCode;
	char	condBuf[4096], tmpBuf[1024];
	char	occur_or_clear[12], levelSymbol[4], probeName[5], logMod[8];

	if( log_mod ){ //LOG-ON
		almCode = getAlarmCodeFromType(SFM_ALM_TYPE_LOGON_SUCCESS_RATE, almLevel);
		sprintf(logMod,"LOGON");
	}else{         //LOG-OUT
		almCode = getAlarmCodeFromType(SFM_ALM_TYPE_LOGOUT_SUCCESS_RATE, almLevel);
		sprintf(logMod,"LOGOUT");
	}

	if( 1 == sysIndex ){
		strcpy(probeName, "SCMA");
	} else {
		strcpy(probeName, "SCMB");
	}

	if (occurFlag) {
		sprintf (occur_or_clear, "OCCURED");
		switch (almLevel) {
			case SFM_ALM_MINOR:    sprintf (levelSymbol, "*"); break;
			case SFM_ALM_MAJOR:    sprintf (levelSymbol, "**"); break;
			case SFM_ALM_CRITICAL: sprintf (levelSymbol, "***"); break;
			default:               sprintf (levelSymbol, " "); break;
		}
	} else {
		sprintf (occur_or_clear, "CLEARED");
		switch (almLevel) {
			case SFM_ALM_MINOR:    sprintf (levelSymbol, "#"); break;
			case SFM_ALM_MAJOR:    sprintf (levelSymbol, "##"); break;
			case SFM_ALM_CRITICAL: sprintf (levelSymbol, "###"); break;
			default:               sprintf (levelSymbol, " "); break;
		}
	}

	sprintf(condBuf,"    %s %s\n%3s A%04d %s SUCCESS RATE ALARM %s\n",
			sysLabel,	// system_name
			commlib_printTStamp(),	// 현재시각 time stamp (년,월,일,시,분,초,요일)
			levelSymbol, almCode, logMod, occur_or_clear);

	sprintf(tmpBuf,"      SYSTEM   = %s\n      RESOURCE = %s_SUCCESS_RATE\n      LEVEL    = %s (%d%%)\n      COMPLETED\n\n\n",
			probeName,
			logMod,
			fimd_printAlmLevel ((unsigned char)almLevel), 
			g_pstLogonRate->rate);
	strcat (condBuf, tmpBuf);

	// cond로 보낸다.
	// - cond가 로그파일에 기록할때, MMC/STATUS/ALARM 메시지별 구분할 수 있도록
	//	mtype을 표시해 보낸다.
	fimd_txMsg2Cond (condBuf, MTYPE_ALARM_REPORT, almCode);

	return almLevel;

}

int	fimd_makeSMChStsAlmMsg(int sysIndex, int almLevel, int occurFlag, int smChID)
{
	int		almCode;
	char	condBuf[4096]; //tmpBuf[1024];
	char	probeName[5], occur_or_clear[12], status[10];
	char    *levelSymbol;

	almCode = getAlarmCodeFromType(SFM_ALM_TYPE_SM_CONN_STS, almLevel);

	if( 1 == sysIndex ){
		strcpy(probeName, "SCMA");
	} else {
		strcpy(probeName, "SCMB");
	}

	if(occurFlag){
		sprintf (occur_or_clear, "OCCURED");
		sprintf (status, "DISCONNECT");
	} else {
		sprintf (occur_or_clear, "CLEARED");
		sprintf (status, "CONNECT");
	}
	levelSymbol = fimd_printAlmLevelSymbol(almLevel, occurFlag);

	sprintf(condBuf,"    %s %s\n%3s A%04d SM CONNECTION STATUS ALARM %s\n",
			sysLabel,	// system_name
			commlib_printTStamp(),	// 현재시각 time stamp (년,월,일,시,분,초,요일)
			levelSymbol, almCode, occur_or_clear);
#if 0
	sprintf(tmpBuf,"      SYSTEM  = %s\n      CHANNEL = SMLINK(%d)\n      COMPLETED\n\n\n",
			probeName, smChID);
	strcat (condBuf, tmpBuf);
#endif

	// cond로 보낸다.
	// - cond가 로그파일에 기록할때, MMC/STATUS/ALARM 메시지별 구분할 수 있도록
	//	mtype을 표시해 보낸다.
	fimd_txMsg2Cond (condBuf, MTYPE_ALARM_REPORT, almCode);

	return 1;

}
