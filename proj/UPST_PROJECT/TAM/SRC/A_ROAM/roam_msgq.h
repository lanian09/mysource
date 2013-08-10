#ifndef _ROAM_MSGQ_H_
#define _ROAM_MSGQ_H_

/**
 *	INCLUDE HEADER FILES
 */
//DQMS
#include "common_stg.h"			/* LOG_RPPI */

// TAM
#include "watch_mon.h"			/* pst_WatchMsg */
#include "rppi_def.h"			/* HData_RPPI */

/**
 * DECLARE FUNCTIONS
 */
extern S32 dSendMonInfo(pst_WatchMsg pstWatch);
extern S32 dSendLogRPPI(HData_RPPI *pstRPPIHash, LOG_RPPI *pstRPPILog);

#endif	/* _ROAM_MSGQ_H_ */
