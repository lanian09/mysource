#include "conflib.h"



/*------------------------------------------------------------------------------
* file을 열어 지정된 section내에서 keyword에 해당하는 라인을 찾아,
*	"=" 뒤쪽에서 n번째 token를 string에 복사한다.
* - conf 파일에는 "keyword = aaa bbb ccc ..." 의 형식으로 구성되어 있어야 한다.
* - conf 파일 한줄에 keyword = 를 제외하고 최대 16개까지 token를 기록할 수 있다.
* - 예를들어, "xxxx = aaa bbb ccc" 에서 bbb를 찾고 싶은 경우,
*	keyword=xxxx, n=2를 지정해야 한다.
------------------------------------------------------------------------------*/
int conflib_getNthTokenInFileSection (
		char *fname,	/* configuration file name */
		char *section,	/* section name */
		char *keyword,	/* keyword -> 찾고자 하는 라인에 있는 첫번째 token name ("=" 앞에 있는 token) */
		int  n,			/* 몇번째 token을 찾을것인지 지정 ("=" 뒤에 있는 n번째 token) */
		char *string	/* 찾은 token이 저장된 string */
		)
{
	FILE	*fp;
	char    buff[256],token[16][CONFLIB_MAX_TOKEN_LEN];
	int		ret,lNum,lNum2;

	/* file을 연다.
	*/
	if ((fp = fopen(fname,"r")) == NULL) {
		fprintf(stderr,"[conflib_getNthTokenInFileSection] fopen fail[%s]; errno=%d(%s)\n", fname, errno, strerror(errno));
		return -1;
	}

	/* section의 시작위치로 이동한다.
	*/
	if ((lNum = conflib_seekSection (fp, section)) < 0) {
		fprintf(stderr,"[conflib_getNthTokenInFileSection] not found section[%s] in file[%s]\n", section, fname);
		fclose(fp);
		return -1;
	}

	/* section내에서 keyword에 해당하는 line을 찾아 "=" 뒤쪽 내용을 buff에 복사한다.
	*/
	if ((lNum2 = conflib_getStringInSection(fp,keyword,buff)) < 0) {
		fprintf(stderr,"[conflib_getNthTokenInFileSection] not found keyword[%s]; file=%s, section=%s\n", keyword, fname, section);
		fclose(fp);
		return -1;
	}
	fclose(fp);
	lNum += lNum2;

	/* n번째 token을 string에 복사한다.
	*/
	if ((ret = sscanf (buff,"%s%s%s%s%s %s%s%s%s%s %s%s%s%s%s %s",
						token[0], token[1], token[2], token[3], token[4],
						token[5], token[6], token[7], token[8], token[9],
						token[10],token[11],token[12],token[13],token[14],
						token[15])) < n) {
		/* 해당 라인에 있는 token이 n개 보다 적은 경우
		*/
		fprintf(stderr,"[conflib_getNthTokenInFileSection] can't found Nth(%d) token; file=%s, section=%s, keyword=%s, lNum=%d\n", n, fname, section, keyword, lNum);
		return -1;
	}

	/* 찾은 token을 user영역에 복사한다.
	*/
	strcpy(string,token[n-1]);

	return 1;

} /** End of conflib_getNthTokenInFileSection **/



/*------------------------------------------------------------------------------
* file을 열어 지정된 section내에서 keyword에 해당하는 라인을 찾아,
*	"=" 뒤에 몇개의 token이 있는지 return한다.
------------------------------------------------------------------------------*/
int conflib_getTokenCntInFileSection (
		char *fname,	/* configuration file name */
		char *section,	/* section name */
		char *keyword	/* keyword -> 찾고자 하는 라인에 있는 첫번째 token name ("=" 앞에 있는 token) */
		)
{
	FILE	*fp;
	char    buff[256],token[20][CONFLIB_MAX_TOKEN_LEN];
	int		i,ret,lNum,lNum2;

	/* file을 연다.
	*/
	if ((fp = fopen(fname,"r")) == NULL) {
		fprintf(stderr,"[conflib_getTokenCntInFileSection] fopen fail[%s]; errno=%d(%s)\n", fname, errno, strerror(errno));
		return -1;
	}

	/* section의 시작위치로 이동한다.
	*/
	if ((lNum = conflib_seekSection (fp, section)) < 0) {
		fprintf(stderr,"[conflib_getTokenCntInFileSection] not found section[%s] in file[%s]\n", section, fname);
		fclose(fp);
		return -1;
	}

	/* section내에서 keyword에 해당하는 line을 찾아 "=" 뒤쪽 내용을 buff에 복사한다.
	*/
	if ((lNum2 = conflib_getStringInSection(fp,keyword,buff)) < 0) {
		fprintf(stderr,"[conflib_getTokenCntInFileSection] not found keyword[%s]; file=%s, section=%s\n", keyword, fname, section);
		fclose(fp);
		return -1;
	}
	fclose(fp);
	lNum += lNum2;

	/* token이 몇개인지 return된다.
	*/
	return (sscanf (buff,"%s%s%s%s%s %s%s%s%s%s %s%s%s%s%s %s%s%s%s%s",
						token[0], token[1], token[2], token[3], token[4],
						token[5], token[6], token[7], token[8], token[9],
						token[10],token[11],token[12],token[13],token[14],
						token[15],token[16],token[17],token[18],token[19]));

} /** End of conflib_getTokenCntInFileSection **/



/*------------------------------------------------------------------------------
* file을 열어 지정된 section내에서 keyword에 해당하는 라인을 찾아,
*	"=" 뒤쪽 string을 n개의 token으로 잘라 string에 복사한다.
* - token들이 저장될 string은 반드시 CONFLIB_MAX_TOKEN_LEN 크기로 된 array로
*	선언되어야 한다.
------------------------------------------------------------------------------*/
int conflib_getNTokenInFileSection (
		char *fname,	/* configuration file name */
		char *section,	/* section name */
		char *keyword,	/* keyword -> 찾고자 하는 라인에 있는 첫번째 token name ("=" 앞에 있는 token) */
		int  n,			/* 몇개의 token으로 잘라 낼 것인지 지정 ("=" 뒤에 있는 n개 token) */
		char string[][CONFLIB_MAX_TOKEN_LEN]	/* 찾은 token들이 저장된 string */
		)
{
	FILE	*fp;
	char    buff[256],token[20][CONFLIB_MAX_TOKEN_LEN];
	int		i,ret,lNum,lNum2;

	/* file을 연다.
	*/
	if ((fp = fopen(fname,"r")) == NULL) {
		fprintf(stderr,"[conflib_getNTokenInFileSection] fopen fail[%s]; errno=%d(%s)\n", fname, errno, strerror(errno));
		return -1;
	}

	/* section의 시작위치로 이동한다.
	*/
	if ((lNum = conflib_seekSection (fp, section)) < 0) {
		fprintf(stderr,"[conflib_getNTokenInFileSection] not found section[%s] in file[%s]\n", section, fname);
		fclose(fp);
		return -1;
	}

	/* section내에서 keyword에 해당하는 line을 찾아 "=" 뒤쪽 내용을 buff에 복사한다.
	*/
	if ((lNum2 = conflib_getStringInSection(fp,keyword,buff)) < 0) {
		fprintf(stderr,"[conflib_getNTokenInFileSection] not found keyword[%s]; file=%s, section=%s\n", keyword, fname, section);
		fclose(fp);
		return -1;
	}
	fclose(fp);
	lNum += lNum2;

	/* n번째 token을 string에 복사한다.
	*/
	if ((ret = sscanf (buff,"%s%s%s%s%s %s%s%s%s%s %s%s%s%s%s %s%s%s%s%s",
						token[0], token[1], token[2], token[3], token[4],
						token[5], token[6], token[7], token[8], token[9],
						token[10],token[11],token[12],token[13],token[14],
						token[15],token[16],token[17],token[18],token[19])) < n) {
		/* 해당 라인에 있는 token이 n개 보다 적은 경우
		*/
		fprintf(stderr,"[conflib_getNTokenInFileSection] can't found N(%d) token; file=%s, section=%s, keyword=%s, lNum=%d\n", n, fname, section, keyword, lNum);
		return -1;
	}

	/* 잘라낸 token들을 user영역에 복사한다.
	*/
	for (i=0; i<n; i++) {
		strcpy(string[i],token[i]);
	} 
	return 1;

} /** End of conflib_getNTokenInFileSection **/



/*------------------------------------------------------------------------------
* file을 열어 지정된 section내에서 keyword에 해당하는 라인을 찾아,
*	"=" 뒤에 있는 내용을 string에 모두 복사한다.
------------------------------------------------------------------------------*/
int conflib_getStringInFileSection (
		char *fname,	/* configuration file name */
		char *section,	/* section name */
		char *keyword,	/* keyword -> 찾고자 하는 라인에 있는 첫번째 token name ("=" 앞에 있는 token) */
		char *string	/* 찾은 내용이 저장된 string */
		)
{
	FILE    *fp;
	char    getBuf[256],*next;

	/* file을 연다.
	*/
	if ((fp = fopen (fname,"r")) == NULL) {
		fprintf(stderr,"[conflib_getStringInFileSection] fopen fail[%s]; errno=%d(%s)\n",fname,errno,strerror(errno));
		return -1;
	}

	/* section의 시작위치로 이동한다.
	*/
	if (conflib_seekSection(fp,section) < 0) {
		fprintf(stderr,"[conflib_getStringInFileSection] not found section[%s] in file[%s]\n",section,fname);
		fclose(fp);
		return -1;
	}

	/* section내에서 keyword에 해당하는 line을 찾아 "=" 뒤쪽 내용을 string에 복사한다.
	*/
	if (conflib_getStringInSection(fp,keyword,string) < 0) {
		fprintf(stderr,"[conflib_getStringInFileSection] not found keyword[%s]; file=%s, section=%s\n", keyword, fname, section);
		fclose(fp);
		return -1;
	}

	fclose(fp);
	return 1;

} /** End of conflib_getStringInFileSection **/



/*------------------------------------------------------------------------------
* fopen으로 열려있는 파일에서 지정된 section으로 fp를 이동한다.
* - 파일의 처음부터 한줄씩 읽어 지정된 section을 찾고, line_number를 return한다.
* - conf 파일에서 section의 구분은 "[xxxx]"가 있는 줄부터 시작되어 다음 section의
*	시작까지이다.
------------------------------------------------------------------------------*/
int conflib_seekSection (
		FILE *fp,		/* file pointer */
		char *section	/* section name */
		)
{
	char    getBuf[256];
	int		lNum=0;

	rewind(fp);
	while (fgets(getBuf,sizeof(getBuf),fp) != NULL) {
		lNum++;
		if (getBuf[0] != '[')
			continue;
		if (strstr(getBuf,section)) {
			return lNum;
		}
	}
	fprintf(stderr,"[conflib_seekSection] not found section[%s]\n", section);
	return -1;

} /** End of conflib_seekSection **/



/*------------------------------------------------------------------------------
* fopen으로 열려있는 파일에서 현재 fp위치에서부터 현재 section내에서 지정된 keyword에
*	해당하는 line을 찾아, "=" 뒤쪽 내용을 string에 복사한다.
* - 검색한 line_number를 return한다.
------------------------------------------------------------------------------*/
int conflib_getStringInSection (
		FILE *fp,		/* file pointer */
		char *keyword,	/* keyword -> 찾고자 하는 라인에 있는 첫번째 token name ("=" 앞에 있는 token) */
		char *string	/* 찾은 내용이 저장된 string */
		)
{
	char    getBuf[256],token[CONFLIB_MAX_TOKEN_LEN],*next;
	int		lNum=0;

	/* section내에서 keyword에 해당하는 line을 찾아 "=" 뒤쪽 내용을 string에 복사한다.
	*/
	while (fgets(getBuf,sizeof(getBuf),fp) != NULL) {
		lNum++;
		if (getBuf[0] == '[') /* end of section */
			break;
		if (getBuf[0]=='#' || getBuf[0]=='\n') /* comment line or empty */
			continue;

		sscanf (getBuf,"%s",token);
		if (!strcmp(token, keyword)) {
			strtok_r(getBuf, "=", &next);
			for (; isspace(*next); next++) ; /* 앞쪽 white-space를 없앤다.*/
			strcpy (string, next);
			return lNum;
		}
	}
	fprintf(stderr,"[conflib_getStringInSection] not found keyword[%s]\n", keyword);
	return -1;

} /** End of conflib_getStringInSection **/
