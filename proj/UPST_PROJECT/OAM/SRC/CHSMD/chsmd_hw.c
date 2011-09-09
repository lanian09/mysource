/***** A.1 * File Include *******************************/

/* SYS HEADER */
#include <stdio.h>
#include <string.h>				/* strerror(3) */
#include <stdlib.h>				/* atoi(3) */
#include <linux/limits.h>		/* PATH_MAX	*/
#include <errno.h>
#include <pthread.h>
/* LIB HEADER */
#include "commdef.h"
#include "filedb.h"
#include "loglib.h"
/* PRO HEADER */
#include "path.h"
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
#include "almstat.h"
/* LOC HEADER */
#include "chsmd_hw.h"

/**B.1*  Definition of New Constants ******************************************/
/**B.2*  Definition of New Type  **********************************************/
/**C.1*  Declaration of Variables  ********************************************/
int hwpwrcnt;
int hwfancnt;
int hwdiskcnt;

int dDagType;

char chFlag[30];
char szRetMsg[30][LSIZE];
char szDagBinDir[30];
char szHPLogDir[30];

st_PthrdDirect	stPthrdDir[MAX_DIRECT_COUNT];
st_PthrdDirect	stPthrdSW[MAX_SWITCH_COUNT];

st_DIRECT_MNG director;
st_SWITCH_MNG swch;

/**C.2*  Declaration of Variables  ( Local ) **********************************/
extern pst_NTAM         fidb;
extern st_keepalive    *keepalive;
extern st_PthrdDirect	stPthrdDir[MAX_DIRECT_COUNT];
extern st_PthrdDirect	stPthrdSW[MAX_SWITCH_COUNT];

/**D.1*  Definition of Functions  *********************************************/
/**D.2*  Definition of Functions  *********************************************/
/******************************************************************************
	hplog result string
# hplog -p
	ID     TYPE        LOCATION      STATUS  REDUNDANT
	 1  Standard     Pwr. Supply Bay Normal     No
	 2  Standard     Pwr. Supply Bay Failed     No

# hplog -f
	ID     TYPE        LOCATION      STATUS  REDUNDANT FAN SPEED
	 1  Basic Fan    System Board    Normal     Yes     Normal
	 2  Basic Fan    System Board    Normal     Yes     Normal
	 3  Basic Fan    System Board    Normal     Yes     Normal
	 4  Basic Fan    System Board    Normal     Yes     Normal
	 5  Basic Fan    I/O Zone        Normal     Yes     Normal
	 6  Basic Fan    I/O Zone        Normal     Yes     Normal
******************************************************************************/
int dCntMsg(char *szCommand)
{
	int		i, dCnt;
	FILE	*fp;
	char	szBuf[LSIZE];

	if( (fp = popen(szCommand, "r")) == NULL)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN popen(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__,
			szCommand, errno, strerror(errno));
		return -1;
	}

	i = 0;
	while(fgets(szBuf, LSIZE, fp) != NULL)
	{
		if(i < 1)
		{
			i++;
			continue;
		}

		sprintf(&szRetMsg[i-1][0], "%s", szBuf);
		log_print(LOGN_INFO, ">[%d] %s ", i-1, szRetMsg[i-1]);
		i++;
	}
	pclose(fp);

	dCnt = i-2;

	return dCnt;
}

int dGetHWSts(char *flag, int cnt, int type)
{
	int		i, failcnt, SuccIdx, devCnt;
	char	*target, szCommand[1024], szType[32];

	SuccIdx	= type;
	switch(type)
	{
		case INVTYPE_POWER:
			sprintf(szCommand, "%s/hplog -p", szHPLogDir);
			sprintf(szType, "%s", "POWER");
			break;
		case INVTYPE_FAN:
			sprintf(szCommand, "%s/hplog -f", szHPLogDir);
			sprintf(szType, "%s", "FAN");
			break;
		default :
			log_print(LOGN_WARN, "UNVALID HW TYPE[%d]", type);
			return -1;
	}
	devCnt = dCntMsg(szCommand);

	for(failcnt = 0, i = 0; i < cnt && i < devCnt ; i++)
	{
		flag[i] = -1;
		target = strstr(&szRetMsg[i][0], "Normal");
		if(target == NULL)
			failcnt++;		/*	FAIL COUNT	*/
		else
			flag[i] = type;	/*	NORMAL		*/

		log_print(LOGN_INFO, "%s %02d STATUS %s", szType, i+1, (flag[i] < 0)?"DEAD[CRITICAL]":"ALIVE[NORMAL]" );
	}

	return devCnt;
}

/*******************************************************************************
ENDACE CARD STATUS CHECK
*******************************************************************************/
#if 0
int dGetDagSts(char *flag, int cnt, int type)
{
    char    szCommand[256];
    int     value1[16], value2[16];
    int     i, scan_cnt, failcnt;

    i           = 0;
    failcnt     = 0;

    if(dDagType < 2){
        snprintf(szCommand, sizeof(szCommand), "%s/dagthree -d /dev/dag%d -s", szDagBinDir, i);
    }else if(dDagType == 2){
        snprintf(szCommand, sizeof(szCommand), "%s/dagfour -d /dev/dag%d -s", szDagBinDir, i);
    }else{
        snprintf(szCommand, sizeof(szCommand), "%s/dagconfig -d /dev/dag%d -s", szDagBinDir, i);
	}

    /*
    * READ MESSAGE
    */
    dCntMsg( szCommand );

    /*************************************************************************
    * TWO PORT 인 경우에만 사용.
    *  출력 포맷은 아래와 같다.
    *
    *
    * case 1) 3_70G - 'Lnk' field check
    * root/>/usr/local/bin/dagthree -d /dev/dag0 -s
    * Spd Lnk FD Neg JB MA RF Err   Spd Lnk FD Neg JB MA RF Err
    * 100   1  1   1  0  1  1   0   100   1  1   1  0  1  1   0
    *
    * case 2) 3_80S - los, label, sync
    * root/>/usr/local/bin/dagthree -d /dev/dag0 -s
    *    los bip3 bip2 bip1 lop oof lof los label lcd sync
    * A:   0    0    0    0   0   0   0   0    13   0    1
    * B:   0    0    0    0   0   0   0   0    13   0    1
    *
    * case 3) 4_3GE - 'Link' field check
    * root/>/usr/local/bin/dagfour -d /dev/dag0 -s
    * Port A: Sync Link Auto RFlt Port B: Sync Link Auto RFlt
    *            1    1    0    0            1    1    0    0
    *
    * case 4) 4_5GE - 'Link' field check
    * root/>/usr/local/bin/dagconfig -d /dev/dag0 -s
    * Port  Link  PLink  RFault  LOF  LOS
    *    A     0      0       0    1    0
    *    B     0      0       0    1    0
   *************************************************************************/
    switch(dDagType){
        case TYPE_DAG3_70G :
            scan_cnt = sscanf( szRetMsg[0], "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
                        &value1[0], &value1[1], &value1[2], &value1[3],
                        &value1[4], &value1[5], &value1[6], &value1[7],
                        &value2[0], &value2[1], &value2[2], &value2[3],
                        &value2[4], &value2[5], &value2[6], &value2[7]);

            /*
            *   Port A
            */
            if( value1[1] != _UP ){
                failcnt++;
                flag[0] = -1;
            }
            else
                flag[0] = type;

            /*
            *   Port B
            */
            if( value2[1] != _UP ){
                failcnt++;
                flag[1] = -1;
            }
            else
                flag[1] = type;
            break;

        case TYPE_DAG3_80S:
            scan_cnt = sscanf( szRetMsg[0], "%*s %d %d %d %d %d %d %d %d %d %d %d",
                        &value1[0], &value1[1], &value1[2], &value1[3], &value1[4],
                        &value1[5], &value1[6], &value1[7], &value1[8], &value1[9], &value1[10] );

            scan_cnt = sscanf( szRetMsg[1], "%*s %d %d %d %d %d %d %d %d %d %d %d",
                        &value2[0], &value2[1], &value2[2], &value2[3], &value2[4],
                        &value2[5], &value2[6], &value2[7], &value2[8], &value2[9], &value2[10] );

            /*
            * Port A
            */
            if( value1[0] !=0 || value1[8] != 13 || value1[10] != 1 ){
                failcnt++;
                flag[0] = -1;
            }
            else
                flag[0] = type;

            /*
            * Port B
            */
            if( value2[0] !=0 || value2[8] != 13 || value2[10] != 1 ){
                failcnt++;
                flag[1] = -1;
            }
            else
                flag[1] = type;

            break;

        case TYPE_DAG4_3GE :
            scan_cnt = sscanf( szRetMsg[0], "%d %d %d %d %d %d %d %d",
                        &value1[0], &value1[1], &value1[2], &value1[3],
                        &value2[0], &value2[1], &value2[2], &value2[3] );
            /*
            * Port A
            */
            if( value1[1] != _UP ){
                failcnt++;
                flag[0] = -1;
            }
            else
                flag[0] = type;

            /*
            * Port B
            */
            if( value2[1] != _UP ){
                failcnt++;
                flag[1] = -1;
            }
            else
                flag[1] = type;


            break;

        case TYPE_DAG4_5GE:
            scan_cnt = sscanf(szRetMsg[0], "%*s %d %d %d %d %d", &value1[0], &value1[1], &value1[2], &value1[3], &value1[4]);
            scan_cnt = sscanf(szRetMsg[1], "%*s %d %d %d %d %d", &value2[0], &value2[1], &value2[2], &value2[3], &value2[4]);

            /*  Port A  */
            if( (value1[0] == _UP) && (value1[1] == _UP))
                flag[0] = type;
            else
            {
                failcnt++;
                flag[0] = -1;
            }

            /*  Port B  */
            if( (value2[0] == _UP) && (value2[1] == _UP))
                flag[1] = type;
            else
            {
                failcnt++;
                flag[1] = -1;
            }

            break;

        case TYPE_DAG7_5G2 :
                        scan_cnt = sscanf(szRetMsg[0], "%*s %d %d %d %d %d %*d %*d", &value1[0], &value1[1], &value1[2], &value1[3], &value1[4]);
                        scan_cnt = sscanf(szRetMsg[1], "%*s %d %d %d %d %d %*d %*d", &value2[0], &value2[1], &value2[2], &value2[3], &value2[4]);

                        /* Port A */
                        if( value1[4] == _UP ) flag[0] = type;
                        else{
                                failcnt++;
                                flag[0] = -1;
                        }

                        /* Port B */
                        if( value2[4] == _UP ) flag[1] = type;
                        else{
                                failcnt++;
                                flag[1] = -1;
                        }
                        break;
    }

    log_print(LOGN_DEBUG,"ENDACE LINK : Port A STATUS %s", (flag[0]<0)?"DOWN[CRITICAL]":"UP[NORMAL]");
    log_print(LOGN_DEBUG,"ENDACE LINK : Port B STATUS %s", (flag[1]<0)?"DOWN[CRITICAL]":"UP[NORMAL]");

    return failcnt;
}

/*******************************************************************************
DAG CARD (ENDACE CARD) TYPE CHECK
*******************************************************************************/
int dCheckDagType(void)
{
    int     i, scan_cnt;
    FILE    *fp;
    char    szBuf[LSIZE], szField[32], szValue1[16], szValue2[16], szCommand[LSIZE];

    snprintf(szCommand, sizeof(szCommand), "%s/daginf", szDagBinDir);

    if( (fp = popen( szCommand, "r" )) == NULL)
    {
        log_print(LOGN_CRI, "[%s]FAILED IN popen :[%s] [%d:%s]", __FUNCTION__, szCommand, errno, strerror(errno));
        return -1;
    }

    i = 0;
    while(fgets(szBuf, 256, fp) != NULL)
    {
        if( (scan_cnt = sscanf(szBuf, "%s %s %s", szField, szValue1, szValue2)) != 3)
            continue;
        if(strcmp(szField, "model") == 0)
        {
            if(strcmp(szValue1, "DAG") == 0)
            {
                if(strcmp(szValue2, "3.7G") == 0)
                {
                    dDagType = TYPE_DAG3_70G;
                    log_print(LOGN_CRI, "ENDACE CARD MODEL : DAG 3.7G");
                }
                else if(strcmp(szValue2, "3.8S") == 0)
                {
                    dDagType = TYPE_DAG3_80S;
                    log_print(LOGN_CRI, "ENDACE CARD MODEL : DAG 3.8S");
                }
                else if(strcmp(szValue2, "4.3GE") == 0)
                {
                    dDagType = TYPE_DAG4_3GE;
                    log_print(LOGN_CRI, "ENDACE CARD MODEL : DAG 4.3GE");
                }
                else if(strcmp(szValue2, "4.5GE") == 0)
                {
                    dDagType = TYPE_DAG4_5GE;
                    log_print(LOGN_CRI, "ENDACE CARD MODEL : DAG 4.5GE");
                }
                else if(strcmp(szValue2, "4.5G2") == 0)
                {
                    dDagType = TYPE_DAG4_5GE;
                    log_print(LOGN_CRI, "ENDACE CARD MODEL : DAG 4.5G2");
                }
                else if(strcmp(szValue2, "7.5G2") == 0)
                {
                    dDagType = TYPE_DAG7_5G2;
                    log_print(LOGN_CRI, "ENDACE CARD MODEL : DAG 7.5G2");
                }
                else
                {
                    log_print(LOGN_CRI, "[ERROR]Unvalid Dag Model[%s]",szValue2);
                    pclose(fp);
                    return -2;
                }
                break;
            }
        }
    }
    pclose(fp);

    return 0;
}
#endif

int dCheckHW(void)
{
	int		i, devCnt, hwFlag = NOT_EQUIP;

	log_print(LOGN_INFO, " # HARDWARE CHECK ############################");

	/*** Init Variable **********************************************/
	hwpwrcnt	= fidb->hwPwrCnt;
	hwfancnt	= fidb->hwFanCnt;

	if(hwpwrcnt == 0){
		log_print(LOGN_WARN, ">POWER CHECK COUNT IS ZERO[%d]", hwpwrcnt);
	}else{
		hwFlag++;
	}

	if(hwfancnt == 0){
		log_print(LOGN_WARN, ">FAN CHECK COUNT IS ZERO[%d]", hwfancnt);
	}else{
		hwFlag++;
	}

	if( NOT_EQUIP == hwFlag ){
		return -1;
	}

	/*** 1. POWER CHECK ***********************************************/
	devCnt = dGetHWSts(chFlag, hwpwrcnt, INVTYPE_POWER);

	for( i = 0; i< hwpwrcnt && i < devCnt; i++ )
	{
		log_print(LOGN_INFO," # POWER %02d STATUS[0x%x] ", i, fidb->hwPWR[i] );
		if(chFlag[i] < 0)
		{
			/* CRITICAL STATUS **************************************/
			if( (fidb->hwPWR[i]!=CRITICAL) && (fidb->hwPWR[i]!=NOT_EQUIP) && (fidb->hwPWR[i]<MASK))
			{
				Send_CondMess(SYSTYPE_TAM, LOCTYPE_PHSC, INVTYPE_POWER, i, CRITICAL, fidb->hwPWR[i]);
				fidb->hwPWR[i] = CRITICAL;
				log_print(LOGN_CRI, "ALARM OCCURED POWER[%d] STATUS : CRITICAL", i);
			}
		}
		else
		{
			/* NORMAL STATUS *************************************/
			if( (fidb->hwPWR[i]!=NORMAL) && (fidb->hwPWR[i]<MASK))
			{
				Send_CondMess(SYSTYPE_TAM, LOCTYPE_PHSC, INVTYPE_POWER, i, NORMAL, fidb->hwPWR[i]);
				fidb->hwPWR[i] = NORMAL;
				log_print(LOGN_DEBUG,"ALARM CLEARED POWER[%d] STATUS : NORMAL", i);
			}
		}
		log_print(LOGN_INFO," # POWER %02d STATUS[0x%x] ", i, fidb->hwPWR[i] );
	}
	
	if( hwpwrcnt > devCnt ){
		for( i = devCnt; i< hwpwrcnt; i++ ){
			if( fidb->hwPWR[i] < MASK) fidb->hwPWR[i] = NOT_EQUIP;
		}
	}

	/*** 2. FAN CHECK *************************************************/
	devCnt = dGetHWSts( chFlag, hwfancnt, INVTYPE_FAN );

	for(i = 0; i< hwfancnt && i < devCnt; i++)
	{
		if(chFlag[i] < 0)
		{
			/* CRITICAL STATUS **************************************/
			if( (fidb->hwFan[i]!=CRITICAL) && (fidb->hwFan[i]!=NOT_EQUIP) && (fidb->hwFan[i]<MASK))
			{
				Send_CondMess(SYSTYPE_TAM, LOCTYPE_PHSC, INVTYPE_FAN, i, CRITICAL, fidb->hwFan[i]);
				fidb->hwFan[i] = CRITICAL;
				log_print(LOGN_CRI,"ALARM OCCURED FAN[%d] STATUS : CRITICAL",i);
			}
		}
		else
		{
			/* NORMAL STATUS *************************************/
			if( (fidb->hwFan[i]!=NORMAL) && (fidb->hwFan[i]<MASK))
			{
				Send_CondMess(SYSTYPE_TAM, LOCTYPE_PHSC, INVTYPE_FAN, i, NORMAL, fidb->hwFan[i]);
				fidb->hwFan[i] = NORMAL;
				log_print(LOGN_DEBUG,"ALARM CLEARED FAN[%d] STATUS : NORMAL", i );
			}
		}
		log_print(LOGN_INFO," # FAN %02d STATUS[%s] ", i, (fidb->hwFan[i]==NORMAL)?"NORMAL[0x03]":"CRITICAL[0x06]");
	}
	
	if( hwfancnt > devCnt ){
		for( i = devCnt; i < hwfancnt; i++ ){
			if( fidb->hwFan[i] < MASK) fidb->hwFan[i] = NOT_EQUIP;
		}
	}

#if 0
	/*** 3. DAG CHECK : ENDACE CARD LINK STATUS ******************************************/
    dGetDagSts(chFlag, hwportcnt, INVTYPE_PORT);

    for( i=0; i< hwportcnt; i++ ){
        if( chFlag[i] < 0 ){
            /*** CRITICAL **********************************/
            if( fidb->portsts[i] != CRITICAL
                && fidb->portsts[i] != NOT_EQUIP
                && fidb->portsts[i] < MASK ){
                Send_CondMess( SYSTYPE_TAM, LOCTYPE_PHSC, INVTYPE_PORT, i, CRITICAL, fidb->portsts[i] );
                fidb->portsts[i] = CRITICAL;
                log_print(LOGN_CRI, "ALARM OCCURED DAG LINK[%d] PORT[%d] DOWN", i/2, i%2);
            }
        }
        else{
            /*** NORMAL   **********************************/
            if( fidb->portsts[i] != NORMAL
                && fidb->portsts[i] < MASK ){
                Send_CondMess( SYSTYPE_TAM, LOCTYPE_PHSC, INVTYPE_PORT, i, NORMAL, fidb->portsts[i] );
                fidb->portsts[i] = NORMAL;
                log_print(LOGN_DEBUG, "ALARM CLEARED DAG LINK[%d] PORT[%d] UP", i/2, i%2);
            }
        }
    }
#endif

	return 0;
}

#ifdef CLI_VERSION
void CheckDirector(int dMyID)
{
	int		dCriticalIdx, dScanNumber;
	char	sCmd[BUFSIZ], sResult[BUFSIZ], n1[(MAX_MIRROR_PORT_COUNT/2)], n2[(MAX_MIRROR_PORT_COUNT/2)], m1[MAX_MONITOR_PORT_COUNT];
	time_t	tUpdate;
	FILE	*fp;

	tUpdate = time(NULL);
	sprintf(sCmd, "%s/SCRIPT/SNMP/dis-tap.sh %d port", START_PATH, dMyID+1);
	if( (fp = popen(sCmd, "r")) == NULL)
		return;

	if(fgets(sResult, BUFSIZ, fp) != NULL)
	{
		pclose(fp);
		if( (dScanNumber = sscanf(sResult, "%c %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c",
				&n1[0], &n1[1], &n1[2], &n1[3], &n1[4], &n1[5], &n1[6], &n1[7], &n1[8], &n1[9], &n1[10], &n1[11],
				&n2[0], &n2[1], &n2[2], &n2[3], &n2[4], &n2[5], &n2[6], &n2[7], &n2[8], &n2[9], &n2[10], &n2[11],
				&m1[0], &m1[1], &m1[2], &m1[3], &m1[4], &m1[5], &m1[6], &m1[7], &m1[8], &m1[9])) == 34)
		{
			DecideResult(dMyID, tUpdate, n1, n2, m1);
		}
		else
		{
			for(dCriticalIdx = 0; dCriticalIdx < MAX_MONITOR_PORT_COUNT; dCriticalIdx++)
			{
				if(director.stDIRECT[dMyID].cMonitorPort[dCriticalIdx] != NOT_EQUIP)
				{
					if(director.stDIRECT[dMyID].cMonitorPort[dCriticalIdx] != CRITICAL)
						Send_CondDirSWMess(dMyID, INVTYPE_PORT_MONITOR, dCriticalIdx, CRITICAL, director.stDIRECT[dMyID].cMonitorPort[dCriticalIdx]);

					pthread_mutex_lock(&stPthrdDir[dMyID].PthrdMutex);
					director.stDIRECT[dMyID].cMirrorPort[dCriticalIdx] = CRITICAL;
					pthread_mutex_unlock(&stPthrdDir[dMyID].PthrdMutex);
				}
			}

			for(dCriticalIdx = 0; dCriticalIdx < MAX_MIRROR_PORT_COUNT; dCriticalIdx++)
			{
				if(director.stDIRECT[dMyID].cMirrorPort[dCriticalIdx] != NOT_EQUIP)
				{
					if(director.stDIRECT[dMyID].cMirrorPort[dCriticalIdx] != CRITICAL)
						Send_CondDirSWMess(dMyID, INVTYPE_PORT_MIRROR, dCriticalIdx, CRITICAL, director.stDIRECT[dMyID].cMirrorPort[dCriticalIdx]);

					pthread_mutex_lock(&stPthrdDir[dMyID].PthrdMutex);
					director.stDIRECT[dMyID].cMirrorPort[dCriticalIdx] = CRITICAL;
					pthread_mutex_unlock(&stPthrdDir[dMyID].PthrdMutex);
				}
			}

		}
		pthread_mutex_lock(&stPthrdDir[dMyID].PthrdMutex);
		director.stDIRECT[dMyID].tEachUpTime = tUpdate;
		pthread_mutex_unlock(&stPthrdDir[dMyID].PthrdMutex);
	}
}

void DecideResult(int dDirectIdx, time_t tUpdate, char *n1, char *n2, char *m1)
{
	int		dIndex, cCheckAlarm;

	if(director.cDirectorMask[dDirectIdx]!=MASK)
	{
		for(dIndex = 0; dIndex < MAX_MONITOR_PORT_COUNT; dIndex++)
		{
			if(director.stDIRECT[dDirectIdx].cMonitorPort[dIndex]!=NOT_EQUIP)
			{
				if(m1[dIndex] == '1')
					cCheckAlarm = NORMAL;
				else
					cCheckAlarm = CRITICAL;

				if(director.stDIRECT[dDirectIdx].cMonitorPort[dIndex] != cCheckAlarm)
					Send_CondDirSWMess(dDirectIdx, INVTYPE_PORT_MONITOR, dIndex, cCheckAlarm, director.stDIRECT[dDirectIdx].cMonitorPort[dIndex]);

				pthread_mutex_lock(&stPthrdDir[dDirectIdx].PthrdMutex);
				director.stDIRECT[dDirectIdx].cMonitorPort[dIndex] = cCheckAlarm;
				pthread_mutex_unlock(&stPthrdDir[dDirectIdx].PthrdMutex);
			}
		}

		for(dIndex = 0; dIndex < MAX_MIRROR_PORT_COUNT; dIndex++)
		{
			if(director.stDIRECT[dDirectIdx].cMirrorPort[dIndex]!=NOT_EQUIP)
			{
				if(dIndex < (MAX_MIRROR_PORT_COUNT/2))
				{
					if(n1[dIndex] == '1')
						cCheckAlarm = NORMAL;
					else
						cCheckAlarm = CRITICAL;
				}
				else
				{
					if(n2[dIndex-(MAX_MIRROR_PORT_COUNT/2)] == '1')
						cCheckAlarm = NORMAL;
					else
						cCheckAlarm = CRITICAL;
				}

				if(director.stDIRECT[dDirectIdx].cMirrorPort[dIndex] != cCheckAlarm)
					Send_CondDirSWMess(dDirectIdx, INVTYPE_PORT_MIRROR, dIndex, cCheckAlarm, director.stDIRECT[dDirectIdx].cMirrorPort[dIndex]);

				pthread_mutex_lock(&stPthrdDir[dDirectIdx].PthrdMutex);
				director.stDIRECT[dDirectIdx].cMirrorPort[dIndex] = cCheckAlarm;
				pthread_mutex_unlock(&stPthrdDir[dDirectIdx].PthrdMutex);
			}
		}
	}
}
#else
void CheckDirector(int dMyID)
{
	int		dLine, dScanNumber;
	char	sCmd[BUFSIZ], sResult[BUFSIZ], cTmp, status;
	time_t	tUpdate;

	tUpdate = time(NULL);
	sprintf(sCmd, "%s/SCRIPT/SNMP/snmp-tap.sh %d", START_PATH, dMyID);
	if( (stPthrdDir[dMyID].fPipe = popen(sCmd, "r")) == NULL)
		return;

	dLine	= 0;
	while(fgets(sResult, BUFSIZ, stPthrdDir[dMyID].fPipe) != NULL)
	{
		if( (dScanNumber = sscanf(sResult, "%c", &cTmp)) == 1)
		{
			if(dLine < MAX_MONITOR_PORT_COUNT)
			{
				status = director.stDIRECT[dMyID].cMonitorPort[dLine];
				log_print(LOGN_INFO,"%s.MONITOR(%d):%c[%s]",__FUNCTION__,dLine,cTmp, status&MASK?"MASKED":"UNMASK");
				if(cTmp == '1')
				{
					if( (status != NOT_EQUIP) && (status & MASK))
					{
						if(status != NORMAL)
							Send_CondDirSWMess(dMyID, INVTYPE_PORT_MONITOR, dLine, NORMAL, status);
						pthread_mutex_lock(&stPthrdDir[dMyID].PthrdMutex);
						director.stDIRECT[dMyID].cMonitorPort[dLine] = NORMAL;
						pthread_mutex_unlock(&stPthrdDir[dMyID].PthrdMutex);
					}
				}
				else
				{
					if( (status != NOT_EQUIP) && (status & MASK))
					{
						if(status != CRITICAL)
							Send_CondDirSWMess(dMyID, INVTYPE_PORT_MONITOR, dLine, CRITICAL, status);

						pthread_mutex_lock(&stPthrdDir[dMyID].PthrdMutex);
						director.stDIRECT[dMyID].cMonitorPort[dLine] = CRITICAL;
						pthread_mutex_unlock(&stPthrdDir[dMyID].PthrdMutex);
					}
				}
			}
			else if( (dLine >= MAX_MONITOR_PORT_COUNT) && (dLine < (MAX_MONITOR_PORT_COUNT+MAX_MIRROR_PORT_COUNT)))
			{
				status = director.stDIRECT[dMyID].cMirrorPort[dLine-MAX_MONITOR_PORT_COUNT];
				log_print(LOGN_INFO,"%s.MIRROR(%d):%c[%s]",__FUNCTION__,dLine,cTmp, status&MASK?"MASKED":"UNMASK");
				if(cTmp == '1')
				{
					if( (status != NOT_EQUIP) && (status & MASK))
					{
						if(status != NORMAL)
							Send_CondDirSWMess(dMyID, INVTYPE_PORT_MIRROR, (dLine-MAX_MONITOR_PORT_COUNT), NORMAL, status);

						pthread_mutex_lock(&stPthrdDir[dMyID].PthrdMutex);
						director.stDIRECT[dMyID].cMirrorPort[dLine-MAX_MONITOR_PORT_COUNT] = NORMAL;
						pthread_mutex_unlock(&stPthrdDir[dMyID].PthrdMutex);
					}
				}
				else
				{
					if( (status != NOT_EQUIP) && (status & MASK))
					{
						if(status != CRITICAL)
							Send_CondDirSWMess(dMyID, INVTYPE_PORT_MIRROR, (dLine-MAX_MONITOR_PORT_COUNT), CRITICAL,status); 

						pthread_mutex_lock(&stPthrdDir[dMyID].PthrdMutex);
						director.stDIRECT[dMyID].cMirrorPort[dLine-MAX_MONITOR_PORT_COUNT] = CRITICAL;
						pthread_mutex_unlock(&stPthrdDir[dMyID].PthrdMutex);
					}
				}
			}
			else if( (dLine >= (MAX_MONITOR_PORT_COUNT+MAX_MIRROR_PORT_COUNT)) && (dLine < (MAX_MONITOR_PORT_COUNT+MAX_MIRROR_PORT_COUNT+MAX_DIRECT_POWER_COUNT)))
			{
				status = director.stDIRECT[dMyID].cPower[dLine-(MAX_MONITOR_PORT_COUNT+MAX_MIRROR_PORT_COUNT)];
				log_print(LOGN_INFO,"%s.POWER(%d):%c[%s]",__FUNCTION__,dLine,cTmp,status&MASK?"MASKED":"UNMASK");
				if(cTmp == '1')
				{
					if( (status != NOT_EQUIP) && (status & MASK))
					{
						if(status != NORMAL)
							Send_CondDirSWMess(dMyID, INVTYPE_POWER_DIRECTOR, (dLine-(MAX_MONITOR_PORT_COUNT+MAX_MIRROR_PORT_COUNT)), NORMAL, status);

						pthread_mutex_lock(&stPthrdDir[dMyID].PthrdMutex);
						director.stDIRECT[dMyID].cPower[dLine-(MAX_MONITOR_PORT_COUNT+MAX_MIRROR_PORT_COUNT)] = NORMAL;
						pthread_mutex_unlock(&stPthrdDir[dMyID].PthrdMutex);
					}
				}
				else
				{
					if( (status != NOT_EQUIP) && (status & MASK))
					{
						if(status != CRITICAL)
							Send_CondDirSWMess(dMyID, INVTYPE_POWER_DIRECTOR, (dLine-(MAX_MONITOR_PORT_COUNT+MAX_MIRROR_PORT_COUNT)), CRITICAL, status);

						pthread_mutex_lock(&stPthrdDir[dMyID].PthrdMutex);
						director.stDIRECT[dMyID].cPower[dLine-(MAX_MONITOR_PORT_COUNT+MAX_MIRROR_PORT_COUNT)] = CRITICAL;
						pthread_mutex_unlock(&stPthrdDir[dMyID].PthrdMutex);
					}
				}
			}
			dLine++;
		}
		else
		{
			if(dLine < MAX_MONITOR_PORT_COUNT)
			{
				status = director.stDIRECT[dMyID].cMonitorPort[dLine];
				log_print(LOGN_INFO,"%s.MONITOR(TYPE2)(%d):%c[%s]",__FUNCTION__,dLine,cTmp,status&MASK?"MASKED":"UNMASK");
				if( (status != NOT_EQUIP) && (status & MASK))
				{
					if(status != CRITICAL)
						Send_CondDirSWMess(dMyID, INVTYPE_PORT_MONITOR, dLine, CRITICAL, status);

					pthread_mutex_lock(&stPthrdDir[dMyID].PthrdMutex);
					director.stDIRECT[dMyID].cMonitorPort[dLine] = CRITICAL;
					pthread_mutex_unlock(&stPthrdDir[dMyID].PthrdMutex);
				}
			}
			else if( (dLine >= MAX_MONITOR_PORT_COUNT) && (dLine < (MAX_MONITOR_PORT_COUNT+MAX_MIRROR_PORT_COUNT)))
			{
				status = director.stDIRECT[dMyID].cMirrorPort[dLine-MAX_MONITOR_PORT_COUNT];
				log_print(LOGN_INFO,"%s.MIRROR(TYPE2)(%d):%c[%s]",__FUNCTION__,dLine,cTmp,status&MASK?"MASKED":"UNMASK");
				if( (status != NOT_EQUIP) && (status & MASK))
				{
					if(status!= CRITICAL)
						Send_CondDirSWMess(dMyID, INVTYPE_PORT_MIRROR, (dLine-MAX_MONITOR_PORT_COUNT), CRITICAL, status);

					pthread_mutex_lock(&stPthrdDir[dMyID].PthrdMutex);
					director.stDIRECT[dMyID].cMirrorPort[dLine-MAX_MONITOR_PORT_COUNT] = CRITICAL;
					pthread_mutex_unlock(&stPthrdDir[dMyID].PthrdMutex);
				}
			}
			else if( (dLine >= (MAX_MONITOR_PORT_COUNT+MAX_MIRROR_PORT_COUNT)) && (dLine < (MAX_MONITOR_PORT_COUNT+MAX_MIRROR_PORT_COUNT+MAX_DIRECT_POWER_COUNT)))
			{
				status = director.stDIRECT[dMyID].cPower[dLine-(MAX_MONITOR_PORT_COUNT+MAX_MIRROR_PORT_COUNT)];
				log_print(LOGN_INFO,"%s.POWER(TYPE2)(%d):%c[%s]",__FUNCTION__,dLine,cTmp,status&MASK?"MASKED":"UNMASK");
				if( (status != NOT_EQUIP) && (status & MASK))
				{
					if(status != CRITICAL)
						Send_CondDirSWMess(dMyID, INVTYPE_POWER_DIRECTOR, (dLine-(MAX_MONITOR_PORT_COUNT+MAX_MIRROR_PORT_COUNT)), CRITICAL, status);

					pthread_mutex_lock(&stPthrdDir[dMyID].PthrdMutex);
					director.stDIRECT[dMyID].cPower[dLine-(MAX_MONITOR_PORT_COUNT+MAX_MIRROR_PORT_COUNT)] = CRITICAL;
					pthread_mutex_unlock(&stPthrdDir[dMyID].PthrdMutex);
				}
			}
			dLine++;
		}
		pthread_mutex_lock(&stPthrdDir[dMyID].PthrdMutex);
		director.stDIRECT[dMyID].tEachUpTime = tUpdate;
		pthread_mutex_unlock(&stPthrdDir[dMyID].PthrdMutex);
	}

	if(!dLine)
	{
		for(dLine = 0; dLine < (MAX_MONITOR_PORT_COUNT+MAX_MIRROR_PORT_COUNT); dLine++)
		{
			if(dLine < MAX_MONITOR_PORT_COUNT)
			{
				status = director.stDIRECT[dMyID].cMonitorPort[dLine];
				if( (status != NOT_EQUIP) && (status & MASK))
				{
					if(status != CRITICAL)
						Send_CondDirSWMess(dMyID, INVTYPE_PORT_MONITOR, dLine, CRITICAL, status);

					pthread_mutex_lock(&stPthrdDir[dMyID].PthrdMutex);
					director.stDIRECT[dMyID].cMonitorPort[dLine] = CRITICAL;
					pthread_mutex_unlock(&stPthrdDir[dMyID].PthrdMutex);
				}
			}
			else if( (dLine >= MAX_MONITOR_PORT_COUNT) && (dLine < (MAX_MONITOR_PORT_COUNT+MAX_MIRROR_PORT_COUNT)))
			{
				status = director.stDIRECT[dMyID].cMirrorPort[dLine-MAX_MONITOR_PORT_COUNT];
				if( (status != NOT_EQUIP) && (status & MASK))
				{
					if(status != CRITICAL)
						Send_CondDirSWMess(dMyID, INVTYPE_PORT_MIRROR, (dLine-MAX_MONITOR_PORT_COUNT), CRITICAL, status);

					pthread_mutex_lock(&stPthrdDir[dMyID].PthrdMutex);
					director.stDIRECT[dMyID].cMirrorPort[dLine-MAX_MONITOR_PORT_COUNT] = CRITICAL;
					pthread_mutex_unlock(&stPthrdDir[dMyID].PthrdMutex);
				}
			}
			else if( (dLine >= (MAX_MONITOR_PORT_COUNT+MAX_MIRROR_PORT_COUNT)) && (dLine < (MAX_MONITOR_PORT_COUNT+MAX_MIRROR_PORT_COUNT+MAX_DIRECT_POWER_COUNT)))
			{
				status = director.stDIRECT[dMyID].cPower[dLine-(MAX_MONITOR_PORT_COUNT+MAX_MIRROR_PORT_COUNT)];
				if( (status != NOT_EQUIP) && (status & MASK))
				{
					if(status != CRITICAL)
						Send_CondDirSWMess(dMyID, INVTYPE_POWER_DIRECTOR, (dLine-(MAX_MONITOR_PORT_COUNT+MAX_MIRROR_PORT_COUNT)), CRITICAL, status);

					pthread_mutex_lock(&stPthrdDir[dMyID].PthrdMutex);
					director.stDIRECT[dMyID].cPower[dLine-(MAX_MONITOR_PORT_COUNT+MAX_MIRROR_PORT_COUNT)] = CRITICAL;
					pthread_mutex_unlock(&stPthrdDir[dMyID].PthrdMutex);
				}
			}
		}
	}
	pclose(stPthrdDir[dMyID].fPipe);
}
#endif

void CheckSwitch(int dMyID)
{
	int				dLine, dPercentMEM;
	char			sCmd[BUFSIZ], sResult[BUFSIZ], status;
	unsigned long	luTotalMEM, luUsedMEM, load;
	time_t			tUpdate;

	tUpdate = time(NULL);
	sprintf(sCmd, "%s/SCRIPT/SNMP/snmp-switch.sh %d", START_PATH, dMyID);
	if( (stPthrdSW[dMyID].fPipe = popen(sCmd, "r")) == NULL)
		return;

	dLine	= 0;
	while(fgets(sResult, BUFSIZ, stPthrdSW[dMyID].fPipe) != NULL)
	{
		if(dLine < MAX_SWITCH_PORT_COUNT)
		{
			status = swch.stSwitch[dMyID].cSwitchPort[dLine];
			log_print(LOGN_INFO,"%s.PORT[%d:%s]=%s",__FUNCTION__,dLine,status&MASK?"MASKED":"UNMASK", sResult);
			if( (status != NOT_EQUIP) && (status & MASK))
			{
				if(strncasecmp(sResult, "UP", 2) == 0)
				{
					if(status != NORMAL)
						Send_CondDirSWMess(dMyID, INVTYPE_PORT_SWITCH, dLine, NORMAL, status);
					pthread_mutex_lock(&stPthrdSW[dMyID].PthrdMutex);
					swch.stSwitch[dMyID].cSwitchPort[dLine] = NORMAL;
					pthread_mutex_unlock(&stPthrdSW[dMyID].PthrdMutex);
				}
				else
				{
					if(status != CRITICAL)
						Send_CondDirSWMess(dMyID, INVTYPE_PORT_SWITCH, dLine, CRITICAL, status);

					pthread_mutex_lock(&stPthrdSW[dMyID].PthrdMutex);
					swch.stSwitch[dMyID].cSwitchPort[dLine] = CRITICAL;
					pthread_mutex_unlock(&stPthrdSW[dMyID].PthrdMutex);
				}
			}
		}
		else if(dLine < (MAX_SWITCH_PORT_COUNT+MAX_SWITCH_CPU_COUNT))
		{
			log_print(LOGN_INFO,"%s.CPU[%d]=%s",__FUNCTION__,dLine-MAX_SWITCH_PORT_COUNT,sResult);
			swch.stSwitch[dMyID].uSwitchCPU[dLine-MAX_SWITCH_PORT_COUNT] = (unsigned int)atoi(sResult);

			load   = swch.stSwitch[dMyID].uSwitchCPU[dLine-MAX_SWITCH_PORT_COUNT];
			status = swch.stSwitch[dMyID].cSwitchCPUStatus[dLine-MAX_SWITCH_PORT_COUNT];

			if(load < keepalive->stSWCHLoad.cpu.usMinor)
			{
				if(status != NORMAL)
					Send_CondDirSWMess(dMyID, INVTYPE_CPU_SWITCH, (dLine-MAX_SWITCH_PORT_COUNT), NORMAL, status);

				swch.stSwitch[dMyID].cSwitchCPUStatus[dLine-MAX_SWITCH_PORT_COUNT] = NORMAL;
			}
			else if(load < keepalive->stSWCHLoad.cpu.usMajor)
			{
				if(status != MINOR)
					Send_CondDirSWMess(dMyID, INVTYPE_CPU_SWITCH, (dLine-MAX_SWITCH_PORT_COUNT), MINOR, status);

				swch.stSwitch[dMyID].cSwitchCPUStatus[dLine-MAX_SWITCH_PORT_COUNT] = MINOR;
			}
			else if(load < keepalive->stSWCHLoad.cpu.usCritical)
			{
				if(status != MAJOR)
					Send_CondDirSWMess(dMyID, INVTYPE_CPU_SWITCH, (dLine-MAX_SWITCH_PORT_COUNT), MAJOR, status);

				swch.stSwitch[dMyID].cSwitchCPUStatus[dLine-MAX_SWITCH_PORT_COUNT] = MAJOR;
			}
			else
			{
				if(status != CRITICAL)
					Send_CondDirSWMess(dMyID, INVTYPE_CPU_SWITCH, (dLine-MAX_SWITCH_PORT_COUNT), CRITICAL, status);

				swch.stSwitch[dMyID].cSwitchCPUStatus[dLine-MAX_SWITCH_PORT_COUNT] = CRITICAL;
			}
		}
		else if(dLine < (MAX_SWITCH_PORT_COUNT+MAX_SWITCH_CPU_COUNT+MAX_SWITCH_MEM_COUNT))
		{
			swch.stSwitch[dMyID].uSwitchMEM[dLine-(MAX_SWITCH_PORT_COUNT+MAX_SWITCH_CPU_COUNT)] = (unsigned int)atoi(sResult);
			if( (dLine+1) == (MAX_SWITCH_PORT_COUNT+MAX_SWITCH_CPU_COUNT+MAX_SWITCH_MEM_COUNT))
			{
				luUsedMEM	= swch.stSwitch[dMyID].uSwitchMEM[0];
				luTotalMEM	= swch.stSwitch[dMyID].uSwitchMEM[0]+swch.stSwitch[dMyID].uSwitchMEM[1];

				dPercentMEM = ( (float)luUsedMEM/(float)luTotalMEM)*100;
				log_print(LOGN_INFO,"%s.MEM=%d(%ld/%ld)",__FUNCTION__,dPercentMEM, luUsedMEM, luTotalMEM);

				status= swch.stSwitch[dMyID].cSwitchMEMStatus;

				if(dPercentMEM < keepalive->stSWCHLoad.mem.usMinor)
				{
					if(status!= NORMAL)
						Send_CondDirSWMess(dMyID, INVTYPE_MEMORY_SWITCH, 0, NORMAL, status);

					swch.stSwitch[dMyID].cSwitchMEMStatus = NORMAL;
				}
				else if(dPercentMEM < keepalive->stSWCHLoad.mem.usMajor)
				{
					if(status != MINOR);
						Send_CondDirSWMess(dMyID, INVTYPE_MEMORY_SWITCH, 0, MINOR, status);

					swch.stSwitch[dMyID].cSwitchMEMStatus = MINOR;
				}
				else if(dPercentMEM < keepalive->stSWCHLoad.mem.usCritical)
				{
					if(status != MAJOR);
						Send_CondDirSWMess(dMyID, INVTYPE_MEMORY_SWITCH, 0, MAJOR, status);

					swch.stSwitch[dMyID].cSwitchMEMStatus = MAJOR;
				}
				else
				{
					if(status != CRITICAL);
						Send_CondDirSWMess(dMyID, INVTYPE_MEMORY_SWITCH, 0, CRITICAL, status);

					swch.stSwitch[dMyID].cSwitchMEMStatus = CRITICAL;
				}
			}
		}
		else
			log_print(LOGN_CRI, "F=%s:%s.%d: snmp-switch.sh dMyID[%d] dLine[%d] ERROR", __FILE__, __FUNCTION__, __LINE__, dMyID, dLine);

		dLine++;
	}

	if(!dLine)
	{
		for(dLine = 0; dLine < (MAX_SWITCH_PORT_COUNT+MAX_SWITCH_CPU_COUNT+MAX_SWITCH_MEM_COUNT); dLine++)
		{
			if(dLine < MAX_SWITCH_PORT_COUNT)
			{
				status = swch.stSwitch[dMyID].cSwitchPort[dLine];
				if( (status != NOT_EQUIP) && (status & MASK))
				{
					if(status != CRITICAL)
						Send_CondDirSWMess(dMyID, INVTYPE_PORT_SWITCH, dLine, CRITICAL, status);

					pthread_mutex_lock(&stPthrdSW[dMyID].PthrdMutex);
					swch.stSwitch[dMyID].cSwitchPort[dLine] = CRITICAL;
					pthread_mutex_unlock(&stPthrdSW[dMyID].PthrdMutex);
				}
			}
			else if(dLine < (MAX_SWITCH_PORT_COUNT+MAX_SWITCH_CPU_COUNT))
			{
				pthread_mutex_lock(&stPthrdSW[dMyID].PthrdMutex);
				swch.stSwitch[dMyID].uSwitchCPU[dLine-MAX_SWITCH_PORT_COUNT] = 0;
				pthread_mutex_unlock(&stPthrdSW[dMyID].PthrdMutex);
			}
			else if(dLine < (MAX_SWITCH_PORT_COUNT+MAX_SWITCH_CPU_COUNT+MAX_SWITCH_MEM_COUNT))
			{
				pthread_mutex_lock(&stPthrdSW[dMyID].PthrdMutex);
				swch.stSwitch[dMyID].uSwitchMEM[dLine-(MAX_SWITCH_PORT_COUNT+MAX_SWITCH_CPU_COUNT)] = 0;
				pthread_mutex_unlock(&stPthrdSW[dMyID].PthrdMutex);
			}
			else
				log_print(LOGN_CRI, "F=%s:%s.%d: snmp-switch.sh dMyID[%d] dLine[%d] ERROR", __FILE__, __FUNCTION__, __LINE__, dMyID, dLine);
		}
	}
	pclose(stPthrdSW[dMyID].fPipe);

	pthread_mutex_lock(&stPthrdSW[dMyID].PthrdMutex);
	swch.stSwitch[dMyID].tEachUpTime = tUpdate;
	pthread_mutex_unlock(&stPthrdSW[dMyID].PthrdMutex);
}

