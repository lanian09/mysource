/** @file gifo.c
 * 	Interface Library file.
 *
 *	$Id: gifo.c,v 1.3 2011/08/31 04:26:04 uamyd Exp $
 *
 *	Copyright (c) 2006~ by Upresto Inc, Korea
 *	All rights reserved.
 * 
 *	@Author      $Author: uamyd $
 *	@version     $Revision: 1.3 $
 *	@date        $Date: 2011/08/31 04:26:04 $
 *	@ref         gifo.h
 *	@warning     nothing
 *	@todo        nothing
 *
 *	@section     Intro(소개)
 *	@section     Requirement
 *
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <errno.h>
#include <signal.h>

#include "gifo.h"


stCIFO *gifo_init(stGIFO *pstGIFO, st_CIFOCONF *pstCIFOCONF)
{
	stCIFO	*pstCIFO;
	stGIFO	*pHEAD;

	if((pstCIFO = cifo_init(pstCIFOCONF)) == NULL)
	{
		FPRINTF(LOG_BUG, "F=%s:%s.%d cifo_init NULL\n", __FILE__, __FUNCTION__, __LINE__);
		return NULL;
	}

	pHEAD = (stGIFO *)((U8 *)pstCIFO + pstCIFO->offsetHeadRoom);

	if( pstCIFO->uiHeadRoomSize < stGIFO_SIZE )
	{
		FPRINTF(LOG_BUG, "F=%s:%s.%d HeadRoomSize[%u] [%zd] NULL\n", __FILE__, __FUNCTION__, __LINE__,
				pstCIFO->uiHeadRoomSize, stGIFO_SIZE);
		return NULL;
	}
	memcpy(pHEAD, pstGIFO, stGIFO_SIZE);

	return pstCIFO;
}

/* gifo_init_group function
 * 
 * 초기화 함수
 *
 * @param *cifo_confFile : cifo config file name ("/DIFO/DATA/cifo.conf")
 * @param *gifo_confFile : gifo config file name ("/DIFO/DATA/gifo.conf")
 *
 * @return 		stCIFO * (CIFO pointer)
 *
 */

stCIFO *gifo_init_group(S8 *cifo_confFile, S8 *gifo_confFile)
{
	S32		dRet;
	stGIFO	stGIFO;
	st_CIFOCONF	stCIFOCONF;

	if((dRet = cifo_conf_init(&stCIFOCONF, cifo_confFile)) < 0)
	{
		FPRINTF(LOG_BUG, "F=%s:%s.%d cifo_conf_init dRet=%d\n", __FILE__, __FUNCTION__, __LINE__, dRet);
		return NULL;
	}
#ifdef DEBUG
	cifo_conf_print(&stCIFOCONF);
#endif

	if((dRet = gifo_conf_init(&stGIFO, &stCIFOCONF, gifo_confFile)) < 0)
	{
		FPRINTF(LOG_BUG, "F=%s:%s.%d gifo_conf_init dRet=%d\n", __FILE__, __FUNCTION__, __LINE__, dRet);
		return NULL;
	}
#ifdef DEBUG
	gifo_conf_print(&stGIFO);
#endif

	return gifo_conf_check(&stGIFO, &stCIFOCONF) < 0 ? NULL : gifo_init(&stGIFO, &stCIFOCONF);
}

S32 gifo_conf_check(stGIFO *pGIFO, st_CIFOCONF *pCIFOCONF)
{
	S32				i, j, dRet = 0;
	U32				chID;
	S32				procCnt[MAX_CHAN_CNT];

	for(i = 0; i < MAX_CHAN_CNT; i++)
	{
		procCnt[i] = 0;
	}

	for(i = 0; i < MAX_SEQ_PROC_NUM*MAX_SEQ_PROC_NUM; i++)
	{
		chID = pGIFO->uiMatrixChID[i];
		if(chID != INVALID_ID)
		{
			procCnt[chID]++;
		}
	}

	for(i = 0; i < MAX_CHAN_CNT; i++)
	{
		if((procCnt[i] > 1) && (pCIFOCONF->stCHANCONF[i].uiWSemFlag != MEMS_SEMA_ON))
		{
			FPRINTF(LOG_BUG, "HAVE TO USE SEMAPHORE CHID=%d PROCESS_CNT=%d\n", i, procCnt[i]);
			for(j = 0; j < MAX_SEQ_PROC_NUM*MAX_SEQ_PROC_NUM; j++)
			{
				if(pGIFO->uiMatrixChID[j] == i)
				{
					FPRINTF(LOG_BUG, "===> USE CHID=%d PROCESS SEQ=%d:%d\n", i, j/MAX_SEQ_PROC_NUM, j%MAX_SEQ_PROC_NUM);
				}
			}
			dRet = -1;
		}
	}

	return dRet;
}

void gifo_conf_print(stGIFO *pGIFO)
{
	S32 i, j;
	st_GROUP *pGr;
	
	/* gifo print */
	FPRINTF(LOG_BUG, "GIFO CONF: GroupCnt[%d] \n", pGIFO->uiGrCnt);
	for(i=0;i<pGIFO->uiGrCnt;i++)
	{
		pGr = &pGIFO->stGROUP[i];
		FPRINTF(LOG_BUG, "GROUP ID[%d] Channel Cnt[%d] \n", pGr->uiGrID, pGr->uiChCnt);
		for(j=0;j<pGr->uiChCnt;j++)
			FPRINTF(LOG_BUG, "CHANNEL ID[%d] \n", pGr->uiChID[j]);
	}
	
	FPRINTF(LOG_BUG, "GIFO CONF - Write PROC & Read Proc Channel Mapping \n");

	for(i=0;i<MAX_SEQ_PROC_NUM*MAX_SEQ_PROC_NUM;i++)
	{
		if(pGIFO->uiMatrixChID[i] != INVALID_ID)
		{
			FPRINTF(LOG_BUG, "WRITE PROC[%d] READ PROC[%d] CHANNEL ID[%d] \n", 
					i/1000, i%1000, pGIFO->uiMatrixChID[i]);
		}
	}

	FPRINTF(LOG_BUG, "GIFO CONF - READ PROC & GROUP Mapping \n");
	for(i=0;i<MAX_SEQ_PROC_NUM;i++)
	{
		if(pGIFO->uiMatrixGrID[i] != INVALID_ID)
		{
			FPRINTF(LOG_BUG, "READ PROC[%d] GROUP ID[%d]\n ",
					i, pGIFO->uiMatrixGrID[i]);
		}
	}
	FPRINTF(LOG_BUG, "GIFO CONF END \n");
}

/* gifo_write function
 *
 * @note : semaphore, buffering 관련 연산은 차후 고려. 
 *
 */
int	gifo_write(stMEMSINFO *pstMEMSINFO, stCIFO *pstCIFO, U32 uiWProcID, U32 uiRProcID, OFFSET offset)
{
	// wProcID * 1000 + rProcID -> chID
	stGIFO	*pstGIFO = (stGIFO *)((U8 *)pstCIFO + pstCIFO->offsetHeadRoom);
	return cifo_write(pstMEMSINFO, pstCIFO, pstGIFO->uiMatrixChID[uiWProcID*1000+uiRProcID], offset);
}

OFFSET gifo_read(stMEMSINFO *pstMEMSINFO, stCIFO *pstCIFO, U32 uiRProcID)
{
	int			curChCnt;
	st_GROUP	*pstGROUP = NULL;
	stGIFO		*pstGIFO = (stGIFO *)((U8 *)pstCIFO + pstCIFO->offsetHeadRoom);
	U32			uiGID = INVALID_ID;
	OFFSET		offset;
	U8 			*pHead, *pNext;

	if( uiRProcID >= MAX_SEQ_PROC_NUM )
	{
		FPRINTF(LOG_BUG, "uiRPocID range Error [%u / %u]\n", uiRProcID, MAX_SEQ_PROC_NUM);
		return -1;
	}

	uiGID = pstGIFO->uiMatrixGrID[uiRProcID];
		
	if( uiGID == INVALID_ID || uiGID >= pstGIFO->uiGrCnt )
	{
		FPRINTF(LOG_BUG, "Group ID Invalid uiRProcID[%u] GROUP COUNT[%u]\n", uiRProcID, pstGIFO->uiGrCnt);
		return -2;
	}

	pstGROUP = &pstGIFO->stGROUP[uiGID];

	pHead = NULL;
	for(curChCnt = 0; curChCnt < pstGROUP->uiChCnt; curChCnt++ )
	{
		offset = cifo_read(pstMEMSINFO, pstCIFO, pstGROUP->uiChID[curChCnt]);
		if(offset > 0)
		{
			pNext = nifo_ptr(pstMEMSINFO, offset);

			if(pHead == NULL) {
				pHead = pNext;
			}
			else {
				nifo_splice_nont(pstMEMSINFO, pNext, pHead);
			}
		}
	}
	return nifo_offset(pstMEMSINFO, pHead);
}


void gifo_print_group(stCIFO *pstCIFO, U32 uiGroupID)
{
	U32 i;

	stGIFO		*pstGIFO = (stGIFO *)((U8 *)pstCIFO + pstCIFO->offsetHeadRoom);
	st_GROUP	*pGroup = &pstGIFO->stGROUP[uiGroupID];

	for(i = 0; i < pGroup->uiChCnt; i++)
	{
		cifo_print_channel(pstCIFO, pGroup->uiChID[i]);
	}
}

void gifo_print(stCIFO *pstCIFO)
{
	U32 i;

	stGIFO	*pstGIFO = (stGIFO *)((U8 *)pstCIFO + pstCIFO->offsetHeadRoom);
	
	for(i = 0; i < pstGIFO->uiGrCnt; i++)
	{
		gifo_print_group(pstCIFO, pstGIFO->stGROUP[i].uiGrID);
	}
}

S32 gifo_set_buffcnt(stCIFO *pstCIFO, U32 uiWProcID, U32 uiRProcID, U32 uiBuffCnt, S32 uiRWFlag)
{
	stGIFO  *pstGIFO = (stGIFO *)((U8 *)pstCIFO + pstCIFO->offsetHeadRoom);

	if((uiWProcID*1000+uiRProcID) >=(MAX_SEQ_PROC_NUM*MAX_SEQ_PROC_NUM))
		return -1;

	return cifo_set_buffcnt(pstCIFO, pstGIFO->uiMatrixChID[uiWProcID*1000+uiRProcID], uiBuffCnt, uiRWFlag);
}

U32 gifo_get_buffcnt(stCIFO *pstCIFO, U32 uiWProcID, U32 uiRProcID, S32 uiRWFlag)
{
	stGIFO  *pstGIFO = (stGIFO *)((U8 *)pstCIFO + pstCIFO->offsetHeadRoom);
	if((uiWProcID*1000+uiRProcID) >=(MAX_SEQ_PROC_NUM*MAX_SEQ_PROC_NUM))
		return 0;
	
	return cifo_get_buffcnt(pstCIFO, pstGIFO->uiMatrixChID[uiWProcID*1000+uiRProcID], uiRWFlag);
}

U32 gifo_get_channel(stCIFO *pstCIFO, U32 uiWProcID, U32 uiRProcID)
{
	stGIFO  *pstGIFO = (stGIFO *)((U8 *)pstCIFO + pstCIFO->offsetHeadRoom);
	
	if((uiWProcID*1000+uiRProcID) >=(MAX_SEQ_PROC_NUM*MAX_SEQ_PROC_NUM))
		return INVALID_ID;
		
	return pstGIFO->uiMatrixChID[uiWProcID*1000+uiRProcID];
}

/*
 *     $Log: gifo.c,v $
 *     Revision 1.3  2011/08/31 04:26:04  uamyd
 *     modified
 *
 *     Revision 1.2  2011/08/28 11:51:00  uamyd
 *     fprintf ...line feed character added
 *
 *     Revision 1.1.1.1  2011/08/19 00:53:28  uamyd
 *     upst library
 *
 *     Revision 1.1  2011/07/26 04:52:23  dhkim
 *     *** empty log message ***
 *
 *     Revision 1.3  2011/03/24 07:26:19  dark264sh
 *     nifo, gifo: check validatoin
 *
 *     Revision 1.2  2011/01/21 01:39:38  swpark
 *     LOG classfy
 *
 *     Revision 1.1.1.1  2011/01/11 01:33:02  jjinri
 *     DIFO
 *
 *     Revision 1.16  2011/01/05 08:57:29  swpark
 *     PATH delete
 *
 *     Revision 1.15  2010/12/29 02:53:41  swpark
 *     config file name get param
 *
 *     Revision 1.14  2010/12/28 08:52:17  swpark
 *     gifo_get_channel exception add
 *
 *     Revision 1.13  2010/12/28 08:47:11  swpark
 *     gifo_get_channel exception add
 *
 *     Revision 1.12  2010/12/28 08:45:54  swpark
 *     gifo_get_channel exception add
 *
 *     Revision 1.11  2010/12/28 08:40:07  swpark
 *     gifo_get_procid delete
 *
 *     Revision 1.10  2010/12/28 07:24:45  swpark
 *     buffcnt api add
 *
 *     Revision 1.9  2010/12/14 10:42:21  jjinri
 *     Headroom size check
 *
 *     Revision 1.8  2010/12/10 08:26:31  dark264sh
 *     gifo gifo_init 에러 체크 추가
 *
 *     Revision 1.7  2010/12/10 05:35:50  dark264sh
 *     gifo gifo_conf_init시 에러 체크 추가
 *
 *     Revision 1.6  2010/12/09 08:41:08  jjinri
 *     gifo_print
 *
 *     Revision 1.5  2010/12/09 07:11:12  jjinri
 *     gifo init
 *
 *     Revision 1.4  2010/12/09 06:42:48  jjinri
 *     INAVLID_ID
 *
 *     Revision 1.3  2010/12/09 06:34:48  jjinri
 *     *** empty log message ***
 *
 *     Revision 1.2  2010/12/09 06:27:37  jjinri
 *     ..
 *
 *     Revision 1.1  2010/12/09 06:27:17  jjinri
 *     gifo start
 *
 */
