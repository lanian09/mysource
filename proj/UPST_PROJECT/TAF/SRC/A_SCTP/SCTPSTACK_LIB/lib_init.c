/******************************************************************************* 
        @file   lib_init.c
 *      - A_SCTP 프로세스를 초기화 하는 함수들
 *
 *      Copyright (c) 2006~ by Upresto Inc. Korea
 *      All rights reserved
 *
 *      $Id: lib_init.c,v 1.2 2011/09/06 02:07:45 dcham Exp $
 *
 *      @Author     $Author: dcham $
 *      @version    $Revision: 1.2 $
 *      @date       $Date: 2011/09/06 02:07:45 $
 *      @ref        lib_init.c
 *
 *      @section    Intro(소개)
 *      - ASSOCIATION 관련 MMDB 
 *
 *      @section    Requirement
 *
*******************************************************************************/

/**A.1*  File Inclusion *******************************************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sctpstack.h>
#include "loglib.h"

/**B.1*  Definition of New Constants ******************************************/
#define MAX_WRITEBUF_SIZE  1024*1000

#define	SHM_CREATE	1
#define	SHM_EXIST	2
#define	SHM_ERROR 	-1
#define SHM_PARAM	-2
#define MAP_CREATE	SHM_CREATE
#define MAP_EXIST	SHM_EXIST
#define MAP_ERROR	SHM_ERROR
#define MAP_PARAM	SHM_PARAM

/**B.2*  Definition of New Type  **********************************************/

/**C.1*  Declaration of Variables  ********************************************/
int                  gdASSOSHMID;    /* SHM ID */
pASSOSTACK_TABLE  	 pstASSOSTACKTbl; /* MMDB&LIST TABLE POINTER */
pASSO_TABLE          pstAssoTbl;     /* ASSOCIATION MMDB POINTER */
pSTACK_TBL           pstStackTbl;    /* STACK LIST TABLE POINTER */

/**D.1*  Definition of Functions  *********************************************/

/*******************************************************************************

*******************************************************************************/
int dInitMMAPMMDB(char *szMMAPFile)
{
    int  err;

    /* 공유메모리를 생성하고 Point 지정한다*/
    if ((err = dMmapGet(szMMAPFile)) < 0) {
        log_print(LOGN_CRI,"MMDBSESS MMDB[%s] FAILED RET[%d]", szMMAPFile, err);
    } else if (err == MAP_CREATE) { /* shared memory가 처음 생성된 경우 */
        log_print(LOGN_CRI,"SESS MMDB[%s] Data must be initialized", szMMAPFile);
		Init_SessDB();
		InitStackList();
        log_print(LOGN_CRI,"SESS MMDB MMAP Data are initialized");
    } else if (err == MAP_EXIST) {  /* shared memory가 이미 있는 경우 */
        log_print(LOGN_CRI,"SESS MMDB MMAP Data are reloaded");
    }

    return err;
}


/*******************************************************************************

*******************************************************************************/
int dInitSHMMMAB(int dMMDBSHMKey)
{
    int  err;

    /* 공유메모리를 생성하고 Point 지정한다*/
    if ((err = dShmGet(dMMDBSHMKey)) < 0) {
        log_print(LOGN_CRI,"MMDBSESS MMDB[%d] FAILED RET[%d]", dMMDBSHMKey, err);
    } else if (err == SHM_CREATE) { /* shared memory가 처음 생성된 경우 */
        log_print(LOGN_CRI,"SESS MMDB[%d] Data must be initialized", dMMDBSHMKey);
		Init_SessDB();
		InitStackList();
        log_print(LOGN_CRI,"SESS MMDB SHM Data are initialized");
    } else if (err == SHM_EXIST) {  /* shared memory가 이미 있는 경우 */
        log_print(LOGN_CRI,"SESS MMDB SHM Data are reloaded");
    }

    return err;
}


/*******************************************************************************

*******************************************************************************/ 
int dMmapGet(char *szPath)
{
	struct stat stStat;
	int         i, fd, dRet, dLen=0, dSize, dCount;
	char		szTemp[MAX_WRITEBUF_SIZE];

	pstASSOSTACKTbl = (pASSOSTACK_TABLE)&szTemp[0];
	if ((fd = open (szPath, O_CREAT|O_RDWR|O_EXCL, 0644)) < 0) {

		if (errno == EEXIST) 
		{
			log_print(LOGN_CRI,"MMAP OPEN EXIST [%s]", szPath);
			/* compare file size & required size */
			if (stat (szPath, &stStat) < 0) {
				log_print(LOGN_CRI, "[MmapGet] stat failed, error[%d] = [%s]",
					errno, strerror (errno));
				return MAP_ERROR;
			}

			if ((stStat.st_size != ASSOSTACK_TABLE_SIZE)) { /* FILE SIZE DIFF */
				log_print(LOGN_CRI, "MMAP FILE SIZE ERROR :: FILE SIZE[%d] DEF SIZE[%ld], ASSOSTACK_TABLE_SIZE[%ld]", 
					dLen, stStat.st_size, ASSOSTACK_TABLE_SIZE);
				if ((fd = open (szPath, O_RDWR|O_TRUNC)) < 0) {
					log_print(LOGN_CRI,"[MmapGet] stat failed, error[%d] = [%s]",
						errno, strerror(errno));
					return MAP_ERROR;
				}

				dCount = ASSOSTACK_TABLE_SIZE/MAX_WRITEBUF_SIZE;
				dSize = 0;
				log_print(LOGN_CRI,
					"START FILE MAKE :: WRITE SIZE[%d] T[%ld] COUNT[%d]", 
					dSize, ASSOSTACK_TABLE_SIZE, dCount);
				for(i = 0 ; i < dCount; i++)
				{
					dLen = write (fd, (char *)pstASSOSTACKTbl, MAX_WRITEBUF_SIZE);
					if (dLen == -1) {
						log_print(LOGN_CRI,"[MmapGet] write for failed, error[%d][%s]Len[%d]Size[%d]",
							errno, strerror(errno), dLen, dSize);
						return MAP_ERROR;
					}
					dSize += dLen;
				}

				dCount = ASSOSTACK_TABLE_SIZE%MAX_WRITEBUF_SIZE;
				if(dCount != 0)
				{
					log_print(LOGN_CRI,
						"LAST FILE MAKE :: WRITE SIZE[%d] T[%ld] LAST[%d]", 
						dSize, ASSOSTACK_TABLE_SIZE, dCount);
					dLen = write (fd, (char *)pstASSOSTACKTbl, dCount);
					if (dLen == -1) {
						log_print(LOGN_CRI,"[MmapGet] write last failed, error[%d][%s]Len[%d]Size[%d]",
							errno, strerror(errno), dLen, dSize);
						return MAP_ERROR;
					}
					dSize += dLen;
				}

				if (dSize != ASSOSTACK_TABLE_SIZE) {
					log_print(LOGN_CRI,"[MmapGet] write failed, error[%d] = [%s]Len[%d]",
						errno, strerror (errno), dLen);
					return MAP_ERROR;
				}

				log_print(LOGN_CRI,"MAKED FILE :: FILE M SIZE[%d] T[%ld]", 
					dSize, ASSOSTACK_TABLE_SIZE);

				dRet = MAP_CREATE;
			}
			else { /* FILE SIZE SAME */
				if ((fd = open (szPath, O_RDWR)) < 0) {
					log_print(LOGN_CRI,"[MmapGet] EXIST & Open Failed, error[%d] = [%s] Size[%d]",
						errno, strerror (errno), dSize);
					return MAP_ERROR;
				}

				dRet = MAP_EXIST;
			}
		}  /* File Exist End */
		else 
		{
			log_print(LOGN_CRI,"[MmapGet] Open failed, error[%d] = [%s]",
				errno, strerror (errno));
			return MAP_ERROR;
		} /* File ETC End */
	}
	else /* FILE CREATE */
	{
		dCount = ASSOSTACK_TABLE_SIZE/MAX_WRITEBUF_SIZE;
		dSize = 0;
		log_print(LOGN_CRI,
			"START FILE CREATE :: WRITE SIZE[%d] T[%ld] COUNT[%d]", 
			dSize, ASSOSTACK_TABLE_SIZE, dCount);
		for(i = 0 ; i < dCount; i++)
		{
			dLen = write (fd, (char *)pstASSOSTACKTbl, MAX_WRITEBUF_SIZE);
			if (dLen == -1) {
				log_print(LOGN_CRI,"[MmapGet] write C for failed, error[%d][%s]Len[%d]Size[%d]",
					errno, strerror(errno), dLen, dSize);
				return MAP_ERROR;
			}
			dSize += dLen;
		}

		dCount = ASSOSTACK_TABLE_SIZE%MAX_WRITEBUF_SIZE;
		if(dCount != 0)
		{
			log_print(LOGN_CRI,
				"LAST FILE CREATE :: WRITE SIZE[%d] T[%ld] LAST[%d]", 
				dSize, ASSOSTACK_TABLE_SIZE, dCount);
			dLen = write (fd, (char *)pstASSOSTACKTbl, dCount);
			if (dLen == -1) {
				log_print(LOGN_CRI,"[MmapGet] write C last failed, error[%d][%s]Len[%d]Size[%d]",
					errno, strerror(errno), dLen, dSize);
				return MAP_ERROR;
			}
			dSize += dLen;
		}

		if (dSize != ASSOSTACK_TABLE_SIZE)
		{
			log_print(LOGN_CRI,"[MmapGet] write failed, error[%d] = [%s] Size[%d]",
				errno, strerror (errno), dSize);
			return MAP_ERROR;
		}

		log_print(LOGN_CRI,"MAKED FILE :: FILE M SIZE[%d] T[%ld]", dSize, ASSOSTACK_TABLE_SIZE);
		dRet = MAP_CREATE;
	}

	if(	(pstASSOSTACKTbl =
		(pASSOSTACK_TABLE)mmap(NULL, ASSOSTACK_TABLE_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0))
		== MAP_FAILED)
	{
		log_print(LOGN_CRI,"[MmapGet] mmap failed, error[%d] = [%s]", errno, strerror(errno));
		return MAP_ERROR;
	}

	pstAssoTbl = &pstASSOSTACKTbl->stAssoTbl;
	pstStackTbl = &pstASSOSTACKTbl->stStackTbl;

	return dRet;

}	

/*******************************************************************************

*******************************************************************************/
int dShmGet(int dSHMKey)
{
	int	dRet;

	gdASSOSHMID = shmget(dSHMKey, ASSOSTACK_TABLE_SIZE, IPC_EXCL|IPC_CREAT|0644) ;
	if (gdASSOSHMID >= 0){ /* 새롭게 shared memory가 생성이 된 경우 */
        pstASSOSTACKTbl = (pASSOSTACK_TABLE)shmat(gdASSOSHMID, 0, 0);
		if((void *)pstASSOSTACKTbl == (void *)-1) 
		{
			log_print(LOGN_CRI, "MMDB SHMAT Level1 ERROR %d %s", errno, strerror(errno));
			return SHM_ERROR;
		}

		dRet = SHM_CREATE;
	} 
	else 
	{
		if (errno == EEXIST) /* 기존에 shared memory가 생성되어 있었던 경우 */
		{
    		gdASSOSHMID = shmget(dSHMKey, ASSOSTACK_TABLE_SIZE, IPC_CREAT|0644) ;
    		if ((int)gdASSOSHMID < 0) 
			{
        		log_print(LOGN_CRI,
            		"MMDB SHMGET Level2 ERROR %d %s", errno, strerror(errno));
        		return SHM_ERROR;
    		}

    		/* memory attatch를 할수 없는 경우 */
    		pstASSOSTACKTbl = (pASSOSTACK_TABLE)shmat(gdASSOSHMID, 0, 0);
    		if((void *)pstASSOSTACKTbl == (void *)-1) 
			{
        		log_print(LOGN_CRI, 
					"MMDB SHMAT Level3 ERROR %d %s", errno, strerror(errno));
				return SHM_ERROR;
			}

			dRet = SHM_EXIST;

		} 
		else /* shared memory를 생성할수 없는 경우 */
		{
			log_print(LOGN_CRI,
				"MMDB SHMGET Level1 ERROR %d %s", errno, strerror(errno));
			return SHM_ERROR;
		}
	}

	pstAssoTbl = &pstASSOSTACKTbl->stAssoTbl;
	pstStackTbl = &pstASSOSTACKTbl->stStackTbl;

	return dRet;
}

/*
* $Log: lib_init.c,v $
* Revision 1.2  2011/09/06 02:07:45  dcham
* *** empty log message ***
*
* Revision 1.1.1.1  2011/08/29 05:56:42  dcham
* NEW OAM SYSTEM
*
* Revision 1.1  2011/08/05 02:38:57  uamyd
* A_SCTP modified
*
* Revision 1.2  2011/01/11 04:09:05  uamyd
* modified
*
* Revision 1.1.1.1  2010/08/23 01:13:04  uamyd
* DQMS With TOTMON, 2nd-import
*
* Revision 1.1  2009/05/27 07:42:36  dqms
* Makefile
*
* Revision 1.1  2009/05/13 11:42:49  upst_cvs
* NEW
*
* Revision 1.1  2008/01/11 12:09:02  pkg
* import two-step by uamyd
*
* Revision 1.2  2007/04/29 13:09:34  doit1972
* CVS LOG 정보 추가
*
*/
