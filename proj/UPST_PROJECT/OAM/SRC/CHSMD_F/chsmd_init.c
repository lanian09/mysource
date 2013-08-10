/**A.1*  File Inclusion *******************************************************/

/* SYS HEADER */
#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>		/* READ(2), WRITE(2), CLOSE(2) */
#include <sys/ipc.h>	/* IPC_CREAT/// */
#include <errno.h>
#include <string.h>		/* memcpy, memset, strcpy */
#include <stdlib.h>		/* EXIT(3) */
#include <sys/shm.h>	/* shmget(), shmat(), SHM_REATE */
/* LIB HEADER */
#include "mems.h"		/* stMEMSINFO */
#include "cifo.h"		/* stCIFO */
#include "gifo.h"		/* gifo_init_group() */
#include "nifo.h"		/* nifo_init_zone() */
#include "filedb.h"		/* st_NTAF */
#include "commdef.h"	/* FILE_MIRROR_TIME */
#include "loglib.h"
#include "ipclib.h"		/* shm_init() */
/* PRO HEADER */
#include "sshmid.h"
#include "msgdef.h"		/* SEQ_PROC* */
#include "path.h"
#include "procid.h"
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
#include "almstat.h"	/* st_NTPSTS */
/* LOC HEADER */
#include "chsmd_load.h"	/* st_linkdev */
#include "chsmd_msg.h"	/* SetFIDBValue */
#include "chsmd_disk.h" /* st_SoldfList */
#include "chsmd_init.h"

/**B.1*  Definition of New Constants ******************************************/
/**B.2*  Definition of New Type  **********************************************/
/**C.1*  Declaration of Variables  ********************************************/
pst_NTAF 		fidb;
pst_keepalive	keepalive;

int  Finishflag;
int	 gdIndex;
int  dNIDIndex;
int  link_oldstat[MAX_NTAF_LINK];
char cpu_oldstat;
char mem_oldstat;
char queue_oldstat;
char disk_oldstat[MAX_NTAF_DISK_COUNT];

/**C.2*  Declaration of Variables  ********************************************/
extern stMEMSINFO  *gpMEMSINFO;
extern stCIFO	   *gpCIFO;
extern st_NTP_STS   stNTPSTS;
extern st_NTP_STS   oldNTPSTS;
extern st_linkdev   stLinkDev[MAX_NTAF_LINK];
extern int  StopFlag;
extern char ucStatus;
extern char ucDStatus;
extern char ucStatusFlag;
extern char szDagBinDir[];
extern char szHPLogDir[];

/**D.1*  Definition of Functions  *********************************************/
/**D.2*  Definition of Functions  *********************************************/
int dGetBlocks(char *fn, char (*p)[30])
{
    int     ln;
    int     rdcnt;
	int     scan_cnt;
    char    buf[BUF_LEN];
    char    Bname[PROC_NAME_LEN];
    FILE    *fp;

    fp = fopen(fn,"r");
    if( NULL == fp ){
        return -1;  /* fopen error */
    }

    ln = 0;
	rdcnt = 0;
    while(fgets(buf, BUF_LEN, fp) != NULL ){

        ln++;
		/*
		* from Source to Target : sscanf
		*/
		if( buf[0] != '#' ){
			log_print(LOGN_CRI,"SYNTAX ERROR FILE:%s, LINE:%d - FIRST CHARACTER IS MUST '#'!\n",fn, ln);
			return -1;
		}
		else if( buf[1] == '#' ){
			continue;
		}
		else if( buf[1] == 'E' ){
			/*
			* EOF
			*/
			break;
		}
		else if( buf[1] == '@' ){
				scan_cnt= sscanf( &buf[2], "%s %*s", Bname );
				if( scan_cnt != 1 ){
					sprintf( Bname, " - " );
				}

				sprintf( *(p+rdcnt), "%s", Bname );
				rdcnt++;
		}
		else{
			log_print(LOGN_CRI,"SYNTAX ERROR FILE:%s, LINK:%d\n",fn, ln);
			return -2;
		}

    }/* while */
    fclose(fp);

    return rdcnt;
}

int dGetBlockBIN(char *sBlockName, char *sBinName, int dBinLength)
{
	int		dLineNum, dRdCnt, dScanCnt;
	char    sBuf[BUF_LEN], sBname[PROC_NAME_LEN], sBinPath[BUFSIZ];
	size_t	szBinStrLen;
	FILE    *fp;

	if( (fp = fopen(FILE_MC_INIT, "r")) == NULL)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN fopen(%s) errno[%d-%s]", __FILE__, __FUNCTION__, __LINE__,
			FILE_MC_INIT, errno, strerror(errno));
		return -1;
	}

	dLineNum	= 0;
	dRdCnt		= 0;
	while(fgets(sBuf, BUF_LEN, fp) != NULL)
	{
		dLineNum++;
		/*	from Source to Target : sscanf	*/
		if(sBuf[0] != '#')
		{
			log_print(LOGN_CRI, "SYNTAX ERROR FILE:%s, LINE:%d - FIRST CHARACTER IS MUST '#'!\n", FILE_MC_INIT, dLineNum);
			fclose(fp);
			return -2;
		}
		else if(sBuf[1] == '#')
			continue;
		else if(sBuf[1] == 'E')
			break;
		else if(sBuf[1] == '@')
		{
			if( (dScanCnt = sscanf(&sBuf[2], "%s %s", sBname, sBinPath)) != 2)
				log_print(LOGN_CRI, "F=%s:%s.%d: ERROR IN sscanf[%s] sBname[%s] sBinPath[%s]", __FILE__, __FUNCTION__, __LINE__, &sBuf[2], sBname, sBinPath);
			else
			{
				if(strcmp(sBname, sBlockName) == 0)
				{
					szBinStrLen = strlen(sBinPath);
					if(dBinLength > szBinStrLen)
					{
						strncpy(sBinName, sBinPath, dBinLength);
						break;
					}
					else
					{
						log_print(LOGN_CRI, "F=%s:%s.%d: sBinPath[%s] is longer than dBinLength[%d]", __FILE__, __FUNCTION__, __LINE__,
							sBinPath, dBinLength);
						fclose(fp);
						return -3;
					}
				}
				else
				{
					log_print(LOGN_DEBUG, "F=%s:%s.%d: sBname[%s] is different from sBlockName[%s]", __FILE__, __FUNCTION__, __LINE__,
						sBname, sBlockName);
				}
			}
			dRdCnt++;
		}
		else
		{
			log_print(LOGN_CRI,"SYNTAX ERROR FILE:%s, LINK:%d\n", FILE_MC_INIT, dLineNum);
			fclose(fp);
			return -4;
		}
	}
	fclose(fp);

	return 0;
}

int dReadLinkDevInfo(void)
{
   
    FILE        *fp;
    int         dFlag, dLinkDevCnt;
    char        sType[8], sDevName[8], sBuf[256];
    size_t      szLinkDev;
    st_linkdev  stTmpLinkDev[MAX_NTAF_LINK];
    
    dLinkDevCnt = 0;
    szLinkDev   = DEF_LINKDEV_SIZE*MAX_NTAF_LINK;
    memcpy(stTmpLinkDev, stLinkDev, szLinkDev);
    memset(stLinkDev, 0x00, szLinkDev);

    if( (fp = fopen(FILE_SYS_CONFIG, "r")) == NULL)
    {
        log_print(LOGN_CRI, "F=%s:%s.%d FILE_SYS_CONFIG OPEN [%s]", __FILE__, __FUNCTION__, __LINE__, FILE_SYS_CONFIG);
        return -1;
    }   

    while(fgets(sBuf, 256, fp) != NULL)
    {   
        if(sBuf[0] != '#')
        {
            log_print(LOGN_CRI, "SYNTAX ERROR FILE_SYS_CONFIG");
            break;
        }
    
        if(sBuf[1] == '#')
            continue;
        else if(sBuf[1] == 'E')
            break;
        else if(sBuf[1] == '@')
        {
            if(dLinkDevCnt > MAX_NTAF_LINK)
            {
                log_print(LOGN_CRI, "OVER LINK DEVICE COUNT");
                break;
            }

            if(sscanf(&sBuf[2], "%s %s %d", sType, sDevName, &dFlag) == 3)
            {
                if(strcmp(sType, "NETTYPE") == 0)
                {
                    strcpy(stLinkDev[dLinkDevCnt].szDevName, sDevName);
                    if(dFlag == NOT_EQUIP)
                        dFlag = NOT_EQUIP;

                    stLinkDev[dLinkDevCnt].ucFlag = dFlag;
                    log_print(LOGN_DEBUG, "[%02d] [%s]", dLinkDevCnt, stLinkDev[dLinkDevCnt].szDevName);
                    dLinkDevCnt++;
                }
            }
        }
    }
    fclose(fp);
	return dLinkDevCnt;
}
int init_CHSMD(void)
{
	int	i, dRet;

	if( (dRet = shm_init(S_SSHM_KEEPALIVE, DEF_KEEPALIVE_TAF_SIZE, (void**)&keepalive)) < 0 ){
        log_print(LOGN_CRI,LH"FAILED IN shm_init(S_SSHM_KEEPALIVE[0x%04X], dRet=%d)"EH,
			LT,S_SSHM_KEEPALIVE,dRet,ET);
        return -1;
    }

	if( (dRet = shm_init(S_SSHM_FIDB, DEF_TAF_SIZE, (void**)&fidb)) < 0 ){
        log_print(LOGN_CRI,LH"FAILED IN shm_init(S_SSHM_FIDB[0x%04X], dRet=%d)"EH,
			LT,S_SSHM_FIDB,dRet,ET);
        return -1;
    }

	if(read_FIDB() < 0)
	{
		log_print(LOGN_DEBUG, "READ_FIDB FAIL --- SYSTEM INITIAL STATUS");
		if(dRet != SHM_EXIST)
			Init_STATUS_SHM_VALUE();

		if(fidb->hwfancnt == 0)
			fidb->hwfancnt = CURR_FAN_CNT;

		if(fidb->hwpwrcnt == 0)
			fidb->hwpwrcnt = CURR_PWR_CNT;

		if(fidb->hwportcnt == 0)
			fidb->hwportcnt = CURR_PORT_CNT;

		/*	1 Gbps = 134217728 bytes/sec	*/
		fidb->bytests.lMax	= 134217728;
	}

	dNIDIndex		= 1;
	gdIndex			= 1;
	cpu_oldstat		= fidb->cpu;
	mem_oldstat		= fidb->mem;
	queue_oldstat	= fidb->queue;

	for(i = 0; i < MAX_NTAF_DISK_COUNT; i++)
		disk_oldstat[i] = fidb->disk[i];

	if( (dRet = dReadLinkDevInfo()) < 0 ){
		log_print(LOGN_CRI,"FAILED IN dReaDLinkDevInfo(), dRet=%d", dRet);
		return -1;
	}

	for(i = 0; i < MAX_NTAF_LINK && i < dRet ; i++){
		if( stLinkDev[i].ucFlag != NOT_EQUIP ){
			link_oldstat[i] = fidb->link[i];
		}else{
			link_oldstat[i] = NOT_EQUIP;
		}
	}
	if( dRet < MAX_NTAF_LINK ){
		for(i = dRet; i < MAX_NTAF_LINK; i++ ){
			link_oldstat[i] = NOT_EQUIP;
		}
	}

	ucDStatus		= fidb->hwntp[0];
	ucStatus		= fidb->hwntp[1];
	ucStatusFlag	= 0;

	memset(&stNTPSTS, 0x00, sizeof(stNTPSTS));
	memset(&oldNTPSTS, 0x00, sizeof(oldNTPSTS));

	return 0;
}

/*******************************************************************************
 * MESSAGE QUEUE INITIALIZATION ( CHSMD, COND )
*******************************************************************************/
int init_ipcs()
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

	return 1;
}


/*******************************************************************************
 *
*******************************************************************************/
void UserControlledSignal(int sign)
{
	log_print(LOGN_CRI," CAPTURE SIGNAL [%d]", sign );

    Finishflag = sign;
    StopFlag = 0;
	return;
}


/*******************************************************************************
 * SIGNAL CONTROL FUNCTION
*******************************************************************************/
void IgnoreSignal(int sign)
{
    if(sign != SIGALRM)
	{
		log_print(LOGN_CRI, " UNWANTED SIGNAL IS RECEIVED, signal = %d", sign);
	}

    signal(sign, IgnoreSignal);

	return;
}


/*******************************************************************************
 * SIGNAL CONTROL FUNCTION
*******************************************************************************/
void AlarmSignal(int sign)
{
    if(sign != SIGALRM)
	{
		log_print(LOGN_CRI, " UNWANTED SIGNAL IS RECEIVED, signal = %d", sign);
	}

    signal(sign, AlarmSignal);

	return;
}


/*******************************************************************************
 * WHEN CHSMD FINISHED, RUNNING
*******************************************************************************/
void FinishProgram(void)
{
	write_FIDB();

	log_print(LOGN_CRI, "PROGRAM IS NORMALLY TERMINATED, Cause = %d", Finishflag);

	exit(0);
}


/*******************************************************************************
 * SHARED MEMORY ( FIDB ) INITIALIZATION
*******************************************************************************/
int Init_STATUS_SHM_VALUE(void)
{
	int i;

	for(i = 0; i < MAX_NTAF_LINK; i++)
		SetFIDBValue(&fidb->link[i], NOT_EQUIP);

	for(i = 0; i < MAX_SW_COUNT; i++)
		SetFIDBValue(&fidb->mpsw[i], STOP);

	fidb->hwfancnt = CURR_FAN_CNT;

	for(i = 0; i < 8; i++)
		SetFIDBValue(&fidb->hwfan[i], NOT_EQUIP);

	fidb->hwdiskcnt = 2;
	for(i = 0; i < 2; i++)
		SetFIDBValue(&fidb->hwdisk[i], NORMAL);

	fidb->hwpwrcnt = CURR_PWR_CNT;
	for(i = 0; i < 2; i++)
		SetFIDBValue(&fidb->hwpwr[i], NORMAL);

	for(i = 0; i < 4; i++)
		SetFIDBValue(&fidb->link[i], NOT_EQUIP);

	fidb->hwntpcnt = CURR_NTP_CNT;
	for(i = 0; i < 2; i++)
		SetFIDBValue(&fidb->hwntp[i], NOT_EQUIP);

	fidb->hwportcnt = CURR_PORT_CNT;
	for(i = 0; i < 8 ; i++)
		SetFIDBValue(&fidb->hwport[i], NOT_EQUIP);

	return 1;
}


/*******************************************************************************
 * WHEN CHSMD FINISHED, WRITE SHARED MEMORY TO FILE
*******************************************************************************/
int write_FIDB()
{
    int     fd;

	fd = open(FILE_FIDB, O_RDWR | O_CREAT, 0666);
    if( fd < 0 ) {
        log_print(LOGN_CRI, "%s CREATION FAILED", FILE_FIDB);

    } else if (write (fd, (st_NTAF *)fidb, sizeof(st_NTAF)) < 0) {
        log_print(LOGN_CRI, "%s WRITE FAILED", FILE_FIDB);
    }

    close(fd);

    return 1;
}


/*******************************************************************************
 * WHEN CHSMD RERUN, READ SHARED MEMORY( FIDB ) FROM FILE
*******************************************************************************/
int read_FIDB(void)
{
	int			fd, read_err;
	st_NTAF		stFidb;

	if( (fd = open(FILE_FIDB, O_RDONLY, 0666)) < 0)
	{
		log_print(LOGN_CRI, "%s OPEN FAILED", FILE_FIDB);
		return -1;
	}

	read_err = read(fd, (st_NTAF*)&stFidb, sizeof(st_NTAF));
	close(fd);

	stFidb.mpsw[SEQ_PROC_CHSMD]			= CRITICAL;
	stFidb.mpswinfo[SEQ_PROC_CHSMD].pid	= 0;

	if(read_err == sizeof(st_NTAF))
	{
		*fidb = stFidb;
		return 0;
	}
	else
		return -1;
}


/*******************************************************************************
 * INITIALIZE SIGNAL FUNCTION AND IPC INITIALIZATION
*******************************************************************************/
int init_prcmd()
{
	int dRet;

    /* WANTED SIGNALS   */
    signal(SIGTERM, UserControlledSignal);
    signal(SIGINT,  UserControlledSignal);
    signal(SIGQUIT, UserControlledSignal);

    /* UNWANTED SIGNALS */
    signal(SIGHUP,  IgnoreSignal);
    signal(SIGALRM, AlarmSignal);
    signal(SIGPIPE, IgnoreSignal);
    signal(SIGPOLL, IgnoreSignal);
    signal(SIGPROF, IgnoreSignal);

    if( (dRet =  init_ipcs()) < 0 ){

		log_print(LOGN_CRI,"* ERROR GET MSGQ[%d]",dRet);
        return -1;
	}

    return 1;
}

int dGetSYSCFG(st_SoldfList *pstSolDfList)
{
	FILE	*fa;
	char	szBuf[1024], szType[64], szTmp[64], szInfo[64];
	int		i, dRet, dDiskCnt;

	dRet	= -1;
	if( (fa = fopen(FILE_SYS_CONFIG,"r")) == NULL)
	{
		log_print(LOGN_CRI, "LOAD SYSTEM CONFIG : %s FILE OPEN FAIL (%s)", FILE_SYS_CONFIG, strerror(errno));
		return -1;
	}

	i 			= 0;
	dDiskCnt	= 0;
	while(fgets(szBuf, 1024, fa) != NULL)
	{
		if(szBuf[0] != '#')
			log_print(LOGN_WARN, "FAILED IN %s() : %s File [%d] row format error", __FUNCTION__, FILE_SYS_CONFIG, i);

		i++;
		if(szBuf[1] == '#')
			continue;
		else if(szBuf[1] == 'E')
			break;
		else if(szBuf[1] == '@')
		{
			if(sscanf(&szBuf[2], "%s %s %s", szType, szTmp, szInfo) == 3)
			{
				if(strcmp(szType, "SYS") == 0)
				{
					if(strcmp(szTmp, "NO") == 0)
					{
						dRet = atoi(szInfo);
						log_print(LOGN_DEBUG, "LOAD SYSNO[%d]", dRet);
					}
				}
				else if(strcmp(szType, "DISK") == 0)
				{
					if(strcmp(szTmp, "NUM") == 0)
					{
						if( atoi(szInfo) > MAX_NTAF_DISK_COUNT)
						{
							log_print(LOGN_CRI, "F=%s:%s.%d: Disk count[%d] is over maximum[%d] in FIDB", __FILE__, __FUNCTION__, __LINE__,
								atoi(szInfo), MAX_NTAF_DISK_COUNT);
							fclose(fa);
							return -1;
						}
						else
							pstSolDfList->dCount	= atoi(szInfo);
					}
					else if(strcmp(szTmp, "PATH") == 0)
					{
						sprintf(pstSolDfList->stSoldf[dDiskCnt].szMountp, "%s", szInfo);
						log_print(LOGN_DEBUG, "F=%s:%s.%d: stSolDfList.stSoldf[%d].szMountp=\'%s\'", __FILE__, __FUNCTION__, __LINE__,
							dDiskCnt, pstSolDfList->stSoldf[dDiskCnt].szMountp);
						dDiskCnt++;
					}
				}
				else if(strcmp(szType, "HPLOG") == 0)
				{
					if(strcmp(szTmp, "DIR") == 0)
					{
						sprintf(&szHPLogDir[0], "%s", szInfo);
						log_print(LOGN_DEBUG, "LOAD hplog DIR : %s", szHPLogDir);
					}
				}
				else if(strcmp(szType, "DAGBIN" ) == 0)
				{
					if(strcmp(szTmp, "DIR") == 0)
					{
						sprintf(&szDagBinDir[0],"%s",szInfo);
						log_print(LOGN_DEBUG,"LOAD DagBin DIR : %s",szDagBinDir );
					}
				}
				else if(strcmp(szType, "NET") == 0)
				{
					if(strcmp(szTmp, "ETH0") == 0)
					{
						if(strcmp(szInfo, "ENABLE") == 0)
							fidb->link[0]	= NORMAL;
						else
							fidb->link[0]	= NOT_EQUIP;
					}
					else if(strcmp(szTmp, "ETH1") == 0)
					{
						if(strcmp(szInfo, "ENABLE") == 0)
							fidb->link[1]	= NORMAL;
						else
							fidb->link[1]	= NOT_EQUIP;
					}
					else if(strcmp(szTmp, "ETH2") == 0)
					{
						if(strcmp(szInfo, "ENABLE") == 0)
							fidb->link[2]	= NORMAL;
						else
							fidb->link[2]	= NOT_EQUIP;
					}
					else if(strcmp(szTmp, "ETH3") == 0)
					{
						if(strcmp(szInfo, "ENABLE") == 0)
							fidb->link[3]	= NORMAL;
						else
							fidb->link[3]	= NOT_EQUIP;
					}

				}
			}
		}
	}
	fclose(fa);

	if(pstSolDfList->dCount != dDiskCnt)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: Disk Count[%d] is not match DISK NUM field[%d]", __FILE__, __FUNCTION__, __LINE__,
			dDiskCnt, pstSolDfList->dCount);

		return -2;
	}

	return dRet;
}

int dInit_Mirror_Timer(int *dTerm)
{
	FILE    *fa;
    char    szBuf[1024];
    char    szGubun[36];
    int     dValue;

    if( (fa=fopen(FILE_MIRROR_TIME, "r")) == NULL)
    {
        log_print(LOGN_DEBUG, "FILE OPEN ERROR : %s FILE NOT FOUND", FILE_MIRROR_TIME);
        return -1;
    }

    while(fgets(szBuf, 1024, fa) != NULL)
    {
        if(szBuf[0] != '#')
        {
            log_print(LOGN_DEBUG, "%s FILE FORMAT ERROR", FILE_MIRROR_TIME);
            fclose(fa);
            return -1;
        }

        if(szBuf[1] == '#')
            continue;
        else if(szBuf[1] == 'E')
            break;

        if( sscanf(&szBuf[2], "%s %d", szGubun, &dValue) == 2)
        {
            if(strcmp(szGubun, "MIRROR") == 0)
            {
				*dTerm = dValue;
                fclose(fa);
                return 0;
            }
        }
    } /* while-loop end*/

    fclose(fa);

    return 0;
}
