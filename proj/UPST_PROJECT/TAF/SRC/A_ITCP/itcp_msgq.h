#ifndef _ITCP_MSGQ_H_
#define _ITCP_MSGQ_H_

/**
 * Include headers
 */
// TOP
#include "common_stg.h"
#include "func_time_check.h"

// LIB
#include "typedef.h"
#include "mems.h"
#include "cifo.h"

// TAF
#include "debug.h"

extern st_FuncTimeCheckList	*pFUNC;
extern ATCP_SUBINFO			*pATCPSUBINFO;
extern int              	gAHTTPCnt;
extern UINT					guiSeqProcID;
extern stCIFO				*gpCIFO;
extern stMEMSINFO			*pMEMSINFO;
extern TCP_SESS_KEY			*pTCPSESSKEY;

extern S32					dGetCALLProcID(U32 uiClientIP);

#endif	/* _ITCP_MSGQ_H_ */
