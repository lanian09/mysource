#ifndef __PRE_A_FRAG_H__
#define __PRE_A_FRAG_H__

/**
 *	Include headers
 */
// LIB
#include "typedef.h"
#include "mems.h"
#include "Analyze_Ext_Abs.h"

/**
 *	Define cons.
 */
#define DEF_MAC_SIZE				14
#define IP_FRAG_HASH_CNT			107
#define IP_FRAG_MORE				0x2000
#define IP_FRAG_OFFSET				0x1FFF
#define DEF_IP_FRAG_TIMEOUT			3

#define DEF_IPFRAG_NOT_FINISH		0
#define DEF_IPFRAG_FINISH			1

#define DEF_IPFRAG_END				1
#define DEF_IPFRAG_MID				2
#define DEF_IPFRAG_FAIL				3

#pragma pack(1)

/**
 *	Define structures
 */
typedef struct _st_IP_FRAG_KEY {
	IP4				sip;				/**< Source IP */
	IP4				dip;				/**< Destination IP */
	U16				identification;
} IP_FRAG_KEY, *pIP_FRAG_KEY;

#define IP_FRAG_KEY_SIZE		sizeof(IP_FRAG_KEY)

typedef struct _st_IP_FRAG {
	U64				timerNID;
	U16				cnt;
	U16				finish;
	OFFSET 			offset_NODE;
} IP_FRAG, *pIP_FRAG;
#define IP_FRAG_SIZE			sizeof(IP_FRAG)

typedef struct _st_IP_FRAG_Common {
	IP_FRAG_KEY		IPFRAGKEY;
} IP_FRAG_COMMON, *pIP_FRAG_COMMON;
#define IP_FRAG_COMMON_SIZE		sizeof(IP_FRAG_COMMON)

/**
 *	Declare func.
 */
extern OFFSET ip_frag_sort(stMEMSINFO *pMEMSINFO, U8 *pHEAD, U32 node_cnt, U8 *pNODE, U16 ipfrag_offset);
extern S32 ip_frag_merge(stMEMSINFO *pMEMSINFO, U8 *pHEAD, U32 node_cnt, U8 *pRST);
extern S32 ip_frag_check_finish(stMEMSINFO *pMEMSINFO, U8 *pHEAD, U32 node_cnt);
extern U8 *ip_frag(stMEMSINFO *pMEMSINFO, stHASHOINFO *pIPFRAGHASH, stTIMERNINFO *pIPFRAGTIMER, INFO_ETH *pINFOETH, U8 *pNODE);

#endif /* __PRE_A_FRAG_H__ */
