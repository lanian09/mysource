/**
 *	Include headers
 */
// TOP
#include "common_stg.h"
#include "commdef.h"
#include "capdef.h"

// LIB
#include "mems.h"
#include "cifo.h"
#include "nifo.h"
#include "gifo.h"
#include "Analyze_Ext_Abs.h"
#include "loglib.h"
#include "utillib.h"

// .
#include "prea_frag.h"

/**
 *	Declare func.
 */
extern void invoke_del_ipfrag(void *p);

/**
 *	Implement func.
 */
OFFSET ip_frag_sort(stMEMSINFO *pMEMSINFO, U8 *pHEAD, U32 node_cnt, U8 *pNODE, U16 ipfrag_offset)
{
	U16			ipfrag_offset_prev;
	S32			i;
	U8			*pPREV;
	OFFSET		offset;
	INFO_ETH	*pINFOETH;

	pPREV = pHEAD;

	for(i = 0; i < node_cnt; i++)
	{
		pPREV = (U8 *)nifo_entry(nifo_ptr(pMEMSINFO, ((NIFO *)pPREV)->nont.offset_prev), NIFO, nont);
		offset = nifo_offset(pMEMSINFO, pPREV);
		pINFOETH = (INFO_ETH *)nifo_get_value(pMEMSINFO, INFO_ETH_NUM, offset);

		ipfrag_offset_prev = (pINFOETH->stIP.usIPFrag & IP_FRAG_OFFSET) * 8;
		
		if(ipfrag_offset > ipfrag_offset_prev)
		{
			nifo_node_link_nont_next(pMEMSINFO, pPREV, pNODE);
			return nifo_offset(pMEMSINFO, pHEAD);
		} 
	}

	nifo_node_link_nont_prev(pMEMSINFO, pHEAD, pNODE);
	return nifo_offset(pMEMSINFO, pNODE);
}

S32 ip_frag_merge(stMEMSINFO *pMEMSINFO, U8 *pHEAD, U32 node_cnt, U8 *pRST)
{
	S32					i, dataLen, len;
	U8					*pNEXT;
	TLV					*tlv;
	NIFO				*pNIFO;
	OFFSET				offset;
	Capture_Header_Msg	*pCAPHEAD, *pRCAPHEAD;
	INFO_ETH			*pINFOETH, *pRINFOETH;
	U8					*pDATA, *pTMP;

	pNEXT = pHEAD;

	offset = nifo_offset(pMEMSINFO, pHEAD);
	pCAPHEAD = (Capture_Header_Msg *)nifo_get_value(pMEMSINFO, CAP_HEADER_NUM, offset);
	pINFOETH = (INFO_ETH *)nifo_get_value(pMEMSINFO, INFO_ETH_NUM, offset);
	pDATA = (U8 *)nifo_get_value(pMEMSINFO, ETH_DATA_NUM, offset);

	pNIFO = (NIFO *)pRST;

	if((pTMP = (U8 *)nifo_tlv_alloc(pMEMSINFO, pRST, CAP_HEADER_NUM, CAP_HRD_LEN, DEF_MEMSET_OFF)) == NULL) 
	{
		log_print(LOGN_CRI, LH"nifo_tlv_alloc NULL", LT);
		return -1;
    }
	memcpy(pTMP, pCAPHEAD, CAP_HRD_LEN);

	pRCAPHEAD = (Capture_Header_Msg *)pTMP;

	if((pTMP = (U8 *)nifo_tlv_alloc(pMEMSINFO, pRST, INFO_ETH_NUM, INFO_ETH_SIZE, DEF_MEMSET_OFF)) == NULL) 
	{
		log_print(LOGN_CRI, LH"nifo_tlv_alloc NULL", LT);
		return -2;
    }
	memcpy(pTMP, pINFOETH, INFO_ETH_SIZE);

	pRINFOETH = (INFO_ETH *)pTMP;

	if((pTMP = (U8 *)nifo_tlv_alloc(pMEMSINFO, pRST, ETH_DATA_NUM, pCAPHEAD->datalen, DEF_MEMSET_OFF)) == NULL) 
	{
		log_print(LOGN_CRI, LH"nifo_tlv_alloc NULL", LT);
		return -3;
    }
	memcpy(pTMP, pDATA, pCAPHEAD->datalen);

	tlv = (TLV *)((char *)pTMP - TLV_SIZE);	
	len = pINFOETH->stUDPTCP.wDataLen;

	for(i = 1; i < node_cnt; i++)
	{
		pNEXT = (U8 *)nifo_entry(nifo_ptr(pMEMSINFO, ((NIFO *)pNEXT)->nont.offset_next), NIFO, nont);
		offset = nifo_offset(pMEMSINFO, pNEXT);
		pINFOETH = (INFO_ETH *)nifo_get_value(pMEMSINFO, INFO_ETH_NUM, offset);
		pDATA = (U8 *)nifo_get_value(pMEMSINFO, ETH_DATA_NUM, offset);

		pDATA = pDATA + DEF_MAC_SIZE + pINFOETH->stIP.wIPHeaderLen;
		dataLen = pINFOETH->stIP.wTotalLength - pINFOETH->stIP.wIPHeaderLen;

		if(pNIFO->maxoffset < pNIFO->lastoffset + dataLen) {
			log_print(LOGN_CRI, LH"OVER FLOW NODE datalen=%d copylen=%d lastoffset=%ld",
					LT, tlv->len, dataLen, pNIFO->lastoffset);
			return -4;
		}

		pNIFO->lastoffset += dataLen;

		memcpy(pTMP + tlv->len, pDATA, dataLen);
		tlv->len += dataLen;
		len += dataLen;
	}

	pRINFOETH->stUDPTCP.wDataLen = len;
	pRCAPHEAD->datalen = len;

	return 0;
}

S32 ip_frag_check_finish(stMEMSINFO *pMEMSINFO, U8 *pHEAD, U32 node_cnt)
{
	S32					i, total_size;
	U16					ipfrag_offset;
	U8					*pNEXT;
	OFFSET				offset;
	INFO_ETH			*pINFOETH;

	pNEXT = pHEAD;

	offset = nifo_offset(pMEMSINFO, pHEAD);
	pINFOETH = (INFO_ETH *)nifo_get_value(pMEMSINFO, INFO_ETH_NUM, offset);

	ipfrag_offset = (pINFOETH->stIP.usIPFrag & IP_FRAG_OFFSET) * 8;
	total_size = pINFOETH->stIP.wTotalLength - pINFOETH->stIP.wIPHeaderLen;

	if(ipfrag_offset != 0) {
		log_print(LOGN_INFO, "NOT HAVE START PACKET OFFSET[%d]", ipfrag_offset);
		return -1;
	}

	for(i = 1; i < node_cnt; i++)
	{
		pNEXT = (U8 *)nifo_entry(nifo_ptr(pMEMSINFO, ((NIFO *)pNEXT)->nont.offset_next), NIFO, nont);
		offset = nifo_offset(pMEMSINFO, pNEXT);
		pINFOETH = (INFO_ETH *)nifo_get_value(pMEMSINFO, INFO_ETH_NUM, offset);

		ipfrag_offset = (pINFOETH->stIP.usIPFrag & IP_FRAG_OFFSET) * 8;

		if(ipfrag_offset != total_size) {
			log_print(LOGN_INFO, "NOT HAVE SEQ=%d/%d PACKET OFFSET[%d] != SIZE[%d]", i, node_cnt, ipfrag_offset, total_size);
			return -2;
		}

		total_size += (pINFOETH->stIP.wTotalLength - pINFOETH->stIP.wIPHeaderLen);
	}

	return 1;
}

U8 *ip_frag(stMEMSINFO *pMEMSINFO, stHASHOINFO *pIPFRAGHASH, stTIMERNINFO *pIPFRAGTIMER, INFO_ETH *pINFOETH, U8 *pNODE)
{
	S32				rst;
	U16				ipfrag_offset, ipfrag_more;
	U8				*pHEAD, *pRST;
	IP_FRAG_KEY		IPFRAGKEY;
	IP_FRAG_KEY		*pIPFRAGKEY = &IPFRAGKEY;
	IP_FRAG			IPFRAG;
	IP_FRAG			*pIPFRAG = &IPFRAG;
	stHASHONODE		*pHASHNODE;
	IP_FRAG_COMMON	IPFRAGCOMMON;
	IP_FRAG_COMMON	*pIPFRAGCOMMON = &IPFRAGCOMMON;

    U8				szSIP[INET_ADDRSTRLEN];
    U8				szDIP[INET_ADDRSTRLEN];

	/* Make Hash Key */
	pIPFRAGKEY->sip = pINFOETH->stIP.dwSrcIP;
	pIPFRAGKEY->dip = pINFOETH->stIP.dwDestIP;
	pIPFRAGKEY->identification = pINFOETH->stIP.usIdent;

	ipfrag_offset = (pINFOETH->stIP.usIPFrag & IP_FRAG_OFFSET) * 8;
	ipfrag_more = (pINFOETH->stIP.usIPFrag & IP_FRAG_MORE);

	log_print(LOGN_INFO, "IP FRAG SIP=%s DIP=%s ID=%X:%d OFFSET=%d MORE=%d", 
			util_cvtipaddr(szSIP, pINFOETH->stIP.dwSrcIP), util_cvtipaddr(szDIP, pINFOETH->stIP.dwDestIP),
			pINFOETH->stIP.usIdent, pINFOETH->stIP.usIdent, 
			ipfrag_offset, ipfrag_more);

	/* search hash */
	if((pHASHNODE = hasho_find(pIPFRAGHASH, (U8 *)pIPFRAGKEY)) == NULL)
	{
		log_print(LOGN_INFO, "IP FRAG NEW SIP=%s DIP=%s ID=%X:%d",
				util_cvtipaddr(szDIP, pINFOETH->stIP.dwSrcIP), util_cvtipaddr(szDIP, pINFOETH->stIP.dwDestIP),
				pINFOETH->stIP.usIdent, pINFOETH->stIP.usIdent);
		memcpy(&pIPFRAGCOMMON->IPFRAGKEY, pIPFRAGKEY, IP_FRAG_KEY_SIZE);

		pIPFRAG->cnt = 1;
		if(ipfrag_more == 0) {
			pIPFRAG->finish = DEF_IPFRAG_FINISH;
			log_print(LOGN_INFO, "IP FRAG NEW END PACKET SIP=%s DIP=%s ID=%X:%d",
					util_cvtipaddr(szSIP, pINFOETH->stIP.dwSrcIP), util_cvtipaddr(szDIP, pINFOETH->stIP.dwDestIP),
					pINFOETH->stIP.usIdent, pINFOETH->stIP.usIdent);
		} else {
			pIPFRAG->finish = DEF_IPFRAG_NOT_FINISH;
		}
		pIPFRAG->offset_NODE = nifo_offset(pMEMSINFO, pNODE);
		pHEAD = pNODE;

		if((pHASHNODE = hasho_add(pIPFRAGHASH, (U8 *)pIPFRAGKEY, (U8 *)pIPFRAG)) == NULL)
		{
			log_print(LOGN_CRI, LH"SIP=%s DIP=%s ID=%X:%d hasho_add NULL ???", 
					LT,
					util_cvtipaddr(szSIP, pINFOETH->stIP.dwSrcIP), util_cvtipaddr(szDIP, pINFOETH->stIP.dwDestIP),
					pINFOETH->stIP.usIdent, pINFOETH->stIP.usIdent);
			nifo_node_delete(pMEMSINFO, pNODE);
			return NULL;
		}
		else
		{
			pIPFRAG = (IP_FRAG *)nifo_ptr(pIPFRAGHASH, pHASHNODE->offset_Data);
			pIPFRAG->timerNID = timerN_add(pIPFRAGTIMER, invoke_del_ipfrag, (U8 *)pIPFRAGCOMMON, IP_FRAG_COMMON_SIZE, time(NULL) + DEF_IP_FRAG_TIMEOUT);
		}
	}
	else
	{
		pIPFRAG = (IP_FRAG *)nifo_ptr(pIPFRAGHASH, pHASHNODE->offset_Data);

		log_print(LOGN_INFO, "IP FRAG HAVE SIP=%s DIP=%s ID=%X:%d CNT=%d",
				util_cvtipaddr(szSIP, pINFOETH->stIP.dwSrcIP), util_cvtipaddr(szDIP, pINFOETH->stIP.dwDestIP),
				pINFOETH->stIP.usIdent, pINFOETH->stIP.usIdent, pIPFRAG->cnt);

		if(ipfrag_more == 0) {
			pIPFRAG->finish = DEF_IPFRAG_FINISH;
			log_print(LOGN_INFO, "IP FRAG HAVE END PACKET SIP=%s DIP=%s ID=%X:%d",
					util_cvtipaddr(szSIP, pINFOETH->stIP.dwSrcIP), util_cvtipaddr(szDIP, pINFOETH->stIP.dwDestIP),
					pINFOETH->stIP.usIdent, pINFOETH->stIP.usIdent);
		}
		pHEAD = nifo_ptr(pMEMSINFO, pIPFRAG->offset_NODE);
		pIPFRAG->offset_NODE = ip_frag_sort(pMEMSINFO, pHEAD, pIPFRAG->cnt, pNODE, ipfrag_offset);
		pIPFRAG->cnt++;
		pHEAD = nifo_ptr(pMEMSINFO, pIPFRAG->offset_NODE);

		/* update timer */
		pIPFRAG->timerNID = timerN_update(pIPFRAGTIMER, pIPFRAG->timerNID, time(NULL) + DEF_IP_FRAG_TIMEOUT);
	}

	if((pIPFRAG->finish == DEF_IPFRAG_FINISH) && (pIPFRAG->cnt > 1))
	{
		if((rst = ip_frag_check_finish(pMEMSINFO, pHEAD, pIPFRAG->cnt)) > 0)
		{
			log_print(LOGN_INFO, "IP FRAG FINISH PACKET SIP=%s DIP=%s ID=%X:%d",
					util_cvtipaddr(szSIP, pINFOETH->stIP.dwSrcIP), util_cvtipaddr(szDIP, pINFOETH->stIP.dwDestIP),
					pINFOETH->stIP.usIdent, pINFOETH->stIP.usIdent);

			if((pRST = nifo_node_alloc(pMEMSINFO)) == NULL)
			{
				log_print(LOGN_CRI, LH"SIP=%s DIP=%s ID=%X:%d nifo_node_alloc NULL", 
						LT,
						util_cvtipaddr(szSIP, pINFOETH->stIP.dwSrcIP), util_cvtipaddr(szDIP, pINFOETH->stIP.dwDestIP),
						pINFOETH->stIP.usIdent, pINFOETH->stIP.usIdent);
			}
			else if((rst = ip_frag_merge(pMEMSINFO, pHEAD, pIPFRAG->cnt, pRST)) < 0)
			{
				log_print(LOGN_CRI, LH"SIP=%s DIP=%s ID=%X:%d ip_frag_merge dRet=%d", 
						LT, 
						util_cvtipaddr(szSIP, pINFOETH->stIP.dwSrcIP), util_cvtipaddr(szDIP, pINFOETH->stIP.dwDestIP),
						pINFOETH->stIP.usIdent, pINFOETH->stIP.usIdent,
						rst);
				nifo_node_delete(pMEMSINFO, pRST);
				pRST = NULL;
			}

			nifo_node_delete(pMEMSINFO, pHEAD);
			hasho_del(pIPFRAGHASH, (U8 *)pIPFRAGKEY);
			timerN_del(pIPFRAGTIMER, pIPFRAG->timerNID);
			return pRST;
		} 
		else 
		{
			log_print(LOGN_CRI, LH"SIP=%s DIP=%s ID=%X:%d ip_frag_check_finish dRet=%d", 
					LT, 
					util_cvtipaddr(szSIP, pINFOETH->stIP.dwSrcIP), util_cvtipaddr(szDIP, pINFOETH->stIP.dwDestIP),
					pINFOETH->stIP.usIdent, pINFOETH->stIP.usIdent,
					rst);
/*
			nifo_node_delete(pMEMSINFO, pHEAD);
			hasho_del(pIPFRAGHASH, (U8 *)pIPFRAGKEY);
			timerN_del(pIPFRAGTIMER, pIPFRAG->timerNID);
			return NULL;
*/
		}
	}
	return NULL;
}

/*
 * $Log: prea_frag.c,v $
 * Revision 1.2  2011/09/06 12:46:43  hhbaek
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 * NEW OAM SYSTEM
 *
 * Revision 1.1  2011/08/21 14:04:34  uamyd
 * added prea
 *
 * Revision 1.1.1.1  2011/08/05 00:27:17  uamyd
 * init DQMS2
 *
 * Revision 1.2  2011/01/11 04:09:12  uamyd
 * modified
 *
 * Revision 1.1.1.1  2010/08/23 01:12:59  uamyd
 * DQMS With TOTMON, 2nd-import
 *
 * Revision 1.1  2009/07/07 08:47:32  dqms
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2009/05/26 02:14:47  dqms
 * Init TAF_RPPI
 *
 * Revision 1.2  2009/01/28 06:51:49  dark264sh
 * PRE_A ip fragmentation 버그 수정
 *
 * Revision 1.1  2008/09/18 07:28:44  dark264sh
 * IM 서비스 추가 (SIP, XCAP, MSRP)
 *
 * Revision 1.1.1.1  2007/12/27 08:17:41  uamyd
 * import
 *
 * Revision 1.1.1.1  2007/10/22 07:50:43  jsyoon
 * AQUA3 Project = AQUA2 + IMS Protocol + Packet Dump
 *
 * Revision 1.9  2007/05/28 08:42:43  jsyoon
 * IP Fragmentation 처리 버그 수정
 *
 * Revision 1.8  2007/05/28 07:30:04  dark264sh
 * *** empty log message ***
 *
 * Revision 1.7  2007/05/27 09:06:56  dark264sh
 * *** empty log message ***
 *
 * Revision 1.6  2007/05/27 06:22:47  dark264sh
 * *** empty log message ***
 *
 * Revision 1.5  2007/05/24 09:22:06  dark264sh
 * *** empty log message ***
 *
 * Revision 1.4  2007/05/04 03:23:55  dark264sh
 * *** empty log message ***
 *
 * Revision 1.3  2007/04/25 10:56:19  dark264sh
 * NFO_ETH, Capture_Header_Msg 삭제, st_PKT_INFO 추가에 따른 변경
 *
 * Revision 1.2  2007/02/16 06:50:32  dark264sh
 * ip fragmentation 처리
 *
 * Revision 1.1  2007/02/15 04:22:37  dark264sh
 * *** empty log message ***
 *
 */
