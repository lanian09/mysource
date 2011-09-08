#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/time.h> 
#include <sys/stat.h> 
#include <fcntl.h> 
#include <errno.h>
#include <string.h>
#include "rcapd.h"
#include "rcapd_file.h"


int findTargetFile (int rdrtype, char *fname)
{
	const int _LINE_BUF_SIZE = 1024;
	FILE *ptr=NULL;
	char cmd[128];
	char buf[_LINE_BUF_SIZE];
	int offset=0, sLen=0;

	memset(cmd, 0x00, sizeof(char)*128);
	if (rdrtype == RDR_TYPE_TRANSACTION) 
		sprintf(cmd, "ls -l %s | awk '{print $9}'|grep tmp", TRANSACTION_RDR_PATH);
	else
		sprintf(cmd, "ls -l %s | awk '{print $9}'|grep tmp", BLOCK_RDR_PATH);

	dAppLog(LOG_DEBUG, "%s] %s", getStrRdrType(rdrtype), cmd); 
	if ((ptr = popen(cmd, "r"))  == NULL) {
		dAppLog(LOG_DEBUG, "Target rdr file not found, err=%d(%s)"
				,errno, strerror(errno)); // 10.15 by jjinri : LOG_CRI -> LOG_DEBUG
		return -1;
	}
	
	if (fgets(buf, _LINE_BUF_SIZE, ptr) != NULL) {
		sLen = strlen(buf);
		sprintf(fname, "%s", buf);
		offset = strstr(fname, "tmp") - fname;
		dAppLog(LOG_DEBUG, "offset=%d, sLen=%d\n", offset, sLen);
		fname[offset+3]='\0';
	}
	else {
		dAppLog(LOG_DEBUG, "%s] Not found target file", getStrRdrType(rdrtype)); // 10.15 by jjinri : LOG_CRI -> LOG_DEBUG
		return -1;
	}

	pclose(ptr);
	dAppLog(LOG_CRI, "%s] find OK, filename is %s", getStrRdrType(rdrtype), fname); // 10.15 by jjinri : LOG_DEBUG -> LOG_CRI
	return 0;
}


/* 2008-12-24
 * by june
 * Desc   :
 *    tail을 적용할 파일을 연다.
 * Return :
 *    실패 NULL, 성공 시 TAIL 객체(구조체 포인터)를 리턴한다. 
 */ 
TAIL *openTail (char *fname) 
{ 
    TAIL *OTAIL; 
    struct stat fbuf; 

	OTAIL = (TAIL *)malloc(sizeof(TAIL)); 
   
	if ((OTAIL->fp = fopen(fname, "r")) == NULL) { 
		dAppLog(LOG_CRI, "FILE OPEN] fopen fail(%s), err=%d(%s)", fname, errno, strerror(errno));
		return NULL;
    } 
    if (stat(fname, &fbuf) < 0) {
		dAppLog(LOG_CRI, "FILE OPEN] stat fail(%s), err=%d(%s)", fname, errno, strerror(errno));
		return NULL;
	}
 
    strncpy(OTAIL->fullFName, fname, 127);
 
    OTAIL->fd = fileno(OTAIL->fp);
    OTAIL->revsize = fbuf.st_size;
	dAppLog(LOG_CRI, "FILE OPEN] %s : size=%d", fname, OTAIL->revsize); // 10.15 by jjinri LOG_DEBUG -> LOG_CRI
    return OTAIL;
} 

int	procRdrStatus (int result, int rdrtype, TAIL *pTail)
{
	int rtn=0, rLen=0;
	TAIL tmpTail;
	TAIL *pTmpTail;
	char tmpFileName[128];
	char bkupFileName[128];
	char path[FILE_PATH_SHORT_LEN];

	memset(&tmpTail, 0x00, sizeof(tmpTail));
	memset(tmpFileName, 0x00, sizeof(tmpFileName));
	memset(bkupFileName, 0x00, sizeof(bkupFileName));
	memset(path, 0x00, sizeof(path));

	pTail->rdrtype = rdrtype;

	if (result > 0) {
		rLen = getLineBufLen (pTail->buf, pTail->buf +_LINE_BUFFER_LEN);
		dAppLog(LOG_CRI, "%s] %s(%d)", getStrRdrType(rdrtype), pTail->buf, rLen);
		dSendMsg_RDRANA(dANAQid, rdrtype, pTail->buf, rLen);
		rtn = 1;
	}
	else if (result == 0) {
		rtn = 0;
	}
	else {

		switch (pTail->fdStat)
		{
			case _FD_STAT_TMP:

				if (pTail == NULL) { rtn = -1; break; }
				memcpy(&tmpTail, pTail, sizeof(TAIL));
				if (getBackupFileName (pTail->fileName, bkupFileName) == 0) {
					dAppLog(LOG_DEBUG, "%s] rdr xxx.cvs is not found(%s)", getStrRdrType(rdrtype), pTail->fileName); // 10.15 by jjinri LOG_DEBUG -> LOG_CRI
					rtn = -1; break;
				}

				if (rdrtype == RDR_TYPE_TRANSACTION)
					sprintf(path, "%s%s", TRANSACTION_RDR_PATH, bkupFileName);
				else
					sprintf(path, "%s%s", BLOCK_RDR_PATH, bkupFileName);

				dAppLog(LOG_CRI, "%s] path = %s", getStrRdrType(rdrtype), path);
				sprintf(pTail->fileName, "%s", bkupFileName);
				sprintf(pTail->fullFName, "%s", path);

				if((pTmpTail = openTail (path)) == NULL) {
					dAppLog(LOG_CRI, "%s] opentail2 failed, err=%d(%s)"
							, getStrRdrType(rdrtype), errno, strerror(errno));
					rtn = -1; break;
				}
				dAppLog(LOG_CRI, "%s] new create file size = %d, old file size = %d"
						, getStrRdrType(rdrtype), pTmpTail->revsize, tmpTail.revsize);
				sprintf(pTmpTail->fileName, "%s", bkupFileName);
				pTmpTail->operflag = 1;
				pTmpTail->revsize = tmpTail.revsize;
				pTmpTail->buf = (char *)malloc(_LINE_BUFFER_LEN);
				pTmpTail->fdStat = _FD_STAT_BKUP;
				fseek(pTmpTail->fp, tmpTail.revsize, SEEK_SET);
				if (rdrtype == RDR_TYPE_TRANSACTION) g_pTail[0] = pTmpTail;
				else g_pTail[1] = pTmpTail;
				
				closeTail(pTail); pTail = NULL;
				break;

			case _FD_STAT_BKUP:
				
				memset(tmpFileName, 0x00, sizeof(tmpFileName));
				if (findTargetFile (rdrtype, tmpFileName) < 0) {
					dAppLog(LOG_DEBUG, "%s] rdr xxx.cvs.tmp is not found", getStrRdrType(rdrtype)); // 10.15 by jjinri : LOG_CRI -> LOG_DEBUG
					rtn = -1; break;
				}
				if (rdrtype == RDR_TYPE_TRANSACTION)
					sprintf(path, "%s%s", TRANSACTION_RDR_PATH, tmpFileName);
				else
					sprintf(path, "%s%s", BLOCK_RDR_PATH, tmpFileName);

				sprintf(pTail->fileName, "%s", tmpFileName);
				sprintf(pTail->fullFName, "%s", path);

				if((pTmpTail = openTail (path)) == NULL) {
					dAppLog(LOG_CRI, "%s] opentail failed, err=%d(%s)"
								, getStrRdrType(rdrtype), errno, strerror(errno));
					rtn = -1; break;
				}
				dAppLog(LOG_CRI, "%s] path = %s", getStrRdrType(rdrtype), path);
				sprintf(pTmpTail->fileName, "%s", tmpFileName);
				pTmpTail->operflag = 1;
				pTmpTail->buf = (char *)malloc(_LINE_BUFFER_LEN);
				pTmpTail->fdStat = _FD_STAT_TMP;
				pTmpTail->revsize = 0; // 10.15 by jjinri : pTmpTail->revsize = 0 초기화 추가 
				if (rdrtype == RDR_TYPE_TRANSACTION) g_pTail[0] = pTmpTail;
				else g_pTail[1] = pTmpTail;
				
				closeTail(pTail); pTail = NULL;
				rtn = 1; break;

			default:
				dAppLog(LOG_CRI, "not defined status\n");
				rtn = -1; break;
				break;
		}
	}
	return rtn;
}


/* 2008-12-24
 * by june
 * Desc : 
 *    1) 파일로 부터 추가된 내용을 줄단위로 읽어 온다.
 *    2) 줄이 추가되면 size만큼 읽어서 buf에 복사한다.
 *    3) sec시간 간격으로 파일에 추가된 내용을 읽어온다.   
 *    4) 추가된 줄이 없을 경우 sec만큼 기다린다.
 *    5) 현재 파일크기가 이전 파일크기 보다 작다면
 *       a. 파일이 truncate() 되었다고 가정
 *       b. rewind() 후 첫라인 부터 다시 읽어 들인다.
 * Return : 
 *    성공 1, 실패 -1을 리턴한다.
 */ 
int readTail (TAIL **RTAIL, size_t size, int sec, int isRdrExist) 
{ 
    fd_set rfds; 
    struct timeval tv; 
    struct stat fbuf; 
	int retval=0, maxFD=0, rst=0;
    char *ret; 
 
    FD_ZERO(&rfds);
	switch (isRdrExist)
	{
	case RDR_EXIST_TRS:
		if (RTAIL[0] != NULL) FD_SET(RTAIL[0]->fd, &rfds); 
		maxFD = RTAIL[0]->fd+1;
		break;
	case RDR_EXIST_BLK: 
		if (RTAIL[1] != NULL) FD_SET(RTAIL[1]->fd, &rfds); 
		maxFD = RTAIL[1]->fd+1;
		break;
	case RDR_EXIST_BOTH:
		if (RTAIL[0] != NULL) FD_SET(RTAIL[0]->fd, &rfds); 
		if (RTAIL[1] != NULL) FD_SET(RTAIL[1]->fd, &rfds); 
		maxFD = (RTAIL[0]->fd > RTAIL[1]->fd) ? RTAIL[0]->fd+1 : RTAIL[1]->fd+1;
		break;
	default:
		return -1;
	}
    tv.tv_sec  = 0; tv.tv_usec = sec*10000; // 10.15 by jjinri : tv_sec=sec -> tv_usec = sec*10000
    
	retval = select(maxFD, &rfds, NULL, NULL, &tv); 
    if (retval) 
	{
		/** TRANSACTION RDR event ******************************************/
		if ((RTAIL[0] != NULL) && (RTAIL[0]->operflag)) {
			if (FD_ISSET(RTAIL[0]->fd, &rfds)) {
				
				if (stat(RTAIL[0]->fullFName, &fbuf) < 0) { 
					dAppLog(LOG_CRI, "TRANSACTION EVENT] file is not exist(%s) read size(%d)"
							, RTAIL[0]->fullFName, RTAIL[0]->revsize);
					rst = -1; goto TRSRDR_RST_PROC;
				} 

				if ((RTAIL[0]->fdStat == _FD_STAT_BKUP) && (RTAIL[0]->revsize == fbuf.st_size)) {
					dAppLog(LOG_CRI, "TRANSACTION EVENT] size is equal\n");
					rst = -1; goto TRSRDR_RST_PROC;
				}

				ret = fgets(RTAIL[0]->buf, size, RTAIL[0]->fp);
				if (ret == NULL) {
					RTAIL[0]->revsize = fbuf.st_size;
					rst = 0; goto TRSRDR_RST_PROC;
				} 
				//RTAIL[0]->revsize = fbuf.st_size; 
				RTAIL[0]->revsize += getLineBufLen (RTAIL[0]->buf, RTAIL[0]->buf +_LINE_BUFFER_LEN)+1; // 10.15 by jjinri : RTAIL[0]->revsize = getLineBufLen() ----> RTAIL[0]->revsize += getLineBufLen() + 1
				rst = 1; goto TRSRDR_RST_PROC;
			}
TRSRDR_RST_PROC:
			procRdrStatus (rst, RDR_TYPE_TRANSACTION, RTAIL[0]);
		}

		/** BLOCK RDR event ************************************************/
		if ((RTAIL[1] != NULL) && (RTAIL[1]->operflag)) {
			if (FD_ISSET(RTAIL[1]->fd, &rfds)) {

				if (stat(RTAIL[1]->fullFName, &fbuf) < 0) { 
					dAppLog(LOG_CRI, "BLOCK EVENT] file is n0t exist");
					rst = -1; goto BLKRDR_RST_PROC;
				} 

				if ((RTAIL[1]->fdStat == _FD_STAT_BKUP) && (RTAIL[1]->revsize == fbuf.st_size)) {
					dAppLog(LOG_CRI, "BLOCK EVENT] size is equal");
					rst = -1; goto BLKRDR_RST_PROC;
				}

				ret = fgets(RTAIL[1]->buf, size, RTAIL[1]->fp);
				if (ret == NULL) {
					RTAIL[1]->revsize = fbuf.st_size;
					rst = 0; goto BLKRDR_RST_PROC;
				} 
				//RTAIL[1]->revsize = fbuf.st_size; 
				RTAIL[1]->revsize += getLineBufLen (RTAIL[1]->buf, RTAIL[1]->buf +_LINE_BUFFER_LEN)+1;  // 10.15 by jjinri : RTAIL[0]->revsize = getLineBufLen() ----> RTAIL[0]->revsize += getLineBufLen() + 1
				rst = 1; goto BLKRDR_RST_PROC;
			}
BLKRDR_RST_PROC:
			procRdrStatus (rst, RDR_TYPE_BLOCK, RTAIL[1]);
		}
    } 
    else { 
		dAppLog(LOG_CRI, "reval < 0, err=%d(%s)", errno, strerror(errno));	
        return -1; 
    }
	return rst;
} 

/* 2008-12-24
 * by june
 * Desc : resource 반환
 */
void closeTail(TAIL *OTAIL) 
{    
    fclose(OTAIL->fp);
	free(OTAIL->buf);
    free(OTAIL); 
}

#if 0 
int main(int argc, char **argv)
{
	TAIL *tf;
	char *buf;
	int n, i;

	if((tf = openTail (argv[1])) == NULL) {
		perror("error ");
		exit(1);
	}

	buf = (char *)malloc(1024);
	int issleep = 0;

	while(1)
	{
		issleep = 0;
		n = readTail (tf, buf, 1023, 1);
		if(n > 0)
			printf("%d : %s", i, buf);

		issleep |= n;
		if (issleep == 0) {
			sleep(1);
			continue;
		}
	}
	closeTail(tf);
	free(buf);
}
#endif

#if 0
/* 2008-12-24
 * by june
 * Desc : tail을 적용할 대상 정보
 */ 
typedef struct _TAIL 
{ 
    FILE *fp;            /* 파일 스트림 */ 
    char filename[256];  /* 오픈한 파일 이름 */
    int  fd;             /* 파일 스트림에 대한 파일 지정자 */
    int  revsize;        /* 최근에 읽었던 파일 크기 */ 
} TAIL; 

#endif

#if 0
int checkDir(char *homeDir)
{
	struct stat pathStat;
	int eol;

	char path[BUFSIZ];
	char date[BUFSIZ];

	bzero(_log[idx].fname, BUFSIZ);
	bzero(path, BUFSIZ);
	bzero(date, BUFSIZ);

	sprintf(path, "%s/%s"   /* ~log/year            */
				,homeDir
				,fmttm((char*)"%Y", date)
	);
	if (stat(path, &pathStat)<0) {
		if(mkdir(path, 0755)==-1) return -1;
	}

	sprintf(path, "%s/%s"   /* ~log/year/month      */
					,path
					,fmttm((char*)"%m", date)
	);
	if(stat(path, &pathStat)<0) {
		if(mkdir(path, 0755)==-1) return -1;
	}

	sprintf(path, "%s/%s"   /* ~log/year/month/day  */
					,path
					,fmttm((char*)"%d", date)
	);
	if(stat(path, &pathStat)<0) {
		if(mkdir(path, 0755)==-1) return -1;
	}

	sprintf(_log[idx].fname, "%s/%s.%s.log"
						,path
						,_log[idx].procs
						,fmttm((char*)"%Y%m%d", date)
	);

	sprintf(_log[idx].err_fname, "%s/%s.%s.err"
						,path
						,_log[idx].procs
						,fmttm((char*)"%Y%m%d", date)
	);
	
	return 1;
}
#endif


