#include <nmsif_proto.h>
#include <nmsif.h>


MYSQL_ROW	row;

extern  SFM_sfdb	*sfdb;
extern	ListenInfo	ne_info;
extern	OidInfo		oid_info[NMS_STAT_NUM];
extern	MYSQL		*conn;

extern	int		MY_NE_ID, STAT_ID_BASE, OID_BASE;
extern	int		ixpcQid;
extern	int		trcFlag, trcLogFlag;
extern	int		mySysEntry;
extern	char		mySysName[], myAppName[];
extern	char		traceBuf[];


handle_statistic (GeneralQMsgType *rmsg)
{
	int		period=0, k;
	int		res=0;
	int		find=0;
	char		f_name[400];
	char		sndbuf[4096];
	Packet1		*finfo;
	StatQueryInfo	*qry_t;

	memset (f_name, 0, 400);
	memset (sndbuf, 0, 4096);

	finfo = (Packet1 *)sndbuf;

	qry_t = (StatQueryInfo *)rmsg->body;

	period = qry_t->period;

	/* 1. DB query
	 * 2. make file
	 */
	if (form_dsc2nms ((StatQueryInfo *)qry_t, f_name) < 0) {
		printf ("[*] fail statistics file \n");
		return -1;
	}

	/* make NMS signal (file information)
	*/
	sprintf (sndbuf, "F%s", f_name);

	/* send signal to NMS (file information)
	*/
	for (k=0; k<MAX_NMS_CON; k++) {
		if ((sfdb->nmsInfo.fd[k] > 0) &&
			(sfdb->nmsInfo.ptype[k] == FD_TYPE_DATA) &&
			(sfdb->nmsInfo.port[k] == ne_info.port[PORT_IDX_STAT])) {
			find = 1;
			res = 0;
			res = send_packet (sfdb->nmsInfo.fd[k], sndbuf, 129);

			if (res > 0) {
				save_fname (f_name, sfdb->nmsInfo.fd[k], period);
			}
		}
	}

	/* not exist NMS connection
	*/
	if (find == 0) {
		if (trcFlag || trcLogFlag) {
			sprintf (traceBuf, "not exist NMS connection\n");
			trclib_writeLog (FL, traceBuf);
		}
		save_fname (f_name, 0, period);
	}

	return 1;

} /* End of handle_statistic () */


/* 참고 :
 *    - 통계 포맷 변환(Local->NMS) 시 DB 컬럼 순서대로 나열하는 것을 원칙으로 함 (개발의 용이성)
 *    - 이는 통계 출력 파라미터 별 SID 값이 규칙성(sequential) 있게 정의되는 것을 전제로 하며,
 *      이 전제가 지켜지지 않으면 하드코딩이 불가피함.
 */
form_dsc2nms(StatQueryInfo *qry_info, char *ofile)
{
	size_t			flen, wlen;
	int				i, j, k, t, prd, p_idx;
	unsigned int	uCount[EQUIP_MAX_COUNT][LEVEL_MAX_COUNT], uLoad[EQUIP_MAX_COUNT][LOAD_MAX_COUNT];
	char			cEquipIdx, *env, query[1024], wbuf[2048], fpath[200];
	MYSQL_RES		*result;
	struct tm		*ntm;
	FILE			*ofp;

	if( (env = getenv(IV_HOME)) == NULL)
	{
		sprintf (traceBuf, "not found %s env_variable\n", IV_HOME);
		trclib_writeLogErr (FL, traceBuf);
		return -1;
	}

	ntm = (struct tm*)now();

	prd = qry_info->period;

	if(prd == STAT_PERIOD_5MIN)
	{
		sprintf(fpath, "%s/%s5MIN/", env, NMS_STAT_DIR);
		sprintf(ofile, "10DSC%02d-M-%04d%02d%02d%02d%02d", mySysEntry, ntm->tm_year+1900, ntm->tm_mon+1, ntm->tm_mday, ntm->tm_hour, get_abs_min (ntm->tm_min));
	}
	else if (prd == STAT_PERIOD_HOUR)
	{
		sprintf(fpath, "%s/%sHOUR/", env, NMS_STAT_DIR);
		sprintf(ofile, "10DSC%02d-A-%04d%02d%02d%02d", mySysEntry, ntm->tm_year+1900, ntm->tm_mon+1, ntm->tm_mday, ntm->tm_hour);
	}
	else
	{
		sprintf(fpath, "%s/%sDAY/", env, NMS_STAT_DIR);
		sprintf(ofile, "10DSC%02d-A-%04d%02d%02d", mySysEntry, ntm->tm_year+1900, ntm->tm_mon+1, ntm->tm_mday);
	}

	flen = strlen(fpath);
	sprintf(&fpath[flen], "%s", ofile);

	/*	~/LOG/NMS_DIR/5MIN/ or ~/.../1HOUR/ 폴더에 통계 전송화일을 생성한다.	*/
	if( (ofp = fopen(fpath, "a+")) == NULL)
	{
		sprintf(traceBuf, "can't open file (%s) : %s\n", fpath, strerror(errno));
		trclib_writeLogErr(FL, traceBuf);
		return -2;
	}

	p_idx = prd - 1;

	for(i = 0; i < NMS_STAT_NUM; i++)
	{
		/* DB 테이블명에 없는 경우 skip */
		if(strlen(oid_info[i].cTableName[p_idx]) == 0)
			continue;

		switch(i)
		{
			case HW_FAULT:
				sprintf(query, "SELECT system_name, etc_hw_cri_cnt, etc_hw_maj_cnt, etc_hw_min_cnt from %s where (stat_date>='%s' and stat_date<='%s')",
					oid_info[i].cTableName[p_idx], qry_info->stime, qry_info->stime);

				if(mysql_query(conn, query) != 0)
				{
					sprintf(traceBuf, "mysql_query: %s\n", mysql_error(conn));
					trclib_writeLogErr(FL, traceBuf);
					fclose(ofp);
					return -3;
				}

				if( (result = mysql_store_result(conn)) == NULL)
				{
					sprintf(traceBuf, "F=%s:%s.%d: FAILED IN mysql_store_result [%d] [%s] [%s]", __FILE__, __FUNCTION__, __LINE__,
						mysql_errno(conn), mysql_error(conn), query);
					trclib_writeLogErr(FL, traceBuf);
					fclose(ofp);
					return -4;
				}

				for(j = 0; j < EQUIP_MAX_COUNT; j++)
				{
					for(k = 0; k < LEVEL_MAX_COUNT; k++)
						uCount[j][k]	= 0;
				}

				t = 0;
				while( (row = mysql_fetch_row(result)) != NULL)
				{
					if(t >= EQUIP_MAX_COUNT)
					{
						sprintf(traceBuf, "F=%s:%s.%d: MAX_EQUIP_COUNT[%d] OVER[%d] [%s]", __FILE__, __FUNCTION__, __LINE__, EQUIP_MAX_COUNT, i, query);
						trclib_writeLogErr(FL, traceBuf);
						break;
					}

					if(strcmp(row[0], "TAPA") == 0)
						cEquipIdx = EQUIP_TAPA;
					else if(strcmp(row[0], "TAPB") == 0)
						cEquipIdx = EQUIP_TAPB;
					else if(strcmp(row[0], "SCEA") == 0)
						cEquipIdx = EQUIP_SCEA;
					else if(strcmp(row[0], "SCEB") == 0)
						cEquipIdx = EQUIP_SCEB;
					else if(strcmp(row[0], "SCMA") == 0)
						cEquipIdx = EQUIP_SCMA;
					else if(strcmp(row[0], "SCMB") == 0)
						cEquipIdx = EQUIP_SCMB;
					else if(strcmp(row[0], "DSCM") == 0)
						cEquipIdx = EQUIP_DSCM;
					else
					{
						sprintf(traceBuf, "F=%s:%s.%d: Unknown EquipType[%s]", __FILE__, __FUNCTION__, __LINE__, row[0]);
						trclib_writeLogErr(FL, traceBuf);
						continue;
					}

					uCount[cEquipIdx][LEVEL_CRITICAL]	= (unsigned int)atoi(row[1]);
					uCount[cEquipIdx][LEVEL_MAJOR]		= (unsigned int)atoi(row[2]);
					uCount[cEquipIdx][LEVEL_MINOR]		= (unsigned int)atoi(row[3]);
					t++;
				}
				mysql_free_result(result);

				memset(wbuf, 0x00, sizeof(wbuf));
				wlen = 0;

				sprintf(&wbuf[wlen], "REPLACE\n");
				wlen = strlen(wbuf);

				sprintf(&wbuf[wlen], "%d:I:%d:\n", MY_NE_ID, oid_info[HW_FAULT].dObjectID);
				wlen = strlen(wbuf);

				for(j = 0; j < EQUIP_MAX_COUNT; j++)
				{
					for(k = 0; k < LEVEL_MAX_COUNT; k++)
					{
						sprintf(&wbuf[wlen], "%d:I:%u:\n", OID_BASE+oid_info[i].dSidFirstNum+((j*LEVEL_MAX_COUNT)+k), uCount[j][k]);
						wlen = strlen(wbuf);
					}
				}
				/* 통계 출력 파라미터 종료 표시 => 10043:C:xxxx: */
				sprintf(&wbuf[wlen], "10043:C:%s:\n", (char*)get_oidtime(prd));
				fprintf(ofp, "%s\n", wbuf);
				fflush(ofp);
				break;

			case CPU_FAULT:
				sprintf(query, "SELECT system_name, cpu_cri_cnt, cpu_maj_cnt, cpu_min_cnt from %s where (stat_date>='%s' and stat_date<='%s')",
					oid_info[i].cTableName[p_idx], qry_info->stime, qry_info->stime);

				if(mysql_query(conn, query) != 0)
				{
					sprintf(traceBuf, "mysql_query: %s\n", mysql_error(conn));
					trclib_writeLogErr(FL, traceBuf);
					fclose(ofp);
					return -3;
				}

				if( (result = mysql_store_result(conn)) == NULL)
				{
					sprintf(traceBuf, "F=%s:%s.%d: FAILED IN mysql_store_result [%d] [%s] [%s]", __FILE__, __FUNCTION__, __LINE__,
						mysql_errno(conn), mysql_error(conn), query);
					trclib_writeLogErr(FL, traceBuf);
					fclose(ofp);
					return -4;
				}

				for(j = 0; j < EQUIP_MAX_COUNT; j++)
				{
					for(k = 0; k < LEVEL_MAX_COUNT; k++)
						uCount[j][k]	= 0;
				}

				t = 0;
				while( (row = mysql_fetch_row(result)) != NULL)
				{
					if(t >= EQUIP_MAX_COUNT)
					{
						sprintf(traceBuf, "F=%s:%s.%d: MAX_EQUIP_COUNT[%d] OVER[%d] [%s]", __FILE__, __FUNCTION__, __LINE__, EQUIP_MAX_COUNT, i, query);
						trclib_writeLogErr(FL, traceBuf);
						break;
					}

					if(strcmp(row[0], "TAPA") == 0)
						cEquipIdx = EQUIP_TAPA;
					else if(strcmp(row[0], "TAPB") == 0)
						cEquipIdx = EQUIP_TAPB;
					else if(strcmp(row[0], "SCEA") == 0)
						cEquipIdx = EQUIP_SCEA;
					else if(strcmp(row[0], "SCEB") == 0)
						cEquipIdx = EQUIP_SCEB;
					else if(strcmp(row[0], "SCMA") == 0)
						cEquipIdx = EQUIP_SCMA;
					else if(strcmp(row[0], "SCMB") == 0)
						cEquipIdx = EQUIP_SCMB;
					else if(strcmp(row[0], "DSCM") == 0)
						cEquipIdx = EQUIP_DSCM;
					else
					{
						sprintf(traceBuf, "F=%s:%s.%d: Unknown EquipType[%s]", __FILE__, __FUNCTION__, __LINE__, row[0]);
						trclib_writeLogErr(FL, traceBuf);
						continue;
					}

					uCount[cEquipIdx][LEVEL_CRITICAL]	= (unsigned int)atoi(row[1]);
					uCount[cEquipIdx][LEVEL_MAJOR]		= (unsigned int)atoi(row[2]);
					uCount[cEquipIdx][LEVEL_MINOR]		= (unsigned int)atoi(row[3]);
					t++;
				}
				mysql_free_result(result);

				memset(wbuf, 0x00, sizeof(wbuf));
				wlen = 0;

				sprintf(&wbuf[wlen], "REPLACE\n");
				wlen = strlen(wbuf);

				sprintf(&wbuf[wlen], "%d:I:%d:\n", MY_NE_ID, oid_info[i].dObjectID);
				wlen = strlen(wbuf);

				for(j = 0; j < EQUIP_MAX_COUNT; j++)
				{
					switch(j)
					{
						case 0:
							cEquipIdx = EQUIP_SCMA;
							break;
						case 1:
							cEquipIdx = EQUIP_SCMB;
							break;
						case 2:
							cEquipIdx = EQUIP_DSCM;
							break;
						case 3:
							cEquipIdx = EQUIP_SCEA;
							break;
						case 4:
							cEquipIdx = EQUIP_SCEB;
							break;
						case 5:
							cEquipIdx = EQUIP_TAPA;
							break;
						case 6:
							cEquipIdx = EQUIP_TAPB;
							break;
					}

					for(k = 0; k < LEVEL_MAX_COUNT; k++)
					{
						sprintf(&wbuf[wlen], "%d:I:%u:\n", OID_BASE+oid_info[i].dSidFirstNum+((j*LEVEL_MAX_COUNT)+k), uCount[cEquipIdx][k]);
						wlen = strlen(wbuf);
					}
				}
				/* 통계 출력 파라미터 종료 표시 => 10043:C:xxxx: */
				sprintf(&wbuf[wlen], "10043:C:%s:\n", (char*)get_oidtime(prd));
				fprintf(ofp, "%s\n", wbuf);
				fflush(ofp);
				break;

			case MEM_FAULT:
				sprintf(query, "SELECT system_name, mem_cri_cnt, mem_maj_cnt, mem_min_cnt from %s where (stat_date>='%s' and stat_date<='%s')",
					oid_info[i].cTableName[p_idx], qry_info->stime, qry_info->stime);

				if(mysql_query(conn, query) != 0)
				{
					sprintf(traceBuf, "mysql_query: %s\n", mysql_error(conn));
					trclib_writeLogErr(FL, traceBuf);
					fclose(ofp);
					return -3;
				}

				if( (result = mysql_store_result(conn)) == NULL)
				{
					sprintf(traceBuf, "F=%s:%s.%d: FAILED IN mysql_store_result [%d] [%s] [%s]", __FILE__, __FUNCTION__, __LINE__,
						mysql_errno(conn), mysql_error(conn), query);
					trclib_writeLogErr(FL, traceBuf);
					fclose(ofp);
					return -4;
				}

				for(j = 0; j < EQUIP_MAX_COUNT; j++)
				{
					for(k = 0; k < LEVEL_MAX_COUNT; k++)
						uCount[j][k]	= 0;
				}

				t = 0;
				while( (row = mysql_fetch_row(result)) != NULL)
				{
					if(t >= EQUIP_MAX_COUNT)
					{
						sprintf(traceBuf, "F=%s:%s.%d: MAX_EQUIP_COUNT[%d] OVER[%d] [%s]", __FILE__, __FUNCTION__, __LINE__, EQUIP_MAX_COUNT, i, query);
						trclib_writeLogErr(FL, traceBuf);
						break;
					}

					if(strcmp(row[0], "TAPA") == 0)
						cEquipIdx = EQUIP_TAPA;
					else if(strcmp(row[0], "TAPB") == 0)
						cEquipIdx = EQUIP_TAPB;
					else if(strcmp(row[0], "SCEA") == 0)
						cEquipIdx = EQUIP_SCEA;
					else if(strcmp(row[0], "SCEB") == 0)
						cEquipIdx = EQUIP_SCEB;
					else if(strcmp(row[0], "SCMA") == 0)
						cEquipIdx = EQUIP_SCMA;
					else if(strcmp(row[0], "SCMB") == 0)
						cEquipIdx = EQUIP_SCMB;
					else if(strcmp(row[0], "DSCM") == 0)
						cEquipIdx = EQUIP_DSCM;
					else
					{
						sprintf(traceBuf, "F=%s:%s.%d: Unknown EquipType[%s]", __FILE__, __FUNCTION__, __LINE__, row[0]);
						trclib_writeLogErr(FL, traceBuf);
						continue;
					}

					uCount[cEquipIdx][LEVEL_CRITICAL]	= (unsigned int)atoi(row[1]);
					uCount[cEquipIdx][LEVEL_MAJOR]		= (unsigned int)atoi(row[2]);
					uCount[cEquipIdx][LEVEL_MINOR]		= (unsigned int)atoi(row[3]);
					t++;
				}
				mysql_free_result(result);

				memset(wbuf, 0x00, sizeof(wbuf));
				wlen = 0;

				sprintf(&wbuf[wlen], "REPLACE\n");
				wlen = strlen(wbuf);

				sprintf(&wbuf[wlen], "%d:I:%d:\n", MY_NE_ID, oid_info[i].dObjectID);
				wlen = strlen(wbuf);

				for(j = 0; j < EQUIP_MAX_COUNT; j++)
				{
					switch(j)
					{
						case 0:
							cEquipIdx = EQUIP_SCMA;
							break;
						case 1:
							cEquipIdx = EQUIP_SCMB;
							break;
						case 2:
							cEquipIdx = EQUIP_DSCM;
							break;
						case 3:
							cEquipIdx = EQUIP_SCEA;
							break;
						case 4:
							cEquipIdx = EQUIP_SCEB;
							break;
						case 5:
							cEquipIdx = EQUIP_TAPA;
							break;
						case 6:
							cEquipIdx = EQUIP_TAPB;
							break;
					}

					for(k = 0; k < LEVEL_MAX_COUNT; k++)
					{
						sprintf(&wbuf[wlen], "%d:I:%u:\n", OID_BASE+oid_info[i].dSidFirstNum+((j*LEVEL_MAX_COUNT)+k), uCount[cEquipIdx][k]);
						wlen = strlen(wbuf);
					}
				}
				/* 통계 출력 파라미터 종료 표시 => 10043:C:xxxx: */
				sprintf(&wbuf[wlen], "10043:C:%s:\n", (char*)get_oidtime(prd));
				fprintf(ofp, "%s\n", wbuf);
				fflush(ofp);
				break;

			case PROC_FAULT:
				sprintf(query, "SELECT system_name, proc_cri_cnt, proc_maj_cnt, proc_min_cnt from %s where (stat_date>='%s' and stat_date<='%s')",
					oid_info[i].cTableName[p_idx], qry_info->stime, qry_info->stime);

				if(mysql_query(conn, query) != 0)
				{
					sprintf(traceBuf, "mysql_query: %s\n", mysql_error(conn));
					trclib_writeLogErr(FL, traceBuf);
					fclose(ofp);
					return -3;
				}

				if( (result = mysql_store_result(conn)) == NULL)
				{
					sprintf(traceBuf, "F=%s:%s.%d: FAILED IN mysql_store_result [%d] [%s] [%s]", __FILE__, __FUNCTION__, __LINE__,
						mysql_errno(conn), mysql_error(conn), query);
					trclib_writeLogErr(FL, traceBuf);
					fclose(ofp);
					return -4;
				}

				for(j = 0; j < EQUIP_MAX_COUNT; j++)
				{
					for(k = 0; k < LEVEL_MAX_COUNT; k++)
						uCount[j][k]	= 0;
				}

				t = 0;
				while( (row = mysql_fetch_row(result)) != NULL)
				{
					if(t >= EQUIP_MAX_COUNT)
					{
						sprintf(traceBuf, "F=%s:%s.%d: MAX_EQUIP_COUNT[%d] OVER[%d] [%s]", __FILE__, __FUNCTION__, __LINE__, EQUIP_MAX_COUNT, i, query);
						trclib_writeLogErr(FL, traceBuf);
						break;
					}

					if(strcmp(row[0], "TAPA") == 0)
						cEquipIdx = EQUIP_TAPA;
					else if(strcmp(row[0], "TAPB") == 0)
						cEquipIdx = EQUIP_TAPB;
					else if(strcmp(row[0], "SCEA") == 0)
						cEquipIdx = EQUIP_SCEA;
					else if(strcmp(row[0], "SCEB") == 0)
						cEquipIdx = EQUIP_SCEB;
					else if(strcmp(row[0], "SCMA") == 0)
						cEquipIdx = EQUIP_SCMA;
					else if(strcmp(row[0], "SCMB") == 0)
						cEquipIdx = EQUIP_SCMB;
					else if(strcmp(row[0], "DSCM") == 0)
						cEquipIdx = EQUIP_DSCM;
					else
					{
						sprintf(traceBuf, "F=%s:%s.%d: Unknown EquipType[%s]", __FILE__, __FUNCTION__, __LINE__, row[0]);
						trclib_writeLogErr(FL, traceBuf);
						continue;
					}

					uCount[cEquipIdx][LEVEL_CRITICAL]	= (unsigned int)atoi(row[1]);
					uCount[cEquipIdx][LEVEL_MAJOR]		= (unsigned int)atoi(row[2]);
					uCount[cEquipIdx][LEVEL_MINOR]		= (unsigned int)atoi(row[3]);
					t++;
				}
				mysql_free_result(result);

				memset(wbuf, 0x00, sizeof(wbuf));
				wlen = 0;

				sprintf(&wbuf[wlen], "REPLACE\n");
				wlen = strlen(wbuf);

				sprintf(&wbuf[wlen], "%d:I:%d:\n", MY_NE_ID, oid_info[i].dObjectID);
				wlen = strlen(wbuf);

				for(j = 0; j < EQUIP_MAX_COUNT; j++)
				{
					switch(j)
					{
						case 0:
							cEquipIdx = EQUIP_SCMA;
							break;
						case 1:
							cEquipIdx = EQUIP_SCMB;
							break;
						case 2:
							cEquipIdx = EQUIP_DSCM;
							break;
						default:
							continue;
					}

					for(k = 0; k < LEVEL_MAX_COUNT; k++)
					{
						sprintf(&wbuf[wlen], "%d:I:%u:\n", OID_BASE+oid_info[i].dSidFirstNum+((j*LEVEL_MAX_COUNT)+k), uCount[cEquipIdx][k]);
						wlen = strlen(wbuf);
					}
				}
				/* 통계 출력 파라미터 종료 표시 => 10043:C:xxxx: */
				sprintf(&wbuf[wlen], "10043:C:%s:\n", (char*)get_oidtime(prd));
				fprintf(ofp, "%s\n", wbuf);
				fflush(ofp);
				break;

			case CPU_LOAD:
				sprintf(query, "SELECT system_name, avr_cpu0, max_cpu0 from %s where (stat_date>='%s' and stat_date<='%s')",
					oid_info[i].cTableName[p_idx], qry_info->stime, qry_info->stime);

				if(mysql_query(conn, query) != 0)
				{
					sprintf(traceBuf, "mysql_query: %s\n", mysql_error(conn));
					trclib_writeLogErr(FL, traceBuf);
					fclose(ofp);
					return -3;
				}

				if( (result = mysql_store_result(conn)) == NULL)
				{
					sprintf(traceBuf, "F=%s:%s.%d: FAILED IN mysql_store_result [%d] [%s] [%s]", __FILE__, __FUNCTION__, __LINE__,
						mysql_errno(conn), mysql_error(conn), query);
					trclib_writeLogErr(FL, traceBuf);
					fclose(ofp);
					return -4;
				}

				for(j = 0; j < EQUIP_MAX_COUNT; j++)
				{
					for(k = 0; k < LOAD_MAX_COUNT; k++)
						uLoad[j][k]	= 0;
				}

				t = 0;
				while( (row = mysql_fetch_row(result)) != NULL)
				{
					if(t >= EQUIP_MAX_COUNT)
					{
						sprintf(traceBuf, "F=%s:%s.%d: MAX_EQUIP_COUNT[%d] OVER[%d] [%s]", __FILE__, __FUNCTION__, __LINE__, EQUIP_MAX_COUNT, i, query);
						trclib_writeLogErr(FL, traceBuf);
						break;
					}

					if(strcmp(row[0], "TAPA") == 0)
						cEquipIdx = EQUIP_TAPA;
					else if(strcmp(row[0], "TAPB") == 0)
						cEquipIdx = EQUIP_TAPB;
					else if(strcmp(row[0], "SCEA") == 0)
						cEquipIdx = EQUIP_SCEA;
					else if(strcmp(row[0], "SCEB") == 0)
						cEquipIdx = EQUIP_SCEB;
					else if(strcmp(row[0], "SCMA") == 0)
						cEquipIdx = EQUIP_SCMA;
					else if(strcmp(row[0], "SCMB") == 0)
						cEquipIdx = EQUIP_SCMB;
					else if(strcmp(row[0], "DSCM") == 0)
						cEquipIdx = EQUIP_DSCM;
					else
					{
						sprintf(traceBuf, "F=%s:%s.%d: Unknown EquipType[%s]", __FILE__, __FUNCTION__, __LINE__, row[0]);
						trclib_writeLogErr(FL, traceBuf);
						continue;
					}

					uLoad[cEquipIdx][LOAD_AVERAGE]		= (unsigned int)atoi(row[1]);
					uLoad[cEquipIdx][LOAD_MAXIMUM]		= (unsigned int)atoi(row[2]);
					t++;
				}
				mysql_free_result(result);

				memset(wbuf, 0x00, sizeof(wbuf));
				wlen = 0;

				sprintf(&wbuf[wlen], "REPLACE\n");
				wlen = strlen(wbuf);

				sprintf(&wbuf[wlen], "%d:I:%d:\n", MY_NE_ID, oid_info[i].dObjectID);
				wlen = strlen(wbuf);

				for(j = 0; j < EQUIP_MAX_COUNT; j++)
				{
					for(k = 0; k < LOAD_MAX_COUNT; k++)
					{
						sprintf(&wbuf[wlen], "%d:I:%u:\n", OID_BASE+oid_info[i].dSidFirstNum+((j*LOAD_MAX_COUNT)+k), uLoad[j][k]);
						wlen = strlen(wbuf);
					}
				}
				/* 통계 출력 파라미터 종료 표시 => 10043:C:xxxx: */
				sprintf(&wbuf[wlen], "10043:C:%s:\n", (char*)get_oidtime(prd));
				fprintf(ofp, "%s\n", wbuf);
				fflush(ofp);
				break;

			case MEM_LOAD:
				sprintf(query, "SELECT system_name, avr_memory, max_memory from %s where (stat_date>='%s' and stat_date<='%s')",
					oid_info[i].cTableName[p_idx], qry_info->stime, qry_info->stime);

				if(mysql_query(conn, query) != 0)
				{
					sprintf(traceBuf, "mysql_query: %s\n", mysql_error(conn));
					trclib_writeLogErr(FL, traceBuf);
					fclose(ofp);
					return -3;
				}

				if( (result = mysql_store_result(conn)) == NULL)
				{
					sprintf(traceBuf, "F=%s:%s.%d: FAILED IN mysql_store_result [%d] [%s] [%s]", __FILE__, __FUNCTION__, __LINE__,
						mysql_errno(conn), mysql_error(conn), query);
					trclib_writeLogErr(FL, traceBuf);
					fclose(ofp);
					return -4;
				}

				for(j = 0; j < EQUIP_MAX_COUNT; j++)
				{
					for(k = 0; k < LOAD_MAX_COUNT; k++)
						uLoad[j][k]	= 0;
				}

				t = 0;
				while( (row = mysql_fetch_row(result)) != NULL)
				{
					if(t >= EQUIP_MAX_COUNT)
					{
						sprintf(traceBuf, "F=%s:%s.%d: MAX_EQUIP_COUNT[%d] OVER[%d] [%s]", __FILE__, __FUNCTION__, __LINE__, EQUIP_MAX_COUNT, i, query);
						trclib_writeLogErr(FL, traceBuf);
						break;
					}

					if(strcmp(row[0], "TAPA") == 0)
						cEquipIdx = EQUIP_TAPA;
					else if(strcmp(row[0], "TAPB") == 0)
						cEquipIdx = EQUIP_TAPB;
					else if(strcmp(row[0], "SCEA") == 0)
						cEquipIdx = EQUIP_SCEA;
					else if(strcmp(row[0], "SCEB") == 0)
						cEquipIdx = EQUIP_SCEB;
					else if(strcmp(row[0], "SCMA") == 0)
						cEquipIdx = EQUIP_SCMA;
					else if(strcmp(row[0], "SCMB") == 0)
						cEquipIdx = EQUIP_SCMB;
					else if(strcmp(row[0], "DSCM") == 0)
						cEquipIdx = EQUIP_DSCM;
					else
					{
						sprintf(traceBuf, "F=%s:%s.%d: Unknown EquipType[%s]", __FILE__, __FUNCTION__, __LINE__, row[0]);
						trclib_writeLogErr(FL, traceBuf);
						continue;
					}

					uLoad[cEquipIdx][LOAD_AVERAGE]		= (unsigned int)atoi(row[1]);
					uLoad[cEquipIdx][LOAD_MAXIMUM]		= (unsigned int)atoi(row[2]);
					t++;
				}
				mysql_free_result(result);

				memset(wbuf, 0x00, sizeof(wbuf));
				wlen = 0;

				sprintf(&wbuf[wlen], "REPLACE\n");
				wlen = strlen(wbuf);

				sprintf(&wbuf[wlen], "%d:I:%d:\n", MY_NE_ID, oid_info[i].dObjectID);
				wlen = strlen(wbuf);

				for(j = 0; j < EQUIP_MAX_COUNT; j++)
				{
					for(k = 0; k < LOAD_MAX_COUNT; k++)
					{
						sprintf(&wbuf[wlen], "%d:I:%u:\n", OID_BASE+oid_info[i].dSidFirstNum+((j*LOAD_MAX_COUNT)+k), uLoad[j][k]);
						wlen = strlen(wbuf);
					}
				}
				/* 통계 출력 파라미터 종료 표시 => 10043:C:xxxx: */
				sprintf(&wbuf[wlen], "10043:C:%s:\n", (char*)get_oidtime(prd));
				fprintf(ofp, "%s\n", wbuf);
				fflush(ofp);
				break;

			case DISK_LOAD:
				sprintf(query, "SELECT system_name, avr_disk, max_disk from %s where (stat_date>='%s' and stat_date<='%s')",
					oid_info[i].cTableName[p_idx], qry_info->stime, qry_info->stime);

				if(mysql_query(conn, query) != 0)
				{
					sprintf(traceBuf, "mysql_query: %s\n", mysql_error(conn));
					trclib_writeLogErr(FL, traceBuf);
					fclose(ofp);
					return -3;
				}

				if( (result = mysql_store_result(conn)) == NULL)
				{
					sprintf(traceBuf, "F=%s:%s.%d: FAILED IN mysql_store_result [%d] [%s] [%s]", __FILE__, __FUNCTION__, __LINE__,
						mysql_errno(conn), mysql_error(conn), query);
					trclib_writeLogErr(FL, traceBuf);
					fclose(ofp);
					return -4;
				}

				for(j = 0; j < EQUIP_MAX_COUNT; j++)
				{
					for(k = 0; k < LOAD_MAX_COUNT; k++)
						uLoad[j][k]	= 0;
				}

				t = 0;
				while( (row = mysql_fetch_row(result)) != NULL)
				{
					if(t >= EQUIP_MAX_COUNT)
					{
						sprintf(traceBuf, "F=%s:%s.%d: MAX_EQUIP_COUNT[%d] OVER[%d] [%s]", __FILE__, __FUNCTION__, __LINE__, EQUIP_MAX_COUNT, i, query);
						trclib_writeLogErr(FL, traceBuf);
						break;
					}

					if(strcmp(row[0], "TAPA") == 0)
						cEquipIdx = EQUIP_TAPA;
					else if(strcmp(row[0], "TAPB") == 0)
						cEquipIdx = EQUIP_TAPB;
					else if(strcmp(row[0], "SCEA") == 0)
						cEquipIdx = EQUIP_SCEA;
					else if(strcmp(row[0], "SCEB") == 0)
						cEquipIdx = EQUIP_SCEB;
					else if(strcmp(row[0], "SCMA") == 0)
						cEquipIdx = EQUIP_SCMA;
					else if(strcmp(row[0], "SCMB") == 0)
						cEquipIdx = EQUIP_SCMB;
					else if(strcmp(row[0], "DSCM") == 0)
						cEquipIdx = EQUIP_DSCM;
					else
					{
						sprintf(traceBuf, "F=%s:%s.%d: Unknown EquipType[%s]", __FILE__, __FUNCTION__, __LINE__, row[0]);
						trclib_writeLogErr(FL, traceBuf);
						continue;
					}

					uLoad[cEquipIdx][LOAD_AVERAGE]		= (unsigned int)atoi(row[1]);
					uLoad[cEquipIdx][LOAD_MAXIMUM]		= (unsigned int)atoi(row[2]);
					t++;
				}
				mysql_free_result(result);

				memset(wbuf, 0x00, sizeof(wbuf));
				wlen = 0;

				sprintf(&wbuf[wlen], "REPLACE\n");
				wlen = strlen(wbuf);

				sprintf(&wbuf[wlen], "%d:I:%d:\n", MY_NE_ID, oid_info[i].dObjectID);
				wlen = strlen(wbuf);

				for(j = 0; j < EQUIP_MAX_COUNT; j++)
				{
					switch(j)
					{
						case 0:
							cEquipIdx = EQUIP_SCMA;
							break;
						case 1:
							cEquipIdx = EQUIP_SCMB;
							break;
						case 2:
							cEquipIdx = EQUIP_DSCM;
							break;
						default:
							continue;
					}

					for(k = 0; k < LOAD_MAX_COUNT; k++)
					{
						sprintf(&wbuf[wlen], "%d:I:%u:\n", OID_BASE+oid_info[i].dSidFirstNum+((j*LOAD_MAX_COUNT)+k), uLoad[cEquipIdx][k]);
						wlen = strlen(wbuf);
					}
				}
				/* 통계 출력 파라미터 종료 표시 => 10043:C:xxxx: */
				sprintf(&wbuf[wlen], "10043:C:%s:\n", (char*)get_oidtime(prd));
				fprintf(ofp, "%s\n", wbuf);
				fflush(ofp);
				break;

			case MSGQ_LOAD:
				sprintf(query, "SELECT system_name, avr_msgQ, max_msgQ from %s where (stat_date>='%s' and stat_date<='%s')",
					oid_info[i].cTableName[p_idx], qry_info->stime, qry_info->stime);

				if(mysql_query(conn, query) != 0)
				{
					sprintf(traceBuf, "mysql_query: %s\n", mysql_error(conn));
					trclib_writeLogErr(FL, traceBuf);
					fclose(ofp);
					return -3;
				}

				if( (result = mysql_store_result(conn)) == NULL)
				{
					sprintf(traceBuf, "F=%s:%s.%d: FAILED IN mysql_store_result [%d] [%s] [%s]", __FILE__, __FUNCTION__, __LINE__,
						mysql_errno(conn), mysql_error(conn), query);
					trclib_writeLogErr(FL, traceBuf);
					fclose(ofp);
					return -4;
				}

				for(j = 0; j < EQUIP_MAX_COUNT; j++)
				{
					for(k = 0; k < LOAD_MAX_COUNT; k++)
						uLoad[j][k]	= 0;
				}

				t = 0;
				while( (row = mysql_fetch_row(result)) != NULL)
				{
					if(t >= EQUIP_MAX_COUNT)
					{
						sprintf(traceBuf, "F=%s:%s.%d: MAX_EQUIP_COUNT[%d] OVER[%d] [%s]", __FILE__, __FUNCTION__, __LINE__, EQUIP_MAX_COUNT, i, query);
						trclib_writeLogErr(FL, traceBuf);
						break;
					}

					if(strcmp(row[0], "TAPA") == 0)
						cEquipIdx = EQUIP_TAPA;
					else if(strcmp(row[0], "TAPB") == 0)
						cEquipIdx = EQUIP_TAPB;
					else if(strcmp(row[0], "SCEA") == 0)
						cEquipIdx = EQUIP_SCEA;
					else if(strcmp(row[0], "SCEB") == 0)
						cEquipIdx = EQUIP_SCEB;
					else if(strcmp(row[0], "SCMA") == 0)
						cEquipIdx = EQUIP_SCMA;
					else if(strcmp(row[0], "SCMB") == 0)
						cEquipIdx = EQUIP_SCMB;
					else if(strcmp(row[0], "DSCM") == 0)
						cEquipIdx = EQUIP_DSCM;
					else
					{
						sprintf(traceBuf, "F=%s:%s.%d: Unknown EquipType[%s]", __FILE__, __FUNCTION__, __LINE__, row[0]);
						trclib_writeLogErr(FL, traceBuf);
						continue;
					}

					uLoad[cEquipIdx][LOAD_AVERAGE]		= (unsigned int)atoi(row[1]);
					uLoad[cEquipIdx][LOAD_MAXIMUM]		= (unsigned int)atoi(row[2]);
					t++;
				}
				mysql_free_result(result);

				memset(wbuf, 0x00, sizeof(wbuf));
				wlen = 0;

				sprintf(&wbuf[wlen], "REPLACE\n");
				wlen = strlen(wbuf);

				sprintf(&wbuf[wlen], "%d:I:%d:\n", MY_NE_ID, oid_info[i].dObjectID);
				wlen = strlen(wbuf);

				for(j = 0; j < EQUIP_MAX_COUNT; j++)
				{
					switch(j)
					{
						case 0:
							cEquipIdx = EQUIP_SCMA;
							break;
						case 1:
							cEquipIdx = EQUIP_SCMB;
							break;
						case 2:
							cEquipIdx = EQUIP_DSCM;
							break;
						default:
							continue;
					}

					for(k = 0; k < LOAD_MAX_COUNT; k++)
					{
						sprintf(&wbuf[wlen], "%d:I:%u:\n", OID_BASE+oid_info[i].dSidFirstNum+((j*LOAD_MAX_COUNT)+k), uLoad[j][k]);
						wlen = strlen(wbuf);
					}
				}
				/* 통계 출력 파라미터 종료 표시 => 10043:C:xxxx: */
				sprintf(&wbuf[wlen], "10043:C:%s:\n", (char*)get_oidtime(prd));
				fprintf(ofp, "%s\n", wbuf);
				fflush(ofp);
				break;

			case TOTAL_TRAFFIC:
				break;

			case LEG_PROCESS:
				sprintf(query, "SELECT system_name, pdsn_ip, rx_cnt, start, interim, STOP, start_logon_cnt, int_logon_cnt, logout_cnt from %s where (stat_date>='%s' and stat_date<='%s')",
					oid_info[i].cTableName[p_idx], qry_info->stime, qry_info->stime);

				if(mysql_query(conn, query) != 0)
				{
					sprintf(traceBuf, "mysql_query: %s\n", mysql_error(conn));
					trclib_writeLogErr(FL, traceBuf);
					fclose(ofp);
					return -3;
				}

				if( (result = mysql_store_result(conn)) == NULL)
				{
					sprintf(traceBuf, "F=%s:%s.%d: FAILED IN mysql_store_result [%d] [%s] [%s]", __FILE__, __FUNCTION__, __LINE__,
						mysql_errno(conn), mysql_error(conn), query);
					trclib_writeLogErr(FL, traceBuf);
					fclose(ofp);
					return -4;
				}

				while( (row = mysql_fetch_row(result)) != NULL)
				{
					memset(wbuf, 0x00, sizeof(wbuf));
					wlen = 0;

					sprintf(&wbuf[wlen], "REPLACE\n");
					wlen = strlen(wbuf);

					sprintf(&wbuf[wlen], "%d:I:%d:\n", MY_NE_ID, oid_info[i].dObjectID);
					wlen = strlen(wbuf);

					sprintf(&wbuf[wlen], "%d:C:%s:\n", OID_BASE+oid_info[TOTAL_TRAFFIC].dSidFirstNum, row[0]);
					wlen = strlen(wbuf);

					for(t = 0; t < 8; t++)
					{
						sprintf(&wbuf[wlen], "%d:%c:%s:\n", OID_BASE+oid_info[i].dSidFirstNum+t, (t == 0) ? 'C' : 'I', row[t+1]);
						wlen = strlen(wbuf);
					}

					/* 통계 출력 파라미터 종료 표시 => 10043:C:xxxx: */
					sprintf(&wbuf[wlen], "10043:C:%s:\n", (char*)get_oidtime(prd));
					fprintf(ofp, "%s\n", wbuf);
					fflush(ofp);
				}
				mysql_free_result(result);
				break;

			case LOGON_PROCESS:
				sprintf(query, "SELECT system_name, log_req, log_succ, log_fail, HBIT_0, HBIT_1, HBIT_2, HBIT_3, SM_INT_ERR, OP_ERR, OP_TIMEOUT, ETC_FAIL from %s where (stat_date>='%s' and stat_date<='%s')",
					oid_info[i].cTableName[p_idx], qry_info->stime, qry_info->stime);

				if(mysql_query(conn, query) != 0)
				{
					sprintf(traceBuf, "mysql_query: %s\n", mysql_error(conn));
					trclib_writeLogErr(FL, traceBuf);
					fclose(ofp);
					return -3;
				}

				if( (result = mysql_store_result(conn)) == NULL)
				{
					sprintf(traceBuf, "F=%s:%s.%d: FAILED IN mysql_store_result [%d] [%s] [%s]", __FILE__, __FUNCTION__, __LINE__,
						mysql_errno(conn), mysql_error(conn), query);
					trclib_writeLogErr(FL, traceBuf);
					fclose(ofp);
					return -4;
				}

				while( (row = mysql_fetch_row(result)) != NULL)
				{
					memset(wbuf, 0x00, sizeof(wbuf));
					wlen = 0;

					sprintf(&wbuf[wlen], "REPLACE\n");
					wlen = strlen(wbuf);

					sprintf(&wbuf[wlen], "%d:I:%d:\n", MY_NE_ID, oid_info[i].dObjectID);
					wlen = strlen(wbuf);

					sprintf(&wbuf[wlen], "%d:C:%s:\n", OID_BASE+oid_info[TOTAL_TRAFFIC].dSidFirstNum, row[0]);
					wlen = strlen(wbuf);

					for(t = 0; t < 11; t++)
					{
						sprintf(&wbuf[wlen], "%d:I:%s:\n", OID_BASE+oid_info[i].dSidFirstNum+t, row[t+1]);
						wlen = strlen(wbuf);
					}

					/* 통계 출력 파라미터 종료 표시 => 10043:C:xxxx: */
					sprintf(&wbuf[wlen], "10043:C:%s:\n", (char*)get_oidtime(prd));
					fprintf(ofp, "%s\n", wbuf);
					fflush(ofp);
				}
				mysql_free_result(result);
				break;

			case RULESET_TRAFFIC:
				break;

			case RULESETENTRY_TRAFFIC:
				break;

			case SMSC_INTERLOCK:
				sprintf(query, "SELECT system_name, smsc_ip, req, succ, fail, smpp_err, svr_err, smsc_err, etc_err from %s where (stat_date>='%s' and stat_date<='%s')",
					oid_info[i].cTableName[p_idx], qry_info->stime, qry_info->stime);

				if(mysql_query(conn, query) != 0)
				{
					sprintf(traceBuf, "mysql_query: %s\n", mysql_error(conn));
					trclib_writeLogErr(FL, traceBuf);
					fclose(ofp);
					return -3;
				}

				if( (result = mysql_store_result(conn)) == NULL)
				{
					sprintf(traceBuf, "F=%s:%s.%d: FAILED IN mysql_store_result [%d] [%s] [%s]", __FILE__, __FUNCTION__, __LINE__,
						mysql_errno(conn), mysql_error(conn), query);
					trclib_writeLogErr(FL, traceBuf);
					fclose(ofp);
					return -4;
				}

				while( (row = mysql_fetch_row(result)) != NULL)
				{
					memset(wbuf, 0x00, sizeof(wbuf));
					wlen = 0;

					sprintf(&wbuf[wlen], "REPLACE\n");
					wlen = strlen(wbuf);

					sprintf(&wbuf[wlen], "%d:I:%d:\n", MY_NE_ID, oid_info[i].dObjectID);
					wlen = strlen(wbuf);

					sprintf(&wbuf[wlen], "%d:C:%s:\n", OID_BASE+oid_info[TOTAL_TRAFFIC].dSidFirstNum, row[0]);
					wlen = strlen(wbuf);

					for(t = 0; t < 8; t++)
					{
						sprintf(&wbuf[wlen], "%d:%c:%s:\n", OID_BASE+oid_info[i].dSidFirstNum+t, (t == 0) ? 'C' : 'I', row[t+1]);
						wlen = strlen(wbuf);
					}

					/* 통계 출력 파라미터 종료 표시 => 10043:C:xxxx: */
					sprintf(&wbuf[wlen], "10043:C:%s:\n", (char*)get_oidtime(prd));
					fprintf(ofp, "%s\n", wbuf);
					fflush(ofp);
				}
				mysql_free_result(result);
				break;

			case DELAY_PACKET:
				sprintf(query, "SELECT system_name, avg_usec from %s where (stat_date>='%s' and stat_date<='%s')",
					oid_info[i].cTableName[p_idx], qry_info->stime, qry_info->stime);

				if(mysql_query(conn, query) != 0)
				{
					sprintf(traceBuf, "mysql_query: %s\n", mysql_error(conn));
					trclib_writeLogErr(FL, traceBuf);
					fclose(ofp);
					return -3;
				}

				if( (result = mysql_store_result(conn)) == NULL)
				{
					sprintf(traceBuf, "F=%s:%s.%d: FAILED IN mysql_store_result [%d] [%s] [%s]", __FILE__, __FUNCTION__, __LINE__,
						mysql_errno(conn), mysql_error(conn), query);
					trclib_writeLogErr(FL, traceBuf);
					fclose(ofp);
					return -4;
				}

				while( (row = mysql_fetch_row(result)) != NULL)
				{
					memset(wbuf, 0x00, sizeof(wbuf));
					wlen = 0;

					sprintf(&wbuf[wlen], "REPLACE\n");
					wlen = strlen(wbuf);

					sprintf(&wbuf[wlen], "%d:I:%d:\n", MY_NE_ID, oid_info[i].dObjectID);
					wlen = strlen(wbuf);

					sprintf(&wbuf[wlen], "%d:C:%s:\n", OID_BASE+oid_info[TOTAL_TRAFFIC].dSidFirstNum, row[0]);
					wlen = strlen(wbuf);

					sprintf(&wbuf[wlen], "%d:F:%s:\n", OID_BASE+oid_info[i].dSidFirstNum, row[1]);
					wlen = strlen(wbuf);

					/* 통계 출력 파라미터 종료 표시 => 10043:C:xxxx: */
					sprintf(&wbuf[wlen], "10043:C:%s:\n", (char*)get_oidtime(prd));
					fprintf(ofp, "%s\n", wbuf);
					fflush(ofp);
				}
				mysql_free_result(result);
				break;

			default:
				break;
		}
	}
	fclose(ofp);

	return 1;
} /* End of form_dsc2nms () */



send_primitive (int mtype, int *tsfd)
{
	int			rval=0;
	int			snd_len=0;
	char		txbuf[4096];
	Packet1		*pkt1;
	Packet2		*pkt2;
	Packet3		*pkt3;


	memset (txbuf, 0, 4096);

	switch (mtype) {

		case PRMT_INIT_HWALM_CONFIRM :
		case PRMT_INIT_SWALM_CONFIRM :
		case PRMT_HW_EQUIP_CONFIRM :
		case PRMT_SW_EQUIP_CONFIRM :
		case PRMT_MMC_CONFIRM :

			pkt2 = (Packet2 *)txbuf;

			pkt2->hdr.chd.ver		= 0;
			pkt2->hdr.chd.ctrlbit	= 0;
			pkt2->hdr.chd.pktype	= 2;	// packet2
			pkt2->hdr.chd.ctrlseq	= 0;
			pkt2->hdr.chd.src		= 0x03;	// BSD
			pkt2->hdr.chd.dest		= 0x01;	// NMS
			pkt2->hdr.prmt			= 0;
			pkt2->hdr.len			= PKT2_HEAD_SIZE;
			pkt2->msgtype			= mtype;

			send_packet (*tsfd, (char *)txbuf, PKT2_HEAD_SIZE);

			break;

		case PRMT_INIT_HWALM_DATA :
		case PRMT_INIT_SWALM_DATA :

			pkt3 = (Packet3 *)txbuf;

			pkt3->hdr.chd.ver		= 0;
			pkt3->hdr.chd.ctrlbit	= 0;
			pkt3->hdr.chd.pktype	= 3;	// packet3
			pkt3->hdr.chd.ctrlseq	= 0;
			pkt3->hdr.chd.src		= 0x03;	// BSD
			pkt3->hdr.chd.dest		= 0x01;	// NMS
			pkt3->hdr.prmt			= 0;
			pkt3->msgtype			= mtype;

			/* send data (continue) */
			rval = send_current_alm (tsfd, txbuf, mtype);

			/* send end signal */
			if (mtype == PRMT_INIT_HWALM_DATA)
				pkt3->msgtype	= PRMT_INIT_HWALM_DATA_END;
			else pkt3->msgtype	= PRMT_INIT_SWALM_DATA_END;

			pkt3->hdr.len			= PKT3_HEAD_SIZE;

			send_packet (*tsfd, (char *)txbuf, PKT3_HEAD_SIZE);

			break;

#if 0 // at work
		case PRMT_HW_EQUIP_DATA :
		case PRMT_SW_EQUIP_DATA :

			rval = get_config_data (mtype, );

			for (i=0; i<rval; i++) {
				txpkt->hdr.prmt = mtype;
				txpkt->hdr.len	= ??;
				.....
			}
			break;
#endif
		default :
			printf ("[*] \n");
			break;
	}

} /* End of send_primitive () */


send_current_alm (int *sfd, char *tbuf, int mtype)
{
	int			ret, i, len=0;
	char		query[1024];
	Packet3		*almpkt;
	MYSQL_ROW	row;
	MYSQL_RES	*res;

	almpkt = (Packet3 *)tbuf;

	sprintf (query, "SELECT * FROM %s", "current_alarm");
	if (mysql_query (conn, query) != 0) {
		sprintf (traceBuf, "mysql_query : %s\n", mysql_error (conn));
		trclib_writeLogErr (FL, traceBuf);
		return -1;
	}

	res = mysql_store_result (conn);

	i = 0;
	while ((row = mysql_fetch_row (res)) != NULL) {

		memset (query, 0, 1024);
		sprintf (query, "SELECT * FROM %s", "current_alarm");

		ret = mysql_select_query (query);

		if (ret == 1) {

			/* HW 요청 시 SW 장애 정보는 제외
			*/
			if (mtype == PRMT_INIT_HWALM_DATA &&
				atoi (row[3]) != SFM_ALM_TYPE_MP_HW) {
				puts ("skip 111");
				continue;
			}

			/* SW 요청 시 HW 장애 정보는 제외
			*/
			if (mtype == PRMT_INIT_SWALM_DATA &&
				atoi (row[3]) == SFM_ALM_TYPE_MP_HW) {
				puts ("skip 222");
				continue;
			}

			len	= 0;
			memset (almpkt->data, 0, MAX_NMS_BUF-16);

			almpkt->serno	= ++i;
			almpkt->attrcnt	= 0;

			sprintf (&almpkt->data[len], "10001:C:%s:\n", mySysName);
			len = strlen (almpkt->data);

			sprintf (&almpkt->data[len], "10002:C:%s:\n", row[2]);
			len = strlen (almpkt->data);

			sprintf (&almpkt->data[len], "10040:C:A%d:\n",
								get_alm_code (atoi (row[3])));
			len = strlen (almpkt->data);

			sprintf (&almpkt->data[len], "10042:I:%d:\n",
								get_alm_class (atoi (row[4])));
			len = strlen (almpkt->data);

			sprintf (&almpkt->data[len], "10041:C:%s:\n",
								get_supple_info (row[2], atoi (row[3]), row[6]));
			len = strlen (almpkt->data);

			sprintf (&almpkt->data[len], "10043:C:%s:\n", row[5]);
			len = strlen (almpkt->data);

			almpkt->hdr.len	= PKT3_HEAD_SIZE + len;

	//lala
	printf ("[send current alarm] sfd=%d, almtype=%d, rsc=%s\n(%s)\n",
			*sfd, atoi (row[3]), row[6], almpkt->data);

			send_packet (*sfd, (char *)tbuf, almpkt->hdr.len);

		}
	}
	mysql_free_result (res);

	return 1;

} /* End of send_current_alm () */



handle_console (GeneralQMsgType *rmsg)
{
	int			k;
	char		sbuf[4096];
	Packet2		*pkt2;
	IxpcQMsgType	*rxinfo;

	memset (sbuf, 0, 4096);

	pkt2 = (Packet2 *)sbuf;

	rxinfo = (IxpcQMsgType *)rmsg->body;

	pkt2->hdr.chd.ver		= 0;
	pkt2->hdr.chd.ctrlbit	= 0;
	pkt2->hdr.chd.pktype	= 2;	// packet2
	pkt2->hdr.chd.ctrlseq	= 0;
	pkt2->hdr.chd.src		= 0x03;	// BSD
	pkt2->hdr.chd.dest		= 0x01;	// NMS
	pkt2->hdr.prmt			= 0;
	pkt2->hdr.len			= PKT2_HEAD_SIZE +
							sizeof (IxpcQMsgHeadType) + strlen (rxinfo->body);
	pkt2->msgtype			= MT_ALARM_FAULT;

	strcpy (pkt2->data, rxinfo->body);

	for (k=0; k<MAX_NMS_CON; k++) {
		if ((sfdb->nmsInfo.port[k] == ne_info.port[PORT_IDX_CONS]) &&
			(sfdb->nmsInfo.ptype[k] == FD_TYPE_DATA)) {
			//lala
			printf ("send console [sfd=%d-%d/%d/%d/%d] ----------------\n(%s)\n",
					sfdb->nmsInfo.fd[k], pkt2->hdr.chd.pktype, pkt2->hdr.chd.src,
					pkt2->hdr.chd.dest, pkt2->hdr.len, pkt2->data);

			send_packet (sfdb->nmsInfo.fd[k], (char *)sbuf, pkt2->hdr.len);
		}
	}
	return 1;

} /* End of handle_console () */


handle_alarm (GeneralQMsgType *rmsg)
{
	int			k, len=0;
	char		sbuf[4096];
	Packet3		*pkt3;
	NmsAlmInfo	*alminfo;

	memset (sbuf, 0, 4096);

	pkt3 = (Packet3 *)sbuf;

	alminfo = (NmsAlmInfo *)rmsg->body;

	pkt3->hdr.chd.ver		= 0;
	pkt3->hdr.chd.ctrlbit	= 0;
	pkt3->hdr.chd.pktype	= 3;	// packet3
	pkt3->hdr.chd.ctrlseq	= 0;
	pkt3->hdr.chd.src		= 0x03;	// BSD
	pkt3->hdr.chd.dest		= 0x01;	// NMS
	pkt3->hdr.prmt			= 0;
	pkt3->msgtype			= MT_ALARM_FAULT;

	sprintf (&pkt3->data[len], "10001:C:%s:", mySysName);
	len = strlen (pkt3->data);

	sprintf (&pkt3->data[len], "10002:C:%s:", alminfo->sysname);
	len = strlen (pkt3->data);

	sprintf (&pkt3->data[len], "10040:C:A%d:",
							get_alm_code (alminfo->atype));
	len = strlen (pkt3->data);

	sprintf (&pkt3->data[len], "10042:I:%d:",
						get_alm_class (alminfo->aclass));
	len = strlen (pkt3->data);

	sprintf (&pkt3->data[len], "10041:C:%s:",
			get_supple_info (alminfo->sysname, alminfo->atype, alminfo->desc));
	len = strlen (pkt3->data);

	sprintf (&pkt3->data[len], "10043:C:%s:", (char *)get_time_str (alminfo->time));
	len = strlen (pkt3->data);

	printf("FIMD aType[%d]: [%s]\n", alminfo->atype, pkt3->data);

	pkt3->hdr.len	= PKT3_HEAD_SIZE + len;

	for (k=0; k<MAX_NMS_CON; k++) {
		if ((sfdb->nmsInfo.fd[k] > 0) &&
			(sfdb->nmsInfo.ptype[k] == FD_TYPE_DATA) &&
			(sfdb->nmsInfo.port[k] == ne_info.port[PORT_IDX_ALM])) {

			if (trcFlag || trcLogFlag) {
				sprintf (traceBuf, "[send alarm] sfd=%d\n(%s)\n",
						sfdb->nmsInfo.fd[k], pkt3->data);
				trclib_writeLog (FL, traceBuf);
			}

			send_packet (sfdb->nmsInfo.fd[k], sbuf, pkt3->hdr.len);
		}
	}

} /* End of handle_alarm () */


handle_mmc_request (GeneralQMsgType *rmsg)
{
	int		i, j, cnt=0, llen=0;
	int		qsize=0;
	char	*pbuf;
	IxpcQMsgType	*r_ixpc, *t_ixpc;
	GeneralQMsgType	tmsg;
	MMLReqMsgType	*imd;
	MMLResMsgType	*omd;


	r_ixpc	= (IxpcQMsgType *)rmsg->body;
	imd 	= (MMLReqMsgType *)r_ixpc->body;

	printf ("[handle_mmc_request] %s ***************\n", r_ixpc->body);

	memset (&tmsg, 0, sizeof (GeneralQMsgType));

	if (!strcasecmp (imd->head.cmdName, "dis-nms-sts")) {

		t_ixpc	= (IxpcQMsgType *)tmsg.body;
		omd		= (MMLResMsgType *)t_ixpc->body;

		tmsg.mtype = MTYPE_MMC_RESPONSE;

		strcpy (t_ixpc->head.srcSysName, mySysName);
		strcpy (t_ixpc->head.srcAppName, myAppName);
		strcpy (t_ixpc->head.dstSysName, r_ixpc->head.srcSysName);
		strcpy (t_ixpc->head.dstAppName, r_ixpc->head.srcAppName);

		omd->head.mmcdJobNo 	= imd->head.mmcdJobNo;
		omd->head.extendTime 	= 0;
		omd->head.resCode 		= 0;
		omd->head.contFlag 		= 0;
		strcpy (omd->head.cmdName, imd->head.cmdName);

		pbuf = omd->body;
		sprintf (&pbuf[llen], "\n%s\n%s\n",
				"    TYPE       PORT       FD       IP_ADDR",
				"    ==============================================");
		llen = strlen (pbuf);

		for (i=0; i<PORT_IDX_LAST; i++) {
			if (i == PORT_IDX_ALM)
				sprintf (&pbuf[llen], "    %-10s", "ALARM");
			else if (i == PORT_IDX_CONS)
				sprintf (&pbuf[llen], "    %-10s", "CONSOLE");
			else if (i == PORT_IDX_CONF)
				sprintf (&pbuf[llen], "    %-10s", "CONFIG");
			else if (i == PORT_IDX_MMC)
				sprintf (&pbuf[llen], "    %-10s", "MMC");
			else if (i == PORT_IDX_STAT)
				sprintf (&pbuf[llen], "    %-10s", "STAT");
			llen = strlen (pbuf);

			cnt = 0;
			for (j=0; j<MAX_NMS_CON; j++) {
				if ((ne_info.port[i] == sfdb->nmsInfo.port[j]) &&
					(sfdb->nmsInfo.fd[j] > 0) && (sfdb->nmsInfo.ptype[j] == FD_TYPE_DATA)) {

					if (cnt == 0)
						sprintf (&pbuf[llen], " %-10d %-8d %s\n",
							sfdb->nmsInfo.port[j], sfdb->nmsInfo.fd[j],
							//(char *)int2dot (sfdb->nmsInfo.ipaddr));
							(char *)int2dot (sfdb->nmsInfo.ipaddr[j]));	// 070604, poopee
					else
						sprintf (&pbuf[llen], "    %-10s %-10d %-8d %s\n",
							"", sfdb->nmsInfo.port[j], sfdb->nmsInfo.fd[j],
							//(char *)int2dot (sfdb->nmsInfo.ipaddr));
							(char *)int2dot (sfdb->nmsInfo.ipaddr[j]));	// 070604, poopee

					llen = strlen (pbuf);
					cnt++;
				}
			}
			if (cnt == 0) {
				sprintf (&pbuf[llen], " %-10s %-8s %s\n", "-", "-", "-");
				llen = strlen (pbuf);
				//lala
				printf ("llen=%d\n", llen);
			}

			if (llen > 3000) {
				sprintf (&pbuf[llen], "%s\n\n",
					"    ==============================================");
				llen = strlen (pbuf);
				t_ixpc->head.segFlag 	= 1;
				t_ixpc->head.seqNo 		+= 1;
				t_ixpc->head.bodyLen	= llen + sizeof (MMLResMsgHeadType);

				qsize = llen + sizeof (MMLResMsgHeadType) + sizeof (IxpcQMsgHeadType) + 4;

				if (msgsnd (ixpcQid, &tmsg, qsize, IPC_NOWAIT) < 0) {
					printf ("[cont] msgsnd fail (->ixpc) : %s\n", strerror (errno));
					printf ("len=%d\n(%s)\n", qsize, pbuf);
					return -1;
				}
				llen = 0;
			}
		}
		sprintf (&pbuf[llen], "%s\n\n",
			"    ==============================================");
		llen = strlen (pbuf);
		t_ixpc->head.segFlag 	= 0;
		t_ixpc->head.seqNo 		= 0;
		t_ixpc->head.bodyLen	= llen + sizeof (MMLResMsgHeadType);

		qsize = llen + sizeof (MMLResMsgHeadType) + sizeof (IxpcQMsgHeadType) + 4;

		if (msgsnd (ixpcQid, &tmsg, qsize, IPC_NOWAIT) < 0) {
			printf ("[end] msgsnd fail (->ixpc) : %s\n", strerror (errno));
			printf ("len=%d\n(%s)\n", qsize, pbuf);
			return -1;
		}
		return 1;
	}
	return 1;

} /* End of handle_mmc_request () */


send_heartbit (int *h_sfd)
{
	char	hbbuf[200];

	memset (hbbuf, 0, 200);

	hbbuf[0] = 'A';

	// printf ("[send hearbit] fd=%d\n(%c)\n", *h_sfd, hbbuf[0]);
	send_packet (*h_sfd, hbbuf, 1);

	return 1;

} /* End of send_heartbit () */
