#ifndef __M_SVCMON_FUNC_H__
#define __M_SVCMON_FUNC_H__

#include "watch_mon.h"
#include "nsocklib.h"

#define MAX_SVCMONFNAME_SIZE		7
#define MAX_SVCMONFILE_CNT			12
#define MONTYPE_CORE_DATA			0
#define MONTYPE_CORE_DEFECT			1
#define MONTYPE_BSC_DATA			2
#define MONTYPE_BSC_DEFECT			3
#define MONTYPE_BTS_DATA			4
#define MONTYPE_BTS_DEFECT			5
#define MONTYPE_FA_DATA				6
#define MONTYPE_FA_DEFECT			7
#define MONTYPE_SEC_DATA			8
#define MONTYPE_SEC_DEFECT			9
#define MONTYPE_SVC_DATA			10
#define MONTYPE_SVC_DEFECT			11
#define SVCMON_DATA_PATH            START_PATH"/SVCMON"


typedef struct _st_SFILE
{
	st_SI_DB	stSIDB;
	FILE		*fp;
} st_SFILE, *pst_SFILE;

typedef struct _st_SFILE_List
{
	st_SFILE	stSFILE[MAX_SVCMONFILE_CNT];
} st_SFILE_List, *pst_SFILE_List;

extern S32 dProcSvcMonMsg(st_MonTotal *pMON, st_SvcMonMsg *pSvcMonMsg);
extern S32 dProcSvcMon1MinMsg(st_MonTotal_1Min *pMON1Min, st_SvcMonMsg *pSvcMonMsg);
extern void vCloseFile(int cnt, st_SFILE_List *pList);
extern void vSaveCOREData(FILE *fp, U32 office, U32 systype, U32 ip, st_MonAlarm *pAlarm, st_MonInfo *pInfo);
extern void vSaveCOREDefect(FILE *fp, U32 office, U32 systype, U32 ip, st_Defect *pDefect);
extern void vSaveBSCData(FILE *fp, U32 bsc, st_MonAlarm *pAlarm, st_MonInfo *pInfo);
extern void vSaveBSCDefect(FILE *fp, U32 bsc, st_Defect *pDefect);
extern void vSaveBTSData(FILE *fp, U64 bts, st_MonAlarm *pAlarm, st_MonInfo *pInfo);
extern void vSaveBTSDefect(FILE *fp, U64 bts, st_Defect *pDefect);
extern void vSaveFAData(FILE *fp, U64 bts, U32 fa, st_MonAlarm *pAlarm, st_MonInfoS *pInfoS);
extern void vSaveFADefect(FILE *fp, U64 bts, U32 fa, st_DefectS *pDefectS);
extern void vSaveSECData(FILE *fp, U64 bts, U32 fa, U32 sec, st_MonAlarm *pAlarm, st_MonInfoS *pInfoS);
extern void vSaveSECDefect(FILE *fp, U64 bts, U32 fa, U32 sec, st_DefectS *pDefectS);
extern void vSaveSVCData(FILE *fp, U32 svcidx, U32 ip, U32 l4svctype, st_MonAlarm *pAlarm, st_MonInfo *pInfo);
extern void vSaveSVCDefect(FILE *fp, U32 svcidx, U32 ip, U32 l4svctype, st_Defect *pDefect);
extern void vSaveData(FILE *fp, st_MonAlarm *pAlarm, st_MonInfo *pInfo);
extern void vSaveDataS(FILE *fp, st_MonAlarm *pAlarm, st_MonInfoS *pInfoS);

#endif /* __M_SVCMON_FUNC_H__ */
