#ifndef _SIPT_FUNC_H_
#define _SIPT_FUNC_H_

/**
 *	Include headers
 */
// LIB
#include "typedef.h"
#include "mems.h"

/**
 * Define structures
 */
typedef struct _st_SIP_INFO_KEY {
	U8			CallID[SIP_CALLID_SIZE];	/**< Call-ID */
	U8			Reserved1[2];
	U8			RTX;
	U32			CSeq;						/**< [CSeq: 264 INVITE] 에서 264 부분 */
	U8			SeqType;
	U8			FromTag[SIP_TAG_SIZE];		/**< From 필드의 tag */
	U8			Reserved2[6];
	U32			ClientIP;					/**< Client IP */
	U32			ServerIP;					/**< Server IP */
} SIP_INFO_KEY, *pSIP_INFO_KEY;
#define SIP_INFO_KEY_SIZE			sizeof(SIP_INFO_KEY)


typedef struct _st_SIP_COMMON {
	SIP_INFO_KEY	SIPINFOKEY;
} SIP_COMMON, *pSIP_COMMON;
#define SIP_COMMON_SIZE				sizeof(SIP_COMMON)

/**
 *	Declare functions
 */
extern S32 dProcSIPStart(stMEMSINFO *pMEMSINFO, SIP_INFO_KEY *pSIPINFOKEY, SIP_INFO *pSIPINFO, TSIP_INFO *pTSIPINFO, U8 *pDATA, U32 len);
extern S32 dProcSIPTrans(stMEMSINFO *pMEMSINFO, stHASHOINFO *pSIPHASH, stTIMERNINFO *pTIMER, TEXT_INFO *pTEXTINFO, U8 *pDATA, U32 len, OFFSET offset);
extern S32 dCloseSIPTrans(stMEMSINFO *pMEMSINFO, stHASHOINFO *pSIPHASH, SIP_INFO_KEY *pSIPINFOKEY, SIP_INFO *pSIPINFO);

#endif	/* _SIPT_FUNC_H_ */
