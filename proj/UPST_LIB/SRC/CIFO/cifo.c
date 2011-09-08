/** @file cifo.c
 * 	Interface Library file.
 *
 *	$Id $
 *
 *	Copyright (c) 2006~ by Upresto Inc, Korea
 *	All rights reserved.
 * 
 *	@Author      $Author: uamyd $
 *	@version     $Revision: 1.4 $
 *	@date        $Date: 2011/09/05 08:52:50 $
 *	@ref         cifo.h
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

#include "cifo.h"

/** cifo_init function. 
 *
 *  초기화 함수 
 *
 * 	@param uiShmKey: Shared Memory Key
 * 	@param uiWSemKey: Write Semaphore Key
 * 	@param uiRSemKey: Read Semaphore Key
 *
 *  @return     stCIFO *	
 *  @see        cifo.h cifo.c
 *
 *  @exception  규칙에 틀린 곳을 찾아주세요.
 *  @note       Nothing
 **/
stCIFO *cifo_init(st_CIFOCONF *pstCIFOCONF)
{
	U32				i;
	U8 				*pBase, *pHeadRoom;
	stCIFO 			*pstCIFO = NULL;
	OFFSET			uiTotMemSize;
	int				shm_id;
	st_CHAN			*pstCHAN;
	st_CHANCONF		*pstCHANCONF;
	OFFSET			offsetNodeStart;

	uiTotMemSize = (OFFSET)(st_CIFO_SIZE + pstCIFOCONF->uiHeadRoomSize);

	for(i = 0; i < pstCIFOCONF->uiChCnt; i++)
	{
		pstCHANCONF = &pstCIFOCONF->stCHANCONF[i];
		uiTotMemSize += (OFFSET)((pstCHANCONF->uiCellSize) * pstCHANCONF->uiCellCnt);	
	}

	if( uiTotMemSize > CIFO_SHM_SIZE )
	{
		FPRINTF(LOG_BUG, "CIFO_SHM_SIZE =%d uiTotMemSize = %ld\n", CIFO_SHM_SIZE, uiTotMemSize);
		return NULL;
	}

#ifdef DEBUG
	FPRINTF(LOG_BUG, "st_CIFO_SIZE =%zd st_CHAN_SIZE = %zd\n", st_CIFO_SIZE, st_CHAN_SIZE);
	FPRINTF(LOG_BUG, "CIFO TOTMEMSIZE=%ld CIFO_SHM_SIZE=%d\n", uiTotMemSize, CIFO_SHM_SIZE);
#endif

	if ((shm_id = shmget (pstCIFOCONF->uiShmKey, CIFO_SHM_SIZE, 0666 | IPC_CREAT | IPC_EXCL)) < 0) {

		if (errno == EEXIST) {
           	shm_id = shmget (pstCIFOCONF->uiShmKey, CIFO_SHM_SIZE, 0666 | IPC_CREAT);
           	if (shm_id < 0) {
				FPRINTF(LOG_BUG,"##%s Shm create fail!!: exit()\n",__FUNCTION__);
				return NULL;
			}
#ifdef DEBUG
			FPRINTF(LOG_LEVEL, "EEXIST..........\n");
#endif
           	if((pstCIFO = (stCIFO *) shmat(shm_id, 0, 0)) == (void *)-1)
			{
				FPRINTF(LOG_BUG, "shmat FAIL 1\n");
				return NULL;
			}

			return pstCIFO;
		}
	}

#ifdef DEBUG
	FPRINTF(LOG_LEVEL, " NO EEXIST..........\n");
#endif
	if((pstCIFO = (stCIFO *) shmat(shm_id, 0, 0)) == (void *)-1)
	{
		FPRINTF(LOG_BUG, "shmat FAIL 2\n");
		return NULL;
	}

	/* 맨 처음 초기화 되었을시 */
   	pstCIFO->uiTotMemSize 		= uiTotMemSize;
   	pstCIFO->uiShmKey 			= pstCIFOCONF->uiShmKey;
   	pstCIFO->uiHeadRoomSize 	= pstCIFOCONF->uiHeadRoomSize;
   	pstCIFO->offsetHeadRoom 	= st_CIFO_SIZE;
	pstCIFO->uiChCnt 			= pstCIFOCONF->uiChCnt;

#ifdef DEBUG
	FPRINTF(LOG_LEVEL, "HEADROOM=%ld HEADROOMSIZE=%u\n", 
						pstCIFO->offsetHeadRoom, pstCIFO->uiHeadRoomSize);
#endif

	offsetNodeStart = pstCIFO->offsetHeadRoom + pstCIFO->uiHeadRoomSize;
	for(i = 0; i < pstCIFOCONF->uiChCnt; i++)
	{
		pstCHANCONF = &pstCIFOCONF->stCHANCONF[i];
		pstCHAN = &pstCIFO->stCHAN[i];

		pstCHAN->uiWSemFlag 	= pstCHANCONF->uiWSemFlag;
		pstCHAN->uiWSemKey 		= pstCHANCONF->uiWSemKey;
		pstCHAN->iWSemID 		= mems_sem_init(pstCHANCONF->uiWSemKey, pstCHANCONF->uiWSemFlag);
		pstCHAN->iRSemID 		= mems_sem_init(pstCHANCONF->uiRSemKey, pstCHANCONF->uiRSemFlag);
		pstCHAN->uiChID 		= pstCHANCONF->uiChID;
   		pstCHAN->uiCellSize 	= pstCHANCONF->uiCellSize;
   		pstCHAN->uiCellCnt 		= pstCHANCONF->uiCellCnt;
		pstCHAN->cellStartOffset 	= offsetNodeStart;
   		pstCHAN->cellEndOffset 	= pstCHAN->cellStartOffset + (pstCHAN->uiCellSize * pstCHAN->uiCellCnt);
		pstCHAN->uiWBuffCnt 	= pstCHANCONF->uiWBuffCnt;
		pstCHAN->uiRBuffCnt 	= pstCHANCONF->uiRBuffCnt;
		pstCHAN->uiWCnt 		= 0;
		pstCHAN->uiRCnt 		= 0;
		pstCHAN->wOffset 		= 0;
		pstCHAN->rOffset 		= 0;
		offsetNodeStart 		= pstCHAN->cellEndOffset;
#ifdef DEBUG
		FPRINTF(LOG_LEVEL, "CH=%u START=%ld END=%ld\n", 
							pstCHAN->uiChID, pstCHAN->cellStartOffset, pstCHAN->cellEndOffset);
#endif
	}

	pBase = (U8 *)(pstCIFO); 
    pHeadRoom = (U8 *)(pBase + pstCIFO->offsetHeadRoom);
    memset(pHeadRoom, 0x00, pstCIFO->uiHeadRoomSize);

#ifdef DEBUG
	FPRINTF(LOG_LEVEL, "SHMKEY=%u TOTMEMSIZE=%ld HEADROOMSIZE=%u HEADROOMOFFSET=%ld CHCNT=%u\n",
						pstCIFO->uiShmKey, pstCIFO->uiTotMemSize, pstCIFO->uiHeadRoomSize,
						pstCIFO->offsetHeadRoom, pstCIFO->uiChCnt);
#endif

	for(i = 0; i < pstCIFO->uiChCnt; i++)
	{
		pstCHAN = &pstCIFO->stCHAN[i];

#ifdef DEBUG
		FPRINTF(LOG_LEVEL, "WSEMFLAG=%u WSEMKEY=%u WSEMID=%d RSEMFLAG=%u RSEMKEY=%u RSEMID=%d "
							"CHID=%u CELLCNT=%u CELLSIZE=%u START=%ld END=%ld\n",
			pstCHAN->uiWSemFlag, pstCHAN->uiWSemKey, pstCHAN->iWSemID, 
			pstCHAN->uiRSemFlag, pstCHAN->uiRSemKey, pstCHAN->iRSemID, 
			pstCHAN->uiChID, pstCHAN->uiCellCnt, pstCHAN->uiCellSize, 
			pstCHAN->cellStartOffset, pstCHAN->cellEndOffset);
#endif
	}

	return pstCIFO;
}

/* cifo_init_channel function
 * 
 * 초기화 함수 
 *
 * @param *confFile : cifo config file name ("/DIFO/DATA/cifo.conf")
 *
 * @return      stCIFO * (CIFO pointer)
 *
 */

stCIFO *cifo_init_channel(S8 *confFile)
{
	S32				dRet;
	st_CIFOCONF		stCIFOCONF;
	st_CIFOCONF		*pCIFOCONF = &stCIFOCONF;

	if((dRet = cifo_conf_init(pCIFOCONF, confFile)) < 0)
	{
		FPRINTF(LOG_BUG, "F=%s:%s.%d cifo_conf_init dRet=%d\n", __FILE__, __FUNCTION__, __LINE__, dRet);
		return NULL;
	}

#ifdef DEBUG
	cifo_conf_print(pCIFOCONF);
#endif

	return cifo_init(pCIFOCONF);
}

void cifo_conf_print(st_CIFOCONF *pCIFOCONF)
{
	S32 i;
	st_CHANCONF *pCHANCONF;
	
	FPRINTF(LOG_BUG, "CIFO CONF: ShmKey[%d], HeadRoomSize[%d], Channel Cnt[%d] \n"
			, pCIFOCONF->uiShmKey, pCIFOCONF->uiHeadRoomSize, pCIFOCONF->uiChCnt);

	for(i=0;i<pCIFOCONF->uiChCnt;i++)
	{
		pCHANCONF = &pCIFOCONF->stCHANCONF[i];
		FPRINTF(LOG_BUG, "CIFO CONF: Channel ID[%d], Cell Cnt[%d], Cell Size[%d], Write Buffer Cnt[%d], Read Buffer Cnt[%d], Write Sema Flag[%d], Write Sema Key[%d], Write Sema ID[%d], Read Sema Flag[%d], Read Sema Key[%d], Read Sema ID[%d] \n", 
				pCHANCONF->uiChID, pCHANCONF->uiCellCnt, pCHANCONF->uiCellSize, 
				pCHANCONF->uiWBuffCnt, pCHANCONF->uiRBuffCnt, pCHANCONF->uiWSemFlag, 
				pCHANCONF->uiWSemKey, pCHANCONF->iWSemID, pCHANCONF->uiRSemFlag, 
				pCHANCONF->uiRSemKey, pCHANCONF->iRSemID);
	}
	FPRINTF(LOG_BUG, "CIFO CONF END \n");
}

/* cifo_write function
 *
 * @note : semaphore, buffering 관련 연산은 차후 고려. 
 *
 */
int	cifo_write(stMEMSINFO *pstMEMSINFO, stCIFO *pstCIFO, U32 uiChID, OFFSET offset)
{
	OFFSET	 	*pCell = NULL;
	st_CHAN		*pCHAN;
	U32		curReadPos, curWritePos, nextWritePos;
	OFFSET		input = 0;
	U8		*pHead, *pNext;

	// Channel ID Range Check
	if(uiChID >= pstCIFO->uiChCnt)
	{
		FPRINTF(LOG_BUG, "%s:Channel ID Range Error[%u/%u]\n", __FUNCTION__, uiChID, pstCIFO->uiChCnt);
		return -1;
	}
	
	pCHAN = &pstCIFO->stCHAN[uiChID];

	P(pCHAN->iWSemID, pCHAN->uiWSemFlag);
	curReadPos = pCHAN->rCellPos;
	curWritePos = pCHAN->wCellPos;
	nextWritePos = cifo_next_pos(pCHAN->wCellPos, pCHAN->uiCellCnt);

	// Circle Queue Full Check
	if((nextWritePos == curReadPos) && ((pCHAN->uiWCnt + 1) >= pCHAN->uiWBuffCnt))
	{
		//FPRINTF(LOG_BUG, "Queue Full nextWritePos[%u] wCellPos[%u] rCellPos[%u]", 
		//		nextWritePos, curWritePos, curReadPos);
		V(pCHAN->iWSemID, pCHAN->uiWSemFlag);
		return -2;
	}

	if(pCHAN->wOffset > 0)
	{
		pHead = nifo_ptr(pstMEMSINFO, pCHAN->wOffset);
		pNext = nifo_ptr(pstMEMSINFO, offset);
		nifo_splice_nont(pstMEMSINFO, pNext, pHead);
	}
	else
	{
		pCHAN->wOffset = offset;
	}
	pCHAN->uiWCnt++;

	if(pCHAN->uiWCnt >= pCHAN->uiWBuffCnt)
	{
		input = pCHAN->wOffset;
		pCHAN->wOffset = 0;
		pCHAN->uiWCnt = 0;
	}

	if(input > 0)
	{
		pCell = (OFFSET *)nifo_ptr(pstCIFO, cifo_get_cell(pCHAN, curWritePos));
		*pCell = input;
		pCHAN->wCellPos = nextWritePos;
	}

	V(pCHAN->iWSemID, pCHAN->uiWSemFlag);

	return 1;
}

OFFSET cifo_read(stMEMSINFO *pstMEMSINFO, stCIFO *pstCIFO, U32 uiChID)
{
	OFFSET		*pCell = NULL;
	U8			*pHead, *pNext;
	U32			curCnt = 0;
	st_CHAN		*pCHAN;
	U32			curReadPos, nextReadPos, curWritePos;

	// Channel ID Range Check
	if(uiChID >= pstCIFO->uiChCnt)
	{
		FPRINTF(LOG_BUG, "%s: Channel ID Range Error[%u/%u]\n", __FUNCTION__, uiChID, pstCIFO->uiChCnt);
		return -1;
	}

	pCHAN = &pstCIFO->stCHAN[uiChID];

	P(pCHAN->iRSemID, pCHAN->uiRSemFlag);
	curWritePos = pCHAN->wCellPos;

	pHead = NULL;
	for(curCnt = 0; curCnt < pCHAN->uiRBuffCnt; curCnt++)
	{
		curReadPos = pCHAN->rCellPos;
		nextReadPos = cifo_next_pos(pCHAN->rCellPos, pCHAN->uiCellCnt);

		if(curReadPos == curWritePos)
		{
			//FPRINTF(LOG_LEVEL, "Queue Empty nextReadPos[%u] wCellPos[%u] rCellPos[%u]", 
			//	nextReadPos, curWritePos, curReadPos);
			break;
		}

		pCell = (OFFSET *)nifo_ptr(pstCIFO, cifo_get_cell(pCHAN, curReadPos));
		pNext = nifo_ptr(pstMEMSINFO, *pCell);
		
		if(pHead == NULL) {
			pHead = pNext;
		}
		else {
			nifo_splice_nont(pstMEMSINFO, pNext, pHead);
		}

		pCHAN->rCellPos = nextReadPos;
	}

	V(pCHAN->iRSemID, pCHAN->uiRSemFlag);
	return nifo_offset(pstMEMSINFO, pHead);
}

U32 cifo_get_count(S64 llWPos, S64 llRPos, S64 llTotCnt)
{
	S64	llCnt;

	llCnt = llWPos - llRPos;

	if(llCnt >= 0)
	{
		return llCnt;
	}
	else
	{
		return (llTotCnt + llCnt);
	}
}

U32 cifo_get_buffcnt(stCIFO *pstCIFO, U32 uiChID, S32 uiRWFlag)
{
	U32 uiBuffCnt = 0;
	if(uiChID != INVALID_ID)
	{
		switch (uiRWFlag)
		{
			case CIFO_R_FLAG:
				uiBuffCnt = pstCIFO->stCHAN[uiChID].uiRBuffCnt;
				break;
			case CIFO_W_FLAG:
				uiBuffCnt = pstCIFO->stCHAN[uiChID].uiWBuffCnt;
				break;
			default:
				break;
		}
	}
	return uiBuffCnt;
}

S32 cifo_set_buffcnt(stCIFO *pstCIFO, U32 uiChID, U32 uiBuffCnt, S32 uiRWFlag)
{
	S32 dRet = 0;
	if(uiBuffCnt == 0 || uiChID == INVALID_ID)
		dRet = -1;
	else 
	{
		switch (uiRWFlag)
		{
			case CIFO_R_FLAG:
				pstCIFO->stCHAN[uiChID].uiRBuffCnt = uiBuffCnt;
				dRet = 1;
				break;
			case CIFO_W_FLAG:
				pstCIFO->stCHAN[uiChID].uiWBuffCnt = uiBuffCnt;
				dRet = 1;
				break;
			default:
				break;
		}
	}
	return dRet;
}

U32 cifo_used_count(stCIFO *pstCIFO, U32 uiChID)
{
	return (cifo_get_count(pstCIFO->stCHAN[uiChID].wCellPos, pstCIFO->stCHAN[uiChID].rCellPos, pstCIFO->stCHAN[uiChID].uiCellCnt));
}

U32 cifo_free_count(stCIFO *pstCIFO, U32 uiChID)
{
	return (pstCIFO->stCHAN[uiChID].uiCellCnt - cifo_used_count(pstCIFO, uiChID));
}

U32 cifo_max_count(stCIFO *pstCIFO, U32 uiChID)
{
	return pstCIFO->stCHAN[uiChID].uiCellCnt;	
}

void cifo_print_channel(stCIFO *pstCIFO, U32 uiChID)
{
	U32 uiWPos, uiRPos;

	uiWPos = pstCIFO->stCHAN[uiChID].wCellPos;
	uiRPos = pstCIFO->stCHAN[uiChID].rCellPos;

	FPRINTF(LOG_BUG, "CHID[%u]wCellPos[%u]rCellPos[%u]MAX[%u]USED[%u]FREE[%u]\n", 
		pstCIFO->stCHAN[uiChID].uiChID, uiWPos, uiRPos, pstCIFO->stCHAN[uiChID].uiCellCnt,
		cifo_get_count(uiWPos, uiRPos, pstCIFO->stCHAN[uiChID].uiCellCnt),
		pstCIFO->stCHAN[uiChID].uiCellCnt - cifo_get_count(uiWPos, uiRPos, pstCIFO->stCHAN[uiChID].uiCellCnt));
}

void cifo_print(stCIFO *pstCIFO)
{
	U32 loop;
	for(loop = 0; loop < pstCIFO->uiChCnt; loop++)
	{
		cifo_print_channel(pstCIFO, loop);
	}
}

/** main function.
 *
 *  @return     void
 *  @see        cifo.h
 *
 **/
#if 0
int main(int argc, char *argv[])
{
	stCIFO		*pstCIFO;

	/* CIFO INIT */
	pstCIFO = cifo_init_channel("../../DATA/cifo.conf");

	return 0;
}
#endif /* TEST */

/*
 *     $Log: cifo.c,v $
 *     Revision 1.4  2011/09/05 08:52:50  uamyd
 *     modified
 *
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
 *     Revision 1.5  2011/03/29 11:40:10  dark264sh
 *     cifo: Queue Full일때 return 값 변경 (0 => -2)
 *
 *     Revision 1.4  2011/03/24 07:25:44  dark264sh
 *     nifo, gifo: check validatoin
 *
 *     Revision 1.3  2011/03/24 01:44:43  dark264sh
 *     cifo: cifo_get_count에서 사용량 잘못 계산되는 버그 수정
 *
 *     Revision 1.2  2011/01/21 01:39:37  swpark
 *     LOG classfy
 *
 *     Revision 1.1.1.1  2011/01/11 01:33:02  jjinri
 *     DIFO
 *
 *     Revision 1.20  2011/01/05 08:57:27  swpark
 *     PATH delete
 *
 *     Revision 1.19  2010/12/29 02:53:39  swpark
 *     config file name get param
 *
 *     Revision 1.18  2010/12/28 08:52:25  swpark
 *     gifo_get_channel exception add
 *
 *     Revision 1.17  2010/12/28 07:24:36  swpark
 *     buffcnt api add
 *
 *     Revision 1.16  2010/12/13 07:40:50  dark264sh
 *     cifo cifo_read *pCell offset -> 실제 메모리 주소로 수정
 *
 *     Revision 1.15  2010/12/13 04:14:03  dark264sh
 *     *** empty log message ***
 *
 *     Revision 1.14  2010/12/13 02:01:31  swpark
 *     *pCell -> real memory
 *
 *     Revision 1.13  2010/12/10 05:51:46  dark264sh
 *     cifo CIFO_SHM_SIZE 출력 추가
 *
 *     Revision 1.12  2010/12/10 05:25:37  dark264sh
 *     cifo dAppLog변경 및 cifo_conf_init시 에러 체크 추가
 *
 *     Revision 1.11  2010/12/09 08:27:06  jjinri
 *     cifo_print
 *
 *     Revision 1.10  2010/12/09 07:41:54  jjinri
 *     cifo_init CIFO_SHM_SIZE
 *
 *     Revision 1.9  2010/12/09 07:11:28  jjinri
 *     cifo conf init
 *
 *     Revision 1.8  2010/12/09 06:31:27  jjinri
 *     *** empty log message ***
 *
 *     Revision 1.7  2010/12/09 06:29:15  jjinri
 *     ..
 *
 *     Revision 1.6  2010/12/09 06:28:56  jjinri
 *     semaphore add
 *
 */
