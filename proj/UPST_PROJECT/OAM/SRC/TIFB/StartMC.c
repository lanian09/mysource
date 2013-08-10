/**A.1*  File Inclusion *******************************************************/

/* SYS HEADER */
#include <ctype.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <dirent.h>
#include <sys/stat.h>
#include <signal.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/msg.h>
/* LIB HEADER */
#include "commdef.h"
#include "mems.h"
#include "cifo.h"
#include "clisto.h"
#include "gifo.h"
#include "nifo.h"
#include "utillib.h"
/* PRO HEADER */
#include "procid.h"
#include "path.h"
#include "msgdef.h"
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
/* LOC HEADER */
#include "StartMC.h"
#include "tifb_util.h"

/**B.1*  Definition of New Constants ******************************************/
extern char STR_TSW_COM[MAX_SW_COUNT][30];

int     dCurrBlockCnt;

/**B.2*  Definition of New Type  **********************************************/

typedef struct {
	char	bname[PROC_NAME_LEN];
	char	fname[BUF_SIZE];
} blocks_t;

typedef struct {
	int		isinit;
	int		pid;
} init_t;

/**C.1*  Declaration of Variables  ********************************************/

static int		cntr;
static blocks_t	blocks[MAX_SW_COUNT];
static init_t	inits[MAX_SW_COUNT];

/**D.1*  Declare of Functions  ************************************************/

void init_isr(), final_isr();
/**D.2*  Definition of Functions  *********************************************/

/*******************************************************************************
 SEND MESSAGE TO CHSMD. PID, INDEX, WHEN
*******************************************************************************/
void send_pid_msg(int idx, int pid)
{
	long    pmsg[4];
	time_t	when;

	int         dRet;
    pst_MsgQ    pstSndMsg;
    pst_MsgQSub pstMsgSub;
	unsigned char  *pNODE;

	if( (dRet = dGetNode(&pNODE, &pstSndMsg)) < 0 ){
		fprintf(stderr,"%s.%d:%s FAILED IN dGetNode(TIFB), errno=%d:%s\n",
				__FILE__,__LINE__,__FUNCTION__,errno,strerror(errno));
        exit(0);
	}

	time(&when);

	pmsg[0] = 3;
	/* CHSMD의 StartMC와 동일한 값을 기록 */
	pmsg[1] = idx;
	pmsg[2] = pid;
	pmsg[3] = when;

	pstMsgSub = (pst_MsgQSub)&pstSndMsg->llMType;
    pstMsgSub->usType = DEF_SYS;
    pstMsgSub->usSvcID = SID_STATUS;
    pstMsgSub->usMsgID = MID_ALARM;

	pstSndMsg->llMType = 3;

    util_makenid( SEQ_PROC_TIFB, &pstSndMsg->llNID ); 
	pstSndMsg->ucNTAFID = 0;
	pstSndMsg->ucProID = SEQ_PROC_TIFB;
    pstSndMsg->llIndex = 0;

    pstSndMsg->dMsgQID = 0;
    pstSndMsg->usBodyLen = sizeof(long)*4;
    pstSndMsg->usRetCode = 0;

    memcpy( &pstSndMsg->szBody, pmsg, pstSndMsg->usBodyLen );

	if( (dRet = dMsgsnd(SEQ_PROC_CHSMD, pNODE)) < 0 ){
		fprintf(stderr, "%s.%d:%s FAILED IN dMsgsnd(CHSMD)\n",__FILE__,__LINE__,__FUNCTION__);
		exit(0);
	}
}

/*******************************************************************************
 WRONG COMMAND, PRINT FUNCTION
*******************************************************************************/
void UsageExit()
{
	char errbuf[2048];

	sprintf(errbuf, "ILLEGAL USE\n  USAGE: StartMC \n");
	strcat(errbuf, "         StartMC -b Block_Name\n");
	PrintOut(TIFB_FAIL, errbuf);
	exit(0);
}


/*******************************************************************************
 MAIN FUNCTION
*******************************************************************************/
int main(int ac, char **av)
{
	FILE *fp;
	int	fd;
	int  pid;

	int i, j, ln, err, kf, dRet;
	int init_only_one = 0;
	int verbose_flag = 0;

	char buf[BUF_SIZE];
	char fname[BUF_SIZE];
	char block_name[BUF_SIZE];
	char tmpbuf[128], errbuf[2048];

	signal(SIGHUP, SIG_IGN);
	signal(SIGCHLD, SIG_IGN);
	signal(SIGQUIT, init_isr);

	dRet = dGetUserPermission();
    if( dRet == 0 )
    {
        sprintf(errbuf, "INAVLID USER PERMISSION\n" );
        PrintOut(TIFB_FAIL, errbuf);

        return 1;
    }

	dRet = dGetBlocks(FILE_MC_INIT,STR_TSW_COM);
	if( dRet < 0 ){
        sprintf(errbuf, "FAILED IN McInit\n" );
        PrintOut(TIFB_FAIL, errbuf);

        return 1;

	}
	else{
		dCurrBlockCnt = dRet;
	}

	if( init_proc() < 0 ){
		exit(0);
	}

	if (ac == 1)	/*** ALL START ********************************************/
	{
		strcpy(fname, FILE_MC_INIT); /*** "McInit" File ****************************/
	}
	else if (ac > 3)
	{
		UsageExit();
	}
	else
	{
		for(i = 1;i < ac;i++)
		{
			if (av[i][0] != '-')
			{
				UsageExit();
			}
			else if(av[i][1] == 'b')
			{
				if( ac < 3 )
				{
					UsageExit();
				}
				else	/*** ONLY ONE BLOCK START *****************************/
				{
					++i;
					init_only_one = 1;
 					for(j = 0;j < strlen(av[i]);j++)
						block_name[j] = toupper(av[i][j]);
					block_name[j] = 0;
				}
			}
			else if( av[i][1] == 'v' )	/*** ALL START ************************/
			{
				verbose_flag = 1;
			}
			else if(av[i][1] == 'd') {

                /* FOR SYSTEM AUTO RESTART : WAIT BOOTTING COMPLETE */
                sleep(300);
                verbose_flag = 1;
            }
			else
			{
				UsageExit();
			}
		}
	}

 	//freopen("/dev/null", "w", stdout);

	/*** 한개 블럭을 기동 시킴 ************************************************/
	if (init_only_one)
	{
		pid = InitOnlyOneBlock(block_name);
        if(pid > 0)
        {
            sprintf(errbuf, "NEW BLOCK : %s\n  PROCESS ID: %d\n", block_name, pid);
            PrintOut(TIFB_SUCCESS, errbuf);

            for(i=0; i< dCurrBlockCnt; i++)
            {
				if( STR_TSW_COM[i] == NULL )
					continue;

                if( !strcasecmp(block_name, STR_TSW_COM[i]) )
                {
                    send_pid_msg(i, pid);
                    break;
                }
            }
        }
		exit(0);
	}

	/***************************************************************************
	 READ FIDB_FILE, SET SHARED MEMORY VALUE
	***************************************************************************/
	if ((fd = open (FILE_FIDB, O_RDONLY, 0666)) < 0)
    {
    }
	else
	{
		remove( FILE_FIDB );
	}

	strcpy(fname, FILE_MC_INIT);

	if((fp = fopen(fname, "r")) == NULL)
	{
		sprintf(errbuf, "CAN'T OPEN FILE %s\n", fname);
		PrintOut(TIFB_FAIL, errbuf);
		exit(1);
	}

	ln = cntr = 0;

	/***************************************************************************
	 READ McInit File. SET blocks(bname, fname) VALUE
	***************************************************************************/
	while(fgets(buf, BUF_SIZE, fp) != NULL)
	{
		ln++;
		if (AnalyzeLine(buf) < 0)
		{
			fclose(fp);
			close(fd);
			sprintf(errbuf, "SYNTAX ERROR (FILE: %s, LINE: %d)\n", fname, ln);
			PrintOut(TIFB_FAIL, errbuf);
			exit(1);
		}
	}

	fclose(fp);

	/***************************************************************************
	 WHEN ALL BLOCK IS STARTED, CHECK 2 TIMES
	***************************************************************************/
	if( verbose_flag == 0 )
	{
		fprintf(stderr, "\n\tBlocks to initialize are follows.\n");
		PrintBlocks();

		fprintf(stderr, "\tDo you want initialize (y/n) ? ");
		err = GetYorN();
		if(err == _NO)
		{
			sprintf(errbuf, "STOPPED BY USER REQUEST\n");
			PrintOut(TIFB_SUCCESS, errbuf);
			exit(0);
		}
		fprintf(stderr, "\n\tDo you really want initialize (y/n) ? ");
		err = GetYorN();
		if(err == _NO)
		{
			sprintf(errbuf, "STOPPED BY USER REQUEST\n");
			PrintOut(TIFB_SUCCESS, errbuf);
			exit(0);
		}
	}

    sprintf(errbuf, "MAIN COMPUTER PROCESSS INITIALIZATION STARTED\n");
	PrintOut( TIFB_SUCCESS , errbuf );

	signal(SIGINT, init_isr);


	/*** 프로세스 기동을 개시함 ***********************************************/
	for(i=0; i<cntr; i++)
	{
		/*** 파일의 존재 및 수행가능성 조사 ***********************************/
		if( IsCorrectBlock(i) < 0 )
		{
			if( verbose_flag == 1 )
			{
				continue;
			}
			else
			{
				sprintf(errbuf, "FILE %s DOES NOT EXIST\n", blocks[i].fname);

				PrintOut(TIFB_FAIL, errbuf);

				fprintf(stderr, "\n\tDo you want to continue (y/n) ? ");

				err = GetYorN();

				if(err == _YES)
				{
					inits[i].isinit = _FALSE;
					continue;
				}
				else
				{
					final_isr();
				}
			}
		}

		/*** 이미 실행중인지 여부를 검사 **************************************/
		if ( (pid = GetProcessID(blocks[i].bname)) > 0 )
		{
			/* 이미 실행중인 경우 */

			if( verbose_flag == 1 )
			{
				err = _YES ;
			}
			else
			{
				fprintf(stderr, "\n\tBlock \"%s\" is already running.\n", blocks[i].bname);
				fprintf(stderr, "\tDo you want replace block \"%s\" (y/n) ? ", blocks[i].bname);
				err = GetYorN();
			}

			if(err == _NO)
			{
				inits[i].isinit = _FALSE;
				continue;
			}
			else
			{
				/* New Version of Killing */
				kf = kill(pid, SIGTERM);

				if (kf < 0)
				{
					//sleep(2);
					if (kill(pid, SIGTERM) < 0)
					{
			   			if (errno == ESRCH)
			       			kf = 0;
			  		}
					else
					{
			   			kf = 0;
					}
			   	}

				if ( kf && kill(pid, SIGKILL) < 0)
				{
					if( verbose_flag == 1 )
					{
						err = _YES ;
					}
					else
					{
						fprintf(stderr, "\tCan't kill process \"%d\"(%s)\n", pid, blocks[i].bname);
						fprintf(stderr, "\tDo you want to continue (y/n) ? ");
						err = GetYorN();
					}


					if(err == _YES)
					{
						inits[i].isinit = _FALSE;
						continue;
					}
					else
					{
						final_isr();
					}
				}
				else
				{
					sprintf(errbuf, "PROCESS %s(PID=%d) KILLED\n", blocks[i].fname, pid);
					PrintOut(TIFB_SUCCESS, errbuf);
				}
			}
		}

		signal(SIGTERM, init_isr);

		/***********************************************************************
		 PROCESS START
		***********************************************************************/
		err = ProcessStart(i);

		if(err == -1)
		{

			if( verbose_flag == 1 )
			{
				continue;
			}
			else
			{
				sprintf(errbuf, "CAN'T START PROCESS BLOCK: %s\n", blocks[i].bname);
				PrintOut(TIFB_FAIL, errbuf);

				fprintf(stderr, "\tDo you want to continue (y/n) ? ");
			}
		}
		else if(err == -2)
		{
			if( verbose_flag == 1 )
			{
				continue;
			}
			else
			{
				sprintf(errbuf, "PROGRAM NAME %s DOES NOT MEET NAME CONVENTION\n", blocks[i].fname);
				strcat(errbuf, "  ABANDON EXECUTING PROCESS\n");
				PrintOut(TIFB_FAIL, errbuf);

				fprintf(stderr, "\tDo you want to continue (y/n) ? ");
			}
		}

		if(err < 0)
		{
			if( verbose_flag == 1 )
			{
				err = _YES;
			}
			else
			{
				err = GetYorN();
			}


			if(err == _NO)
			{
				final_isr();
			}
			else
			{
				inits[i].isinit = _FALSE;
				continue;
			}
		}
		else
		{
			sprintf(errbuf, "A PROCESS INIAILIZATION SUCCESS\n");
			sprintf(tmpbuf, "  BLOCK NAME   : %s\n", blocks[i].bname);
			strcat(errbuf, tmpbuf);
			sprintf(tmpbuf, "  PROGRAM NAME : %s\n", blocks[i].fname);
			strcat(errbuf, tmpbuf);
			sprintf(tmpbuf, "  PROCESS ID   : %d\n", err);
			strcat(errbuf, tmpbuf);

			PrintOut(TIFB_SUCCESS, errbuf);
			sleep(1);


			for(j=0; j< dCurrBlockCnt; j++)
			{
				if( STR_TSW_COM[i] == NULL )
					continue;

				if( !strcasecmp(blocks[i].bname, STR_TSW_COM[j]) )
				{
					send_pid_msg(j, err );
					break;
				}
			}

			inits[i].isinit = _TRUE;
			inits[i].pid = err;
		}
	}

	PrintSuccessBlocks();
	printf("UPRESTO co. GTAM\n\n");
	exit(1);
}


/*******************************************************************************
 READ McInit ONE LINE, CHECK REGISTERED BLOCK
*******************************************************************************/
int AnalyzeLine(char *buf)
{
	int 	i;
	char 	Fname[BUF_SIZE];
	char 	Bname[PROC_NAME_LEN];


	/*** 기본 요건을 조사하여 검증된 경우 sscanf를 처리함 *********************/
	if( buf[0] == '#' ){
		if( buf[1] == 'E' || buf[1] == '#' ){
			return 0;
		}
		else if( buf[1] == '@' ){
			for( i=2;i < strlen(buf) && i < BUF_SIZE;i++)
			{
				if (!isalnum(buf[i]) && buf[i] != '\n' && buf[i] != '\0' &&
						  buf[i] != '\t' && buf[i] != ' ' && buf[i] != '/' &&
						  buf[i] != '.' && buf[i] != '_')
				{
					printf("[DEBUG] 'isalnum' ERROR\n");
					return -1;
				}
			}

			if (sscanf(&buf[2], "%s%s", Bname, Fname) != 2)
				return 0;


			if (is_registered_block(Bname) < 0)
				return -2;

			strcpy(blocks[cntr].bname, Bname);
			strcpy(blocks[cntr].fname, Fname);

			cntr++;
		}
		else
			return -3;
	}
	return 0;
}


/*******************************************************************************
 BEFORE ALL START, PRINT ALL BLOCK
*******************************************************************************/
void PrintBlocks()
{
	int i;

	fprintf(stderr, "\t====================================================\n");
	fprintf(stderr, "\t  %-20s%-30s\n", "Block Name", "File Name");
	fprintf(stderr, "\t----------------------------------------------------\n");
	for(i=0;i < cntr;i++) {
		fprintf(stderr, "\t  %-20s%-30s\n", blocks[i].bname, blocks[i].fname);
	}
	fprintf(stderr, "\t====================================================\n");
}


/*******************************************************************************

*******************************************************************************/
void PrintSuccessBlocks()
{
	int i, success;
	char errbuf[MAX_ERRBUF_LEN];

	for(i=success=0;i < cntr;i++)
		if(inits[i].isinit == _TRUE)
			success++;

	sprintf(errbuf, "MAIN COMPUTER PROCESS INITIALIZATION SUCCESSFULLY COMPLETED\n  NO OF INITIALIZED PROCESS = %d\n", success);
	PrintOut(TIFB_SUCCESS, errbuf);
}

/*******************************************************************************
 ONLY ONE BLOCK START
*******************************************************************************/
int InitOnlyOneBlock(char *bname)
{
	int 	i, pid;
	char 	pname[BUF_SIZE];
	char 	fname[BUF_SIZE];
	char 	errbuf[MAX_ERRBUF_LEN];
	struct stat statbuf;

    if (is_registered_block(bname) < 0)
	{
		sprintf(errbuf, "NON EXIST BLOCK NAME\n");
		PrintOut(TIFB_FAIL, errbuf);
		return -1;
	}

	for( i=0 ; i < strlen(bname); i++)
		pname[i] = toupper(bname[i]);

	pname[strlen(bname)] = 0x00;

	pid = GetProcessID(pname);
	if(pid > 0) /*** ALREADY RUNNING *****************************************/
	{
		sprintf(errbuf, "ALREADY RUNNING PROCESS %s\n", pname);
		PrintOut(TIFB_FAIL, errbuf);
		return -1;
	}

	//sprintf(fname, "%s%s", APP_HOME_BIN, pname);
	fname[0] = 0x00;
    if( dGetBlockBIN(pname, fname, 256) < 0)
    {
        sprintf(errbuf, "NOT Find Binary(%s) in McInit\n", pname);
        PrintOut(TIFB_FAIL, errbuf);
    }

    if(stat(fname, &statbuf) < 0)
        return -3;

	pid = fork();
	if(pid < 0)
	{
		sprintf(errbuf, "CAN'T CREATE A NEW PROCESS\n");
		PrintOut(TIFB_FAIL, errbuf);
		return -1;
	}
	else if(pid == 0)
	{
		freopen("/dev/null", "w", stdout);
        freopen("/dev/null", "r", stdout);
		if (execl(fname, pname, (char *)0) < 0)
		{
			memset(errbuf, 0x0, MAX_ERRBUF_LEN);
			sprintf(errbuf, "CAN'T EXECUTE A NEW BLOCK : %s(%s)\n", bname, fname);
			PrintOut(TIFB_FAIL, errbuf);
		}
		exit(0);
	}
	else
	{
		return pid;
	}
}


void init_isr( int Signo )
{
	int err;
	char errbuf[MAX_ERRBUF_LEN];

	fprintf(stderr, "\n\tDo you want to stop initialization(y/n) ? ");
	err = GetYorN();

	if(err == _YES)
	{
		sprintf(errbuf, "INITIALIZATION STOPPED IN PROGRESS\n");
		strcat(errbuf, "  BLOCKS ALREADY INITLAIZED CAN'T AUTOMATICALLY CANCELLED\n");
		PrintOut(TIFB_FAIL, errbuf);
		exit(0);
	}

    signal(SIGQUIT, init_isr);
}


void final_isr( int Signo )
{
	char errbuf[MAX_ERRBUF_LEN];

	sprintf(errbuf, "STOPPED BY USER REQUEST\n");
	PrintOut(TIFB_SUCCESS, errbuf);
	exit(0);
}


/*******************************************************************************
 PROCESS START
*******************************************************************************/
int ProcessStart(int idx)
{
	int pid, i;
	char pname[BUF_SIZE];
	char errbuf[MAX_ERRBUF_LEN];

    for(i = 0;i < strlen(blocks[idx].bname);i++)
        pname[i] = toupper(blocks[idx].bname[i]);

    pname[strlen(blocks[idx].bname)] = 0x00;

	pid = fork();

	if(pid < 0)
	{
		return -1;
	}
	else if(pid == 0)
	{
		freopen("/dev/null", "w", stdout);
        freopen("/dev/null", "r", stdout);
		if (execl(blocks[idx].fname, pname, (char *)0) < 0)
		{
			sprintf(errbuf, "CAN'T EXECUTE FILE : %s [%s]\n", blocks[idx].fname, strerror(errno));
			PrintOut(TIFB_FAIL, errbuf);
		}
		exit(0);
	}
	else
	{
		return pid;
	}
}


/*******************************************************************************
 CHECK PROCESS FILE
*******************************************************************************/
int IsCorrectBlock(int idx)
{
	struct stat statbuf;

	if (stat(blocks[idx].fname, &statbuf) < 0 )
		return -1;

	return 1;
}


