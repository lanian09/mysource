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

#include <sys/types.h>
#include <arpa/inet.h>

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stropts.h>
#include <string.h>
#include <fcntl.h>
#include <sys/errno.h>

/** B. DEFINITION OF NEW CONSTANTS ********************************************/

/** C. DEFINITION OF NEW TYPES ************************************************/

/** D. DECLARATION OF VARIABLES ***********************************************/
extern int      JiSTOPFlag;
extern int      FinishFlag;
extern int		gTRCDRcnt;

//extern  T_CAP   *shm_cap;

/** E.1 DEFINITION OF FUNCTIONS ***********************************************/
extern int log_write( char *fmt, ... );
extern int dAppLog(int dIndex, char *fmt, ...);

int      dLoad_PDSNIP();
int dAdd_PDSNIP(unsigned int uiPDSNIP);
int dDelete_PDSNIP(unsigned int uiPDSNIP);
void Log_PDSNIP(int level);


/** E.2 DEFINITION OF FUNCTIONS ***********************************************/

/*******************************************************************************

*******************************************************************************/
void FinishProgram()
{
    /*
    * SHM에 WRITE하는 부분에서 고려하지 않기 위해
    */
    //shm_cap->ReadPos[MRG_READER_ANA] = -2;

    dAppLog( LOG_CRI, "PROGRAM IS NORMALLY TERMINATED, Cause = %d", FinishFlag);
    exit(0);
}

/*******************************************************************************

*******************************************************************************/
void UserControlledSignal(int sign)
{
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
//    signal(SIGINT,  UserControlledSignal);
    signal(SIGINT,  IgnoreSignal);
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

	dAppLog(LOG_CRI, "SIGNAL HANDLER WAS INSTALLED");
}

#if 0
int dLoad_PDSNIP()
{

	FILE        *fa;
	char        szBuffer[1024];
	char        szIP[16];
	char        szValue[32];
	int         i = 0;
	UINT		uiIP;

	if( (fa=fopen("/BSD/NEW/DATA/PDSN.conf", "r")) == NULL) {
		dAppLog( LOG_CRI, "dLoad_PDSNIP: %s FILE NOT FOUND", "/BSD/NEW/DATA/PDSN.conf" );
		return -1;
	}		

	while( fgets( szBuffer, 1024, fa ) != NULL ) {
		if(szBuffer[0] == '@')
			continue;

		if( sscanf( &szBuffer[0], "%s %s", szIP, szValue ) == 2 ) {
			uiIP = inet_addr(szIP);
			radius_stat->uiPDSNAddr[i] = htonl(uiIP);

			radius_stat->uiCurCnt++;
			if( radius_stat->uiCurCnt >= DEF_PDSN_CNT ) //32
				return 0;
			i++;
		} else {
			dAppLog( LOG_CRI, "dLoad_PDSNIP: %s FILE FORMAT ERROR", "/BSD/NEW/DATA/PDSN.conf" );

			fclose(fa);
			return -1;
		}
	}

	return 0;
}
#endif

int dLoad_PDSNIP()
{

#if 0
	FILE        *fa;    
	char        szBuffer[1024];
	char        szIP[16];
	char        szValue[32];
	UINT        uiIP;   

	if( (fa=fopen("/DSC/NEW/DATA/PDSN.conf", "r")) == NULL) { 
		dAppLog( LOG_CRI, "dLoad_PDSNIP: %s FILE NOT FOUND", "/DSC/NEW/DATA/PDSN.conf" );
		return -1;
	}       

	fseek(fa, 0, SEEK_SET);
	while( fgets( szBuffer, 1024, fa ) != NULL ) {
		if(szBuffer[0] == '@') 
			continue;

		if( sscanf( &szBuffer[0], "%s %s", szIP, szValue ) == 2 ) {
			uiIP = ntohl(inet_addr(szIP));
			dAdd_PDSNIP(uiIP);
			if( radius_stat->uiCurCnt >= DEF_PDSN_CNT ) //32
				return 0;
		}
		else {
			dAppLog( LOG_CRI, "dLoad_PDSNIP: %s FILE FORMAT ERROR", "/DSC/NEW/DATA/PDSN.conf" );

			fclose(fa);
			return -1;
		}       
	}
	fclose(fa);
	Log_PDSNIP(LOG_INFO);

#if 0
    if( (fa=fopen("/BSD/NEW/DATA/PDSN.conf", "r")) == NULL) { 
        dAppLog( LOG_CRI, "dLoad_PDSNIP: %s FILE NOT FOUND", "/BSD/NEW/DATA/PDSN.conf" );
        return -1;
    }       

	fseek(fa, 0, SEEK_SET);
    while( fgets( szBuffer, 1024, fa ) != NULL ) {
        if(szBuffer[0] == '@') 
            continue;

        if( sscanf( &szBuffer[0], "%s %s", szIP, szValue ) == 2 ) {
            uiIP = ntohl(inet_addr(szIP));
            dDelete_PDSNIP(uiIP);
            if( radius_stat->uiCurCnt >= DEF_PDSN_CNT ) //32
                return 0;
        }       
        else {  
            dAppLog( LOG_CRI, "dLoad_PDSNIP: %s FILE FORMAT ERROR", "/BSD/NEW/DATA/PDSN.conf" );

            fclose(fa);
            return -1;
        }       
    }
	fclose(fa);
#endif
#endif

	return 0;
}




int dAdd_PDSNIP(unsigned int uiPDSNIP)
{
#if 0
	int i, j;

	for( i = 0; i < DEF_PDSN_CNT; i++ ) { 
		/* Shared Memory에 존재하는 PDSN IP인지 확인 */
		if( radius_stat->uiPDSNAddr[i] == uiPDSNIP )
			return -1;  
	}       

	/* Shared Memory에 없는 PDSN IP를 추가해 준다. */
	radius_stat->uiPDSNAddr[radius_stat->uiCurCnt] = uiPDSNIP; 
	for( j = 0; j < MAX_MP_NUM1; j++ )
		memset(&radius_stat->rad_stat[j][radius_stat->uiCurCnt], 0, sizeof(st_RadiusStat_List));
	radius_stat->uiCurCnt++;

	//Log_PDSNIP(LOG_CRI);
#endif
	return 0;
}

int dDelete_PDSNIP(unsigned int uiPDSNIP)
{
#if 0
	int i, j, idx;

	for( i = 0; i < DEF_PDSN_CNT; i++ ) { 
		/* Shared Memory에 존재하는 PDSN IP인지 확인 */
		if( radius_stat->uiPDSNAddr[i] == uiPDSNIP )
			break;
	}       

	if( i == DEF_PDSN_CNT ) {
		/* PDSN IP가 존재하지 않는다 */
		return -1;
	}

	idx = i;
	for( i = idx; i < radius_stat->uiCurCnt-1; i++ ) {
		radius_stat->uiPDSNAddr[i] = radius_stat->uiPDSNAddr[i+1];
		for( j = 0; j < MAX_MP_NUM1; j++ )
			radius_stat->rad_stat[j][i] = radius_stat->rad_stat[j][i+1];
	}
	radius_stat->uiPDSNAddr[radius_stat->uiCurCnt-1] = 0;
	for( j = 0; j < MAX_MP_NUM1; j++ )
		memset(&radius_stat->rad_stat[j][radius_stat->uiCurCnt-1], 0, sizeof(st_RadiusStat_List));
	radius_stat->uiCurCnt--;

	//Log_PDSNIP(LOG_CRI);
#endif
	return 0;
}

void Log_PDSNIP(int level)
{
#if 0
	int i;
	struct in_addr in;

#if 1
	dAppLog(level, "##### RAD STAT ####");
	dAppLog(level, "COUNT = [%u]", radius_stat->uiCurCnt);
	for( i = 0; i < radius_stat->uiCurCnt; i++ ) {
		in.s_addr = htonl(radius_stat->uiPDSNAddr[i]);
		dAppLog(level, "IDX[%d]: PDSN=[%s]", i, inet_ntoa(in));
	}
#endif
#if 0
	fprintf(stderr, "##### RAD STAT ####\n");
	fprintf(stderr, "COUNT = [%u]\n", radius_stat->uiCurCnt);
	for( i = 0; i < radius_stat->uiCurCnt; i++ ) {
		in.s_addr = htonl(radius_stat->uiPDSNAddr[i]);
		fprintf(stderr, "IDX[%d]: PDSN=[%s]\n", i, inet_ntoa(in));
	}
#endif
#endif
}

