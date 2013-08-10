/***** A.1 * File Include *******************************/
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "loglib.h"
#include "ipclib.h"
#include "mems.h"
#include "cifo.h"
#include "nifo.h"
#include "gifo.h"

#include "sshmid.h"
#include "procid.h"
#include "path.h"
#include "filedb.h"
#include "almstat.h"

#include "watch_mon.h"

#include "mond_init.h"

extern stMEMSINFO	*gpMEMSINFO;
extern stCIFO		*gpCIFO;
extern int 			gdSvrSfd;
extern int			gdStopFlag;

pst_WNTAM         	fidb;
pst_NTAF_List_SHM 	gpNtafSHM_db;
pst_MonTotal 		gpMonTotal;
pst_keepalive   	gpKeepAlive;


int init_proc(void)
{
	if((gpMEMSINFO = nifo_init_zone((U8*)"MOND", SEQ_PROC_MOND, FILE_NIFO_ZONE)) == NULL) {
        log_print(LOGN_CRI, LH"FAILED IN nifo_init_zone NULL", LT);
        return -1;
    }

	if((gpCIFO = gifo_init_group(FILE_CIFO_CONF, FILE_GIFO_CONF)) == NULL ){
        log_print(LOGN_CRI, LH"FAILED IN gifo_init_group, cifo=%s, gifo=%s",
            LT, FILE_CIFO_CONF, FILE_GIFO_CONF);
        return -2;
    }
	return 0;
}


void SetUpSignal()
{
    /*
	* WANTED SIGNALS
	*/
    signal(SIGTERM, UserControlledSignal);
    signal(SIGINT,  UserControlledSignal);
    signal(SIGQUIT, UserControlledSignal);

    /*
	* UNWANTED SIGNALS
	*/
    signal(SIGHUP,  IgnoreSignal);
    signal(SIGALRM, IgnoreSignal);
    signal(SIGPIPE, IgnoreSignal);
    signal(SIGPOLL, IgnoreSignal);
    signal(SIGPROF, IgnoreSignal);
    signal(SIGUSR1, IgnoreSignal);
    signal(SIGUSR2, IgnoreSignal);
    signal(SIGVTALRM, IgnoreSignal);
    signal(SIGCLD, SIG_IGN);

    return;
}

void UserControlledSignal(int sign)
{
	if( gdSvrSfd > 0 )
		close(gdSvrSfd);

	gdStopFlag = 0;
}


void FinishProgram()
{
	if( gdSvrSfd > 0 )
		close(gdSvrSfd);

	gdStopFlag = 0;
    log_print(LOGN_CRI, "PROGRAM IS NORMALLY TERMINATED, Cause = %d", gdStopFlag);
}

void IgnoreSignal(int sign)
{
    if (sign != SIGALRM)
        log_print(LOGN_CRI, "UNWANTED SIGNAL IS RECEIVED, signal = %d", sign);

    signal(sign, IgnoreSignal);
}

int Load_KeepAlive()
{
	FILE        *fp_ipam;
    int         dRet;
    char        szBuffer[512];
    char        szName[128];
    int         dCri, dMaj, dMin;

/*** noted by uamyd0626 06.06.16 for w-ntas : DON'T USED MORE ****
*    더이상 NTAF를 관리하지 않음.                                *

	dRet = dReadNTAFAlm();
    if( dRet < 0 ) {
        log_print( LOGN_CRI, "[ERROR] READ NTAF ALARM LEVEL");
        return -1;
    }
******************************************************************/

    if( (fp_ipam = fopen(FILE_TAM_LOAD_DATA, "r")) == NULL ) {
        log_print( LOGN_DEBUG, "[ERROR] NTAM LOAD ALARM FILE OPEN [%s]", FILE_TAM_LOAD_DATA );
        return -1;
    }

    while( fgets( szBuffer, 512, fp_ipam ) != NULL ) {
        if( szBuffer[0] != '#' ) {
            dRet = -1;
            break;
        }

        if(szBuffer[1] == '#')
            continue;
        else if(szBuffer[1] == 'E')
            break;
        else {
            sscanf( &szBuffer[2], "%s %d %d %d", szName, &dCri, &dMaj, &dMin );

            if( !strcmp( szName, "CPU" ) ) {
                gpKeepAlive->stTAMLoad.cpu.usMinor = dMin;
                gpKeepAlive->stTAMLoad.cpu.usMajor = dMaj;
                gpKeepAlive->stTAMLoad.cpu.usCritical = dCri;
            }
            else if( !strcmp( szName, "MEM" ) ) {
                gpKeepAlive->stTAMLoad.mem.usMinor = dMin;
                gpKeepAlive->stTAMLoad.mem.usMajor = dMaj;
                gpKeepAlive->stTAMLoad.mem.usCritical = dCri;
            }
			else if( !strcmp( szName, "DISK" ) ) {
                gpKeepAlive->stTAMLoad.disk.usMinor = dMin;
                gpKeepAlive->stTAMLoad.disk.usMajor = dMaj;
                gpKeepAlive->stTAMLoad.disk.usCritical = dCri;
            }
            else if( !strcmp( szName, "QUE" ) ) {
                gpKeepAlive->stTAMLoad.que.usMinor = dMin;
                gpKeepAlive->stTAMLoad.que.usMajor = dMaj;
                gpKeepAlive->stTAMLoad.que.usCritical = dCri;
            }
        }
    }

    fclose( fp_ipam );

    return 1;
}

int dGetSYSCFG()
{
	FILE *fa;
    char szBuf[1024];
    char szType[64];
    char szTmp[64];
    char szInfo[64];
    int i;
    int dRet;

    dRet = 0;
    fa = fopen(FILE_SYS_CONFIG,"r");
    if( fa == NULL ){

        log_print(LOGN_CRI,"dGetSYSCFG: %s FILE OPEN FAIL (%s)",
        FILE_SYS_CONFIG, strerror(errno) );
        return -1;
    }

    i = 0;

    while( fgets(szBuf, 1024, fa) != NULL ){

        if( szBuf[0] != '#' ){

            log_print(LOGN_WARN,"FAILED IN dGetSYSCFG() : %s File [%d] row format error",
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

	return dRet;

}

int Init_Fidb(void)
{
	int     dRet;
	if( (dRet = shm_init(S_SSHM_FIDB, DEF_WNTAM_SIZE, (void**)&fidb)) < 0 ){
		log_print(LOGN_CRI,LH"FAILED IN shm_init(S_SSHM_FIDB[0x%04X], dRet=%d), errno=%d:%s",
				LT,S_SSHM_FIDB,dRet,errno,strerror(errno));
		return -6;
	}
	return 1;
}

int Init_SubFidb(void)
{
	int     dRet,i,j;
	time_t	nowt;
	if( (dRet = shm_init(S_SSHM_TAF_FIDB, DEF_TAF_LIST_SIZE, (void**)&gpNtafSHM_db)) < 0 ){
		log_print(LOGN_CRI,LH"FAILED IN shm_init(S_SSHM_FIDB[0x%04X], dRet=%d), errno=%d:%s",
				LT,S_SSHM_TAF_FIDB,dRet,errno,strerror(errno));
		return -6;
	}
	memset(gpNtafSHM_db, 0x00, sizeof(st_NTAF_List_SHM));
	time(&nowt);
	for(i = 0; i < MAX_NTAF_COUNT; i++)
	{
		gpNtafSHM_db->stNTAF[i].tUpTime	= nowt;
		for(j = 0; j < MAX_NTAF_SW_BLOCK; j++)
			gpNtafSHM_db->stNTAF[i].mpswinfo[j].when = nowt;
	}
	log_print(LOGN_DEBUG, LH"st_NTAF_List_SHM SIZE: %ld bytes", LT, DEF_TAF_LIST_SIZE);
	return 1;
}

int Init_LoadStat(void)
{
	int     dRet;
	if( (dRet = shm_init(S_SSHM_LOADSTAT, DEF_KEEPALIVE_SIZE, (void**)&gpKeepAlive)) < 0 ){
		log_print(LOGN_CRI,LH"FAILED IN shm_init(S_SSHM_KEEPALIVE[0x%04X], dRet=%d), errno=%d:%s",
				LT,S_SSHM_LOADSTAT,dRet,errno,strerror(errno));
		return -6;
	}
	return 0;
}

int Init_Keepalive(void)
{
	int     dRet;
	if( (dRet = shm_init(S_SSHM_KEEPALIVE, DEF_KEEPALIVE_SIZE, (void**)&gpKeepAlive)) < 0 ){
		log_print(LOGN_CRI,LH"FAILED IN shm_init(S_SSHM_KEEPALIVE[0x%04X], dRet=%d), errno=%d:%s",
				LT,S_SSHM_KEEPALIVE,dRet,errno,strerror(errno));
		return -6;
	}
	return 0;
}

int InitMonTotalShm(void)
{

	int     dRet;
	if( (dRet = shm_init(S_SSHM_MON_TOTAL, DEF_MONTOTAL_SIZE, (void**)&gpMonTotal)) < 0 ){
		log_print(LOGN_CRI,LH"FAILED IN shm_init(S_SSHM_KEEPALIVE[0x%04X], dRet=%d), errno=%d:%s",
				LT,S_SSHM_MON_TOTAL,dRet,errno,strerror(errno));
		return -6;
	}
	return 0;
}
