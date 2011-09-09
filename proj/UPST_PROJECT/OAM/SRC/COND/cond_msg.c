/***** A.1 * File Include *******************************/

/* SYS HEADER */
#include <stdio.h>
#include <string.h> /* STRLEN, STRCPY, MEMCPY, MEMSET */
/* LIB HEADER */
#include "filedb.h"
#include "loglib.h"
/* PRO HEADER */
#include "msgdef.h"
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
#include "almstat.h"
/* LOC HEADER */
#include "cond_func.h"
#include "cond_msg.h"

extern char      gszNTAMName[64];
extern pst_NTAM  fidb;

char SWLST_M_PRIMARY[MAX_SW_COUNT][30];
char SWLST_M_SECONDARY[MAX_SW_COUNT][30];
char SWLST_F_PRIMARY[MAX_SW_COUNT][30];
char SWLST_F_SECONDARY[MAX_SW_COUNT][30];
//char SWLST_F_TERTIARY[MAX_SW_COUNT][30];
char (*STR_TSW_COM)[30];


/*
* almstat.h 에 정의되어 있는 값들을 인덱스로 짝을 맞추어 String 을 사용.
*/
char *AlmLocClassMsg[]		= {" ", "H/W", "LOAD", "S/W", "CHANNEL", "NTP", "SERVICE"};
char *AlmSystemType[]		= {"TOTMON", "TAM_APP", "TAF", "DIRECTOR", "SWITCH" };

char *AlmPhscMsg[]	= {" ", "POWER", "LINK", "FAN", "DISK", "M/C", "S/C", "UPS", "CLU","BDF MIRROR", "", "", "" };
char *AlmPhscMsg2[]	= {" ", "PWR", "LINK", "FAN", "DISK", "M/C", "S/C", "UPS", "CLU", "", "", "", "", "" };

char *AlmPhscPWR[]	= {"NOT_EQUIP", "STOP", "MASK", "ON", "OFF", "OFF", "OFF" };
char *AlmPhscLink[]	= {"NOT_EQUIP", "STOP", "MASK", "UP", "DOWN", "DOWN", "DOWN" };
char *AlmPhscFAN[]	= {"NOT_EQUIP", "STOP", "MASK", "NORMAL", "ABNORMAL", "ABNORMAL", "ABNORMAL" };
char *AlmPhscDISK[]	= {"NOT_EQUIP", "STOP", "MASK", "NORMAL", "ABNORMAL", "ABNORMAL", "ABNORMAL" };

char *AlmLoadMsg[]	= { " ", "CPU", "MEM", "DISK", "QUEUE", "NIFO", "SESS", "MySQL STATUS", "CPU", "MEM"};
char *AlmLoad2[]	= {"NOT_EQUIP", "STOP", "MASK", "NORMAL", "OVER MINOR LEVEL", "OVER MAJOR LEVEL", "OVER CRITICAL LEVEL" };
char *AlmProcMsg[]	= { " ", "DAEMON", "O/S" };
char *AlmProcMsg2[]	= { " ", "PROC", "O/S" };
char *AlmProc2[]	= {"NOT_EQUIP", "STOP", "MASK", "ALIVE", "DEAD", "DEAD", "DEAD" };
char *AlmChnlMsg[]	= { " ", "TAF", "DBMS", "NMS" , "DNMS" };
char *AlmChnl2[]	= {"NOT_EQUIP", "STOP", "MASK", "UP", "DOWN", "DOWN", "DOWN" };
char *AlmNTPMsg[]	= { " ", "NTPSVR","TIMESYNC" };
char *AlmNTPMsg2[]	= {" " , "NTPSVR","TIMESYNC" };
char *STR_NTP_INV[]	= {"","DAEMON", "TIMESYNC" };

char *AlmNTP2[]		= {"NOT_EQUIP", "STOP", "MASK", "NORMAL", "ABNORMAL", "ABNORMAL", "ABNORMAL"};

char *AlmSessLoadMsg[] = { " ", "DAEMON" };
char *AlmSessLoadMsg2[] = { " ", "PROC" };

char *AlmSuccRMsg[] = { " ", "MSG" };
char *AlmSuccRMsg2[] = { " ", "MSG" };
char *AlmSuccR2[] = {"NOT_EQUIP", "STOP", "MASK", "NORMAL", "UNDER THRESHOLD", "UNDER THRESHOLD", "UNDER THRESHOLD" };

char *AlmHeadTail[] ={ "NOT EQUIP", "STOP", "MASK", "FAULT CLEARED", "FAULT OCCURED", "FAULT OCCURED", "FAULT OCCURED"};
char *AlmHeadAlmTail[] ={ "NOT EQUIP", "STOP", "MASK", "ALARM CLEARED", "ALARM OCCURED", "ALARM OCCURED", "ALARM OCCURED"};

char *AlmNTAMLinkMsg[] = { "ETH0", "ETH1", "ETH2", "ETH3", "ETH4", "ETH5", "ETH6", "ETH7", "ETH8", "ETH9", "ETH0A", "ETH0B", "ETH0C"};
char *AlmNTAFPWRMsg[] = { "A(Right)", "B(Left)", "UNKNOWN"};

char *NTAMDiskLabel[] = { "/", "/DATA", "/LOG", "/BACKUP" };
char *AlmServiceMsg[] = {"MENU","DN","STREAM","MMS","WIDGET","PHONE","EMS","BANK","FV","IM","VT","ETC","CORP","REGI","INET","RECVCALL","IM_RECV","VT_RECV"};
char *AlmSvcInvokeNo[] = {"NORMAL_ALARM","RATE_ALARM","MIN_ALARM","MAX_ALARM"};
/* added by dcham 20110616, SERVICE Alarm Grade*/

char *AlmServiceInfo[] = {" "," "," ","NORMAL","ABNORMAL","ABNORMAL","ABNORMAL"};

int Set_Cond_Msg(st_MsgQ *pstMsg, char *szTotMsg, st_CondCount *pstCnt, int ucTAMID, int ucTAFID)
{
	int				dRet, dTotLen, dTempLen;
	char			AlmInfo[1024], AlmCode[6], AlmClass[128], AlmSts[6], AlmHeader[64], szTempMsg[8192];
	long long		llLoadVal;
	st_MsgQ			stMq;
	st_almsts	    alm;

	dTotLen		= 0;
	dTempLen	= 0;
	dRet		= 0;

	memcpy(&alm, &pstMsg->szBody[0], sizeof(st_almsts));
	llLoadVal = alm.llLoadVal;

	log_print(LOGN_INFO, "ucLocType[%hu] ucSysType[%hu] ucSysNo[%hu] ucInvType[%hu] ucInvNo[%hu]", alm.ucLocType, alm.ucSysType, alm.ucSysNo, alm.ucInvType, alm.ucInvNo);
	log_print(LOGN_INFO, "ucAlmLevel[%hu] ucOldAlmLevel[%hu] ucReserv[%hu]", alm.ucAlmLevel, alm.ucOldAlmLevel, alm.ucReserv);
	log_print(LOGN_INFO, "tWhen[%10lu] uiIPAddr[%10u] llLoadVal[%lld]", alm.tWhen, alm.uiIPAddr, alm.llLoadVal);

	if(alm.ucAlmLevel > CRITICAL)
	{
		log_print(LOGN_CRI, "ALARM MESSAGE ALARM > CRI[%hu:%hu]", alm.ucAlmLevel, alm.ucOldAlmLevel);
		return -1;
	}

	memset(&stMq, 0x00, sizeof(stMq));
	memset(AlmInfo, 0x00, sizeof(AlmInfo));
	memset(AlmCode, 0x00, sizeof(AlmCode));
	memset(AlmClass, 0x00, sizeof(AlmClass));
	memset(AlmSts, 0x00, sizeof(AlmSts));
	memset(AlmHeader, 0x00, sizeof(AlmHeader));
	memset(szTempMsg, 0x00, sizeof(szTempMsg));

	if(alm.ucLocType == LOCTYPE_PROCESS)
	{
		if(alm.ucSysType == SYSTYPE_TAF)
		{
			if(alm.ucSysNo <= TYPE_F_PRIMARY)
				sprintf (AlmCode, "A%hu%hu%02hu:", alm.ucLocType, alm.ucSysType, alm.ucInvNo+1);
			else if(alm.ucSysNo <= TYPE_F_SECONDARY)
				sprintf (AlmCode, "A%hu%hu%02hu:", alm.ucLocType, alm.ucSysType+1, alm.ucInvNo+1);
		}
		else
			sprintf (AlmCode, "A%hu%hu%02hu:", alm.ucLocType, alm.ucSysType, alm.ucInvNo+1);
	}
	else if( (alm.ucLocType == LOCTYPE_PHSC) &&
		(((alm.ucSysType == SYSTYPE_DIRECT) && (alm.ucInvType == INVTYPE_POWER_DIRECTOR)) ||
		((alm.ucSysType == SYSTYPE_SWITCH) && (alm.ucInvType == INVTYPE_PORT_SWITCH))))
		sprintf (AlmCode, "A%hu%hu%hu:", alm.ucLocType, alm.ucSysType, alm.ucInvType);
	else
		sprintf (AlmCode, "A%hu%hu%hu0:", alm.ucLocType, alm.ucSysType, alm.ucInvType);

	switch(alm.ucAlmLevel)
	{
		case STOP:
			sprintf(&AlmSts[0], "****");
			sprintf(&AlmClass[0], "   CLASS  = STOP");
			break;
		case NORMAL:
			switch( alm.ucOldAlmLevel )
			{
				case STOP:
					sprintf(&AlmSts[0], "####");
					sprintf(&AlmClass[0], "   CLASS  = STOP");
					break;
				case NOT_EQUIP:
				case MASK:
				case NORMAL:
					sprintf(&AlmSts[0], " ");
					sprintf(&AlmClass[0], "   CLASS  = NORMAL");
					break;
				case MINOR :
					sprintf(&AlmSts[0], "#   ");
					sprintf(&AlmClass[0], "   CLASS  = MINOR");
					break;
				case MAJOR :
					sprintf( &AlmSts[0], "##  ");
					sprintf(&AlmClass[0], "   CLASS  = MAJOR");
					break;
				case CRITICAL:
					sprintf( &AlmSts[0], "### ");
					sprintf(&AlmClass[0], "   CLASS  = CRITICAL");
					break;
				default:
					sprintf( &AlmSts[0], "### ");
					sprintf(&AlmClass[0], "   CLASS  = CRITICAL");
					log_print(LOGN_CRI, "ALARM MESSAGE MAKE[%hu:%hu]", alm.ucAlmLevel, alm.ucOldAlmLevel);
					break;
			}
			break;
		case MINOR:
			sprintf(&AlmSts[0], "*   ");
			sprintf(&AlmClass[0], "   CLASS  = MINOR");
			break;
		case MAJOR:
			sprintf(&AlmSts[0], "**  ");
			sprintf(&AlmClass[0], "   CLASS  = MAJOR");
			break;
		case CRITICAL:
			sprintf(&AlmSts[0], "*** ");
			sprintf(&AlmClass[0], "   CLASS  = CRITICAL");
			break;
	}

	switch(alm.ucLocType)
	{
		case LOCTYPE_PHSC:
			dRet = MakeHWAlmInfo(alm, &AlmHeader[0], &AlmInfo[0]);
			break;
		case LOCTYPE_LOAD:
			dRet = MakeLOADAlmInfo(alm,  &AlmHeader[0], &AlmInfo[0], llLoadVal);
			break;
		case LOCTYPE_PROCESS:
			dRet = MakeSWAlmInfo(alm, &AlmHeader[0],  &AlmInfo[0], ucTAMID, ucTAFID);
			break;
		case LOCTYPE_CHNL:
			dRet = MakeCHNLAlmInfo(alm, &AlmHeader[0],  &AlmInfo[0]);
			break;
		case LOCTYPE_NTP:
			dRet = MakeNTPAlmInfo(alm,  &AlmHeader[0],  &AlmInfo[0]);
			break;
#if 0
		case LOCTYPE_SESSLOAD:
			dRet = MakeSESSLOADAlmInfo(alm, &AlmHeader[0],  &AlmInfo[0]);
			break;
		case LOCTYPE_SUCCRATE:
			dRet = MakeSUCCRATEAlmInfo(alm, &AlmHeader[0], &AlmInfo[0]);
			break;
		case LOCTYPE_SUCCRATE2:
			dRet = MakeSUCCRATEAlmInfo2(alm, &AlmHeader[0], &AlmInfo[0]);
			break;
#endif
		case LOCTYPE_SVC: // added by dcham 20110616
			dRet = MakeServiceAlmInfo(alm, &AlmHeader[0], &AlmInfo[0]);
			break;
	}

	if(dRet < 0)
	{
		log_print(LOGN_CRI,"ALARM MESS MAKE[%hu:%hu] LOC[%hu] RET[%d]", alm.ucAlmLevel, alm.ucOldAlmLevel, alm.ucLocType, dRet);
		return -1;
	}

	sprintf(&szTempMsg[0], "%s\n%s%s %s\n%s\n", cvtTime(alm.tWhen), AlmSts, AlmCode, AlmHeader, AlmClass);
	dTempLen = strlen(szTempMsg);

	sprintf(&szTempMsg[dTempLen], "%s\nCOMPLETED\n", AlmInfo);
	dTempLen = strlen(szTempMsg);

	dTotLen = strlen(gszNTAMName);
	memcpy(&szTotMsg[0], gszNTAMName, dTotLen);

	szTotMsg[dTotLen] = 0x20;
	dTotLen++;

	memcpy(&szTotMsg[dTotLen], &szTempMsg[0], dTempLen);
	dTotLen += dTempLen;

	szTotMsg[dTotLen] = 0x00;

	pstCnt->usTotPage	= 1;
    pstCnt->usCurPage	= 1;
    pstCnt->usSerial	= 1;

	return dTotLen;
}




int MakeHWAlmInfo(st_almsts alm, char *AlmHeader, char *AlmInfo)
{
	size_t		InfoLen;

	InfoLen	= 0;

	if( (alm.ucSysType>SYSTYPE_SWITCH) || (alm.ucInvType>INVTYPE_POWER_DIRECTOR) || (alm.ucAlmLevel>CRITICAL))
		return -1;

	sprintf(AlmHeader, "%s %s %s %s", AlmLocClassMsg[alm.ucLocType], AlmSystemType[alm.ucSysType], AlmPhscMsg[alm.ucInvType], AlmHeadTail[alm.ucAlmLevel]);
	sprintf(&AlmInfo[InfoLen], "   LOCATE = %s %02d / %s", AlmSystemType[alm.ucSysType], alm.ucSysNo, AlmPhscMsg2[alm.ucInvType]);

	InfoLen		= strlen(AlmInfo);
	switch(alm.ucSysType)
	{
		case SYSTYPE_GTAM:
		case SYSTYPE_TAM:
			switch(alm.ucInvType)
			{
				case INVTYPE_POWER:
					if(alm.ucInvNo >= MAX_PWR_COUNT)
						return -101;

					sprintf(&AlmInfo[InfoLen], " %02d\n", alm.ucInvNo);
					InfoLen = strlen(AlmInfo);
					sprintf(&AlmInfo[InfoLen], "   INFORM = %s", AlmPhscPWR[alm.ucAlmLevel]);
					set_time(alm.ucLocType, alm.ucInvType, alm.ucInvNo, alm.tWhen);
					break;

				case INVTYPE_LINK:
					if(alm.ucInvNo >= MAX_LINK_COUNT)
						return -102;

					sprintf(&AlmInfo[InfoLen], " %s\n", AlmNTAMLinkMsg[alm.ucInvNo]);
					InfoLen = strlen(AlmInfo);
					sprintf(&AlmInfo[InfoLen], "   INFORM = %s", AlmPhscLink[alm.ucAlmLevel]);
					set_time(alm.ucLocType, alm.ucInvType, alm.ucInvNo, alm.tWhen);
					break;

				case INVTYPE_FAN:
					if(alm.ucInvNo >= MAX_FAN_COUNT)
						return -103;

					sprintf(&AlmInfo[InfoLen], " %02d\n", alm.ucInvNo);
					InfoLen = strlen(AlmInfo);
					sprintf(&AlmInfo[InfoLen], "   INFORM = %s", AlmPhscFAN[alm.ucAlmLevel]);
					set_time(alm.ucLocType, alm.ucInvType, alm.ucInvNo, alm.tWhen);
					break;

				case INVTYPE_DISKARRAY:
					if(alm.ucInvNo >= MAX_DISK_COUNT)
						return -104;

					sprintf(&AlmInfo[InfoLen], " %02d\n", alm.ucInvNo);
					InfoLen = strlen(AlmInfo);
					sprintf(&AlmInfo[InfoLen], "   INFORM = %s", AlmPhscDISK[alm.ucAlmLevel]);
					set_time(alm.ucLocType, alm.ucInvType, alm.ucInvNo, alm.tWhen);
					break;

				default:
					return -10;
			}
			break;

		case SYSTYPE_TAF:
			switch(alm.ucInvType)
			{
				case INVTYPE_POWER:
					if(alm.ucInvNo >= 2)
						return -111 ;

					sprintf(&AlmInfo[InfoLen], " %s\n", AlmNTAFPWRMsg[alm.ucInvNo]);
					InfoLen = strlen(AlmInfo);
					sprintf(&AlmInfo[InfoLen], "   INFORM = %s", AlmPhscPWR[alm.ucAlmLevel]);
					break;

				case INVTYPE_LINK:
					if(alm.ucInvNo >= 8)
						return -112;

					sprintf(&AlmInfo[InfoLen], " ETH(%d)\n", alm.ucInvNo);
					InfoLen = strlen(AlmInfo);
					sprintf(&AlmInfo[InfoLen], "   INFORM = %s", AlmPhscLink[alm.ucAlmLevel]);
					break;

				case INVTYPE_FAN :
					if(alm.ucInvNo >= 6)
						return -113;

					sprintf(&AlmInfo[InfoLen], " %02d\n", alm.ucInvNo);
					InfoLen = strlen(AlmInfo);
					sprintf(&AlmInfo[InfoLen], "   INFORM = %s", AlmPhscFAN[alm.ucAlmLevel]);
					break;

				case INVTYPE_DISKARRAY:
					if(alm.ucInvNo > 1)
						return -114;

					sprintf(&AlmInfo[InfoLen], " PARTITION(%d)\n", alm.ucInvNo);
					InfoLen = strlen(AlmInfo);
					sprintf(&AlmInfo[InfoLen], "   INFORM = %s", AlmPhscDISK[alm.ucAlmLevel]);
					break;

				case INVTYPE_PORT_MONITOR:
					if(alm.ucInvNo > 2)
						return -115;

					sprintf(&AlmInfo[InfoLen], " CAPTURE CARD(%d)\n", alm.ucInvNo);
					InfoLen = strlen(AlmInfo);
					sprintf(&AlmInfo[InfoLen], "   INFORM = %s", AlmPhscDISK[alm.ucAlmLevel]);
					break;

				case INVTYPE_MIRROR_STS:
					if(alm.ucInvNo > 2)
						return -116;

					sprintf(&AlmInfo[InfoLen], " MIRROR STATUS(%d)\n", alm.ucInvNo);
					InfoLen = strlen(AlmInfo);
					sprintf(&AlmInfo[InfoLen], "   INFORM = %s", AlmPhscDISK[alm.ucAlmLevel]);
					break;

				case INVTYPE_MIRROR_ACT:
					if(alm.ucInvNo > 2)
						return -117;

					sprintf(&AlmInfo[InfoLen], " MIRROR ACTIVE(%d)\n", alm.ucInvNo);
					InfoLen = strlen(AlmInfo);
					sprintf(&AlmInfo[InfoLen], "   INFORM = %s", AlmPhscDISK[alm.ucAlmLevel]);
					break;

				default:
					return -11;
			}
			break;

		case SYSTYPE_DIRECT:
			switch(alm.ucInvType)
			{
				case INVTYPE_PORT_MONITOR:
					if(alm.ucInvNo >= MAX_MONITOR_PORT_COUNT)
						return -121;
					sprintf(&AlmInfo[InfoLen], " MONITOR_PORT[%hu]\n", alm.ucInvNo+1);
					InfoLen = strlen(AlmInfo);
					sprintf(&AlmInfo[InfoLen], "   INFORM = %s", AlmPhscLink[alm.ucAlmLevel]);
					break;
				case INVTYPE_PORT_MIRROR:
					if(alm.ucInvNo >= MAX_MIRROR_PORT_COUNT)
						return -122;
					sprintf(&AlmInfo[InfoLen], " MIRROR_PORT[%hu]\n", alm.ucInvNo+1);
					InfoLen = strlen(AlmInfo);
					sprintf(&AlmInfo[InfoLen], "   INFORM = %s", AlmPhscLink[alm.ucAlmLevel]);
					break;
				case INVTYPE_POWER_DIRECTOR:
					if(alm.ucInvNo >= MAX_DIRECT_POWER_COUNT)
						return -123;
					sprintf(&AlmInfo[InfoLen], " POWER[%hu]\n", alm.ucInvNo+1);
					InfoLen = strlen(AlmInfo);
					sprintf(&AlmInfo[InfoLen], "   INFORM = %s", AlmPhscLink[alm.ucAlmLevel]);
					break;
				default:
					return -12;
			}
			break;

		case SYSTYPE_SWITCH:
			switch(alm.ucInvType)
			{
				case INVTYPE_PORT_SWITCH:
					if(alm.ucInvNo >= MAX_SWITCH_PORT_COUNT)
						return -131;
					sprintf(&AlmInfo[InfoLen], " SWITCH_PORT[%hu]\n", alm.ucInvNo+1);
					InfoLen = strlen(AlmInfo);
					sprintf(&AlmInfo[InfoLen], "   INFORM = %s", AlmPhscLink[alm.ucAlmLevel]);
					break;
				default:
					return -13;
			}
			break;
	}

	return 0;
}

int MakeLOADAlmInfo(st_almsts alm, char *AlmHeader, char *AlmInfo, long long llLoadVal)
{
	size_t	InfoLen;

	InfoLen	= 0;

	if( (alm.ucSysType>SYSTYPE_SWITCH) || 
		(alm.ucInvType>INVTYPE_MEMORY_SWITCH) || 
		(alm.ucAlmLevel>CRITICAL))
		return -2;

	if( (alm.ucSysType == SYSTYPE_TAM) && (alm.ucInvType == INVTYPE_NIFO))
		sprintf(AlmHeader, "%s %s %s %s", AlmLocClassMsg[alm.ucLocType], AlmSystemType[alm.ucSysType], "NIFO", AlmHeadAlmTail[alm.ucAlmLevel]);
	else if( (alm.ucSysType == SYSTYPE_TAF) && (alm.ucInvType == INVTYPE_TRAFFIC))
		sprintf(AlmHeader, "%s %s %s %s", AlmLocClassMsg[alm.ucLocType], AlmSystemType[alm.ucSysType], "TRAFFIC", AlmHeadAlmTail[alm.ucAlmLevel]);
	else
		sprintf(AlmHeader, "%s %s %s %s", AlmLocClassMsg[alm.ucLocType], AlmSystemType[alm.ucSysType], AlmLoadMsg[alm.ucInvType], AlmHeadAlmTail[alm.ucAlmLevel]);

	if( (alm.ucSysType==SYSTYPE_TAM) && (alm.ucInvType==INVTYPE_DISK))
	{
		if(alm.ucInvNo < 4)
			sprintf(&AlmInfo[InfoLen], "   LOCATE = %s %02d / %s(%-7s)\n", AlmSystemType[alm.ucSysType], alm.ucSysNo, AlmLoadMsg[alm.ucInvType], NTAMDiskLabel[alm.ucInvNo]);
		else
			sprintf(&AlmInfo[InfoLen], "   LOCATE = %s %02d / %s\n", AlmSystemType[alm.ucSysType], alm.ucSysNo, AlmLoadMsg[alm.ucInvType]);
	}
	else if( (alm.ucSysType == SYSTYPE_TAM) && (alm.ucInvType == INVTYPE_NIFO))
		sprintf(&AlmInfo[InfoLen], "   LOCATE = %s %02d / %s\n", AlmSystemType[alm.ucSysType], alm.ucSysNo, "NIFO");
	else if( (alm.ucSysType == SYSTYPE_SWITCH) && (alm.ucInvType == INVTYPE_CPU_SWITCH))
		sprintf(&AlmInfo[InfoLen], "   LOCATE = %s %02d / %s[%02d]\n", AlmSystemType[alm.ucSysType], alm.ucSysNo, AlmLoadMsg[alm.ucInvType], alm.ucInvNo);
	else if( (alm.ucSysType == SYSTYPE_TAF) && (alm.ucInvType == INVTYPE_TRAFFIC))
		sprintf(&AlmInfo[InfoLen], "   LOCATE = %s %02d / %s\n", AlmSystemType[alm.ucSysType], alm.ucSysNo, "TRAFFIC");
	else
		sprintf(&AlmInfo[InfoLen], "   LOCATE = %s %02d / %s\n", AlmSystemType[alm.ucSysType], alm.ucSysNo, AlmLoadMsg[alm.ucInvType]);

	InfoLen = strlen(AlmInfo);

	if( (alm.ucSysType==SYSTYPE_TAM) && (alm.ucInvType==INVTYPE_DBSTATUS))
		sprintf(&AlmInfo[InfoLen], "   INFORM = %s", AlmProc2[alm.ucAlmLevel]);
	else
		sprintf(&AlmInfo[InfoLen], "   INFORM = %s [%2lld%%]", AlmLoad2[alm.ucAlmLevel], llLoadVal);

	return 0;
}

int MakeSWAlmInfo(st_almsts alm, char *AlmHeader, char *AlmInfo, int ucTAMID, int ucTAFID)
{
	int		InfoLen=0;

	if( (alm.ucSysType > SYSTYPE_TAF) || (alm.ucInvType > INVTYPE_USERPROC) || (alm.ucAlmLevel > CRITICAL))
		return -3;

	sprintf(AlmHeader, "%s %s %s %s", AlmLocClassMsg[alm.ucLocType],
		AlmSystemType[alm.ucSysType], AlmProcMsg[alm.ucInvType], AlmHeadAlmTail[alm.ucAlmLevel]);

	sprintf(&AlmInfo[InfoLen], "   LOCATE = %s %02d / %s", AlmSystemType[alm.ucSysType],
		alm.ucSysNo, AlmProcMsg2[alm.ucInvType]);

	InfoLen	= strlen(AlmInfo);
	switch(alm.ucSysType)
	{
		case SYSTYPE_TAM:
			if(alm.ucInvNo > MAX_SW_COUNT)
				return -301;

/***	 일단은 TAM 의 종류가 하나밖에 없으므로,( @DQMS )
			if(ucTAMID == TYPE_M_PRIMARY )
				STR_TSW_COM = SWLST_M_PRIMARY;
			else
				STR_TSW_COM = SWLST_M_SECONDARY;
***/
			STR_TSW_COM = SWLST_M_PRIMARY;

			sprintf(&AlmInfo[InfoLen], " %s\n", STR_TSW_COM[alm.ucInvNo] );
			InfoLen = strlen(AlmInfo);
			sprintf(&AlmInfo[InfoLen], "   INFORM = %s", AlmProc2[alm.ucAlmLevel] );
			break;

		case SYSTYPE_TAF:
			if(ucTAMID == TYPE_M_PRIMARY )
			{
				/* GnGi WNTAM */
				if(ucTAFID <= TYPE_F_PRIMARY)
					STR_TSW_COM = SWLST_F_PRIMARY;
				else if(ucTAFID <= TYPE_F_SECONDARY)
					STR_TSW_COM = SWLST_F_SECONDARY;
			}

			sprintf(&AlmInfo[InfoLen], " %s\n", STR_TSW_COM[alm.ucInvNo]);
			InfoLen = strlen(AlmInfo);
			sprintf(&AlmInfo[InfoLen], "   INFORM = %s", AlmProc2[alm.ucAlmLevel]);
			break;

		default :
			return -30;
	}

	return 0;
}


int MakeCHNLAlmInfo(st_almsts alm, char *AlmHeader, char *AlmInfo)
{
	int		szInfoLen;
	char	sInterlock[12];

	/*
	* ucSysType : 0x00 - GTAM(NEW), 0x01 - NTAM, 0x02 - NTAF, 0x03 - BDF
	* ucInvType : 0x01 - DBMS
	* noted by uamyd0626 06.06.20
	* changed by uamyd 20110721
	*/
	if( (alm.ucSysType>SYSTYPE_TAM) || (alm.ucInvType>INVTYPE_DBMS) || (alm.ucAlmLevel>CRITICAL))
		return -4;

	if(alm.ucInvNo > 32)
		return -40;

	if(alm.ucInvType == INVTYPE_LINK)
		sprintf(AlmHeader, "%s %s", "EXTERN CHANNEL TAM_APP", AlmHeadTail[alm.ucAlmLevel]);
	else
		sprintf(AlmHeader, "%s %s %s %s", AlmLocClassMsg[alm.ucLocType], AlmSystemType[alm.ucSysType], AlmChnlMsg[alm.ucInvType], AlmHeadTail[alm.ucAlmLevel]);

	log_print(LOGN_INFO, "alm.ucSysNo[%d], alm.ucInvNo[%d]", alm.ucSysNo, alm.ucInvNo);
	if( (alm.ucInvType!=INVTYPE_DBMS) && (alm.ucInvType!=INVTYPE_LINK))
		sprintf(AlmInfo, "   LOCATE = %s %02d / CH%d %s", AlmSystemType[alm.ucSysType], alm.ucSysNo, alm.ucInvNo+1, AlmChnlMsg[alm.ucInvType]);
	else if(alm.ucInvType == INVTYPE_DBMS)
		sprintf(AlmInfo, "   LOCATE = %s %02d DB CHANNEL", AlmSystemType[alm.ucSysType], alm.ucSysNo);
	else if(alm.ucInvType == INVTYPE_LINK)
	{
		switch(alm.ucInvNo)
		{
			//case SI_DB_INTERLOCK:
			case INVTYPE_DBMS:
				sprintf(sInterlock, "SI_DB");
				break;
			//case SI_SVCMON_INTERLOCK:
			case INVTYPE_DNMS:
				sprintf(sInterlock, "SI_SVCMON");
				break;
			//case SI_NMS_INTERLOCK:
			case INVTYPE_NMS:
				sprintf(sInterlock, "SI_NMS");
				break;
			default:
				log_print(LOGN_CRI, "F=%s:%s.%d: Invalid alm.ucInvNo[%hu]", __FILE__, __FUNCTION__, __LINE__, alm.ucInvNo);
				return -45;
		}
		sprintf(AlmInfo, "   LOCATE = %s %02d / EXTERN CH %s\n", AlmSystemType[alm.ucSysType], alm.ucSysNo, sInterlock);
	}
	else
		sprintf(AlmInfo, "   LOCATE = TAM_APP %02d / ALL CHANNEL", alm.ucSysNo);

	szInfoLen = strlen(AlmInfo);
	switch(alm.ucInvType)
	{
		case INVTYPE_LINK:
			if(alm.ucInvNo > MAX_ICH_COUNT)
				return -411;

			sprintf(&AlmInfo[szInfoLen], "   INFORM = %s", AlmChnl2[alm.ucAlmLevel]);
			set_time(alm.ucLocType, alm.ucInvType, alm.ucInvNo, alm.tWhen);
			break;
		case INVTYPE_CLIENT:
			if(alm.ucInvNo > MAX_CH_COUNT)
				return -411;

			sprintf(&AlmInfo[szInfoLen], " %s\n", szCvtIPAddr(alm.uiIPAddr));
			szInfoLen = strlen(AlmInfo);
			sprintf(&AlmInfo[szInfoLen], "   INFORM = %s", AlmChnl2[alm.ucAlmLevel]);
			set_time(alm.ucLocType, alm.ucInvType, alm.ucInvNo, alm.tWhen);
			break;

#if 0
		case INVTYPE_DB_LINK:
			/*
			* DB_CHANNEL
			* added by uamyd0626 06.06.21
			*/
			szInfoLen = strlen(AlmInfo);
			sprintf(&AlmInfo[szInfoLen],"\n   INFORM = %s",AlmChnl2[alm.ucAlmLevel]);
			set_time(alm.ucLocType, alm.ucInvType, alm.ucInvNo, alm.tWhen);
			break;
#endif

		default:
			return -41;
	}

	return 0;
}


#if 0
int MakeSESSLOADAlmInfo(st_almsts alm, char *AlmHeader, char *AlmInfo)
{
	size_t	InfoLen;

	InfoLen	= 0;
	if( (alm.ucSysType>SYSTYPE_TAM) || (alm.ucInvType>INVTYPE_SESSLOAD) || (alm.ucAlmLevel>CRITICAL))
		return -6;

	sprintf(AlmHeader, "%s %s %s %s", AlmLocClassMsg[alm.ucLocType], AlmSystemType[alm.ucSysType], AlmSessLoadMsg[alm.ucInvType], AlmHeadAlmTail[alm.ucAlmLevel]);
	sprintf(&AlmInfo[InfoLen], "   LOCATE = %s %02d / %s", AlmSystemType[alm.ucSysType], alm.ucSysNo, AlmSessLoadMsg2[alm.ucInvType]);

	InfoLen	= strlen(AlmInfo);
	switch(alm.ucInvType)
	{
		case INVTYPE_SESSLOAD:
			if(alm.ucInvNo > 32)
				return -601;

			sprintf(&AlmInfo[InfoLen], " %s\n", STR_TSW_COM[alm.ucInvNo]);
			InfoLen = strlen(AlmInfo);
			sprintf(&AlmInfo[InfoLen], "   INFORM = %s", AlmProc2[alm.ucAlmLevel]);
			fidb->tEventUpTime[16+alm.ucInvNo] = alm.tWhen;
			break;

		default:
			return -60;
	}

	return 0;
}
#endif

int MakeNTPAlmInfo(st_almsts alm, char *AlmHeader, char *AlmInfo)
{
	size_t		InfoLen;

	InfoLen	= 0;
	if( (alm.ucSysType>SYSTYPE_TAF) || (alm.ucInvType>INVTYPE_TIMESYNC) || (alm.ucAlmLevel>CRITICAL))
		return -9;

	sprintf(AlmHeader, "%s %s %s %s", AlmLocClassMsg[alm.ucLocType], AlmSystemType[alm.ucSysType], AlmNTPMsg[alm.ucInvType], AlmHeadTail[alm.ucAlmLevel]);
	sprintf(&AlmInfo[InfoLen], "   LOCATE = %s %02d / %s", AlmSystemType[alm.ucSysType], alm.ucSysNo, AlmNTPMsg2[alm.ucInvType]);

	InfoLen = strlen(AlmInfo);
	switch(alm.ucSysType)
	{
		case SYSTYPE_TAM:
			if(alm.ucInvNo > 1)
				return -901;

			sprintf(&AlmInfo[InfoLen], " %s\n", STR_NTP_INV[alm.ucInvNo] );
			InfoLen = strlen(AlmInfo);
			sprintf(&AlmInfo[InfoLen], "   INFORM = %s", AlmNTP2[alm.ucAlmLevel] );
			fidb->tEventUpTime[74+alm.ucInvNo] = alm.tWhen;
			break;

		case SYSTYPE_TAF:
			if(alm.ucInvNo > 1)
				return -902;

			sprintf(&AlmInfo[InfoLen], " %s\n", STR_NTP_INV[alm.ucInvNo] );
			InfoLen = strlen(AlmInfo);
			sprintf(&AlmInfo[InfoLen], "   INFORM = %s", AlmNTP2[alm.ucAlmLevel] );
			break;

		default:
		return -90;
	}

	return 0;
}

/* added by dcham 20110616 */
int MakeServiceAlmInfo(st_almsts alm, char *AlmHeader, char *AlmInfo)
{
	int  InfoLen=0;
    char szBuf[27];

	sprintf( AlmHeader, "%s %s %s",
		AlmLocClassMsg[alm.ucLocType],
		AlmSystemType[alm.ucSysType],
		AlmServiceMsg[alm.ucInvType]);

	sprintf( &AlmInfo[InfoLen], "   LOCATE = %s SERVICE / %s(%s)\n",
			AlmSystemType[alm.ucSysType],
			AlmServiceMsg[alm.ucInvType], 
			AlmSvcInvokeNo[alm.ucInvNo]);
	InfoLen = strlen(AlmInfo);
    /* added by dcham 2011.06.29, 동일 SERVICE내의 중복 IP에 대한 Masking 구분을 위해 L4 type 추가 */
    if(alm.ucReserv){
        if(alm.ucReserv == 1)
            strcpy(szBuf,"60300(L4_INET_TCP_USER)");
        else if(alm.ucReserv == 2)
            strcpy(szBuf,"60300(L4_INET_TCP)");
        else if(alm.ucReserv == 3)
            strcpy(szBuf,"60400(L4_INET_TCP_RECV_USER)");
        else if(alm.ucReserv == 4)
            strcpy(szBuf,"60400(L4_INET_TCP_RECV)");
        else if(alm.ucReserv == 5)
            strcpy(szBuf,"60500(L4_INET_HTTP_USER)");
        else if(alm.ucReserv == 6)
            strcpy(szBuf,"60500(L4_INET_HTTP)");
        else if(alm.ucReserv == 7)
            strcpy(szBuf,"60600(L4_INET_HTTP_RECV_USER)");
        else if(alm.ucReserv == 8)
            strcpy(szBuf,"60600(L4_INET_HTTP_RECV)");
	    sprintf(&AlmInfo[InfoLen], "   L4     = %s\n", szBuf);
	    InfoLen = strlen(AlmInfo);
    }

	sprintf(&AlmInfo[InfoLen], "   IP     = %s\n", szCvtIPAddr(alm.uiIPAddr));
	InfoLen = strlen(AlmInfo);

	sprintf(&AlmInfo[InfoLen], "   VALUE  = %lld\n", alm.llLoadVal);
	InfoLen = strlen(AlmInfo);

	sprintf(&AlmInfo[InfoLen], "   INFORM = %s", AlmServiceInfo[alm.ucAlmLevel] );

	return 0;
}
