#ifndef __O_SVCMON_GET_H__
#define __O_SVCMON_GET_H__

extern st_FirstMon *getFirstMon(stHASHOINFO *pHash, S32 office, S32 systype, S32 subtype, st_FirstMon *aFirstMon);
extern st_MonCore *getMonCore(stHASHOINFO *pHash, st_WatchMsg *pWatchMsg, S32 systype, st_MonCore *aMonCore, st_MonCore *aMonCore1Min);
extern st_MonBTS *getMonBTS(stHASHOINFO *pHash, st_WatchMsg *pWatchMsg, st_MonBTS *aMonBTS);
extern st_MonBSC *getMonBSC(stHASHOINFO *pHash, st_WatchMsg *pWatchMsg, st_MonBSC *aMonBTS);
extern st_MonSvc *getMonSvc(stHASHOINFO *pHash, st_WatchMsg *pWatchMsg, st_MonSvc *aMonSvc);
extern st_MonFA *getMonFA(st_WatchMsg *pWatchMsg, st_MonBTS *pMonBTS);
extern st_MonSec *getMonSec(st_WatchMsg *pWatchMsg, st_MonFA *pMonFA);
extern st_MonSvc *getMonRoam(stHASHOINFO *pHash, stHASHOINFO *pNasIPHash, st_WatchMsg *pWatchMsg, st_MonSvc *aMonSvc);

#endif /* __O_SVCMON_GET_H__ */
