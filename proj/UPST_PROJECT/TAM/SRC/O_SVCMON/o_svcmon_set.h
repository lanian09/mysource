#ifndef __O_SVCMON_SET_H__
#define __O_SVCMON_SET_H__

extern S32 dSetFirstMon(stHASHOINFO *pDefHash, st_WatchMsg *pWatchMsg, st_FirstMon *pFirstMon);
extern S32 dSetCore(stHASHOINFO *pDefHash, st_WatchMsg *pWatchMsg, st_MonCore *pMonCore);
extern S32 dSetBTS(stHASHOINFO *pDefHash, st_WatchMsg *pWatchMsg, st_MonBTS *pMonBTS);
extern S32 dSetBSC(stHASHOINFO *pDefHash, st_WatchMsg *pWatchMsg, st_MonBSC *pMonBSC);
extern S32 dSetSvc(stHASHOINFO *pDefHash, st_WatchMsg *pWatchMsg, st_MonSvc *pMonSvc);
extern S32 dSetFA(stHASHOINFO *pDefHash, st_WatchMsg *pWatchMsg, st_MonFA *pMonFA);
extern S32 dSetSec(stHASHOINFO *pDefHash, st_WatchMsg *pWatchMsg, st_MonSec *pMonSec);
extern S32 dSetRoam(stHASHOINFO *pDefHash, st_WatchMsg *pWatchMsg, st_MonSvc *pMonSvc);

#endif /* __O_SVCMON_SET_H__ */
