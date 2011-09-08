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

	/* 최대로 열수 있는 file의 제한이 있다.
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

	/* log file의 정보를 저장한다.
	*/
	strcpy(logInfoTbl[logId].fname, fname);
	logInfoTbl[logId].mode     = mode;
	logInfoTbl[logId].fp       = NULL;
	logInfoTbl[logId].lastTime = 0;

	/* LOGLIB_MODE_LIMIT_SIZE인 경우 "xxxx.0" 파일의 유무를 확인하고,
	** - 없으면 생성한 후 logInfoTbl[logId].fp에 저장되어 return되고,
	** - 있으면 limit 초과 여부를 확인하여 초과하지 않았으면 append 모드로 open한 후
	**	logInfoTbl[logId].fp에 저장되어 return되고,
	** - 파일이 존재하고 limit를 초과한 경우 기존파일을 rename한 후 "xxxx.0"를
	**	다시 생성하여 logInfoTbl[logId].fp에 저장하고 return된다.
	*/
	if (mode & LOGLIB_MODE_LIMIT_SIZE) {
		if (loglib_checkLimitSize(logId) < 0)
			return -1;
	}

	/* LOGLIB_MODE_DAILY인 경우, 오늘날짜의 파일(xxxx.yyyy-mm-dd) append 모드로
	**	open하고 logInfoTbl[logId].fp에 저장되어 return된다.
	** - LOGLIB_MODE_7DAYS인 경우 7일전 파일을 지운다.
	*/
	if (mode & LOGLIB_MODE_DAILY) {
		if (loglib_checkDate(logId) < 0)
			return -1;
		logInfoTbl[logId].lastTime = time(0);
	}

	/* LOGLIB_MODE_HOURLY인 경우, 현재시각 이름의 파일(xxxx/yyyy/mm/ddhh) append 모드로
	**	open하고 logInfoTbl[logId].fp에 저장되어 return된다.
	** - directory가 없으면 먼저 생성한 후 파일을 생성한다.
	*/
	if (mode & LOGLIB_MODE_HOURLY) {
		if (loglib_checkTimeHour(logId) < 0)
			return -1;
		logInfoTbl[logId].lastTime = time(0);
	}
	/* LOGLIB_MODE_ONE_DIR인 경우, 현재시각 이름의 파일(xxxx/yyyy.mm.dd/hh) append 모드로
	**	open하고 logInfoTbl[logId].fp에 저장되어 return된다.
	** - directory가 없으면 먼저 생성한 후 파일을 생성한다.
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
		/* 현재 열려있는 파일이 limit를 초과했는지 확인한다.
		** - 초과된 경우 이전파일을 rename하고 다시 연다.
		*/
		if (loglib_checkLimitSize(logId) < 0)
			return -1;
	}
	else if (logInfoTbl[logId].mode & LOGLIB_MODE_DAILY)
	{
		/* 현재 열려있는 파일정보와 현재 시각을 비교하여 날짜가 바뀌었는지 확인한다.
		** - 날짜가 바뀌었으면 오늘 날짜 파일의 연다.
		** - LOGLIB_MODE_7DAYS인 경우 7일전 파일을 지운다.
		*/
		if (loglib_checkDate(logId) < 0)
			return -1;
	}
	else if (logInfoTbl[logId].mode & LOGLIB_MODE_HOURLY)
	{
		/* 현재 열려있는 파일정보와 현재 시각을 비교하여 시간(hour)이 바뀌었는지 확인한다.
		** - 시간이 바뀌었으면 현재 시간 파일의 연다.
		** - 년/월 directory 구조 밑에 파일이 생성되므로 directory가 없으면 먼저 만든다.
		*/
		if (loglib_checkTimeHour(logId) < 0)
			return -1;
	}
	else if (logInfoTbl[logId].mode & LOGLIB_MODE_ONE_DIR)
	{
		/* 현재 열려있는 파일정보와 현재 시각을 비교하여 시간(hour)이 바뀌었는지 확인한다.
		** - 시간이 바뀌었으면 현재 시간 파일의 연다.
		** - 년/월 directory 구조 밑에 파일이 생성되므로 directory가 없으면 먼저 만든다.
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
	/* LOGLIB_TIME_STAMP이면 현재 시각을 함께 기록한다.
	*/
	if (logInfoTbl[logId].mode & LOGLIB_TIME_STAMP) {
		strftime(tmp, 32, "%m-%d %T", localtime((time_t*)&curr.tv_sec));
		sprintf (optBuf, "[%s.%03d]", tmp, (curr.tv_usec/1000));
	}
	/* LOGLIB_FNAME_LNUM이면 소스 파일 이름과 라인 수를 함께 기록한다.
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
* LOGLIB_MODE_LIMIT_SIZE인 경우 limit size를 초과했는지 확인한다.
* - 초과된 경우 xxxx.0을 1로, 1을 2로, ... 등 기존 파일을 rename한 후 "xxxx.0"을
*	다시 open한다.
------------------------------------------------------------------------------*/
int loglib_checkLimitSize (int logId)
{
	int		i;
	char	newFile[256], oldFile[256];
	struct stat     fStatus;
	
	/* 실제 log_file은 뒤에 .0이 붙는다.
	*/
	sprintf (newFile, "%s.0", logInfoTbl[logId].fname);

	/* 파일의 유무를 확인한다.
	*/
	if (stat(newFile, &fStatus) < 0) {
		if (errno != ENOENT) {
			fprintf(stderr,"[loglib_checkLimitSize] stat fail[%s]; errno=%d(%s)\n",newFile,errno,strerror(errno));
			return -1;
		}
		/* "xxxx.0" 파일이 존재하지 않으면 생성하고 fp를 저장한후 곧바로 return된다.
		*/
		goto openNewLimitSizeFile;
	}

	/* 아직 limit를 초과하지 않은 경우
	*/
	if (fStatus.st_size < LOGLIB_MAX_FILE_SIZE) {
		if (logInfoTbl[logId].fp == NULL) {
			/* 파일을 아직 열지 않은 경우 open하여 fp를 저장한 후 return된다.
			*/
			goto openNewLimitSizeFile;
		}
		return 1;
	}

	/* limit를 초과한 경우,
	** - 열려있는 파일("xxxx.0")을 close하고 "xxxx.0"을 "xxxx.1"로, 1을 2로, ... 등
	**	기존 파일을 rename한 후 "xxxx.0"을 다시 open한다.
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
* LOGLIB_MODE_DAILY인 경우 날짜가 바뀌었는지 확인한다.
* - 날짜가 바뀐 경우 오늘 날짜 파일을 연다.
* - LOGLIB_MODE_7DAYS인 경우 7일전 파일을 삭제한다.
------------------------------------------------------------------------------*/
int loglib_checkDate (int logId)
{
	int		i;
	char	newFile[256], oldFile[256], tmp[32];
	struct stat     fStatus;
	time_t	now;

	/* 오늘 날짜 파일이름을 구성한다.
	*/
	now = time(0);
	strftime(tmp, 32, "%Y-%m-%d", localtime((time_t*)&now));
	sprintf(newFile, "%s.%s", logInfoTbl[logId].fname, tmp);

	/* 처음 open하는 경우에는 fp==NULL이므로 생성하고 곧바로 return한다.
	*/
	if (logInfoTbl[logId].fp == NULL) {
		goto openNewDailyFile;
	}

	/* 마지막 기록 날짜 파일이름을 구성한다.
	*/
	now = time(0);
	strftime(tmp, 32, "%Y-%m-%d", localtime((time_t*)&logInfoTbl[logId].lastTime));
	sprintf(oldFile, "%s.%s", logInfoTbl[logId].fname, tmp);

	/* 파일이름이 같으면 날짜가 아직 바뀌지 않은 경우
	*/
	if (!strcmp(newFile,oldFile)) {
		if (logInfoTbl[logId].fp == NULL) {
			/* 파일을 아직 열지 않은 경우 open하여 fp를 저장한 후 return된다.
			*/
			goto openNewDailyFile;
		}
		return 1;
	}

	/* 날짜가 바뀐경우,
	** - 열려있는 파일을 받고, 새로운 파일을 연다.
	** - LOGLIB_MODE_7DAYS이면 7일전 파일을 삭제한다.
	*/

	/* close current file */
	fflush(logInfoTbl[logId].fp);
	fclose(logInfoTbl[logId].fp);

	/* 7일전 파일을 삭제한다.  */
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
* LOGLIB_MODE_HOURLY인 경우 시간(Hour)이 바뀌었는지 확인한다.
* - 시간이 바뀐 경우 현재 시간 파일을 연다.
* - file의 생성규칙은 logInfoTbl에 저장된 fname의 directory 밑에 년도 directory 밑에
*	월별 directory 밑에 날짜와 시간이름을 갖는 파일을 생성한다.(fname/yyyy/mm/ddhh)
* - directory가 없으면 directory를 먼저 생성한 후 해당 파일을 생성한다.
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

	/* 현재 시간 파일이름을 구성한다.
	*/
	now = time(0);
	strftime(sufNew, 32, "%Y/%m/%d%H", localtime((time_t*)&now));
	sprintf(newFile, "%s/%s", logInfoTbl[logId].fname, sufNew);

	/* 처음 open하는 경우에는 fp==NULL이므로 생성하고 곧바로 return한다.
	*/
	if (logInfoTbl[logId].fp == NULL) {
		goto openNewHourlyFile;
	}

	/* 마지막 기록 날짜 파일이름을 구성한다.
	*/
	strftime(sufOld, 32, "%Y/%m/%d%H", localtime((time_t*)&logInfoTbl[logId].lastTime));
	sprintf(oldFile, "%s/%s", logInfoTbl[logId].fname, sufOld);

	/* 파일이름이 같으면 시간이 아직 바뀌지 않은 경우
	*/
	if (!strcmp(newFile,oldFile)) {
		if (logInfoTbl[logId].fp == NULL) {
			/* 파일을 아직 열지 않은 경우 open하여 fp를 저장한 후 return된다.
			*/
			goto openNewHourlyFile;
		}
		return 1;
	}

	/* 시간이 바뀐경우,
	** - 열려있는 파일을 받고, 새로운 파일을 연다.
	** - directory 존재 여부를 확인하여 없으면 먼저 만든다.
	*/

	/* close current file */
	fflush(logInfoTbl[logId].fp);
	fclose(logInfoTbl[logId].fp);

openNewHourlyFile:
	/* 각 directory가 없으면 생성하기 위해 newFile을 xxxx, yyyy, mm, ddhh로 잘라낸다.
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
* LOGLIB_MODE_ONE_DIR인 경우 시간(Hour)이 바뀌었는지 확인한다.
* - 시간이 바뀐 경우 현재 시간 파일을 연다.
* - file의 생성규칙은 logInfoTbl에 저장된 fname의 directory 밑에 년도 directory 밑에
*	월별 directory 밑에 날짜와 시간이름을 갖는 파일을 생성한다.(fname/yyyy/mm/ddhh)
* - directory가 없으면 directory를 먼저 생성한 후 해당 파일을 생성한다.
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

	/* 현재 시간 파일이름을 구성한다.
	*/
	now = time(0);
	strftime(sufNew, 32, "%Y/%m/%d/%H", localtime((time_t*)&now));
	sprintf(newFile, "%s/%s", logInfoTbl[logId].fname, sufNew);

	/* 처음 open하는 경우에는 fp==NULL이므로 생성하고 곧바로 return한다.
	*/
	if (logInfoTbl[logId].fp == NULL) {
		goto openNewHourlyFile;
	}

	/* 마지막 기록 날짜 파일이름을 구성한다.
	*/
	strftime(sufOld, 32, "%Y/%m/%d/%H", localtime((time_t*)&logInfoTbl[logId].lastTime));
	sprintf(oldFile, "%s/%s", logInfoTbl[logId].fname, sufOld);

	/* 파일이름이 같으면 시간이 아직 바뀌지 않은 경우
	*/
	if (!strcmp(newFile,oldFile)) {
		if (logInfoTbl[logId].fp == NULL) {
			/* 파일을 아직 열지 않은 경우 open하여 fp를 저장한 후 return된다.
			*/
			goto openNewHourlyFile;
		}
#if 1 //040820.hphlr.cjs  특정 화일이 예기치 못한 상황에서 삭제될 경우 재 생성.
		else if (access(oldFile, F_OK) != 0) {
			    fflush(logInfoTbl[logId].fp);
			    fclose(logInfoTbl[logId].fp);
			    logInfoTbl[logId].fp = fopen(oldFile,"a+");
			    fprintf(stderr,"%s recreated....\n",oldFile);
		}
		
#endif
		return 1;
	}

	/* 시간이 바뀐경우,
	** - 열려있는 파일을 받고, 새로운 파일을 연다.
	** - directory 존재 여부를 확인하여 없으면 먼저 만든다.
	*/

	/* close current file */
	fflush(logInfoTbl[logId].fp);
	fclose(logInfoTbl[logId].fp);

openNewHourlyFile:
	/* 각 directory가 없으면 생성하기 위해 newFile을 xxxx, yyyy, mm, ddhh로 잘라낸다.
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
