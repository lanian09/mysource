#include "fimd_proto.h"

#define INTERFACE_IS(x) (x==1 ? "SMSC" : (x==2 ? "VAS" : (x==3 ? "LMSC" : (x==4 ? "IDR" : (x==5 ? "SCP" : (x==6 ? "WISE" : "UNKNOWN"))))))
extern SFM_sfdb		*sfdb;
extern SFM_L3PD		*l3pd;
extern SFM_SCE		*g_pstSCEInfo;
/* hjjung */
//extern SFM_LEG		*g_pstLEGInfo;
extern SFM_CALL		*g_pstCALLInfo;
extern SFM_L2Dev    *g_pstL2Dev;
extern SFM_LOGON    g_stLogonRate[LOG_MOD_CNT][2];
extern SFM_LOGON    *g_pstLogonRate;
extern time_t		currentTime;
extern char		trcBuf[4096], trcTmp[1024];
extern int		trcFlag, trcLogId, trcLogFlag;
extern unsigned char       rmtdb_conn_lvl[SYSCONF_MAX_ASSO_SYS_NUM][SFM_MAX_DB_CNT];
extern int  g_hwswType[SFM_MAX_HPUX_HW_COM];

char	*rsrcName[SFM_MAX_RSRC_LOAD_CNT] = {
	"CDR_TCPIP", "CDR_SESSION" , "CDR" , NULL  , NULL  ,
	"TRCDR_SESSION", "WAP1_SESSION", "WAP2_SESSION", "HTTP_SESSION", NULL  ,
	"UDRGEN_CALL" , "VOD_SESSION" , "CDR2_TCPIP"  , "CDR2_SESSION"  , "CDR2" ,
	"VT_CALL"};


char    *nmsifName[5] = {"ALARM", "CONSOLE", "CONFIG", "MMC", "STATISTICS"}; // by helca 10.30
char	descBuf[100];
extern time_t  alarm_Time[SYSCONF_MAX_ASSO_SYS_NUM][RADIUS_IP_CNT];
extern int     chkCnt2[SYSCONF_MAX_ASSO_SYS_NUM][RADIUS_IP_CNT];

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
MYSQL	*mysql_conn, sql;
int fimd_mysql_init (void)
{
	mysql_init (&sql);

	if ((mysql_conn = mysql_real_connect (&sql, "localhost", "root", "mysql",
		SFM_ALM_DB_NAME, 0, 0, 0)) == NULL) {
		sprintf(trcBuf,">>> mysql_real_connect fail; err=%d:%s\n", mysql_errno(&sql), mysql_error(&sql));
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	return 1;

} //----- End of fimd_mysql_init -----//



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int fimd_mysql_query (char *query)
{

	if (mysql_query (mysql_conn, query) != 0) {
		sprintf(trcBuf,">>> mysql_query fail; err=%d:%s\n  query = %s\n", mysql_errno(mysql_conn), mysql_error(mysql_conn), query);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	return 1;

} //----- End of fimd_mysql_query -----//



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int fimd_saveCpuUsageAlmInfo2DB (int sysIndex, int cpuIndex, int almLevel, int occurFlag)
{

	char	query[1024];
	int almCode;

	// get alarm code
	//almCode = getAlarmCodeFromType(SFM_ALM_TYPE_CPU_USAGE);
	almCode = getAlarmCodeFromType(SFM_ALM_TYPE_CPU_USAGE, almLevel);

	//
	// alarm_history DB table에는 발생/해지에 관계없이 무조건 insert한다.
	//
	sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, %d, '%s', '%s')",
			SFM_ALM_HIS_DB_TABLE_NAME,
			sfdb->sys[sysIndex].commInfo.type,
			sfdb->sys[sysIndex].commInfo.group,
			sfdb->sys[sysIndex].commInfo.name,
			SFM_ALM_TYPE_CPU_USAGE,
			almCode,		// alarm code 추가 : sjjeon
			almLevel,
			occurFlag,
			commlib_printDateTime(currentTime),
			SQL_CPU_ALM_INFO
			);

	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_saveCpuUsageAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}


	//
	// current_alarm DB table에는 발생시 insert하고 해지시 delete한다.
	//

	//  이전 장애는 무조건 삭제한다.
	sprintf (query, "DELETE FROM %s WHERE (system_type='%s' AND system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='%s')",
			SFM_CURR_ALM_DB_TABLE_NAME,
			sfdb->sys[sysIndex].commInfo.type,
			sfdb->sys[sysIndex].commInfo.group,
			sfdb->sys[sysIndex].commInfo.name,
			SFM_ALM_TYPE_CPU_USAGE,
			SQL_CPU_ALM_INFO
			);
	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_saveCpuUsageAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}


	if (  occurFlag ){ // 장애가 발생했을 경우만 처리.
		sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, '%s', '%s', 0)",
				SFM_CURR_ALM_DB_TABLE_NAME,
				sfdb->sys[sysIndex].commInfo.type,
				sfdb->sys[sysIndex].commInfo.group,
				sfdb->sys[sysIndex].commInfo.name,
				SFM_ALM_TYPE_CPU_USAGE,
				almCode,        // alarm code 추가 : sjjeon
				almLevel,
				commlib_printDateTime(currentTime),
				SQL_CPU_ALM_INFO
				);

		if (fimd_mysql_query (query) < 0) {
			sprintf(trcBuf,"[fimd_saveCpuUsageAlmInfo2DB] fimd_mysql_query fail\n");
			trclib_writeLogErr (FL,trcBuf);
			return -1;
		}

	}
#if 0
	if(occurFlag == 0)
		almLevel = 5; // NMS에서는 alarm clear시 alarm_level이 '5'이다.
#else
	almLevel = getNmsLevel(almLevel,occurFlag);
#endif

	memset (descBuf, 0, 100);
	sprintf (descBuf, "%s%d", SQL_CPU_ALM_INFO, cpuIndex);
	fimd_txMsg2Nmsif (sfdb->sys[sysIndex].commInfo.name, SFM_ALM_TYPE_CPU_USAGE, almLevel, descBuf, currentTime);
	return 1;

} //----- End of fimd_saveCpuUsageAlmInfo2DB -----//



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int fimd_saveMemUsageAlmInfo2DB (int sysIndex, int almLevel, int occurFlag)
{

	char	query[1024];
	int almCode;

	//almCode = getAlarmCodeFromType(SFM_ALM_TYPE_MEMORY_USAGE);
	almCode = getAlarmCodeFromType(SFM_ALM_TYPE_MEMORY_USAGE, almLevel);

	sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, %d, '%s', 'Memory')",
			SFM_ALM_HIS_DB_TABLE_NAME,
			sfdb->sys[sysIndex].commInfo.type,
			sfdb->sys[sysIndex].commInfo.group,
			sfdb->sys[sysIndex].commInfo.name,
			SFM_ALM_TYPE_MEMORY_USAGE,
			almCode,        // alarm code 추가 : sjjeon
			almLevel,
			occurFlag,
			commlib_printDateTime(currentTime));

#if 0
		sprintf(trcBuf,"[fimd_saveProcAlmInfo2DB] ALM_HIS=%s\n", query);
		trclib_writeLogErr (FL,trcBuf);
#endif
	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_saveMemUsageAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	//  이전 장애는 무조건 삭제한다.
	sprintf (query, "DELETE FROM %s WHERE (system_type='%s' AND system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='Memory')",
				SFM_CURR_ALM_DB_TABLE_NAME,
				sfdb->sys[sysIndex].commInfo.type,
				sfdb->sys[sysIndex].commInfo.group,
				sfdb->sys[sysIndex].commInfo.name,
				SFM_ALM_TYPE_MEMORY_USAGE );

#if 0
		sprintf(trcBuf,"[fimd_saveProcAlmInfo2DB] CUR ALM=%s\n", query);
		trclib_writeLogErr (FL,trcBuf);
#endif
	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_saveMemUsageAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}


	if (  occurFlag ){ // 장애가 발생했을 경우만 처리.
		sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, '%s', 'Memory', 0)",
				SFM_CURR_ALM_DB_TABLE_NAME,
				sfdb->sys[sysIndex].commInfo.type,
				sfdb->sys[sysIndex].commInfo.group,
				sfdb->sys[sysIndex].commInfo.name,
				SFM_ALM_TYPE_MEMORY_USAGE,
				almCode,        // alarm code 추가 : sjjeon
				almLevel,
				commlib_printDateTime(currentTime));
#if 0
		sprintf(trcBuf,"[fimd_saveProcAlmInfo2DB] CUR ALM=%s\n", query);
		trclib_writeLogErr (FL,trcBuf);
#endif
		if (fimd_mysql_query (query) < 0) {
			sprintf(trcBuf,"[fimd_saveMemUsageAlmInfo2DB] fimd_mysql_query fail\n");
			trclib_writeLogErr (FL,trcBuf);
			return -1;
		}
	}

#if 0
    if(occurFlag == 0)
        almLevel = 5; // NMS에서는 alarm clear시 alarm_level이 '5'이다.
#else
    almLevel = getNmsLevel(almLevel,occurFlag);
#endif

	memset (descBuf, 0, 100);
	sprintf (descBuf, "%s", "Memory");
	fimd_txMsg2Nmsif (sfdb->sys[sysIndex].commInfo.name, SFM_ALM_TYPE_MEMORY_USAGE, almLevel, descBuf, currentTime);


	return 1;

} //----- End of fimd_saveMemUsageAlmInfo2DB -----//



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int fimd_saveDiskUsageAlmInfo2DB (int sysIndex, int diskIndex, int almLevel, int occurFlag)
{

	char	query[1024];
	int almCode;

	//almCode = getAlarmCodeFromType(SFM_ALM_TYPE_DISK_USAGE);
	almCode = getAlarmCodeFromType(SFM_ALM_TYPE_DISK_USAGE, almLevel);

	sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, %d, '%s', '%s')",
			SFM_ALM_HIS_DB_TABLE_NAME,
			sfdb->sys[sysIndex].commInfo.type,
			sfdb->sys[sysIndex].commInfo.group,
			sfdb->sys[sysIndex].commInfo.name,
			SFM_ALM_TYPE_DISK_USAGE,
			almCode,        // alarm code 추가 : sjjeon
			almLevel,
			occurFlag,
			commlib_printDateTime(currentTime),
			sfdb->sys[sysIndex].commInfo.diskInfo[diskIndex].name);

	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_saveDiskUsageAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	//  이전 장애는 무조건 삭제한다.
	sprintf (query, "DELETE FROM %s WHERE (system_type='%s' AND system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='%s')",
				SFM_CURR_ALM_DB_TABLE_NAME,
				sfdb->sys[sysIndex].commInfo.type,
				sfdb->sys[sysIndex].commInfo.group,
				sfdb->sys[sysIndex].commInfo.name,
				SFM_ALM_TYPE_DISK_USAGE,
				sfdb->sys[sysIndex].commInfo.diskInfo[diskIndex].name);

	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_saveDiskUsageAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}
	//fprintf (stderr,"fimd_saveDiskUsageAlmInfo2DB[%d] almlevel = %d\n", sysIndex, almLevel );


	if ( occurFlag ){ // 장애가 발생했을 경우만 처리.
		sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, '%s', '%s', 0)",
				SFM_CURR_ALM_DB_TABLE_NAME,
				sfdb->sys[sysIndex].commInfo.type,
				sfdb->sys[sysIndex].commInfo.group,
				sfdb->sys[sysIndex].commInfo.name,
				SFM_ALM_TYPE_DISK_USAGE,
				almCode,        // alarm code 추가 : sjjeon
				almLevel,
				commlib_printDateTime(currentTime),
				sfdb->sys[sysIndex].commInfo.diskInfo[diskIndex].name
				);

		if (fimd_mysql_query (query) < 0) {
			sprintf(trcBuf,"[fimd_saveDiskUsageAlmInfo2DB] fimd_mysql_query fail\n");
			trclib_writeLogErr (FL,trcBuf);
			return -1;
		}
	}

#if 0
    if(occurFlag == 0)
        almLevel = 5; // NMS에서는 alarm clear시 alarm_level이 '5'이다.
#else
    almLevel = getNmsLevel(almLevel,occurFlag);
#endif

	memset (descBuf, 0, 100);
	sprintf (descBuf, "%s%d", sfdb->sys[sysIndex].commInfo.diskInfo[diskIndex].name, diskIndex);
	fimd_txMsg2Nmsif (sfdb->sys[sysIndex].commInfo.name, SFM_ALM_TYPE_DISK_USAGE, almLevel, descBuf, currentTime);


	return 1;

} //----- End of fimd_saveDiskUsageAlmInfo2DB -----//



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int fimd_saveLanAlmInfo2DB (int sysIndex, int lanIndex, int almLevel, int occurFlag)
{
    int  alarm_Level = SFM_ALM_CRITICAL;
    char    query[1024];
	int almCode;

	//almCode = getAlarmCodeFromType(SFM_ALM_TYPE_LAN);
	almCode = getAlarmCodeFromType(SFM_ALM_TYPE_LAN, almLevel);

    sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, %d, '%s', '%s(%s)')",
            SFM_ALM_HIS_DB_TABLE_NAME,
            sfdb->sys[sysIndex].commInfo.type,
            sfdb->sys[sysIndex].commInfo.group,
            sfdb->sys[sysIndex].commInfo.name,
            SFM_ALM_TYPE_LAN,
			almCode,        // alarm code 추가 : sjjeon
            almLevel,
            occurFlag,
            commlib_printDateTime(currentTime),
            sfdb->sys[sysIndex].commInfo.lanInfo[lanIndex].name,
            sfdb->sys[sysIndex].commInfo.lanInfo[lanIndex].targetIp);

    if (fimd_mysql_query (query) < 0) {
        sprintf(trcBuf,"[fimd_saveLanAlmInfo2DB] fimd_mysql_query fail\n");
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }

    if (occurFlag) {
        sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, '%s', '%s(%s)', 0)",
                SFM_CURR_ALM_DB_TABLE_NAME,
                sfdb->sys[sysIndex].commInfo.type,
                sfdb->sys[sysIndex].commInfo.group,
                sfdb->sys[sysIndex].commInfo.name,
                SFM_ALM_TYPE_LAN,
				almCode,        // alarm code 추가 : sjjeon
                alarm_Level,
                commlib_printDateTime(currentTime),
                sfdb->sys[sysIndex].commInfo.lanInfo[lanIndex].name,
                sfdb->sys[sysIndex].commInfo.lanInfo[lanIndex].targetIp);
    } else {
        sprintf (query, "DELETE FROM %s WHERE (system_type='%s' AND system_group='%s' AND system_name='%s' AND alarm_type=%d AND alarm_level=%d AND information='%s(%s)')",
                SFM_CURR_ALM_DB_TABLE_NAME,
                sfdb->sys[sysIndex].commInfo.type,
                sfdb->sys[sysIndex].commInfo.group,
                sfdb->sys[sysIndex].commInfo.name,
                SFM_ALM_TYPE_LAN,
                alarm_Level,
                sfdb->sys[sysIndex].commInfo.lanInfo[lanIndex].name,
                sfdb->sys[sysIndex].commInfo.lanInfo[lanIndex].targetIp);
    }

    if (fimd_mysql_query (query) < 0) {
        sprintf(trcBuf,"[fimd_saveLanAlmInfo2DB] fimd_mysql_query fail\n");
        trclib_writeLogErr (FL,trcBuf);
        return -1;
	}

#if 0
    if(occurFlag == 0)
        almLevel = 5; // NMS에서는 alarm clear시 alarm_level이 '5'이다.
#else
    almLevel = getNmsLevel(almLevel,occurFlag);
#endif

    memset (descBuf, 0, 100);
    sprintf (descBuf, "%s%d", sfdb->sys[sysIndex].commInfo.lanInfo[lanIndex].name, lanIndex);
    fimd_txMsg2Nmsif (sfdb->sys[sysIndex].commInfo.name, SFM_ALM_TYPE_LAN, almLevel, descBuf, currentTime);

    return 1;
} //----- End of fimd_saveLanAlmInfo2DB -----//

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int fimd_saveCpsAlmInfo2DB (int sysIndex, int lanIndex, int almLevel, int occurFlag)
{
	int	 alarm_Level = SFM_ALM_MINOR;
	char	query[1024];
	int almCode;

	//almCode = getAlarmCodeFromType(SFM_ALM_TYPE_CPS_OVER);
	almCode = getAlarmCodeFromType(SFM_ALM_TYPE_CPS_OVER, almLevel);

	sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, %d, '%s', '%s')",
			SFM_ALM_HIS_DB_TABLE_NAME,
			sfdb->sys[sysIndex].commInfo.type,
			sfdb->sys[sysIndex].commInfo.group,
			sfdb->sys[sysIndex].commInfo.name,
			SFM_ALM_TYPE_CPS_OVER,
			almCode,        // alarm code 추가 : sjjeon
			almLevel,
			occurFlag,
			commlib_printDateTime(currentTime),
			"OvER_CPS");

	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_saveCpsAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	if (occurFlag) {
		sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, '%s', '%s', 0)",
				SFM_CURR_ALM_DB_TABLE_NAME,
				sfdb->sys[sysIndex].commInfo.type,
				sfdb->sys[sysIndex].commInfo.group,
				sfdb->sys[sysIndex].commInfo.name,
				SFM_ALM_TYPE_CPS_OVER,
				almCode,        // alarm code 추가 : sjjeon
				alarm_Level,
				commlib_printDateTime(currentTime),
				"OvER_CPS");
	} else {
		sprintf (query, "DELETE FROM %s WHERE (system_type='%s' AND system_group='%s' AND system_name='%s' AND alarm_type=%d AND alarm_level=%d AND information='%s')",
				SFM_CURR_ALM_DB_TABLE_NAME,
				sfdb->sys[sysIndex].commInfo.type,
				sfdb->sys[sysIndex].commInfo.group,
				sfdb->sys[sysIndex].commInfo.name,
				SFM_ALM_TYPE_CPS_OVER,
				alarm_Level,
				"OvER_CPS");
	}

	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_saveCpsAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

#if 0
    if(occurFlag == 0)
        almLevel = 5; // NMS에서는 alarm clear시 alarm_level이 '5'이다.
#else
    almLevel = getNmsLevel(almLevel,occurFlag);
#endif

	memset (descBuf, 0, 100);
	sprintf (descBuf, "%s%d", sfdb->sys[sysIndex].commInfo.lanInfo[lanIndex].name, lanIndex);
	fimd_txMsg2Nmsif (sfdb->sys[sysIndex].commInfo.name, SFM_ALM_TYPE_CPS_OVER, almLevel, descBuf, currentTime);

	return 1;

} //----- End of fimd_saveCpsAlmInfo2DB-----//

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int fimd_saveDbRepGapAlmInfo2DB (int sysIndex, int almLevel, int occurFlag)
{

	char	query[1024];
	int almCode;

	//almCode = getAlarmCodeFromType(SFM_ALM_TYPE_DB_REP_GAP);
	almCode = getAlarmCodeFromType(SFM_ALM_TYPE_DB_REP_GAP,almLevel);

	sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, %d, '%s', '%s')",
			SFM_ALM_HIS_DB_TABLE_NAME,
			sfdb->sys[sysIndex].commInfo.type,
			sfdb->sys[sysIndex].commInfo.group,
			sfdb->sys[sysIndex].commInfo.name,
			SFM_ALM_TYPE_DB_REP_GAP,
			almCode,        // alarm code 추가 : sjjeon
			almLevel,
			occurFlag,
			commlib_printDateTime(currentTime),
			SQL_REPLGAP_ALM_INFO);


	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_saveDbRepGapAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	//  이전 장애는 무조건 삭제한다.
	sprintf (query, "DELETE FROM %s WHERE (system_type='%s' AND system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='%s')",
			SFM_CURR_ALM_DB_TABLE_NAME,
			sfdb->sys[sysIndex].commInfo.type,
			sfdb->sys[sysIndex].commInfo.group,
			sfdb->sys[sysIndex].commInfo.name,
			SFM_ALM_TYPE_DB_REP_GAP,
			SQL_REPLGAP_ALM_INFO);

	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_saveDbRepGapAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}


	if ( occurFlag ){ // 장애가 발생했을 경우만 처리.
		sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, '%s', '%s', 0)",
				SFM_CURR_ALM_DB_TABLE_NAME,
				sfdb->sys[sysIndex].commInfo.type,
				sfdb->sys[sysIndex].commInfo.group,
				sfdb->sys[sysIndex].commInfo.name,
				SFM_ALM_TYPE_DB_REP_GAP,
				almCode,        // alarm code 추가 : sjjeon
				almLevel,
				commlib_printDateTime(currentTime),
				SQL_REPLGAP_ALM_INFO);

		if (fimd_mysql_query (query) < 0) {
			sprintf(trcBuf,"[fimd_saveDbRepGapAlmInfo2DB] fimd_mysql_query fail\n");
			fprintf(stderr,"[fimd_saveDbRepGapAlmInfo2DB] insert fimd_mysql_query fail\n");
			trclib_writeLogErr (FL,trcBuf);
			return -1;
		}
	}


	return 1;

} //----- End of fimd_saveDbRepGapAlmInfo2DB -----//


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int fimd_saveProcAlmInfo2DB (int sysIndex, int procIndex, int almLevel, int occurFlag)
{

	char	query[1024];
	int almCode, almType;

#ifndef _NOT_USED_PROCESS_ALARM_TYPE_
	/* process 이름으로 Alarm type 획득*/
	almType = getAlarmTypeFromProcName(sysIndex, procIndex);
	/* Alarm type으로 Alarm code 획득 */
	//almCode = getAlarmCodeFromType(almType);
	almCode = getAlarmCodeFromType(almType, almLevel);

	if(almCode<0){
		sprintf(trcBuf,"[%s] Invalid alarm code.(%d)\n", __FUNCTION__, almCode);
		return -1;
	}
	//fprintf(stderr,"[%s] sysIndex: %d, procIndex: %d\n",__FUNCTION__, sysIndex, procIndex);
	//fprintf(stderr,"[%s] almType : %d, almCode : %d\n",__FUNCTION__, almType, almCode);
#else
	almType = SFM_ALM_TYPE_PROC;
	//almCode = getAlarmCodeFromType(SFM_ALM_TYPE_PROC);
	almCode = getAlarmCodeFromType(SFM_ALM_TYPE_PROC, almLevel);
#endif
	if(almCode<0){
		sprintf(trcBuf,"[%s] Invalid alarm code.(%d)\n", __FUNCTION__, almCode);
		return -1;
	}

	sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, %d, '%s', '%s')",
			SFM_ALM_HIS_DB_TABLE_NAME,
			sfdb->sys[sysIndex].commInfo.type,
			sfdb->sys[sysIndex].commInfo.group,
			sfdb->sys[sysIndex].commInfo.name,
			almType,
			almCode,        // alarm code 추가 : sjjeon
			almLevel,
			occurFlag,
			commlib_printDateTime(currentTime),
			sfdb->sys[sysIndex].commInfo.procInfo[procIndex].name);
/* DEBUG: by june, 2010-10-07
 * DESC : #if 1 LOG 추가.
 */
#if 0
		sprintf(trcBuf,"[fimd_saveProcAlmInfo2DB] ALM_HIS=%s\n", query);
		trclib_writeLogErr (FL,trcBuf);
#endif
	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_saveProcAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}
	sprintf (query, "DELETE FROM %s WHERE (system_type='%s' AND system_group='%s' AND system_name='%s' AND alarm_type=%d AND alarm_level=%d AND information='%s')",
			SFM_CURR_ALM_DB_TABLE_NAME,
			sfdb->sys[sysIndex].commInfo.type,
			sfdb->sys[sysIndex].commInfo.group,
			sfdb->sys[sysIndex].commInfo.name,
			almType,
			almLevel,
			sfdb->sys[sysIndex].commInfo.procInfo[procIndex].name);
#if 1
	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_saveProcAlmInfo2DB] fimd_mysql_delete query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}
#endif

	if (occurFlag) {
		sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, '%s', '%s', 0)",
				SFM_CURR_ALM_DB_TABLE_NAME,
				sfdb->sys[sysIndex].commInfo.type,
				sfdb->sys[sysIndex].commInfo.group,
				sfdb->sys[sysIndex].commInfo.name,
				almType,
				almCode,        // alarm code 추가 : sjjeon
				almLevel,
				commlib_printDateTime(currentTime),
				sfdb->sys[sysIndex].commInfo.procInfo[procIndex].name);
		if (fimd_mysql_query (query) < 0) {
			sprintf(trcBuf,"[fimd_saveProcAlmInfo2DB] fimd_mysql_insert query fail\n");
			trclib_writeLogErr (FL,trcBuf);
			return -1;
		}
	}
/* DEBUG: by june, 2010-10-07
 * DESC : #if 1 LOG 추가.
 */
#if 0
		sprintf(trcBuf,"[fimd_saveProcAlmInfo2DB] CUR ALM=%s\n", query);
		trclib_writeLogErr (FL,trcBuf);
#endif

#if 0
    if(occurFlag == 0)
        almLevel = 5; // NMS에서는 alarm clear시 alarm_level이 '5'이다.
#else
    almLevel = getNmsLevel(almLevel,occurFlag);
#endif

	memset (descBuf, 0, 100);
	sprintf (descBuf, "%s", sfdb->sys[sysIndex].commInfo.procInfo[procIndex].name);
	//fimd_txMsg2Nmsif (sfdb->sys[sysIndex].commInfo.name, SFM_ALM_TYPE_PROC, almLevel, descBuf, currentTime);
	fimd_txMsg2Nmsif (sfdb->sys[sysIndex].commInfo.name, almType, almLevel, descBuf, currentTime);

	return 1;

} //----- End of fimd_saveProcAlmInfo2DB -----//


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int fimd_saveHwComAlmInfo2DB (int sysIndex, SFM_HpUxHWInfo *hwInfo, int comIndex, int almLevel, int occurFlag)
{
	int	 len;
	char	hwname[16];
	char	query[1024];
	int almCode, almType;

	if(g_hwswType[comIndex]==LOC_HW_MIRROR_TYPE){
		//almCode = getAlarmCodeFromType(SFM_ALM_TYPE_HW_MIRROR);
		almCode = getAlarmCodeFromType(SFM_ALM_TYPE_HW_MIRROR, almLevel);
		almType = SFM_ALM_TYPE_HW_MIRROR;
	}else{
		//almCode = getAlarmCodeFromType(SFM_ALM_TYPE_MP_HW);
		almCode = getAlarmCodeFromType(SFM_ALM_TYPE_MP_HW, almLevel);
		almType = SFM_ALM_TYPE_MP_HW;
	}
	//fprintf(stderr, "[%s] almcode = %d, idx: %d, type : %d \n", __FUNCTION__, almCode, comIndex, g_hwswType[comIndex] );

	if(almCode<0){
		sprintf(trcBuf,"[%s] Invalid alarm code.(%d)\n", __FUNCTION__, almCode);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	len = strlen(hwInfo->hwcom[comIndex].name);
	hw_name_mapping(hwInfo->hwcom[comIndex].name, len, hwname);
	
	sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, %d, '%s', '%s')",
			SFM_ALM_HIS_DB_TABLE_NAME,
			sfdb->sys[sysIndex].commInfo.type,
			sfdb->sys[sysIndex].commInfo.group,
			sfdb->sys[sysIndex].commInfo.name,
			almType,
			almCode,        // alarm code 추가 : sjjeon
			almLevel,
			occurFlag,
			commlib_printDateTime(currentTime),
			hwname);

	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_saveHwComAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}


	if (occurFlag) {
		sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, '%s', '%s', 0)",
				SFM_CURR_ALM_DB_TABLE_NAME,
				sfdb->sys[sysIndex].commInfo.type,
				sfdb->sys[sysIndex].commInfo.group,
				sfdb->sys[sysIndex].commInfo.name,
				almType,
				almCode,        // alarm code 추가 : sjjeonn
				almLevel,
				commlib_printDateTime(currentTime),
				hwname);
	} else {
		sprintf (query, "DELETE FROM %s WHERE (system_type='%s' AND system_group='%s' AND system_name='%s' AND alarm_type=%d AND alarm_level=%d AND information='%s')",
				SFM_CURR_ALM_DB_TABLE_NAME,
				sfdb->sys[sysIndex].commInfo.type,
				sfdb->sys[sysIndex].commInfo.group,
				sfdb->sys[sysIndex].commInfo.name,
				almType,
				almLevel,
				hwname);
	}
	// Query print
	//fprintf(stderr,"[%s] query : %s\n", __FUNCTION__, query);

	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_saveHwComAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

#if 0
    if(occurFlag == 0)
        almLevel = 5; // NMS에서는 alarm clear시 alarm_level이 '5'이다.
#else
    almLevel = getNmsLevel(almLevel,occurFlag);
#endif

	//memset (descBuf, 0, 100);
	//sprintf (descBuf, "%s%d", hwInfo->hwcom[comIndex].name, comIndex); // e1000g33 이 찍힘..
	//fimd_txMsg2Nmsif (sfdb->sys[sysIndex].commInfo.name, SFM_ALM_TYPE_MP_HW, almLevel, descBuf, currentTime);
	//sprintf (descBuf, "%s", hwInfo->hwcom[comIndex].name);

	// sjjeon
	fimd_txMsg2Nmsif (sfdb->sys[sysIndex].commInfo.name, almType, almLevel, 
						hwInfo->hwcom[comIndex].name, currentTime);

	return 1;

} //----- End of fimd_saveHwComAlmInfo2DB -----//


//------------------------------------------------------------------------------
// 주기적으로 호출되어 alarm_history DB에 들어있는 오래된 놈들을 지운다.
//------------------------------------------------------------------------------
int fimd_deleteOldAlmInfoDB (void)
{

	char	query[1024];

	sprintf (query, "DELETE FROM %s WHERE (alarm_date < '%s')",
			SFM_ALM_HIS_DB_TABLE_NAME,
			commlib_printDateTime(currentTime - FIMD_DELETE_PERIOD_OLD_ALARM_HISTORY_DB));

	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_deleteOldAlmInfoDB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}
//sprintf(trcBuf,"[fimd_deleteOldAlmInfoDB] fimd_mysql_query success !!\n");
//trclib_writeLogErr (FL,trcBuf);
	
	fimd_broadcastAlmEvent2Client ();

	return 1;

} //----- End of fimd_deleteOldAlmInfoDB -----//

// 현재 MYSQL 알람과 실제 상태를 확인하는 audit기능 수행
// 
int fimd_checkCurrentAlarm ()
{
    int     	i,j;
    int     	select_cnt;
    int     	rowNum;
    int     	sysIndex;
    int			devIndex;
    int     	almType;
    int     	almLevel, intValue;
    int     	findKey,mask;
    int     	deleteFlag,chgFlag;
    char    	query[1024],temp[128], *info;
    char		hwname[16], temp1[50];
    MYSQL_RES  	*result;
    MYSQL_ROW  	row;

    //
    // 모든 Alarm 정보를 조회한다
    //
    select_cnt = 0;

    sprintf(query, "SELECT * FROM %s", SFM_CURR_ALM_DB_TABLE_NAME);

    if (fimd_mysql_query (query) < 0) {
        sprintf(trcBuf,"[fimd_checkCurrentAlarm] fimd_mysql_query fail\n");
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }

    //
    // 현재 Alarm 이 유효한지 확인하고 필요 없으면 날려 보내자.
    result = mysql_store_result(mysql_conn);
    rowNum = mysql_num_rows(result);
    while((row = mysql_fetch_row(result)) != NULL)
    {
#if 0
// MODIFY: by june, 2010-09-09
sprintf(trcBuf,"[fimd_checkCurrentAlarm] row=%s\n", row[2]);
trclib_writeLogErr (FL,trcBuf);
#endif
        if(rowNum <= 0)
            break;

        // Flag Setting
        deleteFlag = chgFlag = 0;

		sysIndex = devIndex = -1;

        // Alarm 정보를 읽는다 
        if(!strcasecmp(row[2], "TAPA"))
			devIndex = 0;
		else if (!strcasecmp(row[2], "TAPB"))
			devIndex = 1;
		else if (!strcasecmp(row[2], "SCEA"))
			devIndex = 0;
		else if (!strcasecmp(row[2], "SCEB"))
			devIndex = 1;
		/* hjjung */
//		else if (!strcasecmp(row[2], "SCMA"))
//			devIndex = 0;
//		else if (!strcasecmp(row[2], "SCMB"))
//			devIndex = 1;
		else if (!strcasecmp(row[2], "L2SWA"))
			devIndex = 0;
		else if (!strcasecmp(row[2], "L2SWB"))
			devIndex = 1;
		else{
			if((sysIndex = fimd_getSysIndexByName(row[2])) < 0){
			deleteFlag =1;
			goto DELETE_RTN;
		}
	}

	almType  = atoi(row[3]);
	almLevel = atoi(row[5]);
	info	 = row[7];
	mask	 = atoi(row[8]);

	// mask 는 그냥 넘어간다.
	if ( mask ) continue;

	// 현재 발생한 Alarm 과 동일한지 보자
	// 알람 등급이 다를 경우 일단 삭제하자
	switch(almType) {
		case SFM_ALM_TYPE_CPU_USAGE:
			// information에서 cpu index를 가져온다. 
			findKey = atoi(&info[strlen(SQL_CPU_ALM_INFO)]);
			if(sfdb->sys[sysIndex].commInfo.cpuInfo.level[0] != almLevel)
				deleteFlag = 1;
			break;

		case SFM_ALM_TYPE_MEMORY_USAGE:
			if(sfdb->sys[sysIndex].commInfo.memInfo.level != almLevel)
				deleteFlag = 1;
			break;

		case SFM_ALM_TYPE_DISK_USAGE:
			if( sysIndex < 0 ){
				sprintf(trcBuf,"[fimd_checkCurrentAlarm][CRI] ALM_TYPE_DISK_USAGE, sysIndex ZERO UNDER\n");
				trclib_writeLogErr (FL, trcBuf);
				continue;
			}

			for (i=0; i<SFM_MAX_DISK_CNT; i++){
				if ( !strcasecmp ( info, sfdb->sys[sysIndex].commInfo.diskInfo[i].name) )
					break;
			}

			if ( i>=SFM_MAX_DISK_CNT ) deleteFlag = 1;
			else {
				if(sfdb->sys[sysIndex].commInfo.diskInfo[i].level != almLevel)
					deleteFlag = 1;
			}
			break;

		case SFM_ALM_TYPE_LAN:
			// LAN INFO "name(target_ip)"로 구성
			for (i=0; i<SFM_MAX_LAN_CNT; i++){
				sprintf(temp, "%s(%s)", sfdb->sys[sysIndex].commInfo.lanInfo[i].name,
										sfdb->sys[sysIndex].commInfo.lanInfo[i].targetIp );
				if ( !strcasecmp ( info, temp))
						break;
			}
			if ( i>=SFM_MAX_LAN_CNT )
				deleteFlag = 1;
			else {
				if(sfdb->sys[sysIndex].commInfo.lanInfo[i].level != almLevel)
					deleteFlag = 1;
			}
			break;
		/* by helca */
		case SFM_ALM_TYPE_RMT_LAN:
			// LAN INFO "name(target_ip)"로 구성
			for (i=0; i<sfdb->sys[sysIndex].commInfo.rmtLanCnt; i++){
				sprintf(temp, "%s(%s)", sfdb->sys[sysIndex].commInfo.rmtLanInfo[i].name,
										sfdb->sys[sysIndex].commInfo.rmtLanInfo[i].targetIp );
/* DEBUG: by june, 2010-10-07
 * DESC : LOG ADD
 */
#if 0
				sprintf(trcBuf,"[fimd_checkCurrentAlarm] SFM_ALM_TYPE_RMT_LAN, DB[%s] MEM[%s]\n", info, temp); 
				trclib_writeLogErr (FL,trcBuf);
#endif
				if ( !strcasecmp ( info, temp))
						break;
			}
			if ( i>=sfdb->sys[sysIndex].commInfo.rmtLanCnt ) deleteFlag = 1;
			else {
				if(sfdb->sys[sysIndex].commInfo.rmtLanInfo[i].level != almLevel)
					deleteFlag = 1;
			}
			
			break;
		case SFM_ALM_TYPE_OPT_LAN:
			// LAN INFO "name"로 구성
			for (i=0; i<SFM_MAX_LAN_CNT; i++){
				if ( !strcasecmp ( info, sfdb->sys[sysIndex].commInfo.optLanInfo[i].name))
						break;
			}
			if ( i>=SFM_MAX_LAN_CNT ) deleteFlag = 1;
			else {
				if(sfdb->sys[sysIndex].commInfo.optLanInfo[i].level != almLevel)
					deleteFlag = 1;
			}

			break;
		//case SFM_ALM_TYPE_PD_CPU_USAGE:
		case SFM_ALM_TYPE_TAP_CPU_USAGE:
			if(l3pd->l3ProbeDev[devIndex].cpuInfo.level != almLevel)
				deleteFlag = 1;
			break;
	
		//case SFM_ALM_TYPE_PD_MEMORY_USAGE:
		case SFM_ALM_TYPE_TAP_MEMORY_USAGE:
			if(l3pd->l3ProbeDev[devIndex].memInfo.level != almLevel)
				deleteFlag = 1;
			break;
		//case SFM_ALM_TYPE_PD_FAN_STS:
		case SFM_ALM_TYPE_TAP_FAN_STS:
			for(i=0; i<4; i++){
				if(l3pd->l3ProbeDev[devIndex].fanInfo.level[i] != almLevel)
					deleteFlag = 1;
			}	
			break;
		//case SFM_ALM_TYPE_PD_GIGA_LAN:
		case SFM_ALM_TYPE_TAP_PORT_STS:
			for(i=0; i<52; i++){
				sprintf(temp, "%s(%d)",SQL_TAP_PORT_ALM_INFO, i+1);
				if(!strcasecmp(temp, info)){
					if(l3pd->l3ProbeDev[devIndex].gigaLanInfo[i].level != almLevel)
						deleteFlag = 1;
					break;
				}
			}
			if(i >= 52)
				deleteFlag = 1;
			break;
		case SFM_ALM_TYPE_TAP_POWER_STS: // 20110424 by dcham
			for(i=0; i<2; i++){
				sprintf(temp, "%s(%d)",SQL_TAP_POWER_ALM_INFO, i+1);
				if(!strcasecmp(temp, info)){
				if(l3pd->l3ProbeDev[devIndex].powerInfo[i].level != almLevel)
					deleteFlag = 1;
				}
			}   
			break;
		case SFM_ALM_TYPE_SCE_CPU:
			for(i=0;i<MAX_SCE_CPU_CNT;i++){
				sprintf(temp1,"%s(%d)",SQL_SCE_CPU_ALM_INFO, i+1);
				if(!strcasecmp(info,temp1)){
					if(g_pstSCEInfo->SCEDev[devIndex].cpuInfo[i].level != almLevel)
						deleteFlag = 1;
					break;
				}
			}
			if(i >= MAX_SCE_CPU_CNT)
				deleteFlag = 1;
			break;

		case SFM_ALM_TYPE_SCE_MEM:
			for(i=0;i<MAX_SCE_MEM_CNT;i++){
				sprintf(temp1,"%s(%d)",SQL_SCE_MEM_ALM_INFO,i+1);
				if(!strcasecmp(info,temp1)){
					if(g_pstSCEInfo->SCEDev[devIndex].memInfo[i].level != almLevel)
						deleteFlag = 1;
					break;
				}
			}
			if(i >= MAX_SCE_MEM_CNT)
				deleteFlag = 1;
			break;

		case SFM_ALM_TYPE_SCE_DISK:
			if(g_pstSCEInfo->SCEDev[devIndex].diskInfo.level != almLevel)
				deleteFlag = 1;
			break;

		case SFM_ALM_TYPE_SCE_PWR:
			if(g_pstSCEInfo->SCEDev[devIndex].pwrStatus.level != almLevel)
				deleteFlag = 1;
			break;

// yhshin
		case SFM_ALM_TYPE_SCE_FAN:
			if(g_pstSCEInfo->SCEDev[devIndex].fanStatus.level != almLevel)
				deleteFlag = 1;
			break;
		case SFM_ALM_TYPE_SCE_TEMP:
			if(g_pstSCEInfo->SCEDev[devIndex].tempStatus.level != almLevel)
				deleteFlag = 1;
			break;

		case SFM_ALM_TYPE_SCE_VOLT:
			if(g_pstSCEInfo->SCEDev[devIndex].voltStatus.level != almLevel)
				deleteFlag = 1;
			break;

		case SFM_ALM_TYPE_SCE_RDR:
/*
			for(i=0;i<MAX_SCE_RDR_INFO_CNT;i++){
				if(g_pstSCEInfo->SCEDev[devIndex].rdrStatus[i].level != almLevel)
					deleteFlag = 1;
			}
*/
			break;

		case SFM_ALM_TYPE_SCE_RDR_CONN:
			// by sjjeon
			// DB에 저장된 INFO와 비교하여 동일한 INFO 내용을 찾은후, 레벨을 비교한다.
			for(i=0;i<MAX_SCE_RDR_INFO_CNT;i++){
				sprintf(temp1,"%s(%d)", SQL_SCE_RDR_CONN_ALM_INFO,i+1);
				if(!strcasecmp(info,temp1)){
					if(g_pstSCEInfo->SCEDev[devIndex].rdrConnStatus[i].level != almLevel){
				//		fprintf(stderr,"%s) lv: %d <-> %d\n",  
				//				temp1, g_pstSCEInfo->SCEDev[devIndex].rdrConnStatus[i].level, almLevel);
						deleteFlag = 1;
					}
					break;
				}
			}

			break;

		//by sjjeon			
		case SFM_ALM_TYPE_SCE_STATUS:
			if(g_pstSCEInfo->SCEDev[devIndex].sysStatus.level != almLevel)
				deleteFlag = 1;
			break;

		case SFM_ALM_TYPE_SCE_PORT_MGMT:
		case SFM_ALM_TYPE_SCE_PORT_LINK:

			for (i=0; i < MAX_SCE_IFN_CNT; i++) {
				if (!strcasecmp(info, (char*)fimd_getScePortName(i))) {
					if(g_pstSCEInfo->SCEDev[devIndex].portStatus[i].level != almLevel) {
						//fprintf(stderr,"i:%d level: %d, almLevle: %d\n", i, g_pstSCEInfo->SCEDev[devIndex].portStatus[i].level, almLevel);
						deleteFlag = 1;
					}
					break;
				}
			}

			if(i >= MAX_SCE_IFN_CNT)
				deleteFlag = 1;

			break;

		case SFM_ALM_TYPE_SUCC_RATE:
       		if(!strstr(info, "RADIUS")){
				for(i=0; i<SFM_REAL_SUCC_RATE_CNT; i++){
					if( !strcasecmp((char*)sfdb->sys[sysIndex].commInfo.succRate[i].name, "UAWAP")) {
						continue;	
					}
					else if( !strcasecmp((char*)sfdb->sys[sysIndex].commInfo.succRate[i].name, "AAA")) {
						continue;
					}
					else if( !strcasecmp((char*)sfdb->sys[sysIndex].commInfo.succRate[i].name, "ANAAA")) {
						continue;
					}
					else if (strstr(info, (char*)sfdb->sys[sysIndex].commInfo.succRate[i].name)){
						if(sfdb->sys[sysIndex].commInfo.succRate[i].level != almLevel) {
							deleteFlag = 1;
						}
						break;
					}
				}
			}else{
				char temp1[80];
				for(j=0; j<RADIUS_IP_CNT; j++){
					sprintf(temp1, "RADIUS(%s)", sfdb->sys[sysIndex].succRateIpInfo.radius[j].ipAddr);
					
					if(strstr(info, temp1)){
						if (sfdb->sys[sysIndex].succRateIpInfo.radius[j].level != almLevel)
							deleteFlag = 1;
						break;
					}
				}
			}
       		//if( (i >= SFM_REAL_SUCC_RATE_CNT) || (j >= RADIUS_IP_CNT))
          	//	deleteFlag = 1;

			if(j >= RADIUS_IP_CNT)
				deleteFlag = 1;
        		break;	

		case SFM_ALM_TYPE_PROC:
		case SFM_ALM_TYPE_PROCESS_SAMD:
		case SFM_ALM_TYPE_PROCESS_IXPC:
		case SFM_ALM_TYPE_PROCESS_FIMD:
		case SFM_ALM_TYPE_PROCESS_COND:
		case SFM_ALM_TYPE_PROCESS_STMD:
		case SFM_ALM_TYPE_PROCESS_MMCD:
		case SFM_ALM_TYPE_PROCESS_MCDM:
		case SFM_ALM_TYPE_PROCESS_NMSIF:
		case SFM_ALM_TYPE_PROCESS_CDELAY:
		case SFM_ALM_TYPE_PROCESS_HAMON:
		case SFM_ALM_TYPE_PROCESS_MMCR:
		case SFM_ALM_TYPE_PROCESS_RDRANA:
		case SFM_ALM_TYPE_PROCESS_RLEG0: /* RLEG0~4 added by dcham 2011.04.26 */
		case SFM_ALM_TYPE_PROCESS_RLEG1:
		case SFM_ALM_TYPE_PROCESS_RLEG2:
		case SFM_ALM_TYPE_PROCESS_RLEG3:
		case SFM_ALM_TYPE_PROCESS_RLEG4:
		case SFM_ALM_TYPE_PROCESS_SMPP:
		case SFM_ALM_TYPE_PROCESS_PANA:
		case SFM_ALM_TYPE_PROCESS_RANA:
		case SFM_ALM_TYPE_PROCESS_RDRCAPD:
		case SFM_ALM_TYPE_PROCESS_CAPD:
		case SFM_ALM_TYPE_PROCESS_SCEM:
		case SFM_ALM_TYPE_PROCESS_CSCM:
		case SFM_ALM_TYPE_PROCESS_DIRM:
		/* hjjung */
		case SFM_ALM_TYPE_LEG_SESSION:
		case SFM_ALM_TYPE_TPS: // added by dcham 2011.05.25
/* MODIFY: by june, 2010-09-10
 *		현주 버그 잠시 주석 처리
 */
#if 0
			// PROC INFO "name"로 구성
			for (i=0; i<SFM_MAX_PROC_CNT; i++){
				if ( !strcasecmp ( info, sfdb->sys[sysIndex].commInfo.procInfo[i].name ) )
					break;
			}
			if ( i>=SFM_MAX_PROC_CNT )
				deleteFlag = 1;
			else {
				if(sfdb->sys[sysIndex].commInfo.procInfo[i].status == SFM_STATUS_ALIVE)
					deleteFlag = 1;
			}
#endif
			break;
		case SFM_ALM_TYPE_DUP_HEARTBEAT:
			if(sfdb->sys[sysIndex].commInfo.systemDup.heartbeatLevel != almLevel)
				deleteFlag = 1;
			break;
		case SFM_ALM_TYPE_DUAL_ACT:
			if(sfdb->sys[sysIndex].commInfo.systemDup.dualStsAlmLevel != almLevel)
				deleteFlag = 1;
			break;
		case SFM_ALM_TYPE_DUAL_STD:
			if(sfdb->sys[sysIndex].commInfo.systemDup.dualStsAlmLevel != almLevel)
				deleteFlag = 1;
			break;
		case SFM_ALM_TYPE_DUAL_STS_QRY_TIME_OUT:
			if(sfdb->sys[sysIndex].commInfo.systemDup.timeOutAlmLevel != almLevel)
				deleteFlag = 1;
			break;

			//20100916 by dcham
		case SFM_ALM_TYPE_SCM_FAULTED:
			if(sfdb->sys[sysIndex].commInfo.systemDup.dualStsAlmLevel != almLevel)
				deleteFlag = 1;

	
#if 0		/* not manage OOP status alarm. */
		case SFM_ALM_TYPE_DUP_OOS:
			break;
#endif
		case SFM_ALM_TYPE_HWNTP:
			if(strstr(info, "DAEMON"))
				intValue = 0;
			else
				intValue = 1;

			if(sfdb->sys[sysIndex].commInfo.ntpSts[intValue].level != almLevel)
				deleteFlag = 1;
		
			break;
		case SFM_ALM_TYPE_QUEUE_LOAD:
			for(i = 0; i < SFM_MAX_QUE_CNT; i++){
				if(!strcasecmp(sfdb->sys[sysIndex].commInfo.queInfo[i].qNAME,
								info)){
					if(sfdb->sys[sysIndex].commInfo.queInfo[i].level != almLevel)
						deleteFlag = 1;
					
					break;
				}
			}
			if(i >= SFM_MAX_QUE_CNT)
				deleteFlag = 1;
			break;
		case SFM_ALM_TYPE_RSRC_LOAD:
			for(i = 0; i < SFM_MAX_RSRC_LOAD_CNT; i++){
				if(rsrcName[i] && !strcasecmp(rsrcName[i], info)){
					if(sfdb->sys[sysIndex].commInfo.rsrcSts[i].level != almLevel)
						deleteFlag = 1;
					break;
				}
			}
			if(i >= SFM_MAX_RSRC_LOAD_CNT)
				deleteFlag = 1;
			break;
	
		case SFM_ALM_TYPE_NMSIF_CONNECT:
			for(i=0; i<MAX_NMS_CON; i++){
				if(i > 4){
					continue;
				}
				if(strcasecmp(sfdb->sys[sysIndex].commInfo.name, "DSCM")){
					continue;
				}

				sprintf(temp1, "NMSIF ALARM %s",nmsifName[i]);
				if(!strcasecmp(info,temp1))
				{
					if(sfdb->nmsInfo.level[i] != almLevel){
						sprintf(trcBuf,"[%s] NMS different almlevel, sfdb->nmsInfo[i:%d].level:%d <-> almLevel:%d\n"
								,__FUNCTION__,i, sfdb->nmsInfo.level[i], almLevel);
						trclib_writeLogErr (FL,trcBuf);

						deleteFlag = 1;
						break;
					}
				}
			}
			break;	
	
		case SFM_ALM_TYPE_MP_HW:
		case SFM_ALM_TYPE_HW_MIRROR:
			for(i = 0; i < SFM_MAX_HPUX_HW_COM; i++){
				intValue = strlen(sfdb->sys[sysIndex].specInfo.u.sms.hpuxHWInfo.hwcom[i].name);
				hw_name_mapping(sfdb->sys[sysIndex].specInfo.u.sms.hpuxHWInfo.hwcom[i].name,
								intValue, hwname);
			
				if(!strcasecmp(info, hwname)){
					if(sfdb->sys[sysIndex].specInfo.u.sms.hpuxHWInfo.hwcom[i].level != almLevel)
						deleteFlag = 1;
					break;
				}
			}

			if(i >= SFM_MAX_HPUX_HW_COM)
				deleteFlag = 1;

			break;

		case SFM_ALM_TYPE_DBCON_STST:
			for(i = 0; i < SFM_MAX_DB_CNT; i++){
				if(!strcasecmp(sfdb->sys[sysIndex].commInfo.name, "DSCM"))
					continue;
				if(strlen((char*)sfdb->sys[sysIndex].commInfo.rmtDbSts[i].sIpAddress) < 7)
					continue;

				sprintf(temp, "%s(%s)", sfdb->sys[sysIndex].commInfo.rmtDbSts[i].sIpAddress,
										sfdb->sys[sysIndex].commInfo.rmtDbSts[i].sDbAlias );
										
				if(!strcasecmp(info, temp)){
					if(rmtdb_conn_lvl[sysIndex][i] != almLevel)
						 deleteFlag = 1;
					break;
				}
			}

			if(i >= SFM_MAX_DB_CNT)
				deleteFlag = 1;
			break;

		case SFM_ALM_TYPE_L2_CPU:
			if(g_pstL2Dev->l2Info[devIndex].cpuInfo.level != almLevel)
				deleteFlag = 1;
			break;

		case SFM_ALM_TYPE_L2_MEM:
			if(g_pstL2Dev->l2Info[devIndex].memInfo.level != almLevel)
				deleteFlag = 1;
			break;

		case SFM_ALM_TYPE_L2_LAN:
			for(i=0; i<MAX_L2_PORT_NUM; i++){
				sprintf(temp, "%s(%d)",SQL_L2SW_PORT_ALM_INFO, i+1);
				if(!strcasecmp(temp, info)){
					if(g_pstL2Dev->l2Info[devIndex].portInfo[i].level != almLevel)
					{
					//	sprintf(trcBuf,"[fimd_checkCurrentAlarm] L2 port status change (%d <-> %d)\n",
					//			g_pstL2Dev->l2Info[devIndex].portInfo[i].level, almLevel);
					//	trclib_writeLogErr (FL,trcBuf);
						deleteFlag = 1;
					}
					break;
				}
			}
			if(i >= MAX_L2_PORT_NUM)
				deleteFlag = 1;
			break;

		case SFM_ALM_TYPE_CPS_OVER:
			if (sfdb->sys[sysIndex].commInfo.cpsOverSts.level != almLevel)
				deleteFlag = 1;
			break;
			// ADD: by june, 2010-09-09
			// 현주가 똥싸놔서 추가한 루틴
			// 현상: ALM_TYPE이 정의되지 않아 current alarm table에서 insert 된 이후 
			//       이곳에서 delete가 발생되어 사라짐.
		case SFM_ALM_TYPE_SCE_USER:

// MODIFY: by june, 2010-09-09
#if 0
sprintf(trcBuf,"[fimd_checkCurrentAlarm] alarm level(%d) level(%d)\n"
		, g_pstSCEInfo->SCEDev[devIndex].userInfo.level, almType);
trclib_writeLogErr (FL,trcBuf);
#endif
			if (g_pstSCEInfo->SCEDev[devIndex].userInfo.level != almLevel)
				deleteFlag = 1;
			break;

		/* added by uamyd 20110209. LOGON 성공율 감시를 위한 */
		case SFM_ALM_TYPE_LOGON_SUCCESS_RATE:
			g_pstLogonRate = &g_stLogonRate[0][sysIndex-1];
            if( g_pstLogonRate->level != almLevel ){
                sprintf(trcBuf,"[%s] Logon Success Rate Alarm Type=%d/memory level=%d/current_alarm level=%d\n",
						__FUNCTION__, almType, g_pstLogonRate->level, almLevel);
                trclib_writeLogErr (FL,trcBuf);
                deleteFlag = 1;
            }
            break;
		case SFM_ALM_TYPE_LOGOUT_SUCCESS_RATE:
			g_pstLogonRate = &g_stLogonRate[1][sysIndex-1];
            if( g_pstLogonRate->level != almLevel ){
                sprintf(trcBuf,"[%s] Logout Success Rate Alarm Type=%d/memory level=%d/current_alarm level=%d\n",
						__FUNCTION__, almType, g_pstLogonRate->level, almLevel);
                trclib_writeLogErr (FL,trcBuf);
                deleteFlag = 1;
            }
            break;

		case SFM_ALM_TYPE_SM_CONN_STS:
            if( sfdb->sys[sysIndex].commInfo.smChSts.level != almLevel ){
                sprintf(trcBuf,"[%s] SM Connection Status Alarm Type=%d/memory level=%d/current_alarm level=%d\n",
						__FUNCTION__, almType, sfdb->sys[sysIndex].commInfo.smChSts.level, almLevel);
                trclib_writeLogErr (FL,trcBuf);
                deleteFlag = 1;
            }
            break;

		default:
			// 알 수 없으므로 지우자 
			sprintf(trcBuf,"[fimd_checkCurrentAlarm] unknown Alarm Type (%d)\n",almType);
			trclib_writeLogErr (FL,trcBuf);

			deleteFlag = 1;
			break;
        }

DELETE_RTN:
        if(deleteFlag) {
            sprintf (query, "DELETE FROM %s WHERE (system_type='%s' AND system_group='%s' "
				"AND system_name='%s' AND alarm_type='%s' AND alarm_level='%s' AND alarm_date='%s' AND information='%s')",
                SFM_CURR_ALM_DB_TABLE_NAME,row[0],row[1],row[2],row[3],row[5],row[6],row[7]);
		
			if (fimd_mysql_query (query) < 0) {
       			sprintf(trcBuf,"[fimd_checkCurrentAlarm] fimd_mysql_query fail(%s)\n",query);
       			trclib_writeLogErr (FL,trcBuf);
			} else {
       			sprintf(trcBuf,"[fimd_checkCurrentAlarm] delete current_alarm row abnormal situation \n(%s)\n",query);
       			trclib_writeLogErr (FL,trcBuf);
			}
			if(sysIndex >= 0)
				fimd_updateSysAlmInfo(sysIndex);

			if(devIndex >= 0)
				fimd_updatePDAlmInfo (devIndex);

			fimd_broadcastAlmEvent2Client ();
        }

        rowNum--;
		keepalivelib_increase();
    }
    mysql_free_result(result);

    return 1;

} //----- End of fimd_checkCurrentAlarm -----// 


int fimd_getAlmCount(char *sys_name, int almLevel)
{
	int ret = -1;
	char query[4096];
    	MYSQL_RES   	*result;
    	MYSQL_ROW   	row;

	memset(query, 0, 4096);

	if(strlen(sys_name))
		sprintf(query, "SELECT COUNT(*) FROM %s WHERE ALARM_LEVEL = '%d' AND mask = 0 AND system_name='%s'", 
			SFM_CURR_ALM_DB_TABLE_NAME, almLevel, sys_name);
	else 
		sprintf(query, "SELECT COUNT(*) FROM %s WHERE ALARM_LEVEL = '%d' AND mask = 0", 
			SFM_CURR_ALM_DB_TABLE_NAME, almLevel);

	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_getAlmCount] fimd_mysql_query fail(%s)\n",query);
		trclib_writeLogErr (FL,trcBuf);
		return ret;
	} 

    	result = mysql_store_result(mysql_conn);
    	row = mysql_fetch_row(result);

    	if(row != NULL){
		ret = atoi(row[0]);
		sprintf(trcBuf,"[fimd_getAlmCount] current alm count(%d)\n", ret);
		trclib_writeLogErr (FL,trcBuf);

	}
	if(result != NULL)
    	mysql_free_result(result);

	return ret;
}


int fimd_saveCallInfoAlmInfo2DB (int sysIndex, int almLevel, int occurFlag)
{
	char    query[1024];
	int almCode;

	//almCode = getAlarmCodeFromType(SFM_ALM_TYPE_CALL_INFO);
	almCode = getAlarmCodeFromType(SFM_ALM_TYPE_CALL_INFO,almLevel);

	if(almCode<0){
		sprintf(trcBuf,"[%s] Invalid alarm code.(%d)\n", __FUNCTION__, almCode);
		return -1;
	}

	sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, %d, '%s', 'CallInfo')",
			SFM_ALM_HIS_DB_TABLE_NAME,
			sfdb->sys[sysIndex].commInfo.type,
			sfdb->sys[sysIndex].commInfo.group,
			sfdb->sys[sysIndex].commInfo.name,
			SFM_ALM_TYPE_CALL_INFO,
			almCode,        // alarm code 추가 : sjjeon
			almLevel,
			occurFlag,
			commlib_printDateTime(currentTime));

	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_saveCallInfoAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	//  이전 장애는 무조건 삭제한다.
	sprintf (query, "DELETE FROM %s WHERE (system_type='%s' AND system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='CallInfo')",
			SFM_CURR_ALM_DB_TABLE_NAME,
			sfdb->sys[sysIndex].commInfo.type,
			sfdb->sys[sysIndex].commInfo.group,
			sfdb->sys[sysIndex].commInfo.name,
			SFM_ALM_TYPE_CALL_INFO );

	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_saveCallInfoAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	if (  occurFlag ){ // 장애가 발생했을 경우만 처리.
		sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, '%s', 'CallInfo', 0)",
				SFM_CURR_ALM_DB_TABLE_NAME,
				sfdb->sys[sysIndex].commInfo.type,
				sfdb->sys[sysIndex].commInfo.group,
				sfdb->sys[sysIndex].commInfo.name,
				SFM_ALM_TYPE_CALL_INFO,
				almCode,        // alarm code 추가 : sjjeon
				almLevel,
				commlib_printDateTime(currentTime));

		if (fimd_mysql_query (query) < 0) {
			sprintf(trcBuf,"[fimd_saveCallInfoAlmInfo2DB] fimd_mysql_query fail\n");
			trclib_writeLogErr (FL,trcBuf);
			return -1;
		}
	}

#if 0
    if(occurFlag == 0)
        almLevel = 5; // NMS에서는 alarm clear시 alarm_level이 '5'이다.
#else
    almLevel = getNmsLevel(almLevel,occurFlag);
#endif

	memset (descBuf, 0, 100);
	sprintf (descBuf, "CallInfo");
	fimd_txMsg2Nmsif (sfdb->sys[sysIndex].commInfo.name, SFM_ALM_TYPE_CALL_INFO, almLevel, descBuf, currentTime);
	return 1;

} //----- End of fimd_saveCallInfoAlmInfo2DB -----//

//-------------------------------------------//
//-------------------------------------------//
/* by helca */

int fimd_saveRmtLanAlmInfo2DB (int sysIndex, int lanIndex, int almLevel, int occurFlag)
{
	int	 alarm_level = SFM_ALM_CRITICAL;
	char	query[1024];
	int almCode;

	//almCode = getAlarmCodeFromType(SFM_ALM_TYPE_RMT_LAN);
	almCode = getAlarmCodeFromType(SFM_ALM_TYPE_RMT_LAN, almLevel);

	if(almCode<0){
		sprintf(trcBuf,"[%s] Invalid alarm code.(%d)\n", __FUNCTION__, almCode);
		return -1;
	}

	sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, %d, '%s', '%s(%s)')",
			SFM_ALM_HIS_DB_TABLE_NAME,
			sfdb->sys[sysIndex].commInfo.type,
			sfdb->sys[sysIndex].commInfo.group,
			sfdb->sys[sysIndex].commInfo.name,
			SFM_ALM_TYPE_RMT_LAN,
			almCode,        // alarm code 추가 : sjjeon
			almLevel,
			occurFlag,
			commlib_printDateTime(currentTime),
			//&sfdb->sys[sysIndex].commInfo.rmtLanInfo[lanIndex].name[1], // why name[1]???
			sfdb->sys[sysIndex].commInfo.rmtLanInfo[lanIndex].name,
			sfdb->sys[sysIndex].commInfo.rmtLanInfo[lanIndex].targetIp);
/* DEBUG: by june, 2010-10-07
 * DESC : #if 1 LOG 추가.
 */
#if 0
	sprintf(trcBuf,"[fimd_saveRmtLanAlmInfo2DB] ALM_HIS query=%s(%s)\n"
			, query
			, sfdb->sys[sysIndex].commInfo.rmtLanInfo[lanIndex].name);
	trclib_writeLogErr (FL,trcBuf);
#endif
	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_saveRmtLanAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}


	if (occurFlag) {
		sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, '%s', '%s(%s)', 0)",
				SFM_CURR_ALM_DB_TABLE_NAME,
				sfdb->sys[sysIndex].commInfo.type,
				sfdb->sys[sysIndex].commInfo.group,
				sfdb->sys[sysIndex].commInfo.name,
				SFM_ALM_TYPE_RMT_LAN,
				almCode,        // alarm code 추가 : sjjeon
				alarm_level,
				commlib_printDateTime(currentTime),
				sfdb->sys[sysIndex].commInfo.rmtLanInfo[lanIndex].name,
				sfdb->sys[sysIndex].commInfo.rmtLanInfo[lanIndex].targetIp);
	} else {
		sprintf (query, "DELETE FROM %s WHERE (system_type='%s' AND system_group='%s' AND system_name='%s' AND alarm_type=%d AND alarm_level=%d AND information='%s(%s)')",
				SFM_CURR_ALM_DB_TABLE_NAME,
				sfdb->sys[sysIndex].commInfo.type,
				sfdb->sys[sysIndex].commInfo.group,
				sfdb->sys[sysIndex].commInfo.name,
				SFM_ALM_TYPE_RMT_LAN,
				alarm_level,
				sfdb->sys[sysIndex].commInfo.rmtLanInfo[lanIndex].name,
				sfdb->sys[sysIndex].commInfo.rmtLanInfo[lanIndex].targetIp);
	}

/* DEBUG: by june, 2010-10-07
 * DESC : #if 1 LOG 추가.
 */
#if 0
	sprintf(trcBuf,"[fimd_saveRmtLanAlmInfo2DB] CUR_ALM query=%s\n", query);
	trclib_writeLogErr (FL,trcBuf);
#endif
	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_saveRmtLanAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

#if 0
    if(occurFlag == 0)
        almLevel = 5; // NMS에서는 alarm clear시 alarm_level이 '5'이다.
#else
    almLevel = getNmsLevel(almLevel,occurFlag);
#endif

	memset (descBuf, 0, 100);
	sprintf (descBuf, "%s", sfdb->sys[sysIndex].commInfo.rmtLanInfo[lanIndex].name);
	fimd_txMsg2Nmsif (sfdb->sys[sysIndex].commInfo.name, SFM_ALM_TYPE_RMT_LAN, almLevel, descBuf, currentTime);

	return 1;

} //----- End of fimd_saveRmtLanAlmInfo2DB -----//


int fimd_saveOptLanAlmInfo2DB (int sysIndex, int lanIndex, int almLevel, int occurFlag)
{
	int	 alarm_level = SFM_ALM_CRITICAL;
	char	query[1024];
	int almCode;

	//almCode = getAlarmCodeFromType(SFM_ALM_TYPE_OPT_LAN);
	almCode = getAlarmCodeFromType(SFM_ALM_TYPE_OPT_LAN, almLevel);

	if(almCode<0){
		sprintf(trcBuf,"[%s] Invalid alarm code.(%d)\n", __FUNCTION__, almCode);
		return -1;
	}

	sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, %d, '%s', '%s')",
			SFM_ALM_HIS_DB_TABLE_NAME,
			sfdb->sys[sysIndex].commInfo.type,
			sfdb->sys[sysIndex].commInfo.group,
			sfdb->sys[sysIndex].commInfo.name,
			SFM_ALM_TYPE_OPT_LAN,
			almCode,        // alarm code 추가 : sjjeon
			almLevel,
			occurFlag,
			commlib_printDateTime(currentTime),
			sfdb->sys[sysIndex].commInfo.optLanInfo[lanIndex].name
			);

	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_saveOptLanAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}


	if (occurFlag) {
		sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, '%s', '%s', 0)",
				SFM_CURR_ALM_DB_TABLE_NAME,
				sfdb->sys[sysIndex].commInfo.type,
				sfdb->sys[sysIndex].commInfo.group,
				sfdb->sys[sysIndex].commInfo.name,
				SFM_ALM_TYPE_OPT_LAN,
				almCode,        // alarm code 추가 : sjjeon
				alarm_level,
				commlib_printDateTime(currentTime),
				sfdb->sys[sysIndex].commInfo.optLanInfo[lanIndex].name
				);
	} else {
		sprintf (query, "DELETE FROM %s WHERE (system_type='%s' AND system_group='%s' AND system_name='%s' AND alarm_type=%d AND alarm_level=%d AND information='%s')",
				SFM_CURR_ALM_DB_TABLE_NAME,
				sfdb->sys[sysIndex].commInfo.type,
				sfdb->sys[sysIndex].commInfo.group,
				sfdb->sys[sysIndex].commInfo.name,
				SFM_ALM_TYPE_OPT_LAN,
				alarm_level,
				sfdb->sys[sysIndex].commInfo.optLanInfo[lanIndex].name
				);
	}

	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_saveOptLanAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

#if 0
    if(occurFlag == 0)
        almLevel = 5; // NMS에서는 alarm clear시 alarm_level이 '5'이다.
#else
    almLevel = getNmsLevel(almLevel,occurFlag);
#endif

	memset (descBuf, 0, 100);
	sprintf (descBuf, "%s%d", sfdb->sys[sysIndex].commInfo.optLanInfo[lanIndex].name, lanIndex);
	fimd_txMsg2Nmsif (sfdb->sys[sysIndex].commInfo.name, SFM_ALM_TYPE_OPT_LAN, almLevel, descBuf, currentTime);

	return 1;

} //----- End of fimd_saveOptLanAlmInfo2DB -----//


int fimd_saveDupHeartAlmInfo2DB (int sysIndex, int almLevel, int occurFlag)
{
	char	query[1024];
	int almCode;

	//almCode = getAlarmCodeFromType(SFM_ALM_TYPE_DUP_HEARTBEAT);
	almCode = getAlarmCodeFromType(SFM_ALM_TYPE_DUP_HEARTBEAT,almLevel);

	if(almCode<0){
		sprintf(trcBuf,"[%s] Invalid alarm code.(%d)\n", __FUNCTION__, almCode);
		return -1;
	}

	sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, %d, '%s', '%s')",
			SFM_ALM_HIS_DB_TABLE_NAME,
			sfdb->sys[sysIndex].commInfo.type,
			sfdb->sys[sysIndex].commInfo.group,
			sfdb->sys[sysIndex].commInfo.name,
			SFM_ALM_TYPE_DUP_HEARTBEAT,
			almCode,        // alarm code 추가 : sjjeon
			almLevel,
			occurFlag,
			commlib_printDateTime(currentTime),
			(occurFlag == 1?"HEARTBEAT FAIL":"HEARTBEAT OK"));

	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_saveDupHeartAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	sprintf (query, "DELETE FROM %s WHERE (system_type='%s' AND system_group='%s' AND system_name='%s' AND alarm_type=%d AND alarm_level=%d)",
			SFM_CURR_ALM_DB_TABLE_NAME,
			sfdb->sys[sysIndex].commInfo.type,
			sfdb->sys[sysIndex].commInfo.group,
			sfdb->sys[sysIndex].commInfo.name,
			SFM_ALM_TYPE_DUP_HEARTBEAT,
			almLevel);

	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_saveDupHeartAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	if (occurFlag) {
		sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, '%s', '%s', 0)",
				SFM_CURR_ALM_DB_TABLE_NAME,
				sfdb->sys[sysIndex].commInfo.type,
				sfdb->sys[sysIndex].commInfo.group,
				sfdb->sys[sysIndex].commInfo.name,
				SFM_ALM_TYPE_DUP_HEARTBEAT,
				almCode,        // alarm code 추가 : sjjeon
				almLevel,
				commlib_printDateTime(currentTime),
			    	"HEARTBEAT FAIL");
		if (fimd_mysql_query (query) < 0) {
			sprintf(trcBuf,"[fimd_saveDupHeartAlmInfo2DB] fimd_mysql_query fail\n");
			trclib_writeLogErr (FL,trcBuf);
			return -1;
		}
	}

#if 0
    if(occurFlag == 0)
        almLevel = 5; // NMS에서는 alarm clear시 alarm_level이 '5'이다.
#else
    almLevel = getNmsLevel(almLevel,occurFlag);
#endif

	memset (descBuf, 0, 100);
	if(occurFlag == 0)
		sprintf (descBuf, "HEARTBEAT OK");
	else
		sprintf (descBuf, "HEARTBEAT FAIL");
	fimd_txMsg2Nmsif (sfdb->sys[sysIndex].commInfo.name, SFM_ALM_TYPE_DUP_HEARTBEAT, almLevel, descBuf, currentTime);

	return 1;

} //----- End of fimd_saveDupHeartAlmInfo2DB -----//


int fimd_saveDupOosAlmInfo2DB (int sysIndex, int almLevel, int occurFlag)
{
	char	query[1024];
	int almCode;

	//almCode = getAlarmCodeFromType(SFM_ALM_TYPE_DUP_OOS);
	almCode = getAlarmCodeFromType(SFM_ALM_TYPE_DUP_OOS, almLevel);

	if(almCode<0){
		sprintf(trcBuf,"[%s] Invalid alarm code.(%d)\n", __FUNCTION__, almCode);
		return -1;
	}

	sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, %d, '%s', '%d')",
			SFM_ALM_HIS_DB_TABLE_NAME,
			sfdb->sys[sysIndex].commInfo.type,
			sfdb->sys[sysIndex].commInfo.group,
			sfdb->sys[sysIndex].commInfo.name,
			SFM_ALM_TYPE_DUP_OOS,
			almCode,        // alarm code 추가 : sjjeon
			almLevel,
			occurFlag,
			commlib_printDateTime(currentTime),
			sfdb->sys[sysIndex].commInfo.systemDup.oosAlm);

	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_saveDupOosAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}


	if (occurFlag) {
		sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, '%s', '%d', 0)",
				SFM_CURR_ALM_DB_TABLE_NAME,
				sfdb->sys[sysIndex].commInfo.type,
				sfdb->sys[sysIndex].commInfo.group,
				sfdb->sys[sysIndex].commInfo.name,
				SFM_ALM_TYPE_DUP_OOS,
				almCode,        // alarm code 추가 : sjjeon
				almLevel,
				commlib_printDateTime(currentTime),
				sfdb->sys[sysIndex].commInfo.systemDup.oosAlm);
	} else {
		sprintf (query, "DELETE FROM %s WHERE (system_type='%s' AND system_group='%s' AND "
				"system_name='%s' AND alarm_type=%d AND alarm_level=%d AND information='%d')",
				SFM_CURR_ALM_DB_TABLE_NAME,
				sfdb->sys[sysIndex].commInfo.type,
				sfdb->sys[sysIndex].commInfo.group,
				sfdb->sys[sysIndex].commInfo.name,
				SFM_ALM_TYPE_DUP_OOS,
				almLevel,
				sfdb->sys[sysIndex].commInfo.systemDup.oosAlm);
	}

	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_saveDupOosAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}
	

	return 1;

} //----- End of fimd_saveDupOosAlmInfo2DB -----//


int fimd_saveSuccRateAlmInfo2DB (int sysIndex, int succIndex, int almLevel, int count, int rate, int occurFlag)
{
	char	query[1024];
	int almCode;

	//almCode = getAlarmCodeFromType(SFM_ALM_TYPE_SUCC_RATE);
	almCode = getAlarmCodeFromType(SFM_ALM_TYPE_SUCC_RATE,almLevel);

	if(almCode<0){
		sprintf(trcBuf,"[%s] Invalid alarm code.(%d)\n", __FUNCTION__, almCode);
		return -1;
	}

	sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, %d, '%s', '%s(CUR %d %d%% THR %d %d%%)')",
			SFM_ALM_HIS_DB_TABLE_NAME,
			sfdb->sys[sysIndex].commInfo.type,
			sfdb->sys[sysIndex].commInfo.group,
			sfdb->sys[sysIndex].commInfo.name,
			SFM_ALM_TYPE_SUCC_RATE,
			almCode,        // alarm code 추가 : sjjeon
			almLevel,
			occurFlag,
			commlib_printDateTime(currentTime),
			sfdb->sys[sysIndex].commInfo.succRate[succIndex].name,
			count,rate,
			sfdb->sys[sysIndex].commInfo.succRate[succIndex].cnt,
			sfdb->sys[sysIndex].commInfo.succRate[succIndex].rate	
			);

	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_saveSuccRateAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	sprintf (query, "DELETE FROM %s WHERE (system_type='%s' AND system_group='%s' AND system_name='%s' AND "
			"alarm_type=%d AND alarm_level=%d AND information like '%s(%%')",
			SFM_CURR_ALM_DB_TABLE_NAME,
			sfdb->sys[sysIndex].commInfo.type,
			sfdb->sys[sysIndex].commInfo.group,
			sfdb->sys[sysIndex].commInfo.name,
			SFM_ALM_TYPE_SUCC_RATE,
			almLevel,
			sfdb->sys[sysIndex].commInfo.succRate[succIndex].name
			);

	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_saveSuccRateAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	if (occurFlag) {
		sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, '%s', '%s(CUR %d %d%% THR %d %d%%)', 0)",
				SFM_CURR_ALM_DB_TABLE_NAME,
				sfdb->sys[sysIndex].commInfo.type,
				sfdb->sys[sysIndex].commInfo.group,
				sfdb->sys[sysIndex].commInfo.name,
				SFM_ALM_TYPE_SUCC_RATE,
				almCode,        // alarm code 추가 : sjjeon
				almLevel,
				commlib_printDateTime(currentTime),
				sfdb->sys[sysIndex].commInfo.succRate[succIndex].name,
				count,rate,
				sfdb->sys[sysIndex].commInfo.succRate[succIndex].cnt,
				sfdb->sys[sysIndex].commInfo.succRate[succIndex].rate	
				);
		if (fimd_mysql_query (query) < 0) {
			sprintf(trcBuf,"[fimd_saveSuccRateAlmInfo2DB] fimd_mysql_query fail\n");
			trclib_writeLogErr (FL,trcBuf);
			return -1;
		}
	}

	if(occurFlag == 0)
		almLevel = 5; // NMS에서는 alarm clear시 alarm_level이 '5'이다.

	memset (descBuf, 0, 100);
	sprintf (descBuf, "%s%d", sfdb->sys[sysIndex].commInfo.succRate[succIndex].name, succIndex);
	fimd_txMsg2Nmsif (sfdb->sys[sysIndex].commInfo.name, SFM_ALM_TYPE_SUCC_RATE, almLevel, descBuf, currentTime);

	return 1;

} //----- End of fimd_saveSuccRateAlmInfo2DB -----//


int fimd_saveSuccRateIpAlmInfo2DB (int sysIndex, SuccRateIpInfo succRateIp, SFM_SysSuccRate *succRate, char *remoteSys, int almLevel, int occurFlag)
{
	char	query[1024];
	struct in_addr tIPaddr;
	int almCode;

	//almCode = getAlarmCodeFromType(SFM_ALM_TYPE_SUCC_RATE);
	almCode = getAlarmCodeFromType(SFM_ALM_TYPE_SUCC_RATE,almLevel);

	if(almCode<0){
		sprintf(trcBuf,"[%s] Invalid alarm code.(%d)\n", __FUNCTION__, almCode);
		return -1;
	}

	tIPaddr.s_addr = succRateIp.ipAddr;

	sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, %d, '%s', '%s(%s)(CUR %d %d%% THR %d %d%%)')",
			SFM_ALM_HIS_DB_TABLE_NAME,
			sfdb->sys[sysIndex].commInfo.type,
			sfdb->sys[sysIndex].commInfo.group,
			sfdb->sys[sysIndex].commInfo.name,
			SFM_ALM_TYPE_SUCC_RATE,
			almCode,        // alarm code 추가 : sjjeon
			almLevel,
			occurFlag,
			commlib_printDateTime(currentTime),
			remoteSys,
			inet_ntoa(tIPaddr),
			succRateIp.count,
			succRateIp.rate,
			succRate->cnt,
			succRate->rate
			);

	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_saveSuccRateAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	sprintf (query, "DELETE FROM %s "
			" WHERE (system_type='%s' AND system_group='%s' "
			"   AND system_name='%s' AND alarm_type=%d "
			"   AND alarm_level=%d AND information like '%s(%s)%%')",
			SFM_CURR_ALM_DB_TABLE_NAME,
			sfdb->sys[sysIndex].commInfo.type,
			sfdb->sys[sysIndex].commInfo.group,
			sfdb->sys[sysIndex].commInfo.name,
			SFM_ALM_TYPE_SUCC_RATE,
			almLevel,
			remoteSys,
			inet_ntoa(tIPaddr)
			);

	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_saveSuccRateAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	if (occurFlag) {
		sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, '%s', '%s(%s)(CUR %d %d%% THR %d %d%%)', 0)",
				SFM_CURR_ALM_DB_TABLE_NAME,
				sfdb->sys[sysIndex].commInfo.type,
				sfdb->sys[sysIndex].commInfo.group,
				sfdb->sys[sysIndex].commInfo.name,
				SFM_ALM_TYPE_SUCC_RATE,
				almCode,        // alarm code 추가 : sjjeon
				almLevel,
				commlib_printDateTime(currentTime),
				remoteSys,
				inet_ntoa(tIPaddr),
				succRateIp.count,
				succRateIp.rate,
				succRate->cnt,
				succRate->rate
				);
		if (fimd_mysql_query (query) < 0) {
			sprintf(trcBuf,"[fimd_saveSuccRateAlmInfo2DB] fimd_mysql_query fail\n");
			trclib_writeLogErr (FL,trcBuf);
			return -1;
		}
	}

#if 0
    if(occurFlag == 0)
        almLevel = 5; // NMS에서는 alarm clear시 alarm_level이 '5'이다.
#else
    almLevel = getNmsLevel(almLevel,occurFlag);
#endif
	memset (descBuf, 0, 100);
	sprintf (descBuf, "%s-%s", remoteSys, inet_ntoa(tIPaddr));
	fimd_txMsg2Nmsif (remoteSys, SFM_ALM_TYPE_SUCC_RATE, almLevel, descBuf, currentTime);

	return 1;
}

#if 0
int fimd_saveSessLoadAlmInfo2DB (int sysIndex, unsigned short sess_load, int almLevel, int occurFlag)
{
	char	query[1024];

	sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, '%s', '%d')",
			SFM_ALM_HIS_DB_TABLE_NAME,
			sfdb->sys[sysIndex].commInfo.type,
			sfdb->sys[sysIndex].commInfo.group,
			sfdb->sys[sysIndex].commInfo.name,
			SFM_ALM_TYPE_SESS_LOAD,
			almLevel,
			occurFlag,
			commlib_printDateTime(currentTime),
			sess_load
			);

	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_saveSessLoadAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}


	if (occurFlag) {
		sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, '%s', '%d', 0)",
				SFM_CURR_ALM_DB_TABLE_NAME,
				sfdb->sys[sysIndex].commInfo.type,
				sfdb->sys[sysIndex].commInfo.group,
				sfdb->sys[sysIndex].commInfo.name,
				SFM_ALM_TYPE_SESS_LOAD,
				almLevel,
				commlib_printDateTime(currentTime),
				sess_load
				);
	} else {
		sprintf (query, "DELETE FROM %s WHERE (system_type='%s' AND system_group='%s' AND "
				"system_name='%s' AND alarm_type=%d AND alarm_level=%d AND information='%d')",
				SFM_CURR_ALM_DB_TABLE_NAME,
				sfdb->sys[sysIndex].commInfo.type,
				sfdb->sys[sysIndex].commInfo.group,
				sfdb->sys[sysIndex].commInfo.name,
				SFM_ALM_TYPE_SESS_LOAD,
				almLevel,
				sess_load
				);
	}

	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_saveSessLoadAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	if(occurFlag == 0)
		almLevel = 5; // NMS에서는 alarm clear시 alarm_level이 '5'이다.

	memset (descBuf, 0, 100);
	sprintf (descBuf, "%d", sess_load);
	fimd_txMsg2Nmsif (sfdb->sys[sysIndex].commInfo.name, SFM_ALM_TYPE_SESS_LOAD, almLevel, descBuf, currentTime);

	return 1;

} //----- End of fimd_saveSessLoadAlmInfo2DB -----//
#endif


int fimd_saveRmtDbStsAlmInfo2DB (int sysIndex, int rmtDbIndex, int almLevel, int occurFlag)
{
	char	query[1024];
	int almCode;

	//almCode = getAlarmCodeFromType(SFM_ALM_TYPE_DBCON_STST);
	almCode = getAlarmCodeFromType(SFM_ALM_TYPE_DBCON_STST,almLevel);

	if(almCode<0){
		sprintf(trcBuf,"[%s] Invalid alarm code.(%d)\n", __FUNCTION__, almCode);
		return -1;
	}

	sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, %d, '%s', '%s(%s)')",
			SFM_ALM_HIS_DB_TABLE_NAME,
			sfdb->sys[sysIndex].commInfo.type,
			sfdb->sys[sysIndex].commInfo.group,
			sfdb->sys[sysIndex].commInfo.name,
			SFM_ALM_TYPE_DBCON_STST,
			almCode,        // alarm code 추가 : sjjeon
			almLevel,
			occurFlag,
			commlib_printDateTime(currentTime),
			sfdb->sys[sysIndex].commInfo.rmtDbSts[rmtDbIndex].sIpAddress,
			sfdb->sys[sysIndex].commInfo.rmtDbSts[rmtDbIndex].sDbAlias
			);

	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_saveRmtDbStsAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}


	if (occurFlag) {
		sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, '%s', '%s(%s)', 0)",
				SFM_CURR_ALM_DB_TABLE_NAME,
				sfdb->sys[sysIndex].commInfo.type,
				sfdb->sys[sysIndex].commInfo.group,
				sfdb->sys[sysIndex].commInfo.name,
				SFM_ALM_TYPE_DBCON_STST,
				almCode,        // alarm code 추가 : sjjeon
				almLevel,
				commlib_printDateTime(currentTime),
				sfdb->sys[sysIndex].commInfo.rmtDbSts[rmtDbIndex].sIpAddress,
				sfdb->sys[sysIndex].commInfo.rmtDbSts[rmtDbIndex].sDbAlias
				);
	} else {
		sprintf (query, "DELETE FROM %s WHERE (system_type='%s' AND system_group='%s' AND "
			"system_name='%s' AND alarm_type=%d AND alarm_level=%d AND information='%s(%s)')",
			SFM_CURR_ALM_DB_TABLE_NAME,
			sfdb->sys[sysIndex].commInfo.type,
			sfdb->sys[sysIndex].commInfo.group,
			sfdb->sys[sysIndex].commInfo.name,
			SFM_ALM_TYPE_DBCON_STST,
			almLevel,
			sfdb->sys[sysIndex].commInfo.rmtDbSts[rmtDbIndex].sIpAddress,
			sfdb->sys[sysIndex].commInfo.rmtDbSts[rmtDbIndex].sDbAlias
			);
	}

	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_saveRmtDbStsAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

#if 0
    if(occurFlag == 0)
        almLevel = 5; // NMS에서는 alarm clear시 alarm_level이 '5'이다.
#else
    almLevel = getNmsLevel(almLevel,occurFlag);
#endif

	memset (descBuf, 0, 100);
	sprintf (descBuf, "%s%d", sfdb->sys[sysIndex].commInfo.rmtDbSts[rmtDbIndex].sDbAlias, rmtDbIndex);
	fimd_txMsg2Nmsif (sfdb->sys[sysIndex].commInfo.name, SFM_ALM_TYPE_DBCON_STST, almLevel, descBuf, currentTime);

	return 1;

} //----- End of fimd_saveRmtDbStsAlmInfo2DB -----//


int fimd_savehwNTPAlmInfo2DB (int sysIndex, int hwNTPIndex, int almLevel, int occurFlag)
{
	char	query[1024], ntpName[2][10], ntpSts[2][10];
	int almCode;

	//almCode = getAlarmCodeFromType(SFM_ALM_TYPE_HWNTP);
	almCode = getAlarmCodeFromType(SFM_ALM_TYPE_HWNTP, almLevel);

	if(almCode<0){
		sprintf(trcBuf,"[%s] Invalid alarm code.(%d)\n", __FUNCTION__, almCode);
		return -1;
	}

	if(hwNTPIndex == 0)
		strcpy(ntpName[hwNTPIndex], "DAEMON");
	else
		strcpy(ntpName[hwNTPIndex], "CHANNEL");

	if(!occurFlag)
		strcpy(ntpSts[hwNTPIndex], "NORMAL");
	else
		strcpy(ntpSts[hwNTPIndex], "ABNORMAL");

	sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, %d, '%s', 'NTP %s %s')",
			SFM_ALM_HIS_DB_TABLE_NAME,
			sfdb->sys[sysIndex].commInfo.type,
			sfdb->sys[sysIndex].commInfo.group,
			sfdb->sys[sysIndex].commInfo.name,
			SFM_ALM_TYPE_HWNTP,
			almCode,        // alarm code 추가 : sjjeon
			almLevel,
			occurFlag,
			commlib_printDateTime(currentTime),
			ntpName[hwNTPIndex],
			ntpSts[hwNTPIndex]
			);

	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_savehwNTPAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}


	if (occurFlag) {
		sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, '%s', 'NTP %s %s', 0)",
				SFM_CURR_ALM_DB_TABLE_NAME,
				sfdb->sys[sysIndex].commInfo.type,
				sfdb->sys[sysIndex].commInfo.group,
				sfdb->sys[sysIndex].commInfo.name,
				SFM_ALM_TYPE_HWNTP,
				almCode,        // alarm code 추가 : sjjeon
				almLevel,
				commlib_printDateTime(currentTime),
				ntpName[hwNTPIndex],
				ntpSts[hwNTPIndex]
				);
	} else {
		sprintf (query, "DELETE FROM %s WHERE (system_type='%s' AND system_group='%s' AND "
			"system_name='%s' AND alarm_type=%d AND alarm_level=%d AND information like 'NTP %s %%')",
			SFM_CURR_ALM_DB_TABLE_NAME,
			sfdb->sys[sysIndex].commInfo.type,
			sfdb->sys[sysIndex].commInfo.group,
			sfdb->sys[sysIndex].commInfo.name,
			SFM_ALM_TYPE_HWNTP,
			almLevel,
			ntpName[hwNTPIndex]
			);
	}

	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_savehwNTPAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

#if 0
    if(occurFlag == 0)
        almLevel = 5; // NMS에서는 alarm clear시 alarm_level이 '5'이다.
#else
    almLevel = getNmsLevel(almLevel,occurFlag);
#endif

	memset (descBuf, 0, 100);
	sprintf (descBuf, "%s%d", ntpName[hwNTPIndex], hwNTPIndex);
	fimd_txMsg2Nmsif (sfdb->sys[sysIndex].commInfo.name, SFM_ALM_TYPE_HWNTP, almLevel, descBuf, currentTime);

	return 1;

} //----- End of fimd_savehwNTPAlmInfo2DB -----//


int fimd_savePDCpuUsageAlmInfo2DB (int devIndex, int almLevel, int occurFlag)
{
	char	query[1024], probeName[5];
	int almCode;

	//almCode = getAlarmCodeFromType(SFM_ALM_TYPE_PD_CPU_USAGE);
	almCode = getAlarmCodeFromType(SFM_ALM_TYPE_TAP_CPU_USAGE,almLevel);

	if(almCode<0){
		sprintf(trcBuf,"[%s] Invalid alarm code.(%d)\n", __FUNCTION__, almCode);
		return -1;
	}

	//
	// alarm_history DB table에는 발생/해지에 관계없이 무조건 insert한다.
	//
	if(devIndex == 0) strcpy(probeName, "TAPA");
	else if (devIndex == 1) strcpy(probeName, "TAPB");
	
	sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, %d, '%s', '%s')",
			SFM_ALM_HIS_DB_TABLE_NAME,
			"TAP",
			"TAP",
			probeName,
			//SFM_ALM_TYPE_PD_CPU_USAGE,
			SFM_ALM_TYPE_TAP_CPU_USAGE,
			almCode,        // alarm code 추가 : sjjeon
			almLevel,
			occurFlag,
			commlib_printDateTime(currentTime),
			SQL_PD_CPU_ALM_INFO);

	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_savePDCpuUsageAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	//
	// current_alarm DB table에는 발생시 insert하고 해지시 delete한다.
	//

	//  이전 장애는 무조건 삭제한다.
	sprintf (query, "DELETE FROM %s WHERE (system_type='%s' AND system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='%s')",
			SFM_CURR_ALM_DB_TABLE_NAME,
			"TAP",
			"TAP",
			probeName,
			//SFM_ALM_TYPE_PD_CPU_USAGE,
			SFM_ALM_TYPE_TAP_CPU_USAGE,
			SQL_PD_CPU_ALM_INFO);
	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_savePDCpuUsageAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	if (  occurFlag ){ // 장애가 발생했을 경우만 처리.
		sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, '%s', '%s', 0)",
				SFM_CURR_ALM_DB_TABLE_NAME,
				"TAP",
				"TAP",
				probeName,
				//SFM_ALM_TYPE_PD_CPU_USAGE,
				SFM_ALM_TYPE_TAP_CPU_USAGE,
				almCode,        // alarm code 추가 : sjjeon
				almLevel,
				commlib_printDateTime(currentTime),
				SQL_PD_CPU_ALM_INFO);

		if (fimd_mysql_query (query) < 0) {
			sprintf(trcBuf,"[fimd_savePDCpuUsageAlmInfo2DB] fimd_mysql_query fail\n");
			trclib_writeLogErr (FL,trcBuf);
			return -1;
		}

	}

#if 0
    if(occurFlag == 0)
        almLevel = 5; // NMS에서는 alarm clear시 alarm_level이 '5'이다.
#else
    almLevel = getNmsLevel(almLevel,occurFlag);
#endif

	memset (descBuf, 0, 100);
	sprintf (descBuf, "%s", SQL_PD_CPU_ALM_INFO);
	//fimd_txMsg2Nmsif (probeName, SFM_ALM_TYPE_PD_CPU_USAGE, almLevel, descBuf, currentTime);
	fimd_txMsg2Nmsif (probeName, SFM_ALM_TYPE_TAP_CPU_USAGE, almLevel, descBuf, currentTime);

	return 1;

} //----- End of fimd_savePDCpuUsageAlmInfo2DB -----//


int fimd_saveL2swCpuUsageAlmInfo2DB (int devIndex, int almLevel, int occurFlag)
{
	char	query[1024], L2Name[10];
	int almCode;

	//almCode = getAlarmCodeFromType(SFM_ALM_TYPE_L2_CPU);
	almCode = getAlarmCodeFromType(SFM_ALM_TYPE_L2_CPU, almLevel);

	if(almCode<0){
		sprintf(trcBuf,"[%s] Invalid alarm code.(%d)\n", __FUNCTION__, almCode);
		return -1;
	}
	//
	// alarm_history DB table에는 발생/해지에 관계없이 무조건 insert한다.
	//
	if(devIndex == 0) strcpy(L2Name, "L2SWA");
	else if (devIndex == 1) strcpy(L2Name, "L2SWB");
	
	sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, %d, '%s', '%s')",
			SFM_ALM_HIS_DB_TABLE_NAME,
			"L2SW",
			"L2SW",
			L2Name,
			SFM_ALM_TYPE_L2_CPU,
			almCode,        // alarm code 추가 : sjjeon
			almLevel,
			occurFlag,
			commlib_printDateTime(currentTime),
			SQL_L2SW_CPU_ALM_INFO);

	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[%s] fimd_mysql_query fail\n",__FUNCTION__);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	//
	// current_alarm DB table에는 발생시 insert하고 해지시 delete한다.
	//

	//  이전 장애는 무조건 삭제한다.
	sprintf (query, "DELETE FROM %s WHERE (system_type='%s' AND system_group='%s' AND system_name='%s' "
							"AND alarm_type=%d AND information='%s')",
			SFM_CURR_ALM_DB_TABLE_NAME,
			"L2SW",
			"L2SW",
			L2Name,
			SFM_ALM_TYPE_L2_CPU,
			SQL_L2SW_CPU_ALM_INFO);
	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_savePDCpuUsageAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	if (  occurFlag ){ // 장애가 발생했을 경우만 처리.
		sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, '%s', '%s', 0)",
				SFM_CURR_ALM_DB_TABLE_NAME,
				"L2SW",
				"L2SW",
				L2Name,
				SFM_ALM_TYPE_L2_CPU,
				almCode,        // alarm code 추가 : sjjeon
				almLevel,
				commlib_printDateTime(currentTime),
				SQL_L2SW_CPU_ALM_INFO);

		if (fimd_mysql_query (query) < 0) {
			sprintf(trcBuf,"[fimd_savePDCpuUsageAlmInfo2DB] fimd_mysql_query fail\n");
			trclib_writeLogErr (FL,trcBuf);
			return -1;
		}
	}

#if 0
    if(occurFlag == 0)
        almLevel = 5; // NMS에서는 alarm clear시 alarm_level이 '5'이다.
#else
    almLevel = getNmsLevel(almLevel,occurFlag);
#endif
	memset (descBuf, 0, 100);
	sprintf (descBuf, "%s", SQL_L2SW_CPU_ALM_INFO);
	fimd_txMsg2Nmsif (L2Name, SFM_ALM_TYPE_L2_CPU, almLevel, descBuf, currentTime);

	return 1;
}/* End of fimd_saveL2swCpuUsageAlmInfo2DB */


int fimd_savePDMemUsageAlmInfo2DB (int devIndex, int almLevel, int occurFlag)
{
	char	query[1024], probeName[2][5];
	int almCode;

	//almCode = getAlarmCodeFromType(SFM_ALM_TYPE_PD_MEMORY_USAGE);
	almCode = getAlarmCodeFromType(SFM_ALM_TYPE_TAP_MEMORY_USAGE, almLevel);

	if(almCode<0){
		sprintf(trcBuf,"[%s] Invalid alarm code.(%d)\n", __FUNCTION__, almCode);
		return -1;
	}

	if(devIndex == 0) strcpy(probeName[0], "TAPA");
	else if (devIndex == 1) strcpy(probeName[1], "TAPB");

	sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, %d, '%s', 'Memory')",
			SFM_ALM_HIS_DB_TABLE_NAME,
			"TAP",
			"TAP",
			probeName[devIndex],
			//SFM_ALM_TYPE_PD_MEMORY_USAGE,
			SFM_ALM_TYPE_TAP_MEMORY_USAGE,
			almCode,        // alarm code 추가 : sjjeon
			almLevel,
			occurFlag,
			commlib_printDateTime(currentTime));

	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_savePDMemUsageAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	//  이전 장애는 무조건 삭제한다.
	sprintf (query, "DELETE FROM %s WHERE (system_type='%s' AND system_group='%s' AND "
 					"system_name='%s' AND alarm_type=%d AND information='Memory')",
				SFM_CURR_ALM_DB_TABLE_NAME,
				"TAP",
				"TAP",
				probeName[devIndex],
				//SFM_ALM_TYPE_PD_MEMORY_USAGE );
				SFM_ALM_TYPE_TAP_MEMORY_USAGE );

	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_savePDMemUsageAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	if (  occurFlag ){ // 장애가 발생했을 경우만 처리.
		sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, '%s', 'Memory', 0)",
				SFM_CURR_ALM_DB_TABLE_NAME,
				"TAP",
				"TAP",
				probeName[devIndex],
				//SFM_ALM_TYPE_PD_MEMORY_USAGE,
				SFM_ALM_TYPE_TAP_MEMORY_USAGE,
				almCode,        // alarm code 추가 : sjjeon
				almLevel,
				commlib_printDateTime(currentTime));
		if (fimd_mysql_query (query) < 0) {
			sprintf(trcBuf,"[fimd_savePDMemUsageAlmInfo2DB] fimd_mysql_query fail\n");
			trclib_writeLogErr (FL,trcBuf);
			return -1;
		}
	}

#if 0
    if(occurFlag == 0)
        almLevel = 5; // NMS에서는 alarm clear시 alarm_level이 '5'이다.
#else
    almLevel = getNmsLevel(almLevel,occurFlag);
#endif

	memset (descBuf, 0, 100);
	sprintf (descBuf, "%s", "Memory");
	//fimd_txMsg2Nmsif (probeName[devIndex], SFM_ALM_TYPE_PD_MEMORY_USAGE, almLevel, descBuf, currentTime);
	fimd_txMsg2Nmsif (probeName[devIndex], SFM_ALM_TYPE_TAP_MEMORY_USAGE, almLevel, descBuf, currentTime);

	return 1;

} //----- End of fimd_savePDMemUsageAlmInfo2DB -----//

/*
	by sjjeon
**/
int fimd_saveL2swMemUsageAlmInfo2DB (int devIndex, int almLevel, int occurFlag)
{
	char	query[1024], L2Name[10];
	int almCode;

	//almCode = getAlarmCodeFromType(SFM_ALM_TYPE_L2_MEM);
	almCode = getAlarmCodeFromType(SFM_ALM_TYPE_L2_MEM, almLevel);

	if(almCode<0){
		sprintf(trcBuf,"[%s] Invalid alarm code.(%d)\n", __FUNCTION__, almCode);
		return -1;
	}

	bzero(query, sizeof(query));
	bzero(L2Name, sizeof(L2Name));

	if(devIndex == 0) strcpy(L2Name, "L2SWA");
	else if (devIndex == 1) strcpy(L2Name, "L2SWB");

	sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, %d, '%s', '%s')",
			SFM_ALM_HIS_DB_TABLE_NAME,
			"L2SW",
			"L2SW",
			L2Name,
			SFM_ALM_TYPE_L2_MEM,
			almCode,        // alarm code 추가 : sjjeon
			almLevel,
			occurFlag,
			commlib_printDateTime(currentTime),
			SQL_L2SW_MEM_ALM_INFO
			);

	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[%s] fimd_mysql_query fail\n",__FUNCTION__);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	//  이전 장애는 무조건 삭제한다.
	sprintf (query, "DELETE FROM %s WHERE (system_type='%s' AND system_group='%s' AND "
							"system_name='%s' AND alarm_type=%d AND information='%s')",
			SFM_CURR_ALM_DB_TABLE_NAME,
			"L2SW",
			"L2SW",
			L2Name,
			SFM_ALM_TYPE_L2_MEM,
			SQL_L2SW_MEM_ALM_INFO	
			);

	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_savePDMemUsageAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	bzero(query, sizeof(query));
	if (  occurFlag ){ // 장애가 발생했을 경우만 처리.
		sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, '%s', '%s', 0)",
				SFM_CURR_ALM_DB_TABLE_NAME,
				"L2SW",
				"L2SW",
				L2Name,
				SFM_ALM_TYPE_L2_MEM,
				almCode,        // alarm code 추가 : sjjeon
				almLevel,
				commlib_printDateTime(currentTime),
				SQL_L2SW_MEM_ALM_INFO
				);

		if (fimd_mysql_query (query) < 0) {
			sprintf(trcBuf,"[%s] fimd_mysql_query fail\n",__FUNCTION__);
			trclib_writeLogErr (FL,trcBuf);
			return -1;
		}
	}

#if 0
    if(occurFlag == 0)
        almLevel = 5; // NMS에서는 alarm clear시 alarm_level이 '5'이다.
#else
    almLevel = getNmsLevel(almLevel,occurFlag);
#endif

	memset (descBuf, 0, 100);
	sprintf (descBuf, "%s", "Memory");
	fimd_txMsg2Nmsif ((char*)L2Name, SFM_ALM_TYPE_L2_MEM, almLevel, descBuf, currentTime);

	return 1;

} //----- End of fimd_saveL2swMemUsageAlmInfo2DB-----//


int fimd_savePDFanAlmInfo2DB (int devIndex, int fanIndex, int almLevel, int occurFlag)
{
	char	query[1024], probeName[2][5];
	int almCode;

	//almCode = getAlarmCodeFromType(SFM_ALM_TYPE_PD_FAN_STS);
	almCode = getAlarmCodeFromType(SFM_ALM_TYPE_TAP_FAN_STS,almLevel);

	if(almCode<0){
		sprintf(trcBuf,"[%s] Invalid alarm code.(%d)\n", __FUNCTION__, almCode);
		return -1;
	}

	if(devIndex == 0) strcpy(probeName[0], "TAPA");
	else if (devIndex == 1) strcpy(probeName[1], "TAPB");

	sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, %d, '%s', '%s(%d)')",
			SFM_ALM_HIS_DB_TABLE_NAME,
			"TAP",
			"TAP",
			probeName[devIndex],
			//SFM_ALM_TYPE_PD_FAN_STS,
			SFM_ALM_TYPE_TAP_FAN_STS,
			almCode,        // alarm code 추가 : sjjeon
			almLevel,
			occurFlag,
			commlib_printDateTime(currentTime),
			"FAN",
			fanIndex);

	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_savePDFanAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}


	if (occurFlag) {
		sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, '%s', '%s(%d)', 0)",
				SFM_CURR_ALM_DB_TABLE_NAME,
				"TAP",
				"TAP",
				probeName[devIndex],
				//SFM_ALM_TYPE_PD_FAN_STS,
				SFM_ALM_TYPE_TAP_FAN_STS,
				almCode,        // alarm code 추가 : sjjeon
				almLevel,
				commlib_printDateTime(currentTime),
				"FAN",
				fanIndex);
	} else {
		sprintf (query, "DELETE FROM %s WHERE (system_type='%s' AND system_group='%s' AND system_name='%s' AND alarm_type=%d AND alarm_level=%d AND information='%s(%d)')",
				SFM_CURR_ALM_DB_TABLE_NAME,
				"TAP",
				"TAP",
				probeName[devIndex],
				//SFM_ALM_TYPE_PD_FAN_STS,
				SFM_ALM_TYPE_TAP_FAN_STS,
				almLevel,
				"FAN",
				fanIndex);
	}

	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_savePDFanAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

#if 0
    if(occurFlag == 0)
        almLevel = 5; // NMS에서는 alarm clear시 alarm_level이 '5'이다.
#else
    almLevel = getNmsLevel(almLevel,occurFlag);
#endif

	memset (descBuf, 0, 100);
	sprintf (descBuf, "%s%d", "FAN", fanIndex);
	//fimd_txMsg2Nmsif (probeName[devIndex], SFM_ALM_TYPE_PD_FAN_STS, almLevel, descBuf, currentTime);
	fimd_txMsg2Nmsif (probeName[devIndex], SFM_ALM_TYPE_TAP_FAN_STS, almLevel, descBuf, currentTime);

	return 1;

} //----- End of fimd_savePDFanAlmInfo2DB -----//


int fimd_saveGigaLanAlmInfo2DB (int devIndex, int gigaIndex, int almLevel, int occurFlag)
{
	int	 GigaLanLevel = 3;
	char	query[1024], probeName[5];;
	time_t now;
	int almCode;

	//almCode = getAlarmCodeFromType(SFM_ALM_TYPE_PD_GIGA_LAN);
	almCode = getAlarmCodeFromType(SFM_ALM_TYPE_TAP_PORT_STS, almLevel);

	if(almCode<0){
		sprintf(trcBuf,"[%s] Invalid alarm code.(%d)\n", __FUNCTION__, almCode);
		return -1;
	}

	now = time(0);
	if(devIndex == 0) strcpy(probeName, "TAPA");
	else if (devIndex == 1) strcpy(probeName, "TAPB");

	sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, %d, '%s', 'LINK(%d)')",
			SFM_ALM_HIS_DB_TABLE_NAME,
			"TAP",
			"TAP",
			probeName,
			//SFM_ALM_TYPE_PD_GIGA_LAN,
			SFM_ALM_TYPE_TAP_PORT_STS,
			almCode,        // alarm code 추가 : sjjeon
			almLevel,
			occurFlag,
			commlib_printDateTime(currentTime),
			gigaIndex+1
			);

	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_saveGigaLanAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}
	
	sprintf (query, "DELETE FROM %s WHERE (system_type='%s' AND system_group='%s' "
					"AND system_name='%s' AND alarm_type=%d AND alarm_level=%d AND information='LINK(%d)')",
			SFM_CURR_ALM_DB_TABLE_NAME,
			"TAP",
			"TAP",
			probeName,
			//SFM_ALM_TYPE_PD_GIGA_LAN,
			SFM_ALM_TYPE_TAP_PORT_STS,
			GigaLanLevel,
			gigaIndex+1
			);
	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_saveGigaLanAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}
	if (occurFlag) {
		sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, '%s', 'LINK(%d)', 0)",
				SFM_CURR_ALM_DB_TABLE_NAME,
				"TAP",
				"TAP",
				probeName,
				//SFM_ALM_TYPE_PD_GIGA_LAN,
				SFM_ALM_TYPE_TAP_PORT_STS,
				almCode,        // alarm code 추가 : sjjeon
				GigaLanLevel,
				commlib_printDateTime(currentTime),
				gigaIndex+1
				);
		if (fimd_mysql_query (query) < 0) {
			sprintf(trcBuf,"[fimd_saveGigaLanAlmInfo2DB] fimd_mysql_query fail\n");
			trclib_writeLogErr (FL,trcBuf);
			return -1;
		}
	}

#if 0
    if(occurFlag == 0)
        almLevel = 5; // NMS에서는 alarm clear시 alarm_level이 '5'이다.
#else
    almLevel = getNmsLevel(almLevel,occurFlag);
#endif

	memset (descBuf, 0, 100);
	sprintf (descBuf, "%s%d", "LINK", gigaIndex);
	//fimd_txMsg2Nmsif (probeName, SFM_ALM_TYPE_PD_GIGA_LAN, almLevel, descBuf, currentTime);
	fimd_txMsg2Nmsif (probeName, SFM_ALM_TYPE_TAP_PORT_STS, almLevel, descBuf, currentTime);

	return 1;
} //----- End of fimd_saveGigaLanAlmInfo2DB -----//


int fimd_saveRsrcLoadAlmInfo2DB (int sysIndex, int loadIndex, int almLevel, int occurFlag)
{
	char	query[1024];
	int almCode;

	//almCode = getAlarmCodeFromType(SFM_ALM_TYPE_RSRC_LOAD);
	almCode = getAlarmCodeFromType(SFM_ALM_TYPE_RSRC_LOAD, almLevel);

	if(almCode<0){
		sprintf(trcBuf,"[%s] Invalid alarm code.(%d)\n", __FUNCTION__, almCode);
		return -1;
	}

	sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, %d, '%s', '%s')",
			SFM_ALM_HIS_DB_TABLE_NAME,
			sfdb->sys[sysIndex].commInfo.type,
			sfdb->sys[sysIndex].commInfo.group,
			sfdb->sys[sysIndex].commInfo.name,
			SFM_ALM_TYPE_RSRC_LOAD,
			almCode,        // alarm code 추가 : sjjeon
			almLevel,
			occurFlag,
			commlib_printDateTime(currentTime),
			rsrcName[loadIndex]);

	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_saveRsrcLoadAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}


	if (occurFlag) {
		sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, '%s', '%s', 0)",
				SFM_CURR_ALM_DB_TABLE_NAME,
				sfdb->sys[sysIndex].commInfo.type,
				sfdb->sys[sysIndex].commInfo.group,
				sfdb->sys[sysIndex].commInfo.name,
				SFM_ALM_TYPE_RSRC_LOAD,
				almCode,        // alarm code 추가 : sjjeon
				almLevel,
				commlib_printDateTime(currentTime),
				rsrcName[loadIndex]);
	} else {
		sprintf (query, "DELETE FROM %s WHERE (system_type='%s' AND system_group='%s' AND system_name='%s' AND alarm_type=%d AND alarm_level=%d AND information='%s')",
				SFM_CURR_ALM_DB_TABLE_NAME,
				sfdb->sys[sysIndex].commInfo.type,
				sfdb->sys[sysIndex].commInfo.group,
				sfdb->sys[sysIndex].commInfo.name,
				SFM_ALM_TYPE_RSRC_LOAD,
				almLevel,
				rsrcName[loadIndex]);
	}

	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_saveRsrcLoadAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

#if 0
    if(occurFlag == 0)
        almLevel = 5; // NMS에서는 alarm clear시 alarm_level이 '5'이다.
#else
    almLevel = getNmsLevel(almLevel,occurFlag);
#endif

	memset (descBuf, 0, 100);
	sprintf (descBuf, "%s%d", rsrcName[loadIndex], loadIndex);
	fimd_txMsg2Nmsif (sfdb->sys[sysIndex].commInfo.name, SFM_ALM_TYPE_RSRC_LOAD, almLevel, descBuf, currentTime);

	return 1;

} //----- End of fimd_saveRsrcLoadAlmInfo2DB -----//


int fimd_saveQueLoadAlmInfo2DB (int sysIndex, int queLoadIndex, int almLevel, int occurFlag)
{
	char	query[1024];
	int almCode;

	//almCode = getAlarmCodeFromType(SFM_ALM_TYPE_QUEUE_LOAD);
	almCode = getAlarmCodeFromType(SFM_ALM_TYPE_QUEUE_LOAD, almLevel);

	if(almCode<0){
		sprintf(trcBuf,"[%s] Invalid alarm code.(%d)\n", __FUNCTION__, almCode);
		return -1;
	}

	sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, %d, '%s', '%s')",
			SFM_ALM_HIS_DB_TABLE_NAME,
			sfdb->sys[sysIndex].commInfo.type,
			sfdb->sys[sysIndex].commInfo.group,
			sfdb->sys[sysIndex].commInfo.name,
			SFM_ALM_TYPE_QUEUE_LOAD,
			almCode,        // alarm code 추가 : sjjeon
			almLevel,
			occurFlag,
			commlib_printDateTime(currentTime),
			sfdb->sys[sysIndex].commInfo.queInfo[queLoadIndex].qNAME);

	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_saveDiskUsageAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	//  이전 장애는 무조건 삭제한다.
	sprintf (query, "DELETE FROM %s WHERE (system_type='%s' AND system_group='%s' AND "
 					"system_name='%s' AND alarm_type=%d AND information='%s')",
				SFM_CURR_ALM_DB_TABLE_NAME,
				sfdb->sys[sysIndex].commInfo.type,
				sfdb->sys[sysIndex].commInfo.group,
				sfdb->sys[sysIndex].commInfo.name,
				SFM_ALM_TYPE_QUEUE_LOAD,
				sfdb->sys[sysIndex].commInfo.queInfo[queLoadIndex].qNAME);

	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_saveDiskUsageAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}


	if ( occurFlag ){ // 장애가 발생했을 경우만 처리.
		sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, '%s', '%s', 0)",
				SFM_CURR_ALM_DB_TABLE_NAME,
				sfdb->sys[sysIndex].commInfo.type,
				sfdb->sys[sysIndex].commInfo.group,
				sfdb->sys[sysIndex].commInfo.name,
				SFM_ALM_TYPE_QUEUE_LOAD,
				almCode,        // alarm code 추가 : sjjeon
				almLevel,
				commlib_printDateTime(currentTime),
				sfdb->sys[sysIndex].commInfo.queInfo[queLoadIndex].qNAME);

		if (fimd_mysql_query (query) < 0) {
			sprintf(trcBuf,"[fimd_saveDiskUsageAlmInfo2DB] fimd_mysql_query fail\n");
			trclib_writeLogErr (FL,trcBuf);
			return -1;
		}
	}

#if 0
    if(occurFlag == 0)
        almLevel = 5; // NMS에서는 alarm clear시 alarm_level이 '5'이다.
#else
    almLevel = getNmsLevel(almLevel,occurFlag);
#endif
	memset (descBuf, 0, 100);
	sprintf (descBuf, "%s", sfdb->sys[sysIndex].commInfo.queInfo[queLoadIndex].qNAME);
	fimd_txMsg2Nmsif (sfdb->sys[sysIndex].commInfo.name, SFM_ALM_TYPE_QUEUE_LOAD, almLevel, descBuf, currentTime);

	return 1;
} //----- End of fimd_saveQueLoadAlmInfo2DB -----//



#if 0
// Not used - sjjeon
void fimd_transTotalUpDownFrameRate(int sysIndex)
{
	int		percentage = 0;
	char		query[1024], fname[256], lineBuf[10];
	char		*env;
	unsigned int    recent_frame, previous_frame;
    	MYSQL_RES   	*result;
    	MYSQL_ROW   	row;
	FILE *fp;
	static char	prevStatDate[SYSCONF_MAX_ASSO_SYS_NUM][25];

	if(!strcasecmp(sfdb->sys[sysIndex].commInfo.type, "OMP"))
		return;

	if ((env = getenv(IV_HOME)) == NULL)
        return;

	sprintf(query,  "SELECT A.stat_date, A.TOTAL_FRAME, B.stat_date, B.TOTAL_FRAME  "
			"FROM (SELECT stat_date, SUM(ip_up_frames+ip_down_frames) AS TOTAL_FRAME  "
			"       FROM tran_5minute_statistics  "
			"       WHERE system_name = '%s'  "
			"             AND stat_date = (select max(stat_date)  "
			"                              from tran_5minute_statistics  "
			"                              where system_name = '%s' ) "
			"       GROUP BY stat_date) A,  "
			"       (SELECT stat_date, SUM(ip_up_frames+ip_down_frames) AS TOTAL_FRAME  "
			"       FROM tran_5minute_statistics  "
			"       WHERE system_name = '%s'  "
			"             and stat_date = (select max(stat_date)  "
			"                              from tran_5minute_statistics  "
			"                              where system_name = '%s'  "
			"                              and stat_date < (select max(stat_date)  "
			"                                               from tran_5minute_statistics  "
			"                                               where system_name = '%s')) "
			"       GROUP BY stat_date) B  ",
		     sfdb->sys[sysIndex].commInfo.name, sfdb->sys[sysIndex].commInfo.name, sfdb->sys[sysIndex].commInfo.name,
			 sfdb->sys[sysIndex].commInfo.name, sfdb->sys[sysIndex].commInfo.name);

	if(fimd_mysql_query(query) < 0){
		// do exception handling
		sprintf(trcBuf, "transTotalUpDownFrameRate mysql_query fail \n");
		trclib_writeLogErr(FL, trcBuf);
		previous_frame = 0;
		recent_frame = 0;
	}

	return; /*DB 에러로 인하여 return.   sjjeon.*/
	result = mysql_store_result(mysql_conn);
	row = mysql_fetch_row(result);
	if(row == NULL){
		sprintf(trcBuf, "[fimd_transTotalUpDownFrameRate]%s, select get no any row\n",
						sfdb->sys[sysIndex].commInfo.name);
		trclib_writeLogErr(FL, trcBuf);
		mysql_free_result(result);
		return;
	}

	if(!row[0] || !row[1]){
		sprintf(trcBuf, "[fimd_transTotalUpDownFrameRate]%s, select get no any row\n",
						sfdb->sys[sysIndex].commInfo.name);
		trclib_writeLogErr(FL, trcBuf);
		mysql_free_result(result);
		return;

	}
		
	recent_frame = atoi(row[1]);
	previous_frame = atoi(row[3]);

	// 19 means the length of 'YYYY-MM-DD hh:mm:ss'
	if(!strncmp(prevStatDate[sysIndex], row[0], 19)){
		mysql_free_result(result);
		return;
	}else
		strcpy(prevStatDate[sysIndex], row[0]);
	
	mysql_free_result(result);

	sprintf (fname, "%s/DATA/traffic_file", env); // by helca 09.11

	fp = fopen(fname, "r");

	if(fp){
		fgets (lineBuf, sizeof(lineBuf), fp);
		fclose(fp);
	}else{
		strcpy(lineBuf, "50");
	}
	
	if(lineBuf){
		percentage = atoi(lineBuf);
	}else
		printf("lineBuf is null\n");


//	fprintf(stderr, "recent: %d previous: %d  \n", recent_frame, previous_frame);
//	fprintf(stderr, "percentage: %d\n", percentage);
	// compare recent fram with previous frame
	// in case of below 50%
	// alarm to GUI
	if((previous_frame*percentage)/100 > recent_frame){
		fimd_rateSts(previous_frame, recent_frame, sysIndex);
	}

}
#endif

/* NMSIF */
int fimd_saveNmsifstsAlmInfo2DB (int sysIndex, int nmsIndex, int almLevel, int occurFlag)
{
	char    query[1024];
	int almCode;

	//almCode = getAlarmCodeFromType(SFM_ALM_TYPE_NMSIF_CONNECT);
	almCode = getAlarmCodeFromType(SFM_ALM_TYPE_NMSIF_CONNECT, almLevel);

	if(almCode<0){
		sprintf(trcBuf,"[%s] Invalid alarm code.(%d)\n", __FUNCTION__, almCode);
		return -1;
	}

	sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, %d, '%s', 'NMSIF ALARM %s')",
			SFM_ALM_HIS_DB_TABLE_NAME,
			sfdb->sys[sysIndex].commInfo.type,
			sfdb->sys[sysIndex].commInfo.group,
			sfdb->sys[sysIndex].commInfo.name,
			SFM_ALM_TYPE_NMSIF_CONNECT,
			almCode,        // alarm code 추가 : sjjeon
			almLevel,
			occurFlag,
			commlib_printDateTime(currentTime),
			nmsifName[nmsIndex]);

	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_saveNmsifstsAlmInfo2DB] (HISTORY INSERT)fimd_mysql_query fail = %s\n", query);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}


	sprintf (query, "DELETE FROM %s WHERE (system_type='%s' AND system_group='%s' AND system_name='%s' \
		AND alarm_type=%d AND alarm_level=%d AND information='NMSIF ALARM %s')",
			SFM_CURR_ALM_DB_TABLE_NAME,
			sfdb->sys[sysIndex].commInfo.type,
			sfdb->sys[sysIndex].commInfo.group,
			sfdb->sys[sysIndex].commInfo.name,
			SFM_ALM_TYPE_NMSIF_CONNECT,
			almLevel,
			nmsifName[nmsIndex]);

	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_saveNmsifstsAlmInfo2DB] (CURRENT DELETE)fimd_mysql_query fail = %s\n", query);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	if (occurFlag) {
		sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, '%s', 'NMSIF ALARM %s', 0)",
				SFM_CURR_ALM_DB_TABLE_NAME,
				sfdb->sys[sysIndex].commInfo.type,
				sfdb->sys[sysIndex].commInfo.group,
				sfdb->sys[sysIndex].commInfo.name,
				SFM_ALM_TYPE_NMSIF_CONNECT,
				almCode,        // alarm code 추가 : sjjeon
				almLevel,
				commlib_printDateTime(currentTime),
				nmsifName[nmsIndex]);

		if (fimd_mysql_query (query) < 0) {
			sprintf(trcBuf,"[fimd_saveNmsifstsAlmInfo2DB] (CURRENT INSERT)fimd_mysql_query fail = %s\n", query);
			trclib_writeLogErr (FL,trcBuf);
			return -1;
		}
	}
#if 0
    if(occurFlag == 0)
        almLevel = 5; // NMS에서는 alarm clear시 alarm_level이 '5'이다.
#else
    almLevel = getNmsLevel(almLevel,occurFlag);
#endif

	memset (descBuf, 0, 100);
	sprintf (descBuf, "%s%d", nmsifName[nmsIndex], nmsIndex);
	fimd_txMsg2Nmsif (sfdb->sys[sysIndex].commInfo.name, SFM_ALM_TYPE_NMSIF_CONNECT, almLevel, descBuf, currentTime);
	return 1;

} //----- End of fimd_saveNmsifstsAlmInfo2DB -----//


/* DUAL ACT/STD */
int fimd_saveDualActStsAlmInfo2DB (int sysIndex, int almLevel, int dualActStdFlag, int dupDualOccured)
{
    int     almType;
    char    query[1024], dualType[20];
	int almCode;

    if(dualActStdFlag == 1) {
    	almType = SFM_ALM_TYPE_DUAL_ACT;
    	strcpy(dualType, "ACTIVE");
	}else if(dualActStdFlag == 2) {
    	almType = SFM_ALM_TYPE_DUAL_STD;
    	strcpy(dualType, "STANDBY");
	}

	//almCode = getAlarmCodeFromType(almType);
	almCode = getAlarmCodeFromType(almType, almLevel);

	if(almCode<0){
		sprintf(trcBuf,"[%s] Invalid alarm code.(%d)\n", __FUNCTION__, almCode);
		return -1;
	}
	
    sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, %d, '%s', 'DUAL STATUS %s')",
            SFM_ALM_HIS_DB_TABLE_NAME,
            sfdb->sys[sysIndex].commInfo.type,
            sfdb->sys[sysIndex].commInfo.group,
            sfdb->sys[sysIndex].commInfo.name,
            almType,
			almCode,        // alarm code 추가 : sjjeon
            almLevel,
            dupDualOccured,
            commlib_printDateTime(currentTime),
            dualType);

    if (fimd_mysql_query (query) < 0) {
        sprintf(trcBuf,"[fimd_saveDualActStsAlmInfo2DB] fimd_mysql_query fail\n");
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }

    if (dupDualOccured) {
        sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, '%s', 'DUAL STATUS %s', 0)",
                SFM_CURR_ALM_DB_TABLE_NAME,
                sfdb->sys[sysIndex].commInfo.type,
                sfdb->sys[sysIndex].commInfo.group,
                sfdb->sys[sysIndex].commInfo.name,
                almType,
				almCode,        // alarm code 추가 : sjjeon
                almLevel,
                commlib_printDateTime(currentTime),
                dualType);
    } else {
    	sprintf (query, "DELETE FROM %s WHERE (system_type='%s' AND system_group='%s' AND system_name='%s' "
        		"AND alarm_type=%d AND alarm_level=%d AND information='DUAL STATUS %s')",
                SFM_CURR_ALM_DB_TABLE_NAME,
                sfdb->sys[sysIndex].commInfo.type,
                sfdb->sys[sysIndex].commInfo.group,
                sfdb->sys[sysIndex].commInfo.name,
                almType,
                almLevel,
                dualType);
    }

    if (fimd_mysql_query (query) < 0) {
        sprintf(trcBuf,"[fimd_saveDualActStsAlmInfo2DB] fimd_mysql_query fail\n");
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
        
    if(dupDualOccured == 0)
        almLevel = 5; // NMS에서는 alarm clear시 alarm_level이 '5'이다.
#if 0
    if(dupDualOccured== 0)
        almLevel = 5; // NMS에서는 alarm clear시 alarm_level이 '5'이다.
#else
    almLevel = getNmsLevel(almLevel,dupDualOccured);
#endif 

    memset (descBuf, 0, 100);
    sprintf (descBuf, "DUAL STATUS- %s", dualType);
    fimd_txMsg2Nmsif (sfdb->sys[sysIndex].commInfo.name, almType, almLevel, descBuf, currentTime);
    return 1;

} //----- End of fimd_saveDualActStsAlmInfo2DB -----//


int fimd_saveDupTimeOutAlmInfo2DB (int sysIndex, int almLevel, int occurFlag)
{
	int     almType;
	char    query[1024];
	int almCode;

	almType = SFM_ALM_TYPE_DUAL_STS_QRY_TIME_OUT;
	//almCode = getAlarmCodeFromType(almType);
	almCode = getAlarmCodeFromType(almType, almLevel);

	if(almCode<0){
		sprintf(trcBuf,"[%s] Invalid alarm code.(%d)\n", __FUNCTION__, almCode);
		return -1;
	}

	sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, %d, '%s', 'DUAL STATUS QUERY TIME OUT')",
			SFM_ALM_HIS_DB_TABLE_NAME,
			sfdb->sys[sysIndex].commInfo.type,
			sfdb->sys[sysIndex].commInfo.group,
			sfdb->sys[sysIndex].commInfo.name,
			almType,
			almCode,        // alarm code 추가 : sjjeon
			almLevel,
			occurFlag,
			commlib_printDateTime(currentTime));

	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_saveDupTimeOutAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	if (occurFlag) {
		sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, '%s', 'DUAL STATUS QUERY TIME OUT', 0)",
				SFM_CURR_ALM_DB_TABLE_NAME,
				sfdb->sys[sysIndex].commInfo.type,
				sfdb->sys[sysIndex].commInfo.group,
				sfdb->sys[sysIndex].commInfo.name,
				almType,
				almCode,        // alarm code 추가 : sjjeon
				almLevel,
				commlib_printDateTime(currentTime));
	} else {
		sprintf (query, "DELETE FROM %s WHERE (system_type='%s' AND system_group='%s' AND system_name='%s' "
				"AND alarm_type=%d AND alarm_level=%d AND information='DUAL STATUS QUERY TIME OUT')",
				SFM_CURR_ALM_DB_TABLE_NAME,
				sfdb->sys[sysIndex].commInfo.type,
				sfdb->sys[sysIndex].commInfo.group,
				sfdb->sys[sysIndex].commInfo.name,
				almType,
				almLevel);
	}

	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_saveDupTimeOutAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return 	-1;
	}

#if 0
    if(occurFlag == 0)
        almLevel = 5; // NMS에서는 alarm clear시 alarm_level이 '5'이다.
#else
    almLevel = getNmsLevel(almLevel,occurFlag);
#endif

	memset (descBuf, 0, 100);
	sprintf (descBuf, "DUAL STATUS QUERY TIME OUT");
	fimd_txMsg2Nmsif (sfdb->sys[sysIndex].commInfo.name, almType, almLevel, descBuf, currentTime);
	return 1;
} //----- End of fimd_saveDupTimeOutAlmInfo2DB -----// 


int fimd_saveSceUsageAlmInfo2DB (SCE_USAGE_PARAM *param)
{
	char	query[1024], devName[2][5];
	int		stat, almType;
	int     almCode, almLevel;

	almType = fimd_getAlarmType(param->devKind);
	//almCode = getAlarmCodeFromType(almType);
	if (param->occurFlag)
		almCode = getAlarmCodeFromType(almType, param->curStatus);
	else
		almCode = getAlarmCodeFromType(almType, param->preStatus);


	if(almCode<0){
		sprintf(trcBuf,"[%s] Invalid alarm code.(%d)\n", __FUNCTION__, almCode);
		return -1;
	}

	// alarm_history DB table에는 발생/해지에 관계없이 무조건 insert한다.
	if(param->sysIndex == 0) strcpy(devName[0], "SCEA");
	else if (param->sysIndex == 1) strcpy(devName[1], "SCEB");

	// sjjeon
	if(param->occurFlag)
		stat = param->curStatus;
	else
		stat = param->preStatus;

	sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, %d, '%s', '%s(%d)')",
			SFM_ALM_HIS_DB_TABLE_NAME,
			"SCE",
			"SCE",
			devName[param->sysIndex],
			almType, 
			almCode, 		// alarm code 추가 : sjjeon
			stat,
			param->occurFlag,
			commlib_printDateTime(currentTime),
			fimd_getSceDevName(param->devKind),
			param->devIndex+1
			);
	
	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_saveSceUsageAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	// current_alarm DB table에는 발생시 insert하고 해지시 delete한다.
	//  이전 장애는 무조건 삭제한다.
	sprintf (query, "DELETE FROM %s WHERE (system_type='%s' AND system_group='%s' AND system_name='%s' AND "
					"alarm_type=%d AND information='%s(%d)')",
			SFM_CURR_ALM_DB_TABLE_NAME,
			"SCE",
			"SCE",
			devName[param->sysIndex],
			almType,
			fimd_getSceDevName(param->devKind),
			param->devIndex+1
			);

	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_saveSceUsageAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	if ( param->occurFlag ){ // 장애가 발생했을 경우만 처리.
		sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, '%s', '%s(%d)', 0)",
				SFM_CURR_ALM_DB_TABLE_NAME,
				"SCE",
				"SCE",
				devName[param->sysIndex],
				almType,
				almCode,		// alarm code 추가 : sjjeon
				param->curStatus,
				commlib_printDateTime(currentTime),
				fimd_getSceDevName(param->devKind),
				param->devIndex+1
				);

		if (fimd_mysql_query (query) < 0) {
			sprintf(trcBuf,"[fimd_saveSceUsageAlmInfo2DB] fimd_mysql_query fail\n");
			trclib_writeLogErr (FL,trcBuf);
			return -1;
		}

	}
#if 0
	if(param->occurFlag == 0)
		param->curStatus = 5; // NMS에서는 alarm clear시 alarm_level이 '5'이다.
#else
    //param->curStatus = getNmsLevel(param->curStatus,param->occurFlag);
	// 20110610 by dcham 
	if(param->occurFlag) {
    almLevel = getNmsLevel(param->curStatus,param->occurFlag);
	} else {
		almLevel = getNmsLevel(param->preStatus,param->occurFlag);
	}
#endif

	memset (descBuf, 0, 100);
	sprintf (descBuf, "%s", fimd_getSceDevName(param->devKind));
	//fimd_txMsg2Nmsif (devName[param->sysIndex], fimd_getSceAlarmType(param->devKind), param->curStatus, descBuf, currentTime);
	//fimd_txMsg2Nmsif (devName[param->sysIndex], fimd_getAlarmType(param->devKind), param->curStatus, descBuf, currentTime);
	fimd_txMsg2Nmsif (devName[param->sysIndex], fimd_getAlarmType(param->devKind), almLevel, descBuf, currentTime);

	return 1;

} //----- End of fimd_saveSceUsageAlmInfo2DB -----//


/* hjjung_20100823 */
int fimd_saveLegUsageAlmInfo2DB (LEG_USAGE_PARAM *param)
{
	char query[1024], devName[2][5];
	int	stat, almType;
	int almCode, almLevel;

	almType = fimd_getAlarmType(param->devKind);
	if(param->occurFlag)
		almCode = getAlarmCodeFromType(almType, param->curStatus);
	else		
		almCode = getAlarmCodeFromType(almType, param->preStatus);

	if(almCode<0){
		sprintf(trcBuf,"[%s] Invalid alarm code.(%d)\n", __FUNCTION__, almCode);
		return -1;
	}

	// alarm_history DB table에는 발생/해지에 관계없이 무조건 insert한다.
	//
	if(param->sysIndex == 0) strcpy(devName[0], "SCMA");
	else if (param->sysIndex == 1) strcpy(devName[1], "SCMB");

	if(param->occurFlag)
		stat = param->curStatus;
	else
		stat = param->preStatus;

	sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, %d, '%s', '%s')"
			, SFM_ALM_HIS_DB_TABLE_NAME
			, "RLEG"
			, "RLEG"
			, devName[param->sysIndex]
			, almType
			, almCode // alarm code 추가 : sjjeon
			, stat
			, param->occurFlag
			, commlib_printDateTime(currentTime)
			, fimd_getLegDevName(param->devKind)
			);

/* DEBUG: by june, 2010-10-07
 * DESC : #if 1 LOG 추가.
 */
#if 0
	sprintf(trcBuf,"[fimd_saveLegUsageAlmInfo2DB] query=[%s]\n", query);
	trclib_writeLogErr (FL,trcBuf);
#endif

	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_saveLegUsageAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	// current_alarm DB table에는 발생시 insert하고 해지시 delete한다.
	//  이전 장애는 무조건 삭제한다.
	//
	sprintf (query, "DELETE FROM %s WHERE (system_type='%s' AND system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='%s')"
			, SFM_CURR_ALM_DB_TABLE_NAME
			, "RLEG"
			, "RLEG"
			, devName[param->sysIndex]
			, almType
			, fimd_getLegDevName(param->devKind)
			);

/* DEBUG: by june, 2010-10-07
 * DESC : #if 1 LOG 추가.
 */
#if 0
	sprintf(trcBuf,"[fimd_saveLegUsageAlmInfo2DB] query=[%s]\n", query);
	trclib_writeLogErr (FL,trcBuf);
#endif
	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_saveLegUsageAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	if (param->occurFlag){ // 장애가 발생했을 경우만 처리.
		sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, '%s', '%s', 0)"
				, SFM_CURR_ALM_DB_TABLE_NAME
				, "RLEG"
				, "RLEG"
				, devName[param->sysIndex]
				, almType
				, almCode		// alarm code 추가 : sjjeon
				, param->curStatus
				, commlib_printDateTime(currentTime)
				, fimd_getLegDevName(param->devKind)
				);
/* DEBUG: by june, 2010-10-07
 * DESC : #if 1 LOG 추가.
 */
#if 0
		sprintf(trcBuf,"[fimd_saveLegUsageAlmInfo2DB] query=[%s]\n", query);
		trclib_writeLogErr (FL,trcBuf);
#endif
		if (fimd_mysql_query (query) < 0) {
			sprintf(trcBuf,"[fimd_saveLegUsageAlmInfo2DB] fimd_mysql_query fail\n");
			trclib_writeLogErr (FL,trcBuf);
			return -1;
		}
	}
#if 0
	if(param->occurFlag == 0)
		param->curStatus = 5; // NMS에서는 alarm clear시 alarm_level이 '5'이다.
#else
	//param->curStatus = getNmsLevel(param->curStatus,param->occurFlag);
	// 20110910 by dcham 
	if(param->occurFlag) {
		almLevel = getNmsLevel(param->curStatus,param->occurFlag);
	} else {
		almLevel = getNmsLevel(param->preStatus,param->occurFlag);
	}
#endif

	memset (descBuf, 0, 100);
	sprintf (descBuf, "%s(%s)", fimd_getLegDevName(param->devKind), devName[param->sysIndex]);
	//fimd_txMsg2Nmsif (devName[param->sysIndex], fimd_getAlarmType(param->devKind), param->curStatus, descBuf, currentTime);
	fimd_txMsg2Nmsif (devName[param->sysIndex], fimd_getAlarmType(param->devKind), almLevel, descBuf, currentTime);

	return 1;
} //----- End of fimd_saveLegUsageAlmInfo2DB -----//

/* added by dcham 20110525 for TPS */
int fimd_saveLegTpsAlmInfo2DB (TPS_PARAM *param)
{
	char query[1024], devName[2][5];
	int	stat, almType;
	int almCode, almLevel;

	almType = fimd_getAlarmType(param->devKind);
	if(param->occurFlag)
		almCode = getAlarmCodeFromType(almType, param->curStatus);
	else		
		almCode = getAlarmCodeFromType(almType, param->preStatus);

	if(almCode<0){
		sprintf(trcBuf,"[%s] Invalid alarm code.(%d)\n", __FUNCTION__, almCode);
		return -1;
	}

	// alarm_history DB table에는 발생/해지에 관계없이 무조건 insert한다.
	if(param->sysIndex == 0) strcpy(devName[0], "SCMA");
	else if (param->sysIndex == 1) strcpy(devName[1], "SCMB");

	if(param->occurFlag)
		stat = param->curStatus;
	else
		stat = param->preStatus;

	sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, %d, '%s', '%s')"
			, SFM_ALM_HIS_DB_TABLE_NAME
			, sfdb->sys[param->sysIndex].commInfo.type
			, sfdb->sys[param->sysIndex].commInfo.group
			, devName[param->sysIndex]
			, almType
			, almCode // alarm code 추가 : sjjeon
			, stat
			, param->occurFlag
			, commlib_printDateTime(currentTime)
			, fimd_getLegDevName(param->devKind)
			);

	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_saveLegTpsAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	// current_alarm DB table에는 발생시 insert하고 해지시 delete한다.
	//  이전 장애는 무조건 삭제한다.
	sprintf (query, "DELETE FROM %s WHERE (system_type='%s' AND system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='%s')"
			, SFM_CURR_ALM_DB_TABLE_NAME
			, sfdb->sys[param->sysIndex].commInfo.type
			, sfdb->sys[param->sysIndex].commInfo.group
			, devName[param->sysIndex]
			, almType
			, fimd_getLegDevName(param->devKind)
			);

	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_saveLegTpsAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	if (param->occurFlag){ // 장애가 발생했을 경우만 처리.
		sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, '%s', '%s', 0)"
				, SFM_CURR_ALM_DB_TABLE_NAME
				, sfdb->sys[param->sysIndex].commInfo.type
				, sfdb->sys[param->sysIndex].commInfo.group
				, devName[param->sysIndex]
				, almType
				, almCode		// alarm code 추가 : sjjeon
				, param->curStatus
				, commlib_printDateTime(currentTime)
				, fimd_getLegDevName(param->devKind)
				);
		if (fimd_mysql_query (query) < 0) {
			sprintf(trcBuf,"[fimd_saveLegTpsAlmInfo2DB] fimd_mysql_query fail\n");
			trclib_writeLogErr (FL,trcBuf);
			return -1;
		}
	}
#if 1
	/* 20110610 by dcham */
	if(param->occurFlag) {
		almLevel = getNmsLevel(param->curStatus,param->occurFlag);
	} else {
		almLevel = getNmsLevel(param->preStatus,param->occurFlag);
	}

#endif

	memset (descBuf, 0, 100);
	sprintf (descBuf, "%s(%s)", fimd_getLegDevName(param->devKind), devName[param->sysIndex]);
	fimd_txMsg2Nmsif (devName[param->sysIndex], fimd_getAlarmType(param->devKind), almLevel, descBuf, currentTime);
	return 1;
} //----- End of fimd_saveLegTpsAlmInfo2DB -----//

int fimd_saveSceLinkAlmInfo2DB (SCE_USAGE_PARAM *param, char *dev_type)
{
	char	query[1024], devName[2][5];
	int		almType, almCode, almLevel;

	almType = fimd_getAlarmType(param->devKind);
	if(almType<0){
		sprintf(trcBuf,"[%s] Invalid alarm type.(%d)\n", __FUNCTION__, almType);
		return -1;
	}

	//almCode = getAlarmCodeFromType(almType);
	almCode = getAlarmCodeFromType(almType, param->curStatus);

	if(almCode<0){
		sprintf(trcBuf,"[%s] Invalid alarm code.(%d)\n", __FUNCTION__, almCode);
		return -1;
	}

	//
	// alarm_history DB table에는 발생/해지에 관계없이 무조건 insert한다.
	//
	if(param->sysIndex == 0) strcpy(devName[0], "SCEA");
	else if (param->sysIndex == 1) strcpy(devName[1], "SCEB");
	
	sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, %d, '%s', '%s')",
			SFM_ALM_HIS_DB_TABLE_NAME,
			"SCE",
			"SCE",
			devName[param->sysIndex],
			almType,
			almCode,        // alarm code 추가 : sjjeon
			param->curStatus,
			param->occurFlag,
			commlib_printDateTime(currentTime),
			dev_type
			);

	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_saveSceUsageAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	//
	// current_alarm DB table에는 발생시 insert하고 해지시 delete한다.
	//

	//  이전 장애는 무조건 삭제한다.
	sprintf (query, "DELETE FROM %s WHERE (system_type='%s' AND system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='%s')",
			SFM_CURR_ALM_DB_TABLE_NAME,
			"SCE",
			"SCE",
			devName[param->sysIndex],
			almType,
			dev_type
			);

	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_saveSceUsageAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	if ( param->occurFlag ){ // 장애가 발생했을 경우만 처리.
		sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, '%s', '%s', 0)",
				SFM_CURR_ALM_DB_TABLE_NAME,
				"SCE",
				"SCE",
				devName[param->sysIndex],
				almType, 
				almCode,        // alarm code 추가 : sjjeon
				param->curStatus,
				commlib_printDateTime(currentTime),
				dev_type
				);

		if (fimd_mysql_query (query) < 0) {
			sprintf(trcBuf,"[fimd_saveSceUsageAlmInfo2DB] fimd_mysql_query fail\n");
			trclib_writeLogErr (FL,trcBuf);
			return -1;
		}
	}
#if 0
	if(param->occurFlag == 0)
		param->curStatus = 5; // NMS에서는 alarm clear시 alarm_level이 '5'이다.
#else
    //param->curStatus = getNmsLevel(param->curStatus, param->occurFlag);
    almLevel = getNmsLevel(param->curStatus, param->occurFlag);
#endif

	memset (descBuf, 0, 100);
	sprintf (descBuf, "%s(%d)", fimd_getSceDevName(param->devKind), param->devIndex);
	//fimd_txMsg2Nmsif (devName[param->sysIndex], fimd_getSceAlarmType(param->devKind), param->curStatus, dev_type/*descBuf*/, currentTime);
	//fimd_txMsg2Nmsif (devName[param->sysIndex], fimd_getAlarmType(param->devKind), param->curStatus, dev_type/*descBuf*/, currentTime);
	fimd_txMsg2Nmsif (devName[param->sysIndex], fimd_getAlarmType(param->devKind), almLevel, dev_type/*descBuf*/, currentTime);

	return 1;

} //----- End of fimd_saveSceUsageAlmInfo2DB -----//


/* hjjung */
int fimd_saveLegLinkAlmInfo2DB (LEG_USAGE_PARAM *param, char *dev_type)
{
	char	query[1024], devName[2][5];
	int		almType, almCode, almLevel;

	almType = fimd_getAlarmType(param->devKind);
	if(almType<0){
		sprintf(trcBuf,"[%s] Invalid alarm type.(%d)\n", __FUNCTION__, almType);
		return -1;
	}

	//almCode = getAlarmCodeFromType(almType);
	almCode = getAlarmCodeFromType(almType, param->curStatus);

	if(almCode<0){
		sprintf(trcBuf,"[%s] Invalid alarm code.(%d)\n", __FUNCTION__, almCode);
		return -1;
	}

	//
	// alarm_history DB table에는 발생/해지에 관계없이 무조건 insert한다.
	//
	if(param->sysIndex == 0) strcpy(devName[0], "SCMA");
	else if (param->sysIndex == 1) strcpy(devName[1], "SCMB");
	
	sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, %d, '%s', '%s')",
			SFM_ALM_HIS_DB_TABLE_NAME,
			"RLEG",
			"RLEG",
			devName[param->sysIndex],
			almType,
			almCode,        // alarm code 추가 : sjjeon
			param->curStatus,
			param->occurFlag,
			commlib_printDateTime(currentTime),
			dev_type
			);

	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_saveLegUsageAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	//
	// current_alarm DB table에는 발생시 insert하고 해지시 delete한다.
	//

	//  이전 장애는 무조건 삭제한다.
	sprintf (query, "DELETE FROM %s WHERE (system_type='%s' AND system_group='%s' AND system_name='%s' "
					"AND alarm_type=%d AND information='%s')",
			SFM_CURR_ALM_DB_TABLE_NAME,
			"RLEG",
			"RLEG",
			devName[param->sysIndex],
			almType,
			dev_type
			);

	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_saveLegUsageAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	if ( param->occurFlag ){ // 장애가 발생했을 경우만 처리.
		sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, '%s', '%s', 0)",
				SFM_CURR_ALM_DB_TABLE_NAME,
				"RLEG",
				"RLEG",
				devName[param->sysIndex],
				almType, 
				almCode,        // alarm code 추가 : sjjeon
				param->curStatus,
				commlib_printDateTime(currentTime),
				dev_type
				);

		if (fimd_mysql_query (query) < 0) {
			sprintf(trcBuf,"[fimd_saveLegUsageAlmInfo2DB] fimd_mysql_query fail\n");
			trclib_writeLogErr (FL,trcBuf);
			return -1;
		}
	}
#if 0
	if(param->occurFlag == 0)
		param->curStatus = 5; // NMS에서는 alarm clear시 alarm_level이 '5'이다.
#else
    //param->curStatus = getNmsLevel(param->curStatus, param->occurFlag);
    almLevel = getNmsLevel(param->curStatus, param->occurFlag);
#endif

	memset (descBuf, 0, 100);
	sprintf (descBuf, "%s(%d)", fimd_getLegDevName(param->devKind), param->devIndex);
	//fimd_txMsg2Nmsif (devName[param->sysIndex], fimd_getAlarmType(param->devKind), param->curStatus, dev_type/*descBuf*/, currentTime);
	fimd_txMsg2Nmsif (devName[param->sysIndex], fimd_getAlarmType(param->devKind), almLevel, dev_type/*descBuf*/, currentTime);

	return 1;

} //----- End of fimd_saveLegLinkAlmInfo2DB -----//


int fimd_saveSceRDRConnAlmInfo2DB(SCE_USAGE_PARAM *param, char *dev_type)
{
	char query[1024], devName[5];
	int  almCode, almType, almLevel;

	almType =  fimd_getAlarmType(param->devKind);
	if(almType<0){
		sprintf(trcBuf,"[%s] Invalid alarm type.(%d)\n", __FUNCTION__, almCode);
		return -1;
	}

	//almCode = getAlarmCodeFromType(almType);
	almCode = getAlarmCodeFromType(almType, param->curStatus);

	if(almCode<0){
		sprintf(trcBuf,"[%s] Invalid alarm code.(%d)\n", __FUNCTION__, almCode);
		return -1;
	}

	//
	// alarm_history DB table에는 발생/해지에 관계없이 무조건 insert한다.
	//
	if(param->sysIndex == 0) strcpy(devName, "SCEA");
	else if (param->sysIndex == 1) strcpy(devName, "SCEB");
	
	sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, %d, '%s', '%s')",
			SFM_ALM_HIS_DB_TABLE_NAME,
			"SCE",
			"SCE",
			devName,
			almType,
			almCode,        // alarm code 추가 : sjjeon
			param->curStatus,
			param->occurFlag,
			commlib_printDateTime(currentTime),
			dev_type
			);

	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_saveSceUsageAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	//
	// current_alarm DB table에는 발생시 insert하고 해지시 delete한다.
	//

	//  이전 장애는 무조건 삭제한다.
	sprintf (query, "DELETE FROM %s WHERE (system_type='%s' AND system_group='%s' AND system_name='%s' "
							"AND alarm_type=%d AND information='%s')",
			SFM_CURR_ALM_DB_TABLE_NAME,
			"SCE",
			"SCE",
			devName,
			fimd_getAlarmType(param->devKind),
			dev_type
			);
	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_saveSceUsageAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	if ( param->occurFlag ){ // 장애가 발생했을 경우만 처리.
		sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, '%s', '%s', 0)",
				SFM_CURR_ALM_DB_TABLE_NAME,
				"SCE",
				"SCE",
				devName,
				almType,
				almCode,        // alarm code 추가 : sjjeon
				param->curStatus,
				commlib_printDateTime(currentTime),
				dev_type
				);

		if (fimd_mysql_query (query) < 0) {
			sprintf(trcBuf,"[fimd_saveSceUsageAlmInfo2DB] fimd_mysql_query fail\n");
			trclib_writeLogErr (FL,trcBuf);
			return -1;
		}

	}
#if 0
    if(param->occurFlag == 0)
        param->curStatus = 5; // NMS에서는 alarm clear시 alarm_level이 '5'이다.
#else
    //param->curStatus = getNmsLevel(param->curStatus, param->occurFlag);
    almLevel = getNmsLevel(param->curStatus, param->occurFlag);
#endif

	memset (descBuf, 0, 100);
	sprintf (descBuf, "%s(%d)", fimd_getSceDevName(param->devKind), param->devIndex);
	//fimd_txMsg2Nmsif (devName, fimd_getSceAlarmType(param->devKind), param->curStatus, dev_type, currentTime);
	//fimd_txMsg2Nmsif (devName, fimd_getAlarmType(param->devKind), param->curStatus, dev_type, currentTime);
	fimd_txMsg2Nmsif (devName, fimd_getAlarmType(param->devKind), almLevel, dev_type, currentTime);

	return 1;

} //----- End of fimd_saveSceRDRConnAlmInfo2DB-----//

/* 
   sjjeon
   L2 switch link alarm Information
*/
int fimd_saveL2SWLanAlmInfo2DB (int devIndex, int gigaIndex, int almLevel, int occurFlag)
{
	int	GigaLanLevel = 3;
	char	query[1024], L2Name[10];;

	time_t now;
	int almCode;

	//almCode = getAlarmCodeFromType(SFM_ALM_TYPE_L2_LAN);
	almCode = getAlarmCodeFromType(SFM_ALM_TYPE_L2_LAN, almLevel);

	if(almCode<0){
		sprintf(trcBuf,"[%s] Invalid alarm code.(%d)\n", __FUNCTION__, almCode);
		return -1;
	}

	now = time(0);

	bzero(L2Name , sizeof(L2Name));
	bzero(query, sizeof(query));
	if(devIndex == 0) strcpy(L2Name, "L2SWA");
	else if (devIndex == 1) strcpy(L2Name, "L2SWB");

	sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, %d, '%s', '%s(%d)')",
			SFM_ALM_HIS_DB_TABLE_NAME,
			"L2SW",
			"L2SW",
			L2Name,
			SFM_ALM_TYPE_L2_LAN,
			almCode,        // alarm code 추가 : sjjeon
			almLevel,
			occurFlag,
			commlib_printDateTime(currentTime),
			SQL_L2SW_PORT_ALM_INFO,
			gigaIndex+1
			);

	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[%s] fimd_mysql_query fail\n query : %s",__FUNCTION__, query);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}
	
	sprintf (query, "DELETE FROM %s WHERE (system_type='%s' AND system_group='%s' "
					"AND system_name='%s' AND alarm_type=%d AND alarm_level=%d AND information='%s(%d)')",
			SFM_CURR_ALM_DB_TABLE_NAME,
			"L2SW",
			"L2SW",
			L2Name,
			SFM_ALM_TYPE_L2_LAN,
			GigaLanLevel,
			SQL_L2SW_PORT_ALM_INFO,
			gigaIndex+1
			);
	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[%s] fimd_mysql_query fail\n qeury : %s",__FUNCTION__,query);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}
	if (occurFlag) {
		sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, '%s', '%s(%d)', 0)",
				SFM_CURR_ALM_DB_TABLE_NAME,
				"L2SW",
				"L2SW",
				L2Name,
				SFM_ALM_TYPE_L2_LAN,
				almCode,        // alarm code 추가 : sjjeon
				GigaLanLevel,
				commlib_printDateTime(currentTime),
				SQL_L2SW_PORT_ALM_INFO,
				gigaIndex+1
				);
		if (fimd_mysql_query (query) < 0) {
			sprintf(trcBuf,"[fimd_saveGigaLanAlmInfo2DB] fimd_mysql_query fail\n");
			trclib_writeLogErr (FL,trcBuf);
			return -1;
		}
	}

#if 0
    if(occurFlag == 0)
        almLevel = 5; // NMS에서는 alarm clear시 alarm_level이 '5'이다.
#else
    almLevel = getNmsLevel(almLevel,occurFlag);
#endif

	memset (descBuf, 0, 100);
	sprintf (descBuf, "%s(%d)", SQL_L2SW_PORT_ALM_INFO, gigaIndex);
	fimd_txMsg2Nmsif (L2Name, SFM_ALM_TYPE_L2_LAN, almLevel, descBuf, currentTime);

	return 1;

} //----- End of fimd_saveGigaLanAlmInfo2DB -----//

// 20100917 by dcham
int fimd_saveSCMFaultStsAlmInfo2DB(int sysIndex, int almLevel, int occurFlag)
{

	char    query[1024];
	int almCode;

	// get alarm code
	almCode = getAlarmCodeFromType(SFM_ALM_TYPE_SCM_FAULTED, almLevel);

	// alarm_history DB table에는 발생/해지에 관계없이 무조건 insert한다.
	//
	sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, %d, '%s', '%s')",
			SFM_ALM_HIS_DB_TABLE_NAME,
			sfdb->sys[sysIndex].commInfo.type,
			sfdb->sys[sysIndex].commInfo.group,
			sfdb->sys[sysIndex].commInfo.name,
			SFM_ALM_TYPE_SCM_FAULTED,
			almCode,
			almLevel,
			occurFlag,
			commlib_printDateTime(currentTime),
			SQL_SCM_ALM_INFO
			);

	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_saveSCMFaulStstAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	// current_alarm DB table에는 발생시 insert하고 해지시 delete한다.
	//  이전 장애는 무조건 삭제한다.
	//
	sprintf (query, "DELETE FROM %s WHERE (system_type='%s' AND system_group='%s' AND " \
			"system_name='%s' AND alarm_type=%d AND information='%s')",
			SFM_CURR_ALM_DB_TABLE_NAME,
			sfdb->sys[sysIndex].commInfo.type,
			sfdb->sys[sysIndex].commInfo.group,
			sfdb->sys[sysIndex].commInfo.name,
			SFM_ALM_TYPE_SCM_FAULTED,
			SQL_SCM_ALM_INFO
			);

	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_saveCpuUsageAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	if (occurFlag){ // 장애가 발생했을 경우만 처리.
		sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, '%s', '%s', 0)",
				SFM_CURR_ALM_DB_TABLE_NAME,
				sfdb->sys[sysIndex].commInfo.type,
				sfdb->sys[sysIndex].commInfo.group,
				sfdb->sys[sysIndex].commInfo.name,
				SFM_ALM_TYPE_SCM_FAULTED,
				almCode,	// alarm code 추가 : sjjeon
				almLevel,
				commlib_printDateTime(currentTime),
				SQL_SCM_ALM_INFO
				);

		if (fimd_mysql_query (query) < 0) {
			sprintf(trcBuf,"[fimd_saveCpuUsageAlmInfo2DB] fimd_mysql_query fail\n");
			trclib_writeLogErr (FL,trcBuf);
			return -1;
		}
	}
#if 0
	if(occurFlag == 0)
		almLevel = 5; // NMS에서는 alarm clear시 alarm_level이 '5'이다.
#else
	almLevel = getNmsLevel(almLevel,occurFlag);
#endif
	memset (descBuf, 0, 100);
	sprintf (descBuf, "%s", SQL_SCM_ALM_INFO);
	fimd_txMsg2Nmsif (sfdb->sys[sysIndex].commInfo.name, SFM_ALM_TYPE_SCM_FAULTED, almLevel, descBuf, currentTime);

	return 1;
} //----- End of fimd_saveSCMFaultStsAlmInfo2DB -----//


/*
	by sjjeon.
	Status Alarm 공통 모듈 (mysql 적용)
*/
int fimd_saveCommStsAlmInfo2DB(int sysType,
							int unitType,
							int unitIndex,
							int curLevel,
							int occurFlag,
							char *devInfo)
{
	int		almType, almCode;
	char	query[1024], sysName[6], szDBSysType[10],szDBGrpType[10];

	bzero(query, sizeof(query));
	bzero(sysName, sizeof(sysName));
	bzero(szDBSysType, sizeof(szDBSysType));
	bzero(szDBGrpType, sizeof(szDBGrpType));

	//
	// alarm_history DB table에는 발생/해지에 관계없이 무조건 insert한다.
	//

    switch (sysType)
	{    
		case DSCM:
		case SCMA:
		case SCMB:
			if(sysType == DSCM) sprintf(sysName, "DSCM");
			else if(sysType == SCMA) sprintf(sysName, "SCMA");
			else strcpy(sysName, "SCMB");
			sprintf(trcBuf,"[%s:%d] Unknown sys type. %d\n", __FUNCTION__, __LINE__, sysType);
			trclib_writeLogErr (FL,trcBuf);
			return -1;
			break;

		case TAPA:
		case TAPB:
			if(sysType == TAPA) sprintf(sysName, "TAPA");
			else sprintf(sysName, "TAPB");
			sprintf(szDBSysType,"TAP");
			sprintf(szDBGrpType,"TAP");
			//almCode = fimd_getAlarmCode(unitType);
			almType = fimd_getAlarmType(unitType);
			break;

		case SCEA:
		case SCEB:
			if(sysType == SCEA) sprintf(sysName, "SCEA");
			else sprintf(sysName, "SCEB");
			sprintf(szDBSysType,"SCE");
			sprintf(szDBGrpType,"SCE");
			//almCode = fimd_getAlarmCode(unitType);
			almType = fimd_getAlarmType(unitType);
			break;

		case L2SWA:
		case L2SWB:
			if(sysType == L2SWA) sprintf(sysName, "L2SWA");
			else sprintf(sysName, "L2SWB");
			sprintf(szDBSysType,"L2SW");
			sprintf(szDBGrpType,"L2SW");
			//almCode = fimd_getAlarmCode(unitType);
			almType = fimd_getAlarmType(unitType);
			break;

		default:
			sprintf(trcBuf,"[%s:%d] Unknown sys type. %d\n", __FUNCTION__, __LINE__, sysType);
			trclib_writeLogErr (FL,trcBuf);
			return -1;
	}

	//almCode = getAlarmCodeFromType(almType);
	almCode = getAlarmCodeFromType(almType, curLevel);

	if(almCode<0){
		sprintf(trcBuf,"[%s] Invalid alarm code.(%d)\n", __FUNCTION__, almCode);
		return -1;
	}

	/* HISTORY */
	sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, %d, '%s', '%s')",
			SFM_ALM_HIS_DB_TABLE_NAME,
			szDBSysType,
			szDBGrpType,
			sysName,
			almType,
			almCode,        // alarm code 추가 : sjjeon
			curLevel,
			occurFlag,
			commlib_printDateTime(currentTime),
			devInfo
			);


	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_saveSceUsageAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

#if 0
		sprintf(trcBuf,"[%s] DB Query : %s\n", __FUNCTION__, query);
		trclib_writeLogErr (FL,trcBuf);
#endif

	//
	// current_alarm DB table에는 발생시 insert하고 해지시 delete한다.
	//

	//  이전 장애는 무조건 삭제한다.
	sprintf (query, "DELETE FROM %s WHERE (system_type='%s' AND system_group='%s' AND system_name='%s' "
							"AND alarm_type=%d AND information='%s')",
			SFM_CURR_ALM_DB_TABLE_NAME,
			szDBSysType,
			szDBGrpType,
			sysName,
			almType,
			devInfo
			);
	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[%s:%d] fimd_mysql_query fail\n",__FUNCTION__, __LINE__);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	if (occurFlag ){ // 장애가 발생했을 경우만 처리.
		sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, '%s', '%s', 0)",
				SFM_CURR_ALM_DB_TABLE_NAME,
				szDBSysType,
				szDBGrpType,
				sysName,
				almType,
				almCode,        // alarm code 추가 : sjjeon
				curLevel,
				commlib_printDateTime(currentTime),
				devInfo
				);

		if (fimd_mysql_query (query) < 0) {
			sprintf(trcBuf,"[%s:%d] fimd_mysql_query fail\n",__FUNCTION__,__LINE__);
			trclib_writeLogErr (FL,trcBuf);
			return -1;
		}
	}

#if 0
    if(occurFlag == 0)
        almLevel = 5; // NMS에서는 alarm clear시 alarm_level이 '5'이다.
#else
    curLevel = getNmsLevel(curLevel,occurFlag);
#endif

	memset (descBuf, 0, 100);
	sprintf (descBuf, "%d(%d)", almType, unitIndex);
	fimd_txMsg2Nmsif (sysName, almType, curLevel, devInfo, currentTime);

	return 1;

} //----- End of fimd_saveCommStsAlmInfo2DB-----//


/*
   by sjjeon
   NMSIF로 줄 level 설정한다.
   alarm level별로 nms level을 설정한다.
   기존의 1,2,3,5 -> 1,2,3,5,6,7,8
	6 : minor clear
	7 : major clear
	8 : critical clear
*/
int getNmsLevel(int curLevel, int occurflag)
{
	int level=0;

	if(occurflag == 0){
		if(curLevel == SFM_ALM_MINOR)
			level = 6;
		else if(curLevel == SFM_ALM_MAJOR)
			level = 7;
		else if(curLevel == SFM_ALM_CRITICAL)
			level = 8;
		else
			//level = curLevel;
			level = 5; // added by dcham 20110610 for NMS Alarm Clear value
	}else
		level = curLevel; 

	return level;
}
/*End of getNmsLevel*/

/*
* LOGON 성공율 감시를 위해 발생한 Alarm Event 를 Current Alarm Table 에 입력하는 함수
* added by uamyd 20110209
*/
int fimd_saveLogonSuccessRateAlmInfo2DB (int sysIndex, int log_mod, int almLevel, int occurFlag)
{
	char query[1024], probeName[5], sqlInfo[32];
	int  almCode, almType;

	if( !log_mod ){
		almCode = getAlarmCodeFromType(SFM_ALM_TYPE_LOGON_SUCCESS_RATE, almLevel);
		almType = SFM_ALM_TYPE_LOGON_SUCCESS_RATE;
		sprintf(sqlInfo, "%s", SQL_LOGON_SUCCESS_RATE_ALM_INFO);
	} else {
		almCode = getAlarmCodeFromType(SFM_ALM_TYPE_LOGOUT_SUCCESS_RATE, almLevel);
		almType = SFM_ALM_TYPE_LOGOUT_SUCCESS_RATE;
		sprintf(sqlInfo, "%s", SQL_LOGOUT_SUCCESS_RATE_ALM_INFO);
	}

	if(almCode < 0){
		sprintf(trcBuf,"[%s] Invalid alarm code.(%d)\n", __FUNCTION__, almCode);
		return -1;
	}

	//
	// alarm_history DB table에는 발생/해지에 관계없이 무조건 insert한다.
	//
	if( 1 == sysIndex ){
		strcpy( probeName, "SCMA" );
	} else {
		strcpy( probeName, "SCMB" );
	}
	
	sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, %d, '%s', '%s')",
			SFM_ALM_HIS_DB_TABLE_NAME,
			"MP", "DSC", probeName,
			almType,
			almCode,
			almLevel,
			occurFlag,
			commlib_printDateTime(currentTime),
			sqlInfo);

	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_saveLogonSuccessRateAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	//
	// current_alarm DB table에는 발생시 insert하고 해지시 delete한다.
	//  이전 장애는 무조건 삭제한다.
	//
	sprintf (query, "DELETE FROM %s WHERE (system_type='%s' AND system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='%s')",
			SFM_CURR_ALM_DB_TABLE_NAME,
			"MP", "DSC", probeName,
			almType, sqlInfo);

	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_saveLogonSuccessRateAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	if (  occurFlag ){ // 장애가 발생했을 경우만 처리.
		sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, '%s', '%s', 0)",
				SFM_CURR_ALM_DB_TABLE_NAME,
				"MP", "DSC", probeName,
				almType,
				almCode,
				almLevel,
				commlib_printDateTime(currentTime),
				sqlInfo);

		if (fimd_mysql_query (query) < 0) {
			sprintf(trcBuf,"[fimd_saveLogonSuccessRateAlmInfo2DB] fimd_mysql_query fail\n");
			trclib_writeLogErr (FL,trcBuf);
			return -1;
		}

	}

    almLevel = getNmsLevel(almLevel,occurFlag);

	memset (descBuf, 0, 100);
	sprintf (descBuf, "%s[%s] (%d%%)", sqlInfo, probeName, g_pstLogonRate->rate);
	fimd_txMsg2Nmsif (probeName, almType, almLevel, descBuf, currentTime);

	return 1;

}

int fimd_saveSMChStsAlmInfo2DB (int sysIndex, int almLevel, int occurFlag, int smChID)
{
	char query[1024], probeName[10];
	int  almCode;

	almCode = getAlarmCodeFromType(SFM_ALM_TYPE_SM_CONN_STS, almLevel);

	if( almCode < 0 ){
		sprintf(trcBuf,"[%s] Invalid alarm code.(%d)\n", __FUNCTION__, almCode);
		return -1;
	}

	if( 1 == sysIndex ){
        strcpy( probeName, "SCMA" );
    } else {
        strcpy( probeName, "SCMB" );
    }

	sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, %d, '%s', 'SMLINK(%d)')",
			SFM_ALM_HIS_DB_TABLE_NAME,
			sfdb->sys[sysIndex].commInfo.type,
			sfdb->sys[sysIndex].commInfo.group,
			sfdb->sys[sysIndex].commInfo.name,
			SFM_ALM_TYPE_SM_CONN_STS,
			almCode,        // alarm code 추가 : sjjeon
			almLevel,
			occurFlag,
			commlib_printDateTime(currentTime),
			smChID);

	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_saveRmtDbStsAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	// 이전 장애 기록은 무조건 지운다. 
	sprintf (query, "DELETE FROM %s WHERE (system_type='%s' AND system_group='%s' AND "
		"system_name='%s' AND alarm_type=%d AND alarm_level=%d AND information='SMLINK(%d)')",
		SFM_CURR_ALM_DB_TABLE_NAME,
		sfdb->sys[sysIndex].commInfo.type,
		sfdb->sys[sysIndex].commInfo.group,
		sfdb->sys[sysIndex].commInfo.name,
		SFM_ALM_TYPE_SM_CONN_STS,
		almLevel,
		smChID);

	if (occurFlag) {
		sprintf (query, "INSERT INTO %s VALUES ('%s', '%s', '%s', %d, %d, %d, '%s', 'SMLINK(%d)', 0)",
				SFM_CURR_ALM_DB_TABLE_NAME,
				sfdb->sys[sysIndex].commInfo.type,
				sfdb->sys[sysIndex].commInfo.group,
				sfdb->sys[sysIndex].commInfo.name,
				SFM_ALM_TYPE_SM_CONN_STS,
				almCode,        // alarm code 추가 : sjjeon
				almLevel,
				commlib_printDateTime(currentTime),
				smChID);
	}

	if (fimd_mysql_query (query) < 0) {
		sprintf(trcBuf,"[fimd_saveRmtDbStsAlmInfo2DB] fimd_mysql_query fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

    almLevel = getNmsLevel(almLevel,occurFlag);

	memset (descBuf, 0, 100);
	//"SCMA SM CONNECTION STATUS ABNORMAL, CHANNEL(1)"
	sprintf (descBuf, "%s SM CONNECTION STATUS %s, CHANNEL(%d)", probeName, !occurFlag?"NORMAL":"ABNORMAL", smChID);
	fimd_txMsg2Nmsif (sfdb->sys[sysIndex].commInfo.name, SFM_ALM_TYPE_SM_CONN_STS, almLevel, descBuf, currentTime);

	return 1;

}
