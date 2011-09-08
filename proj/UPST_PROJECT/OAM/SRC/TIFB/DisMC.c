/**A.1*  File Inclusion *******************************************************/

/* SYS HEADER */
#include <stdio.h>
#include <stdlib.h>
#include <sys/sysinfo.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
/* LIB HEADER */
#include "commdef.h"
#include "ipclib.h"
#include "filedb.h"
#include "verlib.h"
/* PRO HEADER */
#include "sshmid.h"
#include "path.h"
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
#include "almstat.h"
/* LOC HEADER */
#include "DisMC.h"
#include "process_info.h"

/**B.1*  Definition of New Constants ******************************************/

/**B.2*  Definition of New Type  **********************************************/
typedef struct _st_sw_ {
	unsigned char mpsw[MAX_SW_COUNT];
	st_ProcInfo   mpswinfo[MAX_SW_COUNT];
} st_Proc;

/**C.1*  Declaration of Variables  ********************************************/
int dCurrBlockCnt;
int debug_flag;
char errbuf[1024];
char szPid[20];
char szTime[20];
char STR_VER_COM[MAX_SW_COUNT][30];
st_Proc stProc;
st_Version *version;

/**C.2*  Declaration of Extern Variables  *************************************/
//extern int SEQ_PROC_IDX[];
extern char STR_TSW_COM[MAX_SW_COUNT][30];
extern long starttime;

/**D.1*  Declare of Functions  ************************************************/
int Init_Value(void);
int	InitVersionShm(void);	/* 20041110,poopee */
int dInit_fidb(void);
int dGetVerBlock(char *fn, char (*p)[30]);
void convert_pid(char status, int pid);
void convert_time(char status, time_t when);
void PrintResult(int dVerCount);

/**D.2*  Declare of Extern Functions  *****************************************/
extern int dGetUserPermission(void);
extern char *mtime(void);
extern int GetProcessID(char *);
extern int dGetBlocks(char *fn, char (*p)[30]);
extern void PrintOut(int flag, char *buf);

int main(int ac, char **av)
{
	int	i, dRet, dVerCount;

	if( (dRet = dGetUserPermission()) == 0)
	{
		sprintf(errbuf, "INAVLID USER PERMISSION\n" );
		PrintOut(TIFB_FAIL, errbuf);
		return EXIT_FAILURE;
	}

	if( (dVerCount = dGetVerBlock(FILE_MC_INIT, STR_VER_COM)) < 0)
	{
		sprintf(errbuf, "ERROR IN dGetVerBlock(%s)\n", FILE_MC_INIT);
		PrintOut(TIFB_FAIL, errbuf);
		return EXIT_FAILURE;
	}

	if( (dRet = dGetBlocks(FILE_MC_INIT, STR_TSW_COM)) < 0)
	{
		sprintf(errbuf, "FAILED IN McInit\n" );
		PrintOut(TIFB_FAIL, errbuf);
		return EXIT_FAILURE;
	}
	else
		dCurrBlockCnt = dRet;

	//debug flag setting
	if( ac == 2 && av[1][0] == '-' && av[1][1] == 'd' ){
		debug_flag = 1;
	}else{
		debug_flag = 0;
	}

	Init_Value();
	for(i = 0; i < dCurrBlockCnt; i++)
	{
		if(STR_TSW_COM[i] == NULL)
			continue;

		stProc.mpswinfo[i].pid = GetProcessID(STR_TSW_COM[i]);
		if(stProc.mpswinfo[i].pid > 0)
		{
			stProc.mpsw[i]			= NORMAL;
			stProc.mpswinfo[i].when	= starttime;
		}
		else
			stProc.mpsw[i] = CRITICAL;
	}

	for(i = 0; i < dCurrBlockCnt; i++)
	{
		if(stProc.mpsw[i] == NORMAL)
		{
			if(InitVersionShm() < 0)
			{
				sprintf(errbuf, "SHARED MEMORY( VERSION ) INIT FAILURE\n");
				PrintOut(TIFB_FAIL, errbuf);
				return EXIT_FAILURE;
			}
			break;
		}
	}

	dInit_fidb();
	PrintResult(dVerCount);

	return EXIT_SUCCESS;
}

int dInit_fidb(void)
{
	int			i, dRet;
	st_WNTAM	*fidb;

	if( (dRet = shm_init(S_SSHM_FIDB, DEF_WNTAM_SIZE, (void**)&fidb)) < 0 ){
		fprintf(stderr,"%s.%d:%s FAILED IN shm_init(S_SSHM_FIDB[0x%04X], dRet=%d) errno=%d:%s\n",
				__FILE__,__LINE__,__FUNCTION__,S_SSHM_FIDB,dRet,errno,strerror(errno));
		return -1;
	}

	for(i = 0; i < dCurrBlockCnt; i++)
	{
		if( (stProc.mpsw[i] == CRITICAL) && (fidb->stNTAM.mpsw[i] == STOP))
			stProc.mpsw[i] = STOP;
	}

	return 0;
}

void PrintResult(int dVerCount)
{
	int		i, j, online, absent, downed, stoped;
	char	pid[16], pr_time[20], sVersion[BUF_SIZE];

	printf("\n=================================================================\n");
	printf(" PROCESS         PID        STATUS     UPTIME           VERSION  \n");
	printf("-----------------------------------------------------------------\n");

	online	= 0;
	downed	= 0;
	stoped	= 0;
	absent	= 0;
	for(i = 0; i < dVerCount; i++) {
		sprintf(sVersion, "-  ");
		if(stProc.mpsw[i] == NORMAL) {
			online++;
			//j = SEQ_PROC_IDX[i];
			j = SeqProcID(STR_TSW_COM[i]);
			if( j >= 0 && j <= MAX_SW_COUNT && (int)strlen(version->szVersion[j]) ){
				if( debug_flag ){
					sprintf(sVersion, "%s:%d", version->szVersion[j],j);
				}else{
					sprintf(sVersion, "%s", version->szVersion[j]);
				}
			}else if( j < 0 ){
				if( debug_flag ){
					sprintf(sVersion, " ***:%d ",j);
				}else{
					sprintf(sVersion, " *** ");
				}
			}
			
		}
		else if(stProc.mpsw[i] == CRITICAL)
			downed++;
		else if( stProc.mpsw[i] == STOP )
			stoped++;
		else
			absent++;

		convert_pid(stProc.mpsw[i], stProc.mpswinfo[i].pid);
		convert_time(stProc.mpsw[i], stProc.mpswinfo[i].when);
		sprintf(pid, "%s", szPid);
		sprintf(pr_time, "%s", szTime);

		printf(" %-15s %-10s %-10s %-16s %-6s\n",
			STR_TSW_COM[i], pid, ((stProc.mpsw[i] == NORMAL)?"ACTIVE":(stProc.mpsw[i] == STOP)?"STOP":"DEACTIVE"), pr_time, sVersion);
	} /* extern for loop */

	printf("=================================================================\n");

	if(stoped > 0)
		printf("TOTAL:%d (ACTIVE:%d, STOP:%d, DEACTIVE:%d) ", dCurrBlockCnt, online, stoped, downed+absent);
	else
		printf("TOTAL:%d (ACTIVE:%d, DEACTIVE:%d) ", dCurrBlockCnt, online, downed+absent);

	printf("\n");
}

int Init_Value()
{

    memset( &stProc, 0x00, sizeof(st_Proc));

    return 1;

}

int InitVersionShm(void)
{
	int	dRet;
	if( (dRet = shm_init(S_SSHM_VERSION, sizeof(st_Version), (void**)&version)) < 0 ){
		fprintf(stderr,"%s.%d:%s FAILED IN shm_init(S_SSHM_VERSION[0x%04X], dRet=%d) errno=%d:%s\n",
				__FILE__,__LINE__,__FUNCTION__,S_SSHM_VERSION,dRet,errno,strerror(errno));
		return -1;
	}

	return 0;
}

void convert_pid( char status, int pid )
{
    memset( szPid, 0x00, 20 );

    if( status == NORMAL )
    {
        sprintf( szPid, "%d", pid );
    }
    else
    {
        sprintf( szPid, "   -  ");
    }
}

void convert_time( char status, time_t when )
{
    struct tm *tm_when;

    memset( szTime, 0x00, 20 );

    if( status == NORMAL )
    {
        tm_when = localtime( &when );

        sprintf( szTime, "%02d/%02d %02d:%02d", tm_when->tm_mon+1,
            tm_when->tm_mday, tm_when->tm_hour, tm_when->tm_min );
    }
    else
    {
        sprintf( szTime, "   -   ");
    }
}

int dGetVerBlock(char *fn, char (*p)[30])
{
	int     ln, rdcnt, scan_cnt, ldcnt;
	char    buf[BUF_SIZE], Bname[PROC_NAME_LEN], sCommand[BUF_SIZE], sOnOff[BUF_SIZE];
	FILE    *fp;

	if( (fp = fopen(fn, "r")) == NULL)
		return -1;  /* fopen error */

	ln		= 0;
	rdcnt	= 0;
	ldcnt   = 0;
	while(fgets(buf, BUF_SIZE, fp) != NULL)
	{
		ln++;
		/*
		* from Source to Target : sscanf
		*/
		if(buf[0] != '#')
		{
			printf("SYNTAX ERROR FILE:%s, LINE:%d - FIRST CHARACTER IS MUST '#'!\n", fn, ln);
			return -1;
		}
		else if(buf[1] == '#')
		{
			if( ((scan_cnt= sscanf(&buf[2], "%s %s %s", Bname, sCommand, sOnOff))==3) && (strcasecmp(sOnOff, "ON") == 0))
			{
				sprintf(*(p+rdcnt), "%s", Bname);
				rdcnt++;
			}
		}
		else if(buf[1] == 'E')
			break;
		else if(buf[1] == '@')
		{
			if( (scan_cnt= sscanf(&buf[2], "%s %*s", Bname)) != 1)
				sprintf(Bname, " - ");

			sprintf(*(p+rdcnt), "%s", Bname);
			rdcnt++;
			ldcnt++;
		}
		else
		{
			printf("SYNTAX ERROR FILE:%s, LINK:%d\n",fn, ln);
			return -2;
		}
	}
	fclose(fp);

	return ldcnt;
}
