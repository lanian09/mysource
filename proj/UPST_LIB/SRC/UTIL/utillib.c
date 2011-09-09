/**
	@file		utillib.c
	@author		
	@date		2011-07-08
	@version	v0.1
	@see		typedef.h
	@see		utillib.h
	@brief		À¯Æ¿ ¶óÀÌºê·¯¸®(ÇÑÁÙ¼³¸í)
	¿©·¯ÁÙ ¼³¸í\n
	µÎ¹øÂ°ÁÙ\n
	¼¼¹øÂ°ÁÙ
*/

#include "utillib.h"


/**
	@brief NID¸¦ ¸¸µé¾î ÁÖ´Â ÇÔ¼ö
	@param ucSysType unsigned char, ü ½Ã½ºÅÛ Å¸
	@param pllNID long long*, NID¸¦ ÀúÀåÇÒ Æ÷ÀÎÅÍ º¯¼ö
	@see _st_NID
	@see _un_NID			
*/
void util_makenid(unsigned char ucSysType, long long *pllNID)
{
	/**
		ÇÔ¼ö ¿ÜºÎ¿¡ ¼±¾ğµÇ¾î ÀÖ´ø Global º¯¼ö
	*/
	static time_t dOldTime;			/*< NID¸¦ »ı¼ºÇß´ø ½Ã°£ */
	static unsigned short usSerial = 1;		/*< un_NID ±¸Á¶Ã¼ÀÇ usSerial ÃÊ±â°ª */

    time_t      dNow;				/*< ÇöÀç½Ã°£À» ´ãÀ» time_t º¯¼ö */
    un_NID      unNID;				/*< un_NID ±¸Á¶Ã¼ */

    time(&dNow);
    if(dOldTime != dNow)
    {   
        usSerial    = 1;
        dOldTime    = dNow;
    }   

    unNID.llNID             = 0;
    unNID.stNID.ucSysType   = ucSysType;
    unNID.stNID.usSerial    = usSerial++;
    unNID.stNID.stBuild     = dNow;

    *pllNID = unNID.llNID;
}

/**
	@brief util_getblocknums ÇÔ¼ö
	@param fn
	@param szProcName
	@return	rdcnt
*/
int util_getblocknums(char *fn, char *szProcName)
{
	int     ln, dProcLen;
	int     rdcnt;
	int     scan_cnt;
	char    buf[1024];
	char    Bname[16];
	FILE    *fp;

	fp = fopen(fn, "r");
	if( NULL == fp ){
		return -1;  /* fopen error */
	}

	ln = 0;
	rdcnt = 0;
	dProcLen = strlen(szProcName);
	while(fgets(buf, 1024, fp) != NULL ) {

		ln++;
		/*
		 * from Source to Target : sscanf
		 */
		if( buf[0] != '#' ) {
			//printf("SYNTAX ERROR FILE:%s, LINE:%d - FIRST CHARACTER IS MUST '#'!\n",fn, ln);
			return -1;		/* SYSTAX_ERROR, FIRST CHARACTER MUST BE '#'*/
		}
		else if( buf[1] == '#' ) {
			continue;
		}
		else if( buf[1] == 'E' ) {
			/*
			 * EOF
			 */
			break;
		}
		else if( buf[1] == '@' ) {
			scan_cnt = sscanf( &buf[2], "%s %*s", Bname );
			if(scan_cnt == 1) {
				if(!strncasecmp(Bname, szProcName, dProcLen))
					rdcnt++;
			}
		}
		else {
			//printf("SYNTAX ERROR FILE:%s, LINK:%d\n",fn, ln);
			return -2;		/* SYNTAX ERROR */
		}

	} /* while */
	fclose(fp);

	return rdcnt;
}

/**
	@brief util_getmicrodelta ¼³¸í
	@param tv1
	@param tv2
	@return tv1-tv2 micro time
	@see struct timeval
*/
long long util_getmicrodelta(struct timeval *tv1, struct timeval *tv2)
{
    return( ((long long)tv1->tv_sec*1000000LL + (long long)tv1->tv_usec)
        - ((long long)tv2->tv_sec*1000000LL + (long long)tv2->tv_usec));
}

/**
	@brief util_cvtusort ¼³¸í
	@param value unsigned shortÇü °ª
	@return value °ªÀÇ bit ¸¦ ¿ª¼øÀ¸·Î ¹Ù²Û °ª
*/
unsigned short util_cvtushort(unsigned short value)
{
	union {
		unsigned short xValue;
		char ml[2];
	} u1, u2;

	u1.xValue = value;

	u2.ml[0] = u1.ml[1];
	u2.ml[1] = u1.ml[0];

    return u2.xValue;
}

/**
	@brief util_cvtuint ¼³¸í
	@param value unsigned intÇü °ª
	@return value °ªÀÇ bit ¸¦ ¿ª¼øÀ¸·Î ¹Ù²Û °ª
*/
unsigned int util_cvtuint(unsigned int value)
{
	union {
		unsigned int xValue;
		char ml[4];
	} u1, u2;

	u1.xValue = value;

	u2.ml[0] = u1.ml[3];
	u2.ml[1] = u1.ml[2];
	u2.ml[2] = u1.ml[1];
	u2.ml[3] = u1.ml[0];

    return u2.xValue;
}

/**
	@brief util_cvtint ¼³¸í
	@param value INTÇü °ª
	@return value °ªÀÇ bit ¸¦ ¿ª¼øÀ¸·Î ¹Ù²Û °ª
*/
int util_cvtint(int value) 
{ 
	union { 
		int xValue; 
		char ml[4]; 
	} u1, u2; 

	u1.xValue = value; 

	u2.ml[0] = u1.ml[3]; 
	u2.ml[1] = u1.ml[2]; 
	u2.ml[2] = u1.ml[1]; 
	u2.ml[3] = u1.ml[0]; 

	return u2.xValue; 
}

/**
	@brief util_cvtint64 ¼³¸í
	@param value long longÇü °ª
	@return value °ªÀÇ bit ¸¦ ¿ª¼øÀ¸·Î ¹Ù²Û °ª
*/
long long util_cvtint64( long long value )
{
	union {
		long long xValue;
		char ml[8];
	} u1, u2;

	u1.xValue = value;

	u2.ml[0] = u1.ml[7];
	u2.ml[1] = u1.ml[6];
	u2.ml[2] = u1.ml[5];
	u2.ml[3] = u1.ml[4];
	u2.ml[4] = u1.ml[3];
	u2.ml[5] = u1.ml[2];
	u2.ml[6] = u1.ml[1];
	u2.ml[7] = u1.ml[0];

    return u2.xValue;
}

/**
	@brief util_cvtipaddr ¼³¸í
	@param szIP	¹®ÀÚ¿­·Î º¯È¯ÇÑ I¤IPÁÖ¼Ò ÀúÀå char* ¼ö
				NULLÀÌ ¾Æ´Ï¸é reversed uiIP ÀÇ inet_ntaÀÇ °ªÀ»,
				NULL ÀÌ¸é inet_ntoa ÀÇ ¸®ÅÏ°ªÀ» ¸®ÅÏÇÑ´Ù.				
	@param uiIP	UNITÇü IPÁÖ¼Ò
	@return IPÁÖ¼Ò ¹®ÀÚ¿­ Æ÷ÀÎÅÍ
	@see in_addr
	@see inet_ntoa
	@see INET_ADDRSTRLEN
*/
char* util_cvtipaddr(char* szIP, unsigned int uiIP)
{
	struct in_addr  inaddr;

	if(szIP == NULL)
	{
		inaddr.s_addr = uiIP;
		return (char*)inet_ntoa(inaddr);
	}
	else
	{
		inaddr.s_addr = util_cvtuint(uiIP);
		strncpy(szIP, (char*)inet_ntoa(inaddr), INET_ADDRSTRLEN);
		return szIP;
	}
}

/**
	@brief util_cvttimet ¼³¸í
	@param value ¹Ù²Ü time_t °ª
	@return ºñÆ®°ªÀÌ ¿ª¼øÀ¸·Î ¹Ù²ï °ª
	@see time_t
*/
time_t util_cvttimet(time_t value)
{
    int i;
    time_t xValue;

    char tszValue[4];
    char tszValue2[4];

    memcpy( tszValue, &value, sizeof(time_t) );

    for( i=0; i< 4; i++)
    {
        tszValue2[i] = tszValue[3-i];
    }

    memcpy( &xValue, tszValue2, sizeof(time_t));

    return xValue;
}

/**
	@brief util_cvtulong ¼³¸í
	@param value bit¸¦ ¿ª¼øÀ¸·Î ¹Ù²Ü °ª
	@return bit°¡ ¿ª¼øÀ¸·Î ¹Ù²ï °ª
*/
unsigned long util_cvtulong(unsigned long value)
{
    int i;
    unsigned long xValue;

    char tszValue[4];
    char tszValue2[4];

    memcpy( tszValue, &value, sizeof(unsigned long) );

    for( i=0; i< 4; i++)
    {
        tszValue2[i] = tszValue[3-i];
    }

    memcpy( &xValue, tszValue2, sizeof(unsigned long));

    return xValue;
}

/**
	@brief util_strnpbrk ¼³¸í
	@param s1
	@param s2
	@param n 
	@return s1
	@return NULL s2°¡ s1¿¡ Á¸ÀçÇÏÁö ¾Ê´Â °æ¿ì 
*/
char* util_strnpbrk(const char *s1, const char *s2, size_t n)
{
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

/**
	@brief ¹®ÀÚ¿­À» ¼Ò¹®ÀÚ·Î º¯È¯
	@param src º¯È¯ÇÒ ¹®ÀÚ¿­
*/
void util_strtolower( char *src )
{
	while ((*src != 0))
	{
		*src = (char)tolower(*(unsigned char *) src);
		src++;
	}
}

/**
	@brief util_getendtag ¼³¸í
	@param tar ¼³¸í
	@param end ¼³¸í
	@return ¼³¸í
*/
char* util_getendtag( const char* tar, const char* end )
{
	while( *tar!=0 && tar<end && *tar!='<' && *tar!='>' )
		tar++;

	return ( *tar=='>' || *tar=='<' )?(char*)tar: NULL;
}

/**
	@brief util_strcopytolower
	@param dst
	@param src
*/
void util_strcpytolower( char *dst, const char *src )
{
	while ((*src != '\0'))
	{
		*dst = (char)tolower(*(unsigned char *) src);
		src++;
		dst++;
	}
}

/**
	@brief util_strncpytolower
	@param dst
	@param src
	@param size º¹»çÇÒ ±æÀÌ
	@return º¹»çÇÑ ±æÀÌ
*/
int util_strncpytolower( char *dst, const char *src, int size )
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

/**
	@brief URL ÀÎÄÚµù ÇÔ¼ö
	@param source
	@param dest
	@return ÀÎÄÚµùµÈ ¹®ÀÚ¿­ ±æÀÌ
*/
int util_urlencode(unsigned char *source, unsigned char *dest)
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
            sprintf((char*)hex, "%%%02X", *source);
            strncat((char*)sbuf, (char*)hex,3);
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

/**
	@brief URL µğÄÚµù ÇÔ¼ö
	@param source
	@param dest
	@return µğÄÚµùµÈ ¹®ÀÚ¿­(dest) ±æÀÌ
	@return E_INVALID_PARAM
*/
int util_urldecode(unsigned char *source, unsigned char *dest)
{
    int num=0, i, index=0;
    int retval=0;

	if(source==NULL || dest==NULL )
		return E_INVALID_PARAM;

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

/**
	@brief URL µğÄÚµù ÇÔ¼ö
	@param source µğÄÚµù ÇÒ ¹®ÀÚ¿­
	@param dest µğÄÚµù µÈ ¹®ÀÚ¿­
	@param maxsize ­¸¸ source ¹®ÀÚ¿­ÀÇ maxsize ±æÀÌ¸¸Å­¸¸ µğ
	@return µğÄÚµùµÈ ¹®ÀÚ¿­ ±æÀÌ
	@return NULL if source == NULL or dest == NULL
*/
int util_urlndecode(unsigned char *source, unsigned char *dest, int maxsize)
{
    int num=0, i, index=0;
    int retval=0;

	if(source==NULL || dest==NULL )
		return E_INVALID_PARAM;

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

#if 0
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

#endif


/**
	@brief util_passinvalidchar
	@param sc1
	@param endPtr
	@return sc1
	@return NULL
			return Pointer when Be appeared An Character.
			if reached End Of Data, return NULL
	@see VALIDCHAR
*/
char* util_passinvalidchar(unsigned char* sc1, const unsigned char* endPtr)
{
	while( VALIDCHAR( *sc1 ) < 0 )
	{
		++sc1;
		if ( sc1 >= endPtr )
		{
			return NULL;
		}
	}

	return (char*)sc1;
}

/*
	in argu:
		reqUrl - HTTP¼¼¼ÇÀÇ Request URL
		_url	 - Àı´ë ÁÖ¼Ò·Î ¸¸µé°íÀÚ ÇÏ´Â ´ë»ó
		host - HTTP¼¼¼ÇÀÇ HeaderÀÇ hostÇÊµå °ª: »ç¿ë ¾ÈÇÔ
	out args:
		absUrl - urlÀÇ Àı´ë ÁÖ¼Ò

	return
		< 0 : ¿À·ù ¹ß»ı
		> 0 : Á¤»ó ¼öÇà
*/
/**
	@brief 
	@param	szReqUrl Request URL
	@param	usReqUrlOptPos
	@param	dResUrlLen
	@param	szResAbsUrl
	@return	> 0 : Á¤»ó
	@return	< 0 : ¿À·ù
	@todo	MAX_URL_LEN Á¦°Å	
*/
#if 0
int util_getabsurl(char* szReqUrl, unsigned short usReqUrlOptPos,
				const char* szResUrl, int dResUrlLen, char* szResAbsUrl)
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
		return E_INVALID_PARAM;		/* modifyme Invalid Parameter */
	}

	if( (resurl = util_passinvalidchar( resurl, resurl+resurllen )) == NULL)
	{
		/*< Ã£Áö¸øÇÔ */
		return E_ENDOFDATA;	/* modifyme End Of Data */
	}

	resurllen = szResUrl+resurllen - resurl;

	if ( ( resurlroot = util_findtopdir( resurl, resurl+resurllen ) )!=NULL )
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
		/* resurl Àº »ó´ë URL */
		char* requrl = szReqUrl;
		char* requrlroot;
		char* endrequrl = szReqUrl+usReqUrlOptPos;		/* URLÀÇ ³¡À» °¡¸£Å´ */
		char* resrelurl = resurl;

		/* URL¿¡¼­ RootDirectoryÀ§Ä¡ ¾ò¾î ¿À±â */
		if ( ( requrlroot = util_findtopdir( requrl, endrequrl ) )==NULL )
		{
			/* Àı´ë URLÀ» ¸¸µéÁö ¾ÊÀ½/ÇÁ·ÎÅäÄİÀÌ ¾ø´Â °æ¿ì¿¡´Â */
			strncpy(tmpurl, resurl, dResUrlLen );
			tmpurl[dResUrlLen] = 0;

			dResUrlLen = util_urlndecode(tmpurl, szResAbsUrl, MAX_URL_LEN-1);
			szResAbsUrl[dResUrlLen] = 0;

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
			resurllen = MIN(resurllen, MAX_URL_LEN-1-tmpurllen);
			strncpy( tmpurlroot, resrelurl, resurllen );
			tmpurllen += resurllen;
			tmpurl[tmpurllen]=0;
		}
		else if ( resurl[0] == '?' )	/* 1st Character of Resource Url Is Replace Option */
		{
			len = MIN( (endrequrl-requrlroot), MAX_URL_LEN-1-tmpurllen );
			strncpy( tmpurlroot, requrlroot, len );
			tmpurllen += len;

			resurllen = MIN( resurllen, MAX_URL_LEN-1-tmpurllen );
			strncpy( tmpurl+tmpurllen, resurl, resurllen );
			tmpurllen += resurllen;

			tmpurl[tmpurllen]=0;
		}
		else
		{
			char* bottomdir;
			bottomdir = util_getlastdir( endrequrl, requrlroot );
			if(bottomdir==NULL )
			{
				tmpurlroot[0]='/';
				tmpurllen++;
				resurllen = MIN( resurllen, MAX_URL_LEN-1-tmpurllen );
				strncpy( tmpurl+tmpurllen, resrelurl, resurllen );
				tmpurllen += resurllen;

				tmpurl[tmpurllen]=0;
			}
			else
			{
				if( requrlroot[0]=='/' )
				{
					len = MIN( bottomdir-requrlroot, MAX_URL_LEN-1-tmpurllen );
					strncpy( tmpurlroot, requrlroot, len );
					tmpurllen += len;
				}
				else
				{
					tmpurlroot[0]='/';
					tmpurlroot++;
					tmpurllen++;
					len = MIN( bottomdir-requrlroot, MAX_URL_LEN-1-tmpurllen );
					strncpy( tmpurlroot, requrlroot, len );
					tmpurllen += len;
				}

				if ( tmpurllen<MAX_URL_LEN-1 )
				{
					tmpurl[tmpurllen++]='/';

					resurllen = MIN( resurllen, MAX_URL_LEN-1-tmpurllen );
					strncpy( tmpurl+tmpurllen, resrelurl, resurllen );
					tmpurllen += resurllen;
				}
				tmpurl[tmpurllen]=0;
			}
		}
	}

	tmpurllen = util_rearrangeurl( tmpurl, tmpurlroot, tmpurl+tmpurllen, szUrl );	/* "./"°ú "../" Ã³¸® */
	tmpurllen = util_urlndecode(szUrl, tmpurl, MAX_URL_LEN-1);
	tmpurl[tmpurllen] = 0;

	strcpy( szResAbsUrl, tmpurl );

	return 1;
}
#endif /* getabsurl */

/**
	@brief util_getquerystrpos
	@param resabsurl
	@return QueryString ½ÃÀÛÀ§Ä¡
*/
int util_getquerystrpos( const char* resabsurl )
{
	char* str=(char*)resabsurl;

	while(*str!='?' && *str!=0 )
		str++;

	return str-resabsurl;
}

/**
	@brief URL ¿¡¼­ ÃÖ»óÀ§ µğ·ºÅä¸®ÀÇ À§Ä¡¸¦ ¸®ÅÏ
	@param url
	@param endurl
	@return ÃÖ»óÀ§ µğ·ºÅä¸® À§Ä¡ Æ÷ÀÎÅÍ
	@return NULL if Invalid Parameter
*/
char* util_findtopdir(const char* url, const char* endurl)
{
	int len;
	char* rootIndicator = (char*)url;
	len = endurl - url;

	if ( len > 7 && strncasecmp(url, "http://", 7 ) == 0)
	{
		rootIndicator+=7;

		if ( ( rootIndicator = strchr(rootIndicator, '/' ) ) == NULL
			|| rootIndicator >= endurl )
			rootIndicator = (char*)endurl;

		return rootIndicator;
	}

	//return E_INVALID_PARAM;	/* Invalid Parameter */
	return NULL;
}

/**
	@brief ¹Ù·Î »óÀ§ÀÇ Æú´õ À§Ä¡¸¦ Ã£¾ÆÁÜ
	@param endurl Ã£À» url
	@param topurl root Æú´õ
	@return »óÀ§ Æú´õ À§Ä¡
*/
char* util_getlastdir(const char* endurl, const char* topurl)
{
	while( endurl >= topurl )
	{
		if(*endurl=='/' )
			return (char*)endurl;

		endurl--;
	}
	return NULL;
}

/**
	@brief URL¿¡¼­ ºÒÇÊ¿äÇÑ "./", "../"À» Á¦°Å
	@param tarurl "./", ".." À» Æ÷ÇÔÇÏ´Â ¹®ÀÚ¿­, ¹İµå½Ã ÇÁ·Î?¸ôÀ» Æ÷ÇÔÇØ¾ß ÇÔ.
	@param topdir tarurlÀÇ ÃÖ»óÀ§ µğ·ºÅä¸®
	@param endurl
	@param returl tarurlÀ» ÇÔ¼ö Ã³¸®ÇÑ °á°ú
	@return returl ±æÀÌ
*/
short util_rearrangeurl( const char* tarurl, const char* topdir, const char* endurl, char* returl )
{
	char* start = returl;
	char* returltop;

	strncpy( returl, tarurl, topdir-tarurl ); /* Root µğ·ºÅä¸®±îÁö´Â º¹»ç */
	returl+=topdir-tarurl;
	returltop = returl;
	tarurl=topdir;

	while ( tarurl<endurl )
	{
		/* "./"¿Í "../"ÀÇ Ã³¸® */
		if ( tarurl[0]=='.' )
		{
			if ( tarurl+1<endurl && tarurl[1]=='/' )
			{
				tarurl+=2;
				continue;
			}
			else if ( tarurl+2<endurl && tarurl[1]=='.' && tarurl[2]=='/' )
			{
				char *lasturl = util_getlastdir(tarurl-2, topdir);
				if( lasturl != NULL )
				{
					lasturl = util_getlastdir(returl-2, returltop); // ÇöÀç À§Ä¡¿¡¼­ »óÀ§ µğ·ºÅä¸®¸¦ Ã£À½, tarurlÀÇ º¹»çº»ÀÌ¹Ç·Î tarurl¿¡ Á¸ÀçÇß´Ù¸é ¾øÀ» ¼ö ¾øÀ½
					if (lasturl!=NULL)
					{
						returl=lasturl+1; /* »óÀ§ µğ·ºÅä¸®·Î µ¹¾Æ°¨ */
						tarurl+=3; /* "../"¸¸Å­ ÀÌµ¿ */
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

/**
	@brief	get_valparsing ¼³¸í
			return °ª¿¡ ±æÀÌ °ª ¸¸Å­ »©¼­ value¸¦ ¾òÀ» ¼ö ÀÖ´Ù.
			value´Â ['/"/(empty)]value['/"/(empty)] ÇüÅÂÀÇ ¹®ÀÚ¿­ÀÌ´Ù.
	@param	sc1
	@param	endPtr
	@param	len ÆĞÅ¶ÀÇ ±æÀÌ °ª
	@return	ÆÄ½Ì ÈÄ ´ÙÀ½ Æ÷ÀÎÅÍ °ª
	@return	E_ENDOFDATA util_passinvalidchar(sc1, endPtr) == NULL
	@return	E_INVALID_PARAM *sc1 == '='
	@see	E_ENDOFDATA
*/
char* util_valparsing( char* sc1, char* endPtr, int* len )
{
	char* ptrValueStart;
	char* ptrValueEnd;
	char cValueTermCodeType = ' ';

	if ( (sc1 = util_passinvalidchar( (unsigned char*)sc1, (unsigned char*)endPtr )) == NULL )	/* Ã£Áö ¸øÇÔ */
		return NULL;

	if ( *sc1 == '=' )
	{
		++sc1;

		if ( ( sc1 = util_passinvalidchar( (unsigned char*)sc1, (unsigned char*)endPtr ) ) == NULL )	/* Ã£Áö ¸øÇÔ */
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

/**
	@brief	util_timevalsubop ¼³¸í
	@param	timeval1
	@param	timeval2
	@return ¼³¸í 

*/
long long util_timevalsubop( struct timeval timeval1, struct timeval timeval2 )
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

/**
	@brief	util_makehashcode
	@param	pszURLBuf
	@param	dLength
	@param	maxHashLen MAX_HASHCODE_LEN
*/
void util_makehashcode(const char *pszURLBuf, char *pszHash, int dLength, int maxHashLen)
{
	int         		j, dIndex;
	unsigned char       szResultHash[8] = {0,};

	//memset( pszHash, 0, MAX_HASHCODE_LEN );
	memset( pszHash, 0, maxHashLen);
	

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


	//for( j=0; j<MAX_HASHCODE_LEN; j++)
	for(j = 0 ; j < maxHashLen ; j++)
	{
		if ( pszHash[j]>0x60 )
			pszHash[j]-=0x20;
	}
}

/**
	@brief	¹®ÀÚ¿­ÀÇ ¿ŞÂÊ¿¡ Á¸ÀçÇÏ´Â Æ¯Á¤ ¹®ÀÚ¸¦ Á¦°Å
	@param	szSrc ½ºÄµÇÒ ¹®ÀÚ¿­
	@param	szSkipChar Á¦°ÅÇÒ ¹®ÀÚ
	@return	szSrcÀÇ ¿ŞÂÊ¿¡¼­ szSkipChar À» Á¦°ÅÇÑ ¹®ÀÚ¿­ Æ÷ÀÎÅÍ
*/
char* util_trimleft( char* szSrc, char* szSkipChar )
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

/**	
	@brief	½Ã°£±¸Á¶Ã¼ time_t¸¦ ¹®ÀÚ¿­·Î Ãâ·ÂÇÏ´Â ÇÔ¼ö
	@param	time_t ½Ã°£
	@param	¹®ÀÚ¿­ ½Ã°£À» ÀúÀåÇÒ char Æ÷ÀÎÅÍ
	@return	¹®ÀÚ¿­ ½Ã°£ÀÌ ÀúÀåµÈ char Æ÷ÀÎÅÍ
*/
char *util_printtime(time_t tTime, char *szTime)
{
	struct tm       stTime;

	localtime_r(&tTime, &stTime);

	sprintf(szTime, "%04d%02d%02d%02d%02d%02d",
			stTime.tm_year+1900, stTime.tm_mon+1, stTime.tm_mday, stTime.tm_hour, stTime.tm_min, stTime.tm_sec);

	return szTime;
}












