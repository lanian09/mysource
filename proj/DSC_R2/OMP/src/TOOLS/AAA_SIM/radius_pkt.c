#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <sys/msg.h>
#include <errno.h>

#include <md5global.h>  
#include <md5.h>


#include "radius_define.h"


ULONG CVT_ULONG( ULONG value );
LONG CVT_LONG( LONG value );
INT CVT_INT( INT value );
UINT CVT_UINT( UINT value );
SHORT CVT_SHORT( SHORT value );
USHORT CVT_USHORT( USHORT value );
INT64 CVT_INT64( INT64 value );



extern int errno;

INT64 convert_INT64(INT64 value );


int ParsingRadius( UCHAR *pszBuf, INT dBufSize, unsigned long ulFromIP, pst_ACCInfo pstRDPkt , int *dADR )
{
	int 	dType, dVSType, dValLen; 
	UCHAR	ucTemp;
	short   sTemp;
	UCHAR	szAttrVal[256];
	int 	dIdx=0;
	int		dTempSize=0;
	int 	dTempOrgSize=0;
	int		dRet;
	int     i;
	int    	dTempVal=0;
	int		dMandCnt=0;
	int    	dBothChk=0;
	int		dBothStopChk=0;
	int		dPDSNChk=0;
	int		dPDSNStart=0;
	int		dPDSNStop=0;
	int 	dWSTypeChk=0;
	

	pst_RADIUS_HEAD  pstRds = (st_RADIUS_HEAD*)&pszBuf[0];

	memset( pstRDPkt, 0x00, sizeof(st_ACCInfo) );

	//pstRds->usLength = CVT_USHORT(pstRds->usLength);

	dTempOrgSize = pstRds->usLength - sizeof(st_RADIUS_HEAD);
	dTempSize = dTempOrgSize;

#if 0
	printf(" Code [%d] ID [%d] Length [%d]\n", 
		pstRds->ucCode, pstRds->ucID, pstRds->usLength );
#endif

	pstRDPkt->ucCode  = pstRds->ucCode;
	pstRDPkt->ucID    = pstRds->ucID;
    memcpy( &pstRDPkt->szAuthen[0], &pstRds->szAuth[0], 16 );


	if( pstRds->usLength != dBufSize )
	{
		fprintf(stdout, "============================== 1 pstRds->usLength[%d] dBufSize[%d]\n",
				pstRds->usLength, dBufSize);
		*dADR = 0;
		return -1;
	}

	dIdx += sizeof(st_RADIUS_HEAD) ;

	while(dTempSize > 4)
	{
		dType = 0;
		dVSType = 0;

		dRet = ParsingAttr( &dType, &dVSType, &szAttrVal[0], &dValLen, &pszBuf[dIdx], &dTempSize ); 
		if( dRet < 0 )
		{
			break;
		}

		dIdx += dTempSize;
		dTempOrgSize -= dTempSize;
		dTempSize = dTempOrgSize;


		switch( dType )
		{
		case 26:
			switch( dVSType )
			{
            case 201:
                if( dValLen == sizeof(INT) )
                {
                    memcpy( &pstRDPkt->uiUDRSeq, &szAttrVal[0], dValLen );
                    pstRDPkt->uiUDRSeq = CVT_INT(pstRDPkt->uiUDRSeq) ;
                    pstRDPkt->ucUDRSeqF = 0x01;
				
					return 0;

                }
                else
                    return -1;
            default:
                break;
            }

        default:
            break;
        }
    }
	
	if( dIdx != dBufSize || dTempSize != 0 )
	{
		fprintf(stderr,"INVALID_PARAMETER_VALUE");
		return -1;
	}

		 
	return dRet;		
}


int ParsingAttr( INT *dType, INT *dVSType, UCHAR *szAttrVal, INT *dValLen, UCHAR *pszBuf, INT *dBufSize )
{
	
	INT		Length;
	UCHAR	ucLen, ucVSLen;
	UCHAR	ucType, ucVSType;
	UCHAR	Value[128];
	INT		dValue;
	int 	dIdx=0;
	UINT	dVenderID;

	if( *dBufSize <= 0 )
		return -1;

	ucType = pszBuf[dIdx];
	dIdx += 1;
	*dType = ucType;

	ucLen = pszBuf[dIdx];
	dIdx += 1;

	Length = ucLen;
	Length -= 2;

	if( ucType == 26  )
	{
		/*      Vender Specific Type 		*/
		memcpy( &dVenderID, &pszBuf[dIdx], 4 );
		dIdx += 4;

		ucVSType = pszBuf[dIdx];
		dIdx += 1;

		*dVSType = ucVSType;

		ucVSLen = pszBuf[dIdx];
		dIdx += 1;

		Length = ucVSLen;
		Length -= 2;	

		memcpy( &szAttrVal[0], &pszBuf[dIdx], Length );
		szAttrVal[Length] = 0x00;
		dIdx += Length;
		*dValLen = Length;

		 		
	}
	else
	{
		/*   Normal Value Attribute 		*/
		memcpy( &szAttrVal[0], &pszBuf[dIdx], Length );
		szAttrVal[Length] = 0x00;
		dIdx += Length;
		*dValLen = Length;
	}	

	*dBufSize = dIdx;


	return 0; 

} 



int dCreateAuthMD5( unsigned char *szAuth, char *szSecret, int RcvLen, unsigned char *szBufMsg )
{
    int  slen = 0;
    unsigned char digest    [16];
    unsigned char szTempMsg [2048];
    MD5_CTX pms2;

    memcpy(&szTempMsg[slen], szBufMsg, RcvLen );
    slen += RcvLen;

    memcpy(&szTempMsg[slen], szSecret, strlen(szSecret) );
    slen += strlen(szSecret);

    MD5Init(&pms2);
    MD5Update( &pms2, (unsigned char*)&szTempMsg[0], slen );
    MD5Final( &digest[0] , &pms2 );

    memcpy( &szAuth[0], &digest[0], 16 );

    return 0;
}



INT64 convert_INT64( INT64 value)
{
    int i;
    INT64 xValue;

    char tszValue[8];
    char tszValue2[8];

    memcpy( tszValue, &value, sizeof(INT64) );

    for( i=0; i< 8; i++)
    {
        tszValue2[i] = tszValue[7-i];
    }

    memcpy( &xValue, &tszValue2[0], sizeof(INT64));


    return xValue;
}




SHORT CVT_SHORT( SHORT value )
{

    int i;
    int dValSize=2;

	union {
		INT xValue;
		char ml[2];
	} u1, u2;

	u1.xValue = value;

	for( i=0; i < dValSize ; i++ )
	{
		u2.ml[i] = u1.ml[dValSize-i-1];		
	}

    return u2.xValue;
}

USHORT CVT_USHORT( USHORT value )
{
    int i;
    int dValSize=2;

	union {
		INT xValue;
		char ml[2];
	} u1, u2;

	u1.xValue = value;

	for( i=0; i < dValSize ; i++ )
	{
		u2.ml[i] = u1.ml[dValSize-i-1];		
	}


    return u2.xValue;
}


INT CVT_INT( INT value )
{
	int i;
	int dValSize = 4;

	union {
		INT xValue;
		char ml[4];
	} u1, u2;

	u1.xValue = value;

	for( i=0; i < dValSize ; i++ )
	{
		u2.ml[i] = u1.ml[dValSize-i-1];		
	}
		
    return u2.xValue;
}


UINT CVT_UINT( UINT value )
{
	int i;
	int dValSize = 4;

	union {
		UINT xValue;
		char ml[4];
	} u1, u2;

	u1.xValue = value;

	for( i=0; i < dValSize ; i++ )
	{
		u2.ml[i] = u1.ml[dValSize-i-1];		
	}

    return u2.xValue;
}

LONG CVT_LONG( LONG value )
{
	int i;
	int dValSize = 8;

	union {
		UINT xValue;
		char ml[8];
	} u1, u2;

	u1.xValue = value;

	for( i=0; i < dValSize ; i++ )
	{
		u2.ml[i] = u1.ml[dValSize-i-1];		
	}

    return u2.xValue;
}

ULONG CVT_ULONG( ULONG value )
{
	int i;
	int dValSize = 8;

	union {
		ULONG xValue;
		char ml[8];
	} u1, u2;

	u1.xValue = value;

	for( i=0; i < dValSize ; i++ )
	{
		u2.ml[i] = u1.ml[dValSize-i-1];		
	}

    return u2.xValue;
}

INT64 CVT_INT64( INT64 value )
{
	int i;
	int dValSize = 8;

	union {
		INT64 xValue;
		char ml[8];
	} u1, u2;

	u1.xValue = value;

	for( i=0; i < dValSize ; i++ )
	{
		u2.ml[i] = u1.ml[dValSize-i-1];		
	}

    return u2.xValue;
}



