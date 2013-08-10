#ifndef __O_SVCMON_PRINT_H__
#define __O_SVCMON_PRINT_H__

extern void PrintWatchMsg(S32 lvl, S8 *str, S32 dRet, st_WatchMsg *pWatchMsg);
extern void PrintMonList(S32 lvl, S32 detail, st_MonList *pMonList, st_MonList_1Min *pMonList1Min);
extern void PrintMonCore(S32 lvl, S32 detail, st_MonCore *pMonCore);
extern void PrintFirstMon(S32 lvl, S32 detail, st_FirstMon *pFirstMon);
extern void PrintMonSvc(S32 lvl, S32 detail, st_MonSvc *pMonSvc);
extern void PrintMonBSC(S32 lvl, S32 detail, st_MonBSC *pMonBSC);
extern void PrintMonBTS(S32 lvl, S32 detail, st_MonBTS *pMonBTS);
extern void PrintMonFA(S32 lvl, S32 detail, st_MonFA *pMonFA);
extern void PrintMonSec(S32 lvl, S32 detail, st_MonSec *pMonSec);
extern void PrintMonAlarm(S32 lvl, st_MonAlarm *pMonAlarm);
extern void PrintMonInfo(S32 lvl, st_MonInfo *pMonInfo);
extern void PrintMonInfoS(S32 lvl, st_MonInfoS *pMonInfoS);
extern void PrintDefect(S32 lvl, st_Defect *pMonDefect);
extern void PrintDefectS(S32 lvl, st_DefectS *pMonDefectS);

#endif /* __O_SVCMON_PRINT_H__ */
