#ifndef __UTILLIB_H__
#define __UTILLIB_H__

/**
	@file		utillib.h
	@author		
	@date		2011-07-08
	@version	v0.1
	@brief		유틸 라이브러리 헤더
	자세한 설명\n
	유틸\n
	라이브러리\n
	헤더
*/

#include <stdio.h>
#include <string.h> 
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#include <ctype.h>

#define E_INVALID_PARAM		-1
#define E_ENDOFDATA			-2

//#define MAX_URL_SIZE				556
//#define MAX_URL_LEN					(MAX_URL_SIZE - 1)

#define VALIDCHAR(c)				( ( 33 <= (c) )? 1: -1 )
//#define MIN(a,b)            		(((a) < (b)) ? (a) : (b))

typedef struct _st_NID
{
    char        	ucReserved;
    unsigned char	ucSysType;      /*< 1 : SESSIF, 2 : SESSSVC, 3 : NTAFUIF, 4 : DSCPIF, 5 : NTAFTIF, 6 :NTAFUIF */
    short			usSerial;       /*< Serial Number */
    time_t			stBuild;        /*< Build Time */
} st_NID, *pst_NID;

typedef union _un_NID
{
    long long   llNID;
    st_NID  stNID;
} un_NID, *pun_NID;

/**
	함수 선언
*/
extern void util_makenid(unsigned char ucSysType, long long *pllNID);
extern int util_getblocknums(char *fn, char *szProcName);
extern long long util_getmicrodelta(struct timeval *tv1, struct timeval *tv2);
extern unsigned short util_cvtushort(unsigned short value);
extern unsigned int util_cvtuint(unsigned int value);
extern int util_cvtint(int value); 
extern long long util_cvtint64(long long value);
extern char* util_cvtipaddr(char* szIP, unsigned int uiIP);
extern time_t util_cvttimet(time_t value);
extern unsigned long util_cvtulong(unsigned long value);
extern char* util_strnpbrk(const char *s1, const char *s2, size_t n);
extern void util_strtolower(char *src);
extern char* util_getendtag(const char* tar, const char* end);
extern void util_strcpytolower(char *dst, const char *src);
extern int util_strncpytolower(char *dst, const char *src, int size);
extern int util_urlencode(unsigned char *source, unsigned char *dest);
extern int util_urldecode(unsigned char *source, unsigned char *dest);
extern int util_urlndecode(unsigned char *source, unsigned char *dest, int maxsize);
extern char* util_passinvalidchar(unsigned char* sc1, const unsigned char* endPtr);
//extern int util_getabsurl(char* szReqUrl, unsigned short usReqUrlOptPos, const char* szResUrl, int dResUrlLen, char* szResAbsUrl);
extern int util_getquerystrpos(const char* resabsurl);
extern char* util_findtopdir(const char* url, const char* endurl);
extern char* util_getlastdir(const char* endurl, const char* topurl);
extern short util_rearrangeurl(const char* tarurl, const char* topdir, const char* endurl, char* returl);
extern char* util_valparsing(char* sc1, char* endPtr, int* len);
extern long long util_timevalsubop(struct timeval timeval1, struct timeval timeval2);
extern void util_makehashcode(const char *pszURLBuf, char *pszHash, int dLength, int maxHashLen);
extern char* util_trimleft(char* szSrc, char* szSkipChar);
extern char *util_printtime(time_t tTime, char *szTime);

#endif /* __UTILLIB_H_ */
