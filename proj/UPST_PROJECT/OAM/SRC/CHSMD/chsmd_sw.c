/**A.1 * File Include *********************************************************/

/* SYS HEADER */
#include <stdio.h>
#include <string.h>		/* STRCMP(3) */
#include <stdlib.h>		/* EXIT(3) */
#include <errno.h>		/* ERRNO */
#include <unistd.h>		/* SLEEP(3) */
#include <ctype.h>		/* ISSPACE(3) */
#include <sys/types.h>  /* CLOSEDIR(3), OPEN(2) */
#include <dirent.h>		/* CLOSEDIR(3) */
#include <sys/stat.h>   /* OPEN(2) */
#include <fcntl.h>   	/* OPEN(2) */
#include <signal.h>		/* SIGNAL(2) */
/* LIB HEADER */
#include "filedb.h"
#include "loglib.h"
#include "commdef.h"
/* PRO HEADER */
#include "path.h"
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
#include "almstat.h"
/* LOC HEADER */
#include "chsmd_sw.h"

/**B.1*  Definition of New Constants ******************************************/
/**B.2*  Definition of New Type  **********************************************/
/**C.1*  Declaration of Variables  ********************************************/
int  dCurrBlockCnt;
char STR_TSW_COM[MAX_SW_COUNT][30];

extern pst_NTAM fidb;
/**D.1*  Definition of Functions  *********************************************/

/*******************************************************************************
 GET PROCESS NAME USING INDEX
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
 GET PROCESS INDEX USING NAME
*******************************************************************************/
int id_of_process(char *name)
{
    int     i;

    for(i=0 ; i<dCurrBlockCnt ; i++)
	{
        if( !strcmp(name, STR_TSW_COM[i]) )
		{
            return i;
		}
	}

    return -1;
}

/*******************************************************************************
 GET PROCESS ID USING PROCESS NAME
*******************************************************************************/
int get_proc_id(char *name)
{
    int         fd;
    DIR         *dirp;
    struct dirent *direntp;
    char        pname[PROC_NAME_LEN];
    char        tempbuf[BUF_LEN];


    if((dirp = opendir(PROC_PATH)) == (DIR *)NULL)
    {
        log_print(LOGN_DEBUG,"CAN'T ACCESS PROCESS DIRECTORY (%s)", PROC_PATH);
        exit(0);
    }

    while((direntp = readdir(dirp)) != NULL)
    {
        if(!strcmp(direntp->d_name, PARENT_PATH) ||
           !strcmp(direntp->d_name, HOME_PATH)) continue;

        if( !atoi(direntp->d_name) )
        {
            continue;
        }

        sprintf(tempbuf, "%s/%s/cmdline", PROC_PATH, direntp->d_name);


        fd = open(tempbuf, O_RDONLY);
        if(fd < 0)
        {
            close(fd);
            continue;
        }

        memset( pname, 0x00, PROC_NAME_LEN );
		if( read( fd, pname, PROC_NAME_LEN-1 ) < 0 )
		{
			close(fd);
			continue;
		}
		else
		{
			close(fd);
		}

		pname[PROC_NAME_LEN-1] = 0x00;

        if( !strcmp(name, pname) )
        {
			log_print(LOGN_DEBUG, "F=%s:%s.%d: direntp->d_name[%s] name[%s] pname[%s]", __FILE__, __FUNCTION__, __LINE__, direntp->d_name, name, pname);
			closedir(dirp);
            return atoi(direntp->d_name);
        }
    }
    closedir(dirp);

    return -1;
} /* get_proc_id */


/*******************************************************************************
 RESTART PROCESS USING PROCESS INDEX
*******************************************************************************/
int auto_restart_the_process(int idx)
{
    int         dPid;
    struct stat statbuf;
    char        pname[BUF_LEN];
    int         ret ;
    time_t      temp_now;

    /***************************************************************************
     * CHECK INDEX RANGE
    ***************************************************************************/
    if( idx < 0 || idx >= dCurrBlockCnt)
        return -1;

    /***************************************************************************
     * ALREADY RUNNING, REMOVE PROCESS INFORATION
    ***************************************************************************/
    dPid = get_proc_id(STR_TSW_COM[idx]);
    if(dPid > 0) {
		log_print(LOGN_WARN, "%s ALREADY ALIVE WITH %d", STR_TSW_COM[idx], dPid );
        return dPid;
    }

    sprintf(pname,"%s%s", APP_HOME_BIN, STR_TSW_COM[idx]);

    if(stat(pname, &statbuf) < 0) return -1;

    /* ### CREATE PROCESS ######*/
    signal(SIGCHLD, SIG_IGN);
    dPid = fork();

    log_print(LOGN_DEBUG,"[AUTO RESTART] [NEW PID] [%d]", dPid );

    /* fork에서 실패하면 종료한다. */
    if(dPid < 0) return -1;

    else if (dPid == 0)
    {
        /* Child 프로세스에서는 해당 프로그램 수행을 시도 */
		log_print(LOGN_DEBUG,"CHILD PROCESS INVOKED [%s]", pname);
        (void)freopen("/dev/null", "w", stdout);
        (void)freopen("/dev/null", "r", stdin);
        ret = execl(pname, STR_TSW_COM[idx], (char *)0);

        exit(0);
    }
    else
    {
        /* Parent 프로세스에서는 단순히 child의 pid를 리턴한다.*/
		log_print(LOGN_WARN,"%s START WITH %d", STR_TSW_COM[idx], dPid);

        fidb->mpswinfo[idx].pid = dPid;
        temp_now = time(&temp_now);
        fidb->mpswinfo[idx].when = temp_now;

        return dPid;
    }
}

/*******************************************************************************
 CHECK SOFTWARE STATUS, CHECK RERUN PROCESS
*******************************************************************************/
int check_software_status( int *re_run , int *Pid)
{
    int             i, fd;
	int				dRet;
    char            pname[PROC_NAME_LEN*4];
	time_t			now;
	int				status[MAX_SW_COUNT];
	char			tempbuf[256];

	/*** STATUS INITIALIZATION ************************************************/
	for( i=0; i< dCurrBlockCnt; i++ )
	{
		if( fidb->mpsw[i] == STOP || fidb->mpsw[i] == MASK )
			status[i] = 9;
		else
			status[i] = STOP;
	}

    /*** SET INITIAL VALUE: DEAD **********************************************/
    for(i = 0;i < dCurrBlockCnt;i++)
	{
        re_run[i] = 0;
        Pid[i] = 0;
	}

	/***************************************************************************
		IF VALID PID, STATUS->NORMAL
		IF INVALID PID, PID->0
	***************************************************************************/
	for(i = 0; i < dCurrBlockCnt; i++)
	{
		sprintf(tempbuf, "%s/%lld/cmdline", PROC_PATH, fidb->mpswinfo[i].pid);
		if( (fd = open(tempbuf, O_RDONLY)) < 0)
		{
			Pid[i] = 0;
			continue;
		}
		else
		{
			memset(pname, 0x00, PROC_NAME_LEN);
			if( read(fd, pname, PROC_NAME_LEN-1) < 0)
			{
				close(fd);
				continue;
			}
			else
			{
				close(fd);
				pname[PROC_NAME_LEN-1] = 0x00;
				if( strlen(pname) < 20)
				{
					dRet = id_of_process(pname);
					if( (dRet > 0) && (dRet < dCurrBlockCnt))
					{
						status[dRet] = NORMAL;
						Pid[dRet] = fidb->mpswinfo[dRet].pid;
					}
				}
			}
		}
	}

	now = time(&now);
	for( i = 1; i< dCurrBlockCnt; i++ )
	{
		/***********************************************************************
		 MUST BE ALIVE, DEAD. SET RERUN
		***********************************************************************/
		if( status[i] == STOP && fidb->mpsw[i] > MASK ) {
			if( fidb->mpsw[i] != CRITICAL )
				Send_CondMess( 1, LOCTYPE_PROCESS, INVTYPE_USERPROC, i, CRITICAL, fidb->mpsw[i] );

			if( fidb->mpswinfo[i].pid != 0 ) {
				log_print(LOGN_DEBUG,"[SET RERUN] [%s] [0x%02x]", STR_TSW_COM[i], fidb->mpsw[i]);
				re_run[i] = 1;
			}

			fidb->mpsw[i] = CRITICAL;
		} else if( status[i] == NORMAL ) {
			/*******************************************************************
			 ALIVE, DEAD IN SHARED MEMORY. SET SHARED MEMORY
			*******************************************************************/
			if( fidb->mpsw[i] == STOP || fidb->mpsw[i] == CRITICAL ) {
				Send_CondMess( 1, LOCTYPE_PROCESS, INVTYPE_USERPROC, i, NORMAL, fidb->mpsw[i] );

				fidb->mpsw[i] = NORMAL;
				fidb->mpswinfo[i].pid = Pid[i];
				fidb->mpswinfo[i].when = now;
			}
		}
	}

    return 1;
}

/*******************************************************************************
 CHECK SOFTWARE STATUS, AND THEN RESTART PROCESS
*******************************************************************************/
void check_software()
{
	int 	i, err;
	int 	re_run[MAX_SW_COUNT], dPid[MAX_SW_COUNT];
	time_t	now;

	int		dGetPid;

	if ((err = check_software_status( re_run, dPid )) < 0) {
		log_print(LOGN_WARN, "CHECK SW_STATUS ERROR %d", err);
		return;
	}

	for(i=1 ; i<dCurrBlockCnt ; i++)
	{
		if( re_run[i] == 1 )
			log_print(LOGN_WARN,"[AUTO RERUN PROCESS] [%s]", STR_TSW_COM[i]);
	}

	now = time(&now);

	for(i = 1 ; i < dCurrBlockCnt; i++)
	{
		if(re_run[i] == 1)
		{
			if( (err = auto_restart_the_process(i)) <= 0)
			{
				log_print(LOGN_WARN, "AUTO RESTART FAIL [%s]", STR_TSW_COM[i]);
				fidb->mpsw[i]			= CRITICAL;
				fidb->mpswinfo[i].when	= now;
			}
			else
			{
				fidb->mpswinfo[i].pid	= err;
				fidb->mpswinfo[i].when	= now;
				log_print(LOGN_CRI, "F=%s:%s.%d: %s fidb->mpswinfo[%d].pid[%lld]", __FILE__, __FUNCTION__, __LINE__,
					STR_TSW_COM[i], i, fidb->mpswinfo[i].pid);
			}
		}
	}

	sleep(1);
	for(i = 1; i < dCurrBlockCnt; i++)
	{
		if( (fidb->mpswinfo[i].pid != 0) && (fidb->mpsw[i] == CRITICAL))
		{
			if( (dGetPid = get_proc_id(STR_TSW_COM[i])) < 0)
			{
				log_print(LOGN_CRI, "F=%s:%s.%d: ERROR IN get_proc_id(STR_TSW_COM[%d][%s]) dGetPid[%d]", __FILE__, __FUNCTION__, __LINE__,
					i, STR_TSW_COM[i], dGetPid);
				fidb->mpsw[i] = CRITICAL;
			}
			else
			{
				log_print(LOGN_DEBUG, "[AUTO RESTART SUCCESS] [%s]", STR_TSW_COM[i]);
				Send_CondMess(1, LOCTYPE_PROCESS, INVTYPE_USERPROC, i, NORMAL, fidb->mpsw[i]);
				fidb->mpsw[i] = NORMAL;
			}
		}
	}
}
/******************************************************************************
	# mysqladmin ping
		mysqld is alive
******************************************************************************/
int dCheckMySQLD(void)
{
	FILE	*pp;
	char	cLineNo, sBuf[1024];
	size_t	szLen;

	if( (pp = popen("/usr/bin/mysqladmin ping", "r")) == NULL)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN open(\"%s\") errno[%d-%s]", __FILE__, __FUNCTION__, __LINE__,
			"/usr/bin/mysqladmin ping", errno, strerror(errno));
		return -1;
	}

	cLineNo	= 0;
	while(fgets(sBuf, 1024, pp) != NULL)
	{
		if(cLineNo > 1)
		{
			log_print(LOGN_CRI, "F=%s:%s.%d: cLineNo[%hd] is more than 1", __FILE__, __FUNCTION__, __LINE__, cLineNo);
			pclose(pp);
			return -2;
		}

		szLen = strlen(sBuf);
		while(isspace(sBuf[szLen-1]))
			sBuf[--szLen] = 0x00;

		if(strcmp(sBuf, "mysqld is alive") == 0)
		{
			if(fidb->cDBAlive == CRITICAL)
			{
				Send_CondMess(SYSTYPE_TAM, LOCTYPE_LOAD, INVTYPE_DBSTATUS, 1, NORMAL, CRITICAL);
				fidb->tEventUpTime[3] = time(NULL);
			}

			fidb->cDBAlive = NORMAL;
		}
		else
		{
			if(fidb->cDBAlive == NORMAL)
			{
				Send_CondMess(SYSTYPE_TAM, LOCTYPE_LOAD, INVTYPE_DBSTATUS, 1, CRITICAL, NORMAL);
				fidb->tEventUpTime[3] = time(NULL);
			}

			fidb->cDBAlive	= CRITICAL;
		}
	}
	pclose(pp);

	return 0;
}
