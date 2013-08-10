/** @file mems.c
 * ���� Memory Library file.
 *
 *		$Id: mems.c,v 1.3 2011/08/31 04:26:05 uamyd Exp $
 *
 *     Copyright (c) 2006~ by Upresto Inc, Korea
 *     All rights reserved.
 * 
 *     @Author      $Author: uamyd $
 *     @version     $Revision: 1.3 $
 *     @date        $Date: 2011/08/31 04:26:05 $
 *     @ref         mems.h
 *     @todo        hasho project�� �̰��� ������ ���̴�. 
 *
 *     @section     Intro(�Ұ�)
 *      - mems library ���� 
 *      - mems_init���� �ʱ�ָ� ���Ŀ� �⺻���� primitives �� �̿��ϰ� ��
 *      - primitives
 *      	@li mems_init
 *      	@li mems_alloc
 *      	@li mems_free
 *      	@li mems_print_info
 *      	@li mems_print_node
 *      	@li mems_print_all
 *      	@li Supplement 		: mems_draw_all
 *      	@li Supplement 		: mems_garbage_collector	-  ���� �ð����� ó�� �ȵ� �͵��� �����ְ� �ش� ������ ���
 *      - memory ����\n
 *          +----------------------------------+\n
 *          |    mems_info (stMEMSINFO)        |\n
 *          +----------------------------------+\n
 *          |    Head Room                     |\n
 *          +----------------------------------+\n
 *          |    mems node hdr1(stMEMSNODEHDR) |\n
 *          |    mems node data                |\n
 *          +----------------------------------+\n
 *          |    mems node hdr2(stMEMSNODEHDR) |\n
 *          |    mems node data                |\n
 *          +----------------------------------+\n
 *          |    mems node hdr3(stMEMSNODEHDR) |\n
 *          |    mems node data                |\n
 *          +----------------------------------+\n
 *
 *     @section     Requirement
 *      @li perl , doxygen , graphviz
 *
 *
 **/




#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <errno.h>

#include "mems.h"



# define iscntrl(c) __isctype((c), _IScntrl)
#define WIDTH   16
int
mems_dump_DebugString(char *debug_str,char *s,int len)
{
	char buf[BUFSIZ],lbuf[BUFSIZ],rbuf[BUFSIZ];
	unsigned char *p;
	int line,i;

	FPRINTF(LOG_LEVEL, "### %s\n",debug_str);
	p =(unsigned char *) s;
	for(line = 1; len > 0; len -= WIDTH,line++) {
		memset(lbuf,0,BUFSIZ);
		memset(rbuf,0,BUFSIZ);

		for(i = 0; i < WIDTH && len > i; i++,p++) {
			sprintf(buf,"%02x ",(unsigned char) *p);
			strcat(lbuf,buf);
			sprintf(buf,"%c",(!iscntrl(*p) && *p <= 0x7f) ? *p : '.');
			strcat(rbuf,buf);
		}
		FPRINTF(LOG_LEVEL, "%04x: %-*s    %s\n",line - 1,WIDTH * 3,lbuf,rbuf);
	}
	return line;
}

#define SEMPERM     0666

/** �⺻ mems function : mems_sem_init function. 
 *
 * MEM���� ��� data�� ����ش�.
 * 
 * @param semkey	:  Semaphore Key
 * @param flag	:  Semaphore Flag (MEMS_SEMA_ON) 
 *
 *  @return     S32
 *  @see        mems.h
 *
 *  @note       ������ ���Ϸ� �����ȴ�. 
 **/
S32 mems_sem_init(key_t semkey, U32 flag)
{
	S32 ret = 0, semid = -1;
	
	if(flag != MEMS_SEMA_ON)
		return 0;

	if((semid = semget(semkey, 1, SEMPERM | IPC_CREAT | IPC_EXCL)) == -1)
	{
		if(errno == EEXIST)
			semid = semget(semkey, 1, SEMPERM | IPC_CREAT);
	}
	else	/* if created... */
	{
		ret = semctl(semid, 0, SETVAL, 1);
	}

	if(semid == -1 || ret == -1)
		exit(13);
	else
		return semid;
}

/** �⺻ mems function : mems_sem_remove function. 
 *
 * MEM���� ��� data�� ����ش�.
 * 
 * @param semkey	:  Semaphore Key 
 * @param flag	:  Semaphore Flag (MEMS_SEMA_ON) 
 *
 *  @return     S32
 *  @see        mems.h
 *
 *  @note       ������ ���Ϸ� �����ȴ�. 
 **/
S32 mems_sem_remove(key_t semkey, U32 flag)
{
	S32 ret = 0, semid = -1;

	if(flag != MEMS_SEMA_ON)
		return 0;

	if((semid = semget(semkey, 1, 0)) == -1)
	{
		return 1;	/* error */
	}

	ret = semctl(semid, 1, IPC_RMID, 1);
	if(ret < 0)
	{
		if(errno == EIDRM)
			return 0;
		else
			exit(14);
	}

	return 0;
}

/** �⺻ mems function : P function. 
 *
 * MEM���� ��� data�� ����ش�.
 * 
 * @param semid	:  Semaphore ID
 * @param flag	:  Semaphore Flag (MEMS_SEMA_ON) 
 *
 *  @return     S32
 *  @see        mems.h
 *
 *  @note       ������ ���Ϸ� �����ȴ�. 
 **/
S32 P(S32 semid, U32 flag)
{
	struct sembuf p_buf;

	if(flag != MEMS_SEMA_ON)
		return 0;

	p_buf.sem_num = 0;
	p_buf.sem_op  = -1;
	p_buf.sem_flg = SEM_UNDO;

	if(semop(semid, &p_buf, 1) == -1)
		exit(15);
	else
		return 0;
}

/** �⺻ mems function : V function. 
 *
 * MEM���� ��� data�� ����ش�.
 * 
 * @param semid	:  Semaphore ID
 * @param flag	:  Semaphore Flag (MEMS_SEMA_ON) 
 *
 *  @return     S32
 *  @see        mems.h
 *
 *  @note       ������ ���Ϸ� �����ȴ�. 
 **/
S32 V(S32 semid, U32 flag)
{
	struct sembuf v_buf;

	if(flag != MEMS_SEMA_ON)
		return 0;

	v_buf.sem_num = 0;
	v_buf.sem_op  = 1;
	v_buf.sem_flg = SEM_UNDO;

	if(semop(semid, &v_buf, 1) == -1)
		exit(16);
	else
		return 0;
}

/** �ʱ�ȭ �Լ� : mems_init function. 
 *
 *  �ʱ�ȭ �Լ� 
 *
 * @param uiType	: Main Memory = 1 , Shared Memory = 2 
 * @param uiShmKey  	: Shared Memory Key (case type is shared Memory)
 * @param uiSemFlag  	: Semaphore Flag (MEMS_SEMA_ON)
 * @param uiSemKey  	: Semaphore Key 
 * @param uiHeadRoomSize  	: sort�� ���� key�� byte��
 * @param uiMemNodeBodySize  	: Data�� byte��
 * @param uiMemNodeTotCnt  	: open mems�� array size (open memsũ��) 
 *
 *  @return     void
 *  @see        mems.h
 *
 *  @exception  ��Ģ�� Ʋ�� ���� ã���ּ���.
 *  @note       ���� Shared Memory�ȿ� �ִ� �͵� ���� �� ��. mem ���� �κ��� �߰� �Ǿ����� ��.
 **/
stMEMSINFO *
mems_init(st_MEMSCONF *pstMEMSCONF)
{
	U32	iIndex, i;
	stMEMSNODEHDR *pstMEMSNODEHDR;
	U8 *pBase,*pIndex, *pHeadRoom;
	stMEMSINFO *pstMEMSINFO = NULL;
	OFFSET	uiTotMemSize;
	int		shm_id;
	st_MEMSZONE		*pstMEMSZONE;
	st_MEMSZONECONF	*pstMEMSZONECONF;
	OFFSET			offsetNodeStart;

	uiTotMemSize = (OFFSET)(stMEMSINFO_SIZE + pstMEMSCONF->uiHeadRoomSize);

	for(i = 0; i < pstMEMSCONF->uiZoneCnt; i++)
	{
		pstMEMSZONECONF = &pstMEMSCONF->stMEMSZONECONF[i];
		uiTotMemSize += (OFFSET)((stMEMSNODEHDR_SIZE + pstMEMSZONECONF->uiMemNodeBodySize) * pstMEMSZONECONF->uiMemNodeTotCnt);	
	}

	if( uiTotMemSize > MEMS_SHM_SIZE )
	{
		FPRINTF(LOG_BUG, "MEMS_SHM_SIZE =%d uiTotMemSize = %ld\n", MEMS_SHM_SIZE, uiTotMemSize);
		return NULL;
	}
#ifdef DEBUG
	FPRINTF(LOG_BUG, "NIFO TOTMEMSIZE=%ld MEMS_SHM_SIZE=%d\n", uiTotMemSize, MEMS_SHM_SIZE);
#endif

	switch(pstMEMSCONF->uiType){
		case MEMS_MAIN_MEM:
			pstMEMSINFO = (stMEMSINFO *) MALLOC(uiTotMemSize);
			break;
		case MEMS_SHARED_MEM:
            if ((shm_id = shmget (pstMEMSCONF->uiShmKey, MEMS_SHM_SIZE, SEMPERM | IPC_CREAT | IPC_EXCL)) < 0) {

				if (errno == EEXIST) {
                	shm_id = shmget (pstMEMSCONF->uiShmKey, MEMS_SHM_SIZE, SEMPERM | IPC_CREAT);
                	if (shm_id < 0) {
						FPRINTF(LOG_BUG,"##%s Shm create fail!!: exit()\n",__FUNCTION__);
						return NULL;
					}

#ifdef DEBUG
					FPRINTF(LOG_LEVEL, "EEXIST..........\n");
#endif
                	if((pstMEMSINFO = (stMEMSINFO *) shmat(shm_id, 0, 0)) == (void *)-1)
					{
						FPRINTF(LOG_BUG, "shmat FAIL 1\n");
						return NULL;
					}

					return pstMEMSINFO;
				}
            }

#ifdef DEBUG
			FPRINTF(LOG_LEVEL, " NO EEXIST..........\n");
#endif
            if((pstMEMSINFO = (stMEMSINFO *) shmat(shm_id, 0, 0)) == (void *)-1)
			{
				FPRINTF(LOG_BUG, "shmat FAIL 2\n");
				return NULL;
			}
			break;
		default: 
			return NULL;
	}


	/* �� ó�� �ʱ�ȭ �Ǿ����� */
   	pstMEMSINFO->uiTotMemSize = uiTotMemSize;
   	pstMEMSINFO->uiType = pstMEMSCONF->uiType;
   	pstMEMSINFO->uiShmKey = pstMEMSCONF->uiShmKey;
   	pstMEMSINFO->uiHeadRoomSize = pstMEMSCONF->uiHeadRoomSize;
   	pstMEMSINFO->uiMemNodeHdrSize = stMEMSNODEHDR_SIZE;
   	pstMEMSINFO->offsetHeadRoom = stMEMSINFO_SIZE;
	pstMEMSINFO->uiZoneCnt = pstMEMSCONF->uiZoneCnt;

#ifdef DEBUG
	FPRINTF(LOG_BUG, "HEADROOM=%ld SIZE=%u\n", pstMEMSINFO->offsetHeadRoom, pstMEMSINFO->uiHeadRoomSize);
#endif

	offsetNodeStart = pstMEMSINFO->offsetHeadRoom + pstMEMSINFO->uiHeadRoomSize;
	for(i = 0; i < pstMEMSCONF->uiZoneCnt; i++)
	{
		pstMEMSZONECONF = &pstMEMSCONF->stMEMSZONECONF[i];
		pstMEMSZONE = &pstMEMSINFO->stMEMSZONE[i];

		pstMEMSZONE->uiSemFlag = pstMEMSZONECONF->uiSemFlag;
		pstMEMSZONE->uiSemKey = pstMEMSZONECONF->uiSemKey;
		pstMEMSZONE->iSemID = mems_sem_init(pstMEMSZONECONF->uiSemKey, pstMEMSZONECONF->uiSemFlag);
		pstMEMSZONE->uiZoneID = pstMEMSZONECONF->uiZoneID;
   		pstMEMSZONE->uiMemNodeBodySize = pstMEMSZONECONF->uiMemNodeBodySize;
   		pstMEMSZONE->uiMemNodeAllocedCnt = 0;
   		pstMEMSZONE->uiMemNodeTotCnt = pstMEMSZONECONF->uiMemNodeTotCnt;
		pstMEMSZONE->offsetNodeStart = offsetNodeStart;
   		pstMEMSZONE->offsetFreeList = pstMEMSZONE->offsetNodeStart;
   		pstMEMSZONE->offsetNodeEnd = pstMEMSZONE->offsetNodeStart + 
					(stMEMSNODEHDR_SIZE + pstMEMSZONE->uiMemNodeBodySize) * pstMEMSZONE->uiMemNodeTotCnt;
		offsetNodeStart = pstMEMSZONE->offsetNodeEnd;
		pstMEMSZONE->createCnt = 0;
		pstMEMSZONE->delCnt = 0;
#ifdef DEBUG
		FPRINTF(LOG_BUG, "ZONE=%d START=%ld FREE=%ld END=%ld\n", pstMEMSZONE->uiZoneID, pstMEMSZONE->offsetNodeStart, pstMEMSZONE->offsetFreeList, pstMEMSZONE->offsetNodeEnd);
#endif
	}

	pBase = (U8 *)(pstMEMSINFO); 
    pHeadRoom = (U8 *)(pBase + pstMEMSINFO->offsetHeadRoom);
    memset(pHeadRoom, 0x00, pstMEMSINFO->uiHeadRoomSize);

	for(i = 0; i < pstMEMSINFO->uiZoneCnt; i++)
	{
		pstMEMSZONE = &pstMEMSINFO->stMEMSZONE[i];

		pBase = (U8 *)(pstMEMSINFO); 
		pBase += pstMEMSZONE->offsetNodeStart;
		for(iIndex=0; iIndex < pstMEMSZONE->uiMemNodeTotCnt; iIndex++){
			pIndex = (U8*)(pBase + iIndex * (stMEMSNODEHDR_SIZE + pstMEMSZONE->uiMemNodeBodySize)); 
			pstMEMSNODEHDR = (stMEMSNODEHDR *)pIndex;
			pstMEMSNODEHDR->uiID = MEMS_ID;
			pstMEMSNODEHDR->uiZoneID = pstMEMSZONE->uiZoneID;
			pstMEMSNODEHDR->TimeSec = 0;
			pstMEMSNODEHDR->ucIsFree = MEMS_FREE;
			pstMEMSNODEHDR->DebugStr[0] = 0x00;
			pstMEMSNODEHDR->DelStr[0] = 0x00;
#ifdef DEBUG
			FPRINTF(LOG_LEVEL,"##%s : [%3d] pstMEMSNODEHDR 0x%x, ID :%u\n",__FUNCTION__,iIndex,(U32*) pstMEMSNODEHDR,pstMEMSNODEHDR->uiID);
			OFFSET _offset = iIndex * ( stMEMSNODEHDR_SIZE + pstMEMSZONE-> uiMemNodeBodySize );
			FPRINTF(LOG_LEVEL,"##%s : offset %u 0x%x\n",__FUNCTION__, _offset, _offset);
			//FPRINTF(LOG_LEVEL,"##%s : offset %u 0x%x\n",__FUNCTION__,iIndex * (stMEMSNODEHDR_SIZE + pstMEMSZONE->uiMemNodeBodySize),iIndex * (stMEMSNODEHDR_SIZE + pstMEMSZONE->uiMemNodeBodySize));
#endif
		}
	}

#ifdef DEBUG
	FPRINTF(LOG_BUG, "TYPE=%u SHMKEY=%u TOTMEMSIZE=%ld HEADROOMSIZE=%u MEMHDRSIZE=%u HEADROOMOFFSET=%ld ZONECNT=%u\n",
			pstMEMSINFO->uiType, pstMEMSINFO->uiShmKey, pstMEMSINFO->uiTotMemSize, pstMEMSINFO->uiHeadRoomSize,
			pstMEMSINFO->uiMemNodeHdrSize, pstMEMSINFO->offsetHeadRoom, pstMEMSINFO->uiZoneCnt);
#endif

	for(i = 0; i < pstMEMSINFO->uiZoneCnt; i++)
	{
		pstMEMSZONE = &pstMEMSINFO->stMEMSZONE[i];

#ifdef DEBUG
		FPRINTF(LOG_BUG, "SEMFLAG=%u SEMKEY=%u SEMID=%d ZONEID=%u BODY=%u ALLOC=%u TOT=%u START=%ld FREE=%ld END=%ld\n",
			pstMEMSZONE->uiSemFlag, pstMEMSZONE->uiSemKey, pstMEMSZONE->iSemID, pstMEMSZONE->uiZoneID,
			pstMEMSZONE->uiMemNodeBodySize, pstMEMSZONE->uiMemNodeAllocedCnt, pstMEMSZONE->uiMemNodeTotCnt,
			pstMEMSZONE->offsetNodeStart, pstMEMSZONE->offsetFreeList, pstMEMSZONE->offsetNodeEnd);
#endif
	}

	memcpy(pstMEMSINFO->uiMatrixZoneID, pstMEMSCONF->uiMatrixZoneID, sizeof(U32)*MAX_SEQ_PROC_NUM);
	
	return pstMEMSINFO;
}




/** �⺻ mems function : mems_alloc function. 
 *
 * NODE�� mem���� �Ҵ��Ѵ�. 
 * -DDEBUG  �Ҵ��� �Լ��� �־��ش�.  
 *
 * @li ID�� �����Ͽ� Ȯ���� �Ѵ�.
 * @li Free���� Alloc�� Free�� Ȯ���ϰ� ���� ALLOCED�� set
 * @li ȣ���� �Լ� �̸� ����
 * 
 *
 * @param *pstMEMSINFO	: MEM ������ 
 * @param uiSize	:  NODE�� �Ҵ��� Size (�����δ� INFO�� ���ǵ� Size��ŭ�� alloc�� ���̴�.)  ASSERT(pstMEMSINFO->uiMemNodeBodySize >= uiSize);
 * @param *pDbgPtr	: __FUNCTION__ ���� ȣ���� �Լ��� Ư�� string�� ��Ÿ�� 
 *
 *  @return     success : not 0 , fail : 0 
 *  @see        mems.h
 *
 *  @exception  ��Ģ�� Ʋ�� ���� ã���ּ���.
 *  @note       **pp�� *pstmemsnode[100] ����  pp = &pstmemsnode[1]�� ���� ó���ϱ� ������ \n
 **/
U8 *
mems_alloc(stMEMSINFO *pstMEMSINFO , U32 uiSize, U32 uiZoneID, U8 *pDbgPtr)
{
	U8 *pBase,*pRet = NULL,*pFree;
	stMEMSNODEHDR *pstMEMSNODEHDR;
	U32 iDebug=0;
	st_MEMSZONE		*pstMEMSZONE;

	pstMEMSZONE = &pstMEMSINFO->stMEMSZONE[uiZoneID];

	if(pstMEMSZONE->uiMemNodeBodySize < uiSize) return NULL;

	P(pstMEMSZONE->iSemID, pstMEMSZONE->uiSemFlag);

	pBase = (U8 *) pstMEMSINFO;
	/* pBase += pstMEMSINFO->uiHeadRoomSize; */

#ifdef DEBUG
	FPRINTF(LOG_BUG, "%s : pstMEMSINFO = 0x%x\n",__FUNCTION__,(U32) pstMEMSINFO);
	FPRINTF(LOG_BUG, "%s : pstMEMSZONE->uiMemNodeBodySize=%d >= uiSize=%d\n",__FUNCTION__,(U32) pstMEMSZONE->uiMemNodeBodySize,uiSize);
	FPRINTF(LOG_BUG, "%s : pBase = 0x%x\n",__FUNCTION__,(U32) pBase);
#endif

//	while(pstMEMSINFO->uiMemNodeAllocedCnt < pstMEMSINFO->uiMemNodeTotCnt){
	while(1){
		pFree = (U8*) (pBase + pstMEMSZONE->offsetFreeList);
		pstMEMSNODEHDR = (stMEMSNODEHDR *) pFree;
		if( (pstMEMSZONE->offsetFreeList += (stMEMSNODEHDR_SIZE + pstMEMSZONE->uiMemNodeBodySize)) 
				== pstMEMSZONE->offsetNodeEnd){
    		pstMEMSZONE->offsetFreeList = pstMEMSZONE->offsetNodeStart;
		}
#ifdef DEBUG
	FPRINTF(LOG_LEVEL, "%s : pstMEMSINFO = 0x%x pstMEMSNODEHDR=0x%x\n",__FUNCTION__,(U32) pstMEMSINFO ,(U32) pstMEMSNODEHDR);
	FPRINTF(LOG_LEVEL, "%s : pstMEMSINFO->offsetFreeList = 0x%x\n",__FUNCTION__,pstMEMSZONE->offsetFreeList);
	FPRINTF(LOG_LEVEL, "%s : pstMEMSNODEHDR->ucIsFree = 0x%x  MAX %d\n",__FUNCTION__,pstMEMSNODEHDR->ucIsFree,MEMS_MAX_DEBUG_STR);
#endif
			/*pstMEMSNODEHDR->uiID = MEMS_ID; */
		if(MEMS_FREE == pstMEMSNODEHDR->ucIsFree){
			pstMEMSNODEHDR->TimeSec = time(NULL);
			pstMEMSNODEHDR->ucIsFree = MEMS_ALLOCED;
#ifdef NIFO_TEST
			memcpy(pstMEMSNODEHDR->DebugStr,pDbgPtr,MEMS_MAX_DEBUG_STR-1);
			pstMEMSNODEHDR->DebugStr[MEMS_MAX_DEBUG_STR-1] = 0x00;
			pstMEMSNODEHDR->DelStr[0] = 0x00;
//			pstMEMSINFO->uiMemNodeAllocedCnt++;
#endif
			pstMEMSZONE->createCnt++;
			pRet = (U8*) (pFree + stMEMSNODEHDR_SIZE);
			break;
		}
		if(++iDebug > pstMEMSZONE->uiMemNodeTotCnt){
			FPRINTF(LOG_LEVEL,"Err : %s : iDebug %d , TotCnt %d\n",__FUNCTION__,iDebug,pstMEMSZONE->uiMemNodeTotCnt);
			break;
		}

	}

	V(pstMEMSZONE->iSemID, pstMEMSZONE->uiSemFlag);

	return pRet;
}

/** �⺻ mems function : mems_free function. 
 *
 * NODE�� mem���� free�Ѵ�.
 * -DDEBUG  �Ҵ��� �Լ��� �־��ش�.  
 *
 * @li ID�� �����Ͽ� Ȯ���� �Ѵ�.
 * @li Free���� Alloc�� Free�� Ȯ���ϰ� ���� ALLOCED�� set
 * @li ȣ���� �Լ� �̸� ����
 * 
 *
 * @param *pstMEMSINFO	: MEM ������ 
 * @param *pFree		: free�� pointer
 * @param *pDbgPtr		: string pointer for debugging 
 *
 *  @return     success : 0 , fail : not 0
 *  @see        mems.h
 *
 *  @warning 	���࿡ ���� �Ҵ��� �Ϳ� ���ؼ��� ��� �� ���ΰ�?  free�� ���� �����ִ°� �ϴ� ���̴�. (���� �ð��� set�Ͽ� �װ��� �Ѵ� �͵��� �������� �Ϸ翡 �ѹ��� clear�� ��Ų�ٵ��� �ϴ� �۾��� �ʿ��� ���̴�(��������� garbage collection�� �����Ͽ��� �Ѵ�.)   
 *  @note       mems node header��ŭ �ڷ� ���� Ȯ���� �Ͽ� ó�� 
 **/
S32 
mems_free(stMEMSINFO *pstMEMSINFO , U8 *pFree, U8 *pDbgPtr)
{
	stMEMSNODEHDR *pstMEMSNODEHDR;

//	P(pstMEMSINFO->iSemID, pstMEMSINFO->uiSemFlag);

#ifdef DEBUG
	FPRINTF(LOG_LEVEL, "%s : pstMEMSNODE = 0x%x\n",__FUNCTION__,(U32) pstMEMSINFO);
#endif

	pstMEMSNODEHDR = (stMEMSNODEHDR *) (pFree - stMEMSNODEHDR_SIZE);

	if( (MEMS_ID != pstMEMSNODEHDR->uiID)  ||
		(MEMS_ALLOCED != pstMEMSNODEHDR->ucIsFree) ){
#ifdef DEBUG
	   FPRINTF(LOG_LEVEL,"Err %s : uiID 0x%x  , MEMS_ID 0x%x\n",__FUNCTION__,pstMEMSNODEHDR->uiID,MEMS_ID);
	   FPRINTF(LOG_LEVEL,"Err %s : ucIsFree 0x%x  , MEMS_ALLOCED 0x%x\n",__FUNCTION__,pstMEMSNODEHDR->ucIsFree,MEMS_ALLOCED);
	   FPRINTF(LOG_LEVEL,"Err %s : called function name : %s\n",__FUNCTION__,pstMEMSNODEHDR->DebugStr);
#endif
#ifdef JJIN
	   FPRINTF(LOG_LEVEL,"Err %s : uiID 0x%x  , MEMS_ID 0x%x\n",__FUNCTION__,pstMEMSNODEHDR->uiID,MEMS_ID);
	   FPRINTF(LOG_LEVEL,"Err %s : ucIsFree 0x%x  , MEMS_ALLOCED 0x%x\n",__FUNCTION__,pstMEMSNODEHDR->ucIsFree,MEMS_ALLOCED);
	   FPRINTF(LOG_LEVEL,"Err %s : DebugStr[%s] ProcName[%s]\n",__FUNCTION__,pstMEMSNODEHDR->DebugStr, pDbgPtr);
#endif

//		V(pstMEMSINFO->iSemID, pstMEMSINFO->uiSemFlag);
   		return 1;
	}

	/* memset(pstMEMSNODEHDR,0,stMEMSNODEHDR_SIZE); */
	pstMEMSNODEHDR->ucIsFree = MEMS_FREE;
#ifdef NIFO_TEST
//	pstMEMSNODEHDR->DebugStr[0] = 0x00;
	memcpy(pstMEMSNODEHDR->DelStr, pDbgPtr, MEMS_MAX_DEBUG_STR-1);
	pstMEMSNODEHDR->DelStr[MEMS_MAX_DEBUG_STR-1] = 0x00;
//	pstMEMSINFO->uiMemNodeAllocedCnt--;
#endif
	pstMEMSINFO->stMEMSZONE[pstMEMSNODEHDR->uiZoneID].delCnt++;

//	V(pstMEMSINFO->iSemID, pstMEMSINFO->uiSemFlag);

	return 0;
}

/** �⺻ mems function : mems_print_info function. 
 *
 * INFO print
 * 
 * @param *pcPrtPrefixStr	: print prefix string
 * @param *pstMEMSINFO	: MEM ������ 
 *
 *  @return     void
 *  @see        mems.h
 *
 *  @note       �տ��� prefix string�� �־��� (_%s)
 **/
void
mems_print_info(S8 *pcPrtPrefixStr,stMEMSINFO *pstMEMSINFO)
{
	stMEMSINFO_Prt(pcPrtPrefixStr,pstMEMSINFO);
}

/** �⺻ mems function : mems_print_node function. 
 *
 * INFO print
 * 
 * @param *pcPrtPrefixStr	: print prefix string
 * @param *pstMEMSNODEHDR	: MEM NODE pointer
 *
 *  @return     void
 *  @see        mems.h
 *
 *  @note       �տ��� prefix string�� �־��� (_%s)
 **/
void
mems_print_node(S8 *pcPrtPrefixStr,stMEMSNODEHDR *pstMEMSNODEHDR)
{
	stMEMSNODEHDR_Prt(pcPrtPrefixStr,pstMEMSNODEHDR);
}

/** �⺻ mems function : mems_print_all function. 
 *
 * MEM���� ��� data�� ����ش�.
 * 
 * @param *pcPrtPrefixStr	: print prefix string
 * @param *pstMEMSINFO	: MEM ������ 
 *
 *  @return     void
 *  @see        mems.h
 *
 *  @note       �տ��� prefix string�� �־��� (_%s)
 **/
void
mems_print_all(S8 *pcPrtPrefixStr,stMEMSINFO *pstMEMSINFO)
{
	U32	iIndex, i;
	stMEMSNODEHDR *pstMEMSNODEHDR;
	U8 *pBase,*pIndex;
	st_MEMSZONE		*pstMEMSZONE;

	FPRINTF(LOG_LEVEL,"%s ##1: pstMEMSINFO=%p \n",__FUNCTION__, pstMEMSINFO);
	FPRINTF(LOG_LEVEL,"%s ##1: pstMEMSINFO\n",__FUNCTION__);
	mems_print_info(pcPrtPrefixStr,pstMEMSINFO);

	pBase = (U8 *) (pstMEMSINFO) ; 

	for(i = 0; i < pstMEMSINFO->uiZoneCnt; i++)
	{
		pstMEMSZONE = &pstMEMSINFO->stMEMSZONE[i];

		pBase += pstMEMSZONE->offsetNodeStart;
		for(iIndex=0;iIndex < pstMEMSZONE->uiMemNodeTotCnt;iIndex++){
			pIndex = (U8*) (pBase + iIndex * (stMEMSNODEHDR_SIZE + pstMEMSZONE->uiMemNodeBodySize)); 
			pstMEMSNODEHDR = (stMEMSNODEHDR *) pIndex;
#ifdef DEBUG
			FPRINTF(LOG_LEVEL,"%s ##2: %s : [%3d] ID :0x%x pIndex=0x%x\n",pcPrtPrefixStr,__FUNCTION__,iIndex,pstMEMSNODEHDR->uiID,(U32) pIndex);
			FPRINTF(LOG_LEVEL,"%s ##2: offset %d 0x%x\n",__FUNCTION__,iIndex * (stMEMSNODEHDR_SIZE + pstMEMSZONE->uiMemNodeBodySize),iIndex * (stMEMSNODEHDR_SIZE + pstMEMSZONE->uiMemNodeBodySize));
#endif
			if(pstMEMSNODEHDR->ucIsFree == MEMS_ALLOCED){
				FPRINTF(LOG_LEVEL,"%s ##3: %s : [%3d] \n",pcPrtPrefixStr,__FUNCTION__,iIndex);
				mems_print_node(pcPrtPrefixStr,pstMEMSNODEHDR);
			}
		}
	}
}

/** �⺻ mems function : mems_draw_all function. 
 *
 * MEM���� ��� data�� ����ش�.
 * 
 * @param *filename	:  Write�� filename
 * @param *labelname	: label��  
 * @param *pstMEMSINFO	: MEM ������ 
 *
 *  @return     void
 *  @see        mems.h
 *
 *  @note       ������ ���Ϸ� �����ȴ�. 
 **/
void
mems_draw_all(S8 *filename,S8 *labelname,stMEMSINFO *pstMEMSINFO)
{
	U32	iIndex, i;
	stMEMSNODEHDR *pstMEMSNODEHDR;
	FILE *fp;
	U8 *pBase,*pIndex;
	st_MEMSZONE		*pstMEMSZONE;

	fp = fopen(filename,"w");
	FILEPRINT(fp,"/** @file %s\n",filename);
	for(i = 0; i < pstMEMSINFO->uiZoneCnt; i++)
	{
		pstMEMSZONE = &pstMEMSINFO->stMEMSZONE[i];
		FILEPRINT(fp,"ZONE=%d uiMemNodeTotCnt = %d\\n\n", i, pstMEMSZONE->uiMemNodeTotCnt);
		FILEPRINT(fp,"ZONE=%d uiMemNodeAllocedCnt = %d\\n\n", i, pstMEMSZONE->uiMemNodeAllocedCnt);
	}
	FILEPRINT(fp,"\\dot \n	\
	digraph G{ 	\n\
	fontname=Helvetica; 	\n\
	label=\"Hash Table(%s)\"; 	\n\
	nodesep=.05; 	\n\
	rankdir=LR; 	\n\
	node [fontname=Helvetica,shape=record,width=.1,height=.1]; 	\n\
	node0 [label = \"",labelname);

	pBase = (U8 *) (pstMEMSINFO) ; 
	for(i = 0; i < pstMEMSINFO->uiZoneCnt; i++)
	{
		pstMEMSZONE = &pstMEMSINFO->stMEMSZONE[i];
		pBase += pstMEMSZONE->offsetNodeStart;
		for(iIndex=0;iIndex < pstMEMSZONE->uiMemNodeTotCnt;iIndex++){
			pIndex = (U8*) (pBase + iIndex * (stMEMSNODEHDR_SIZE + pstMEMSZONE->uiMemNodeBodySize)); 
			pstMEMSNODEHDR = (stMEMSNODEHDR *) pIndex;
			if(iIndex == pstMEMSZONE->uiMemNodeTotCnt -1){
				FILEPRINT(fp,"<f%d> %d_%d_%s",iIndex,iIndex,pstMEMSNODEHDR->ucIsFree,pstMEMSNODEHDR->DebugStr);
			} else {
				FILEPRINT(fp,"<f%d> %d_%d_%s |",iIndex,iIndex,pstMEMSNODEHDR->ucIsFree,pstMEMSNODEHDR->DebugStr);
			}
		}
	}

	FILEPRINT(fp,"\",height = 2.5];\n");
	FILEPRINT(fp,"node [width=1.5];\n");
	FILEPRINT(fp,"\n\n");
	FILEPRINT(fp,"}\n\\enddot \n\n");
	FILEPRINT(fp,"*/\n");
	fclose(fp);
}

/** �⺻ mems function : mems_garbage_collector function. 
 *
 *  ���� �ð��� ���� �޸𸮸� ���� ���ش�.\n ���� node������ ���� ���
 * 
 * @param *pcPrtPrefixStr	: print prefix string
 * @param *pstMEMSINFO	: MEM ������ 
 * @param timegap	:  �Ҵ� �ð��� ���� �ð��� ���� �� ��ġ
 * @param *print_func	:  print function pointer
 *
 *  @return     void
 *  @see        mems.h
 *
 *  @note       �տ��� prefix string�� �־��� (_%s)
 **/
void
mems_garbage_collector(S8 *pcPrtPrefixStr,stMEMSINFO *pstMEMSINFO,int timegap, void (*print_func)(stMEMSINFO *pmem, stMEMSNODEHDR * pmemhdr))
{
	U32	iIndex, i;
	stMEMSNODEHDR *pstMEMSNODEHDR;
	st_MEMSZONE		*pstMEMSZONE;
	U8 *pBase,*pIndex;
	U32 ga_before_alloced_cnt =0;
	U32 ga_before_free_cnt =0;
	U32 ga_after_alloced_cnt =0;
	U32 ga_after_free_cnt =0;
	U32 ga_changed_cnt =0;
	int cur_time = time(0);

	FPRINTF(LOG_LEVEL,"%s : %s ##1: pstMEMSINFO=%p \n",__FUNCTION__, pcPrtPrefixStr, pstMEMSINFO);
	mems_print_info(pcPrtPrefixStr,pstMEMSINFO);

	pBase = (U8 *) (pstMEMSINFO) ; 
	for(i = 0; i < pstMEMSINFO->uiZoneCnt; i++)
	{
		pstMEMSZONE = &pstMEMSINFO->stMEMSZONE[i];
		pBase += pstMEMSZONE->offsetNodeStart;
		for(iIndex=0;iIndex < pstMEMSZONE->uiMemNodeTotCnt;iIndex++){
			pIndex = (U8*) (pBase + iIndex * (stMEMSNODEHDR_SIZE + pstMEMSZONE->uiMemNodeBodySize)); 
			pstMEMSNODEHDR = (stMEMSNODEHDR *) pIndex;
#ifdef DEBUG
			FPRINTF(LOG_LEVEL,"%s ##2: %s : [%3d] ID :0x%x pIndex=0x%x\n",pcPrtPrefixStr,__FUNCTION__,iIndex,pstMEMSNODEHDR->uiID,(U32) pIndex);
			FPRINTF(LOG_LEVEL,"%s ##2: offset %d 0x%x\n",__FUNCTION__,iIndex * (stMEMSNODEHDR_SIZE + pstMEMSZONE->uiMemNodeBodySize),iIndex * (stMEMSNODEHDR_SIZE + pstMEMSZONE->uiMemNodeBodySize));
#endif
			if(pstMEMSNODEHDR->ucIsFree == MEMS_ALLOCED){
				ga_before_alloced_cnt ++;
				ga_after_alloced_cnt ++;
				if( (pstMEMSNODEHDR->TimeSec + timegap) < cur_time){		/* Free ���Ѿ� ��. */
//					FPRINTF(LOG_BUG,"CHANGED NODE :  %s ##3: %s : [%3d] \n",pcPrtPrefixStr,__FUNCTION__,iIndex);
					FPRINTF(LOG_BUG,"ID[%u]ZONEID[%u]TIME[%u]IsFree[%d]CREATE_NODE[%s]DEL_NODE[%s]CURTIME[%u]IDX[%u]\n",
						pstMEMSNODEHDR->uiID, pstMEMSNODEHDR->uiZoneID, pstMEMSNODEHDR->TimeSec, pstMEMSNODEHDR->ucIsFree,
						pstMEMSNODEHDR->DebugStr, pstMEMSNODEHDR->DelStr, cur_time, iIndex);
					if(print_func != NULL)
						print_func(pstMEMSINFO, pstMEMSNODEHDR);
					mems_print_node(pcPrtPrefixStr,pstMEMSNODEHDR);
					mems_free(pstMEMSINFO , (U8*)(((char *) pstMEMSNODEHDR) + stMEMSNODEHDR_SIZE), (U8*)pcPrtPrefixStr);
					ga_changed_cnt ++;
					ga_after_alloced_cnt --;
					ga_after_free_cnt ++;
				} 
			} else {
				ga_before_free_cnt ++;
				ga_after_free_cnt ++;
			}
		}
	}
}

/** �⺻ mems function : mems_view function. 
 *
 *  mems node ���� ���
 * 
 * @param *pcPrtPrefixStr	: print prefix string
 * @param *pstMEMSINFO	: MEM ������ 
 * @param timegap	:  �Ҵ� �ð��� ���� �ð��� ���� �� ��ġ
 * @param *print_func	:  print function pointer
 *
 *  @return     void
 *  @see        mems.h
 *
 *  @note       �տ��� prefix string�� �־��� (_%s)
 **/
void
mems_view(S8 *pcPrtPrefixStr, stMEMSINFO *pstMEMSINFO, int timegap, void (*print_func)(stMEMSINFO *pmem, stMEMSNODEHDR * pmemhdr))
{
	U32	iIndex, i;
	stMEMSNODEHDR *pstMEMSNODEHDR;
	st_MEMSZONE		*pstMEMSZONE;
	U8 *pBase,*pIndex;
	int cur_time = time(0);

	pBase = (U8 *) (pstMEMSINFO) ; 
	for(i = 0; i < pstMEMSINFO->uiZoneCnt; i++)
	{
		pstMEMSZONE = &pstMEMSINFO->stMEMSZONE[i];
		pBase += pstMEMSZONE->offsetNodeStart;
		for(iIndex=0;iIndex < pstMEMSZONE->uiMemNodeTotCnt;iIndex++){
			pIndex = (U8*) (pBase + iIndex * (stMEMSNODEHDR_SIZE + pstMEMSZONE->uiMemNodeBodySize)); 
			pstMEMSNODEHDR = (stMEMSNODEHDR *) pIndex;

			if(pstMEMSNODEHDR->ucIsFree == MEMS_ALLOCED){
				if( (pstMEMSNODEHDR->TimeSec + timegap) < cur_time){		/* Free ���Ѿ� ��. */
					FPRINTF(LOG_BUG,"ID=%u ZONEID=%u TIME=%u IsFree=%d CREATE_NODE=%s DEL_NODE=%s CURTIME=%u IDX=%u\n",
						pstMEMSNODEHDR->uiID, pstMEMSNODEHDR->uiZoneID, pstMEMSNODEHDR->TimeSec, pstMEMSNODEHDR->ucIsFree,
						pstMEMSNODEHDR->DebugStr, pstMEMSNODEHDR->DelStr, cur_time, iIndex);
					if(print_func != NULL)
						print_func(pstMEMSINFO, pstMEMSNODEHDR);
				} 
			}
		}
	}
}


/** �⺻ mems function : mems_alloced_cnt function. 
 *
 * 	���� ALLOC�� NODE ����
 * 
 * @param *pstMEMSINFO	: MEM ������ 
 *
 *  @return     void
 *  @see        mems.h
 *
 *  @note       �տ��� prefix string�� �־��� (_%s)
 **/
S32
mems_alloced_cnt(stMEMSINFO *pstMEMSINFO, U32 uiZoneID)
{
	S32	allocedCnt = 0;
	U32	iIndex;
	stMEMSNODEHDR *pstMEMSNODEHDR;
	U8 *pBase,*pIndex;
	st_MEMSZONE		*pstMEMSZONE;

	pBase = (U8 *) (pstMEMSINFO) ; 
	pstMEMSZONE = &pstMEMSINFO->stMEMSZONE[uiZoneID];
	pBase += pstMEMSZONE->offsetNodeStart;
	for(iIndex=0; iIndex < pstMEMSZONE->uiMemNodeTotCnt; iIndex++){
		pIndex = (U8*) (pBase + iIndex * (stMEMSNODEHDR_SIZE + pstMEMSZONE->uiMemNodeBodySize)); 
		pstMEMSNODEHDR = (stMEMSNODEHDR *) pIndex;

		if(pstMEMSNODEHDR->ucIsFree == MEMS_ALLOCED){
			allocedCnt++;
		}
	}

	return allocedCnt;
}

#ifdef TEST

#define SHM_KEY_TEST    8001
#define SEMA_KEY_TEST	8100

/** Free �Լ� : mems_shm_free function. 
 *
 *  shared memory ���� �Լ�
 *
 * @param uiShmKey  	: Shared Memory Key (case type is shared Memory)
 * @param uiSize  		: Shared Memory Size
 *
 *  @return     1: ���� ���� , -1: ������ memory �� ã��
 *  @see        mems.h
 *
 *  @exception  ��Ģ�� Ʋ�� ���� ã���ּ���.
 *  @note       ���� Shared Memory�ȿ� �ִ� �͵� ���� �� ��. mem ���� �κ��� �߰� �Ǿ����� ��.
 **/
int
mems_shm_free (int shm_key, int size)
{
	int shm_id;

	if ((shm_id = shmget(shm_key, size, 0666 | IPC_CREAT)) < 0) 
		return -1;
	
	if (shmctl (shm_id, IPC_RMID, (struct shmid_ds *)0) < 0) 
		return -1;

	return 1;
}

/** main function.
 *
 *  Node�� �����ϰ� , �����ϴ� ������ text �� �׸����� �����ش�.  
 *  -DTEST �� ���ؼ� ���Ǿ����� ������ main�� TEST�� ���ǵɶ��� ����
 *  �� ���α׷��� �⺻������ library��. 
 * 
 *  @return     void
 *  @see        mems.h
 *
 *  @note       �׸����δ� ������ file�� �����Ǹ� file���� code���� �Է��ϰ� �Ǿ����ֽ��ϴ�.
 **/
int
main(int argc, char *argv[])
{
	stMEMSINFO *pstMEMSINFO;
	U8 *a,*c,*d,*e,*f,*g,*h,*i;
	

	if (argc == 1) {
		printf ("main mem\n");
		pstMEMSINFO = mems_init(MEMS_MAIN_MEM, 0, MEMS_SEMA_OFF, 0, 128, 32, 8 /**<mems node count */);
	} else {
		if (strcmp (argv [1], "shm") != 0) {
			printf ("main mem\n");
			pstMEMSINFO = mems_init(MEMS_MAIN_MEM, 0, MEMS_SEMA_OFF, 0, 128, 32, 8 /**<mems node count */);
		} else {
			printf ("shm mem\n");
			pstMEMSINFO = mems_init(MEMS_SHARED_MEM, SHM_KEY_TEST, MEMS_SEMA_ON, SEMA_KEY_TEST, 128, 32, 8 /**<mems node count */);
		}
	}


	mems_print_all((S8 *)__FUNCTION__,pstMEMSINFO);
	
	printf("111\n");
	a = mems_alloc(pstMEMSINFO,32,(S8 *)__FUNCTION__);
	if (a == NULL)
		printf ("NULL!!\n");
	mems_print_all((S8 *)__FUNCTION__,pstMEMSINFO);
	mems_draw_all("TEST_RESULT_01.TXT",(S8 *)__FUNCTION__,pstMEMSINFO);
/*
	printf("222 b\n");
	b = mems_alloc(pstMEMSINFO,40,(S8 *)__FUNCTION__);
	mems_print_all((S8 *)__FUNCTION__,pstMEMSINFO);
	mems_draw_all("TEST_RESULT_02.TXT",(S8 *)__FUNCTION__,pstMEMSINFO);
*/
	printf("333 c\n");
	c = mems_alloc(pstMEMSINFO,32,(S8 *)__FUNCTION__);
	if (c == NULL)
		printf ("NULL!!\n");
	mems_print_all((S8 *)__FUNCTION__,pstMEMSINFO);
	mems_draw_all("TEST_RESULT_03.TXT",(S8 *)__FUNCTION__,pstMEMSINFO);

	printf("444 d \n");
	d = mems_alloc(pstMEMSINFO,32,(S8 *)__FUNCTION__);
	if (d == NULL)
		printf ("NULL!!\n");
	mems_print_all((S8 *)__FUNCTION__,pstMEMSINFO);
	mems_print_all((S8 *)__FUNCTION__,pstMEMSINFO);
	mems_draw_all("TEST_RESULT_04.TXT",(S8 *)__FUNCTION__,pstMEMSINFO);

	printf("555 e\n");
	e = mems_alloc(pstMEMSINFO,32,(S8 *)__FUNCTION__);
	if (e == NULL)
		printf ("NULL!!\n");
	mems_print_all((S8 *)__FUNCTION__,pstMEMSINFO);
	mems_draw_all("TEST_RESULT_05.TXT",(S8 *)__FUNCTION__,pstMEMSINFO);

	printf("666 f\n");
	f = mems_alloc(pstMEMSINFO,32,(S8 *)__FUNCTION__);
	if (f == NULL)
		printf ("NULL!!\n");
	mems_print_all((S8 *)__FUNCTION__,pstMEMSINFO);
	mems_draw_all("TEST_RESULT_06.TXT",(S8 *)__FUNCTION__,pstMEMSINFO);


	printf("777 a\n");
	mems_free(pstMEMSINFO,a);
	mems_print_all((S8 *)__FUNCTION__,pstMEMSINFO);
	mems_draw_all("TEST_RESULT_07.TXT",(S8 *)__FUNCTION__,pstMEMSINFO);

	printf("888 d\n");
	mems_free(pstMEMSINFO,d);
	mems_print_all((S8 *)__FUNCTION__,pstMEMSINFO);
	mems_draw_all("TEST_RESULT_08.TXT",(S8 *)__FUNCTION__,pstMEMSINFO);

	printf("999 c\n");
	mems_free(pstMEMSINFO,c);
	mems_print_all((S8 *)__FUNCTION__,pstMEMSINFO);
	mems_draw_all("TEST_RESULT_09.TXT",(S8 *)__FUNCTION__,pstMEMSINFO);

	printf("111 e\n");
	mems_free(pstMEMSINFO,e);
	mems_print_all((S8 *)__FUNCTION__,pstMEMSINFO);
	mems_draw_all("TEST_RESULT_10.TXT",(S8 *)__FUNCTION__,pstMEMSINFO);

	printf("222 f %d\n", mems_free(pstMEMSINFO,f));
	mems_print_all((S8 *)__FUNCTION__,pstMEMSINFO);
	mems_draw_all("TEST_RESULT_11.TXT",(S8 *)__FUNCTION__,pstMEMSINFO);


#if 1
	printf("###222 f\n");
	g = mems_alloc(pstMEMSINFO,32,(S8 *)__FUNCTION__);
	mems_print_all((S8 *)__FUNCTION__,pstMEMSINFO);
	mems_draw_all("TEST_RESULT_12.TXT",(S8 *)__FUNCTION__,pstMEMSINFO);
	printf("###222 f\n");
	h = mems_alloc(pstMEMSINFO,32,(S8 *)__FUNCTION__);
	mems_print_all((S8 *)__FUNCTION__,pstMEMSINFO);
	mems_draw_all("TEST_RESULT_13.TXT",(S8 *)__FUNCTION__,pstMEMSINFO);
	printf("###222 f\n");
	i = mems_alloc(pstMEMSINFO,32,(S8 *)__FUNCTION__);
	mems_print_all((S8 *)__FUNCTION__,pstMEMSINFO);
	mems_draw_all("TEST_RESULT_14.TXT",(S8 *)__FUNCTION__,pstMEMSINFO);
	printf("###222 f\n");
	g = mems_alloc(pstMEMSINFO,32,(S8 *)__FUNCTION__);
	mems_print_all((S8 *)__FUNCTION__,pstMEMSINFO);
	mems_draw_all("TEST_RESULT_15.TXT",(S8 *)__FUNCTION__,pstMEMSINFO);
	printf("###222 f\n");
	h = mems_alloc(pstMEMSINFO,32,(S8 *)__FUNCTION__);
	mems_print_all((S8 *)__FUNCTION__,pstMEMSINFO);
	mems_draw_all("TEST_RESULT_16.TXT",(S8 *)__FUNCTION__,pstMEMSINFO);
	printf("###222 f\n");
	i = mems_alloc(pstMEMSINFO,32,(S8 *)__FUNCTION__);
	mems_print_all((S8 *)__FUNCTION__,pstMEMSINFO);
	mems_draw_all("TEST_RESULT_17.TXT",(S8 *)__FUNCTION__,pstMEMSINFO);

	if (argc == 3) {
		if (strcmp (argv [2], "del") == 0) 
			mems_shm_free (SHM_KEY_TEST, pstMEMSINFO->uiTotMemSize);
	} 
#endif

	return 0;
}
#endif /* TEST */

/** file mems.c
 *     $Log: mems.c,v $
 *     Revision 1.3  2011/08/31 04:26:05  uamyd
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
 *     Revision 1.1.1.1  2011/01/11 01:33:02  jjinri
 *     DIFO
 *
 *     Revision 1.5  2010/12/28 08:35:39  swpark
 *     zone matrix setting
 *
 *     Revision 1.4  2010/12/10 05:49:26  dark264sh
 *     nifo dAppLog ���� �� MEMS_SHM_SIZE ��� �߰�
 *
 *     Revision 1.3  2010/12/09 08:58:12  jjinri
 *     MEMS_SHM_SIZE
 *
 *     Revision 1.2  2010/12/09 06:32:02  jjinri
 *     *** empty log message ***
 *
 *     Revision 1.1  2010/12/06 06:47:18  jjinri
 *     nifo zone library
 *
 *     Revision 1.5  2010/11/24 17:37:00  upst_cvs
 *     ..
 *
 *     Revision 1.4  2010/11/24 05:14:52  upst_cvs
 *     JJIN
 *
 *     Revision 1.3  2010/11/24 04:30:31  upst_cvs
 *     1124
 *
 *     Revision 1.2  2010/10/18 12:14:06  upst_cvs
 *     ..
 *
 *     Revision 1.1.1.1  2010/10/15 06:54:37  upst_cvs
 *     WTAS2
 *
 *     Revision 1.6  2009/10/11 08:26:31  pkg
 *     mems mems_view LOG_LEVEL => LOG_BUG ��ȯ
 *
 *     Revision 1.5  2009/08/15 08:28:05  pkg
 *     MODIFIED DEFINE NIFO_TEST
 *
 *     Revision 1.4  2009/08/14 14:36:47  pkg
 *     ADD DEFINE NIFO_TEST
 *
 *     Revision 1.3  2009/06/28 14:51:38  dark264sh
 *     *** empty log message ***
 *
 *     Revision 1.2  2009/06/28 08:49:06  dark264sh
 *     *** empty log message ***
 *
 *     Revision 1.1  2009/06/10 16:45:50  dqms
 *     *** empty log message ***
 *
 *     Revision 1.1.1.1  2009/05/26 02:13:36  dqms
 *     Init TAF_RPPI
 *
 *     Revision 1.3  2008/11/17 11:07:49  dark264sh
 *     *** empty log message ***
 *
 *     Revision 1.2  2008/11/17 08:57:29  dark264sh
 *     64bits �۾�
 *
 *     Revision 1.1.1.1  2008/06/09 08:17:20  jsyoon
 *     WATAS3 PROJECT START
 *
 *     Revision 1.2  2007/08/29 07:43:02  dark264sh
 *     *** empty log message ***
 *
 *     Revision 1.1  2007/08/21 12:20:10  dark264sh
 *     no message
 *
 *     Revision 1.22  2006/12/04 08:05:06  dark264sh
 *     *** empty log message ***
 *
 *     Revision 1.21  2006/11/28 00:49:24  cjlee
 *     doxygen
 *
 *     Revision 1.20  2006/11/08 07:47:13  dark264sh
 *     *** empty log message ***
 *
 *     Revision 1.19  2006/11/06 07:27:26  dark264sh
 *     nifo NODE size 4*1024 => 6*1024�� �����ϱ�
 *     nifo_tlv_alloc���� argument�� memset���� ���� �����ϵ��� ����
 *     nifo_node_free���� semaphore ����
 *
 *     Revision 1.18  2006/10/23 11:33:12  dark264sh
 *     *** empty log message ***
 *
 *     Revision 1.17  2006/10/18 10:55:45  dark264sh
 *     *** empty log message ***
 *
 *     Revision 1.16  2006/10/18 08:54:23  dark264sh
 *     nifo debug �ڵ� �߰�
 *
 *     Revision 1.15  2006/10/18 02:23:07  dark264sh
 *     free�� alloced �Ҵ�� ���°� �ƴϸ� �׵��� ó��
 *
 *     Revision 1.14  2006/10/12 07:34:01  dark264sh
 *     lib ������ FPRINTF(LOG_LEVEL, ó���� ����
 *
 *     Revision 1.13  2006/09/27 02:49:07  cjlee
 *     *** empty log message ***
 *
 *     Revision 1.12  2006/09/27 02:47:01  cjlee
 *     mem_garbage_collector ()  �߰�
 *
 *     Revision 1.11  2006/09/11 07:42:38  dark264sh
 *     no message
 *
 *     Revision 1.10  2006/09/11 07:36:10  dark264sh
 *     no message
 *
 *     Revision 1.9  2006/08/31 01:09:14  dark264sh
 *     test main���� ������� �ʴ� ���� *b ����
 *
 *     Revision 1.8  2006/08/31 01:08:06  dark264sh
 *     test main ���� __FUNCTION__ => (S8 *)__FUNCTION__���� ����
 *
 *     Revision 1.7  2006/08/31 01:02:57  dark264sh
 *     test main ����
 *
 *     Revision 1.6  2006/08/07 05:54:23  dark264sh
 *     no message
 *
 *     Revision 1.5  2006/08/07 05:51:46  dark264sh
 *     no message
 *
 *     Revision 1.4  2006/08/07 05:50:39  dark264sh
 *     no message
 *
 *     Revision 1.3  2006/08/07 05:49:22  dark264sh
 *     no message
 *
 *     Revision 1.2  2006/08/04 12:08:15  dark264sh
 *     no message
 *
 *     Revision 1.1  2006/08/03 11:58:37  dark264sh
 *     no message
 *
 *     Revision 1.1  2006/08/01 08:21:37  dark264sh
 *     no message
 *
 *     Revision 1.11  2006/05/08 08:19:21  yhshin
 *     mems_shm_free ����
 *
 *     Revision 1.10  2006/04/26 08:02:07  yhshin
 *     null check ����
 *
 *     Revision 1.9  2006/04/26 07:53:58  yhshin
 *     shared memory �߰�
 *
 *     Revision 1.8  2006/04/26 05:31:04  yhshin
 *     *** empty log message ***
 *
 *     Revision 1.7  2006/04/26 05:22:26  yhshin
 *     bug ����
 *
 *     Revision 1.6  2006/04/26 04:37:43  yhshin
 *     shared memory �߰�
 *
 *     Revision 1.5  2006/04/20 07:43:11  cjlee
 *     minor change: DEBUG ����
 *
 *     Revision 1.4  2006/04/20 00:37:51  cjlee
 *     minor change : draw dot print ���� - main node �߰�
 *
 *     Revision 1.3  2006/03/17 07:30:05  cjlee
 *     minor change
 *
 *     Revision 1.2  2006/03/17 07:29:10  cjlee
 *     minor change
 *
 *     Revision 1.1  2006/03/17 07:23:14  cjlee
 *     INIT
 *
 *
 *     */
