#ifndef __O_SVCMON_ALARM_H__
#define __O_SVCMON_ALARM_H__

extern S32 dAlarmMON1Min(stHASHOINFO *pThresHash, stHASHOINFO *pHash, st_MonList_1Min *pMonList);
extern S32 dAlarmMON(stHASHOINFO *pThresHash, stHASHOINFO *pHash, st_MonList *pMonList, st_MonList *pOldMonList);
extern S32 dCheckPDSN(stHASHOINFO *pThresHash, st_MonCore *aPDSN, S32 cnt, time_t stattime);
extern S32 dCheckAAA(stHASHOINFO *pThresHash, st_MonCore *aAAA, S32 cnt, time_t stattime);
extern S32 dCheckHSS(stHASHOINFO *pThresHash, st_MonCore *aHSS, S32 cnt, time_t stattime);
extern S32 dCheckLNS(stHASHOINFO *pThresHash, st_MonCore *aLNS, S32 cnt, time_t stattime);
extern S32 dCheckPCF(stHASHOINFO *pThresHash, stHASHOINFO *pHash, st_MonCore *aPCF, S32 cnt, st_FirstMon *aFirstMon, time_t stattime);
extern S32 dCheckBSC(stHASHOINFO *pThresHash, stHASHOINFO *pHash, st_MonBSC *aBSC, S32 cnt, st_FirstMon *aFirstMon, time_t stattime);
extern S32 dCheckBTS(stHASHOINFO *pThresHash, stHASHOINFO *pHash, st_MonBTS *aBTS, S32 cnt, st_FirstMon *aFirstMon, time_t stattime);
extern S32 dCheckFA(stHASHOINFO *pThresHash, st_MonFA *aFA, S32 cnt, st_FirstMon *pFirstMonFA, st_FirstMon *pFirstMonSec, S32 office, time_t stattime);
extern S32 dCheckSEC(stHASHOINFO *pThresHash, st_MonSec *aSEC, S32 cnt, st_FirstMon *pFirstMon, S32 office, time_t stattime);
extern S32 dCheckSVC(stHASHOINFO *pThresHash, stHASHOINFO *pHash, st_MonSvc *aSVC, st_MonSvc *oSVC, S32 cnt, st_FirstMon *aFirstMon, time_t stattime);
extern S32 dSetSvcConsole(S32 subtype, S32 alarmvalue, st_MonSvc *pSVC, st_MonAlarm *pMonAlarm, st_MonAlarm *pOldMonAlarm, UINT ip , S32 trialcnt, S32 succcnt);
extern S32 dSetAlarmFlag(S32 systype, S32 subtype, S32 alarmvalue, st_MonAlarm *pMonAlarm);

#endif /* __O_SVCMON_ALARM_H__ */
