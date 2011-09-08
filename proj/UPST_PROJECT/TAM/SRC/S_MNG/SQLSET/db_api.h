#ifndef __DB_API_H__
#define __DB_API_H__

#include <mysql/mysql.h>	/*	MYSQL, MYSQL_ROW, MYSQL_RES structure, mysql_query, mysql_store_result, mysql_fetch_row, mysql_free_result	*/

#include "watch_filter.h"
#include "db_struct.h"
#include "mmcdef.h"			/*  st_UserAdd */

#define MAX_EQUIP_INFO				500
#define MAX_SVR_CNT                 2000
#define MAX_MON_THRESHOLD_COUNT     (MAX_MON_OFFICE_IDX*MAX_SYSTEM_TYPE_IDX*MAX_ALARMTYPE_IDX)+25	/* _IDX 값은 모두 watch_filter.h에 있음 */
#define MAX_CMD_CNT                 12

/**B.1*  Definition of New Constants *********************/
/**B.2*  Definition of New Type  **************************/
/**C.1*  Declaration of Variables  ************************/
/**D.1*  Definition of Functions  *************************/

/**D.2*  Definition of Functions  *************************/
extern int dTruncateTbl(MYSQL *pstMySQL, char *sTblName);
extern int dGetMMCUserCount(MYSQL *pstMySQL, int *dCnt);
extern int dInsertEquip(MYSQL *pstMySQL, st_Info_Equip *stData);
extern int dGetSvcInfo(MYSQL *pstMySQL, pst_SvcMmc pstData, int *dCnt, char *sWhere);

extern int dGetWatchServiceList(MYSQL *pstMySQL, st_WatchServiceList *pstData);
extern int dGetWatchInfoMonThreshold(MYSQL *pstMySQL, st_WatchThresholdList *pstData);
extern int dGetWatchInfoDefectThreshold(MYSQL *pstMySQL, st_DefectThresholdList *pstData);
extern int dGetFltEquipCount(MYSQL *pstMySQL, int *dCount);
extern int dGetFltEquipList(MYSQL *pstMySQL, st_WatchEquipList *pstData);
extern int dGetFltRoamEquipCount(MYSQL *pstMySQL, int *dCount);
extern int dGetFltRoamEquipList(MYSQL *pstMySQL, st_LoamEquipList *pstData);
extern int dGetInfoAccessCount(MYSQL *pstMySQL, unsigned char cSysType, int *dCount);
extern int dGetInfoAccessPCF(MYSQL *pstMySQL, st_WatchPCFList *pstData);
extern int dGetInfoAccessBSC(MYSQL *pstMySQL, st_WatchBSCList *pstData);
extern int dGetInfoAccessBTS(MYSQL *pstMySQL, st_WatchBTSList *pstData);

extern int dGetSCTPInfo(MYSQL *pstMySQL, st_SCTP_DB *pstData, int *dCnt, char *sWhere);
extern int dInsertSvcInfo(MYSQL *pstMySQL, st_SvcInfo *stData);
extern int dInsertSCTPInfo(MYSQL *pstMySQL, st_SCTP_DB *stData);
extern int dGetSvcInfoCount(MYSQL *pstMySQL, int *dCnt);
extern int dGetSCTPInfoCount(MYSQL *pstMySQL, int *dCnt);
extern int dGetCountSVC(MYSQL *pstMySQL, unsigned int uiIP, unsigned short usPort, unsigned short huFlag);
extern int dDeleteSvc(MYSQL *pstMySQL, unsigned int uiIP, unsigned short usPort, unsigned short huFlag);
extern int dDeleteSCTP(MYSQL *pstMySQL, unsigned int uiIP);
extern int dGetCountSCTPIP(MYSQL *pstMySQL, unsigned int uiIP);
extern int dGetNAS(MYSQL *pstMySQL, st_NAS_db *pstData, int *dCnt, char *sWhere);
extern int dInsertMNIP(MYSQL *pstMySQL, st_NAS_db *stData);
extern int dGetMNIPCount(MYSQL *pstMySQL, int *dCnt);
extern int dGetCountMNIP(MYSQL *pstMySQL, unsigned int uMNIP, unsigned short huNetmask, unsigned short huFlag);
extern int dDeleteMNIP(MYSQL *pstMySQL, unsigned int uMNIP, unsigned short huNetmask, unsigned short huFlag);
extern int dGetEquipInfo(MYSQL *pstMySQL, st_InfoEquip_MMC *pstData, int *dCnt);
extern int dGetInfoEquipCount(MYSQL *pstMySQL, int *dCnt);
extern int dDeleteEquip(MYSQL *pstMySQL, unsigned int uiIP);
extern int dGetThres(MYSQL *pstMySQL, st_ThresMMC *pstData, int *dCnt, char *sWhere);
extern int dGetMONThres(MYSQL *pstMySQL, st_MON_ThresMMC *pstData, int *dCnt, char *sWhere);
extern int dIsAdminID(MYSQL *pstMySQL, char *sUserName);
extern int dGetCmdHistoryCount(MYSQL *pstMySQL, char *sUserName, time_t tStartTime, time_t tEndTime, int dIsAdmin, int *dCnt);
extern int dGetCommandList(MYSQL *pstMySQL, char *sUserName, st_Cmd *pstData, time_t tStartTime, time_t tEndTime, int dIsAdmin, int dLoopCnt);
extern int dGetCnt(MYSQL *pstMySQL, char *sCountTableSqlStmt, int *dCnt);
extern int dAddAdminInfo(MYSQL *pstMySQL, st_User_Add *stData);
extern int dDeleteAdminInfo(MYSQL *pstMySQL, st_User_Add *pstData);
extern int dSelectAdminInfo(MYSQL *pstMySQL, st_User_Add *pstData);
extern int dUpdateAdminInfo(MYSQL *pstMySQL, st_User_Add *pstData);
extern int dGetSysStat(MYSQL *pstMySQL, st_SYS_STAT *pstData, char *sQuery, int dType);
extern int dGetUserInfo(MYSQL *pstMySQL, st_UserAdd *pstData, int *dCnt);
extern int dInsertThres(MYSQL *pstMySQL, st_Thres *pstData);
extern int dGet_CONDMsg(MYSQL *pstMySQL, st_SysCONDMsg *pstData, time_t tSTime, time_t tETime, int *dCnt);
extern int dCommonDDL(MYSQL *pstMySQL, char *psCallFn, const char *psQuery);

extern int dGetTrafficStat(MYSQL *pstMySQL, st_traffic_stat *pstData, char *szQuery);
extern int dSelectThres(MYSQL *pstMySQL, st_ThresMMC *pstData);
extern int dChgThres(MYSQL *pstMySQL, st_ThresMMC *pstData);
extern int dSelectMONThres(MYSQL *pstMySQL, st_MON_ThresMMC *pstData);
extern int dChgMONThres(MYSQL *pstMySQL, st_MON_ThresMMC *pstData);
extern int dInsertMONThres(MYSQL *pstMySQL, st_MON_ThresMMC *pstData); /* added by dcham 2011.06.26 */
extern int dSyncInfoEquipTAMDB(MYSQL *pstMySQL, char *sFilePath, char *sFileName);
extern int dCheckStatistics(MYSQL *pstMySQL, time_t tStatTime);
extern int dInsert_CONDMsg(MYSQL *pstMySQL, st_SysCONDMsg *pstData);
extern int dCreate_CONDMsg(MYSQL *pstMySQL);
extern int dCheckHourStat(MYSQL *pstMySQL, time_t tStart, time_t tEnd);

#endif /* __DB_API_H__ */
