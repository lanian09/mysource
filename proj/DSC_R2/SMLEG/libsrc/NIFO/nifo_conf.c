/**	@file	nifo_conf.c
 *	- CONFIG READ function
 *	
 *	$Id: nifo_conf.c,v 1.1 2011/04/26 09:08:46 jjinri Exp $
 *	
 *	Copyright (c) 2006~ by Upresto Inc, Korea
 *	All rights reserved.	
 *	
 *	@Author		$Author: jjinri $
 *	@version	$Revision: 1.1 $
 *	@date		$Date: 2011/04/26 09:08:46 $
 *	@ref		nifo_conf.c
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
#include "nifo.h"

S32 isNumber(S8 *s)
{
	S32 i, len = strlen(s);
	for(i=0;i<len;i++)
	{
		if(!isdigit(*(s+i)))
			return 0;
	}
	return 1;
}

S8 *ltrim(S8 *s)
{
	S8 *begin;
	if(s != NULL)
	{
		begin = s;
		while (*begin != '\0')
		{
			if(isspace(*begin))
				begin++;
			else{
				break;
			}
		}
		s=begin;
	}
	return s;
}
S8* rtrim(S8* s)
{
	S8 *end;
	if(s != NULL)
	{
		end = s + strlen(s) - 1;
		while (end != s && isspace(*end))
			end--;
		*(end + 1) = '\0';
	}

	return s;
}

S8* trim(S8 *s)
{
	return ltrim(rtrim(s));
}

S32 defaultCheck(S8 *s, U32 *uiValue, S32 dFlag)
{
	S32 uiTemp;
	if(!isNumber(s))
	{
		return DEF_ERR_NOTNUM;
	}
	uiTemp = atoi(s);
	switch(dFlag)
	{
		case DEF_CHECK_FLAG_ZERO:
			if(uiTemp < 0)
			{
				return DEF_ERR_LOWZERO;
			}
			break;
		case DEF_CHECK_FLAG_MORE:
			if(uiTemp <= 0)
			{
				return DEF_ERR_LOWONE;
			}
			break;
		case DEF_CHECK_FLAG_BOOL:
			if(uiTemp != 0 && uiTemp != 1)
			{
				return DEF_ERR_NOTBOOL;
			}
			break;
		default:
			return DEF_ERR_NOTFLAG;
	}
	sscanf(s, "%u", uiValue);
	return 1;
}

void errorPrint(S32 errn, S8* grp, S8 *var, U32 line)
{
	FPRINTF(LOG_BUG, "%s: %d line %s ", grp, line, var);
	switch(errn)
	{
		case DEF_ERR_NOTNUM:
			FPRINTF(LOG_BUG, "is not number ");
			break;
		case DEF_ERR_LOWZERO:
			FPRINTF(LOG_BUG, "must be more than 0 or 0 ");
			break;
		case DEF_ERR_LOWONE:
			FPRINTF(LOG_BUG, "must be more than 0 ");
			break;
		case DEF_ERR_NOTBOOL:
			FPRINTF(LOG_BUG, "must be 0 or 1 ");
			break;
		case DEF_ERR_NOTFLAG:
			FPRINTF(LOG_BUG, "check flag error(program) ");
			break;
		case DEF_ERR_EXIST:
			FPRINTF(LOG_BUG, "is exist other  ");
			break;
		case DEF_ERR_FORM:
			FPRINTF(LOG_BUG, "form error  ");
			break;
		case DEF_ERR_MEMBERS:
			FPRINTF(LOG_BUG, "uncorrect members  ");
			break;
		case DEF_ERR_NOT_EXIST:
			FPRINTF(LOG_BUG, "is not exist top config  ");
			break;
		case DEF_MAX_OVERFLOW:
			FPRINTF(LOG_BUG, "is max over flow  ");
			break;
		case DEF_ERR_ORDER:
			FPRINTF(LOG_BUG, "ordered fail ");
			break;
		default:
			FPRINTF(LOG_BUG, "unknown error ");
			break;
	}
}

S32 zone_conf_init(st_MEMSCONF *pMEMSCONF, S8* pTok)
{
	S32 dTokSize = 0, i, dRet;
	U32 uiZoneID, uiSemFlag, uiSemKey, uiMemNodeBodySize, uiMemNodeTotCnt;

	st_MEMSZONECONF *stZone = &pMEMSCONF->stMEMSZONECONF[pMEMSCONF->uiZoneCnt];

	if(pMEMSCONF->uiZoneCnt > MAX_MEMSZONE_CNT)
	{
		errorPrint(DEF_MAX_OVERFLOW, DEBUG_ZONE_CONF, "ZONE", pMEMSCONF->uiZoneCnt+1);
		return DEF_MAX_OVERFLOW;
	}

	/* ZONE conf setting */
	while((pTok = trim(pTok)) != NULL)
	{   
		if(pTok[0] == '#' || pTok[0] == '\0')
			break;
		switch(dTokSize)
		{
			case 0:		/* zone id */
				if( (dRet = defaultCheck(pTok, &uiZoneID, DEF_CHECK_FLAG_ZERO)) < 0)
				{
					errorPrint(dRet, DEBUG_ZONE_CONF, "uiZoneID", pMEMSCONF->uiZoneCnt+1);
					return dRet;
				}
				if(pMEMSCONF->uiZoneCnt != uiZoneID)
				{
					errorPrint(DEF_ERR_ORDER, DEBUG_ZONE_CONF, "uiZoneID", pMEMSCONF->uiZoneCnt+1);
					return DEF_ERR_ORDER;
				}
				break;
			case 1:		/* Semaphore flag */
				if( (dRet = defaultCheck(pTok, &uiSemFlag, DEF_CHECK_FLAG_BOOL)) < 0)
				{
					errorPrint(dRet, DEBUG_ZONE_CONF, "uiSemFlag", pMEMSCONF->uiZoneCnt+1);
					return dRet;
				}
				break;
			case 2:		/* Semaphore key */
				if( (dRet = defaultCheck(pTok, &uiSemKey, DEF_CHECK_FLAG_MORE)) < 0)
				{
					errorPrint(dRet, DEBUG_ZONE_CONF, "uiSemKey", pMEMSCONF->uiZoneCnt+1);
					return dRet;
				}
				if(uiSemFlag == MEMS_SEMA_ON)	/* duplicate check */
				{
					for(i=0;i<pMEMSCONF->uiZoneCnt;i++)
					{
						if( (pMEMSCONF->stMEMSZONECONF[i].uiSemFlag == MEMS_SEMA_ON) && (pMEMSCONF->stMEMSZONECONF[i].uiSemKey == uiSemKey))
						{
							errorPrint(DEF_ERR_EXIST, DEBUG_ZONE_CONF, "uiSemKey", pMEMSCONF->uiZoneCnt+1);
							return DEF_ERR_EXIST;
						}
					}
				}
				break;
			case 3:		/* memory node body size */
				if( (dRet = defaultCheck(pTok, &uiMemNodeBodySize, DEF_CHECK_FLAG_ZERO)) < 0)
				{
					errorPrint(dRet, DEBUG_ZONE_CONF, "uiMemNodeBodySize", pMEMSCONF->uiZoneCnt+1);
					return dRet;
				}
				break;
			case 4:		/* memory node total cnt */
				if( (dRet = defaultCheck(pTok, &uiMemNodeTotCnt, DEF_CHECK_FLAG_ZERO)) < 0)
				{
					errorPrint(dRet, DEBUG_ZONE_CONF, "uiMemNodeTotCnt;", pMEMSCONF->uiZoneCnt+1);
					return dRet;
				}
				break;
			default: /* error */
				errorPrint(DEF_ERR_FORM, DEBUG_ZONE_CONF, "all", pMEMSCONF->uiZoneCnt+1);
				return DEF_ERR_FORM;
		}
		dTokSize++;
		pTok = strtok(NULL, "/");
	}
	if(dTokSize != DEF_ZONECONF_MEMBERS)
	{
		errorPrint(DEF_ERR_MEMBERS, DEBUG_ZONE_CONF, "all", pMEMSCONF->uiZoneCnt+1);
		return DEF_ERR_MEMBERS;
	}
	stZone->uiZoneID = uiZoneID;
	stZone->uiSemFlag = uiSemFlag;
	stZone->uiSemKey = uiSemKey;
	stZone->uiMemNodeBodySize = uiMemNodeBodySize;
	stZone->uiMemNodeTotCnt = uiMemNodeTotCnt;

	pMEMSCONF->uiZoneCnt++;
	
	return 1;
}

S32 zone_proc_conf(st_MEMSCONF *pMEMSCONF, S8* pTok, U32 uiMatrixSize)
{
	S32 dRet, dTokCnt = 0;
	U32 uiProcID, uiZoneID;

	while((pTok = trim(pTok)) != NULL)
	{           
		if(pTok[0] == '#' || pTok[0] == '\0' || dTokCnt >= 2)
			break;
		if(dTokCnt == 0)
		{
			if( (dRet = defaultCheck(pTok, &uiProcID, DEF_CHECK_FLAG_ZERO)) < 0)
			{
				errorPrint(dRet, DEBUG_NIFO_CONF, "PROCESS ID", uiMatrixSize+1);
				return dRet;
			}
			/* is not exists from matrix */
			if( pMEMSCONF->uiMatrixZoneID[uiProcID] != INVALID_ID)
			{
				errorPrint(DEF_ERR_EXIST, DEBUG_NIFO_CONF, "PROCESS ID", uiMatrixSize+1);
				return DEF_ERR_EXIST;
			}
		}
		else if(dTokCnt == 1)
		{
			/* Zone ID */
			if( (dRet = defaultCheck(pTok, &uiZoneID, DEF_CHECK_FLAG_ZERO)) < 0)
			{
				errorPrint(dRet, DEBUG_NIFO_CONF, "ZONE ID", uiMatrixSize+1);
				return dRet;
			}
			/* is exists from cifoconf */
			if(pMEMSCONF->stMEMSZONECONF[uiZoneID].uiZoneID != uiZoneID)
			{   
				errorPrint(DEF_ERR_NOT_EXIST, DEBUG_NIFO_CONF, "ZONE ID", uiMatrixSize+1);
				return DEF_ERR_NOT_EXIST;
			}
		}
		dTokCnt++;
		pTok = strtok(NULL, "/");
	}
	if(dTokCnt != 2)
	{   
		errorPrint(DEF_ERR_MEMBERS, DEBUG_NIFO_CONF, "all", uiMatrixSize+1);
		return DEF_ERR_MEMBERS;
	}

	if(uiProcID > MAX_SEQ_PROC_NUM)
	{   
		errorPrint(DEF_MAX_OVERFLOW, DEBUG_NIFO_CONF, "PROCESS ID", uiMatrixSize+1);
		return DEF_MAX_OVERFLOW;
	}
	pMEMSCONF->uiMatrixZoneID[uiProcID] = uiZoneID;

	return 1;
}

S32 nifo_zone_conf_init(st_MEMSCONF *pMEMSCONF, S8 *confFile)
{
	S8       		buf[BUFSIZ], szType[64], szTmp[64], szInfo[64];
	FILE        	*fp;
	S8       		*pTok;
	S32				i, dRet, dCnt = 0;
	U32				uiMatrixSize = 0;
	
	pMEMSCONF->uiZoneCnt = 0;

	for(i=0;i<MAX_SEQ_PROC_NUM;i++)
	{
		pMEMSCONF->uiMatrixZoneID[i] = INVALID_ID;
	}

	if( (fp = fopen( confFile, "r" )) == NULL )
	{
		FPRINTF( LOG_BUG, "file open failed nifo_zone.conf \n");
		return -1;
	}
	while( fgets( buf, BUFSIZ, fp ))
	{
		if(buf[0] == '#' || buf[0] == '\r' || buf[0] == '\n')
			continue;
		
		/* 1. nifo config setting */
		if(dCnt < 3 )
		{
			dCnt++;
			if(sscanf(buf, "%s %s %s", szType, szTmp, szInfo) == 3)
			{   
				if(strcmp(szType, "uiType") == 0)
				{   
					if( (dRet = defaultCheck(szInfo, &pMEMSCONF->uiType, DEF_CHECK_FLAG_ZERO)) < 0)
					{   
						errorPrint(dRet, DEBUG_NIFO_CONF, "uiType", 1);
						return dRet;
					}
					continue;
				}
				if(strcmp(szType, "uiShmKey") == 0)
				{   
					if( (dRet = defaultCheck(szInfo, &pMEMSCONF->uiShmKey, DEF_CHECK_FLAG_ZERO)) < 0)
					{   
						errorPrint(dRet, DEBUG_NIFO_CONF, "uiShmKey", 2);
						return dRet;
					}
					continue;
				}
				if(strcmp(szType, "uiHeadRoomSize") == 0)
				{   
					if( (dRet = defaultCheck(szInfo, &pMEMSCONF->uiHeadRoomSize, DEF_CHECK_FLAG_ZERO)) < 0)
					{   
						errorPrint(dRet, DEBUG_NIFO_CONF, "uiHeadRoomSize", 3);
						return dRet;
					}
					continue;
				}
			}
			errorPrint(DEF_ERR_FORM, DEBUG_NIFO_CONF, "NIFO CONF", dCnt);
			return DEF_ERR_FORM;
		}

		if(dCnt == 3)
		{
			/* 2. channel config setting */
			pTok = strtok(buf, "#");
			if(pTok != NULL && strcmp(pTok, "END_ZONE") == 0)
			{
				dCnt++;
				continue;
			}
			
			pTok = strtok(buf, "/");
			if( (dRet = zone_conf_init(pMEMSCONF, pTok)) < 0 )
			{
				return dRet;
			}
		}
		else
		{
			/* 3. zone - proc_ID config setting */
			pTok = strtok(buf, "#");
			if(pTok != NULL && strcmp(pTok, "END_MATRIX") == 0)
			{   
				break;
			}

			pTok = strtok(buf, "/");
			if( (dRet = zone_proc_conf(pMEMSCONF, pTok, uiMatrixSize)) < 0 )
			{   
				return dRet;
			}
			uiMatrixSize++;
		}
	}
	return 1;
}

/*
 *     $Log: nifo_conf.c,v $
 *     Revision 1.1  2011/04/26 09:08:46  jjinri
 *     nifo
 *
 *     Revision 1.3  2011/03/28 06:30:23  jjinri
 *     *** empty log message ***
 *
 *     Revision 1.2  2011/01/11 05:45:25  swpark
 *     config init function config file name parameter add
 *
 *     Revision 1.3  2011/01/06 09:07:10  swpark
 *     config file parameter addINC/cifo.h
 *
 *     Revision 1.2  2010/12/29 06:57:51  swpark
 *     zone matrix add
 *
 *     Revision 1.1  2010/12/23 01:26:22  swpark
 *     nifo zone add
 *
 *     Revision 1.6  2010/12/14 05:46:34  swpark
 *     \n del
 *
 *     Revision 1.5  2010/12/10 08:29:52  swpark
 *     switch break add
 *
 *     Revision 1.4  2010/12/10 07:24:13  swpark
 *     uiZone init add
 *
 *     Revision 1.3  2010/12/10 05:38:33  swpark
 *     check
 *
 *     Revision 1.2  2010/12/10 05:17:15  swpark
 *     func name modify
 *
 *     Revision 1.1  2010/12/10 00:47:02  swpark
 *     nifo_conf.c add
 *
 *
 *
 */
