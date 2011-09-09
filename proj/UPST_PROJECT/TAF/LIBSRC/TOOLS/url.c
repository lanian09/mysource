/*******************************************************************************
			DQMS Project

	Author   : Lee Dong-Hwan
	Section  : UTILLIB
	SCCS ID  : @(#)url.c	1.1
	Date     : 03/03/04
	Revision History :

		'04.    03. 03. initial

	Description :

	Copyright (c) uPRESTO 2005
*******************************************************************************/

/**A.1*  File Inclusion *******************************************************/
#include <common_stg.h>

#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>

#include <define.h>
#include <utillib.h>

/** B. DEFINITION OF NEW CONSTANTS ********************************************/

/** C. DEFINITION OF NEW TYPES ************************************************/

/** D. DECLARATION OF VARIABLES ***********************************************/

/** E.1 DEFINITION OF FUNCTIONS ***********************************************/

char *strnpbrk(const char *s1, const char *s2, size_t n)
{   /* find index of first s1[i] that matches any s2[] */
	const char *sc1, *sc2;
	char equal = 0;

	for (sc1 = s1; (*sc1 != '\0') && (sc1 < s1+n); ++s1) // search at most n from s1
	{
		sc1 = s1;
		for (sc2 = s2; *sc2 != '\0'; ++sc2)
		{
			if (*sc1 == *sc2)
			{
				equal = 1;
			}
			else
			{
				equal = 0;
				break;
			}
			++sc1;
		}
		if ( equal > 0 )
			return ((char *) s1);
	}

	return NULL;
}

int urlndecode(unsigned char *source, unsigned char *dest, int maxsize)
{
    int num=0, i, index=0;
    int retval=0;

	if(source==NULL || dest==NULL )
		return -1;

    while(*source)
    {
        if (*source == '%')
        {
            num = 0;
            retval = 0;

			if ( *(source+1)=='2' && *(source+2) =='5' )
			{
				source+=2;
			}

            for (i = 0; i < 2; i++)
            {
                source++;

				if ( *source==0 )
				{
					dest[index]=0;
					return index;
				}

                if (*(source) < ':')
                {
                    num = *(source) - 48;
                }
                else if (*(source) > '@' && *(source) < '[')
                {
                    num = (*(source) - 'A')+10;
                }
                else
                {
                    num = (*(source) - 'a')+10;
                }

                if ((16*(1-i)))
                    num = (num*16);
                retval += num;
            }
            dest[index] = retval;
            index++;
        }
        else
        {
            dest[index] = *source;
            index++;
        }
        source++;
		if(index==maxsize-1)
			break;
    }
	dest[index]=0;
    return index;
}

int str_tolower( char *src )
{
	while ((*src != 0))
	{
		*src = (char)tolower(*(unsigned char *) src);
		src++;
	}
	return 1;
}

char* getEndTag( const char* tar, const char* end )
{
	while( *tar!=0 && tar<end && *tar!='<' && *tar!='>' )
		tar++;

	return ( *tar=='>' || *tar=='<' )?(char*)tar: NULL;
}

int strcopy_tolower( char *dst, const char *src )
{
	while ((*src != '\0'))
	{
		*dst = (char)tolower(*(unsigned char *) src);
		src++;
		dst++;
	}
	return 1;
}

int strncopy_tolower( char *dst, const char *src, int size )
{
	int cnt=0;
	while ((*src != '\0') && cnt<size)
	{
		*dst = (char)tolower(*(unsigned char *) src);
		src++;
		dst++;
		cnt++;
	}
	return cnt;
}

int urlencode(unsigned char *source, unsigned char *dest)
{
    unsigned char hex[4];
    unsigned char *sbuf;
    int size = 0;
    sbuf = dest;
    while(*source)
    {
        if ((*source > 47 && *source < 57) ||
            (*source > 64 && *source < 92) ||
            (*source > 96 && *source < 123) ||
            *source == '-' || *source == '.' || *source == '_')
        {
            *sbuf = *source;
        }
        else
        {
            sprintf(hex, "%%%02X", *source);
            strncat(sbuf, hex,3);
            sbuf++;
            sbuf++;
            size += 2;
        }
        source++;
        sbuf++;
        size++;
    }
    return size;
}

int urldecode(unsigned char *source, unsigned char *dest)
{
    int num=0, i, index=0;
    int retval=0;

	if(source==NULL || dest==NULL )
		return -1;

    while(*source)
    {
        if (*source == '%')
        {
            num = 0;
            retval = 0;

			if ( *(source+1)=='2' && *(source+2) =='5' )
			{
				source+=2;
			}

            for (i = 0; i < 2; i++)
            {
                source++;

				if ( *source==0 )
				{
					dest[index]=0;
					return index;
				}

                if (*(source) < ':')
                {
                    num = *(source) - 48;
                }
                else if (*(source) > '@' && *(source) < '[')
                {
                    num = (*(source) - 'A')+10;
                }
                else
                {
                    num = (*(source) - 'a')+10;
                }

                if ((16*(1-i)))
                    num = (num*16);
                retval += num;
            }
            dest[index] = retval;
            index++;
        }
        else
        {
            dest[index] = *source;
            index++;
        }
        source++;
    }
	dest[index]=0;
    return index;
}

int isRedirected(unsigned short respcode)
{
	int ret;
	switch (respcode)
	{
	case 300:
	case 301:
	case 302:
		ret = 1;
		break;
	default:
		ret=-1;
		break;
	}
	return ret;
}

#if 0
char *strnpbrk(const char *s1, const char *s2, size_t n)
{	/* find index of first s1[i] that matches any s2[] */
	const char *sc1, *sc2;
	char equal = 0;

	for (sc1 = s1; (*sc1 != '\0') && (sc1 < s1+n); ++s1) // search at most n from s1
	{
		sc1 = s1;
		for (sc2 = s2; *sc2 != '\0'; ++sc2)
		{
			if (*sc1 == *sc2)
			{
				equal = 1;
			}
			else
			{
				equal = 0;
				break;
			}
			++sc1;
		}
		if ( equal > 0 )
			return ((char *) s1);
	}

	return NULL;
}

#endif
char *strnpcasebrk(const char *s1, const char *s2, size_t n)
{   /* find index of first s1[i] that matches any s2[] */
    const char *sc1, *sc2;
    char equal = 0;

    for (sc1 = s1; (*sc1 != '\0') && (sc1 < s1+n); ++s1) // search at most n from s1
    {
        sc1 = s1;
        for (sc2 = s2; *sc2 != '\0'; ++sc2)
        {
            if (tolower(*sc1) == tolower(*sc2))
            {
                equal = 1;
            }
            else
            {
                equal = 0;
                break;
            }
            ++sc1;
        }
        if ( equal > 0 )
            return ((char *) s1);
    }

    return NULL;
}

/***
	result:
		return Pointer when Be appeared An Character.
		if reached End Of Data, return NULL
***/
char* passInvalidChar(unsigned char* sc1, const unsigned char* endPtr )
{
	while( VALIDCHAR( *sc1 ) < 0 )
	{
		++sc1;
		if ( sc1 >= endPtr )
		{
			return NULL;
		}
	}

	return sc1;
}

/*
	in argu:
		reqUrl - HTTP세션의 Request URL
		_url	 - 절대 주소로 만들고자 하는 대상
		host - HTTP세션의 Header의 host필드 값: 사용 안함
	out args:
		absUrl - url의 절대 주소

	return
		< 0 : 오류 발생
		> 0 : 정상 수행
*/

int getAbsUrl( char* szReqUrl,
	unsigned short usReqUrlOptPos,
	const char* szResUrl,
	int dResUrlLen,
	char* szResAbsurl )
{
	char szUrl[MAX_URL_LEN];
	char tmpurl[MAX_URL_LEN];
	short	tmpurllen;
	char* tmpurlroot;
	char* resurl=(char*)szResUrl;
	char* resurlroot;
	int resurllen = dResUrlLen;
	int len;

	if ( resurl==NULL || resurllen<=0 )
	{
		dAppLog(LOG_DEBUG, "getAbsUrl: Invalid Parameter(URL)" );
		return -1;		/* modifyme Invalid Parameter */
	}

	if( (resurl = passInvalidChar( resurl, resurl+resurllen )) == NULL )	/* 찾지 못함 */
	{
		return -2;	/* modifyme End Of Data */
	}

	resurllen = szResUrl+resurllen - resurl;

	if ( ( resurlroot=findTopDir( resurl, resurl+resurllen ) )!=NULL )
	{
		strncpy( tmpurl, resurl, resurllen );
		tmpurllen = resurllen;
		tmpurlroot = tmpurl + ( resurlroot-resurl );
		if ( resurlroot[0] != '/' )
		{
			tmpurlroot[0]='/';
			tmpurllen++;
		}
	}
	else
	{
		/* resurl 은 상대 URL */
		char* requrl = szReqUrl;
		char* requrlroot;
		char* endrequrl = szReqUrl+usReqUrlOptPos;		/* URL의 끝을 가르킴 */
		char* resrelurl = resurl;

		/* URL에서 RootDirectory위치 얻어 오기 */
		if ( ( requrlroot = findTopDir( requrl, endrequrl ) )==NULL )
		{
			/* 절대 URL을 만들지 않음/프로토콜이 없는 경우에는 */
			strncpy(tmpurl, resurl, dResUrlLen );
			tmpurl[dResUrlLen] = 0;

			dResUrlLen = urlndecode(tmpurl, szResAbsurl, MAX_URL_LEN-1);
			szResAbsurl[dResUrlLen] = 0;

			return 1;
		}
		else
		{
			tmpurllen = requrlroot - requrl;
			strncpy( tmpurl, requrl, tmpurllen );
			tmpurlroot = tmpurl+tmpurllen;
		}

		if ( resurl[0] == '/' )		/* 1st Character of Resource Url Is ROOT */
		{
			resurllen = min( resurllen, MAX_URL_LEN-1-tmpurllen );
			strncpy( tmpurlroot, resrelurl, resurllen );
			tmpurllen += resurllen;
			tmpurl[tmpurllen]=0;
		}
		else if ( resurl[0] == '?' )	/* 1st Character of Resource Url Is Replace Option */
		{
			len = min( (endrequrl-requrlroot), MAX_URL_LEN-1-tmpurllen );
			strncpy( tmpurlroot, requrlroot, len );
			tmpurllen += len;

			resurllen = min( resurllen, MAX_URL_LEN-1-tmpurllen );
			strncpy( tmpurl+tmpurllen, resurl, resurllen );
			tmpurllen += resurllen;

			tmpurl[tmpurllen]=0;
		}
		else
		{
			char* bottomdir;
			bottomdir = getLastDir( endrequrl, requrlroot );
			if(bottomdir==NULL )
			{
				tmpurlroot[0]='/';
				tmpurllen++;
				resurllen = min( resurllen, MAX_URL_LEN-1-tmpurllen );
				strncpy( tmpurl+tmpurllen, resrelurl, resurllen );
				tmpurllen += resurllen;

				tmpurl[tmpurllen]=0;
			}
			else
			{
				if( requrlroot[0]=='/' )
				{
					len = min( bottomdir-requrlroot, MAX_URL_LEN-1-tmpurllen );
					strncpy( tmpurlroot, requrlroot, len );
					tmpurllen += len;
				}
				else
				{
					tmpurlroot[0]='/';
					tmpurlroot++;
					tmpurllen++;
					len = min( bottomdir-requrlroot, MAX_URL_LEN-1-tmpurllen );
					strncpy( tmpurlroot, requrlroot, len );
					tmpurllen += len;
				}

				if ( tmpurllen<MAX_URL_LEN-1 )
				{
					tmpurl[tmpurllen++]='/';

					resurllen = min( resurllen, MAX_URL_LEN-1-tmpurllen );
					strncpy( tmpurl+tmpurllen, resrelurl, resurllen );
					tmpurllen += resurllen;
				}
				tmpurl[tmpurllen]=0;
			}
		}
	}

	tmpurllen = rearrangeUrl( tmpurl, tmpurlroot, tmpurl+tmpurllen, szUrl );	/* "./"과 "../" 처리 */
	tmpurllen = urlndecode(szUrl, tmpurl, MAX_URL_LEN-1);
	tmpurl[tmpurllen] = 0;

	strcpy( szResAbsurl, tmpurl );

	return 1;
}

int dGetOptPos( const char* resabsurl )
{
	char* str=(char*)resabsurl;

	while(*str!='?' && *str!=0 )
		str++;

	return str-resabsurl;
}

/* url에서 최상위 디렉토리의 위치를 리턴 */
char* findTopDir(const char* url, const char* endurl)
{
	int len;
	char* rootIndicator = (char*)url;
	len = endurl - url;

	if ( len>7
		&& strncasecmp( url, "http://", 7 )==0 )
	{
		rootIndicator+=7;
		if ( ( rootIndicator = strchr(rootIndicator, '/' ) ) == NULL
			|| rootIndicator >= endurl )
			rootIndicator = (char*)endurl;

		return rootIndicator;
	}

	return NULL;	/* Invalid Parameter */
}

/* 바로 상위의 폴더위치를 찾아줌
   endurl은 찾을 url
   topurl는 root폴더 */
char* getLastDir(const char* endurl, const char* topurl)
{
	while( endurl >= topurl )
	{
		if(*endurl=='/' )
			return (char*)endurl;

		endurl--;
	}
	return NULL;
}

/*
   INPUT>
   URL에서 "./", "../"등을 URL에서 적절하게 제거
tarurl: "./", "../"을 포함하는 URL, 반드시 프로토콜을 포함하여야함
topdir: tarurl의 최상위 디렉토리
returl: tarurl을 변경한 결과

	RETURN>
	returl의 길이
 */
short rearrangeUrl( const char* tarurl, const char* topdir, const char* endurl, char* returl )
{
	char* start = returl;
	char* returltop;

	strncpy( returl, tarurl, topdir-tarurl ); /* Root 디렉토리까지는 복사 */
	returl+=topdir-tarurl;
	returltop = returl;
	tarurl=topdir;

	while ( tarurl<endurl )
	{
		/* "./"와 "../"의 처리 */
		if ( tarurl[0]=='.' )
		{
			if ( tarurl+1<endurl && tarurl[1]=='/' )
			{
				tarurl+=2;
				continue;
			}
			else if ( tarurl+2<endurl && tarurl[1]=='.' && tarurl[2]=='/' )
			{
				char *lasturl = getLastDir(tarurl-2, topdir);
				if( lasturl != NULL )
				{
					lasturl = getLastDir(returl-2, returltop); // 현재 위치에서 상위 디렉토리를 찾음, tarurl의 복사본이므로 tarurl에 존재했다면 없을 수 없음
					if (lasturl!=NULL)
					{
						returl=lasturl+1; /* 상위 디렉토리로 돌아감 */
						tarurl+=3; /* "../"만큼 이동 */
					}
					else
					{
						*returl = *tarurl;
						*(returl+1) = *(tarurl+1);
						*(returl+2) = *(tarurl+2);
						returl+=3;
						tarurl+=3;
					}
				}
				else
				{
					*returl = *tarurl;
					*(returl+1) = *(tarurl+1);
					*(returl+2) = *(tarurl+2);
					returl+=3;
					tarurl+=3;
				}
				continue;
			}
		}

		*returl = *tarurl;
		tarurl++;
		returl++;
	}
	*returl=0;

	return returl-start;
}

/***
	output argument
		len : 패킷의 길이 값
	return 값
		파싱 후 다음 포인터
		NULL: 오류발생

	comment:
		return 값에 길이 값 만큼 빼서 value를 얻을 수 있다.
		value는 ['/"/(empty)]value['/"/(empty)] 형태의 문자열이다.
***/
char* valParsing( char* sc1, char* endPtr, int* len )
{
	char* ptrValueStart;
	char* ptrValueEnd;
	char cValueTermCodeType = ' ';

	if ( (sc1 = passInvalidChar( sc1, endPtr )) == NULL )	/* 찾지 못함 */
		return NULL;

	if ( *sc1 == '=' )
	{
		++sc1;

		if ( ( sc1 = passInvalidChar( sc1, endPtr ) ) == NULL )	/* 찾지 못함 */
			return NULL;

		if ( *sc1=='\'' || *sc1=='"' )
		{
			cValueTermCodeType = *sc1;
			ptrValueStart = sc1;
			++sc1;
		}
		else
			ptrValueStart = sc1;

//		if ( ( sc1 = strchr( sc1, cValueTermCodeType ) ) == NULL && cValueTermCodeType!=' ' )
		if ( (( sc1 = strchr( sc1, cValueTermCodeType ) ) == NULL && (cValueTermCodeType!=' ')) || (sc1 >= endPtr) )
                return NULL;
        ptrValueEnd = endPtr;

		if ( sc1 < endPtr && cValueTermCodeType!=' ' )
		{
            ++sc1;
            ptrValueEnd = sc1;
		}
		else if ( sc1 < endPtr && cValueTermCodeType==' ' )
		{
			ptrValueEnd = sc1;
		}
		else if ( cValueTermCodeType == ' ' && *(sc1-1) == '/' )
		{
			ptrValueEnd = sc1-1;
		}

		*len = ptrValueEnd - ptrValueStart;
	}
	else
		return NULL;

	return ptrValueEnd;
}

long long timevalSubOp( struct timeval timeval1, struct timeval timeval2 )
{
	/* timeval1 - timeval2 */
	long long ret;
	long sec;
	long usec;
	sec = timeval1.tv_sec - timeval2.tv_sec;
	usec = timeval1.tv_usec - timeval2.tv_usec;

	if ( sec < 0 || ( sec==0 && usec<0 ) )
		return -1;

	ret = sec*1000000 + usec;
	return ret;
}

int dMakeHashCode( const char *pszURLBuf, char *pszHash, int dLength )
{
	int         		j, dIndex;
	unsigned char       szResultHash[8] = {0,};

	memset( pszHash, 0, MAX_HASHCODE_LEN );

	for( dIndex=0; dIndex<(dLength%8==0?dLength:dLength+(8-dLength%8)); dIndex++) {
		if( dIndex>(dLength-1))
			break;
		szResultHash[dIndex%8] += pszURLBuf[dIndex];
	}

	for( dIndex=0; dIndex<8; dIndex++ ) {
		if( dIndex > (dLength-1) ) {
			sprintf( pszHash+dIndex*2, "00");
			continue;
		}
		if( szResultHash[dIndex]<=0xf )
			sprintf( pszHash+dIndex*2, "0%x", szResultHash[dIndex] );
		else
			sprintf( pszHash+dIndex*2, "%2x", szResultHash[dIndex] );
	}


	for( j=0; j<MAX_HASHCODE_LEN; j++)
	{
		if ( pszHash[j]>0x60 )
			pszHash[j]-=0x20;
	}

	return 1;
}

char* pTrimLeft( char* szSrc, char* szSkipChar )
{
	int i=0;

	while ( *szSrc!=0x0 )
	{
		for( i=0; szSkipChar[i]!=0x0 && (*szSrc!=szSkipChar[i]); i++ )
			;

		if ( *szSrc!=szSkipChar[i] )
			return szSrc;

		szSrc++;
	}

	return szSrc;
}


