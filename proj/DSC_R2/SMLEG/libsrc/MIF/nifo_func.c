/** @file nifo_func.c
 * 	Interface Library file.
 *
 *	$Id: nifo_func.c,v 1.1.1.1 2011/04/19 14:13:42 june Exp $
 *
 *	Copyright (c) 2006~ by Upresto Inc, Korea
 *	All rights reserved.
 * 
 *	@Author      $Author: june $
 *	@version     $Revision: 1.1.1.1 $
 *	@date        $Date: 2011/04/19 14:13:42 $
 *	@ref         nifo.c
 *	@warning     nothing
 *	@todo        nothing
 *
 *	@section     Intro(소개)
 *		- Interface library 파일  (shared memory, message queue, offset linked list 개념으로 작성)
 *		- hasho_init으로 초기화를 한후에 기본적인 primitives 를 이용하게 함
 *		- primitives
 *			@li nifo_get_point_all, nifo_get_point_cont, nifo_get_value, nifo_get_tlv, nifo_get_tlv_all nifo_read_tlv_all
 *			@li nifo_node_for_each_start, nifo_node_for_each_end
 *			@li nifo_node_unlink_nont, nifo_node_unlink_cont
 *			@li	nifo_node_free nifo_node_delete
 *			@li Supplement: Nothing
 *
 *	@section     Requirement
 *		@li 규칙에 틀린 곳을 찾아주세요.
 *
 **/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>

#include "utillib.h"
#include "nifo.h"

#define WIDTH   16
int nifo_dump_DebugString(char *debug_str, char *s, int len)
{
	char buf[BUFSIZ],lbuf[BUFSIZ],rbuf[BUFSIZ];
	unsigned char *p;
	int line,i;

	FPRINTF(LOG_LEVEL,"### %s\n",debug_str);
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
		FPRINTF(LOG_LEVEL,"%04x: %-*s    %s\n",line - 1,WIDTH * 3,lbuf,rbuf);
	}
	return line;
}

/** nifo_get_offset_node function. 
 *
 *  해당 Pointer를 가지고 NODE 시작 OFFSET을 찾는 함수 
 *
 * 	@param *pMEMSINFO: 메모리 관리 정보
 *  @param *ptr: NODE 내부의 Pointer
 *
 *  @return     OFFSET	(NODE 시작 OFFSET)
 *  @see        nifo.h nifo.c
 *
 *  @exception  규칙에 틀린 곳을 찾아주세요.
 *  @note       Nothing
 **/
OFFSET nifo_get_offset_node(stMEMSINFO *pMEMSINFO, U8 *ptr)
{
	OFFSET base;
	S32	cnt;

	base = nifo_offset(pMEMSINFO, ptr);

	base = base - (stMEMSINFO_SIZE + pMEMSINFO->uiHeadRoomSize);	

	cnt = base / (pMEMSINFO->uiMemNodeHdrSize + pMEMSINFO->uiMemNodeBodySize);

	base = stMEMSINFO_SIZE + pMEMSINFO->uiHeadRoomSize + (pMEMSINFO->uiMemNodeHdrSize + pMEMSINFO->uiMemNodeBodySize) * cnt + pMEMSINFO->uiMemNodeHdrSize;

	return base;
}

/** nifo_node_check function. 
 *
 *  NODE들이 유효한지를 판단하는 함수 
 *
 * 	@param *pMEMSINFO: 메모리 관리 정보
 *  @param offset: OFFSET of HEADER NODE 
 *
 *  @return     S32		SUCC: NODE의 개수, FAIL: less 0 
 *  @see        nifo.h nifo.c
 *
 *  @exception  규칙에 틀린 곳을 찾아주세요.
 *  @note       Nothing
 **/
S32 nifo_node_check(stMEMSINFO *pMEMSINFO, OFFSET offset)
{
	S32				nontcnt = 0, contcnt = 0;
	U8				*p;
    clist_head      *pNont, *pCont;
    NIFO            *pNode, *pSNode, *pTmp;
	stMEMSNODEHDR	*pMEMSNODEHDR;
	
	p = nifo_ptr(pMEMSINFO, offset);
    pTmp = (NIFO *)p;

    nifo_node_for_each_start(pMEMSINFO, pNont, &pTmp->nont) {
        pNode = clist_entry(pNont, NIFO, nont);
		nontcnt++;
        nifo_node_for_each_start(pMEMSINFO, pCont, &pNode->cont) {
            pSNode = clist_entry(pCont, NIFO, cont);
			contcnt++;
			pMEMSNODEHDR = (stMEMSNODEHDR *)(((U8 *)pSNode) - stMEMSNODEHDR_SIZE);
			if((MEMS_ID != pMEMSNODEHDR->uiID) || (MEMS_ALLOCED != pMEMSNODEHDR->ucIsFree)) {
				FPRINTF(LOG_BUG, "ALLOC PROCESS[%s] FREE PROCESS[%s] ID[%u][%u]TIME[%u]ISFREE[%d] NONTCNT[%d] CONTCNT[%d]", 
					pMEMSNODEHDR->DebugStr, pMEMSNODEHDR->DelStr, pMEMSNODEHDR->uiID, MEMS_ID,
					pMEMSNODEHDR->TimeSec, pMEMSNODEHDR->ucIsFree, nontcnt, contcnt);
				return -1;
			}

			memcpy(pMEMSNODEHDR->DelStr, procName, MAX_NIFO_PROCNAME_LEN);
			pMEMSNODEHDR->DelStr[MAX_NIFO_PROCNAME_LEN] = 0x00;
			pMEMSNODEHDR->TimeSec = time(NULL);

        } nifo_node_for_each_end(pMEMSINFO, pCont, &pNode->cont)
    } nifo_node_for_each_end(pMEMSINFO, pNont, &pTmp->nont)

	return contcnt;
}

/** nifo_get_tlv function. 
 *
 * 	해당 NODE에 대한 해당 TYPE의 TLV의 Pointer를 찾아서 리턴 해주는 함수
 * 
 * 	@param *pMEMSINFO: 메모리 관리 정보
 * 	@param type: 받을 구조체 TYPE 정보
 * 	@param offset: header list offset 값
 *
 *  @return     U8 *	SUCC: 해당 Pointer, FAIL: NULL
 *  @see        nifo.h nifo.c
 *
 *  @exception  규칙에 틀린 곳을 찾아주세요.
 *  @note       Nothing
 **/
U8 *nifo_get_tlv(stMEMSINFO *pMEMSINFO, U32 type, OFFSET offset)
{
	U8			*pNode;
	NIFO		*pNIFO;
	TLV			*pTLV;
	OFFSET		curoffset;

	pNode = nifo_ptr(pMEMSINFO, offset);
	pNIFO = (NIFO *)pNode;
	curoffset = NIFO_SIZE;

	while(curoffset < pNIFO->lastoffset) {
		pTLV = (TLV *)(pNode + curoffset);
		if(pTLV->type == type) {
			return (pNode + curoffset);
		}
		curoffset += (TLV_SIZE + pTLV->len);
	}

	return NULL;
}

/** nifo_get_point_cont function. 
 *
 * 	linked list 에 들어온 모든 구조체 값의 Pointer를 찾아서 리턴 해주는 함수 (cont)
 * 
 * 	@param *pstMEMSINFO: 메모리 관리 정보
 * 	@param *pREADVALLIST: 받은 구조체 정보
 * 	@param offset: header list offset 값
 *
 *  @return     S32
 *  @see        nifo.h nifo.c
 *
 *  @exception  규칙에 틀린 곳을 찾아주세요.
 *  @note       Nothing
 **/
S32 nifo_get_point_cont(stMEMSINFO *pstMEMSINFO, READ_VAL_LIST *pREADVALLIST, OFFSET offset)
{
	S32				i;
	U32 			dOffset, dOldLen, dIdx, dLen, dCopySize;
	U8				*pBuf, *pSetNode;
	NIFO 			*pRcv;
	NIFO			*pNode, *pSNode;
	TLV				*pTLV;

	pRcv = (NIFO *)nifo_ptr(pstMEMSINFO, offset);

	pNode = clist_entry(pRcv, NIFO, nont);

	dOffset = NIFO_SIZE;

	pSNode = pNode;
	pSetNode = (U8 *)pSNode;
				
	for(i = 0; i < pSNode->cnt; i++) {
		U8 szBuf[8];
		if(dOffset + TLV_SIZE > DEF_MEMNODEBODY_SIZE) {
			pBuf = szBuf;
			dOldLen = 0;
			dLen = TLV_SIZE;
			while(dOldLen < dLen) {
				if((dOffset + dLen - dOldLen) > DEF_MEMNODEBODY_SIZE) {
					dCopySize = DEF_MEMNODEBODY_SIZE - dOffset;
					memcpy(pBuf+dOldLen, pSetNode+dOffset, dCopySize);
					pSNode = clist_entry(nifo_ptr(pstMEMSINFO, (pSNode->cont).offset_next), NIFO, cont); 
					pSetNode = (U8 *)pSNode;
					dOldLen += dCopySize;
					dOffset = (dOffset + dCopySize + NIFO_SIZE) % DEF_MEMNODEBODY_SIZE;
				} else {
					dCopySize = dLen - dOldLen;
					memcpy(pBuf+dOldLen, pSetNode+dOffset, dCopySize);
					dOldLen += dCopySize;
					dOffset += dCopySize;
				}
			}
				
			pTLV = (TLV *)szBuf;
		} else {
			pTLV = (TLV *)(pSetNode+dOffset);
			dOffset += TLV_SIZE;
		}

		dIdx = pTLV->type;
		dLen = pTLV->len;
	
		if(dOffset + dLen > DEF_MEMNODEBODY_SIZE) {
			if(pREADVALLIST->READVAL[dIdx].init == DEF_READ_ON) {
				if((pBuf = malloc(dLen + 1)) == NULL) {
					return -errno;
				}
				pREADVALLIST->READVAL[dIdx].pVal = pBuf;
				pREADVALLIST->READVAL[dIdx].memtype = DEF_READ_MALLOC;
				pREADVALLIST->READVAL[dIdx].len = dLen;

				dOldLen = 0;
				while(dOldLen < dLen) {
					if((dOffset + dLen - dOldLen) > DEF_MEMNODEBODY_SIZE) {
						dCopySize = DEF_MEMNODEBODY_SIZE - dOffset;
						memcpy(pBuf+dOldLen, pSetNode+dOffset, dCopySize);
						pSNode = clist_entry(nifo_ptr(pstMEMSINFO, (pSNode->cont).offset_next), NIFO, cont); 
						pSetNode = (U8 *)pSNode;
						dOldLen += dCopySize;
						dOffset = (dOffset + dCopySize + NIFO_SIZE) % DEF_MEMNODEBODY_SIZE;
					} else {
						dCopySize = dLen - dOldLen;
						memcpy(pBuf+dOldLen, pSetNode+dOffset, dCopySize);
						dOldLen += dCopySize;
						dOffset += dCopySize;
					}

				}
					
			} else {
				dOldLen = 0;
				while(dOldLen < dLen) {
					if((dOffset + dLen - dOldLen) > DEF_MEMNODEBODY_SIZE) {
						dCopySize = DEF_MEMNODEBODY_SIZE - dOffset;
						pSNode = clist_entry(nifo_ptr(pstMEMSINFO, (pSNode->cont).offset_next), NIFO, cont); 
						pSetNode = (U8 *)pSNode;
						dOldLen += dCopySize;
						dOffset = (dOffset + dCopySize + NIFO_SIZE) % DEF_MEMNODEBODY_SIZE;
					} else {
						dCopySize = dLen - dOldLen;
						dOldLen += dCopySize;
						dOffset += dCopySize;
					}

				}
			}
		} else {
			if(pREADVALLIST->READVAL[dIdx].init == DEF_READ_ON) {
				pREADVALLIST->READVAL[dIdx].memtype = DEF_READ_ORIGIN;
				pREADVALLIST->READVAL[dIdx].pVal = pSetNode+dOffset;
				pREADVALLIST->READVAL[dIdx].len = dLen;
			}
			dOffset += dLen;
		}

	}

	return 1;
}

/** nifo_get_point_all function. 
 *
 * 	linked list 에 들어온 모든 구조체 값의 Pointer를 찾아서 리턴 해주는 함수 (nont, cont)
 * 
 * 	@param *pstMEMSINFO: 메모리 관리 정보
 * 	@param *pREADVALLIST: 받은 구조체 정보
 * 	@param offset: header list offset 값
 *
 *  @return     S32
 *  @see        nifo.h nifo.c
 *
 *  @exception  규칙에 틀린 곳을 찾아주세요.
 *  @note       Nothing
 **/
S32 nifo_get_point_all(stMEMSINFO *pstMEMSINFO, READ_VAL_LIST *pREADVALLIST, OFFSET offset)
{
	S32				dRet;
	NIFO 			*pRcv;
	clist_head		*pNont;
	NIFO			*pNode;

	pRcv = (NIFO *)nifo_ptr(pstMEMSINFO, offset);

	nifo_node_for_each_start(pstMEMSINFO, pNont, &pRcv->nont) {
		pNode = clist_entry(pNont, NIFO, nont);
		if((dRet = nifo_get_point_cont(pstMEMSINFO, pREADVALLIST, nifo_offset(pstMEMSINFO, pNode))) < 0) {
			return dRet;
		}
	} nifo_node_for_each_end(pstMEMSINFO, pNont, &pRcv->nont)

	return 1;
}

/** nifo_get_tlv_cont function. 
 *
 * 	linked list 에 들어온 모든 구조체 값의 Pointer를 찾아서 리턴 해주는 함수 (cont)
 * 
 * 	@param *pstMEMSINFO: 메모리 관리 정보
 * 	@param offset: header list offset 값
 * 	@param *exec_func: 처리 function pointer
 * 	@param *out: exec_func 내부 처리 이후 결과를 받을 void pointer
 *
 *  @return     S32
 *  @see        nifo.h nifo.c
 *
 *  @exception  규칙에 틀린 곳을 찾아주세요.
 *  @note       Nothing
 **/
S32 nifo_get_tlv_cont(stMEMSINFO *pstMEMSINFO, OFFSET offset, S32 (*exec_func)(U32 type, U32 len, U8 *data, S32 memflag, void *out), void *out)
{
	S32				i, dRet;
	U32 			dOffset, dOldLen, dIdx, dLen, dCopySize;
	U8				*pBuf, *pSetNode;
	NIFO 			*pRcv;
	NIFO			*pNode, *pSNode;
	TLV				*pTLV;

	pRcv = (NIFO *)nifo_ptr(pstMEMSINFO, offset);

	pNode = clist_entry(pRcv, NIFO, nont);

	dOffset = NIFO_SIZE;

	pSNode = pNode;
	pSetNode = (U8 *)pSNode;
				
	for(i = 0; i < pSNode->cnt; i++) {
		U8 szBuf[8];
		if(dOffset + TLV_SIZE > DEF_MEMNODEBODY_SIZE) {
			pBuf = szBuf;
			dOldLen = 0;
			dLen = TLV_SIZE;
			while(dOldLen < dLen) {
				if((dOffset + dLen - dOldLen) > DEF_MEMNODEBODY_SIZE) {
					dCopySize = DEF_MEMNODEBODY_SIZE - dOffset;
					memcpy(pBuf+dOldLen, pSetNode+dOffset, dCopySize);
					pSNode = clist_entry(nifo_ptr(pstMEMSINFO, (pSNode->cont).offset_next), NIFO, cont); 
					pSetNode = (U8 *)pSNode;
					dOldLen += dCopySize;
					dOffset = (dOffset + dCopySize + NIFO_SIZE) % DEF_MEMNODEBODY_SIZE;
				} else {
					dCopySize = dLen - dOldLen;
					memcpy(pBuf+dOldLen, pSetNode+dOffset, dCopySize);
					dOldLen += dCopySize;
					dOffset += dCopySize;
				}
			}
				
			pTLV = (TLV *)szBuf;
		} else {
			pTLV = (TLV *)(pSetNode+dOffset);
			dOffset += TLV_SIZE;
		}

		dIdx = pTLV->type;
		dLen = pTLV->len;
	
		if(dOffset + dLen > DEF_MEMNODEBODY_SIZE) {
			if((pBuf = malloc(dLen + 1)) == NULL) {
				return -errno;
			}

			dOldLen = 0;
			while(dOldLen < dLen) {
				if((dOffset + dLen - dOldLen) > DEF_MEMNODEBODY_SIZE) {
					dCopySize = DEF_MEMNODEBODY_SIZE - dOffset;
					memcpy(pBuf+dOldLen, pSetNode+dOffset, dCopySize);
					pSNode = clist_entry(nifo_ptr(pstMEMSINFO, (pSNode->cont).offset_next), NIFO, cont); 
					pSetNode = (U8 *)pSNode;
					dOldLen += dCopySize;
					dOffset = (dOffset + dCopySize + NIFO_SIZE) % DEF_MEMNODEBODY_SIZE;
				} else {
					dCopySize = dLen - dOldLen;
					memcpy(pBuf+dOldLen, pSetNode+dOffset, dCopySize);
					dOldLen += dCopySize;
					dOffset += dCopySize;
				}

			}
					

			if((dRet = exec_func(pTLV->type, pTLV->len, pBuf, DEF_READ_MALLOC, out)) < 0) {
				free(pBuf);
				return dRet;
			}
			free(pBuf);

		} else {
			if((dRet = exec_func(pTLV->type, pTLV->len, pSetNode+dOffset, DEF_READ_ORIGIN, out)) < 0)
				return dRet;

			dOffset += dLen;
		}

	}

	return 1;
}


/** nifo_get_tlv_all function. 
 *
 * 	linked list 에 들어온 모든 구조체 값의 Pointer를 찾아서 리턴 해주는 함수 (nont, cont)
 * 
 * 	@param *pstMEMSINFO	: 메모리 관리 정보
 * 	@param offset: header list offset 값
 * 	@param *exec_func: 처리 function pointer
 * 	@param *out: exec_func 내부 처리 이후 결과를 받을 void pointer
 *
 *  @return     S32
 *  @see        nifo.h nifo.c
 *
 *  @exception  규칙에 틀린 곳을 찾아주세요.
 *  @note       Nothing
 **/
S32 nifo_get_tlv_all(stMEMSINFO *pstMEMSINFO, OFFSET offset, S32 (*exec_func)(U32 type, U32 len, U8 *data, S32 memflag, void *out), void *out)
{
	S32				dRet;
	NIFO 			*pRcv;
	clist_head		*pNont;
	NIFO			*pNode;

	pRcv = (NIFO *)nifo_ptr(pstMEMSINFO, offset);

	nifo_node_for_each_start(pstMEMSINFO, pNont, &pRcv->nont) {
		pNode = clist_entry(pNont, NIFO, nont);
		if((dRet = nifo_get_tlv_cont(pstMEMSINFO, nifo_offset(pstMEMSINFO, pNode), exec_func, out)) < 0) {
			return dRet;
		}
	} nifo_node_for_each_end(pstMEMSINFO, pNont, &pRcv->nont)

	return 1;
}

/** nifo_read_tlv_cont function. 
 *
 * 	linked list 에 들어온 모든 구조체 값의 Pointer를 찾아서 리턴 해주는 함수 (cont)
 * 
 * 	@param *pMEMSINFO: 메모리 관리 정보
 * 	@param *pHEAD: HEADER NODE
 * 	@param *type: TLV type
 * 	@param *len: TLV length
 * 	@param **value: TLV value
 * 	@param *ismalloc: malloc된 메오리인지 아닌지 판단 (DEF_READ_MALLOC, DEF_READ_ORGIN)
 * 	@param **nexttlv: Next TLV Pointer
 *
 *  @return     S32		SUCC: 1, FAIL: less 0
 *  @see        nifo.h nifo.c
 *
 *  @exception  규칙에 틀린 곳을 찾아주세요.
 *  @note       Nothing
 **/
S32 nifo_read_tlv_cont(stMEMSINFO *pMEMSINFO, U8 *pHEAD, U32 *type, U32 *len, U8 **value, S32 *ismalloc, U8 **nexttlv)
{
	U32 			dOffset, dOldLen, dLen, dCopySize;
	U8				*pBuf, *pSetNode;
	NIFO			*pSNode;
	TLV				*pTLV;
	OFFSET			base, gap;

	base = nifo_get_offset_node(pMEMSINFO, *nexttlv);

	if((gap = nifo_offset(pMEMSINFO, *nexttlv) - base) == 0) {
		dOffset = NIFO_SIZE;
	} else {
		dOffset = gap;
	}

	pSNode = (NIFO *)nifo_ptr(pMEMSINFO, base);
	pSetNode = (U8 *)pSNode;
				
	U8 szBuf[8];
	if(dOffset + TLV_SIZE > DEF_MEMNODEBODY_SIZE) {
		pBuf = szBuf;
		dOldLen = 0;
		dLen = TLV_SIZE;
		while(dOldLen < dLen) {
			if((dOffset + dLen - dOldLen) > DEF_MEMNODEBODY_SIZE) {
				dCopySize = DEF_MEMNODEBODY_SIZE - dOffset;
				memcpy(pBuf+dOldLen, pSetNode+dOffset, dCopySize);
				pSNode = clist_entry(nifo_ptr(pMEMSINFO, (pSNode->cont).offset_next), NIFO, cont); 
				pSetNode = (U8 *)pSNode;
				dOldLen += dCopySize;
				dOffset = (dOffset + dCopySize + NIFO_SIZE) % DEF_MEMNODEBODY_SIZE;
			} else {
				dCopySize = dLen - dOldLen;
				memcpy(pBuf+dOldLen, pSetNode+dOffset, dCopySize);
				dOldLen += dCopySize;
				dOffset += dCopySize;
			}
		}
				
		pTLV = (TLV *)szBuf;
	} else {
		pTLV = (TLV *)(pSetNode+dOffset);
		dOffset += TLV_SIZE;
	}

	*type = pTLV->type;
	*len = pTLV->len;
	dLen = pTLV->len;
	
	if(dOffset + dLen > DEF_MEMNODEBODY_SIZE) {
		if((pBuf = malloc(dLen + 1)) == NULL) {
			return -errno;
		}

		dOldLen = 0;
		while(dOldLen < dLen) {
			if((dOffset + dLen - dOldLen) > DEF_MEMNODEBODY_SIZE) {
				dCopySize = DEF_MEMNODEBODY_SIZE - dOffset;
				memcpy(pBuf+dOldLen, pSetNode+dOffset, dCopySize);
				pSNode = clist_entry(nifo_ptr(pMEMSINFO, (pSNode->cont).offset_next), NIFO, cont); 
				pSetNode = (U8 *)pSNode;
				dOldLen += dCopySize;
				dOffset = (dOffset + dCopySize + NIFO_SIZE) % DEF_MEMNODEBODY_SIZE;
			} else {
				dCopySize = dLen - dOldLen;
				memcpy(pBuf+dOldLen, pSetNode+dOffset, dCopySize);
				dOldLen += dCopySize;
				dOffset += dCopySize;
			}
		}
			

		*value = pBuf;
		*ismalloc = DEF_READ_MALLOC;

		if(pSNode->lastoffset <= dOffset) {
			if(pSNode->lastoffset == pSNode->maxoffset) {
				pSetNode = (U8 *)clist_entry(nifo_ptr(pMEMSINFO, (pSNode->cont).offset_next), NIFO, cont); 
				if(pHEAD == pSetNode) {
					*nexttlv = NULL;		
				} else {
					*nexttlv = pSetNode;
				}
			} else {
				*nexttlv = NULL;
			}
		}
		else {
			*nexttlv = nifo_ptr(pMEMSINFO, nifo_offset(pMEMSINFO, pSNode) + dOffset);
		}	

	} else {

		*value = pSetNode+dOffset;
		*ismalloc = DEF_READ_ORIGIN;

		dOffset += dLen;

		if(pSNode->lastoffset <= dOffset) {
			if(pSNode->lastoffset == pSNode->maxoffset) {
				pSetNode = (U8 *)clist_entry(nifo_ptr(pMEMSINFO, (pSNode->cont).offset_next), NIFO, cont); 
				if(pHEAD == pSetNode) {
					*nexttlv = NULL;		
				}
				else {
					*nexttlv = pSetNode;
				}
			} else {
				*nexttlv = NULL;
			}
		}
		else {
			*nexttlv = nifo_ptr(pMEMSINFO, nifo_offset(pMEMSINFO, pSNode) + dOffset);
		}
	}

	return 1;
}

/** nifo_copy_tlv_cont function. 
 *
 * 	TLV로 nifo NODE에 Data를 복사 해주는 함수
 * 
 * 	@param *pMEMSINFO: 메모리 관리 정보
 * 	@param type: TLV type
 * 	@param len: TLV length
 * 	@param *value: TLV value
 * 	@param *node: nifo NODE
 *
 *  @return     S32		SUCC: 0, FAIL: less 0
 *  @see        nifo.h nifo.c
 *
 *  @exception  규칙에 틀린 곳을 찾아주세요.
 *  @note       node는 반드시 HEAD NODE 이어야 함.
 **/
S32 nifo_copy_tlv_cont(stMEMSINFO *pMEMSINFO, U32 type, U32 len, U8 *value, U8 *node)
{
	U8				*p, *pNode, *pSubNode;
	TLV				aTLV;
	TLV				*pTLV;
	NIFO			*pNIFO;
	S32				copySize, remainSize, oldLen, copyLen;

	pNIFO = (NIFO *)node;
	pNode = (U8 *)clist_entry(nifo_ptr(pMEMSINFO, (pNIFO->cont).offset_prev), NIFO, cont); 
	pNIFO = (NIFO *)pNode;

	if(pNIFO->maxoffset < pNIFO->lastoffset + TLV_SIZE + len)
	{
		pTLV = &aTLV;
		pTLV->type = type;
		pTLV->len = len;

		if(pNIFO->maxoffset < pNIFO->lastoffset + TLV_SIZE)
		{
			copySize = pNIFO->maxoffset - pNIFO->lastoffset;
			remainSize = TLV_SIZE - copySize;

			p = nifo_ptr(pMEMSINFO, (nifo_offset(pMEMSINFO, pNode) + pNIFO->lastoffset));
			pNIFO->lastoffset += copySize;
			memcpy(p, (U8 *)pTLV, copySize);
			
			if((pSubNode = nifo_node_alloc(pMEMSINFO)) == NULL)
			{
				return -1;
			}

			clist_add_head(pMEMSINFO, &((NIFO *)pSubNode)->cont, &((NIFO *)pNode)->cont);			

			pNode = pSubNode;
			pNIFO = (NIFO *)pNode;

			p = nifo_ptr(pMEMSINFO, (nifo_offset(pMEMSINFO, pNode) + pNIFO->lastoffset));
			pNIFO->lastoffset += remainSize;
			memcpy(p, (U8 *)pTLV + copySize, remainSize);
		}
		else
		{
			p = nifo_ptr(pMEMSINFO, (nifo_offset(pMEMSINFO, pNode) + pNIFO->lastoffset));
			pNIFO->lastoffset += TLV_SIZE;
			memcpy(p, pTLV, TLV_SIZE);
		}

		if(pNIFO->maxoffset < pNIFO->lastoffset + len)
		{
			oldLen = 0;

			while(oldLen < len)
			{
				copyLen = len - oldLen;	

				remainSize = pNIFO->maxoffset - pNIFO->lastoffset;
				copySize = ((copyLen - remainSize) > 0) ? remainSize : copyLen;

				p = nifo_ptr(pMEMSINFO, (nifo_offset(pMEMSINFO, pNode) + pNIFO->lastoffset));
				pNIFO->lastoffset += copySize;
				memcpy(p, (U8 *)value, copySize);
				oldLen += copySize;
			
				if((copyLen - remainSize) > 0)
				{
					if((pSubNode = nifo_node_alloc(pMEMSINFO)) == NULL)
					{
						return -2;
					}

					clist_add_head(pMEMSINFO, &((NIFO *)pSubNode)->cont, &((NIFO *)pNode)->cont);			

					pNode = pSubNode;
					pNIFO = (NIFO *)pNode;
				}
			}
		}
		else
		{
			p = nifo_ptr(pMEMSINFO, (nifo_offset(pMEMSINFO, pNode) + pNIFO->lastoffset));
			pNIFO->lastoffset += len;
			memcpy(p, value, len);
		}
	}
	else
	{
		pTLV = (TLV *)nifo_ptr(pMEMSINFO, (nifo_offset(pMEMSINFO, pNode) + pNIFO->lastoffset));
		pTLV->type = type;
		pTLV->len = len;

		p = (U8 *)pTLV;
		pNIFO->lastoffset += (TLV_SIZE + len);	
		memcpy(p + TLV_SIZE, value, len);
	}

	return 0;
}

/** nifo_splice_nont function. 
 *
 * 	join two lists 
 * 
 * 	@param *pstMEMSINFO: 메모리 관리 정보
 * 	@param *pLIST: 추가 하려는 LIST 
 * 	@param *pHEAD: 기준 LIST head 
 *
 *  @return    	void 
 *  @see        nifo.h nifo.c
 *
 *  @exception  규칙에 틀린 곳을 찾아주세요.
 *  @note       Nothing
 **/
void nifo_splice_nont(stMEMSINFO *pMEMSINFO, U8 *pLIST, U8 *pHEAD)
{
	clist_splice(pMEMSINFO, &((NIFO *)pLIST)->nont, &((NIFO *)pHEAD)->nont);
}

/** nifo_splice_cont function. 
 *
 * 	join two lists 
 * 
 * 	@param *pstMEMSINFO: 메모리 관리 정보
 * 	@param *pLIST: 추가 하려는 LIST 
 * 	@param *pHEAD: 기준 LIST head 
 *
 *  @return    	void 
 *  @see        nifo.h nifo.c
 *
 *  @exception  규칙에 틀린 곳을 찾아주세요.
 *  @note       Nothing
 **/
void nifo_splice_cont(stMEMSINFO *pMEMSINFO, U8 *pLIST, U8 *pHEAD)
{
	clist_splice(pMEMSINFO, &((NIFO *)pLIST)->cont, &((NIFO *)pHEAD)->cont);
}

/** nifo_cont_delete function. 
 *
 * 	NODE를 unlink, free 하는 함수 (cont) 
 * 
 * 	@param *pMEMSINFO: 메모리 관리 정보
 * 	@param *pDel: Delete하려는 Pointer of NODE 
 *
 *  @return     U8 *		Next NODE
 *  @see        nifo.h nifo.c
 *
 *  @exception  규칙에 틀린 곳을 찾아주세요.
 *  @note       Nothing
 **/
U8 *nifo_cont_delete(stMEMSINFO *pMEMSINFO, U8 *pDel)
{
	U8 *pNext;

	pNext = (U8 *)clist_entry(nifo_ptr(pMEMSINFO, ((NIFO *)pDel)->cont.offset_next), NIFO, cont);

	nifo_node_unlink_cont(pMEMSINFO, pDel);

	if(pNext == pDel)
		pNext = NULL;

	nifo_node_free(pMEMSINFO, pDel);

	return pNext;
}

/** nifo_print_info function. 
 *
 * 	INFO print
 * 
 * 	@param *pcPrtPrefixStr: print prefix string
 * 	@param *pstMEMSINFO: 메모리 관리 정보 
 *
 *  @return     void
 *  @see        nifo.h nifo.c
 *
 *  @note       앞에는 prefix string을 넣어줌 (_%s)
 **/
void nifo_print_info(S8 *pcPrtPrefixStr, stMEMSINFO *pstMEMSINFO)
{
	stMEMSINFO_Prt(pcPrtPrefixStr, pstMEMSINFO);
}

/** nifo_print_nont function. 
 *
 * 	nont NODE를 따라 가면서 원하는 print_func을 호출해준다. print_func이 NULL인 경우 기본 정보를 찍는다.
 * 
 * 	@param *pstMEMSINFO: 메모리 관리 정보 
 * 	@param *p: Header Node 
 * 	@param *print_func: print 함수
 * 	@param *PrefixStr: Debug String
 *
 *  @return     void
 *  @see        nifo.h nifo.c
 *
 **/
void nifo_print_nont(stMEMSINFO *pstMEMSINFO, U8 *p, void (*print_func)(stMEMSINFO *pmem, U8 *pnode, U8 *str), U8 *PrefixStr)
{
	clist_head		*pNont;
	NIFO			*pNode, *pTmp;

	pTmp = (NIFO *)p;

	nifo_node_for_each_start(pstMEMSINFO, pNont, &pTmp->nont) {
		pNode = clist_entry(pNont, NIFO, nont);	
		if(print_func == NULL) {
			FPRINTF(LOG_LEVEL, 
				"[%s][%s.%d] [%s]## NONT NODE OFFSET[%lu] nont->prev[%lu] nont->next[%lu] cont->prev[%lu] cont->next[%lu] FROM[%d] TO[%d] LASTOFFSET[%ld] MAXOFFSET[%ld]\n",
				__FILE__, __FUNCTION__, __LINE__, PrefixStr,
				nifo_offset(pstMEMSINFO, pNode), 
				(pNode->nont).offset_prev, (pNode->nont).offset_next,
			  	(pNode->cont).offset_prev, (pNode->cont).offset_next,
				pNode->from, pNode->to, pNode->lastoffset, pNode->maxoffset);	
		} else {
			print_func(pstMEMSINFO, (U8 *)pNode, PrefixStr);
		}
	} nifo_node_for_each_end(pstMEMSINFO, pNont, &pTmp->nont)
}

/** nifo_print_cont function. 
 *
 * 	cont NODE를 따라 가면서 원하는 print_func을 호출해준다. print_func이 NULL인 경우 기본 정보를 찍는다.
 * 
 * 	@param *pstMEMSINFO: 메모리 관리 정보 
 * 	@param *p: Header Node 
 * 	@param *print_func: print 함수
 * 	@param *PrefixStr: Debug String
 *
 *  @return     void
 *  @see        nifo.h nifo.c
 *
 **/
void nifo_print_cont(stMEMSINFO *pstMEMSINFO, U8 *p, void (*print_func)(stMEMSINFO *pmem, U8 *pnode, U8 *str), U8 *PrefixStr)
{
	clist_head		*pCont;
	NIFO			*pNode, *pTmp;

	pTmp = (NIFO *)p;

	nifo_node_for_each_start(pstMEMSINFO, pCont, &pTmp->cont) {
		pNode = clist_entry(pCont, NIFO, cont);	
		if(print_func == NULL) {
			FPRINTF(LOG_LEVEL, 
				"[%s][%s.%d] [%s]## CONT NODE OFFSET[%lu] nont->prev[%lu] nont->next[%lu] cont->prev[%lu] cont->next[%lu] FROM[%d] TO[%d] LASTOFFSET[%ld] MAXOFFSET[%ld]\n",
				__FILE__, __FUNCTION__, __LINE__, PrefixStr,
				nifo_offset(pstMEMSINFO, pNode), 
				(pNode->nont).offset_prev, (pNode->nont).offset_next,
			  	(pNode->cont).offset_prev, (pNode->cont).offset_next,
				pNode->from, pNode->to, pNode->lastoffset, pNode->maxoffset);	
		} else {
			print_func(pstMEMSINFO, (U8 *)pNode, PrefixStr);
		}
	} nifo_node_for_each_end(pstMEMSINFO, pCont, &pTmp->cont)
}

/** 기본 nifo function : nifo_print_node function. 
 *
 * 	NIFO안의 모든 data를 찍어준다.
 * 
 * 	@param *pstMEMSINFO: 메모리 관리 정보 
 * 	@param *p : Header Node 
 *
 *  @return     void
 *  @see        nifo.h nifo.c
 *
 **/
void nifo_print_node(stMEMSINFO *pstMEMSINFO, U8 *p)
{
	clist_head		*pNont, *pCont;
	NIFO			*pNode, *pSNode, *pTmp;

	pTmp = (NIFO *)p;

	nifo_node_for_each_start(pstMEMSINFO, pNont, &pTmp->nont) {
		pNode = clist_entry(pNont, NIFO, nont);	
		nifo_node_for_each_start(pstMEMSINFO, pCont, &pNode->cont) {
			pSNode = clist_entry(pCont, NIFO, cont);
			FPRINTF(LOG_LEVEL, "## CONT OFFSET[%lu] nont->prev[%lu] nont->next[%lu] cont->prev[%lu] cont->next[%lu] FROM[%d] TO[%d]\n",
				nifo_offset(pstMEMSINFO, pSNode), 
				(pSNode->nont).offset_prev, (pSNode->nont).offset_next,
			  	(pSNode->cont).offset_prev, (pSNode->cont).offset_next,
				pSNode->from, pSNode->to);	
		} nifo_node_for_each_end(pstMEMSINFO, pCont, &pNode->cont)
	} nifo_node_for_each_end(pstMEMSINFO, pNont, &pTmp->nont)
}

/** nifo_draw_all function. 
 *
 * 	MEM안의 모든 data를 찍어준다.
 * 
 * 	@param *filename:  Write할 filename
 * 	@param *labelname: label명  
 * 	@param *pstMEMSINFO: MEM 관리자 
 *
 *  @return     void
 *  @see        nifo.h nifo.c
 *
 *  @note       개개의 파일로 생성된다. 
 **/
void nifo_draw_all(S8 *filename, S8 *labelname, stMEMSINFO *pstMEMSINFO)
{
	U32	iIndex;
	stMEMSNODEHDR *pstMEMSNODEHDR;
	FILE *fp;
	U8 *pBase,*pIndex;

	pBase = (U8 *) (pstMEMSINFO) ; 
	pBase += pstMEMSINFO->offsetNodeStart;

	fp = fopen(filename,"w");
	FILEPRINT(fp,"/** @file %s\n",filename);
	FILEPRINT(fp,"uiMemNodeTotCnt = %d\\n\n",pstMEMSINFO->uiMemNodeTotCnt);
	FILEPRINT(fp,"uiMemNodeAllocedCnt = %d\\n\n",pstMEMSINFO->uiMemNodeAllocedCnt);
	FILEPRINT(fp,"\\dot \n	\
	digraph G{ 	\n\
	fontname=Helvetica; 	\n\
	label=\"Hash Table(%s)\"; 	\n\
	nodesep=.05; 	\n\
	rankdir=LR; 	\n\
	node [fontname=Helvetica,shape=record,width=.1,height=.1]; 	\n\
	node0 [label = \"",labelname);
	for(iIndex=0;iIndex < pstMEMSINFO->uiMemNodeTotCnt;iIndex++){
		pIndex = (U8*) (pBase + iIndex * (stMEMSNODEHDR_SIZE + pstMEMSINFO->uiMemNodeBodySize)); 
		pstMEMSNODEHDR = (stMEMSNODEHDR *) pIndex;
		if(iIndex == pstMEMSINFO->uiMemNodeTotCnt -1){
			FILEPRINT(fp,"<f%d> %d_%d_%s",iIndex,iIndex,pstMEMSNODEHDR->ucIsFree,pstMEMSNODEHDR->DebugStr);
		} else {
			FILEPRINT(fp,"<f%d> %d_%d_%s |",iIndex,iIndex,pstMEMSNODEHDR->ucIsFree,pstMEMSNODEHDR->DebugStr);
		}
	}
	FILEPRINT(fp,"\",height = 2.5];\n");
	FILEPRINT(fp,"node [width=1.5];\n");
	FILEPRINT(fp,"\n\n");
	FILEPRINT(fp,"}\n\\enddot \n\n");
	FILEPRINT(fp,"*/\n");
	fclose(fp);
}

