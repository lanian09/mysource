/*******************************************************************************
               DQMS Project

     Author   :
     Section  :
     SCCS ID  :
     Date     :
     Revision History :

     Description :

     Copyright (c) uPRESTO 2005
*******************************************************************************/
/** A.1* FILE INCLUDE *************************************/
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <errno.h>

/* User Define */
#include "loglib.h"
#include "path.h"

#include "chgsvc_mem.h"
#include "chgsvc_init.h"

/** B.1* DEFINITION OF NEW CONSTANTS **********************/
/** C.1* DEFINITION OF NEW TYPES **************************/
/** D.1* DECLARATION OF VARIABLES *************************/
int			gdFinishSignal = 0;
int			gdTAMID = -1;

/* Extern */
extern int	gdStopFlag;

/** E.1* DEFINITION OF FUNCTIONS **************************/
void		SetUpSignal(void);
int			dGetSYSID(void);

/** E.2* DEFINITION OF FUNCTIONS **************************/
/*******************************************************************************
  
*******************************************************************************/
int dInitProc(void)
{
	int dRet;

	/* Signal Init */
	SetUpSignal();

	/* Get TAM ID */
	dRet = dGetSYSID();
	if(dRet < 0) {
		log_print( LOGN_CRI, "[%s:%s][ERROR] dGetSYSID() Fail. RET[%d]", __FILE__, __FUNCTION__, dRet);
		return -1;
	}

	dRet = dInitMem();
	if(dRet < 0) {
		log_print(LOGN_CRI, "[%s.%d] [ERROR] dInitMem() Fail. RET[%d]", __FUNCTION__, __LINE__, dRet);
		return -2;
	}

	/*
	 * TODO : 
	 */

	return 1;
}


/*******************************************************************************
  
*******************************************************************************/
void UserControlledSignal(int isign)
{
	gdFinishSignal = isign;
	log_print(LOGN_CRI, "GET OS SIGNAL [%d]", isign);

	gdStopFlag = 0;

	return;
}

/*******************************************************************************
  
*******************************************************************************/
void FinishProgram(void)
{
	/* TODO 
	*/

	log_print(LOGN_CRI, "PROGRAM IS NORMALLY TERMINATED, Cause = %d", gdFinishSignal);
	log_print(LOGN_CRI, "BYE BYE!");
	log_print(LOGN_CRI, " ");
	log_print(LOGN_CRI, " ");

	return;
}

/*******************************************************************************
  
*******************************************************************************/
void IgnoreSignal(int isign)
{
    if (isign != SIGALRM) {
        log_print(LOGN_CRI, "IGNORE SIGNAL IS SKEEP, signal = %d", isign);
    }

    signal(isign, IgnoreSignal);

	return;
}

/*******************************************************************************
  
*******************************************************************************/
void SetUpSignal(void)
{

	/* WANTED SIGNALS */
	signal(SIGTERM, UserControlledSignal);
	signal(SIGINT, UserControlledSignal);
	signal(SIGQUIT, UserControlledSignal);

	/* UNWANTED SIGNALS */
	signal(SIGHUP, IgnoreSignal);
	signal(SIGALRM, IgnoreSignal);
	signal(SIGPIPE, IgnoreSignal);
	signal(SIGPOLL, IgnoreSignal);
	signal(SIGPROF, IgnoreSignal);
	signal(SIGUSR1, IgnoreSignal);
	signal(SIGUSR2, IgnoreSignal);
	signal(SIGVTALRM, IgnoreSignal);
	signal(SIGCLD, IgnoreSignal);

	return;
}

/*******************************************************************************
  
*******************************************************************************/
int dGetSYSID(void)
{
	int			i = 0;
	FILE		*fa;
	char		szBuf[1024], szType[64], szTmp[64], szInfo[64];

	if( (fa = fopen(FILE_SYS_CONFIG, "r")) == NULL) {
		log_print( LOGN_CRI, "[%s:%s][ERROR] fopen() Fail. FILE[%s] CAUSE[%s]", __FILE__, __FUNCTION__
				, FILE_SYS_CONFIG, strerror(errno));
		return -1;
	}

	while(fgets(szBuf, 1024, fa) != NULL) {
		i++;

		if(szBuf[0] != '#') {
			log_print( LOGN_CRI, "[%s:%s][ERROR] ROW FORMAT ERROR. FILE[%s] POSITION[%d"
					, __FILE__, __FUNCTION__, FILE_SYS_CONFIG, i);
			continue;
		}

		if(szBuf[1] == '#')
			continue;
		else if(szBuf[1] == 'E')
			break;      
		else if(szBuf[1] == '@') {
			if(sscanf(&szBuf[2], "%s %s %s", szType, szTmp, szInfo) == 3) {
				if(strcmp(szType, "SYS") == 0) {
					if(strcmp(szTmp, "TAMNO") == 0) {
						gdTAMID = atoi(szInfo);
						log_print( LOGN_DEBUG, "[%s:%s] GET TAMID[%d]", __FILE__, __FUNCTION__, gdTAMID);
						break;
					}
				}
			}
		}
	}

	if(gdTAMID < 0) {
		log_print( LOGN_CRI, "[%s:%s][ERROR] INVALID TAMID[%d]", __FILE__, __FUNCTION__, gdTAMID);
		fclose(fa);
		return -2;
	}

	fclose(fa);

	return 1;
}





