/**A.1*  File Inclusion *******************************************************/

/* SYS HEADER */
#include <stdio.h>
#include <string.h>		/* STRSTR, STRCPY, STRNCPY, STRLEN */
#include <errno.h>
#include <sys/stat.h>	/* STAT(2) */
/* LIB HEADER */
#include "filedb.h"
#include "loglib.h"
/* PRO HEADER */
#include "path.h"
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
#include "almstat.h"
/* LOC HEADER */
#include "chsmd_msg.h"		/* Send_AlmMsg(), SetFIDBValue() */
#include "chsmd_hw.h"

/**B.1*  Definition of New Constants ******************************************/
/**B.2*  Definition of New Type  **********************************************/
/**C.1*  Declaration of Variables  ********************************************/
char    szMessageBody[409600];
int     hwpwrcnt;
int     hwfancnt;
int     hwdiskcnt;
int     hwportcnt;
int     dDagType;

char    chFlag[30];
char    szRetMsg[30][LSIZE];
char    szDagBinDir[30];
char    szHPLogDir[30];

char    szEnclosure[2][32];

/**C.1*  Declaration of Variables  ********************************************/
extern int     	gdMsgOldSize;
extern pst_NTAF fidb;

/**D.1*  Definition of Functions  *********************************************/
/**D.2*  Definition of Functions  *********************************************/

int dCheckHW(void)
{
	int		i, devCnt;

	log_print(LOGN_INFO, " # HARDWARE CHECK ############################");

	/*** Init Variable **********************************************/
	hwpwrcnt	= fidb->hwpwrcnt;
	hwfancnt	= fidb->hwfancnt;
	hwportcnt	= fidb->hwportcnt;

	if(hwpwrcnt == 0)
		log_print(LOGN_WARN,">POWER CHECK COUNT IS ZERO[%d]",hwpwrcnt);

	if(hwfancnt == 0)
		log_print(LOGN_WARN,">FAN CHECK COUNT IS ZERO[%d]",hwfancnt);

	if(hwportcnt == 0)
		log_print(LOGN_WARN,">DAG PORT CHECK COUNT IS ZERO[%d]", hwportcnt);

	if( (hwpwrcnt==0) && (hwfancnt==0) && (hwportcnt==0))
		return -1;

	/*** 1. POWER CHECK ***********************************************/
	devCnt = dGetHWSts(chFlag, hwpwrcnt, INVTYPE_POWER);

	for( i = 0; i< hwpwrcnt && i< devCnt; i++ ){
		if( chFlag[i] < 0 ){
			/* CRITICAL STATUS **************************************/
			if( fidb->hwpwr[i] != CRITICAL
				&& fidb->hwpwr[i] < MASK ){
				Send_AlmMsg( LOCTYPE_PHSC, INVTYPE_POWER, i, CRITICAL, fidb->hwpwr[i] );
				fidb->hwpwr[i] = CRITICAL;
				log_print(LOGN_CRI,"ALARM OCCURED POWER[%d] STATUS : CRITICAL",i);
			}
		}
		else{
			/* NORMAL STATUS *************************************/
			if( fidb->hwpwr[i] != NORMAL
				&& fidb->hwpwr[i] < MASK )
			{
				Send_AlmMsg( LOCTYPE_PHSC, INVTYPE_POWER, i, NORMAL, fidb->hwpwr[i] );
				fidb->hwpwr[i] = NORMAL;
				log_print(LOGN_DEBUG,"ALARM CLEARED POWER[%d] STATUS : NORMAL", i );
			}
		}
		log_print(LOGN_INFO," # POWER %02d STATUS[0x%x] ", i, fidb->hwpwr[i] );
	}
	/* remain device status, set NOT_EQUIP added by uamyd 20110411 */
	if( hwpwrcnt > devCnt ){
		for( i = devCnt; i < hwpwrcnt; i++ ){
			if( fidb->hwpwr[i] < MASK ) fidb->hwpwr[i] = NOT_EQUIP;
		}
	}

	/*** 2. FAN CHECK *************************************************/
	devCnt = dGetHWSts( chFlag, hwfancnt, INVTYPE_FAN );

	for( i = 0; i< hwfancnt && i< devCnt; i++ ){
		if( chFlag[i] < 0 ){
			/* CRITICAL STATUS **************************************/
			if( fidb->hwfan[i] != CRITICAL
				&& fidb->hwfan[i] != NOT_EQUIP
				&& fidb->hwfan[i] < MASK ){
				Send_AlmMsg( LOCTYPE_PHSC, INVTYPE_FAN, i, CRITICAL, fidb->hwfan[i] );
				fidb->hwfan[i] = CRITICAL;
				log_print(LOGN_CRI,"ALARM OCCURED FAN[%d] STATUS : CRITICAL",i);
			}
		}
		else{
			/* NORMAL STATUS *************************************/
			if( fidb->hwfan[i] != NORMAL
				&& fidb->hwfan[i] < MASK )
			{
				Send_AlmMsg( LOCTYPE_PHSC, INVTYPE_FAN, i, NORMAL, fidb->hwfan[i] );
				fidb->hwfan[i] = NORMAL;
				log_print(LOGN_DEBUG,"ALARM CLEARED FAN[%d] STATUS : NORMAL", i );
			}
		}
		log_print(LOGN_INFO," # FAN %02d STATUS[0x%x] ", i, fidb->hwfan[0] );
	}
	/* remain device status, set NOT_EQUIP added by uamyd 20110411 */
	if( hwfancnt > devCnt ){
		for( i = devCnt; i < hwfancnt; i++ ){
			if( fidb->hwfan[i] < MASK ) fidb->hwfan[i] = NOT_EQUIP;
		}
	}

	/*** 3. DAG CHECK : ENDACE CARD LINK STATUS ******************************************/
	dGetDagSts(chFlag, hwportcnt, INVTYPE_PORT);

	for( i=0; i< hwportcnt; i++ ){
		if( chFlag[i] < 0 ){
			/*** CRITICAL **********************************/
			if( fidb->hwport[i] != CRITICAL
				&& fidb->hwport[i] != NOT_EQUIP
				&& fidb->hwport[i] < MASK ){
				Send_AlmMsg( LOCTYPE_PHSC, INVTYPE_PORT, i, CRITICAL, fidb->hwport[i] );
				fidb->hwport[i] = CRITICAL;
				log_print(LOGN_CRI, "ALARM OCCURED DAG LINK[%d] PORT[%d] DOWN", i/2, i%2);
			}
		}
		else{
			/*** NORMAL   **********************************/
			if( fidb->hwport[i] != NORMAL
				&& fidb->hwport[i] < MASK ){
				Send_AlmMsg( LOCTYPE_PHSC, INVTYPE_PORT, i, NORMAL, fidb->hwport[i] );
				fidb->hwport[i] = NORMAL;
				log_print(LOGN_DEBUG, "ALARM CLEARED DAG LINK[%d] PORT[%d] UP", i/2, i%2);
			}
		}
	}

	return 0;
}

/************************************************************************************
 DISK, POWER의 상태를 MESSAGE FILE을 통해서 감시하는 함수 :: RESERVED 됨 by uamyd0626
************************************************************************************/
int dCheckHW2()
{
	int		i, dRet, dDriveFlag, dPowerFlag, dDriveNum, dPowerNum;
	char	szPow[2], szEncl[32];
	char	*szTmpBuf, *szTmpBuf1;

	dDriveNum	= 0;			/*** UP:[0], DOWN:[1] ***/
	dPowerNum	= 0;			/*** A:[0],  B:[1] ***/
	dDriveFlag	= 0;			/*** RECOVERING:[1], REBUILDING:[2] ***/
	dPowerFlag	= 0;			/*** CAUTION:[1], REPAIRED:[2] ***/

	dRet = dCheckMsgStatus();
	if( dRet < 0 )
	{
		/*** MESSAGE FILE을 OPEN하지 못하는 경우 ******************************/

		return -1;
	}
	else if( dRet == 1 )
	{
		/*** MESSAGE FILE에 변화가 감지된 경우 ********************************/
		log_print(LOGN_DEBUG,"%s", szMessageBody );

		/*** CHECK DISK *******************************************************/
		if( (szTmpBuf = strstr( szMessageBody, "Recovering" )) != NULL )
		{
			szTmpBuf = strstr( szMessageBody, "drive:" );
           	sscanf(szTmpBuf, "%*s %d.", &dDriveNum );
           	dDriveFlag = 1;

			if( dDriveNum == 16 || dDriveNum == 17 )
				dDriveNum = dDriveNum - 16;

			log_print(LOGN_DEBUG,"RECOVERING : DRIVENUM:[%d]", dDriveNum );
		}

		if( (szTmpBuf = strstr( szMessageBody, "Rebuilding" )) != NULL )
		{
			szTmpBuf = strstr( szMessageBody, "drive:" );
            sscanf(szTmpBuf, "%*s %d.", &dDriveNum );
            dDriveFlag = 2;

			if( dDriveNum == 16 || dDriveNum == 17 )
				dDriveNum = dDriveNum - 16;

			log_print(LOGN_DEBUG,"REBUILDING : DRIVENUM:[%d]", dDriveNum );
		}

		/*** CHECK POWER ******************************************************/
		if( (szTmpBuf = strstr( szMessageBody, "Caution" )) != NULL )
        {
			log_print(LOGN_DEBUG,"[%s]", szMessageBody );

			if( (szTmpBuf1 = strstr( szTmpBuf, "Repaired" )) != NULL )
			{
				for( i=0; i<2; i++)
				{
					if( (szTmpBuf = strstr( szTmpBuf1, szEnclosure[i] )) != NULL )
					{
						sscanf(szTmpBuf, "%s %*s %*s %s.", szEncl, szPow );
                    	szPow[1] = 0x00;

                    	if( !strcmp(szPow, "A") )
                        	dPowerNum = 0;
                    	else if( !strcmp(szPow, "B") )
                        	dPowerNum = 1;

                    	dPowerFlag = 2;

					break;
					}
				}

				if( i == 2 )
                {
                    log_print(LOGN_DEBUG,"[POWER CHECK] CANNOT FIND MATCHED ENCLOSURE SERIAL NUMBER");
                }
			}
			else
			{
				for( i=0; i<2; i++)
				{
					if( (szTmpBuf1 = strstr( szTmpBuf, szEnclosure[i] )) != NULL )
					{
						sscanf(szTmpBuf1, "%s %*s %*s %s.", szEncl, szPow );
                		szPow[1] = 0x00;

                		if( !strcmp(szPow, "A") )
                    		dPowerNum = 0;
                		else if( !strcmp(szPow, "B") )
                    		dPowerNum = 1;

						dPowerFlag = 1;

						break;
					}
				}

				if( i == 2 )
				{
					log_print(LOGN_DEBUG,"[POWER CHECK] CANNOT FIND MATCHED ENCLOSURE SERIAL NUMBER");
				}
			}
        }
        else if( (szTmpBuf = strstr( szMessageBody, "Repaired" )) != NULL )
        {
			log_print(LOGN_DEBUG,"[%s]", szMessageBody );
			for( i=0; i<2; i++)
            {
                if( (szTmpBuf1 = strstr( szTmpBuf, szEnclosure[i] )) != NULL )
                {
                    sscanf(szTmpBuf1, "%s %*s %*s %s.", szEncl, szPow );
                    szPow[1] = 0x00;

                    if( !strcmp(szPow, "A") )
                        dPowerNum = 0;
                    else if( !strcmp(szPow, "B") )
                        dPowerNum = 1;

                    dPowerFlag = 2;

                    break;
                }
            }

            if( i == 2 )
            {
                log_print(LOGN_DEBUG,"[POWER CHECK] CANNOT FIND MATCHED ENCLOSURE SERIAL NUMBER");
            }
        }
	}
	else if( dRet == 2 )
	{
		/*** MESSAGE FILE에 변화가 없는 경우 **********************************/
	}

	/*** DRIVE가 변경된 경우 **************************************************/
	if( dDriveFlag != 0 )
	{
		fidb->hwdiskcnt = 2;
		if( dDriveFlag == 1 )
		{
			if( fidb->hwdisk[dDriveNum] != CRITICAL && fidb->hwdisk[dDriveNum] < MASK )
				Send_AlmMsg( LOCTYPE_PHSC, INVTYPE_DISKARRAY, dDriveNum, CRITICAL, fidb->hwdisk[dDriveNum] );

			SetFIDBValue( &fidb->hwdisk[dDriveNum], CRITICAL );
		}
		else if( dDriveFlag == 2 )
		{
			if( fidb->hwdisk[dDriveNum] != NORMAL && fidb->hwdisk[dDriveNum] < MASK )
                Send_AlmMsg( LOCTYPE_PHSC, INVTYPE_DISKARRAY, dDriveNum, NORMAL, fidb->hwdisk[dDriveNum] );

			SetFIDBValue( &fidb->hwdisk[dDriveNum], NORMAL );
		}
	}

	/*** POWER가 변경된 경우 **************************************************/
	if( dPowerFlag != 0 )
	{
		fidb->hwpwrcnt = 2;

		if( dPowerFlag == 1 )
		{
			if( fidb->hwpwr[dPowerNum] != CRITICAL && fidb->hwpwr[dPowerNum] < MASK )
                Send_AlmMsg( LOCTYPE_PHSC, INVTYPE_POWER, dPowerNum, CRITICAL, fidb->hwpwr[dPowerNum] );

			SetFIDBValue( &fidb->hwpwr[dPowerNum], CRITICAL );
		}
        else if( dPowerFlag == 2 )
		{
			if( fidb->hwpwr[dPowerNum] != NORMAL && fidb->hwpwr[dPowerNum] < MASK )
                Send_AlmMsg( LOCTYPE_PHSC, INVTYPE_POWER, dPowerNum, NORMAL, fidb->hwpwr[dPowerNum] );

			SetFIDBValue( &fidb->hwpwr[dPowerNum], NORMAL );
		}

		log_print(LOGN_DEBUG,"POWER CNT:[%d] NUM:[%d] POW:[%s] STATUS:[0x%02x]",
					fidb->hwpwrcnt, dPowerNum, szPow, fidb->hwpwr[dPowerNum] );
	}

/*** reserved by uamyd0626 06.06.16 ******************
	dRet = dCheckFanStatus();
	if( dRet < 0 )
	{
		log_print(LOGN_DEBUG,"[ERROR] FAN FILE OPEN");
	}
*****************************************************/

	return 1;
}


/*******************************************************************************
 MESSAGE FILE에 변화가 있는지 확인하는 함수
*******************************************************************************/
int dCheckMsgStatus()
{
	FILE    *ptr;
	int		dLen;

	char	szBuffer[256];

	struct stat 	stStat;

	stat( FILE_MESSAGE, &stStat );

	if( stStat.st_size < gdMsgOldSize )
	{
		/*** MESSAGE FILE이 새로 업데이트 된 경우 *****************************/

		ptr = fopen( FILE_MESSAGE, "r" );
        if( ptr == NULL )
        {
            log_print(LOGN_CRI,"[HW CHECK] MESSAGE FILE OPEN ERROR [%d] [%s]",
                      errno, strerror(errno) );
            return -1;
        }

		dLen = 0;

        while( fgets(szBuffer, 256, ptr) != NULL )
        {
            strcpy(&szMessageBody[dLen], &szBuffer[0] );
            dLen += strlen(szBuffer);
        }

        fclose(ptr);

		gdMsgOldSize = stStat.st_size;

		return 1;
	}
	else if( stStat.st_size != gdMsgOldSize )
	{
		/*** MESSAGE FILE에 추가된 경우 ***************************************/

		ptr = fopen( FILE_MESSAGE, "r" );
		if( ptr == NULL )
		{
			log_print(LOGN_CRI,"[HW CHECK] MESSAGE FILE OPEN ERROR [%d] [%s]",
					  errno, strerror(errno) );
			return -1;
		}

		fseek( ptr, gdMsgOldSize, SEEK_SET );

		dLen = 0;

		while( fgets(szBuffer, 256, ptr) != NULL )
		{
			strcpy(&szMessageBody[dLen], &szBuffer[0] );
			dLen += strlen(szBuffer);
		}

		fclose(ptr);

		gdMsgOldSize = stStat.st_size;

		return 1;
	}

	return 2;
}

/*******************************************************************************
szCommand를 수행하고 난 결과를 szRetMsg에 저장.
*******************************************************************************/
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
int dCntMsg( char *szCommand )
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
#ifdef _NO_HPLOG
		if( (strstr(szCommand, "hplog_pw") != NULL) || (strstr(szCommand, "hplog_fan") != NULL))
		{
			if(i > 1)
				break;

			if(strlen(szBuf) < LSIZE)
				strncpy(szRetMsg[i], szBuf, LSIZE);
			else
			{
				strncpy(szRetMsg[i], szBuf, LSIZE-1);
				szRetMsg[i][LSIZE-1] = 0x00;
			}
		}
		else
		{
			if(i < 1)
			{
				i++;
				continue;
			}

			sprintf(&szRetMsg[i-1][0], "%s", szBuf);
		}
		log_print(LOGN_INFO, ">[%d] %s ", i, szRetMsg[i]);
#else
		if(i < 1)
		{
			i++;
			continue;
		}

		sprintf(&szRetMsg[i-1][0], "%s", szBuf);
		log_print(LOGN_INFO, ">[%d] %s ", i-1, szRetMsg[i-1]);
#endif
		i++;
	}
	pclose(fp);

#ifdef _NO_HPLOG
	if(strstr(szCommand, "hplog_pw") != NULL)
	{
		dCnt = 2;
		for(i = 1; i < dCnt; i++)
			strncpy(szRetMsg[i], szRetMsg[0], LSIZE);
	}
	else if(strstr(szCommand, "hplog_fan") != NULL)
	{
		dCnt = 6;
		for(i = 1; i < dCnt; i++)
			strncpy(szRetMsg[i], szRetMsg[0], LSIZE);
	}
	else
		dCnt = i;
#else
	dCnt = i-2;
#endif

	return dCnt;
}

/*******************************************************************************
실제 하드웨어의 상태를 체크. flag값에 상태값이 저장됨. type으로 POWER, FAN 구별.
*******************************************************************************/
int dGetHWSts( char *flag, int cnt, int type )
{
	int	 i, failcnt, SuccIdx, devCnt;
	char	*target, szCommand[1024], szType[32];

	SuccIdx = type;
	switch(type)
	{
#ifdef _NO_HPLOG
		case INVTYPE_POWER:
			sprintf(szCommand, "file %s/hplog_pw", szHPLogDir);
			sprintf(szType, "%s", "POWER");
			break;
		case INVTYPE_FAN:
			sprintf(szCommand, "file %s/hplog_fan", szHPLogDir);
			sprintf(szType, "%s", "FAN");
			break;
#else
		case INVTYPE_POWER:
			sprintf(szCommand, "%s/hplog -p", szHPLogDir);
			sprintf(szType, "%s","POWER");
			break;
		case INVTYPE_FAN:
			sprintf(szCommand, "%s/hplog -f", szHPLogDir);
			sprintf(szType, "%s", "FAN");
			break;
#endif
		default :
			log_print(LOGN_WARN, "UNVALID HW TYPE[%d]", type);
			return -1;
	}

	devCnt = dCntMsg(szCommand);

	/*
	cnt - MAX DEV CNT
	devCnt - real check dev cnt
	noted by uamyd 20110411
	*/
	for(failcnt = 0, i = 0; i < cnt && i < devCnt ; i++)
	{
		flag[i]	= -1;
#ifdef _NO_HPLOG
		target = strstr(&szRetMsg[i][0], "cannot open");
#else
		target = strstr(&szRetMsg[i][0], "Normal");
#endif
		if(target == NULL)
			failcnt++;			/*	FAIL COUNT	*/
		else
			flag[i] = type;		/*	NORMAL		*/

		log_print(LOGN_INFO, "%s %02d STATUS %s", szType, i+1, (flag[i]<0)?"DEAD[CRITICAL]":"ALIVE[NORMAL]");
	}

	return devCnt;
}

/*******************************************************************************
ENDACE CARD 상태 체크.
*******************************************************************************/
int dGetDagSts(char *flag, int cnt, int type)
{
	char	szCommand[256];
	int		value1[16], value2[16];
	int		i, scan_cnt, failcnt;

	i			= 0;
	failcnt		= 0;

	if(dDagType < 2)
		snprintf(szCommand, sizeof(szCommand), "%s/dagthree -d /dev/dag%d -s", szDagBinDir, i);
	else if(dDagType == 2)
		snprintf(szCommand, sizeof(szCommand), "%s/dagfour -d /dev/dag%d -s", szDagBinDir, i);
	else
		snprintf(szCommand, sizeof(szCommand), "%s/dagconfig -d /dev/dag%d -s", szDagBinDir, i);

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
			*	Port A
			*/
			if( value1[1] != PORT_STATUS_UP ){
				failcnt++;
				flag[0] = -1;
			}
			else
				flag[0] = type;

			/*
			*	Port B
			*/
			if( value2[1] != PORT_STATUS_UP ){
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
			if( value1[1] != PORT_STATUS_UP ){
				failcnt++;
				flag[0] = -1;
			}
			else
				flag[0] = type;

			/*
			* Port B
			*/
			if( value2[1] != PORT_STATUS_UP ){
				failcnt++;
				flag[1] = -1;
			}
			else
				flag[1] = type;


			break;

		case TYPE_DAG4_5GE:
			scan_cnt = sscanf(szRetMsg[0], "%*s %d %d %d %d %d", &value1[0], &value1[1], &value1[2], &value1[3], &value1[4]);
			scan_cnt = sscanf(szRetMsg[1], "%*s %d %d %d %d %d", &value2[0], &value2[1], &value2[2], &value2[3], &value2[4]);

			/*	Port A	*/
			if( (value1[0] == PORT_STATUS_UP) && (value1[1] == PORT_STATUS_UP))
				flag[0] = type;
			else
			{
				failcnt++;
				flag[0] = -1;
			}

			/*	Port B	*/
			if( (value2[0] == PORT_STATUS_UP) && (value2[1] == PORT_STATUS_UP))
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
                        if( value1[4] == PORT_STATUS_UP ) flag[0] = type;
                        else{
                                failcnt++;
                                flag[0] = -1;
                        }

                        /* Port B */
                        if( value2[4] == PORT_STATUS_UP ) flag[1] = type;
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
DAG CARD (ENDACE CARD) TYPE CHECK by uamyd0626
*******************************************************************************/
int dCheckDagType(void)
{
	int		i, scan_cnt;
	FILE	*fp;
	char	szBuf[LSIZE], szField[32], szValue1[16], szValue2[16], szCommand[LSIZE];

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

/*******************************************************************************
아래는 더이상 사용하지 않음 삭제하지 않고 그냥 둠. by uamyd0626
*******************************************************************************/
int Init_Enclosure(void)
{
	FILE	*fp;

	int		i;
	int		dCnt = 0;
	char	szBuffer[32];

	fp = fopen( FILE_ENCLO_SERIAL, "r" );
	if( fp == NULL )
	{
		log_print(LOGN_CRI,"[ERROR] ENCLOSURE SERIAL FILE OPEN");
		return -1;
	}

	while( fgets(szBuffer, 32, fp) != NULL )
	{
		szBuffer[strlen(szBuffer)-1] = 0x00;

		strcpy(&szEnclosure[dCnt][0], &szBuffer[0] );
		dCnt++;

		if( dCnt == 2 )
			break;
	}

	fclose(fp);

	fidb->hwdiskcnt = 2;
    for(i=0;i<2;i++)
    {
		SetFIDBValue( &fidb->hwdisk[i], NORMAL );
    }

    fidb->hwpwrcnt = 2;
    for(i=0;i<2;i++)
    {
		SetFIDBValue( &fidb->hwpwr[i], NORMAL );
    }

	return 1;
}

int dCheckFanStatus()
{
	FILE		*ptr;

	int			dFanCnt = 0;
	char		szBuffer[256];
	char		*szTmp;

	ptr = fopen( FILE_FAN, "r");
	if( ptr == NULL )
	{
		log_print(LOGN_CRI,"[FAIL] FAN FILE OPEN [%d] [%s]", errno, strerror(errno) );
		return -1;
	}

	while( fgets(szBuffer, 256, ptr) != NULL )
	{
		dFanCnt++;

		if( dFanCnt == 1 )
			continue;

		szTmp = strstr( szBuffer, "Nominal" );
		if( szTmp == NULL )
		{
			if( fidb->hwfan[dFanCnt-2] != CRITICAL && fidb->hwfan[dFanCnt-2] != NOT_EQUIP && fidb->hwfan[dFanCnt-2] < MASK )
				Send_AlmMsg( LOCTYPE_PHSC, INVTYPE_FAN, dFanCnt-2, CRITICAL, fidb->hwfan[dFanCnt-2] );

			//fidb->hwfan[dFanCnt-2] = CRITICAL;
			SetFIDBValue( &fidb->hwfan[dFanCnt-2], CRITICAL );
		}
		else
		{
			if( fidb->hwfan[dFanCnt-2] > NORMAL && fidb->hwfan[dFanCnt-2] < MASK )
				Send_AlmMsg( LOCTYPE_PHSC, INVTYPE_FAN, dFanCnt-2, NORMAL, fidb->hwfan[dFanCnt-2] );

			//fidb->hwfan[dFanCnt-2] = NORMAL;
			SetFIDBValue( &fidb->hwfan[dFanCnt-2], NORMAL );
		}
	}

	if(dFanCnt > 3)
		fidb->hwfancnt = 2;
	else
		fidb->hwfancnt = dFanCnt-1;

	fclose(ptr);

	return 1;
}

char *CheckPowMsg( char *szMessage, int *dStatus )
{
	char	*szTmpBuffer;

	szTmpBuffer = strstr( szMessage, "Caution" );
	if( szTmpBuffer != NULL )
	{
		*dStatus = 1;
		return szTmpBuffer;
	}

	szTmpBuffer = strstr( szMessage, "Repaired" );
	if( szTmpBuffer != NULL )
    {
        *dStatus = 2;
        return szTmpBuffer;
    }

	return szTmpBuffer;
}
/*******************************************************************************
	Revision History :

	$Log: chsmd_hw.c,v $
	Revision 1.2  2011/09/05 05:05:49  dcham
	*** empty log message ***
	
	Revision 1.1  2011/08/29 09:57:07  dcham
	*** empty log message ***
	
	Revision 1.3  2011/08/20 08:25:24  hhbaek
	Split path part in commdef.h into path.h
	
	Revision 1.2  2011/07/28 01:47:27  uamyd
	CHSMD TAF modified
	
	Revision 1.1  2011/07/27 10:01:26  uamyd
	TAF CHSMD added
	
	Revision 1.27  2011/06/29 09:42:47  uamyd
	source refactoring, hw alarm report corrected
	
	Revision 1.26  2011/04/11 07:46:20  jsyoon
	*** empty log message ***
	
	Revision 1.25  2011/04/11 06:31:34  night1700
	dagcard 7.5g2 added, hwfan, hwpower cnt check added
	
	Revision 1.24  2011/01/11 04:09:12  uamyd
	modified
	
	Revision 1.1.1.1  2010/08/23 01:13:00  uamyd
	DQMS With TOTMON, 2nd-import
	
	Revision 1.23  2009/10/06 11:55:45  pkg
	hplog -p 명령어 absent 처리
	
	Revision 1.22  2009/09/24 00:54:49  pkg
	dagthree 명령어 처리시 오류 처리

	Revision 1.21  2009/09/24 00:52:43  pkg
	dagthree 명령어 처리시 오류 처리

	Revision 1.20  2009/08/16 09:08:12  pkg
	DAG 4.5G2 모델에 대한 dagconfig 명령 사용하도록 수정

	Revision 1.19  2009/08/13 07:47:12  dqms
	no message

	Revision 1.18  2009/08/12 16:02:36  dqms
	no message

	Revision 1.17  2009/08/05 12:20:16  hjpark
	no message

	Revision 1.16  2009/07/21 13:12:42  hjpark
	TAF의 FIDB에 Endace port Link 위치 변경(fidb->hwport:fidb->mirrorsts)
	원복

	Revision 1.15  2009/07/21 11:14:01  hjpark
	TAF의 FIDB에 Endace port Link 위치 변경(fidb->hwport:fidb->mirrorsts)

	Revision 1.14  2009/07/17 10:03:23  hjpark
	CHSMD _NO_HPLOG 적용하여 hplog 파일의 존재 여부 평가 테스트로 변경

	Revision 1.13  2009/07/17 08:56:04  hjpark
	no message

	Revision 1.12  2009/07/17 07:31:34  hjpark
	CHSMD _NO_HPLOG 적용하여 hplog 파일의 존재 여부 평가 테스트로 변경

	Revision 1.11  2009/07/02 10:45:17  hjpark
	no message

	Revision 1.10  2009/07/02 10:29:59  hjpark
	no message

	Revision 1.9  2009/07/02 10:25:21  hjpark
	no message

	Revision 1.8  2009/07/02 10:23:37  hjpark
	no message

	Revision 1.7  2009/07/02 09:45:38  hjpark
	no message

	Revision 1.6  2009/07/01 10:37:09  hjpark
	no message

	Revision 1.5  2009/07/01 10:20:55  hjpark
	no message

	Revision 1.4  2009/07/01 10:10:59  hjpark
	no message

	Revision 1.3  2009/06/10 21:25:17  jsyoon
	*** empty log message ***

	Revision 1.2  2009/06/05 05:30:16  jsyoon
	*** empty log message ***

	Revision 1.1.1.1  2009/05/26 02:14:43  dqms
	Init TAF_RPPI

	Revision 1.4  2009-05-21 07:36:53  astone
	no message

	Revision 1.3  2009-05-21 06:47:12  astone
	no message

	Revision 1.2  2009-05-21 06:44:58  hjpark
	no message

	Revision 1.1.1.1  2009-05-19 10:34:23  hjpark
	DQMS
		-Writer: Han-jin Park
		-Date: 2009.05.19

	Revision 1.6  2008/11/19 02:48:44  hjpark
	*** empty log message ***

	Revision 1.5  2008/03/24 10:30:35  uamyd
	*** empty log message ***

	Revision 1.4  2008/03/21 13:31:47  uamyd
	*** empty log message ***

	Revision 1.3  2008/01/15 17:16:18  pkg
	*** empty log message ***

	Revision 1.2  2008/01/03 11:13:46  dark264sh
	modify Makefile options (-g3 -Wall), modify warning code

	Revision 1.1.1.1  2007/12/27 09:04:40  leich
	WNTAS_ADD

	Revision 1.1.1.1  2007/10/31 05:12:12  uamyd
	WNTAS so lated initialized

	Revision 1.1  2002/01/30 17:20:31  swdev4
	Initial revision
*******************************************************************************/
