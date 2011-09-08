#ifndef __MSRPT_FUNC_H__
#define __MSRPT_FUNC_H__

/**
 *	Include headers
 */
// TOP
#include "common_stg.h"

// LIB
#include "mems.h"
#include "hasho.h"
#include "timerN.h"

/**
 *	Define constants
 */
#define MSRPT_TRANS_CNT					100013
//#define DEF_MSRPT_TIMEOUT				5

#define MSRPT_ACT_CONTINUE				0
#define MSRPT_ACT_FINISH				1
#define MSRPT_ACT_DELETE				3

#define MSRPT_STATUS_WAIT				0
#define MSRPT_STATUS_START				1

#define MSRPT_MSG_REQ_NOTHING			1
#define MSRPT_MSG_REQ_CONTINUE			2
#define MSRPT_MSG_REQ_ABORT				3
#define MSRPT_MSG_REQ_END				4
#define MSRPT_MSG_RES					5
#define MSRPT_MSG_REPORT				6
	
/**
 *	Define structures
 */
typedef struct _st_MSRPT_TRANS_KEY {
	U32				uiCliIP;				/**< 단말 IP Address */
	U32				uiSrvIP;				/**< 서버 IP Address */
	U16				usCliPort;				/**< 단말 Port */
	U16				usSrvPort;				/**< 서버 Port */
	U8				szMSGID[MSRP_MSGID_SIZE];
	S8				cReserved;
} st_MSRPT_TRANS_KEY, *pst_MSRPT_TRANS_KEY;
#define DEF_MSRPTTRANSKEY_SIZE				sizeof(st_MSRPT_TRANS_KEY)

typedef struct _st_MSRPT_TRANS {
	U64					timerNID;
	S32					dStatus;
	U16					usFailCode;
	U8					ucFinish;
	U8					cReserved;
	OFFSET				offset_LOG;
	OFFSET				offset_NODE;
} st_MSRPT_TRANS, *pst_MSRPT_TRANS;
#define DEF_MSRPTTRANS_SIZE					sizeof(st_MSRPT_TRANS)

typedef struct _st_MSRPT_COMMON {
    st_MSRPT_TRANS_KEY	stMSRPTTRANSKEY;
} st_MSRPT_COMMON, *pst_MSRPT_COMMON;
#define DEF_MSRPTCOMMON_SIZE					sizeof(st_MSRPT_COMMON)

typedef struct _st_MSRPT_INFO {
	U16				usMethod;
	U16				usSuccessReport;
	U16				usFailureReport;
	U16				usResCode;
	U16				usEndFlag;
	U16				usBodyLen;
	U16				usVendor;
	U8				szToPath[MSRP_PATH_SIZE];
	U8				szFromPath[MSRP_PATH_SIZE];
	U8				szMSGID[MSRP_MSGID_SIZE];
	U8				szContentType[MSRP_CONTENTTYPE_SIZE];
	U8				szTID[MSRP_TID_SIZE];
	U8				szMIN[MAX_MIN_SIZE];
} st_MSRPT_INFO, *pst_MSRPT_INFO;
#define DEF_MSRPTINFO_SIZE				sizeof(st_MSRPT_INFO)

/**
 *	Declare functions
 */
extern S32 dProcMSRPTTrans(stMEMSINFO *pstMEMSINFO, stHASHOINFO *pstMSRPTHASH, stTIMERNINFO *pstTIMERNINFO, TEXT_INFO *pstTEXTINFO, U8 *pDATA);
extern S32 dFlowMSRPTTrans(stMEMSINFO *pstMEMSINFO, st_MSRPT_INFO *pstMSRPTINFO, st_MSRPT_TRANS_KEY *pstMSRPTTRANSKEY, st_MSRPT_TRANS *pstMSRPTTRANS, TEXT_INFO *pstTEXTINFO);
extern S32 dCloseMSRPTTrans(stMEMSINFO *pstMEMSINFO, stHASHOINFO *pstMSRPTHASH, st_MSRPT_TRANS_KEY *pstMSRPTTRANSKEY, st_MSRPT_TRANS *pstMSRPTTRANS);

#endif	/* __MSRPT_FUNC_H__ */

