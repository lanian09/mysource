#ifndef _JNC_FUNC_H_
#define _JNC_FUNC_H_

#include "typedef.h"
#include "common_stg.h"	/* LOG_JNC_TRANS */
#include "mems.h"
#include "hasho.h"

typedef struct _st_JNC_Sess_Key {
	U32		uiCliIP;
	U16		usCliPort;
	U16		usReserved;
}JNC_SESS_KEY;

#define MAX_BUFF_SIZE 80
#define MAX_BUFF_LEN  MAX_BUFF_SIZE-1

typedef struct _st_JNC_Sess {

	
	U8  ucResStartFlag;
	U8	ucREQParaFlag;
	U16	usREQBuffLen;
//	U32	uiIPREQByte;

//	U32 uiTotalSize;
//	U32 uiUseCount;

//	U64 timerNID;


	U8	szREQBuffer[MAX_BUFF_SIZE];

	LOG_JNC_TRANS	szJNCLOG;
}JNC_SESS_DATA;

#define JNC_SESS_SIZE	sizeof(JNC_SESS_DATA)
#define JNC_SESS_KEY_SIZE sizeof(JNC_SESS_KEY)

typedef struct _st_JNC_COMMON {
	JNC_SESS_KEY	JNCSESSKEY;
} JNC_COMMON;


extern JNC_SESS_DATA *pCreateJNCSess(stMEMSINFO *pMEMSINFO, stHASHOINFO *pJNCHASH,JNC_SESS_KEY *pJNCSESSKEY, TCP_INFO *pTCPINFO);
extern S32 dJNCSvcProcess(stMEMSINFO *pMEMSINFO, stHASHOINFO *pJNCHASH,TCP_INFO *pTCPINFO, U8 *pNode, U8 *pDATA);
extern S32 dCloseJNCSess(stMEMSINFO *pMEMSINFO, stHASHOINFO *pJNCHASH, JNC_SESS_KEY *pJNCSESSKEY, JNC_SESS_DATA *pJNCSESS);

#endif /* _JNC_FUNC_H_ */
