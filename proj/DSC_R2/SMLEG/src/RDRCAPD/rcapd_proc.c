#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <errno.h>
#include <string.h>
#include "rcapd.h"
#include "utillib.h"

TAIL    *g_pTail[2];

void dumpOpenFile(TAIL  *f)
{
	dAppLog(LOG_DEBUG, "fd = %p\n", f->fp);
	dAppLog(LOG_DEBUG, "fname = %s\n", f->fileName);
	dAppLog(LOG_DEBUG, "fd = %d\n", f->fd);
	dAppLog(LOG_DEBUG, "size = %d\n", f->revsize);
}

unsigned getLineBufLen( char * p, char * end )
{
	unsigned cnt = 0;
	for( ; p != end; ++p ) {
		if( *p != '\n' ) cnt++;
		else break;
	}
	return cnt;
}

int getBackupFileName (char *fname, char *bkname)
{
	char *pos = NULL;
	int offset = 0;

	pos = strstr(fname, "tmp");
	if (pos==NULL) return 0;
	offset = pos - fname;
	strncpy(bkname, fname, offset-1);
	return 1;
}

void theApp (void)
{
	//TAIL    *pTail[2], tmpTail;
	//TAIL    tmpTail;

	//int 	loopCnt = 0;
	int 	i;
	int 	isTrsRdr=0, isBlkRdr=0, isRdrExist=0;
	char 	tmpFileName[128], bkupFileName[128];
	char	trsPath[FILE_PATH_SHORT_LEN], blkPath[FILE_PATH_SHORT_LEN];
 
	g_pTail[0]=NULL; g_pTail[1]=NULL;
	//memset(&tmpTail, 0x00, sizeof(tmpTail));
	memset(tmpFileName, 0x00, sizeof(tmpFileName)); memset(bkupFileName, 0x00, sizeof(bkupFileName));
	memset(trsPath, 0x00, sizeof(trsPath)); memset(blkPath, 0x00, sizeof(blkPath));

	/* MAIN LOOP */
	while (JiSTOPFlag)
	{
		keepalivelib_increase ();

		/** TRANSACTION RDR **********************************************************************/
		if (!isTrsRdr)
		{
			if (findTargetFile (RDR_TYPE_TRANSACTION, tmpFileName) < 0) {
				dAppLog(LOG_DEBUG, "[theAPP] transaction rdr : xxx.csv.tmp file not found"); // 10.15 by jjinri : LOG_CRI -> LOG_DEBUG
				goto TMP_TRSFILE_OPEN_FAIL;
			}
			sprintf(trsPath, "%s%s", TRANSACTION_RDR_PATH, tmpFileName);
			dAppLog(LOG_CRI, "TRANSACTION PATH = %s", trsPath);	

			if((g_pTail[0] = openTail (trsPath)) == NULL) {
				dAppLog(LOG_CRI, "[theAPP] trsPath opentail failed");
				goto TMP_TRSFILE_OPEN_FAIL;
			}
			isRdrExist = RDR_EXIST_TRS;
			sprintf(g_pTail[0]->fileName, "%s", tmpFileName);
			sprintf(g_pTail[0]->fullFName, "%s", trsPath);
			g_pTail[0]->operflag = isTrsRdr = 1;
			g_pTail[0]->buf = (char *)malloc(_LINE_BUFFER_LEN);
			g_pTail[0]->fdStat = _FD_STAT_TMP;
			fseek(g_pTail[0]->fp, 0, SEEK_END);
		}
TMP_TRSFILE_OPEN_FAIL:

		/** BLOCK RDR ****************************************************************************/
		if (!isBlkRdr)
		{
			if (findTargetFile (RDR_TYPE_BLOCK, tmpFileName) < 0) {
				dAppLog(LOG_DEBUG, "[theAPP] transaction rdr : xxx.csv.tmp file not found"); // 10.15 by jjinri : LOG_CRI -> LOG_DEBUG
				goto TMP_TRSFILE_OPEN_FAIL;
			}
			sprintf(blkPath, "%s%s", BLOCK_RDR_PATH, tmpFileName);
			dAppLog(LOG_CRI, "BLOCK PATH = %s", blkPath);	

			if((g_pTail[1] = openTail (blkPath)) == NULL) {
				dAppLog(LOG_CRI, "[theAPP] blkPath opentail failed");
				goto TMP_BLKFILE_OPEN_FAIL;
			}
			if (isTrsRdr) isRdrExist = RDR_EXIST_BOTH;
			else isRdrExist = RDR_EXIST_BLK;
			sprintf(g_pTail[1]->fileName, "%s", tmpFileName);
			sprintf(g_pTail[1]->fullFName, "%s", blkPath);
			g_pTail[1]->operflag = isBlkRdr = 1;
			g_pTail[1]->buf = (char *)malloc(_LINE_BUFFER_LEN);
			g_pTail[1]->fdStat = _FD_STAT_TMP;
			fseek(g_pTail[1]->fp, 0, SEEK_END);
		}
TMP_BLKFILE_OPEN_FAIL:

		/*****************************************************************************************/
		if ((isTrsRdr) || (isBlkRdr)) {
			readTail (g_pTail, _LINE_BUFFER_LEN, 1, isRdrExist);
			usleep(10000); // 10.15 by jjinri : sleep(1) -> usleep(10000)
		}
		else
			usleep(10000); continue; // 10.15 by jjinri : usleep(1) -> usleep(10000)

	} /* while-loop end */

	for (i=0;i<2;i++) {
		free(g_pTail[i]->buf);
		closeTail(g_pTail[i]);
	}
	finProc();
}

