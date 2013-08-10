/*******************************************************************************
                 ABLEX IPAS Project (IPAF BLOCK)

   Author   :
   Section  : IPAS(IPAF) Project
   SCCS ID  : @(#)ana_init.c (V1.0)
   Date     : 3//03
   Revision History :
        '04.    03. 05. initial

   Description:

   Copyright (c) ABLEX 2004
*******************************************************************************/

/** A. FILE INCLUSION *********************************************************/
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
#include <sys/msg.h>
#include <sys/ipc.h>
#include <unistd.h>

#include <ipaf_shm.h>
#include <ipaf_define.h>
#include <ipaf_error.h>
#include <define.h>
#include <init_shm.h>
#include <ipaf_stat.h>
#include "hasho.h"
#include "hash_pdsn.h"
#include "comm_msgtypes.h"
#include "common_ana.h"
#include "conflib.h"

/** B. DEFINITION OF NEW CONSTANTS ********************************************/

/** C. DEFINITION OF NEW TYPES ************************************************/

/** D. DECLARATION OF VARIABLES ***********************************************/
extern int      	JiSTOPFlag;
extern int      	FinishFlag;

extern PDSN_LIST        	*gpPdsnList[DEF_SET_CNT];
extern PDSN_LIST        	*gpCurPdsn; // CURRENT OPERATION pointer
// PDSN HASH
extern stHASHOINFO         *gpPdsnHash[DEF_SET_CNT];
extern stHASHOINFO         *gpCurPdsnHash; // CURRENT OPERATION pointer

extern st_NOTI				*gpIdx;


/** E.1 DEFINITION OF FUNCTIONS ***********************************************/
int 	dReadFLTIDXFile(void);
int 	InitSHM_PDSN_HASH(void);
int 	InitSHM_PDSN_LIST(void);
void 	dSetCurPdsn(NOTIFY_SIG *pNOTISIG);
extern int log_write( char *fmt, ... );
extern int dAppLog(int dIndex, char *fmt, ...);

/** E.2 DEFINITION OF FUNCTIONS ***********************************************/

/*******************************************************************************

*******************************************************************************/
void FinishProgram()
{
    /*
    * SHM에 WRITE하는 부분에서 고려하지 않기 위해
    *
    shm_cap->ReadPos[MRG_READER_ANA] = -2;
	*/

    dAppLog( LOG_CRI, "PROGRAM IS NORMALLY TERMINATED, Cause = %d", FinishFlag);
    exit(0);
}

/*******************************************************************************

*******************************************************************************/
void UserControlledSignal(int sign)
{
	dAppLog(LOG_CRI, "DEFINED SIGNAL IS RECEIVED, signal = %d", sign);
    JiSTOPFlag = 0;
    FinishFlag = sign;

    FinishProgram();
}


/*******************************************************************************

*******************************************************************************/
void IgnoreSignal(int sign)
{
    if (sign != SIGALRM)
        dAppLog( LOG_CRI, "UNWANTED SIGNAL IS RECEIVED, signal = %d", sign);
    signal(sign, IgnoreSignal);
}

/*******************************************************************************

*******************************************************************************/
void SetUpSignal()
{
    JiSTOPFlag = 1;

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
	dAppLog(LOG_DEBUG, "SIGNAL HANDLER WAS INSTALLED");
}

int dReadFLTIDXFile(void)
{
	FILE *fa;
	char szBuf[1024];
	char szType[64];
	int i = 0, dIdx = 0;

	fa = fopen(DEF_NOTI_INDEX_FILE, "r");
	if(fa == NULL)
	{
		dAppLog(LOG_CRI,"dReadFLTIDXFile : %s FILE OPEN FAIL (%s)",
		DEF_NOTI_INDEX_FILE, strerror(errno));
		return -1;
	}

	while(fgets(szBuf,1024,fa) != NULL)
	{
		if(szBuf[0] != '#')
		{
			dAppLog(LOG_CRI,"dReadFLTIDXFile : %s File [%d] row format error",
			DEF_NOTI_INDEX_FILE, i);
			fclose(fa);
			return -1;
		}

		if(szBuf[1] == '#')
			continue;
		else if(szBuf[1] == 'E')
			break;
		else if(szBuf[1] == '@')
		{

			if(sscanf(&szBuf[2],"%s %d",szType, &dIdx) == 2)
			{
				if( strcmp(szType, "PDSN") == 0 )
				{
					gpIdx->dPdsnIdx = dIdx;
					gpCurPdsn = (dIdx == 0) ? gpPdsnList[0] : gpPdsnList[1];
					gpCurPdsnHash = (dIdx == 0) ? gpPdsnHash[0] : gpPdsnHash[1];
					dAppLog(LOG_CRI, "PDSN READ IDX[%d]", dIdx);
				}
			}
		}
		dIdx = 0; i++;
	}

	fclose(fa);

	return i;
} 

int InitSHM_PDSN_LIST(void)
{
	int 	dRet;
	int  	key;
	char 	tmp[64], fname[256];

	sprintf(fname,"%s", DEF_SYSCONF_FILE );

	if (conflib_getNthTokenInFileSection (fname, "SHARED_MEMORY_KEY", "SHM_PDSN_LIST", 1, tmp) < 0)
		return -1;

	key = strtol(tmp,0,0);
	dRet = Init_shm(key, sizeof(PDSN_LIST), (void **)&gpPdsnList[0]);
	if( dRet < 0 )
	{
		dAppLog( LOG_CRI, "FAIL %s(0) dRet=%d", __FUNCTION__, dRet);
		return -1;
	}

	if (conflib_getNthTokenInFileSection (fname, "SHARED_MEMORY_KEY", "SHM_PDSN_LIST1", 1, tmp) < 0)
		return -1;

	key = strtol(tmp,0,0);
	dRet = Init_shm(key, sizeof(PDSN_LIST), (void **)&gpPdsnList[1]);
	if( dRet < 0 )
	{
		dAppLog( LOG_CRI, "FAIL %s(1) dRet=%d", __FUNCTION__, dRet);
		return -1;
	}
	return 0;
}

int InitSHM_PDSN_HASH(void)
{
	int  	key;
	char 	tmp[64], fname[256];

	sprintf(fname,"%s", DEF_SYSCONF_FILE );

	if (conflib_getNthTokenInFileSection (fname, "SHARED_MEMORY_KEY", "SHM_PDSN_HASH", 1, tmp) < 0)
		return -1;

	key = strtol(tmp,0,0);
	if((gpPdsnHash[0] = hasho_init(key, DEF_PDSN_HASH_KEY_SIZE, DEF_PDSN_HASH_KEY_SIZE, DEF_PDSN_HASH_DATA_SIZE, MAX_HASH_PDSN_CNT, NULL, 0)) == NULL) 
	{
		dAppLog(LOG_CRI, "PDSN HASH hasho_init() NULL");
		return -1;
	}

	if (conflib_getNthTokenInFileSection (fname, "SHARED_MEMORY_KEY", "SHM_PDSN_HASH1", 1, tmp) < 0)
		return -1;

	key = strtol(tmp,0,0);
	if((gpPdsnHash[1] = hasho_init(key, DEF_PDSN_HASH_KEY_SIZE, DEF_PDSN_HASH_KEY_SIZE, DEF_PDSN_HASH_DATA_SIZE, MAX_HASH_PDSN_CNT, NULL, 0)) == NULL) 
	{
		dAppLog(LOG_CRI, "PDSN HASH hasho_init() NULL");
		return -1;
	}

	return 0;
}

void dSetCurPdsn(NOTIFY_SIG *pNOTISIG)
{
	if( pNOTISIG->stNoti.dPdsnIdx < 0 || pNOTISIG->stNoti.dPdsnIdx >= DEF_SET_CNT )
	{
		dAppLog(LOG_CRI, "[%s.%d] Current Index Range ERROR dPdsnIdx[%d]", __FUNCTION__, __LINE__, pNOTISIG->stNoti.dPdsnIdx);
		return;
		//gpCurPdsn = NULL; gpCurPdsnHash = NULL;
	}

	gpIdx->dPdsnIdx = pNOTISIG->stNoti.dPdsnIdx;
	gpCurPdsn       = gpPdsnList[gpIdx->dPdsnIdx];
	gpCurPdsnHash   = gpPdsnHash[gpIdx->dPdsnIdx];

	dAppLog(LOG_CRI, "NOTI] PDSN ACTIVE IDX[%d]", pNOTISIG->stNoti.dPdsnIdx);
}

