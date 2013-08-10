#include "conflib.h"



/*------------------------------------------------------------------------------
* file�� ���� ������ section������ keyword�� �ش��ϴ� ������ ã��,
*	"=" ���ʿ��� n��° token�� string�� �����Ѵ�.
* - conf ���Ͽ��� "keyword = aaa bbb ccc ..." �� �������� �����Ǿ� �־�� �Ѵ�.
* - conf ���� ���ٿ� keyword = �� �����ϰ� �ִ� 16������ token�� ����� �� �ִ�.
* - �������, "xxxx = aaa bbb ccc" ���� bbb�� ã�� ���� ���,
*	keyword=xxxx, n=2�� �����ؾ� �Ѵ�.
------------------------------------------------------------------------------*/
int conflib_getNthTokenInFileSection (
		char *fname,	/* configuration file name */
		char *section,	/* section name */
		char *keyword,	/* keyword -> ã���� �ϴ� ���ο� �ִ� ù��° token name ("=" �տ� �ִ� token) */
		int  n,			/* ���° token�� ã�������� ���� ("=" �ڿ� �ִ� n��° token) */
		char *string	/* ã�� token�� ����� string */
		)
{
	FILE	*fp;
	char    buff[256],token[16][CONFLIB_MAX_TOKEN_LEN];
	int		ret,lNum,lNum2;

	/* file�� ����.
	*/
	if ((fp = fopen(fname,"r")) == NULL) {
		fprintf(stderr,"[conflib_getNthTokenInFileSection] fopen fail[%s]; errno=%d(%s)\n", fname, errno, strerror(errno));
		return -1;
	}

	/* section�� ������ġ�� �̵��Ѵ�.
	*/
	if ((lNum = conflib_seekSection (fp, section)) < 0) {
		fprintf(stderr,"[conflib_getNthTokenInFileSection] not found section[%s] in file[%s]\n", section, fname);
		fclose(fp);
		return -1;
	}

	/* section������ keyword�� �ش��ϴ� line�� ã�� "=" ���� ������ buff�� �����Ѵ�.
	*/
	if ((lNum2 = conflib_getStringInSection(fp,keyword,buff)) < 0) {
		fprintf(stderr,"[conflib_getNthTokenInFileSection] not found keyword[%s]; file=%s, section=%s\n", keyword, fname, section);
		fclose(fp);
		return -1;
	}
	fclose(fp);
	lNum += lNum2;

	/* n��° token�� string�� �����Ѵ�.
	*/
	if ((ret = sscanf (buff,"%s%s%s%s%s %s%s%s%s%s %s%s%s%s%s %s",
						token[0], token[1], token[2], token[3], token[4],
						token[5], token[6], token[7], token[8], token[9],
						token[10],token[11],token[12],token[13],token[14],
						token[15])) < n) {
		/* �ش� ���ο� �ִ� token�� n�� ���� ���� ���
		*/
		fprintf(stderr,"[conflib_getNthTokenInFileSection] can't found Nth(%d) token; file=%s, section=%s, keyword=%s, lNum=%d\n", n, fname, section, keyword, lNum);
		return -1;
	}

	/* ã�� token�� user������ �����Ѵ�.
	*/
	strcpy(string,token[n-1]);

	return 1;

} /** End of conflib_getNthTokenInFileSection **/



/*------------------------------------------------------------------------------
* file�� ���� ������ section������ keyword�� �ش��ϴ� ������ ã��,
*	"=" �ڿ� ��� token�� �ִ��� return�Ѵ�.
------------------------------------------------------------------------------*/
int conflib_getTokenCntInFileSection (
		char *fname,	/* configuration file name */
		char *section,	/* section name */
		char *keyword	/* keyword -> ã���� �ϴ� ���ο� �ִ� ù��° token name ("=" �տ� �ִ� token) */
		)
{
	FILE	*fp;
	char    buff[256],token[20][CONFLIB_MAX_TOKEN_LEN];
	int		i,ret,lNum,lNum2;

	/* file�� ����.
	*/
	if ((fp = fopen(fname,"r")) == NULL) {
		fprintf(stderr,"[conflib_getTokenCntInFileSection] fopen fail[%s]; errno=%d(%s)\n", fname, errno, strerror(errno));
		return -1;
	}

	/* section�� ������ġ�� �̵��Ѵ�.
	*/
	if ((lNum = conflib_seekSection (fp, section)) < 0) {
		fprintf(stderr,"[conflib_getTokenCntInFileSection] not found section[%s] in file[%s]\n", section, fname);
		fclose(fp);
		return -1;
	}

	/* section������ keyword�� �ش��ϴ� line�� ã�� "=" ���� ������ buff�� �����Ѵ�.
	*/
	if ((lNum2 = conflib_getStringInSection(fp,keyword,buff)) < 0) {
		fprintf(stderr,"[conflib_getTokenCntInFileSection] not found keyword[%s]; file=%s, section=%s\n", keyword, fname, section);
		fclose(fp);
		return -1;
	}
	fclose(fp);
	lNum += lNum2;

	/* token�� ����� return�ȴ�.
	*/
	return (sscanf (buff,"%s%s%s%s%s %s%s%s%s%s %s%s%s%s%s %s%s%s%s%s",
						token[0], token[1], token[2], token[3], token[4],
						token[5], token[6], token[7], token[8], token[9],
						token[10],token[11],token[12],token[13],token[14],
						token[15],token[16],token[17],token[18],token[19]));

} /** End of conflib_getTokenCntInFileSection **/



/*------------------------------------------------------------------------------
* file�� ���� ������ section������ keyword�� �ش��ϴ� ������ ã��,
*	"=" ���� string�� n���� token���� �߶� string�� �����Ѵ�.
* - token���� ����� string�� �ݵ�� CONFLIB_MAX_TOKEN_LEN ũ��� �� array��
*	����Ǿ�� �Ѵ�.
------------------------------------------------------------------------------*/
int conflib_getNTokenInFileSection (
		char *fname,	/* configuration file name */
		char *section,	/* section name */
		char *keyword,	/* keyword -> ã���� �ϴ� ���ο� �ִ� ù��° token name ("=" �տ� �ִ� token) */
		int  n,			/* ��� token���� �߶� �� ������ ���� ("=" �ڿ� �ִ� n�� token) */
		char string[][CONFLIB_MAX_TOKEN_LEN]	/* ã�� token���� ����� string */
		)
{
	FILE	*fp;
	char    buff[256],token[20][CONFLIB_MAX_TOKEN_LEN];
	int		i,ret,lNum,lNum2;

	/* file�� ����.
	*/
	if ((fp = fopen(fname,"r")) == NULL) {
		fprintf(stderr,"[conflib_getNTokenInFileSection] fopen fail[%s]; errno=%d(%s)\n", fname, errno, strerror(errno));
		return -1;
	}

	/* section�� ������ġ�� �̵��Ѵ�.
	*/
	if ((lNum = conflib_seekSection (fp, section)) < 0) {
		fprintf(stderr,"[conflib_getNTokenInFileSection] not found section[%s] in file[%s]\n", section, fname);
		fclose(fp);
		return -1;
	}

	/* section������ keyword�� �ش��ϴ� line�� ã�� "=" ���� ������ buff�� �����Ѵ�.
	*/
	if ((lNum2 = conflib_getStringInSection(fp,keyword,buff)) < 0) {
		fprintf(stderr,"[conflib_getNTokenInFileSection] not found keyword[%s]; file=%s, section=%s\n", keyword, fname, section);
		fclose(fp);
		return -1;
	}
	fclose(fp);
	lNum += lNum2;

	/* n��° token�� string�� �����Ѵ�.
	*/
	if ((ret = sscanf (buff,"%s%s%s%s%s %s%s%s%s%s %s%s%s%s%s %s%s%s%s%s",
						token[0], token[1], token[2], token[3], token[4],
						token[5], token[6], token[7], token[8], token[9],
						token[10],token[11],token[12],token[13],token[14],
						token[15],token[16],token[17],token[18],token[19])) < n) {
		/* �ش� ���ο� �ִ� token�� n�� ���� ���� ���
		*/
		fprintf(stderr,"[conflib_getNTokenInFileSection] can't found N(%d) token; file=%s, section=%s, keyword=%s, lNum=%d\n", n, fname, section, keyword, lNum);
		return -1;
	}

	/* �߶� token���� user������ �����Ѵ�.
	*/
	for (i=0; i<n; i++) {
		strcpy(string[i],token[i]);
	} 
	return 1;

} /** End of conflib_getNTokenInFileSection **/



/*------------------------------------------------------------------------------
* file�� ���� ������ section������ keyword�� �ش��ϴ� ������ ã��,
*	"=" �ڿ� �ִ� ������ string�� ��� �����Ѵ�.
------------------------------------------------------------------------------*/
int conflib_getStringInFileSection (
		char *fname,	/* configuration file name */
		char *section,	/* section name */
		char *keyword,	/* keyword -> ã���� �ϴ� ���ο� �ִ� ù��° token name ("=" �տ� �ִ� token) */
		char *string	/* ã�� ������ ����� string */
		)
{
	FILE    *fp;
	char    getBuf[256],*next;

	/* file�� ����.
	*/
	if ((fp = fopen (fname,"r")) == NULL) {
		fprintf(stderr,"[conflib_getStringInFileSection] fopen fail[%s]; errno=%d(%s)\n",fname,errno,strerror(errno));
		return -1;
	}

	/* section�� ������ġ�� �̵��Ѵ�.
	*/
	if (conflib_seekSection(fp,section) < 0) {
		fprintf(stderr,"[conflib_getStringInFileSection] not found section[%s] in file[%s]\n",section,fname);
		fclose(fp);
		return -1;
	}

	/* section������ keyword�� �ش��ϴ� line�� ã�� "=" ���� ������ string�� �����Ѵ�.
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
* fopen���� �����ִ� ���Ͽ��� ������ section���� fp�� �̵��Ѵ�.
* - ������ ó������ ���پ� �о� ������ section�� ã��, line_number�� return�Ѵ�.
* - conf ���Ͽ��� section�� ������ "[xxxx]"�� �ִ� �ٺ��� ���۵Ǿ� ���� section��
*	���۱����̴�.
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
* fopen���� �����ִ� ���Ͽ��� ���� fp��ġ�������� ���� section������ ������ keyword��
*	�ش��ϴ� line�� ã��, "=" ���� ������ string�� �����Ѵ�.
* - �˻��� line_number�� return�Ѵ�.
------------------------------------------------------------------------------*/
int conflib_getStringInSection (
		FILE *fp,		/* file pointer */
		char *keyword,	/* keyword -> ã���� �ϴ� ���ο� �ִ� ù��° token name ("=" �տ� �ִ� token) */
		char *string	/* ã�� ������ ����� string */
		)
{
	char    getBuf[256],token[CONFLIB_MAX_TOKEN_LEN],*next;
	int		lNum=0;

	/* section������ keyword�� �ش��ϴ� line�� ã�� "=" ���� ������ string�� �����Ѵ�.
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
			for (; isspace(*next); next++) ; /* ���� white-space�� ���ش�.*/
			strcpy (string, next);
			return lNum;
		}
	}
	fprintf(stderr,"[conflib_getStringInSection] not found keyword[%s]\n", keyword);
	return -1;

} /** End of conflib_getStringInSection **/
