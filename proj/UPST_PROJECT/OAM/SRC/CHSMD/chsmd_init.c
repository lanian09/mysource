/**A.1 * File Include *******************************/

/* SYS HEADER */
#include <stdio.h>		/* FILE, fp, fopen, fgets, fclose, sscanf */
#include <string.h>		/* strerror */
#include <stdlib.h>		/* EXIT(3) */
#include <ctype.h>		/* isspace(3)	*/
#include <errno.h>		/* errno */
#include <signal.h>
#include <sys/types.h>	/* OPEN(2) */
#include <fcntl.h>		/* OPEN(2) */
#include <unistd.h>		/* WRITE(2) */
#include <linux/limits.h>	/* PATH_MAX */
/* LIB HEADER */
#include "commdef.h"
#include "mems.h"		/* stMEMSINFO */
#include "cifo.h"		/* stCIFO */
#include "gifo.h"		/* gifo_init_group() */
#include "nifo.h"		/* nifo_init_zone() */
#include "filedb.h"		/* pst_DIRECT_MNG */
#include "loglib.h"
#include "ipclib.h"
/* PRO HEADER */
#include "procid.h"
#include "sshmid.h"
#include "path.h"
#include "msgdef.h"
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
#include "almstat.h"
/* LOC HEADER */
#include "chsmd_link.h"	/* dReadLinkDevInfo() */
#include "chsmd_disk.h"	/* st_Soldf, st_SoldfList */
#include "chsmd_ntp.h"	/* st_NTP_STS */
#include "chsmd_mask.h"	/* dReadMaskInfo(), dWriteMaskInfo() */
#include "chsmd_init.h"

/**B.1*  Definition of New Constants *********************/
/**B.2*  Definition of New Type  **************************/
/**C.1*  Declaration of Variables  ( Local ) **************/

int	gdIndex;
int FinishFlag;

pst_WNTAM	  stWNTAM;

/**C.2*  Declaration of Variables ( External ) ****************/
extern stMEMSINFO	 *gpMEMSINFO;
extern stCIFO		 *gpCIFO;
extern pst_NTAM       fidb;
extern pst_DIRECT_MNG director;
extern pst_SWITCH_MNG swch;
extern pst_keepalive  keepalive;

extern int dCurrBlockCnt;
extern int StopFlag;
extern int hwpwrcnt;
extern int hwfancnt;
extern int hwdiskcnt;
extern char szHPLogDir[];
extern unsigned char ucStatus;
extern unsigned char ucDStatus;
extern unsigned char ucStatusFlag;
extern st_NTP_STS stNTPSTS;
extern st_NTP_STS oldNTPSTS;

extern st_linkdev stLinkDev[];


/**D.1*  Definition of Functions  ( Local ) ***************/
/**D.2*  Definition of Functions  *************************/
int dGetBlocks(char *fn, char (*p)[30])
{
	int		dLineCount, dReadCount, dScanCount;
	char	sBuf[BUF_LEN], sBlockName[PROC_NAME_LEN];
	FILE	*fp;

	if( (fp = fopen(fn, "r")) == NULL)
	{
		log_print(LOGN_CRI, LH"FAILED IN fopen(%s)"EH,LT,fn,ET);
		return -1;
	}

	dLineCount	= 0;
	dReadCount	= 0;
	while(fgets(sBuf, BUF_LEN, fp) != NULL)
	{
		dLineCount++;
		/*	from Source to Target: sscanf	*/
		if(sBuf[0] != '#')
		{
			log_print(LOGN_CRI, LH"SYNTAX ERROR FILE:%s, LINE:%d - FIRST CHARACTER IS MUST '#'!", LT, fn, dLineCount);
			fclose(fp);
			return -2;
		}
		else if(sBuf[1] == '#')
			continue;
		else if(sBuf[1] == 'E')
			break;
		else if(sBuf[1] == '@')
		{
			if( (dScanCount = sscanf(&sBuf[2], "%s %*s", sBlockName)) != 1)
				sprintf(sBlockName, "-");

			sprintf(*(p+dReadCount), "%s", sBlockName);
			dReadCount++;
		}
		else
		{
			log_print(LOGN_CRI, LH"SYNTAX ERROR FILE:%s, LINK:%d", LT, fn, dLineCount);
			fclose(fp);
			return -2;
		}
	}
	fclose(fp);

	return dReadCount;
}

int init_ipcs(void)
{
	gpMEMSINFO = nifo_init_zone((U8*)"CHSMD", SEQ_PROC_CHSMD, FILE_NIFO_ZONE);
    if( gpMEMSINFO == NULL ){
        log_print(LOGN_CRI, LH"FAILED IN nifo_init, NULL", LT);
        return -1;
    }

    //GIFO를 사용하기 위한 group 설정
    gpCIFO = gifo_init_group(FILE_CIFO_CONF, FILE_GIFO_CONF);
    if( gpCIFO == NULL ){
        log_print(LOGN_CRI, LH"FAILED IN gifo_init_group. cifo=%s, gifo=%s",
                LT, FILE_CIFO_CONF, FILE_GIFO_CONF);
        return -2;
    }

	return 0;
}

void UserControlledSignal(int sign)
{
    FinishFlag = sign;
    StopFlag   = 0;

	log_print(LOGN_CRI,LH"[EXIT]:[%d]"EH, LT, sign, ET);
}

void IgnoreSignal(int sign)
{
    if(sign != SIGALRM){
        log_print(LOGN_CRI,LH"UNWANTED SIGNAL IS RECEIVED, signal = %d", LT, sign);
	}

    signal(sign, IgnoreSignal);
}

void FinishProgram(void)
{
	int		dRet;

	if( (dRet = dWriteMaskInfo(0)) < 0)
		log_print(LOGN_CRI, LH"ERROR IN dWriteMaskInfo() dRet=%d"EH, LT, dRet, ET);

	write_FIDB();
	log_close();
	log_print(LOGN_CRI, LH"PROGRAM IS NORMALLY TERMINATED, Cause=%d", LT, FinishFlag);

	exit(0);
}

void write_FIDB(void)
{
    int     fd;

    if ((fd = open (FILE_FIDB, O_RDWR | O_CREAT, 0666)) < 0) {
        log_print(LOGN_CRI, LH"%s CREATION FAILED"EH, LT,FILE_FIDB,ET);

    } else if (write (fd, (st_WNTAM *)fidb, DEF_WNTAM_SIZE) < 0) {
        log_print(LOGN_CRI, LH"%s WRITE FAILED"EH, LT,FILE_FIDB,ET);
    }

    close(fd);

	return;
}

int dReadFidb(void)
{
	int			fd;
	ssize_t		sszReadLen;
	st_WNTAM	stFidb;

	if( (fd = open(FILE_FIDB, O_RDONLY, 0666)) == -1) {
		log_print(LOGN_CRI, LH"FAILED IN open(%s)"EH, LT,FILE_FIDB,ET);
		return -1;
	}

	if( (sszReadLen = read(fd, (st_WNTAM*)&stFidb, DEF_WNTAM_SIZE)) == -1) {
		log_print(LOGN_CRI, LH"FAILED IN read(%s)"EH, LT,FILE_FIDB,ET);

		if( close(fd) == -1) {
			log_print(LOGN_CRI, LH"FAILED IN close(%s)"EH, LT,FILE_FIDB,ET);
			return -2;
		}
		return -3;
	}

	if(DEF_WNTAM_SIZE != sszReadLen) {
		log_print(LOGN_CRI, LH"read byte length=%lu is not equal to sizeof(st_fidb)=%lu", 
			LT, sszReadLen, DEF_WNTAM_SIZE);

		if( close(fd) == -1) {
			log_print(LOGN_CRI, LH"FAILED IN close2(%s)"EH, LT,FILE_FIDB,ET);
			return -4;
		}
		return -5;

	}

	memcpy(stWNTAM, &stFidb, DEF_WNTAM_SIZE);

	fidb->mpsw[SEQ_PROC_CHSMD]         = CRITICAL;
	fidb->mpswinfo[SEQ_PROC_CHSMD].pid = 0;

	if(close(fd) == -1) {
		log_print(LOGN_CRI,LH"FAILED IN close3(%s)"EH, LT,FILE_FIDB,ET);
		return -6;
	}

	return 0;
}

int dInitFidb(void)
{
	int		i, dRet;
	time_t	now;

	if( (dRet = shm_init(S_SSHM_FIDB, DEF_WNTAM_SIZE, (void**)&stWNTAM)) < 0 ){
		log_print(LOGN_CRI,LH"FAILED IN shm_init(S_SSHM_FIDB[0x%04X], dRet=%d)"EH, 
		LT,S_SSHM_FIDB,dRet,ET);
		return -6;
	}

	fidb     = &stWNTAM->stNTAM;
	director = &stWNTAM->stDirectTOT;
	swch     = &stWNTAM->stSwitchTOT;

	if( dRet != SHM_EXIST ){
		//initialization
		time(&now);

		for(i = 0; i < MAX_DIRECT_COUNT; i++){
			director->stDIRECT[i].tEachUpTime = now;
		}

		for(i = 0; i < 256; i++){
			fidb->tEventUpTime[i] = now;
		}

		log_print(LOGN_CRI,LH"Initialized a FIDB tEventUpTime=%ld",LT,now);

		Init_STATUS_SHM_VALUE();

	} else {

		log_print(LOGN_CRI,LH"a FIDB exist, tEventUpTime=%lld",LT, fidb->tEventUpTime[0]);

		if( (dRet = dReadFidb()) < 0){
			log_print(LOGN_CRI, LH"ERROR IN dReadFidb(), dRet=%d", LT, dRet);
		}
	}

	if( (dRet = dReadMaskInfo(0)) < 0){
		log_print(LOGN_CRI, LH"ERROR IN dReadMaskInfo(), dRet=%d", LT, dRet);
	}

	return 0;
}

void Init_STATUS_SHM_VALUE(void)
{
	int		i;

	for(i = 0; i < MAX_SW_COUNT; i++) {

		if(i < dCurrBlockCnt){
			fidb->mpsw[i] = STOP;
		}
	}

	fidb->cDBAlive = NORMAL;
	fidb->hwNtpCnt = CURR_NTP_CNT;
}

void signal_handling(void)
{
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
    signal(SIGUSR2, IgnoreSignal);
    signal(SIGVTALRM, IgnoreSignal);
}

int dGetSYSCFG(st_SoldfList *pstSolDfList)
{
	FILE			*fa;
	char			szBuf[1024], szType[64], szTmp[64], szInfo[64];
	int				i, dRet, dDiskCnt;

	dRet	= -1;
	if( (fa = fopen(FILE_SYS_CONFIG, "r")) == NULL)
	{
		log_print(LOGN_CRI,LH"LOAD SYSTEM CONFIG : %s FILE OPEN FAIL"EH, LT,FILE_SYS_CONFIG, ET);
		return -1;
	}

	i		 = 0;
	dDiskCnt = 0;
	while(fgets(szBuf, 1024, fa) != NULL) {

		if(szBuf[0] != '#'){
			log_print(LOGN_WARN,LH"FAILED IN dGetSYSCFG() : %s File [%d] row format error", 
				LT, FILE_SYS_CONFIG, i);
		}

		i++;
		if(szBuf[1] == '#'){
			continue;
		}else if(szBuf[1] == 'E'){
			break;
		}else if(szBuf[1] == '@') {
			if(sscanf(&szBuf[2], "%s %s %s", szType, szTmp, szInfo) == 3) {
				if(!strcmp(szType, "SYS")) {
					if(!strcmp(szTmp, "NO")) {
						dRet = atoi(szInfo);
						log_print(LOGN_DEBUG,LH"LOAD SYSNO : [ %d ]", LT, dRet );
					}

				} else if(!strcmp(szType, "DISK")) {
					if(!strcmp(szTmp, "NUM")) {
						if( atoi(szInfo) > MAX_DISK_COUNT) {
							log_print(LOGN_CRI, LH"Disk count[%d] is over maximum[%d] in FIDB", 
								LT, atoi(szInfo), MAX_DISK_COUNT);
							fclose(fa);
							return -1;

						} else {
							pstSolDfList->dCount	= atoi(szInfo);
						}

					} else if(!strcmp(szTmp, "PATH")) {
						sprintf(pstSolDfList->stSoldf[dDiskCnt].szMountp, "%s", szInfo);
						log_print(LOGN_DEBUG, LH"stSolDfList.stSoldf[%d].szMountp=\'%s\'", 
							LT, dDiskCnt, pstSolDfList->stSoldf[dDiskCnt].szMountp);
						dDiskCnt++;
					}

				} else if(!strcmp(szType, "HPLOG")) {
					if(strcmp(szTmp, "DIR") == 0) {
						sprintf(&szHPLogDir[0],"%s",szInfo);
						log_print(LOGN_DEBUG,LH"LOAD hplog DIR : %s",LT, szHPLogDir );
					}
				}
				/*** HPLOG 는 모든 SYSTEM 에서 HP 장비인 경우 작동 */
				/*** TAF 장비인 경우, DAGBIN 을 찾아야함 FILE LIB 수정 후 hw.c 에 추가 */
				/*** linksts 값의 경우, eth 정보를 읽어다가 적용하도록 link 에 반영 */
			}
		}
	}

	fclose(fa);

	if(pstSolDfList->dCount != dDiskCnt) {
		log_print(LOGN_CRI, LH"Disk Count[%d] is not match DISK NUM field[%d]",
			LT, dDiskCnt, pstSolDfList->dCount);

		return -2;
	}

	return dRet;
}

int dGetDirCFG(void)
{
	FILE			*fa;
	char			sPathMAX[PATH_MAX], sBuf[BUFSIZ], sType[64], sTmp[64], cTmp, cMON_Info[MAX_MIRROR_PORT_COUNT];
	int				i, j, dRet;
	unsigned int	uMaskShift;
	st_DIRECT		*pstDirect;

	dRet	= -1;
	sprintf(sPathMAX, "%s%s", DATA_PATH, FILE_DIRECTOR_CONF);
	if( (fa = fopen(sPathMAX, "r")) == NULL)
	{
		log_print(LOGN_CRI, LH"FAILED IN fopen(%s)"EH, LT, sPathMAX, ET);
		return -1;
	}

	i			= 0;
	while(fgets(sBuf, BUFSIZ, fa) != NULL)
	{
		if(sBuf[0] != '#')
			log_print(LOGN_WARN, LH"File[%s] %d-th row format error", LT, sPathMAX, i);

		i++;
		if(sBuf[1] == '#')
			continue;
		else if(sBuf[1] == 'E')
			break;
		else if(sBuf[1] == '@')
		{
			if( (dRet = sscanf(&sBuf[2], "%s %s %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c", sType, sTmp,
				&cMON_Info[0], &cMON_Info[1], &cMON_Info[2], &cMON_Info[3], &cMON_Info[4], &cMON_Info[5], &cMON_Info[6], &cMON_Info[7], &cMON_Info[8], &cMON_Info[9], &cMON_Info[10], &cMON_Info[11],
				&cMON_Info[12], &cMON_Info[13], &cMON_Info[14], &cMON_Info[15], &cMON_Info[16], &cMON_Info[17], &cMON_Info[18], &cMON_Info[19], &cMON_Info[20], &cMON_Info[21], &cMON_Info[22], &cMON_Info[23])) > 0)
			{
				if(strcmp(sType, "DIRECT") == 0)
				{
					if(strcmp(sTmp, "NUM") == 0)
					{
						cTmp = (char)cMON_Info[0];
						if( (dRet != 3) && (atoi(&cTmp) > MAX_DIRECT_COUNT))
						{
							log_print(LOGN_CRI, LH"Director count[%d] is over maximum[%d] in FILE[%s]",
								LT, atoi(&cTmp), MAX_DIRECT_COUNT, sPathMAX);
							fclose(fa);
							return -2;
						}
					}
					else if(strncmp(sTmp, "MON", 3) == 0)
					{
						if(dRet != (MAX_MONITOR_PORT_COUNT+2))
						{
							log_print(LOGN_CRI, LH"sscanf[%s] dRet[%d] is not %d",
								LT, sBuf, dRet, (MAX_MONITOR_PORT_COUNT+2));
							fclose(fa);
							return -3;
						}

						cTmp		= (unsigned char)sTmp[strlen(sTmp)-1];
						cTmp		= atoi(&cTmp);
						pstDirect	= &director->stDIRECT[cTmp-1];

						for(uMaskShift = 0; uMaskShift < MAX_MONITOR_PORT_COUNT; uMaskShift++)
						{
							cTmp = (char)cMON_Info[uMaskShift];
							if(cTmp == '0')
								pstDirect->cMonitorPort[uMaskShift] = NOT_EQUIP;
							else if( (cTmp == '1') && (pstDirect->cMonitorPort[uMaskShift] != MASK))
								pstDirect->cMonitorPort[uMaskShift] = NORMAL;
							else if(pstDirect->cMonitorPort[uMaskShift] == MASK)
								continue;
							else
							{
								log_print(LOGN_CRI, LH"cTmp[%c] is wrong", LT, cTmp);
								fclose(fa);
								return -4;
							}
						}
					}
					else if(strncmp(sTmp, "MIR", 3) == 0)
					{
						if(dRet != (MAX_MIRROR_PORT_COUNT+2))
						{
							log_print(LOGN_CRI, LH"sscanf[%s] dRet[%d] is not %d", 
								LT,sBuf, dRet, (MAX_MIRROR_PORT_COUNT+2));
							fclose(fa);
							return -5;
						}

						cTmp		= (unsigned char)sTmp[strlen(sTmp)-1];
						cTmp		= atoi(&cTmp);
						pstDirect	= &director->stDIRECT[cTmp-1];

						for(uMaskShift = 0; uMaskShift < MAX_MIRROR_PORT_COUNT; uMaskShift++)
						{
							cTmp = (char)cMON_Info[uMaskShift];
							if(cTmp == '0')
								pstDirect->cMirrorPort[uMaskShift] = NOT_EQUIP;
							else if( (cTmp == '1') && (pstDirect->cMirrorPort[uMaskShift] != MASK))
								pstDirect->cMirrorPort[uMaskShift] = NORMAL;
							else if(pstDirect->cMirrorPort[uMaskShift] == MASK)
								continue;
							else
							{
								log_print(LOGN_CRI, LH"cTmp[%c] is wrong", LT, cTmp);
								fclose(fa);
								return -6;
							}
						}
					}
					else if(strncmp(sTmp, "POWER", 5) == 0)
					{
						if(dRet != (MAX_DIRECT_POWER_COUNT+2))
						{
							log_print(LOGN_CRI, LH"sscanf[%s] dRet[%d] is not %d", 
								LT,sBuf, dRet, (MAX_DIRECT_POWER_COUNT+2));
							fclose(fa);
							return -7;
						}

						cTmp		= (unsigned char)sTmp[strlen(sTmp)-1];
						cTmp		= atoi(&cTmp);
						pstDirect	= &director->stDIRECT[cTmp-1];

						for(uMaskShift = 0; uMaskShift < MAX_DIRECT_POWER_COUNT; uMaskShift++)
						{
							cTmp = (unsigned char)cMON_Info[uMaskShift];
							if(cTmp == '0')
								pstDirect->cPower[uMaskShift] = NOT_EQUIP;
							else if( (cTmp == '1') && (pstDirect->cPower[uMaskShift] != MASK))
								pstDirect->cPower[uMaskShift] = NORMAL;
							else if(pstDirect->cPower[uMaskShift] == MASK)
								continue;
							else
							{
								log_print(LOGN_CRI, LH"cTmp[%c] is wrong", LT, cTmp);
								fclose(fa);
								return -7;
							}
						}
					}
				}
			}
		}
	}
	fclose(fa);

	for(i = 0; i < MAX_DIRECT_COUNT; i++)
	{
		for(j = 0; j < MAX_MONITOR_PORT_COUNT; j++)
			log_print(LOGN_DEBUG, LH"director->stDIRECT.stDIRECT[%d].cMonitorPort[%02d] = 0x%02X", LT, i, j, director->stDIRECT[i].cMonitorPort[j]);

		for(j = 0; j < MAX_MIRROR_PORT_COUNT; j++)
			log_print(LOGN_DEBUG, LH"director->stDIRECT.stDIRECT[%d].cMirrorPort[%02d] = 0x%02X", LT, i, j, director->stDIRECT[i].cMirrorPort[j]);

		for(j = 0; j < MAX_DIRECT_POWER_COUNT; j++)
			log_print(LOGN_DEBUG, LH"director->stDIRECT.stDIRECT[%d].cPower[%02d] = 0x%02X", LT, i, j, director->stDIRECT[i].cPower[j]);
	}

	return 0;
}

int dGetSwitchCFG(void)
{
	FILE			*fa;
	char			sPathMAX[PATH_MAX], sBuf[BUFSIZ], sTmp[64];
	unsigned char	cTmp, cSwitch_Info[MAX_SWITCH_PORT_COUNT];
	int				i, j, dRet, dTmp;
	unsigned int	uMaskShift;
	st_SWITCH		*pstSwitch;

	dRet	= -1;
	sprintf(sPathMAX, "%s%s", DATA_PATH, FILE_SWITCH_CONF);
	if( (fa = fopen(sPathMAX, "r")) == NULL)
	{
		log_print(LOGN_CRI, LH"FAILED IN fopen(%s)"EH,LT,sPathMAX,ET);
		return -1;
	}

	i			= 0;
	while(fgets(sBuf, BUFSIZ, fa) != NULL)
	{
		if(sBuf[0] != '#')
			log_print(LOGN_WARN, LH"File[%s] %d-th row format error", LT, sPathMAX, i);

		i++;
		if(sBuf[1] == '#')
			continue;
		else if(sBuf[1] == 'E')
			break;
		else if(sBuf[1] == '@')
		{
			if( (dRet = sscanf(&sBuf[2], "%s %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c", sTmp,
				&cSwitch_Info[0], &cSwitch_Info[1], &cSwitch_Info[2], &cSwitch_Info[3], &cSwitch_Info[4], &cSwitch_Info[5],
				&cSwitch_Info[6], &cSwitch_Info[7], &cSwitch_Info[8], &cSwitch_Info[9], &cSwitch_Info[10], &cSwitch_Info[11],
				&cSwitch_Info[12], &cSwitch_Info[13], &cSwitch_Info[14], &cSwitch_Info[15], &cSwitch_Info[16], &cSwitch_Info[17],
				&cSwitch_Info[18], &cSwitch_Info[19], &cSwitch_Info[20], &cSwitch_Info[21], &cSwitch_Info[22], &cSwitch_Info[23])) > 0)
			{
				if(strncmp(sTmp, "SWITCH", 6) == 0)
				{
					if(dRet != (MAX_SWITCH_PORT_COUNT+1))
					{
						log_print(LOGN_CRI, LH"sscanf[%s] dRet[%d] is not %d", LT,
							sBuf, dRet, (MAX_SWITCH_PORT_COUNT+1));
						fclose(fa);
						return -3;
					}

					if( (dTmp = atoi(&sTmp[strlen(sTmp)-1])) > MAX_SWITCH_COUNT)
					{
						log_print(LOGN_CRI, LH"sTmp[%s] should be a \"SWITCH1\" or \"SWITCH2\"", LT, sTmp);
						continue;
					}

					pstSwitch	= &swch->stSwitch[dTmp-1];
					for(uMaskShift = 0; uMaskShift < MAX_SWITCH_PORT_COUNT; uMaskShift++)
					{
						cTmp = (unsigned char)cSwitch_Info[uMaskShift];
						if(cTmp == '0')
							pstSwitch->cSwitchPort[uMaskShift] = NOT_EQUIP;
						else if( (cTmp == '1') && (pstSwitch->cSwitchPort[uMaskShift] != MASK))
							pstSwitch->cSwitchPort[uMaskShift] = NORMAL;
						else if(pstSwitch->cSwitchPort[uMaskShift] == MASK)
							continue;
						else
						{
							log_print(LOGN_CRI, LH"cTmp[%c] is wrong", LT, cTmp);
							fclose(fa);
							return -4;
						}
					}
				}
			}
		}
	}
	fclose(fa);

	for(i = 0; i < MAX_SWITCH_COUNT; i++)
	{
		for(j = 0; j < MAX_SWITCH_PORT_COUNT; j++)
			log_print(LOGN_DEBUG, "swch->stSwitch[%d].cSwitchPort[%02d] = 0x%02X", i, j, swch->stSwitch[i].cSwitchPort[j]);
	}

	return 0;
}

int dInitProcess(void)
{
	int		i, dRet;

	fidb     = &stWNTAM->stNTAM;
	director = &stWNTAM->stDirectTOT;
	swch     = &stWNTAM->stSwitchTOT;

	if( (dRet = shm_init(S_SSHM_KEEPALIVE, DEF_KEEPALIVE_SIZE, (void**)&keepalive)) < 0 ){
		log_print(LOGN_CRI,LH"FAILED IN shm_init(S_SSHM_KEEPALIVE[0x%04X], dRet=%d)"EH,
			LT,S_SSHM_KEEPALIVE,dRet,ET);
		return -6;
	}

	if( dRet != SHM_EXIST ){
		//initialization
	}

	if( (dRet = dInitFidb()) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dInitFidb() dRet[%d]", LT, dRet);
		return -1;
	}

	if( (dRet = dReadLinkDevInfo()) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dReadLinkDevInfo() dRet[%d]", LT, dRet);
		return -2;
	}

	for(i = 0; i < MAX_LINK_COUNT; i++)
	{
		if(strlen(stLinkDev[i].szDevName) == 0)
			fidb->link[i] = NOT_EQUIP;

		log_print(LOGN_INFO, LH"DEVICE NAME [%s]", LT, stLinkDev[i].szDevName);
	}

	if( (dRet = init_ipcs()) < 0)
		return -3;

	if( (dRet = Init_Keep_Load()) < 0)
		return -4;

	/*
	* CHANNEL INIT NTAFChnl for NTAM( or NTAMChnl for GTAM )
	*/
	Init_Chnl();

	ucDStatus = fidb->hwNTP[0];
	ucStatus = fidb->hwNTP[1];

	memset(&stNTPSTS, 0x00, sizeof(stNTPSTS));
	memset(&oldNTPSTS, 0x00, sizeof(oldNTPSTS));

	gdIndex = 1;

	fidb->hwPwrCnt= CURR_PWR_CNT;
	for(i = 0; i < MAX_PWR_COUNT; i++)
	{
		if(i < CURR_PWR_CNT)
			fidb->hwPWR[i] = NORMAL;
		else
			fidb->hwPWR[i] = NOT_EQUIP;
	}

	fidb->hwFanCnt= CURR_FAN_CNT;
	for(i = 0; i < MAX_FAN_COUNT; i++)
	{
		if(i < CURR_FAN_CNT)
			fidb->hwFan[i] = NORMAL;
		else
			fidb->hwFan[i] = NOT_EQUIP;
	}

	hwpwrcnt	= fidb->hwPwrCnt;
	hwfancnt	= fidb->hwFanCnt;
	hwdiskcnt	= fidb->hwDiskArrayCnt;
	log_print(LOGN_CRI, LH"HARDWARE POWER COUNT[%d] FAN COUNT[%d]", LT, hwpwrcnt, hwfancnt);

	if( (dRet = dGetDirCFG()) < 0)
		log_print(LOGN_CRI, LH"ERROR IN dGetDirCFG() dRet[%d]", LT, dRet);

	if( (dRet = dGetSwitchCFG()) < 0)
		log_print(LOGN_CRI, LH"ERROR IN dGetSwitchCFG() dRet[%d]", LT, dRet);

	return 0;
}

int Init_Keep_Load(void)
{
	FILE        *fp_ntam;
    int         dRet;
    char        szBuffer[128];
    char        szName[16];
	int         dCri, dMaj, dMin;


	if( (fp_ntam = fopen(FILE_TAM_LOAD_DATA, "r")) == NULL )
    {
        log_print(LOGN_DEBUG,LH"[ERROR] NTAM LOAD ALARM FILE OPEN",LT);
        return -1;
    }

    while( fgets( szBuffer, 128, fp_ntam ) != NULL )
    {
        if( szBuffer[0] != '#' )
        {
            dRet = -1;
            break;
        }

        if(szBuffer[1] == '#')
            continue;
        else if(szBuffer[1] == 'E')
            break;
        else
        {
            sscanf( &szBuffer[2], "%s %d %d %d", szName, &dCri, &dMaj, &dMin );

			if( !strcmp( szName, "CPU" ) )
            {
                keepalive->stTAMLoad.cpu.usMinor		= dMin;
                keepalive->stTAMLoad.cpu.usMajor		= dMaj;
                keepalive->stTAMLoad.cpu.usCritical	= dCri;
            }
            else if( !strcmp( szName, "MEM" ) )
            {
                keepalive->stTAMLoad.mem.usMinor		= dMin;
                keepalive->stTAMLoad.mem.usMajor		= dMaj;
                keepalive->stTAMLoad.mem.usCritical	= dCri;
            }
            else if( !strcmp( szName, "DISK" ) )
            {
                keepalive->stTAMLoad.disk.usMinor	= dMin;
                keepalive->stTAMLoad.disk.usMajor	= dMaj;
                keepalive->stTAMLoad.disk.usCritical	= dCri;
            }
            else if( !strcmp( szName, "QUE" ) )
            {
                keepalive->stTAMLoad.que.usMinor		= dMin;
                keepalive->stTAMLoad.que.usMajor		= dMaj;
                keepalive->stTAMLoad.que.usCritical	= dCri;
            }
			else if( !strcmp( szName, "NIFO" ) )
            {
                keepalive->stTAMLoad.nifo.usMinor	= dMin;
                keepalive->stTAMLoad.nifo.usMajor	= dMaj;
                keepalive->stTAMLoad.nifo.usCritical	= dCri;
            }
            else if( !strcmp( szName, "SWCPU" ) )
            {
                keepalive->stSWCHLoad.cpu.usMinor		= dMin;
                keepalive->stSWCHLoad.cpu.usMajor		= dMaj;
                keepalive->stSWCHLoad.cpu.usCritical	= dCri;
            }
			else if( !strcmp( szName, "SWMEM" ) )
            {
                keepalive->stSWCHLoad.mem.usMinor		= dMin;
                keepalive->stSWCHLoad.mem.usMajor		= dMaj;
                keepalive->stSWCHLoad.mem.usCritical	= dCri;
            }
		}
	}

	fclose( fp_ntam );

	return 1;
}

int dValidIP(char *IPaddr)
{
	int		d1, d2, d3, d4;

	if( (sscanf(IPaddr, "%d.%d.%d.%d", &d1, &d2, &d3, &d4) != 4) ||
			((d1 < 0) || (d1 > 255) || (d2 < 0) || (d2 > 255) || (d3 < 0) || (d3 > 255) || (d4 < 0) || (d4 > 255)))
	{
		log_print(LOGN_WARN, LH"IP address(%s) is not valid", LT, IPaddr);
		return -1;
	}

	return 0;
}
