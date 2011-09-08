#ifndef _SI_DB_FUNC_H_
#define _SI_DB_FUNC_H_

/**
 *	Include headers
 */
// DQMS
#include "path.h"			/* BACKUP_PATH */

// LIB
#include "nsocklib.h"

/**
 *	Declare functions
 */
extern int dHandleMsgQMsg(st_ClientInfo *stNet, st_FDInfo *stFD, st_SI_DB *pSIDB);
extern int dHandleSocketMsg(st_ClientInfo *stNet, int dIdx, st_FDInfo *stFD, char *szBuf);
extern int dHandleFile();
extern int dCheck_Channel(int dSysNo, int dFlag, unsigned int uiIP);
extern int dCheck_File(st_SI_DB *pSIDB);


#endif	/* _SI_DB_FUNC_H_ */
