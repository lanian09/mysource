#ifndef _SIPM_MSGQ_H_
#define _SIPM_MSGQ_H_

/**
 * Include headers
 */
// LIB
#include "common_stg.h"
#include "mems.h"
#include "cifo.h"
#include "gifo.h"
#include "nifo.h"
#include "loglib.h"

// DQMS
#include "procid.h"

/**
 * Declare variables
 */
extern stCIFO	*gpCIFO;

/**
 * Declare functions
 */
extern S32 dSend_SIPM_Data(stMEMSINFO *pMEMSINFO, S32 dSeqProcID, U8 *pNode);


#endif	/* _SIPM_MSGQ_H_ */
