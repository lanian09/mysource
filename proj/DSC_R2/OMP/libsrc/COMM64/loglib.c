#include "loglib.h"


LogInformationTable	logInfoTbl[LOGLIB_MAX_OPEN_FILE];



/*------------------------------------------------------------------------------
------------------------------------------------------------------------------*/
int loglib_openLog (char *fname, int mode)
{
	int			i,logId;
	char		newFile[256];
	struct stat	fStatus;

	if (fname == NULL) {
		fprintf(stderr,"[loglib_openLog] log file name is null \n");
		return -1;
	}

	/* �ִ�� ���� �ִ� file�� ������ �ִ�.
	*/
	for (i=0; i<LOGLIB_MAX_OPEN_FILE; i++) {
		if (logInfoTbl[i].fp == NULL) {
			logId = i;
			break;
		}
	}
	if (i == LOGLIB_MAX_OPEN_FILE) {
		fprintf(stderr,"[loglib_openLog] can't open file any more \n");
		return -1;
	}

	/* log file�� ������ �����Ѵ�.
	*/
	strcpy(logInfoTbl[logId].fname, fname);
	logInfoTbl[logId].mode     = mode;
	logInfoTbl[logId].fp       = NULL;
	logInfoTbl[logId].lastTime = 0;

	/* LOGLIB_MODE_LIMIT_SIZE�� ��� "xxxx.0" ������ ������ Ȯ���ϰ�,
	** - ������ ������ �� logInfoTbl[logId].fp�� ����Ǿ� return�ǰ�,
	** - ������ limit �ʰ� ���θ� Ȯ���Ͽ� �ʰ����� �ʾ����� append ���� open�� ��
	**	logInfoTbl[logId].fp�� ����Ǿ� return�ǰ�,
	** - ������ �����ϰ� limit�� �ʰ��� ��� ���������� rename�� �� "xxxx.0"��
	**	�ٽ� �����Ͽ� logInfoTbl[logId].fp�� �����ϰ� return�ȴ�.
	*/
	if (mode & LOGLIB_MODE_LIMIT_SIZE) {
		if (loglib_checkLimitSize(logId) < 0)
			return -1;
	}

	/* LOGLIB_MODE_DAILY�� ���, ���ó�¥�� ����(xxxx.yyyy-mm-dd) append ����
	**	open�ϰ� logInfoTbl[logId].fp�� ����Ǿ� return�ȴ�.
	** - LOGLIB_MODE_7DAYS�� ��� 7���� ������ �����.
	*/
	if (mode & LOGLIB_MODE_DAILY) {
		if (loglib_checkDate(logId) < 0)
			return -1;
		logInfoTbl[logId].lastTime = time(0);
	}

	/* LOGLIB_MODE_HOURLY�� ���, ����ð� �̸��� ����(xxxx/yyyy/mm/ddhh) append ����
	**	open�ϰ� logInfoTbl[logId].fp�� ����Ǿ� return�ȴ�.
	** - directory�� ������ ���� ������ �� ������ �����Ѵ�.
	*/
	if (mode & LOGLIB_MODE_HOURLY) {
		if (loglib_checkTimeHour(logId) < 0)
			return -1;
		logInfoTbl[logId].lastTime = time(0);
	}
	/* LOGLIB_MODE_ONE_DIR�� ���, ����ð� �̸��� ����(xxxx/yyyy.mm.dd/hh) append ����
	**	open�ϰ� logInfoTbl[logId].fp�� ����Ǿ� return�ȴ�.
	** - directory�� ������ ���� ������ �� ������ �����Ѵ�.
	*/
	if (mode & LOGLIB_MODE_ONE_DIR) {
		if (loglib_checkOneDir(logId) < 0)
			return -1;
		logInfoTbl[logId].lastTime = time(0);
	}

	return logId;

} /** End of loglib_openLog **/



/*------------------------------------------------------------------------------
------------------------------------------------------------------------------*/
int loglib_closeLog (int logId)
{
	int		i;

	if (logInfoTbl[logId].fp != NULL){
		fflush(logInfoTbl[logId].fp);
		fclose(logInfoTbl[logId].fp);
	}

	logInfoTbl[logId].fp = NULL;

	return logId;

} /** End of loglib_closeLog **/



/*------------------------------------------------------------------------------
*
------------------------------------------------------------------------------*/
int logPrint ( int logId, char *fName, int lNum, char*fmt, ...)
{
	int		ret;
	char   	buff[4096], optBuf[256], tmp[80];
	va_list	args;
	struct timeval	curr;

    va_start(args, fmt);
    (void)vsprintf(buff, fmt, args);
    va_end(args);

	gettimeofday (&curr, NULL);

//  cjs
	if(logId > 16) return -1;

	if (logInfoTbl[logId].mode & LOGLIB_MODE_LIMIT_SIZE)
	{
		/* ���� �����ִ� ������ limit�� �ʰ��ߴ��� Ȯ���Ѵ�.
		** - �ʰ��� ��� ���������� rename�ϰ� �ٽ� ����.
		*/
		if (loglib_checkLimitSize(logId) < 0)
			return -1;
	}
	else if (logInfoTbl[logId].mode & LOGLIB_MODE_DAILY)
	{
		/* ���� �����ִ� ���������� ���� �ð��� ���Ͽ� ��¥�� �ٲ������ Ȯ���Ѵ�.
		** - ��¥�� �ٲ������ ���� ��¥ ������ ����.
		** - LOGLIB_MODE_7DAYS�� ��� 7���� ������ �����.
		*/
		if (loglib_checkDate(logId) < 0)
			return -1;
	}
	else if (logInfoTbl[logId].mode & LOGLIB_MODE_HOURLY)
	{
		/* ���� �����ִ� ���������� ���� �ð��� ���Ͽ� �ð�(hour)�� �ٲ������ Ȯ���Ѵ�.
		** - �ð��� �ٲ������ ���� �ð� ������ ����.
		** - ��/�� directory ���� �ؿ� ������ �����ǹǷ� directory�� ������ ���� �����.
		*/
		if (loglib_checkTimeHour(logId) < 0)
			return -1;
	}
	else if (logInfoTbl[logId].mode & LOGLIB_MODE_ONE_DIR)
	{
		/* ���� �����ִ� ���������� ���� �ð��� ���Ͽ� �ð�(hour)�� �ٲ������ Ȯ���Ѵ�.
		** - �ð��� �ٲ������ ���� �ð� ������ ����.
		** - ��/�� directory ���� �ؿ� ������ �����ǹǷ� directory�� ������ ���� �����.
		*/
		if (loglib_checkOneDir(logId) < 0)
			return -1;
	}
	else {
		return -1;
	}

	if ( logInfoTbl[logId].fp == NULL) {
		return -1;
	}

	strcpy (optBuf,"");
	/* LOGLIB_TIME_STAMP�̸� ���� �ð��� �Բ� ����Ѵ�.
	*/
	if (logInfoTbl[logId].mode & LOGLIB_TIME_STAMP) {
		strftime(tmp, 32, "%m-%d %T", localtime((time_t*)&curr.tv_sec));
		sprintf (optBuf, "[%s.%03d]", tmp, (curr.tv_usec/1000));
	}
	/* LOGLIB_FNAME_LNUM�̸� �ҽ� ���� �̸��� ���� ���� �Բ� ����Ѵ�.
	*/
	if (logInfoTbl[logId].mode & LOGLIB_FNAME_LNUM) {
		sprintf(tmp,"[%s:%d] ", fName, lNum);
		strcat (optBuf,tmp);
	}

	if (logInfoTbl[logId].fp != NULL) {
		ret = fprintf(logInfoTbl[logId].fp, "%s%s", optBuf, buff);
		logInfoTbl[logId].lastTime = curr.tv_sec;
	} else {
		return -1;
	}

	if (logInfoTbl[logId].mode & LOGLIB_FLUSH_IMMEDIATE) {
		fflush(logInfoTbl[logId].fp);
	}

	return ret;

} /** End of logPrint **/



/*------------------------------------------------------------------------------
* LOGLIB_MODE_LIMIT_SIZE�� ��� limit size�� �ʰ��ߴ��� Ȯ���Ѵ�.
* - �ʰ��� ��� xxxx.0�� 1��, 1�� 2��, ... �� ���� ������ rename�� �� "xxxx.0"��
*	�ٽ� open�Ѵ�.
------------------------------------------------------------------------------*/
int loglib_checkLimitSize (int logId)
{
	int		i;
	char	newFile[256], oldFile[256];
	struct stat     fStatus;
	
	/* ���� log_file�� �ڿ� .0�� �ٴ´�.
	*/
	sprintf (newFile, "%s.0", logInfoTbl[logId].fname);

	/* ������ ������ Ȯ���Ѵ�.
	*/
	if (stat(newFile, &fStatus) < 0) {
		if (errno != ENOENT) {
			fprintf(stderr,"[loglib_checkLimitSize] stat fail[%s]; errno=%d(%s)\n",newFile,errno,strerror(errno));
			return -1;
		}
		/* "xxxx.0" ������ �������� ������ �����ϰ� fp�� �������� ��ٷ� return�ȴ�.
		*/
		goto openNewLimitSizeFile;
	}

	/* ���� limit�� �ʰ����� ���� ���
	*/
	if (fStatus.st_size < LOGLIB_MAX_FILE_SIZE) {
		if (logInfoTbl[logId].fp == NULL) {
			/* ������ ���� ���� ���� ��� open�Ͽ� fp�� ������ �� return�ȴ�.
			*/
			goto openNewLimitSizeFile;
		}
		return 1;
	}

	/* limit�� �ʰ��� ���,
	** - �����ִ� ����("xxxx.0")�� close�ϰ� "xxxx.0"�� "xxxx.1"��, 1�� 2��, ... ��
	**	���� ������ rename�� �� "xxxx.0"�� �ٽ� open�Ѵ�.
	*/

	/* close current file */
	if (logInfoTbl[logId].fp != NULL) {
		fprintf (logInfoTbl[logId].fp, "\n\n----- FILE CLOSE DUE TO SIZE LIMIT -----\n\n");
		fflush (logInfoTbl[logId].fp);
		fclose (logInfoTbl[logId].fp);
	}

	/* rename files */
	for (i = LOGLIB_MAX_LOG_SUFFIX - 2; i >= 0; i--) {
		sprintf(oldFile, "%s.%d", logInfoTbl[logId].fname, i);
		if (stat(oldFile, &fStatus) == 0) {
			sprintf(newFile, "%s.%d", logInfoTbl[logId].fname, i+1);
			rename(oldFile, newFile);
		}
	}

openNewLimitSizeFile:
	/* reopen "xxxx.0" */
	sprintf (newFile, "%s.0", logInfoTbl[logId].fname);
	if ((logInfoTbl[logId].fp = fopen(newFile,"a+")) == NULL) {
		fprintf(stderr,"[loglib_checkLimitSize] fopen fail[%s]; errno=%d(%s)\n",newFile,errno,strerror(errno));
		return -1;
	}

	return 1;

} /** End of loglib_checkLimitSize **/



/*------------------------------------------------------------------------------
* LOGLIB_MODE_DAILY�� ��� ��¥�� �ٲ������ Ȯ���Ѵ�.
* - ��¥�� �ٲ� ��� ���� ��¥ ������ ����.
* - LOGLIB_MODE_7DAYS�� ��� 7���� ������ �����Ѵ�.
------------------------------------------------------------------------------*/
int loglib_checkDate (int logId)
{
	int		i;
	char	newFile[256], oldFile[256], tmp[32];
	struct stat     fStatus;
	time_t	now;

	/* ���� ��¥ �����̸��� �����Ѵ�.
	*/
	now = time(0);
	strftime(tmp, 32, "%Y-%m-%d", localtime((time_t*)&now));
	sprintf(newFile, "%s.%s", logInfoTbl[logId].fname, tmp);

	/* ó�� open�ϴ� ��쿡�� fp==NULL�̹Ƿ� �����ϰ� ��ٷ� return�Ѵ�.
	*/
	if (logInfoTbl[logId].fp == NULL) {
		goto openNewDailyFile;
	}

	/* ������ ��� ��¥ �����̸��� �����Ѵ�.
	*/
	now = time(0);
	strftime(tmp, 32, "%Y-%m-%d", localtime((time_t*)&logInfoTbl[logId].lastTime));
	sprintf(oldFile, "%s.%s", logInfoTbl[logId].fname, tmp);

	/* �����̸��� ������ ��¥�� ���� �ٲ��� ���� ���
	*/
	if (!strcmp(newFile,oldFile)) {
		if (logInfoTbl[logId].fp == NULL) {
			/* ������ ���� ���� ���� ��� open�Ͽ� fp�� ������ �� return�ȴ�.
			*/
			goto openNewDailyFile;
		}
		return 1;
	}

	/* ��¥�� �ٲ���,
	** - �����ִ� ������ �ް�, ���ο� ������ ����.
	** - LOGLIB_MODE_7DAYS�̸� 7���� ������ �����Ѵ�.
	*/

	/* close current file */
	fflush(logInfoTbl[logId].fp);
	fclose(logInfoTbl[logId].fp);

	/* 7���� ������ �����Ѵ�.  */
	if (logInfoTbl[logId].mode & LOGLIB_MODE_7DAYS) {
		now -= 3600*24*7;
		strftime(tmp, 32, "%Y-%m-%d", localtime((time_t*)&now));
		sprintf(oldFile, "%s.%s", logInfoTbl[logId].fname, tmp);
		unlink(oldFile);
	}

openNewDailyFile:
	/* open new file */
	if ((logInfoTbl[logId].fp = fopen(newFile,"a+")) == NULL) {
		fprintf(stderr,"[loglib_checkDate] fopen fail[%s]; errno=%d(%s)\n",newFile,errno,strerror(errno));
		return -1;
	}

	return 1;

} /** End of loglib_checkDate **/



/*------------------------------------------------------------------------------
* LOGLIB_MODE_HOURLY�� ��� �ð�(Hour)�� �ٲ������ Ȯ���Ѵ�.
* - �ð��� �ٲ� ��� ���� �ð� ������ ����.
* - file�� ������Ģ�� logInfoTbl�� ����� fname�� directory �ؿ� �⵵ directory �ؿ�
*	���� directory �ؿ� ��¥�� �ð��̸��� ���� ������ �����Ѵ�.(fname/yyyy/mm/ddhh)
* - directory�� ������ directory�� ���� ������ �� �ش� ������ �����Ѵ�.
------------------------------------------------------------------------------*/
int loglib_checkTimeHour (int logId)
{
	int		i;
	char	newFile[256], oldFile[256], sufNew[32], sufOld[32];
	char	*next,*yyyy,*mm,*ddhh;
	char	dirName[256];
	DIR		*dp;
	struct stat     fStatus;
	time_t	now;

	/* ���� �ð� �����̸��� �����Ѵ�.
	*/
	now = time(0);
	strftime(sufNew, 32, "%Y/%m/%d%H", localtime((time_t*)&now));
	sprintf(newFile, "%s/%s", logInfoTbl[logId].fname, sufNew);

	/* ó�� open�ϴ� ��쿡�� fp==NULL�̹Ƿ� �����ϰ� ��ٷ� return�Ѵ�.
	*/
	if (logInfoTbl[logId].fp == NULL) {
		goto openNewHourlyFile;
	}

	/* ������ ��� ��¥ �����̸��� �����Ѵ�.
	*/
	strftime(sufOld, 32, "%Y/%m/%d%H", localtime((time_t*)&logInfoTbl[logId].lastTime));
	sprintf(oldFile, "%s/%s", logInfoTbl[logId].fname, sufOld);

	/* �����̸��� ������ �ð��� ���� �ٲ��� ���� ���
	*/
	if (!strcmp(newFile,oldFile)) {
		if (logInfoTbl[logId].fp == NULL) {
			/* ������ ���� ���� ���� ��� open�Ͽ� fp�� ������ �� return�ȴ�.
			*/
			goto openNewHourlyFile;
		}
		return 1;
	}

	/* �ð��� �ٲ���,
	** - �����ִ� ������ �ް�, ���ο� ������ ����.
	** - directory ���� ���θ� Ȯ���Ͽ� ������ ���� �����.
	*/

	/* close current file */
	fflush(logInfoTbl[logId].fp);
	fclose(logInfoTbl[logId].fp);

openNewHourlyFile:
	/* �� directory�� ������ �����ϱ� ���� newFile�� xxxx, yyyy, mm, ddhh�� �߶󳽴�.
	*/
	yyyy = (char*)(long)strtok_r(sufNew, "/", &next);
	mm = (char*)(long)strtok_r(next,"/",&ddhh);

	sprintf(dirName,"%s",logInfoTbl[logId].fname);
	if ((dp = opendir(dirName)) == NULL) {
		if (errno != ENOENT) {
			fprintf(stderr,"[loglib_checkTimeHour] opendir fail[%s]; errno=%d(%s)\n",dirName,errno,strerror(errno));
			return -1;
		}
		if (mkdir(dirName,0755) < 0) {
			fprintf(stderr,"[loglib_checkTimeHour] mkdir fail[%s]; errno=%d(%s)\n",dirName,errno,strerror(errno));
			return -1;
		}
	} else {
		closedir(dp);
	}

	sprintf(dirName,"%s/%s",logInfoTbl[logId].fname,yyyy);
	if ((dp = opendir(dirName)) == NULL) {
		if (errno != ENOENT) {
			fprintf(stderr,"[loglib_checkTimeHour] opendir fail[%s]; errno=%d(%s)\n",dirName,errno,strerror(errno));
			return -1;
		}
		if (mkdir(dirName,0755) < 0) {
			fprintf(stderr,"[loglib_checkTimeHour] mkdir fail[%s]; errno=%d(%s)\n",dirName,errno,strerror(errno));
			return -1;
		}
	} else {
		closedir(dp);
	}

	sprintf(dirName,"%s/%s/%s",logInfoTbl[logId].fname,yyyy,mm);
	if ((dp = opendir(dirName)) == NULL) {
		if (errno != ENOENT) {
			fprintf(stderr,"[loglib_checkTimeHour] opendir fail[%s]; errno=%d(%s)\n",dirName,errno,strerror(errno));
			return -1;
		}
		if (mkdir(dirName,0755) < 0) {
			fprintf(stderr,"[loglib_checkTimeHour] mkdir fail[%s]; errno=%d(%s)\n",dirName,errno,strerror(errno));
			return -1;
		}
	} else {
		closedir(dp);
	}

	if ((logInfoTbl[logId].fp = fopen(newFile,"a+")) == NULL) {
		fprintf(stderr,"[loglib_checkTimeHour] fopen fail[%s]; errno=%d(%s)\n",newFile,errno,strerror(errno));
		return -1;
	}

	return 1;

} /** End of loglib_checkTimeHour **/

/*------------------------------------------------------------------------------
* LOGLIB_MODE_ONE_DIR�� ��� �ð�(Hour)�� �ٲ������ Ȯ���Ѵ�.
* - �ð��� �ٲ� ��� ���� �ð� ������ ����.
* - file�� ������Ģ�� logInfoTbl�� ����� fname�� directory �ؿ� �⵵ directory �ؿ�
*	���� directory �ؿ� ��¥�� �ð��̸��� ���� ������ �����Ѵ�.(fname/yyyy/mm/ddhh)
* - directory�� ������ directory�� ���� ������ �� �ش� ������ �����Ѵ�.
------------------------------------------------------------------------------*/
int loglib_checkOneDir (int logId)
{
	int		i;
	char	newFile[256], oldFile[256], sufOld[32], sufNew[32];
	char	*next,*last,*yyyy,*mm,*dd,*hh;
	char	dirName[256];
	DIR		*dp;
	struct stat     fStatus;
	time_t	now;

	/* ���� �ð� �����̸��� �����Ѵ�.
	*/
	now = time(0);
	strftime(sufNew, 32, "%Y/%m/%d/%H", localtime((time_t*)&now));
	sprintf(newFile, "%s/%s", logInfoTbl[logId].fname, sufNew);

	/* ó�� open�ϴ� ��쿡�� fp==NULL�̹Ƿ� �����ϰ� ��ٷ� return�Ѵ�.
	*/
	if (logInfoTbl[logId].fp == NULL) {
		goto openNewHourlyFile;
	}

	/* ������ ��� ��¥ �����̸��� �����Ѵ�.
	*/
	strftime(sufOld, 32, "%Y/%m/%d/%H", localtime((time_t*)&logInfoTbl[logId].lastTime));
	sprintf(oldFile, "%s/%s", logInfoTbl[logId].fname, sufOld);

	/* �����̸��� ������ �ð��� ���� �ٲ��� ���� ���
	*/
	if (!strcmp(newFile,oldFile)) {
		if (logInfoTbl[logId].fp == NULL) {
			/* ������ ���� ���� ���� ��� open�Ͽ� fp�� ������ �� return�ȴ�.
			*/
			goto openNewHourlyFile;
		}
#if 1 //040820.hphlr.cjs  Ư�� ȭ���� ����ġ ���� ��Ȳ���� ������ ��� �� ����.
		else if (access(oldFile, F_OK) != 0) {
			    fflush(logInfoTbl[logId].fp);
			    fclose(logInfoTbl[logId].fp);
			    logInfoTbl[logId].fp = fopen(oldFile,"a+");
			    fprintf(stderr,"%s recreated....\n",oldFile);
		}
		
#endif
		return 1;
	}

	/* �ð��� �ٲ���,
	** - �����ִ� ������ �ް�, ���ο� ������ ����.
	** - directory ���� ���θ� Ȯ���Ͽ� ������ ���� �����.
	*/

	/* close current file */
	fflush(logInfoTbl[logId].fp);
	fclose(logInfoTbl[logId].fp);

openNewHourlyFile:
	/* �� directory�� ������ �����ϱ� ���� newFile�� xxxx, yyyy, mm, ddhh�� �߶󳽴�.
	*/
	yyyy = (char*)(long)strtok_r(sufNew, "/", &next);
	mm = (char*)(long)strtok_r(next,"/",&last);
	dd = (char*)(long)strtok_r(last,"/",&hh);

	sprintf(dirName,"%s",logInfoTbl[logId].fname);
	if ((dp = opendir(dirName)) == NULL) {
		if (errno != ENOENT) {
			fprintf(stderr,"[loglib_checkOneDir] opendir 1 fail[%s]; errno=%d(%s)\n",dirName,errno,strerror(errno));
			return -1;
		}
		if (mkdir(dirName,0755) < 0) {
			fprintf(stderr,"[loglib_checkOneDir] mkdir 1 fail[%s]; errno=%d(%s)\n",dirName,errno,strerror(errno));
			return -1;
		}
	} else {
		closedir(dp);
	}

	sprintf(dirName,"%s/%s/%s/%s",logInfoTbl[logId].fname,yyyy,mm,dd);
	if ((dp = opendir(dirName)) == NULL) {
		if (errno != ENOENT) {
			fprintf(stderr,"[loglib_checkOneDir] opendir 2 fail[%s]; errno=%d(%s)\n",dirName,errno,strerror(errno));
			return -1;
		}
		
		sprintf(dirName,"%s/%s",logInfoTbl[logId].fname,yyyy);
		if ((dp = opendir(dirName)) == NULL) {
			if (mkdir(dirName,0755) < 0) {
				fprintf(stderr,"[loglib_checkOneDir] mkdir 2 fail[%s]; errno=%d(%s)\n",dirName,errno,strerror(errno));
				return -1;
			}
		}

		sprintf(dirName,"%s/%s/%s",logInfoTbl[logId].fname,yyyy,mm);
		if ((dp = opendir(dirName)) == NULL) {
			if (mkdir(dirName,0755) < 0) {
				fprintf(stderr,"[loglib_checkOneDir] mkdir 2 fail[%s]; errno=%d(%s)\n",dirName,errno,strerror(errno));
				return -1;
			}
		}

		sprintf(dirName,"%s/%s/%s/%s",logInfoTbl[logId].fname,yyyy,mm,dd);
		if ((dp = opendir(dirName)) == NULL) {
			if (mkdir(dirName,0755) < 0) {
				fprintf(stderr,"[loglib_checkOneDir] mkdir 2 fail[%s]; errno=%d(%s)\n",dirName,errno,strerror(errno));
				return -1;
			}
		}

	} else {
		closedir(dp);
	}

	if ((logInfoTbl[logId].fp = fopen(newFile,"a+")) == NULL) {
		fprintf(stderr,"[loglib_checkOneDir] fopen fail[%s]; errno=%d(%s)\n",newFile,errno,strerror(errno));
		return -1;
	}

	return 1;

} /** End of loglib_checkOneDir **/
