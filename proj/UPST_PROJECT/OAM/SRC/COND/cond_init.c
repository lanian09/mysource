/***** A.1 * File Include *******************************/

/* SYS HEADER */
#include <stdio.h>		/* FILE */
#include <string.h>		/* strerror(), memset(), strcpy() */
#include <signal.h>
#include <stdlib.h>		/* EXIT(3) */
#include <unistd.h>		/* CLOSE(2) */
/* LIB HEADER */
#include "mems.h"		/* stMEMSINFO */
#include "cifo.h"		/* stCIFO */
#include "gifo.h"		/* gifo_init_group() */
#include "nifo.h"		/* nifo_init_zone() */
#include "commdef.h" 	/* PROC_NAME_LEN */
#include "filedb.h"		/* DEF_FIDB_SIZE */
#include "loglib.h"
#include "dblib.h"
#include "ipclib.h"
/* PRO HEADER */
#include "msgdef.h"
#include "path.h"
#include "procid.h"
#include "sshmid.h"
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
/* LOC HEADER */
#include "cond_init.h"

/** B.1* DEFINITION OF NEW CONSTANTS **********************/
/** C.1* DEFINITION OF NEW TYPES **************************/
/** D.1* DECLARATION OF VARIABLES *************************/
pst_NTAM  		   fidb;
pst_WNTAM 		   stWNTAM;
int				   Finishflag;

/** D.2* DECLARATION OF EXTERN VARIABLES *************************/
extern stMEMSINFO *gpMEMSINFO;
extern stCIFO     *gpCIFO;
extern int		   gdSvrSfd;
extern int		   gdStopFlag;
extern MYSQL	   stMySQL;

/** E.1* DEFINITION OF FUNCTIONS **************************/
/** E.2* DEFINITION OF EXTERN FUNCTIONS **************************/
int Init_Fidb()
{
	int    dRet, i;
	time_t now;
	
	if( (dRet = shm_init(S_SSHM_FIDB, DEF_WNTAM_SIZE, (void**)&stWNTAM)) < 0 ){
		log_print(LOGN_CRI, LH"FAILED IN shm_init(S_SSHM_FIDB=0x%x], dRet=%d)"EH,
			LT, S_SSHM_FIDB, dRet, ET);
		return -1;
	}

	fidb = &stWNTAM->stNTAM;

	if( dRet != SHM_EXIST ){
		time(&now);
		
		for(i = 0; i < 256; i++ ){
			fidb->tEventUpTime[i] = now;
		}

		log_print(LOGN_CRI, LH"Initialized a FIDB tEventUpTime=%ld", LT, now);
	}

	return 0;
}

int dGetBlocks(char *fn, char (*p)[30])
{
	int		ln, rdcnt, scan_cnt;
	char	buf[256], Bname[PROC_NAME_LEN];
	FILE	*fp;

	if( (fp = fopen(fn, "r")) == NULL)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN fopen(%s) [errno=%d-%s]", __FILE__, __FUNCTION__, __LINE__, fn, errno, strerror(errno));
		return -1;	/*	fopen error	*/
	}

	ln		= 0;
	rdcnt	= 0;
	while(fgets(buf, 256, fp) != NULL)
	{
		ln++;
		if(buf[0] != '#')
		{
			log_print(LOGN_CRI, "SYNTAX ERROR FILE:%s, LINE:%d - FIRST CHARACTER IS MUST '#'!", fn, ln);
			return -1;
		}
		else if(buf[1] == '#')
			continue;
		else if(buf[1] == 'E')
			break;
		else if(buf[1] == '@')
		{
			if( (scan_cnt= sscanf(&buf[2], "%s %*s", Bname)) != 1)
				sprintf(Bname, " - ");
			strcpy(*(p+rdcnt), Bname);
			rdcnt++;
		}
		else
		{
			log_print(LOGN_CRI, "SYNTAX ERROR FILE:%s, LINK:%d", fn, ln);
			return -2;
		}
	}
	fclose(fp);

	return rdcnt;
}

int dInitIPCs(void)
{
	gpMEMSINFO = nifo_init_zone((U8*)"COND", SEQ_PROC_COND, FILE_NIFO_ZONE);
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

void SetUpSignal(void)
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
	signal(SIGUSR1, IgnoreSignal);
	signal(SIGUSR2, IgnoreSignal);
	signal(SIGVTALRM, IgnoreSignal);
	signal(SIGCLD, SIG_IGN);
}

void UserControlledSignal(int sign)
{
	if( gdSvrSfd > 0 )
		close(gdSvrSfd);
	gdStopFlag = 0;
	Finishflag = sign;
}

void FinishProgram(void)
{
	if(gdSvrSfd > 0)
		close(gdSvrSfd);
	gdStopFlag	= 0;
	log_print(LOGN_CRI, "PROGRAM IS NORMALLY TERMINATED, Cause = %d, SIGN=%d", gdStopFlag, Finishflag);
	db_disconn(&stMySQL);

	exit(0);
}

void IgnoreSignal(int sign)
{
    if (sign != SIGALRM)
        log_print(LOGN_WARN,"UNWANTED SIGNAL IS RECEIVED, signal = %d", sign);
    signal(sign, IgnoreSignal);
}

int dGetHostName(char *szNTAMName)
{
	int		FindFlag = 0;
    FILE	*fptr;
    char	szBuf[1024], szHostName[16], szNTAM[64];

    fptr = fopen( FILE_SYS_CONFIG, "r");
    if( fptr == NULL ) {
        log_print(LOGN_CRI, "[FAIL] FILE_SYS_CONFIG=%s] OPEN : %s",
			FILE_SYS_CONFIG, strerror(errno));
        return -1;
    }

    while( fgets(szBuf, 1024, fptr) != NULL )
    {
        if( szBuf[0] != '#' ){
			log_print(LOGN_CRI,"[SYNTAX ERROR]");
            break;
	    }

        if( szBuf[1] == '#' )
            continue;
        else if( szBuf[1] == 'E' )
            break;
        else {
            memset( &szNTAM[0], 0x00, sizeof(szNTAM) );
            memset( &szHostName[0], 0x00, sizeof(szHostName) );
            if( sscanf (&szBuf[2], "%s %s", &szHostName[0], &szNTAM[0]) == 2 ) {
                if( strcmp( &szHostName[0], "LOC-SYS" ) == 0 ) {
					FindFlag = 1;
                    break;
                }
            }
        }
    }

    fclose(fptr);

    if( FindFlag > 0 ) {
        strcpy(szNTAMName, szNTAM);
		log_print(LOGN_CRI, "SYSTEM NAME = %s", szNTAMName);
        return 0;
    }

    log_print(LOGN_CRI, "[FAIL] NOT FOUND LOC-SYSTEM ");
    return -1;
}

int dGetSysNo(void)
{
    int 	i, dRet;
	FILE 	*fa;
    char 	szBuf[1024], szType[64], szTmp[64], szInfo[64];

    dRet = -1;
    fa = fopen(FILE_SYS_CONFIG,"r");
    if( fa == NULL ) {

        log_print(LOGN_CRI,"dGetSysNo: %s FILE OPEN FAIL (%s)",
        	FILE_SYS_CONFIG, strerror(errno) );
        return -1;
    }

    i = 0;
    while( fgets(szBuf, 1024, fa) != NULL ) {
        if( szBuf[0] != '#' ){
            log_print(LOGN_WARN,"FAILED IN dGetSysNo() : %s File [%d] row format error",
            	FILE_SYS_CONFIG, i );
        }

        i++;
        if( szBuf[1] == '#' )
            continue;
        else if( szBuf[1] == 'E' )
            break;
        else if( szBuf[1] == '@' ) {
            if( sscanf( &szBuf[2], "%s %s %s", szType, szTmp, szInfo ) == 3 ) {
                if( strcmp( szType, "SYS" ) == 0 ) {
                    if( strcmp( szTmp, "NO" ) == 0 ) {
                        dRet = atoi(szInfo);
                        log_print(LOGN_CRI, "SYSTEM LOAD SYSNO : [ %d ]", dRet );
                        break;
                    }
                }
            }
        }
    }/* while */

	fclose(fa);

	return dRet;
}
