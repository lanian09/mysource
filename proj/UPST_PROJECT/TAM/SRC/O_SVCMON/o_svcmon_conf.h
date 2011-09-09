#ifndef __O_SVCMON_CONF_H__
#define __O_SVCMON_CONF_H__


extern st_MonList *pGetNextMonList(st_MonTotal *pMON, st_MonList *pBaseMonList);
extern st_MonList_1Min *pGetNextMon1MinList(st_MonTotal_1Min *pMON, st_MonList_1Min *pBaseMonList);
extern S32 dMakeBaseMonList(stHASHOINFO *pHash, stHASHOINFO *pNasIPHash, st_WatchFilter *pWatchFilter, st_MonList *pMonList, st_MonList_1Min *pMonList1Min);
extern S32 dMakeBaseFirstMon(stHASHOINFO *pHash, st_FirstMonList *pFirstMonList, S32 office, S32 systype, S32 subtype, UINT ui1MinMonFlag);
extern S32 dMakeBaseFirstMon(stHASHOINFO *pHash, st_FirstMonList *pFirstMonList, S32 office, S32 systype, S32 subtype, UINT ui1MinMonFlag);
extern S32 dMakeThresHash(stHASHOINFO *pThresHash, st_WatchFilter *pWatchFilter);
extern S32 dMakeNasIPHash(stHASHOINFO *pNasIPHash, U32 uiNasIP, U32 uiNetMask);
extern st_MonCore *pMakePCF(stHASHOINFO *pHash, st_MonList *pBaseList, st_MonList *pMonList, st_WatchMsg *pWatchMsg);
extern st_MonBSC *pMakeBSC(stHASHOINFO *pHash, st_MonList_1Min *pBaseList, st_MonList_1Min *pMonList, st_WatchMsg *pWatchMsg);
extern st_MonBTS *pMakeBTS(stHASHOINFO *pHash, st_MonList *pBaseList, st_MonList *pMonList, st_WatchMsg *pWatchMsg);
extern int dGetSYSCFG(void);


#endif /* __O_SVCMON_CONF_H__ */
