/**A.1*  File Inclusion *******************************************************/

/* SYS HEADER */
#include <stdio.h>
#include <dirent.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>		/* strcmp */
#include <stdlib.h>		/* EXIT(3) */
#include <unistd.h>		/* EXEC(3) */
#include <errno.h>
#include <sys/stat.h>
/* LIB HEADER */
#include "filedb.h"
#include "loglib.h"
#include "verlib.h"
/* PRO HEADER */
#include "path.h"
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
#include "almstat.h"
/* LOC HEADER */
#include "chsmd_init.h"	/* dGetBlockBIN() */
#include "chsmd_msg.h"	/* Send_AlmMsg() */
#include "chsmd_sw.h"

/**B.1*  Definition of New Constants ******************************************/
/**B.2*  Definition of New Type  **********************************************/
/**C.1*  Declaration of Variables  ********************************************/
/**C.2*  Declaration of Variables  ********************************************/
extern pst_NTAF    fidb;

/* 소프트웨어 알람등급 */
extern st_Version *version;

extern int 	dCurrBlockCnt;
extern char STR_TSW_COM[MAX_NTAF_SW_BLOCK][30];

/**D.1*  Definition of Functions  *********************************************/
/**D.2*  Definition of Functions  *********************************************/

/*******************************************************************************
 * GET PROCESS NAME USING PROCESS IDEX VALUE
*******************************************************************************/
char *name_of_process(int id)
{
    if(id < 0 || id > dCurrBlockCnt)
	{
        return "UNKNOWN";
	}
	else
	{
    	return STR_TSW_COM[id];
	}
}


/*******************************************************************************
 * GET PROCESS IDEX VALUE USING PROCESS NAME
*******************************************************************************/
int id_of_process(char *name)
{
    int     i;

    for(i = 0;i < dCurrBlockCnt ;i++)
	{
        if( !strcmp(name, STR_TSW_COM[i]) )
		{
            return i;
		}
	}

    return -1;
}


/*******************************************************************************
 * GET PROCESS ID USING PROCESS NAME
*******************************************************************************/
int get_proc_id(char *name)
{
	int				fd, ret;
	DIR				*dirp;
	struct dirent	*direntp;
	char			pname[PROC_NAME_LEN_128], tempbuf[LSIZE_256];

	if( (dirp = opendir(PROC_PATH)) == (DIR*)NULL)
	{
		log_print(LOGN_CRI, "CAN'T ACCESS PROCESS DIRECTORY (%s) %s", PROC_PATH, strerror(errno));
		exit(-2);
	}

	while( (direntp = readdir(dirp)) != NULL)
	{
		if(!strcmp(direntp->d_name, PARENT_PATH) || !strcmp(direntp->d_name, HOME_PATH))
			continue;

		if(!atoi(direntp->d_name))
			continue;

		/***********************************************************************
		* CHECK PROCESS NAME USING /proc/PID/cmdline FILE
		***********************************************************************/
		sprintf(tempbuf, "%s/%s/cmdline", PROC_PATH, direntp->d_name);
		if( (fd = open(tempbuf, O_RDONLY)) == -1)
			continue;

		memset(pname, 0x00, PROC_NAME_LEN_128);
		if((ret = read(fd, pname, PROC_NAME_LEN_128-1)) < 0)
		{
			close(fd);
			continue;
		}
		else
			close(fd);

		pname[ret] = 0x00;
		if(!strcmp(name, pname))
		{
			closedir(dirp);
			return atoi(direntp->d_name);
		}
	}
    closedir(dirp);

    return -1;
}


/*******************************************************************************
 * PROCESS AUTO RESTART
*******************************************************************************/
int auto_restart_the_process(int idx)
{
	int			pid, ret;
	char		pname[LSIZE_256];
	struct stat	statbuf;

	/***************************************************************************
	* CHECK INDEX RANGE
	***************************************************************************/
	if( (idx < 0) || (idx >= dCurrBlockCnt))
		return -1;

	/***************************************************************************
	* ALREADY RUNNING, REMOVE PROCESS INFORATION
	***************************************************************************/
	if( (pid = get_proc_id(STR_TSW_COM[idx])) > 0)
	{
		fidb->mpsw[idx]				= NORMAL;
		fidb->mpswinfo[idx].pid		= pid;
		fidb->mpswinfo[idx].when	= time(NULL);

		return pid;
	}

	if( (ret = dGetBlockBIN(STR_TSW_COM[idx], pname, LSIZE_256)) < 0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: ERROR IN dGetBlockBIN(STR_TSW_COM[%d][%s]) ret[%d]", __FILE__, __FUNCTION__, __LINE__,
			idx, STR_TSW_COM[idx], ret);
	}

	if(stat(pname, &statbuf) < 0)
		return -2;

	signal(SIGCHLD, SIG_IGN);
	pid = fork();

	log_print(LOGN_DEBUG, "[AUTO RESTART] [NEW PID] [%d]", pid);
	/* fork에서 실패하면 종료한다 */
	if(pid == -1)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN fork(pname[%s]) errno[%d-%s]", __FILE__, __FUNCTION__, __LINE__,
			pname, errno, strerror(errno));
		return -3;
	}
    else if(pid == 0)
	{
		/*	CHILD PROCESS 에서는 해당 프로그램을 수행을 시도하고	*/
		freopen("/dev/null", "w", stdout);
		if( (ret = execl(pname, STR_TSW_COM[idx], (char*)NULL)) == -1)
		{
			log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN execl(pname[%s]) errno[%d-%s]", __FILE__, __FUNCTION__, __LINE__,
				pname, errno, strerror(errno));
			exit(-4);
		}
		exit(0);
	}
	else
	{
		/*	PARENT PROCESS에서는 단순히 CHILD PROCESS의 PID를 RETURN	*/
#ifdef DEBUG
		log_print(LOGN_DEBUG, "%s START WITH %d", STR_TSW_COM[idx], pid);
#endif
		fidb->mpswinfo[idx].pid		= pid;
		fidb->mpswinfo[idx].when	= time(NULL);

		return pid;
	}
}

/*******************************************************************************
 * CHECK PROCESS STATUS FUNCTION
*******************************************************************************/
int check_software_status(int *re_run, int *pid)
{
	int			i, fd, idx, status[MAX_NTAF_SW_BLOCK];
	DIR			*dirp;
	char		pname[PROC_NAME_LEN_128*4], tempbuf[256];
	time_t		now;

	/***************************************************************************
	* STATUS VALUE INITIALIZATION : DEAD
	***************************************************************************/
	for(i = 0; i < dCurrBlockCnt; i++)
	{
		if( (fidb->mpsw[i]==STOP)||(fidb->mpsw[i]==MASK))
			status[i] = 9;
		else
			status[i] = STOP;
	}

	/***************************************************************************
	* SET INITIAL VALUE : NOT RERUN
	***************************************************************************/
	for(i = 0; i < dCurrBlockCnt; i++)
	{
		re_run[i]	= 0;
		pid[i]		= 0;
	}

	if( (dirp=opendir(PROC_PATH)) == (DIR*)NULL)
	{
		log_print(LOGN_CRI, "\n\tCAN'T ACCESS PROCESS DIRECTORY(%s)\n", PROC_PATH);
		exit(0);
	}

	for(i = 0; i < dCurrBlockCnt; i++)
	{
		sprintf(tempbuf, "%s/%lld/cmdline", PROC_PATH, fidb->mpswinfo[i].pid);
		if( (fd = open(tempbuf, O_RDONLY)) < 0)
		{
			pid[i] = 0;
			continue;
		}
		else
		{
			memset(pname, 0x00, PROC_NAME_LEN_128);
			if(read(fd, pname, PROC_NAME_LEN_128-1) < 0)
			{
				close(fd);
				continue;
			}
			else
			{
				close(fd);
				pname[PROC_NAME_LEN_128-1] = 0x00;

				if(strlen(pname) < 20)
				{
					idx = id_of_process(pname);

					if( (idx>=0)&&(idx<dCurrBlockCnt))
					{
						log_print(LOGN_INFO, "PNAME[%s] IDX[%d] VER[%s]", pname, idx, version->szVersion[idx]);
						status[idx] = NORMAL;
						pid[idx] = fidb->mpswinfo[idx].pid;
					}
				}
			}
		}
	}
    closedir(dirp);

	now = time(&now);
	for(i = 1; i < dCurrBlockCnt; i++)
	{
		if( (status[i] == STOP) && (fidb->mpsw[i] > MASK))
		{
			/*** PID가 유효하지 않으면서 상태값이 NORMAL이상인 경우 ***********/
			if(fidb->mpsw[i] != CRITICAL)
			{
				Send_AlmMsg(LOCTYPE_PROCESS, INVTYPE_USERPROC, i, CRITICAL, fidb->mpsw[i]);
				log_print(LOGN_DEBUG, "S/W CRITICAL STATUS[INVALID PID] : [%s]", STR_TSW_COM[i]);
			}

			usleep(200000);

			if(fidb->mpswinfo[i].pid != 0)
				re_run[i] = 1;

			fidb->mpsw[i] = CRITICAL;
		}
		else if(status[i] == NORMAL)
		{
			/*** 유효한 PID를 가지고 있는 경우 ********************************/
			if( (fidb->mpsw[i]==STOP) || (fidb->mpsw[i]==CRITICAL))
			{
				/*** PID는 유효하나, 상태값이 다르게 설정되어 있는 경우 *******/
				Send_AlmMsg(LOCTYPE_PROCESS, INVTYPE_USERPROC, i, NORMAL, fidb->mpsw[i]);
				log_print(LOGN_DEBUG, "S/W NORMAL STATUS[VALID PID] : [%s]", STR_TSW_COM[i]);

				fidb->mpsw[i]			= NORMAL;
				fidb->mpswinfo[i].pid	= pid[i];
				fidb->mpswinfo[i].when	= now;
			}
		}
	}

	return 1;
}


/*******************************************************************************
 * CHECK PROCESS STATUS MAIn FUNCTION
*******************************************************************************/
void check_software(void)
{
	int		i, err, fd, re_run[MAX_NTAF_SW_BLOCK], pid[MAX_NTAF_SW_BLOCK];
	time_t	now;
	char	tempbuf[256];
	DIR		*dirp;

	/***************************************************************************
	* CHECK SW STATUS
	***************************************************************************/
	if( (err = check_software_status(re_run, pid)) < 0)
	{
		log_print(LOGN_DEBUG, "[FUNCTION][CHECK_SOFTWARE_STATUS] FAIL");
		return;
	}

	now = time(&now);
	/***************************************************************************
	* AUTO RERUN PROCESS
	***************************************************************************/
	for(i = 1; i < dCurrBlockCnt; i++)
	{
		if(re_run[i] == 1)
		{
			/*** CASE AURO RERUN PROCESS **************************************/
			if( (err = auto_restart_the_process(i)) <= 0)
			{
				log_print(LOGN_CRI, "AUTO RESTART FAIL [%s]", STR_TSW_COM[i]);

				fidb->mpsw[i]			= CRITICAL;
				fidb->mpswinfo[i].when	= now;
			}
			else
			{
				log_print(LOGN_CRI, "PROCESS [%s][%d][%ld] RESTART", STR_TSW_COM[i], err, now );

				fidb->mpswinfo[i].pid	= err;
				fidb->mpswinfo[i].when	= now;
			}
		}
	}

	sleep(1);

	if( (dirp = opendir(PROC_PATH)) == (DIR*)NULL)
	{
		log_print(LOGN_CRI, "CAN'T ACCESS PROCESS DIRECTORY (%s)", PROC_PATH);
		exit(-1);
	}

	for(i = 1; i < dCurrBlockCnt; i++)
	{
		/***********************************************************************
		* PID는 있는데, 상태값은 CRITICAL인 경우.
		***********************************************************************/
		if( (fidb->mpswinfo[i].pid != 0) && (fidb->mpsw[i] == CRITICAL))
		{
			log_print(LOGN_DEBUG, "[CHECK STATUS] [%s] [%lld] [0x%02x]", STR_TSW_COM[i], fidb->mpswinfo[i].pid, fidb->mpsw[i]);

			sprintf(tempbuf, "%s/%lld/cmdline", PROC_PATH, fidb->mpswinfo[i].pid);

			/*** PID가 유효한지 검사 ******************************************/
			if( (fd = open(tempbuf, O_RDONLY)) == -1)
				fidb->mpsw[i] = CRITICAL;
			else
			{
				Send_AlmMsg(LOCTYPE_PROCESS, INVTYPE_USERPROC, i, NORMAL, fidb->mpsw[i]);
				log_print(LOGN_DEBUG, "S/W NORMAL STATUS[VALID PID] : [%s]", STR_TSW_COM[i]);

				/*** PID가 유효하면 상태값을 NORMAL ***************************/
				fidb->mpsw[i] = NORMAL;
			}
		}
	}

	closedir(dirp);
}
