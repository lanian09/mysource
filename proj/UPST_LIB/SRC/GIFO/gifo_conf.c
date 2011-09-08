/**	@file	gifo_conf.c
 *	- CONFIG READ function
 *	
 *	$Id: gifo_conf.c,v 1.1.1.1 2011/08/19 00:53:28 uamyd Exp $
 *	
 *	Copyright (c) 2006~ by Upresto Inc, Korea
 *	All rights reserved.	
 *	
 *	@Author		$Author: uamyd $
 *	@version	$Revision: 1.1.1.1 $
 *	@date		$Date: 2011/08/19 00:53:28 $
 *	@ref		gifo_conf.c
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
#include "gifo.h"

S32		gifo_group_init(stGIFO *pGIFO, st_CIFOCONF *pCIFOCONF, S8* pTok)
{
	S32 	dRet, i, j;
	U32		uiGrID;
	S8		*pChTok;
	st_GROUP *pGroup = &pGIFO->stGROUP[pGIFO->uiGrCnt];

	/* grID valid check */
	if( (dRet = defaultCheck(pTok, &uiGrID, DEF_CHECK_FLAG_ZERO)) < 0)
	{
		errorPrint(dRet, DEBUG_GROUP_CONF, "uiGrID", pGIFO->uiGrCnt+1);
		return dRet;
	}
	if(pGIFO->uiGrCnt != uiGrID)
	{
		errorPrint(DEF_ERR_EXIST, DEBUG_GROUP_CONF, "uiGrID", pGIFO->uiGrCnt+1);
		return DEF_ERR_EXIST;
	}
	pTok = strtok(NULL, "/");
	/* channel id */
	pChTok = strtok(pTok, ",");
	while((pChTok = trim(pChTok)) != NULL)
	{
		U32 uiChID = 0;
		if(pChTok[0] == '#' || pChTok[0] == '\0')
			break;
		if( (dRet = defaultCheck(pChTok, &uiChID, DEF_CHECK_FLAG_ZERO)) < 0)
		{
			errorPrint(dRet, DEBUG_GROUP_CONF, "uiGrID", pGIFO->uiGrCnt+1);
			return dRet;
		}

		/* valid check - is exist cifoconf, is not exist other group */
		for(i=0;i<pGIFO->uiGrCnt;i++)
		{
			for(j=0;j<pGIFO->stGROUP[i].uiChCnt;j++)
			{
				if(pGIFO->stGROUP[i].uiChID[j] == uiChID)
				{
					errorPrint(DEF_ERR_EXIST, DEBUG_GROUP_CONF, "uiChID", pGIFO->uiGrCnt+1);
					return DEF_ERR_EXIST;
				}
			}
		}
		if(pCIFOCONF->stCHANCONF[uiChID].uiChID != uiChID)
		{
			errorPrint(DEF_ERR_NOT_EXIST, DEBUG_GROUP_CONF, "uiChID", pGIFO->uiGrCnt+1);
			return DEF_ERR_NOT_EXIST;
		}
		if(pGroup->uiChCnt > MAX_CHAN_CNT)
		{
			errorPrint(DEF_MAX_OVERFLOW, DEBUG_GROUP_CONF, "channel id", pGIFO->uiGrCnt+1);
			return DEF_MAX_OVERFLOW;
		}
		pGroup->uiChID[pGroup->uiChCnt] = uiChID;
		pGroup->uiChCnt++;
		pChTok = strtok(NULL, ",");
	}
	if(pGroup->uiChCnt == 0)
	{
		errorPrint(DEF_ERR_LOWONE, DEBUG_GROUP_CONF, "CHID list size", pGIFO->uiGrCnt+1);
		return DEF_ERR_LOWONE;
	}

	pGroup->uiGrID = uiGrID;
	pGIFO->uiGrCnt++;
	return 1;
}

S32 gifo_write_matrix(stGIFO *pGIFO, st_CIFOCONF *pCIFOCONF, S8* pTok, U32 uiMatrixSize)
{
	S32		dRet, dTokCnt = 0;
	U32		uiWSeq, uiRSeq, uiChID;

	while((pTok = trim(pTok)) !=NULL)
	{
		if(pTok[0] == '#' || pTok[0] == '\0' || dTokCnt >= 3)
			break;
		switch(dTokCnt)
		{
			case 0:
				/* write seq */
				if( (dRet = defaultCheck(pTok, &uiWSeq, DEF_CHECK_FLAG_ZERO)) < 0)
				{
					errorPrint(dRet, DEBUG_W_MET_CONF, "Write Seq", uiMatrixSize+1);
					return dRet;
				}
				break;
			case 1:
				/* read seq */
				if( (dRet = defaultCheck(pTok, &uiRSeq, DEF_CHECK_FLAG_ZERO)) < 0)
				{
					errorPrint(dRet, DEBUG_W_MET_CONF, "Read Seq", uiMatrixSize+1);
					return dRet;
				}
				break;
			case 2:
				/* channel id */
				if( (dRet = defaultCheck(pTok, &uiChID, DEF_CHECK_FLAG_ZERO)) < 0)
				{
					errorPrint(dRet, DEBUG_W_MET_CONF, "Channel ID", uiMatrixSize+1);
					return dRet;
				}
				/* is exists from cifoconf */
				if(pCIFOCONF->stCHANCONF[uiChID].uiChID != uiChID)
				{
					errorPrint(DEF_ERR_NOT_EXIST, DEBUG_W_MET_CONF, "uiChID", uiMatrixSize+1);
					return DEF_ERR_NOT_EXIST;
				}
				/* is not exists from matrix */
				/*for(i=0;i<MAX_SEQ_PROC_NUM*MAX_SEQ_PROC_NUM;i++)
				{
					if(pGIFO->uiMatrixChID[i] == uiChID)
					{
						errorPrint(DEF_ERR_EXIST, DEBUG_W_MET_CONF, "uiChID", uiMatrixSize+1);
						return DEF_ERR_EXIST;
					}
				}*/
				break;
		}
		dTokCnt++;
		pTok = strtok(NULL, "/");
	}
	if(dTokCnt != 3)
	{
		errorPrint(DEF_ERR_MEMBERS, DEBUG_W_MET_CONF, "all", uiMatrixSize+1);
		return DEF_ERR_MEMBERS;
	}
	if( (uiWSeq*1000+uiRSeq) > (MAX_SEQ_PROC_NUM*MAX_SEQ_PROC_NUM) )
	{
		errorPrint(DEF_MAX_OVERFLOW, DEBUG_W_MET_CONF, "channel id", uiMatrixSize+1);
		return DEF_MAX_OVERFLOW;
	}

	/* is not exists from matrix */
	if( pGIFO->uiMatrixChID[uiWSeq*1000+uiRSeq] != INVALID_ID)
	{
		errorPrint(DEF_ERR_EXIST, DEBUG_W_MET_CONF, "WRITE SEQ, READ SEQ", uiMatrixSize+1);
		return DEF_ERR_EXIST;
	}

	pGIFO->uiMatrixChID[uiWSeq*1000+uiRSeq] = uiChID;

	return 1;
}


S32	gifo_read_matrix(stGIFO *pGIFO, S8* pTok, U32 uiMatrixSize)
{
	S32     dRet, dTokCnt = 0;
	U32		uiRSeq, uiGrID;

	while((pTok = trim(pTok)) != NULL)
	{
		if(pTok[0] == '#' || pTok[0] == '\0' || dTokCnt >= 2)
			break;
		if(dTokCnt == 0)
		{       
			/* read seq */
			if( (dRet = defaultCheck(pTok, &uiRSeq, DEF_CHECK_FLAG_ZERO)) < 0)
			{   
				errorPrint(dRet, DEBUG_R_MET_CONF, "Read Seq", uiMatrixSize+1);
				return dRet;
			}
			/* is not exists from matrix */
			if( pGIFO->uiMatrixGrID[uiRSeq] != INVALID_ID)
			{
				errorPrint(DEF_ERR_EXIST, DEBUG_R_MET_CONF, "GROUP ID", uiMatrixSize+1);
				return DEF_ERR_EXIST;
			}
		}
		else if(dTokCnt == 1)
		{
			/* group id */
			if( (dRet = defaultCheck(pTok, &uiGrID, DEF_CHECK_FLAG_ZERO)) < 0)
			{
				errorPrint(dRet, DEBUG_R_MET_CONF, "GROUP ID", uiMatrixSize+1);
				return dRet;
			}
			/* is exists from cifoconf */
			if(pGIFO->stGROUP[uiGrID].uiGrID != uiGrID)
			{
				errorPrint(DEF_ERR_NOT_EXIST, DEBUG_R_MET_CONF, "GROUP ID", uiMatrixSize+1);
				return DEF_ERR_NOT_EXIST;
			}
		}
		dTokCnt++;
		pTok = strtok(NULL, "/");
	}
	if(dTokCnt != 2)
	{
		errorPrint(DEF_ERR_MEMBERS, DEBUG_R_MET_CONF, "all", uiMatrixSize+1);
		return DEF_ERR_MEMBERS;
	}
	
	if(uiRSeq > MAX_SEQ_PROC_NUM)
	{
		errorPrint(DEF_MAX_OVERFLOW, DEBUG_R_MET_CONF, "READ SEQ", uiMatrixSize+1);
		return DEF_MAX_OVERFLOW;
	}
	pGIFO->uiMatrixGrID[uiRSeq] = uiGrID;
	
	return 1;
}

S32 gifo_conf_init(stGIFO *pGIFO, st_CIFOCONF *pCIFOCONF, S8 *confFile)
{
	S8				buf[BUFSIZ];
	FILE			*fp;
	S8				*pTok;
	S32				i, dRet, dCurSet = 0;
	U32				uiMatrixSize = 0;
	/* init stGIFO*/
	pGIFO->uiGrCnt = 0;
	for(i=0;i<MAX_SEQ_PROC_NUM*MAX_SEQ_PROC_NUM;i++)
	{
		pGIFO->uiMatrixChID[i] = INVALID_ID;
	}
	for(i=0;i<MAX_SEQ_PROC_NUM;i++)
	{
		pGIFO->uiMatrixGrID[i] = INVALID_ID;
	}
	/* init stGIFO end */
	
	if( (fp = fopen( confFile, "r" )) == NULL )
	{
		FPRINTF( LOG_BUG, "file open failed cifo.conf \n");
		return -1;
	}
	while( fgets( buf, BUFSIZ, fp ))
	{
		if(buf[0] == '#' || buf[0] == '\r' || buf[0] == '\n')
			continue;
		switch(dCurSet)
		{
			case 0:
				/* 1. group setting */
				/* groupId / channel ID(, channel ID) */
				pTok = strtok(buf, "#");
				if(pTok != NULL && strcmp(pTok, "END_GROUP") == 0)
				{
					dCurSet = 1;
					continue;
				}
				pTok = strtok(buf, "/");
				if( (dRet = gifo_group_init(pGIFO, pCIFOCONF, pTok)) < 0)
				{
					return dRet;
				}
				break;
			case 1:
				/* 2. write matrix setting */
				/* wProcSeq/rProcSeq/chid/ */
				pTok = strtok(buf, "#");
				if(pTok != NULL && strcmp(pTok, "END_WRITE_MATRIX") == 0)
				{
					dCurSet = 2;
					uiMatrixSize = 0;
					continue;
				}
				pTok = strtok(buf, "/");
				if( (dRet = gifo_write_matrix(pGIFO, pCIFOCONF, pTok, uiMatrixSize)) < 0)
				{
					return dRet;
				}
				uiMatrixSize++;
				break;
			case 2:
				/* 3. read matrix setting */
				/* rProcSeq/grId */
				pTok = strtok(buf, "#");
				if(pTok != NULL && strcmp(pTok, "END_READ_MATRIX") == 0)
				{
					dCurSet = 3;
					break;
				}
				pTok = strtok(buf, "/");
				if( (dRet = gifo_read_matrix(pGIFO, pTok, uiMatrixSize)) < 0)
				{
					return dRet;
				}   
				uiMatrixSize++;
				break;
			default:
				FPRINTF(LOG_BUG, "case num %d", dCurSet);
				break;
		}
	}
	if(dCurSet != 3)
	{
		errorPrint(DEF_ERR_FORM, DEBUG_GROUP_CONF, "GIFO", 0);
		return DEF_ERR_FORM;
	}
	return 1;
}

/*
 *     $Log: gifo_conf.c,v $
 *     Revision 1.1.1.1  2011/08/19 00:53:28  uamyd
 *     upst library
 *
 *     Revision 1.1  2011/07/26 04:52:23  dhkim
 *     *** empty log message ***
 *
 *     Revision 1.1.1.1  2011/01/11 01:33:02  jjinri
 *     DIFO
 *
 *     Revision 1.11  2011/01/05 08:57:30  swpark
 *     trim add
 *
 *     Revision 1.10  2010/12/29 05:37:23  swpark
 *     strtok NULL check
 *
 *     Revision 1.9  2010/12/29 02:53:42  swpark
 *     config file name get param
 *
 *     Revision 1.8  2010/12/15 04:57:21  swpark
 *     warning check
 *
 *     Revision 1.7  2010/12/14 08:15:23  swpark
 *     write matrix channel duplicate check del
 *
 *     Revision 1.6  2010/12/14 05:51:15  swpark
 *     error check
 *
 *     Revision 1.5  2010/12/14 05:46:18  swpark
 *     error check
 *
 *     Revision 1.4  2010/12/10 09:01:25  swpark
 *     S32 i add
 *
 *     Revision 1.3  2010/12/10 08:29:22  swpark
 *     stGIFO init modify
 *
 *     Revision 1.2  2010/12/10 05:38:10  swpark
 *     check
 *
 *     Revision 1.1  2010/12/09 09:26:29  swpark
 *     gifo config read func
 *
 *
 *
 */
