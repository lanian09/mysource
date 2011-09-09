#ifndef __S_MNG_INIT_H__
#define __S_MNG_INIT_H__

#include <mysql/mysql.h>	/* MYSQL */
#include "typedef.h"

#define _MSCTN_COUNT_ON  1
#define _MSCTN_COUNT_OFF 0

extern int dWriteWatchFilter(char *FilePath);
extern int dReadWatchFilter(void);
extern void InitTamSvclist(void);
extern void UserControlledSignal(int sign);
extern void SetUpSignal(void);
extern void IgnoreSignal(int sign);

extern int dInit_SubSystem_Info(void);
extern int dInit_GINTAF_SubSystem_Info(void);
extern int dInit_FltSvc_Info(void);
extern int dInit_WatchFltSVC_Info(void);
extern int dInit_WatchInfoMonThreshold(void);
extern int dInit_WatchInfoDefectThreshold(void);
extern int dInit_WatchFltEquip_Info(void);
extern int dInit_FltSCTP_Info(void);
extern int dInit_CTNInfo(void);
extern int dInit_IRMInfo(void);
extern int dInit_WatchInfoAccess(void);
extern int dInit_FltIPPool_Info(void);
extern int dInit_FltThres_Info(void);
extern int dInit_User_Info(void);
extern int dInit_Equip_Info(void);

extern int dInit_IPC(void);
extern int dInit_Info(void);
extern int dReadSvcStat(void);
extern void FinishProgram(MYSQL *pstMySQL);
extern int dWriteSvcStat(void);
extern int dMake_DefectInfoFile(void);

#endif /* __S_MNG_INIT_H__ */
