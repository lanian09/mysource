/**A.1*  File Inclusion *******************************************************/

/* SYS HEADER */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
#include <errno.h>
/* LIB HEADER */
#include "filedb.h"
#include "ipclib.h"
#include "loglib.h"
#include "mems.h"
#include "cifo.h"
#include "nifo.h"
#include "gifo.h"
/* PRO HEADER */
#include "path.h"
#include "sshmid.h"
#include "procid.h"
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
#include "almstat.h"
/* LOC HEADER */
#include "almd_init.h"

/**B.1*  Definition of New Constants ******************************************/

/**C.1*  Declaration of Variables  ********************************************/
pst_WNTAM     stWNTAM;
pst_GEN_INFO  gpGenInfo;
pst_NTAM       fidb;

extern stMEMSINFO 	*gpMEMSINFO;
extern stCIFO		*gpCIFO;
extern pst_keepalive_taf *keepalive;

/**D.1*	 Declaration of Function  *********************************************/

/******************************************************************************
 * 
******************************************************************************/

int dInitALMD(void)
{
	int     dRet;

	gpMEMSINFO = nifo_init_zone((U8*)"ALMD", SEQ_PROC_ALMD, FILE_NIFO_ZONE);
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

	if( (dRet = shm_init(S_SSHM_KEEPALIVE, DEF_KEEPALIVE_TAF_SIZE, (void**)&keepalive)) < 0 ){
        log_print(LOGN_CRI,LH"FAILED IN shm_init(S_SSHM_KEEPALIVE[0x%04X], dRet=%d)"EH,
            LT,S_SSHM_KEEPALIVE,dRet,ET);
        return -3;
    }

	if( (dRet = shm_init(S_SSHM_FIDB, DEF_WNTAM_SIZE, (void**)&stWNTAM)) < 0 ){
		log_print(LOGN_CRI,LH"FAILED IN shm_init(S_SSHM_FIDB[0x%04X], dRet=%d)"EH,
				LT,S_SSHM_FIDB,dRet,ET);
		return -6;
	}
	fidb = &stWNTAM->stNTAM;

	if( (dRet = shm_init(S_SSHM_GENINFO, sizeof(st_GEN_INFO), (void**)&gpGenInfo)) < 0 ){
		log_print(LOGN_CRI,LH"FAILED IN shm_init(S_SSHM_GENINFO[0x%04X], dRet=%d)"EH,
				LT,S_SSHM_GENINFO,dRet,ET);
		return -6;
	}

	return 1;
}

void FinishProgram(int sign)
{
	log_print(LOGN_CRI, "PROGRAM IS NORMALLY TERMINATED, Cause = %d", sign);
	exit(0);
}

void UserControlledSignal(int sign)
{
	log_print(LOGN_CRI, "UserControlledSignal : [%d]", sign);
	FinishProgram(sign);
}

void IgnoreSignal(int sign)
{
	if(sign != SIGALRM)
		log_print(LOGN_WARN, "UNWANTED SIGNAL IS RECEIVED, signal = %d", sign);

	signal(sign, IgnoreSignal);
}

unsigned char ucGetSysNo()
{
    FILE *fa;
    char szBuf[1024];
    char szType[64];
    char szTmp[64];
    char szInfo[64];
    int i;
    int dRet;

    dRet = -1;
    fa = fopen(FILE_SYS_CONFIG,"r");
    if( fa == NULL ){

        log_print(LOGN_CRI,"LOAD SYSTEM CONFIG : %s FILE OPEN FAIL (%s)",
        FILE_SYS_CONFIG, strerror(errno) );
        return -1;
    }

    i = 0;

    while( fgets(szBuf, 1024, fa) != NULL ){

        if( szBuf[0] != '#' ){

            log_print(LOGN_WARN,"FAILED IN ucGetSysNo() : %s File [%d] row format error",
            FILE_SYS_CONFIG, i );
        }

        i++;

        if( szBuf[1] == '#' )
            continue;
        else if( szBuf[1] == 'E' )
            break;
        else if( szBuf[1] == '@' ){

            if( sscanf( &szBuf[2], "%s %s %s", szType, szTmp, szInfo ) == 3 ){

                if( strcmp( szType, "SYS" ) == 0 ){

                    if( strcmp( szTmp, "NO" ) == 0 ){

                        dRet = atoi(szInfo);
                        log_print(LOGN_DEBUG,"LOAD SYSNO : [ %d ]", dRet );
                        break;
                    }
                }
            }
        }
    }/* while */

	fclose(fa);

	return (unsigned char)dRet;

}

/*******************************************************************************

	Revision History :

		$Log: almd_init.c,v $
		Revision 1.5  2011/09/07 05:28:17  uamyd
		modified
		
		Revision 1.4  2011/09/07 04:59:33  uamyd
		modified
		
		Revision 1.3  2011/09/05 04:44:39  dcham
		*** empty log message ***
		
		Revision 1.2  2011/09/01 07:17:08  hhbaek
		*** empty log message ***
		
		Revision 1.1  2011/08/29 09:55:32  dcham
		*** empty log message ***
		
		Revision 1.5  2011/08/22 00:15:28  dcham
		*** empty log message ***
		
		Revision 1.4  2011/08/20 07:39:30  dcham
		*** empty log message ***
		
		Revision 1.3  2011/08/09 07:15:25  dcham
		*** empty log message ***
		
		Revision 1.2  2011/08/03 08:19:14  dcham
		*** empty log message ***
		
		Revision 1.1  2011/08/03 00:23:52  dcham
		*** empty log message ***
		
		Revision 1.5  2011/01/11 04:09:05  uamyd
		modified
		
		Revision 1.1.1.1  2010/08/23 01:13:02  uamyd
		DQMS With TOTMON, 2nd-import
		
		Revision 1.4  2009/09/13 11:01:25  pkg
		NIFO 감시 추가
		
		Revision 1.3  2009/06/25 09:03:31  hjpark
		no message

		Revision 1.2  2009/05/27 14:24:48  dqms
		*** empty log message ***

		Revision 1.1.1.1  2009/05/26 02:14:16  dqms
		Init TAF_RPPI

		Revision 1.4  2009-05-21 07:36:54  astone
		no message

		Revision 1.3  2009-05-21 06:47:12  astone
		no message

		Revision 1.2  2009-05-21 06:44:57  hjpark
		no message

		Revision 1.1.1.1  2009-05-19 10:34:23  hjpark
		DQMS
			-Writer: Han-jin Park
			-Date: 2009.05.19

		Revision 1.3  2008/05/06 06:41:36  uamyd
		20080506
		인수시험
		적용소스

		Revision 1.2  2008/01/03 06:55:45  dark264sh
		Makefile 수정 - 옵션 변경 (-g3 -Wall), warning 수정, log_print Log Level 수정

		Revision 1.1.1.1  2007/12/27 09:04:48  leich
		WNTAS_ADD

		Revision 1.1.1.1  2007/10/31 05:12:12  uamyd
		WNTAS so lated initialized

		Revision 1.1  2002/01/30 18:43:15  swdev4
		Initial revision

*******************************************************************************/
