/**	@file	cifo_conf.c
 *	- CONFIG READ function
 *	
 *	$Id: cifo_conf.c,v 1.1.1.1 2011/08/19 00:53:28 uamyd Exp $
 *	
 *	Copyright (c) 2006~ by Upresto Inc, Korea
 *	All rights reserved.	
 *	
 *	@Author		$Author: uamyd $
 *	@version	$Revision: 1.1.1.1 $
 *	@date		$Date: 2011/08/19 00:53:28 $
 *	@ref		cifo_conf.c
 *	@todo		
 *	@section	Intro
 *	- CONFIG READ function
 *	
 *	@section	Requirement
 *	- test
 *
 **/

#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "cifo.h"

S32 cifo_chan_init(st_CIFOCONF *stCIFOCONF, S8* pTok)
{
	S32 dTokSize = 0, i, dRet;
	U32	uiChID, uiCellCnt, uiCellSize, uiWBuffCnt, uiRBuffCnt;
	U32 uiWSemFlag, uiWSemKey, uiRSemFlag, uiRSemKey;

	if(stCIFOCONF->uiChCnt > MAX_CHAN_CNT)
	{
		errorPrint(DEF_MAX_OVERFLOW, DEBUG_CHANNEL_CONF, "channel", stCIFOCONF->uiChCnt+1);
		return DEF_MAX_OVERFLOW;
	}

	/* chan conf setting */
	while((pTok = trim(pTok)) != NULL)
	{
		if(pTok[0] == '#' || pTok[0] == '\0')
			break;
		switch(dTokSize)
		{
			case 0:		/* channel id */
				/* valid check */
				if( (dRet = defaultCheck(pTok, &uiChID, DEF_CHECK_FLAG_ZERO)) < 0)
				{
					errorPrint(dRet, DEBUG_CHANNEL_CONF, "uiChID", stCIFOCONF->uiChCnt+1);
					return dRet;
				}
				if(stCIFOCONF->uiChCnt != uiChID)
				{
					errorPrint(DEF_ERR_EXIST, DEBUG_CHANNEL_CONF, "uiChID", stCIFOCONF->uiChCnt+1);
					return DEF_ERR_EXIST;
				}
				break;
			case 1: /* cell count */
				if( (dRet = defaultCheck(pTok, &uiCellCnt, DEF_CHECK_FLAG_MORE)) < 0)
				{
					errorPrint(dRet, DEBUG_CHANNEL_CONF, "uiCellCnt", stCIFOCONF->uiChCnt+1);
					return dRet;
				}
				break;
			case 2: /* cell size */
				if( (dRet = defaultCheck(pTok, &uiCellSize, DEF_CHECK_FLAG_MORE)) < 0)
				{
					errorPrint(dRet, DEBUG_CHANNEL_CONF, "uiCellSize", stCIFOCONF->uiChCnt+1);
					return dRet;
				}
				break;
			case 3: /* write buff cnt */
				if( (dRet = defaultCheck(pTok, &uiWBuffCnt, DEF_CHECK_FLAG_MORE)) < 0)
				{
					errorPrint(dRet, DEBUG_CHANNEL_CONF, "uiWBuffCnt", stCIFOCONF->uiChCnt+1);
					return dRet;
				}
				break;
			case 4: /* read buff cnt */
				if( (dRet = defaultCheck(pTok, &uiRBuffCnt, DEF_CHECK_FLAG_MORE)) < 0)
				{
					errorPrint(dRet, DEBUG_CHANNEL_CONF, "uiRBuffCnt", stCIFOCONF->uiChCnt+1);
					return dRet;
				}
				break;
			case 5: /* write sema flag */
				if( (dRet = defaultCheck(pTok, &uiWSemFlag, DEF_CHECK_FLAG_BOOL)) < 0)
				{
					errorPrint(dRet, DEBUG_CHANNEL_CONF, "uiWSemFlag", stCIFOCONF->uiChCnt+1);
					return dRet;
				}
				break;
			case 6: /* write sema key */
				if( (dRet = defaultCheck(pTok, &uiWSemKey, DEF_CHECK_FLAG_MORE)) < 0)
				{
					errorPrint(dRet, DEBUG_CHANNEL_CONF, "uiWSemKey", stCIFOCONF->uiChCnt+1);
					return dRet;
				}
				if(uiWSemFlag == MEMS_SEMA_ON)
				{
					for(i=0;i<stCIFOCONF->uiChCnt;i++)
					{
						st_CHANCONF stTemp = stCIFOCONF->stCHANCONF[i];
						if( (stTemp.uiWSemFlag == MEMS_SEMA_ON && stTemp.uiRSemKey == uiWSemKey) 
								|| (stTemp.uiRSemFlag == MEMS_SEMA_ON && stTemp.uiWSemKey == uiWSemKey))
						{
							errorPrint(DEF_ERR_EXIST, DEBUG_CHANNEL_CONF, "uiWSemKey", stCIFOCONF->uiChCnt+1);
							return DEF_ERR_EXIST;
						}
					}
				}
				break;
			case 7: /* read sema flag */
				if( (dRet = defaultCheck(pTok, &uiRSemFlag, DEF_CHECK_FLAG_BOOL)) < 0)
				{
					errorPrint(dRet, DEBUG_CHANNEL_CONF, "uiRSemFlag", stCIFOCONF->uiChCnt+1);
					return dRet;
				}
				break;
			case 8: /* read sema key */
				if( (dRet = defaultCheck(pTok, &uiRSemKey, DEF_CHECK_FLAG_MORE)) < 0)
				{
					errorPrint(dRet, DEBUG_CHANNEL_CONF, "uiRSemKey", stCIFOCONF->uiChCnt+1);
					return dRet;
				}
				if(uiRSemFlag == MEMS_SEMA_ON)
				{
					for(i=0;i<stCIFOCONF->uiChCnt;i++)
					{
						st_CHANCONF stTemp = stCIFOCONF->stCHANCONF[i];
						if( (stTemp.uiWSemFlag == MEMS_SEMA_ON && stTemp.uiRSemKey == uiRSemKey)
								|| (stTemp.uiRSemFlag == MEMS_SEMA_ON && stTemp.uiWSemKey == uiRSemKey))
						{
							errorPrint(DEF_ERR_EXIST, DEBUG_CHANNEL_CONF, "uiRSemKey", stCIFOCONF->uiChCnt+1);
							return DEF_ERR_EXIST;
						}
					}
				}
				break;
			default: /* error */
				errorPrint(DEF_ERR_FORM, DEBUG_CHANNEL_CONF, "all", stCIFOCONF->uiChCnt+1);
				return DEF_ERR_FORM;
		}
		dTokSize++;
		pTok = strtok(NULL, "/");
	}
	if(dTokSize != DEF_CHANCONF_MEMBERS)
	{
		errorPrint(DEF_ERR_MEMBERS, DEBUG_CHANNEL_CONF, "all", stCIFOCONF->uiChCnt+1);
		return DEF_ERR_MEMBERS;
	}

	stCIFOCONF->stCHANCONF[stCIFOCONF->uiChCnt].uiChID = uiChID;
	stCIFOCONF->stCHANCONF[stCIFOCONF->uiChCnt].uiCellCnt = uiCellCnt;
	stCIFOCONF->stCHANCONF[stCIFOCONF->uiChCnt].uiCellSize = uiCellSize;
	stCIFOCONF->stCHANCONF[stCIFOCONF->uiChCnt].uiWBuffCnt = uiWBuffCnt;
	stCIFOCONF->stCHANCONF[stCIFOCONF->uiChCnt].uiRBuffCnt = uiRBuffCnt;
	stCIFOCONF->stCHANCONF[stCIFOCONF->uiChCnt].uiWSemFlag = uiWSemFlag;
	stCIFOCONF->stCHANCONF[stCIFOCONF->uiChCnt].uiWSemKey = uiWSemKey;
	stCIFOCONF->stCHANCONF[stCIFOCONF->uiChCnt].uiRSemFlag = uiRSemFlag;
	stCIFOCONF->stCHANCONF[stCIFOCONF->uiChCnt].uiRSemKey = uiRSemKey;
	
	stCIFOCONF->uiChCnt++;
	return 1;
}


S32 cifo_conf_init(st_CIFOCONF *pCIFOCONF, S8 *confFile)
{
	S8       		buf[BUFSIZ], szType[64], szTmp[64], szInfo[64];
	FILE        	*fp;
	S8       		*pTok;
	S32				dRet, dCnt = 0;
	
	pCIFOCONF->uiChCnt = 0;
	
	if( (fp = fopen( confFile, "r" )) == NULL )
	{
		FPRINTF( LOG_BUG, "file open failed cifo.conf \n");
		return -1;
	}
	while( fgets( buf, BUFSIZ, fp ))
	{
		if(buf[0] == '#' || buf[0] == '\r' || buf[0] == '\n')
			continue;
		dCnt++;
		/* 1. cifo config setting */
		if(dCnt <= 2)
		{
			if(sscanf(buf, "%s %s %s", szType, szTmp, szInfo) == 3)
			{   
				if(strcmp(szType, "uiShmKey") == 0)
				{   
					if( (dRet = defaultCheck(szInfo, &pCIFOCONF->uiShmKey, DEF_CHECK_FLAG_ZERO)) < 0)
					{   
						errorPrint(dRet, DEBUG_CIFO_CONF, "uiShmKey", 1);
						return dRet;
					}
					continue;
				}
				if(strcmp(szType, "uiHeadRoomSize") == 0)
				{
					if( (dRet = defaultCheck(szInfo, &pCIFOCONF->uiHeadRoomSize, DEF_CHECK_FLAG_ZERO)) < 0)
					{
						errorPrint(dRet, DEBUG_CIFO_CONF, "uiHeadRoomSize", 2);
						return dRet;
					}
					continue;
				}
			}
			errorPrint(DEF_ERR_FORM, DEBUG_CIFO_CONF, "CIFO CONF", dCnt);
			return -1;
		}

		/* 2. channel config setting */
		pTok = strtok(buf, "/");
		if( (dRet = cifo_chan_init(pCIFOCONF, pTok)) < 0 )
		{
			return dRet;
		}
	}
	return 1;
}

/*
 *     $Log: cifo_conf.c,v $
 *     Revision 1.1.1.1  2011/08/19 00:53:28  uamyd
 *     upst library
 *
 *     Revision 1.1  2011/07/26 04:52:23  dhkim
 *     *** empty log message ***
 *
 *     Revision 1.1.1.1  2011/01/11 01:33:02  jjinri
 *     DIFO
 *
 *     Revision 1.6  2011/01/05 08:57:28  swpark
 *     trim add
 *
 *     Revision 1.5  2010/12/29 02:53:40  swpark
 *     config file name get param
 *
 *     Revision 1.4  2010/12/10 08:14:35  dark264sh
 *     *** empty log message ***
 *
 *     Revision 1.3  2010/12/10 07:23:47  swpark
 *     uiChCnt init add
 *
 *     Revision 1.2  2010/12/10 05:37:39  swpark
 *     check cifo_conf.c
 *
 *     Revision 1.1  2010/12/09 09:25:58  swpark
 *     cifo config read func
 *
 *
 *
 */
