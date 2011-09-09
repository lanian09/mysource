/*******************************************************************************
               DQMS Project

     Author   :
     Section  :
     SCCS ID  :
     Date     :
     Revision History :

     Description :

     Copyright (c) uPRESTO 2005
*******************************************************************************/
#ifndef __CHGSVCM_PROC_H__
#define __CHGSVCM_PROC_H__

/** A.1* FILE INCLUDE *************************************/
#include <time.h>

/** B.1* DEFINITION OF NEW CONSTANTS **********************/
#define DEF_TYPE_HTTP_LOG		0x01
#define DEF_TYPE_TCP_LOG		0x02
#define MAX_HOSTNAME_DEPTH		3
#define DEF_BACKUP_LOG_PATH		"/TAMAPP/BACKUP"

#define DEF_NONE_FORCE			1
#define DEF_SET_FORCE			2

/** C.1* DEFINITION OF NEW TYPES **************************/
/** D.1* DECLARATION OF VARIABLES *************************/
/** E.1* DEFINITION OF FUNCTIONS **************************/
extern int dGetDirecFileList(char *rootpath, time_t curtime);
extern int dWriteHttpTcpLogStatisticFile(time_t curtime);
extern int dWriteGooglePushList(time_t curtime);


#endif /* __CHGSVCM_PROC_H__ */

