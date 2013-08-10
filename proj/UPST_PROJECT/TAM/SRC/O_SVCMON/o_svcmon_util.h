#ifndef __O_SVCMON_UTIL_H__
#define __O_SVCMON_UTIL_H__

extern S8 *PrintMSGTYPE(S32 msgtype);
extern S8 *PrintOFFICE(S32 office);
extern S8 *PrintSVC(S32 svc);
extern S8 *PrintSYSTYPE(S32 systype);
extern S8 *PrintSUBTYPE(S32 systype, S32 subtype);
extern S8 *PrintPCFTYPE(S32 pcftype);
extern S8 *PrintALARMTYPE(S32 alarmtype);
extern S8 *PrintSVCTYPE(S32 svctype);
extern S8 *PrintALMVALUE(S32 svcvalue);
extern S32 getSvcIndex(S32 svcl4, S32 svcl7);
extern S32 getDefectIndex(stHASHOINFO *pDefHash, U32 result);
extern st_ThresHash_Data *getBaseValue(stHASHOINFO *pThresHash, S32 office, S32 systype, S32 alarmtype, UINT ip);
extern S32 isAlarm(st_ThresHash_Data *pData, time_t stattime, U32 trialcnt, U32 succcnt);
extern S32 isAlarmAll(st_ThresHash_Data *pData, time_t stattime, U32 trialcnt, U32 succcnt);
extern S32 getAlarm(S32 curalarm, S32 inputalarm);
extern S32 getThresFlag(st_ThresHash_Data *pData, time_t stattime);
extern S32 getPCFType(S32 pcftype);
extern unsigned int GetSubNet(unsigned int ip, int netmask);
extern int GetSubNetLoopCnt(int netmask);
extern U32 getNasIP(stHASHOINFO *pNasIPHash, U32 ip);

#endif /* __O_SVCMON_UTIL_H__ */
