#ifndef __RPPI_MSGQ_H__
#define __RPPI_MSGQ_H__

#include "typedef.h"
#include "common_stg.h"	/* LOG_RPPI */
#include "watch_mon.h"	/* pst_WatchMsg */
#include "rppi_def.h"	/* HData_RPPI */

extern S32 dSendMonInfo( pst_WatchMsg pstWatch );
extern S32 dSendLogRPPI(HData_RPPI *pstRPPIHash, LOG_RPPI *pstRPPILOG);
#endif /* __RPPI_MSGQ_H__ */
