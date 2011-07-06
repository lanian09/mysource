
/*******************************************************************************
							ABLEX IPAS Project

	Author	:	tundra
	Section	:	IPAS (NTAF)	CHSMD

	Revision History	:
 		$Log: chsmd_main.c,v $
 		Revision 1.1  2002/01/30 17:20:31  swdev4
 		Initial revision

	Copyright (c) ABLEX 2002
*******************************************************************************/


/**A.1*  File Inclusion *******************************************************/
#include <ipaf_define.h>
#include <ipaf_shm.h>
#include <ipaf_shm2.h>
#include <ipaf_names.h>
#include <ipaf_resc.h>
#include <ipaf_svc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <utillib.h>
#include <ipaf_mmc.h>
#include <almstat.h>
#include <filter.h>
#include <stdarg.h>


/**B.1*  Definition of New Constants ******************************************/
#define		NO_MSG					0  /* MSG Queue NO MSG */
#define 	KillMC					2
#define 	StartMC					3
#define 	PROC_CHECK_LIMIT		2 /* 프로세스 체크 타임 */


/**B.2*  Definition of New Type  **********************************************/
typedef struct _st_Curr_Val{
	long long llCur;
	long long lMax;
}st_CurrVal2, *pst_CurrVal2;

typedef struct _st_mFIDB{
    unsigned char hwntp[2];
    st_CurrVal2 cpusts;
}st_mFIDB;
#define st_NTAF st_mFIDB
st_NTAF stNTAF,*fidb;		


/**C.1*  Declaration of Variables  ********************************************/
/* in load.c **/
char cpu_oldstat = NORMAL;
char mem_oldstat = NORMAL;
char disk_oldstat = NORMAL;

/* in ntpd.c **/
int dNIDIndex;
int EndFlag = 1;

int gdSysNo;    
extern st_NTP_STS stNTPSTS;
extern st_NTP_STS oldNTPSTS;
extern char ucDStatus;
extern char ucStatus;
extern char ucStatusFlag[16];

/**D.2*  Definition of Functions  *********************************************/
int dAppLog(int dIndex, char *fmt, ...)
{
    char            szMsg[30720];
    va_list         args;
    time_t now;
    struct tm tt;
    char cdate[15];
    char level[4];

    va_start(args, fmt);
    vsprintf(szMsg, fmt, args);
    va_end(args);

    time(&now);
    localtime_r(&now,&tt);
    sprintf(cdate,"%02d/%02d %02d:%02d:%02d",
        tt.tm_mon+1,tt.tm_mday,
        tt.tm_hour,tt.tm_min,tt.tm_sec);

    switch(dIndex){

        case LOG_CRI:
            sprintf(level,"CRI");
            break;
        case LOG_DEBUG:
            sprintf(level,"DBG");
			break;
        case LOG_INFO:
            sprintf(level,"INF");
			break;
		case LOG_WARN:
			sprintf(level,"WRN");
			break;
    }

    printf("[%s][%s]%s\n",cdate,level,szMsg);

    return 0;
}
int Send_AlmMsg( char loctype, char invtype, short invno, char almstatus, char oldalm )
{
	dAppLog(LOG_CRI,"Send_AlmMSg");
	dAppLog(LOG_CRI,"LOCTYPE[%d] INVTYPE[%d] invno[%d] almstatus[%d] oldalm[%d]",
		(int)loctype, (int)invtype, (int)invno, (int)almstatus, (int)oldalm);
	return 1;
}

/*******************************************************************************
 * MAIN FUNCTION
*******************************************************************************/
int main(int ac, int **av)
{
	time_t now,load_set;
	int a;
	int timeunit=1;

	if( ac > 1 )
		timeunit = atoi(av[1]);
	
	printf("Started Program![%d]( default : 1 )\n",timeunit);

	fidb = &stNTAF;
	memset( &stNTPSTS,0x00,sizeof(stNTPSTS));
	memset( &stNTPSTS,0x00,sizeof(stNTPSTS));
	ucDStatus = fidb->hwntp[0] = 0;
	ucStatus = fidb->hwntp[1] = 0;
	memset(ucStatusFlag,0x00,16);
	

	while(EndFlag){
		now = time(&now);
		
		if( abs(now - load_set ) > timeunit)
		{
			//CheckNTPStS();
			cpu_compute();
			load_set = now ;
		}
	}

	printf("Finished Program!\n");
	return 0;
}
