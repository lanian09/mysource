/* 
 * $Id: body1.c,v 1.1.1.1 2011/08/29 05:56:44 dcham Exp $
 */
#include <stdio.h>
#include <string.h>
#include "aqua.h"

main()
{
	FILE *fp;
	char buf[BUFSIZ];
	int start = 0;
	char parse[BUFSIZ];
	char sss[BUFSIZ];
	int  i, j;
	BODY *pBODY;
	BODY httpURL,*phttpURL;
	int doublecomma,singlecomma;
#define	URLTYPE_HTTP		1
#define	URLTYPE_SLASH		2
#define	URLTYPE_COMMA		3
	int urltype;
	struct timeval			stStart, stLast;
	struct tm				*pstTime;
	long long				llDelTime;

	BODY  aaa;
	int parse_len;


	phttpURL = &httpURL;
	sprintf(buf,"http://ktf.magicn.com:8080/AAAA/BBBB/CCCCC/t.asp?a=3");

	pBODY = (BODY *) sss;
	memset(buf,0,BUFSIZ);
	memset(parse,0,BUFSIZ);
	memset(sss,0,BUFSIZ);
	fp = fopen("../DATA/BODY1.DAT","r");
	while(fgets(buf,BUFSIZ,fp)){
		sprintf(parse,"%s%s",parse,buf);
	}
	printf("%s\n",parse);
	parse_len = strlen(parse);

		BODY_LEX(parse,parse_len,sss);
		{ BODY_Prt("TT",pBODY);}
}


/* 
 * $Log: body1.c,v $
 * Revision 1.1.1.1  2011/08/29 05:56:44  dcham
 * NEW OAM SYSTEM
 *
 * Revision 1.1  2011/08/19 04:25:50  uamyd
 * CGALIB moved in DQMS
 *
 * Revision 1.1  2011/08/03 06:02:45  uamyd
 * CGA, HASHO, TIMERN library added
 *
 * Revision 1.2  2011/01/11 04:09:04  uamyd
 * modified
 *
 * Revision 1.1.1.1  2010/08/23 01:13:06  uamyd
 * DQMS With TOTMON, 2nd-import
 *
 * Revision 1.1  2009/06/10 16:45:50  dqms
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2009/05/26 02:13:35  dqms
 * Init TAF_RPPI
 *
 * Revision 1.1.1.1  2008/06/09 08:17:19  jsyoon
 * WATAS3 PROJECT START
 *
 * Revision 1.1  2007/08/21 12:22:20  dark264sh
 * no message
 *
 * Revision 1.3  2006/10/19 06:44:12  cjlee
 * *** empty log message ***
 *
 * Revision 1.2  2006/10/19 06:43:14  cjlee
 * *** empty log message ***
 *
 * Revision 1.4  2006/10/11 08:28:10  cjlee
 * *** empty log message ***
 *
 * Revision 1.3  2006/10/11 06:44:14  shlee
 * body.c
 *
 * Revision 1.2  2006/10/11 06:29:23  shlee
 * 1000000 Loop Test && Block Printf
 *
 * Revision 1.1  2006/10/11 05:08:02  cjlee
 * INIT
 *
 * Revision 1.3  2006/10/11 00:57:06  cjlee
 * ???
 *
 */
