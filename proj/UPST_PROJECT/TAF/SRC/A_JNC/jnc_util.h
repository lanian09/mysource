#ifndef _JNC_UTIL_H_
#define _JNC_UTIL_H_

#include "common_stg.h"
#include "jnc_func.h"
#include "mems.h"
#include "hasho.h"

#define JNC_STATUS_REQ_WAIT		0
#define JNC_STATUS_REQ_DOING	1
#define JNC_STATUS_REQ_END		2
#define JNC_STATUS_RES_DOING	3
#define JNC_STATUS_RES_END		4

#define JNC_ERR_NOREQEND		910
#define	JNC_ERR_NORESSTART  	930
#define	JNC_ERR_NORESEND		940
#define	JNC_ERR_NOMNACK			950

enum _METHOD_ {
	CMD_CONNREQ	=0x01,
	
	CMD_RELAY,

	CMD_ACK,

	CMD_TERMREQ,

	CMD_DOWNBILL,

	CMD_COUNTBILL

};

typedef enum {
	FLD_CPCODE = 0x01,
	FLD_CONTENT,
	FLD_MIN,
	FLD_PHONE,
	FLD_TOTALSIZE,
	FLD_TIME,
	FLD_USECOUNT,
	FLD_USETIME,
	FLD_DATA,
	FLD_END
}_FIELD_;


extern void PrintLOGINFO_IMP(LOG_JNC_TRANS *pJNCLOG);
extern void PrintLOGINFO(LOG_JNC_TRANS *pJNCLOG);
extern char *PrintRtx(U8 ucRtxType);
extern void InitJNCSess(JNC_SESS_DATA *pJNCSESS);
extern void UpdateJNCSess(TCP_INFO *pTCPINFO, JNC_SESS_DATA *pJNCSESS);
extern U32 dMakeJNCLOGInfo(stMEMSINFO *pMEMSINFO,JNC_SESS_DATA *pJNCSESS);
extern S32 dCheckUserErrorCode(LOG_JNC_TRANS *pJNCLOG );
extern U32 dJNCMessage(stMEMSINFO *pMEMSINFO, stHASHOINFO *pJNCHASH, 
				JNC_SESS_KEY *pJNCSESSKEY,JNC_SESS_DATA *pJNCSESS, TCP_INFO *pTCPINFO, U8 *pNode, U8 *pDATA);
extern void MakeJNCHashKey(TCP_INFO *pTCPINFO, JNC_SESS_KEY *pJNCSESSKEY);

#endif /* _JNC_UTIL_H_ */
