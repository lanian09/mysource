/**A.1*  File Inclusion *******************************************************/

/* SYS HEADER */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <dirent.h>
#include <stropts.h>
#include <sys/signal.h>
#include <sys/syscall.h>
#include <sys/procfs.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <signal.h>
#include <almstat.h>
/* LIB HEADER */
#include "commdef.h"
#include "mems.h"
#include "cifo.h"
#include "clisto.h"
#include "gifo.h"
#include "nifo.h"
#include "ipclib.h"
#include "filedb.h"
#include "utillib.h"
#include "filelib.h"
/* PRO HEADER */
#include "sshmid.h"
#include "path.h"
#include "procid.h"
#include "msgdef.h"
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
/* LOC HEADER */
#include "KillMC.h"
#include "tifb_util.h"

/**B.1*  Definition of New Constants ******************************************/

/**B.2*  Definition of New Type  **********************************************/

/**C.1*  Declaration of Variables  ********************************************/
extern stMEMSINFO    *gpRECVMEMS;
extern stCIFO        *gpCIFO;

int pid;
int dCurrBlockCnt;

char	errbuf[1024];

st_WNTAM     	*fidb;

extern char STR_TSW_COM[MAX_SW_COUNT][30];	 /* '~/INC/PLAT/rsc_list.h' */
/**D.1*  Definition of Functions  *********************************************/

/**D.2*  Definition of Functions  *********************************************/

int Init_FIDB(void)
{
	int	dRet;

	if( (dRet = shm_init(S_SSHM_FIDB, DEF_WNTAM_SIZE, (void**)&fidb)) < 0 ){
		fprintf(stderr,"%s.%d:%s FAILED IN shm_init(S_SSHM_FIDB[0x%04X], dRet=%d) errno=%d:%s\n",
				__FILE__,__LINE__,__FUNCTION__,S_SSHM_FIDB,dRet,errno,strerror(errno));
		return -1;
	}

	return 0;
}
/*******************************************************************************
 CHSMD블럭으로 프로세스가 죽었음을 전달한다
*******************************************************************************/
void send_pid_msg(int idx, int pid)
{

    int dRet;
	long	      pmsg[4];
	time_t	      when;

	pst_MsgQ       pstSndMsg;
	pst_MsgQSub    pstMsgSub;
	unsigned char  *pNODE;

	if( (dRet = dGetNode(&pNODE, &pstSndMsg)) < 0 ){
		fprintf(stderr,"%s.%d:%s FAILED IN dGetNode(TIFB), errno=%d:%s\n",
				__FILE__,__LINE__,__FUNCTION__,errno,strerror(errno));
        exit(0);
	}

	time(&when);

	pmsg[0] = 2;	/* SMSSMD의 KillMC와 동일한 값을 기록 */
	pmsg[1] = idx;
	pmsg[2] = pid;
	pmsg[3] = when;

	pstMsgSub = (pst_MsgQSub)&pstSndMsg->llMType;
	pstMsgSub->usType = DEF_SYS;
	pstMsgSub->usSvcID = SID_STATUS;
	pstMsgSub->usMsgID = MID_ALARM;

	pstSndMsg->llMType = 2;

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

void final_isr( )
{

	sprintf(errbuf, "STOPPED BY USER REQUEST\n");
	PrintOut(TIFB_SUCCESS, errbuf);
	exit(0);
}

void normal_isr( int Signo )
{
	int err;

	printf("\n\tDou you want to stop finalization (y/n) ? ");
	err = GetYorN();

	if(err == _YES)
	{
		sprintf(errbuf, "FINALIZATION STOPPED IN PROGRESS\n");
		strcat(errbuf, "  BLOCKS ALREADY FINALIZED CAN'T AUTOMATICALLY CANCELLED\n");
		PrintOut(TIFB_FAIL, errbuf);
		exit(0);
	}

	signal(SIGINT, normal_isr);
	signal(SIGTERM, normal_isr);

	return;
}


/*******************************************************************************
 MAIN FUNCTION
*******************************************************************************/
int main(int ac, char **av)
{
	int				i, ii, j, fd, err, idx, dRet;
	int				fd2;
	char			bname[BUF_SIZE];
	char			tempbuf[BUF_SIZE];
	int				fast_kill_flag = 0;
	int				killed, failed, tried;
	int				pid;
	time_t			tNow;

	signal(SIGINT, normal_isr);
	signal(SIGQUIT, normal_isr);

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
	else
		dCurrBlockCnt = dRet;

	dRet = Init_FIDB();
	if( dRet < 0 ){
		printf("FAIL FIDB MEMORY\n");
		exit(1);
	}

	if( init_proc() < 0 ){
		exit(0);
	}

	if(ac >= 2)
	{
		if(av[1][0] != '-')
		{
			UsageAndExit();
		}

		if (av[1][1] == 'v') 	/* Fast Kill처리 */
		{
			fast_kill_flag = 1;
		}
		else if (av[1][1] == 'b')
		{
			if (ac == 2)
			{
				UsageAndExit();
			}

			for(j = 0;j < strlen(av[2]);j++)
			{
				bname[j] = toupper(av[2][j]);
			}

			bname[j] = 0x00;

			KillOnlyOneBlock(bname);

			exit(0);
		}
	}

	/*** 모든 블럭을 Kill하는것에 관하여 2번 확인후 삭제한다 ******************/
	if( fast_kill_flag == 0 )
	{
		printf("\n\tDo you want to finalize all blocks (y/n) ? ");
		err = GetYorN();
		if(err == _NO) {
			sprintf(errbuf, "STOPPED BY USER REQUEST\n");
			PrintOut(TIFB_FAIL, errbuf);
			exit(0);
		}

		printf("\n\tDo you really want to finalize all blocks (y/n) ? ");
		err = GetYorN();
		if(err == _NO)
		{
			sprintf(errbuf, "STOPPED BY USER REQUEST\n");
			PrintOut(TIFB_FAIL, errbuf);
			exit(0);
		}
	}


	/*** 전 블럭을 동시에 Kill한다 ********************************************/
	sprintf(errbuf, "MAIN COMPUTER PROCESS FINALIZATION STARTED\n");
	PrintOut(TIFB_SUCCESS, errbuf);

	signal(SIGINT, normal_isr);

	/*** 결과 정리 ************************************************************/
	killed = failed = tried = 0;

	/*** FIRST, KILL CHSMD ****************************************************/
	if ( KillOnlyOneBlock(STR_TSW_COM[0]) < 0 )
		failed++;
	else{
		killed++;
	}

	time(&tNow);

	for( i=0; i< dCurrBlockCnt;i++ )
	{
		if( fidb->stNTAM.mpsw[i] != CRITICAL && i != 0 )
		{
		}
		fidb->stNTAM.mpsw[i] = CRITICAL;
		fidb->stNTAM.mpswinfo[i].pid = 0;
		fidb->stNTAM.mpswinfo[i].when = tNow;
	}

	sleep(1);

	for( i=1; i< dCurrBlockCnt; i++ )
	{
		if( STR_TSW_COM[i] == NULL )
			continue;

		pid = GetProcessID( STR_TSW_COM[i] );

		/*** OPEN한 PID에 대한 유효성 검사 ************************************/
		idx = is_registered_block( STR_TSW_COM[i] );
		if( pid > 0 && idx >= 0 )
		{
			/* kill 신호를 전송한다 */
			if ( kill(pid, SIGTERM) < 0 )
			{
				if( fast_kill_flag == 0 )
				{
					if( errno == EPERM )
					{
						sprintf(errbuf, "CAN'T KILL PROCESS %d(%s) \n  NO PERMISION \n", pid, STR_TSW_COM[i]);
                        PrintOut(TIFB_FAIL, errbuf);
                        err = _YES;

					}
					else
					{
						sprintf(errbuf, "CAN'T KILL PROCESS %d(%s) \n", pid, STR_TSW_COM[i] );
						PrintOut(TIFB_FAIL, errbuf);
						printf("\tDo you want to continue (y/n) ? ");
						err = GetYorN();
					}
				}
				else
				{
					err = _YES;
				}

				if (err == _NO) final_isr();
				failed++;
			}
			else
			{
				/*** kill 전송후 실제 프로세스의 존재여부를 검사한다 **********/
				if (fast_kill_flag)
				{
					sprintf(errbuf, "A PROCESS WILL BE KILLED\n  PROCESS ID : %d\n  BLOCK NAME : %s\n",
						pid, STR_TSW_COM[i]);
					PrintOut(TIFB_SUCCESS, errbuf);
					tried++;
				}
				else
				{
					/*** 최대 5초간 존재 여부를 검사한다 **********************/
					for (j = 0, ii = 0;ii < 5;ii++)
					{
						sleep(1);
						if ((fd = open(tempbuf, O_RDONLY)) >= 0)
						{
							close(fd);
						}
						else
						{
							j = 1;
							break;
						}
					}

					if (j == 1)
					{
						sprintf(errbuf, "A PROCESS WAS KILLED\n  PROCESS ID : %d\n  BLOCK NAME : %s\n",
						pid, STR_TSW_COM[i]);
						PrintOut(TIFB_SUCCESS, errbuf);
						killed++;
					}
					else
					{
						sprintf(errbuf, "CAN'T KILL PROCESS %d(%s)\n", pid, STR_TSW_COM[i]);
						PrintOut(TIFB_FAIL, errbuf);
						failed++;
					}
				}
			}
		}
	}

	PrintResult(killed, tried, failed);

	/*** REMOVE MESSAGE QUEUE & SHARED MEMORY *********************************/
	if( fast_kill_flag == 1 )
		RemoveSHMnSEMA();

	if ((fd2 = open (FILE_FIDB, O_RDONLY, 0666)) < 0)
    {
    }
    else
    {
        remove(FILE_FIDB);
    }

	exit(0);

} /* main */


/*******************************************************************************
 KILL ONLY ONE BLOCK FUNCTION
*******************************************************************************/
int KillOnlyOneBlock(char *bname)
{
	int             qid;
	int             idx;

	/*** GET PROCESS ID USING PROCESS NAME ************************************/
	qid = GetProcessID(bname);
	if( qid > 0 )
	{
		/*** PROCESS NAME의 유효성 검사 ***************************************/
		idx = is_registered_block(bname);
		send_pid_msg(idx, qid);

		/*** CHSMD에서 공유 메모리 변경에 따른 문제 해결을 위해 ***************/
		sleep(1);

		if(	kill( qid, SIGTERM ) < 0 )
		{
			sprintf(errbuf, "CAN'T KILL PROCESS %d(%s) [%s]\n",
							qid, bname, strerror(errno) );
			PrintOut(TIFB_FAIL, errbuf);
			return -1;
		}
		else
		{
			sprintf(errbuf, "A PROCESS WAS KILLED\n  PROCESS ID : %d\n  BLOCK NAME : %s\n", qid, bname);
			PrintOut(TIFB_SUCCESS, errbuf);
			return 0;
		}
	}

	/*** PROCESS IS NOT RUNNING ***********************************************/
	idx = is_registered_block(bname);

    if( idx >= 0 && idx < dCurrBlockCnt)
    {
        send_pid_msg( idx , 0 );
    }

	sprintf(errbuf, "NON EXIST OR NOT REGISTERD BLOCK NAME [%d]\n", idx);
	PrintOut(TIFB_FAIL, errbuf);

	return -1;

} /* KillOnlyOneBlock */



/*******************************************************************************
 PRINT RESULT : TRIED, KILLED, FAILED INFORMATION
*******************************************************************************/
void PrintResult(int killed, int tried, int failed)
{
	char tmpbuf[128];

	sprintf(errbuf, "MAIN COMPUTER PROCESS FINALIZATION ENDED\n");
	sprintf(tmpbuf, "  KILLED PROCESS (CONFIRMED)     : %d\n", killed);
	strcat(errbuf, tmpbuf);
	sprintf(tmpbuf, "  KILLED PROCESS (NOT CONFIRMED) : %d\n", tried);
	strcat(errbuf, tmpbuf);
	sprintf(tmpbuf, "  ALIVE PROCESS (KILL FAILED)    : %d\n", failed);
	strcat(errbuf, tmpbuf);

	PrintOut(TIFB_SUCCESS, errbuf);

	return;
} /* PrintResult */


/*******************************************************************************
 WRONG COMMAND PRINT
*******************************************************************************/
void UsageAndExit()
{
	sprintf(errbuf, "ILLEGAL USAGE\n");
	strcat(errbuf, "  USAGE: KillMC [-b block-name]\n");
	strcat(errbuf, "        -b : Kill and Clear Queue for only this block\n");
	PrintOut(TIFB_FAIL, errbuf);
	exit(0);

} /* UsageAndExit */

/*******************************************************************************
 REMOVE MESSAGE QUEUE AND SHARED MEMORY
*******************************************************************************/
void RemoveSHMnSEMA(void)
{
	int id, i = 0;
    
	//O&M SSHM KEY REMOVE
    for( i = S_SSHM_START_KEY_OAM; i < S_SSHM_LAST_KEY_OAM; i++ ){
        id = shmget( i, 0, 0666 );
        if( id < 0 ){
            if( errno != ENOENT ){
                fprintf(stderr,"GET SHARED MEMORY FAIL(1) [0x%0x], error=%d:%s\n", i, errno, strerror(errno));
            }
            continue;
        }

        if( shmctl( id, IPC_RMID, (struct shmid_ds *)0 ) < 0 ){
            fprintf(stderr, "REMOVE SHARED MEMORY FAIL(1) [0x%0x], error=%d:%s\n", i, errno, strerror(errno));
        }
    }

	//O&M 제외 나머지 모든 SSHM KEY REMOVE
    for( i = S_SSHM_START_KEY; i < S_SSHM_LAST_KEY; i++ ){
        id = shmget( i, 0, 0666 );
        if( id < 0 ){
            if( errno != ENOENT ){
                fprintf(stderr,"GET SHARED MEMORY FAIL(2) [0x%0x], error=%d:%s\n", i, errno, strerror(errno));
            }
            continue;
        }

        if( shmctl( id, IPC_RMID, (struct shmid_ds *)0 ) < 0 ){
            fprintf(stderr, "REMOVE SHARED MEMORY FAIL(2) [0x%0x], error=%d:%s\n", i, errno, strerror(errno));
        }
    }
	#if 0 // ------------------------------------------------------------------------------------------------------------------------
	// 이거 쓰이지 않음. 
	// 원래는 McInit 에 등록한 값을 읽도록 하려고 아래 소스가 쓰일 것이라 예상했으나...
	// 헤더에 define 에서 사용하기로 함에 따라, 사용되지 않음. 

	int id, i = 0;
	int dShmCnt, dSemCnt = 1;

	dShmCnt = pstShmInfo->dShmCnt;
	dSemCnt = pstSemInfo->dSemCnt;

    /* Get Shared Memory Info List */
	if(get_shm_info(FILE_SHM_INFO, pstShmInfo) < 0)
	{
		fprintf( stderr, "[ERROR] Get Shared Memory Info\n");
		exit(0);
	}
	for( i = 0; i < dShmCnt; i++ )
    {
        id = shmget( *((pstShmInfo->dShmList)+i),0, 0666);
        if( id < 0 ) {
            if( errno != ENOENT )
                printf(" GET SHARED MEMORY FAIL [0x%x:%s][%d:%s]\n", *((pstShmInfo->dShmList)+i), pstShmInfo->sShmName[i], errno, strerror(errno));
        }
        else
        {
            if( shmctl( id, IPC_RMID, (struct shmid_ds *)0 ) < 0 )
            {
                printf(" REMOVE SHARED MEMORY FAIL [0x%x:%s][%d:%s]\n", *((pstShmInfo->dShmList)+i), pstShmInfo->sShmName[i], errno, strerror(errno));
            }
		}
	}

    /* Get Semaphore Info List */
	if(get_sema_info(FILE_SEM_INFO, pstSemInfo) < 0)
	{
		fprintf( stderr, "[ERROR] Get Shared Memory Info\n");
		exit(0);
	}

	for( i = 0; i <  dSemCnt; i++ )
    {
        id = semget( *((pstSemInfo->dSemList)+i), 0, 0666);
        if(id < 0)
		{
            if( errno != ENOENT )
                printf(" GET Semaphore FAIL [0x%04X:%s][%d:%s]\n", *((pstSemInfo->dSemList)+i) , pstSemInfo->sSemName[i], errno, strerror(errno));
        }
        else
        {
            if(semctl(id, 0, IPC_RMID, (struct semid_ds*)0) < 0)
                printf(" REMOVE Semaphore FAIL [0x%x:%s][%d:%s]\n", *((pstSemInfo->dSemList)+i) , pstSemInfo->sSemName[i], errno, strerror(errno));
        }
    }
	#endif // -------------------------------------------------------------------------------------------------------------------------------------
}
