#ifndef __WATCH_FILTER_H__
#define __WATCH_FILTER_H__

#include "define.h"
#include "watch_mon.h"

#define MAX_MODEL_INFO		10000000
#define MAX_ROAM_EQUIP_CNT	500

#define ROAM_ALARM_SYSTYPE		12

typedef struct _st_WatchDefect
{
	S32					dIndex;
	S32					dDefect;
} st_WatchDefect, *pst_WatchDefect;
#define DEF_WATCHDEFECT_SIZE			sizeof(st_WatchDefect)

typedef struct _st_WatchDefectList
{
	S32					dCount;
	st_WatchDefect		stWatchDefect[MAX_MON_DEFECT_IDX];
} st_WatchDefectList, *pst_WatchDefectList;
#define DEF_WATCHDEFECTLIST_SIZE		sizeof(st_WatchDefectList)

typedef struct _st_WatchService
{
	S32					dSvcL4Type;
	S32					dSvcL7Type;
	U32					uiIP;
	U32					uiReserved;
} st_WatchService, *pst_WatchService;
#define DEF_WATCHSERVICE_SIZE			sizeof(st_WatchService)

typedef struct _st_WatchServiceList
{
	S32					dCount;
	st_WatchService		stWatchService[MAX_MON_SVC_CNT];
} st_WatchServiceList, *pst_WatchServiceList;
#define DEF_WATCHSERVICELIST_SIZE		sizeof(st_WatchServiceList)

typedef struct _st_WatchEquip
{
	S32					dType;
	U32					uiIP;
	S32					dMon1MinFlag;
	char				reserved[4];
} st_WatchEquip, *pst_WatchEquip;
#define DEF_WATCHEQUIP_SIZE				sizeof(st_WatchEquip)

/*TODO : jbaek*/
//PDIF Ãß°¡
#if 0
typedef struct _st_WatchEquipList
{
	S32					dCount;
	st_WatchEquip		stWatchEquip[MAX_MON_PDSN_CNT + MAX_MON_AAA_CNT + MAX_MON_HSS_CNT + MAX_MON_LNS_CNT + MAX_MON_PDIF_CNT];
} st_WatchEquipList, *pst_WatchEquipList;

#endif

typedef struct _st_WatchEquipList
{
    S32                 dCount;
    st_WatchEquip       stWatchEquip[MAX_MON_PDSN_CNT + MAX_MON_AAA_CNT + MAX_MON_HSS_CNT + MAX_MON_LNS_CNT];
} st_WatchEquipList, *pst_WatchEquipList;

#define DEF_WATCHEQUIPLIST_SIZE			sizeof(st_WatchEquipList)

typedef struct _st_RoamEquip
{
	S32					dType;
	U32					uiIP;
	U32					uiNetMask;
	U32					dMon1MinFlag;
} st_RoamEquip, *pst_RoamEquip;
#define DEF_ROAMEQUIP_SIZE				sizeof(st_RoamEquip)

typedef struct _st_LoamEquipList
{
	S32					dCount;
	st_RoamEquip		stRoamEquip[MAX_ROAM_EQUIP_CNT];
} st_LoamEquipList, *pst_LoamEquipList;
#define DEF_LOAMEQUIPLIST_SIZE			sizeof(st_LoamEquipList)

/* pcf type */
#define PCFTYPE_LG_1x			1
#define PCFTYPE_LG_EvDO			2
#define PCFTYPE_LG_BOTH			3
#define PCFTYPE_SS_BOTH			4
typedef struct _st_WatchPCF
{
	U8					ucOffice;
	U8					ucPCFType;
	U8					ucReserved[2];
	U32					uiIP;
} st_WatchPCF, *pst_WatchPCF;
#define DEF_WATCHPCF_SIZE				sizeof(st_WatchPCF)

typedef struct _st_WatchPCFList
{
	S32					dCount;
	st_WatchPCF			stWatchPCF[MAX_MON_PCF_CNT];
} st_WatchPCFList, *pst_WatchPCFList;
#define DEF_WATCHPCFLIST_SIZE			sizeof(st_WatchPCFList)

typedef struct _st_WatchBSC
{
	U8					ucOffice;
	U8					ucSYSID;
	U8					ucBSCID;
	U8					ucReserved;
} st_WatchBSC, *pst_WatchBSC;
#define DEF_WATCHBSC_SIZE				sizeof(st_WatchBSC)

typedef struct _st_WatchBSCList
{
	S32					dCount;
	st_WatchBSC			stWatchBSC[MAX_MON_BSC_CNT];
} st_WatchBSCList, *pst_WatchBSCList;
#define DEF_WATCHBSCLIST_SIZE			sizeof(st_WatchBSCList)

typedef struct _st_WatchBTS
{
	U8					ucOffice;
	U8					ucSYSID;
	U8					ucBSCID;
	U8					ucReserved;
	U16					usBTSID;
	U16					usReserved;
} st_WatchBTS, *pst_WatchBTS;
#define DEF_WATCHBTS_SIZE				sizeof(st_WatchBTS)

typedef struct _st_WatchBTSList
{
	S32					dCount;
	st_WatchBTS			stWatchBTS[MAX_MON_BTS_CNT];
} st_WatchBTSList, *pst_WatchBTSList;
#define DEF_WATCHBTSLIST_SIZE			sizeof(st_WatchBTSList)

/* alarm type */
#define DEF_ALARMTYPE_CALL				0
#define DEF_ALARMTYPE_RECALL			1	
#define DEF_ALARMTYPE_AAA				2
#define DEF_ALARMTYPE_HSS				3
#define DEF_ALARMTYPE_LNS				4
#define DEF_ALARMTYPE_MENU				5
#define DEF_ALARMTYPE_DN				6
#define DEF_ALARMTYPE_STREAM			7
#define DEF_ALARMTYPE_MMS				8
#define DEF_ALARMTYPE_WIDGET			9
#define DEF_ALARMTYPE_PHONE				10
#define DEF_ALARMTYPE_EMS				11
#define DEF_ALARMTYPE_BANK				12
#define DEF_ALARMTYPE_FV				13
#define DEF_ALARMTYPE_IM				14
#define DEF_ALARMTYPE_VT				15
#define DEF_ALARMTYPE_ETC				16
#define DEF_ALARMTYPE_CORP				17
#define DEF_ALARMTYPE_REGI				18
#define DEF_ALARMTYPE_INET              19
#define DEF_ALARMTYPE_RECVCALL          20
#define DEF_ALARMTYPE_IM_RECV           21
#define DEF_ALARMTYPE_VT_RECV           22
#define MAX_ALARMTYPE_IDX				23
typedef struct _st_WatchThreshold
{
	U8					ucOffice;
	U8					ucSysType;
	U8					ucAlarmType;
	U8					ucStartTime;
	U8					ucRange;
	U8					ucReserved1[3];
	U8					ucDayTimeRate;
	U8					ucNightTimeRate;
	U8					ucReserved2[2];
	U32					uiDayTimeMinTrial;
	U32					uiNightTimeMinTrial;
	U32					uiPeakTrial;
	U32					uiIP;
	U32					uiReserved;
} st_WatchThreshold, *pst_WatchThreshold;
#define DEF_WATCHTHRESHOLD_SIZE			sizeof(st_WatchThreshold)

typedef struct _st_WatchThresholdList
{
	S32					dCount;
	st_WatchThreshold	stWatchThreshold[MAX_SYSTEM_TYPE_IDX * MAX_MON_OFFICE_IDX * MAX_ALARMTYPE_IDX];
} st_WatchThresholdList, *pst_WatchThresholdList;
#define DEF_WATCHTHRESHOLDLIST_SIZE		sizeof(st_WatchThresholdList)


typedef struct _st_DefectThreshold
{
    U32                 uiSvcType;
	U32					uiTCPSetupTime;
	U32					uiResponseTime;
	U32					uiUpThroughput;
	U32					uiDnThroughput;
	U32					uiUpRetransCnt;
	U32					uiDnRetransCnt;
	U32					UpJitter;
	U32					DnJitter;
	U32					UpPacketLoss;
	U32					DnPacketLoss;
	U32					uiReserved;
} st_DefectThreshold, *pst_DefectThreshold;
#define DEF_DEFECTTHRESHOLD_SIZE         sizeof(st_DefectThreshold)

typedef struct _st_DefectThresholdList
{
    S32                 dCount;
	S32					reserved;
    st_DefectThreshold   stDefectThreshold[MAX_DEFECT_THRES];
} st_DefectThresholdList, *pst_DefectThresholdList;
#define DEF_DEFECTTHRESHOLDLIST_SIZE     sizeof(st_DefectThresholdList)

typedef struct _st_ModelInfo
{
	U8					szIMSI[MAX_MIN_SIZE];
	U8					szModel[MAX_MODEL_SIZE];
	U8					szMIN[MAX_MIN_SIZE];
} st_ModelInfo, *pst_ModelInfo;
#define DEF_MODELINFO_SIZE         sizeof(st_ModelInfo)

/*
typedef struct _st_ModelInfoList
{
    S32                 dCount;
    S32                 reserved;
    st_ModelInfo   		stModelInfo[MAX_MODEL_INFO];
} st_ModelInfoList, *pst_ModelInfoList;
*/
typedef struct _st_ModelInfoList
{       
    S32                 dActiveStatus;			/* 1:USE 1st HASH, 2:USE 2nd HASH */
    S32                 reserved;
} st_ModelInfoList, *pst_ModelInfoList;
#define DEF_MODELINFOLIST_SIZE     sizeof(st_ModelInfoList)

typedef struct _st_WatchFilter
{
	st_WatchDefectList		stWatchDefectList;
	st_WatchServiceList		stWatchServiceList;		/*	FLT_SVC		*/
	st_WatchEquipList		stWatchEquipList;		/*	FLT_SCTP(HSS), TB_MEQUIPMC	*/
	st_WatchPCFList			stWatchPCFList;			/*	TB_MACCESSC					*/
	st_WatchBSCList			stWatchBSCList;			/*	TB_MACCESSC					*/
	st_WatchBTSList			stWatchBTSList;			/*	TB_MACCESSC					*/
	st_WatchThresholdList	stWatchThresholdList;	/*	INFO_MON_THRESHOLD			*/
	st_DefectThresholdList	stDefectThresholdList;	/*	INFO_DEFECT_THRESHOLD		*/
	st_ModelInfoList		stModelInfoList;
	st_LoamEquipList		stLoamEquipList;		/*	TB_MEQUIPMC	*/
} st_WatchFilter, *pst_WatchFilter;
#define DEF_WATCHFILTER_SIZE			sizeof(st_WatchFilter)

#endif
