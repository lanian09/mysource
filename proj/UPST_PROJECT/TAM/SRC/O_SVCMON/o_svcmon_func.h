#ifndef __O_SVCMON_FUNC_H__
#define __O_SVCMON_FUNC_H__


extern S32 dProcMON(stHASHOINFO *pDefHash, stHASHOINFO *pMonHash, stHASHOINFO *pNasIPHash, st_WatchMsg *pWatchMsg, st_MonList *pMonList, st_MonList *pBaseList, st_MonList_1Min *pMonList1Min, st_MonList_1Min *pBaseList1Min);
extern S32 dProcFirstList(stHASHOINFO *pDefHash, stHASHOINFO *pMonHash, st_WatchMsg *pWatchMsg, st_FirstMonList *pFirstMonList, st_FirstMonList *pFirstMonList1Min, S32 pcftype);
extern S32 dProcFirst(stHASHOINFO *pDefHash, stHASHOINFO *pMonHash, st_WatchMsg *pWatchMsg, st_FirstMon *aFirstMon, S32 cnt, st_FirstMon *aFirstMon1Min, S32 cnt1Min, S32 pcftype);
extern S32 dProcROAM(stHASHOINFO *pDefHash, stHASHOINFO *pMonHash, stHASHOINFO *pNasIPHash, st_WatchMsg *pWatchMsg, st_MonList *pMonList, st_MonList_1Min *pMonList1Min);

#endif /* __O_SVCMON_FUNC_H__ */
