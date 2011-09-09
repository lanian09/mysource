#include "udrdb.h"

extern		MYSQL		*conn;
extern		int			errno;
static		int			uni_key;

/**
 *		UDR Insert Procedure
 **/ 
int parse_insert_proc(char *parse_file, int file_format)
{

	char 			temp_file[256];
	char			query[4096];
	char			authen_hexa[33];
	char			year_data[4], time_data[10];
	char			aaa_ip[16], framed_ip[16], nas_ip[16], pcf_ip[16], ha_ip[16], dest_ip[16];
	int				insert_cnt, i;
	int				udr_count, print_flag;
	int				year_int, time_int;
	long long		temp_long, temp_long2;
	char			*date_str;
	char			acct_sess_id[16], correl_id[16];
	char			time_stamp[32], event_time[32], req_time[32], res_time[32];
	time_t			time_stamp_t, event_time_t, req_time_t, res_time_t;
	FILE			*fd;
	st_AAAREQ		aaa_req;
	st_DumpInfo		udr_head;
	st_ACCInfo		*acct_info;
	st_UDRInfo		*udr_info;
	

	/** initialize variables **/
	insert_cnt = print_flag = 0;
	memset(&aaa_req, 	0x00, sizeof(st_AAAREQ));
	memset(&udr_head, 	0x00, sizeof(st_DumpInfo));
	memset(temp_file, 	0x00, sizeof(temp_file));
	memset(authen_hexa, 0x00, sizeof(authen_hexa));
	memset(year_data, 	0x00, sizeof(year_data));
	memset(time_data, 	0x00, sizeof(time_data));
	memset(aaa_ip,		0x00, sizeof(aaa_ip));
	memset(framed_ip,	0x00, sizeof(framed_ip));
	memset(nas_ip,		0x00, sizeof(nas_ip));
	memset(pcf_ip,		0x00, sizeof(pcf_ip));
	memset(ha_ip,		0x00, sizeof(ha_ip));
	memset(dest_ip,		0x00, sizeof(dest_ip));

	if (file_format == COMPRESS_FILE)
		memcpy(temp_file, parse_file, (strlen(parse_file)-3));
	else
		memcpy(temp_file, parse_file, strlen(parse_file));

	date_str = strstr(temp_file, "2");
	date_str = strstr(date_str, "2");

	/** get year info **/
	for(i=0; i<4; i++)
	{
		year_data[i] = *(date_str+i);
	}

	/** get date string **/
	for(i=0; i<10; i++)
	{
		time_data[i] = *(date_str+23+i);
	}

	year_int 	= atoi(year_data);
	time_int 	= atoi(time_data);


	fd = fopen(temp_file, "r");
	if (!fd)
	{
		printf("FILE OPEN ERROR : %s [FILE : %s]\n", 
								strerror(errno), temp_file);
		return (-1);	

	}
	printf("FILE OPEN SUCCESS : %s\n", temp_file); 

	/** read file head info **/
	if ((fread(&udr_head, sizeof(st_DumpInfo), 1, fd)) <= 0)
	{
		fprintf(stderr, "FILE READ ERROR : %s [FILE : %s]\n",
						strerror(errno), temp_file);	
	}

	/** print head info **/
	//fprintf(stdout, "UDR COUNT : %d\n", CVT_INT_CP(udr_head.udr_cnt));

	/** read UDR data **/
	while(1)
	{

		if ((fread(&aaa_req, sizeof(st_AAAREQ), 1, fd)) <= 0)
		{
			//fprintf(stderr, "END READ FILE : %s\n", strerror(errno));
			break;
		}

		acct_info = (st_ACCInfo *)&(aaa_req.stInfo);
		memset(query, 0x00, sizeof(query));
		/** debug
		printf("ucCode 	: %d\n", acct_info->ucCode);
		printf("ucID 	: %d\n", acct_info->ucID);
		printf("MIN 	: %s\n", acct_info->szMIN);
		**/
		CvtBinToHexa(16, acct_info->szAuthen, authen_hexa);

		if (!strcmp(acct_info->szMDN, "\0"))
			strcpy(acct_info->szMDN, " ");
		if (!strcmp(acct_info->szESN, "\0"))
			strcpy(acct_info->szESN, " ");
		if (!strcmp(acct_info->szBSID, "\0"))
			strcpy(acct_info->szBSID, " ");

		print_flag++;

		/** print sample **/
		/*
		if (print_flag == 1)
		{
			printf("############ print parsing data ###############\n\n");

			printf("ucCode 	: %d\n", acct_info->ucCode);
			printf("ucId	: %d\n", acct_info->ucID);
			printf("userLen	: %d\n", acct_info->ucUserLen);
			printf("nasportid_len : %d\n", acct_info->ucNASPortIDLen);
			printf("udr sequence : %d\n", CVT_INT_CP(acct_info->uiUDRSeq));
			printf("time stamp : %u\n", acct_info->uiTimeStamp);
			printf("uiAAAIP : %u\n", acct_info->uiAAAIP);
			printf("AAA-IP : %s\n", cvt_ipaddr(CVT_INT_CP(acct_info->uiAAAIP)));
			printf("uiKey : %u\n", acct_info->uiKey);
			printf("framedIP : %u\n", acct_info->uiFramedIP);
			printf("uiNASIP  : %u\n", acct_info->uiNASIP);
			printf("uiPCFIP  : %u\n", acct_info->uiPCFIP);
			printf("uiHAIP   : %u\n", acct_info->uiHAIP);
			printf("HA-IP   : %s\n", cvt_ipaddr(CVT_INT_CP(acct_info->uiHAIP)));
			printf("radiusLen : %u\n", acct_info->uiRADIUSLen);
			printf("sesscontinue : %u\n", acct_info->uiSessContinue);
			printf("beginnigSess : %u\n", CVT_INT_CP(acct_info->uiBeginnigSess));
			printf("svcopt : %d\n", CVT_INT_CP(acct_info->dSvcOpt));
			printf("acctstatType : %d\n", acct_info->dAcctStatType);
			printf("comptunneled : %d\n", acct_info->dCompTunnelInd);
			printf("numact : %d\n", acct_info->dNumAct);
			printf("svc type : %d\n", acct_info->dSvcType);
			printf("fwd fch mux : %d\n", acct_info->dFwdFCHMux);
			printf("rev fch mux : %d\n", acct_info->dRevFCHMux);
			printf("fwd traf type : %d\n", acct_info->dFwdTrafType);
			printf("rev traf type : %d\n", acct_info->dRevTrafType);
			printf("fch size : %d\n", acct_info->dFCHSize);
			printf("fwd fchrcv : %d\n", acct_info->dFwdFCHRC);
			printf("rev fchrc : %d\n", acct_info->dRevFCHRC);
			printf("ip tech : %d\n", acct_info->dIPTech);
			printf("dcch size : %d\n", acct_info->dDCCHSize);
			printf("nas port : %d\n", acct_info->dNASPort);
			printf("nssport type : %d\n", acct_info->dNASPortType);
			printf("release ind : %d\n", acct_info->dReleaseInd);
			printf("acct in oct : %d\n", acct_info->dAcctInOct);
			printf("acct out oct : %d\n", acct_info->dAcctOutOct);
			printf("acct in pkt : %d\n", acct_info->dAcctInPkt);
			printf("acct out pkt : %d\n", acct_info->dAcctOutPkt);
			printf("Event Time : %u\n", acct_info->uiEventTime);
			printf("act time : %u\n", acct_info->uiActTime);
			printf("acct sess time : %u\n", acct_info->uiAcctSessTime);
			printf("acct delay time : %u\n", acct_info->uiAcctDelayTime);
			printf("term sdboct cnt : %d\n", acct_info->dTermSDBOctCnt);
			printf("org sdb oct cnt : %d\n", acct_info->dOrgSDBOctCnt);
			printf("term num sdb : %d\n", acct_info->dTermNumSDB);
			printf("org num sdb : %d\n", acct_info->dOrgNumSDB);
			printf("rcv hdlc oct : %d\n", acct_info->dRcvHDLCOct);
			printf("ip qos : %d\n", acct_info->dIPQoS);
			printf("air qos : %d\n", acct_info->dAirQoS);
			printf("rp connect id : %d\n", acct_info->dRPConnectID);
			printf("bad ppp framed cnt : %d\n", acct_info->dBadPPPFrameCnt);
			printf("acct auth : %d\n", acct_info->dAcctAuth);
			printf("acct term cause : %d\n", acct_info->dAcctTermCause);
			printf("always on : %d\n", acct_info->dAlwaysOn);
			printf("user id : %d\n", acct_info->dUserID);
			printf("in mip sig cnt : %d\n", acct_info->dInMIPSigCnt);
			printf("out mipsig cnt : %d\n", acct_info->dOutMIPSigCnt);
			printf("acct interim : %d\n", CVT_INT_CP(acct_info->dAcctInterim));
			printf("acct session id : %d\n", acct_info->llAcctSessID);
			printf("correl id : %d\n", acct_info->llCorrelID);
			printf("retry flag : %u\n", acct_info->uiRetryFlg);
			printf("################################################\n\n");
		}
		*/

		/**  ip address format data convert **/
		sprintf(aaa_ip, "%s", cvt_ipaddr(CVT_INT_CP(acct_info->uiAAAIP)));
		sprintf(framed_ip, "%s", cvt_ipaddr(CVT_INT_CP(acct_info->uiFramedIP)));
		sprintf(nas_ip, "%s", cvt_ipaddr(CVT_INT_CP(acct_info->uiNASIP)));
		sprintf(pcf_ip, "%s", cvt_ipaddr(CVT_INT_CP(acct_info->uiPCFIP)));
		sprintf(ha_ip, "%s", cvt_ipaddr(CVT_INT_CP(acct_info->uiHAIP)));

		memset(acct_sess_id, 	0x00, sizeof(acct_sess_id));
		memset(correl_id, 		0x00, sizeof(correl_id));

		CVT_INT64_CP(&temp_long, acct_info->llAcctSessID); 
		conv_id(temp_long, acct_sess_id);

		CVT_INT64_CP(&temp_long, acct_info->llCorrelID);
		conv_id(temp_long, correl_id);

		/** time convert time_t from string **/
		memset(time_stamp, 	0x00, sizeof(time_stamp));
		memset(event_time,	0x00, sizeof(event_time));

		time_stamp_t = CVT_INT_CP(acct_info->uiTimeStamp);
		event_time_t = CVT_INT_CP(acct_info->uiEventTime);

		conv_time(time_stamp_t, time_stamp);
		conv_time(event_time_t, event_time);


		/** insert acct info to mysql **/
		sprintf(query, "INSERT INTO %s VALUES ("
	   				"'%d', '%d', '%d', '%d', '%d', '%s', '%s', '%d', '%s', '%s',"
	    			"'%s', '%s', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d',"
	    			"'%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d',"
	    			"'%d', '%d', '%d', '%d', '%d', '%d', '%s', '%d', '%d', '%d',"
	    			"'%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d',"
	    			"'%d', '%d', '%d', '%d', '%d', '%d', '%s', '%s', '%d', '%s',"
	    			"'%s', '%s', '%s', '%s', '%s', '%s', '%d', '%d', '%d')",
					ACCT_TABLE_NAME,
					acct_info->ucCode, acct_info->ucID, acct_info->ucUserLen,
					acct_info->ucNASPortIDLen, CVT_INT_CP(acct_info->uiUDRSeq),
					time_stamp, aaa_ip,
					CVT_INT_CP(acct_info->uiKey), framed_ip,
					nas_ip, pcf_ip,
					ha_ip, CVT_INT_CP(acct_info->uiRADIUSLen),
					CVT_INT_CP(acct_info->uiSessContinue), CVT_INT_CP(acct_info->uiBeginnigSess),
					CVT_INT_CP(acct_info->dSvcOpt), CVT_INT_CP(acct_info->dAcctStatType),
					CVT_INT_CP(acct_info->dCompTunnelInd), CVT_INT_CP(acct_info->dNumAct),
					CVT_INT_CP(acct_info->dSvcType), CVT_INT_CP(acct_info->dFwdFCHMux),
					CVT_INT_CP(acct_info->dRevFCHMux), CVT_INT_CP(acct_info->dFwdTrafType),
					CVT_INT_CP(acct_info->dRevTrafType), CVT_INT_CP(acct_info->dFCHSize),
					CVT_INT_CP(acct_info->dFwdFCHRC), CVT_INT_CP(acct_info->dRevFCHRC),
					CVT_INT_CP(acct_info->dIPTech), CVT_INT_CP(acct_info->dDCCHSize),
					CVT_INT_CP(acct_info->dNASPort), CVT_INT_CP(acct_info->dNASPortType),
					CVT_INT_CP(acct_info->dReleaseInd), CVT_INT_CP(acct_info->dAcctInOct),
					CVT_INT_CP(acct_info->dAcctOutOct), CVT_INT_CP(acct_info->dAcctInPkt),
					CVT_INT_CP(acct_info->dAcctOutPkt), event_time,
					CVT_INT_CP(acct_info->uiActTime), CVT_INT_CP(acct_info->uiAcctSessTime),
					CVT_INT_CP(acct_info->uiAcctDelayTime), CVT_INT_CP(acct_info->dTermSDBOctCnt),
					CVT_INT_CP(acct_info->dOrgSDBOctCnt), CVT_INT_CP(acct_info->dTermNumSDB),
					CVT_INT_CP(acct_info->dOrgNumSDB), CVT_INT_CP(acct_info->dRcvHDLCOct),
					CVT_INT_CP(acct_info->dIPQoS), CVT_INT_CP(acct_info->dAirQoS),
					CVT_INT_CP(acct_info->dRPConnectID), CVT_INT_CP(acct_info->dBadPPPFrameCnt),
					CVT_INT_CP(acct_info->dAcctAuth), CVT_INT_CP(acct_info->dAcctTermCause),
					CVT_INT_CP(acct_info->dAlwaysOn), CVT_INT_CP(acct_info->dUserID),
					CVT_INT_CP(acct_info->dInMIPSigCnt), CVT_INT_CP(acct_info->dOutMIPSigCnt),
					CVT_INT_CP(acct_info->dAcctInterim), acct_sess_id,
					correl_id, CVT_INT_CP(acct_info->uiRetryFlg), authen_hexa,
					acct_info->szMDN, acct_info->szESN, acct_info->szUserName,
					acct_info->szBSID, acct_info->szNASPortID, acct_info->szMIN,
					year_int, time_int, uni_key);


		if (mysql_query(conn, query) != 0) 
		{
			fprintf(stderr, "[ERR] MYSQL INSERT ERROR : %s\n", mysql_error(conn));
			return (-1);
		}

		udr_count = CVT_INT_CP(aaa_req.dUDRCount);	

		//printf("######## UDR Count : %d\n", udr_count);

		/** insert udr info **/
		for(i=0; i<udr_count; i++)
		{
			long long 		udr_temp_long;
			char			acct_session_id[32];

			udr_info = (st_UDRInfo *)&(aaa_req.stUDRInfo[i]);

			//printf("#### acct session id : %lld\n", udr_info->llAcctSessID);
			memset(query, 0x00, sizeof(query));

			memset(acct_session_id, 0x00, sizeof(acct_session_id));
			CVT_INT64_CP(&udr_temp_long, udr_info->llAcctSessID);
			conv_id(udr_temp_long, acct_session_id);

			sprintf(dest_ip, "%s", cvt_ipaddr(CVT_INT_CP(udr_info->uiDestIP)));

			if (!(udr_info->ucAppIDF))
			{
				if (!strcmp(udr_info->szAppID, "\0"))
					strcpy(udr_info->szAppID, " ");
			}
			if (!(udr_info->ucContentCodeF))
			{
				if (!strcmp(udr_info->szAppID, "\0"))
					strcpy(udr_info->szContentCode, " ");
			}
			if (!(udr_info->ucURLF))
			{
				if (!strcmp(udr_info->szURL, "\0"))
					strcpy(udr_info->szURL, " ");
			}
			if (!(udr_info->ucUserAgentF))
			{
				if (!strcmp(udr_info->szUserAgent, "\0"))
					strcpy(udr_info->szUserAgent, " ");
			}
			if (!(udr_info->ucHostF))
			{
				if (!strcmp(udr_info->szHost, "\0"))
					strcpy(udr_info->szHost, " ");
			}
			if (!(udr_info->ucMDNF))
			{
				if (!strcmp(udr_info->szMDN, "\0"))
					strcpy(udr_info->szMDN, " ");
			}
			if (!(udr_info->ucPhoneNumF))
			{
				if (!strcmp(udr_info->szPhoneNum, "\0"))
					strcpy(udr_info->szPhoneNum, " ");
			}
			if (!(udr_info->ucModelF))
			{
				if (!strcmp(udr_info->szModel, "\0"))
					strcpy(udr_info->szModel, " ");
			}
			if (!(udr_info->ucPacketCntF))
			{
				if (!strcmp(udr_info->szPacketCnt, "\0"))
					strcpy(udr_info->szPacketCnt, " ");
			}

			memset(udr_info->szMDN,  0x00, sizeof(udr_info->szMDN));
			memset(udr_info->szHost, 0x00, sizeof(udr_info->szHost));

			strcpy(udr_info->szMDN, " ");
			strcpy(udr_info->szHost, " ");

			memset(req_time, 	0x00, sizeof(req_time));
			memset(res_time, 	0x00, sizeof(res_time));

			req_time_t = CVT_INT_CP(udr_info->tReqTime);
			res_time_t = CVT_INT_CP(udr_info->tResTime);

			conv_time(req_time_t, req_time);
			conv_time(res_time_t, res_time);

			sprintf(query,  "INSERT INTO %s VALUES ("
						"'%s', '%s', '%d', '%d', '%d', '%s', '%s', '%d', '%s', '%d',"
						"'%d', '%d', '%s', '%s', '%d', '%d', '%d', '%d', '%d', '%d',"
						"'%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d',"
						"'%d', '%d', '%d', '%d', '%s', '%s', '%s', '%s', '%s', '%s',"
						"'%s', '%d', '%d', '%d', '%d')",
						UDR_TABLE_NAME,
						acct_session_id, acct_info->szMIN, i,
						CVT_INT_CP(udr_info->dDataSvcType), CVT_INT_CP(udr_info->uiTranID),
						req_time, res_time,
						CVT_INT_CP(udr_info->tSessionTime), dest_ip,
						CVT_INT_CP(udr_info->dDestPort), CVT_INT_CP(udr_info->dSrcPort),
						CVT_INT_CP(udr_info->dCType), udr_info->szAppID,
						udr_info->szContentCode, CVT_INT_CP(udr_info->dMethodType),
						CVT_INT_CP(udr_info->dResultCode), CVT_INT_CP(udr_info->dIPUpSize),
						CVT_INT_CP(udr_info->dIPDownSize), CVT_INT_CP(udr_info->dRetransInSize),
						CVT_INT_CP(udr_info->dRetransOutSize), CVT_INT_CP(udr_info->dCPCode),
						CVT_INT_CP(udr_info->dUseCount), CVT_INT_CP(udr_info->dUseTime),
						CVT_INT_CP(udr_info->dTotalSize), CVT_INT_CP(udr_info->dTotalTime),
						CVT_INT_CP(udr_info->dBillcomCount), CVT_INT_CP(udr_info->dGWCount),
						CVT_INT_CP(udr_info->dContentLen), CVT_INT_CP(udr_info->dTranComplete),
						CVT_INT_CP(udr_info->dTranTermReason), CVT_INT_CP(udr_info->dAudioInputIPSize),
						CVT_INT_CP(udr_info->dAudioOutputIPSize), CVT_INT_CP(udr_info->dVideoInputIPSize),
						CVT_INT_CP(udr_info->dVideoOutputIPSize), udr_info->szURL,
						udr_info->szUserAgent, udr_info->szHost,
						udr_info->szMDN, udr_info->szPhoneNum,
						udr_info->szModel, udr_info->szPacketCnt, udr_info->ucURLCha,
						year_int, time_int, uni_key);

			if (mysql_query(conn, query) != 0)
			{
				fprintf(stderr, "[ERR] MYSQL INSERT ERROR : %s\n", mysql_error(conn));
#ifdef _DEBUG_PRINT
				fprintf(stdout, "acct sess id : %s [%d]\n",
									acct_sess_id, strlen(acct_sess_id));
				fprintf(stdout, "MIN          : %s [%d]\n",
									acct_info->szMIN, strlen(acct_info->szMIN));
				fprintf(stdout, "dest_ip      : %s [%d]\n",
									dest_ip, strlen(dest_ip));
				fprintf(stdout, "Application Id : %s [%d]\n",
									udr_info->szAppID, strlen(udr_info->szAppID));
				fprintf(stdout, "Content Code : %s [%d]\n",
									udr_info->szContentCode, strlen(udr_info->szContentCode));
				fprintf(stdout, "URL          : %s [%d]\n",
									udr_info->szURL, strlen(udr_info->szURL));
				fprintf(stdout, "User Agent   : %s [%d]\n",
									udr_info->szUserAgent, strlen(udr_info->szUserAgent));
				fprintf(stdout, "Host         : %s [%d]\n",
									udr_info->szHost, strlen(udr_info->szHost));
				fprintf(stdout, "MDN          : %s [%d]\n",
									udr_info->szMDN, strlen(udr_info->szMDN));
				fprintf(stdout, "Phone Number : %s [%d]\n",
									udr_info->szPhoneNum, strlen(udr_info->szPhoneNum));
				fprintf(stdout, "Model        : %s [%d]\n",
									udr_info->szModel, strlen(udr_info->szModel));
				fprintf(stdout, "Packet Count : %s [%d]\n",
									udr_info->szPacketCnt, strlen(udr_info->szPacketCnt));
#endif
			}
			insert_cnt++;

		
		}  /** end of for() **/

		uni_key++;

	} 	/** end of while **/

	if (fclose(fd) != 0)
		fprintf(stderr, "CLOSE FILE ERROR : %s [%s]\n", strerror(errno), temp_file);;

	printf("=======> INSERT UDR COUNT : %d\n", insert_cnt);
									
	return (1);

}


/**
 *		UDR Arrange by Correlation Id
 **/ 
int udr_arrange(int search_year, int start_time, int end_time, char *write_file)
{

	int				check_udr_cnt, udr_count, acct_cnt;
	int				search_key;
	char			query[4096];
	char			data_buf[8192], data_buf2[8192];
	char			search_acct_sess_id[32], search_min[32];
	time_t			offset_time, cur_time;
	FILE			*fd;
	MYSQL_RES		*result1, *result2;
	MYSQL_ROW		row, row1;


	udr_count = acct_cnt = 0;
	memset(query, 0x00, sizeof(query));

	/** file open **/
	fd = fopen(write_file, "w");
	if (!fd)
	{
		fprintf(stderr, "FILE OPEN ERROR : %s\n", strerror(errno));
		return (-1);
	}

	offset_time = time(NULL);

	printf("\n========> SEARCHING ...... WAIT \n\n");
	start_wait();

	sprintf(query, "SELECT * from %s WHERE (crt_year = %d AND crt_time BETWEEN %d AND %d) ORDER BY correl_id",
						ACCT_TABLE_NAME, search_year, start_time, end_time);
	if (mysql_query(conn, query) != 0)
	{
		fprintf(stderr, "Search ACCT INFO TABLE FAILURE : %s\n", mysql_error(conn));
		stop_wait();
		return (-1);
	}


	/** write title to file **/
	write_title(fd);

	result1 = mysql_store_result(conn);
	
	while ((row = mysql_fetch_row(result1)) != NULL)
	{

		memset(data_buf, 0x00, sizeof(data_buf));
		
	
		sprintf(data_buf,  "%s,%s,%s,%s,%s,%s,%s,0x%s,0x%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,", 
						row[5], row[4], row[65], row[60], row[61],
						row[8], row[62], row[56], row[57], row[13],
						row[14], row[11], row[9], row[10], row[63],
						row[52], row[20], row[21], row[15], row[22],
						row[23], row[24], row[25], row[26], row[27],
						row[17], row[31], row[38], row[51], row[33],
						row[32], row[48], row[36], row[37], row[18],
						row[40], row[41], row[42], row[43], row[44],
						row[53], row[54], row[45], row[46], row[34],
						row[35], row[47], row[59], row[38], row[49],
						row[16], row[30], row[29], row[64], row[19],
						row[39]);
						

		fputs(data_buf, fd);
		acct_cnt++;
						
		memset(search_acct_sess_id, 0x00, sizeof(search_acct_sess_id));
		memset(search_min, 0x00, sizeof(search_min));

		strcpy(search_acct_sess_id, row[56]); 
		strcpy(search_min, row[65]);
		search_key = atoi(row[68]);
		
		memset(query, 0x00, sizeof(query));
		sprintf(query, "SELECT * from %s WHERE (acct_sess_id = '%s' AND min = '%s' AND matching_key = %d)",
						UDR_TABLE_NAME, search_acct_sess_id, search_min, search_key);

		if (mysql_real_query(conn, query, strlen(query)) != 0)
		{
			fprintf(stderr, "Search UDR INFO TABLE FAILURE : %s\n", mysql_error(conn));
			stop_wait();
			return (-1);
		}
		
		result2 = mysql_store_result(conn);

		check_udr_cnt = 0;
		while ((row1 = mysql_fetch_row(result2)) != NULL)
		{

			if (check_udr_cnt != 0)
				fputs(data_buf, fd);

			memset(data_buf2, 0x00, sizeof(data_buf));

			/**
			 *			UDR INFO
			 *
			 *		row1[3]		-	data_svc_type
			 *		row1[4]		-	tran_id
			 *		row1[5]		-	req_time
			 *		row1[6]		-	res_time
			 *		row1[7]		-	session_time
			 *		row1[8]		-	dest_ip
			 *		row1[9]		-	dest_port
			 *		row1[10]	-	src_port
			 *		row1[11]	-	c_type
			 *		row1[12]	-	app_id
			 *		row1[13]	-	content_code
			 *		row1[14]	-	method_type
			 *		row1[15]	-	result_code
			 *		row1[16]	-	ip_up_size
			 *		row1[17]	-	ip_down_size
			 *		row1[18]	-	retrans_in_size
			 *		row1[19]	-	retrans_out_size
			 *		row1[20]	-	cp_code
			 *		row1[21]	-	use_count
			 *		row1[22]	-	use_time
			 *		row1[23]	-	total_size
			 *		row1[24]	-	total_time
			 *		row1[25]	-	bill_com_count
			 *		row1[26]	-	gw_count
			 *		row1[27]	-	content_len
			 *		row1[28]	-	tran_complete
			 *		row1[29]	-	tran_term_reason
			 *		row1[30]	-	audio_upload_size
			 *		row1[31]	-	audio_download_size
			 *		row1[32]	-	video_upload_size
			 *		row1[33]	-	video_download_size
			 *		row1[34]	-	url
			 *		row1[35]	-	user_agent
			 *		row1[36]	-	host
			 *		row1[37]	-	mdn
			 *		row1[38]	-	phone_num
			 *		row1[39]	-	model
			 *		row1[40]	-	packet_count
			 **/
			sprintf(data_buf2, "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
								row1[3], row1[4], row1[5], row1[6], row1[7],
								row1[8], row1[9], row1[10], row1[34], row1[11],
								row1[12], row1[13], row1[14], row1[15], row1[16],
								row1[17], row1[18], row1[19], row1[20], row1[38],
								row1[21], row1[22], row1[23], row1[24], row1[25],
								row1[26], row1[39], row1[30], row1[31], row1[32],
								row1[33], row1[27], row1[28], row1[29], row1[35], row1[40]);
			
			fputs(data_buf2, fd);
			check_udr_cnt++;
			udr_count++;
		}
		mysql_free_result(result2);
		if (check_udr_cnt == 0)
		{
			memset(data_buf2, 0x00, sizeof(data_buf2));
			sprintf(data_buf2, "\n");
			fputs(data_buf2, fd);
		}
						
	}

	mysql_free_result(result1);
	//mysql_free_result(result2);

	stop_wait();

	cur_time = time(NULL);
	printf("\n\n**************************************************\n");
	printf("=============> ACCT Count 	: %d\n", acct_cnt);
	printf("=============> UDR Count 	: %d\n", udr_count);
	printf("=============> Elapsed Time	: %d Seconds\n", cur_time - offset_time);
	printf("**************************************************\n");
	fclose(fd);
	return (1);

}


/**
 *		UDR Arrange by Correlation Id
 **/ 
int udr_arrange_sum(int search_year, int start_time, int end_time, char *write_file)
{

	int				check_udr_cnt, udr_count, acct_cnt;
	int				search_key;
	int				ip_up_sum, ip_down_sum, re_in_sum, re_out_sum, acct_stat_type;
	int				ip_up, ip_down, re_in, re_out;
	char			query[4096];
	char			data_buf[8192], data_buf2[8192];
	char			search_acct_sess_id[32], search_min[32];
	time_t			offset_time, cur_time;
	FILE			*fd;
	MYSQL_RES		*result1, *result2;
	MYSQL_ROW		row, row1;


	udr_count = acct_cnt = 0;
	ip_up_sum = ip_down_sum = re_in_sum = re_out_sum = 0;
	ip_up = ip_down = re_in = re_out = 0;
	memset(query, 0x00, sizeof(query));

	offset_time = time(NULL);

	/** file open **/
	fd = fopen(write_file, "w");
	if (!fd)
	{
		fprintf(stderr, "FILE OPEN ERROR : %s\n", strerror(errno));
		return (-1);
	}

	printf("\n========> SEARCHING ...... WAIT\n\n");
	start_wait();

	sprintf(query, "SELECT * from %s WHERE (crt_year = %d AND crt_time BETWEEN %d AND %d) ORDER BY acct_sess_id, min, udr_sequence",
						ACCT_TABLE_NAME, search_year, start_time, end_time);
	if (mysql_query(conn, query) != 0)
	{
		fprintf(stderr, "Search ACCT INFO TABLE FAILURE : %s\n", mysql_error(conn));
		stop_wait();
		return (-1);
	}


	/** write title to file **/
	write_title(fd);

	result1 = mysql_store_result(conn);
	memset(search_acct_sess_id, 0x00, sizeof(search_acct_sess_id));
	memset(search_min, 0x00, sizeof(search_min));
	
	while ((row = mysql_fetch_row(result1)) != NULL)
	{

		memset(data_buf, 0x00, sizeof(data_buf));
		
		sprintf(data_buf,  "%s,%s,%s,%s,%s,%s,%s,0x%s,0x%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,", 
						row[5], row[4], row[65], row[60], row[61],
						row[8], row[62], row[56], row[57], row[13],
						row[14], row[11], row[9], row[10], row[63],
						row[52], row[20], row[21], row[15], row[22],
						row[23], row[24], row[25], row[26], row[27],
						row[17], row[31], row[38], row[51], row[33],
						row[32], row[48], row[36], row[37], row[18],
						row[40], row[41], row[42], row[43], row[44],
						row[53], row[54], row[45], row[46], row[34],
						row[35], row[47], row[59], row[38], row[49],
						row[16], row[30], row[29], row[64], row[19],
						row[39]);
						

		fputs(data_buf, fd);
		acct_cnt++;


		if (strcmp(search_acct_sess_id, row[56]) == 0 && strcmp(search_min, row[65]) == 0)
		{
			;
		}
		else
		{
			ip_up_sum = ip_down_sum = re_in_sum = re_out_sum = 0;	
		}


		/** UDR_INFO TABLE Search **/
		strcpy(search_acct_sess_id, row[56]); 
		strcpy(search_min, row[65]);
		search_key = atoi(row[68]);
		
		/** accouting type : start/interim/stop  **/
		acct_stat_type = atoi(row[16]);

		memset(query, 0x00, sizeof(query));
		sprintf(query, "SELECT * from %s WHERE (acct_sess_id = '%s' AND min = '%s' AND matching_key = %d)",
						UDR_TABLE_NAME, search_acct_sess_id, search_min, search_key);

		if (mysql_real_query(conn, query, strlen(query)) != 0)
		{
			fprintf(stderr, "Search UDR INFO TABLE FAILURE : %s\n", mysql_error(conn));
			stop_wait();
			return (-1);
		}
		
		result2 = mysql_store_result(conn);

		check_udr_cnt = 0;
		while ((row1 = mysql_fetch_row(result2)) != NULL)
		{

			if (check_udr_cnt != 0)
				fputs(data_buf, fd);

			memset(data_buf2, 0x00, sizeof(data_buf));

			if (acct_stat_type == 3)   /** accouting-interim **/
			{
				ip_up = atoi(row1[16]);
				ip_down = atoi(row1[17]);
				re_in = atoi(row1[18]);
				re_out = atoi(row1[19]);

				ip_up_sum += ip_up;
				ip_down_sum += ip_down;
				re_in_sum += re_in;
				re_out_sum += re_out;
			}
				
			if (acct_stat_type == 2 )   /* accounting-stop **/
			{

				/**
				sprintf(data_buf2, "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%d,%d,%d,%d,%s,%s,%s,%s,%s,%s,%s,%s\n",
								row1[3], row1[4], row1[5], row1[6], row1[7],
								row1[8], row1[9], row1[10], row1[11], row1[12],
								row1[13], row1[14], row1[15], ip_up_sum, ip_down_sum,
								re_in_sum, re_out_sum, row1[20], row1[21], row1[22],
								row1[23], row1[24], row1[25], row1[26], row1[27]);
				**/
				sprintf(data_buf2, "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%d,%d,%d,%d,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
								row1[3], row1[4], row1[5], row1[6], row1[7],
								row1[8], row1[9], row1[10], row1[34], row1[11],
								row1[12], row1[13], row1[14], row1[15], ip_up_sum,
								ip_down_sum, re_in_sum, re_out_sum, row1[20], row1[38],
								row1[21], row1[22], row1[23], row1[24], row1[25],
								row1[26], row1[39], row1[30], row1[31], row1[32],
								row1[33], row1[27], row1[28], row1[29], row1[35], row1[40]);
			}
			else   /** any other than accounting-stop **/
			{
				sprintf(data_buf2, "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
								row1[3], row1[4], row1[5], row1[6], row1[7],
								row1[8], row1[9], row1[10], row1[34], row1[11],
								row1[12], row1[13], row1[14], row1[15], row1[16],
								row1[17], row1[18], row1[19], row1[20], row1[38],
								row1[21], row1[22], row1[23], row1[24], row1[25],
								row1[26], row1[39], row1[30], row1[31], row1[32],
								row1[33], row1[27], row1[28], row1[29], row1[35], row1[40]);

			}
			
			fputs(data_buf2, fd);
			check_udr_cnt++;
			udr_count++;
		}

		mysql_free_result(result2);
		if (check_udr_cnt == 0)
		{
			if (acct_stat_type == 2)
			{
				memset(data_buf2, 0x00, sizeof(data_buf2));
				sprintf(data_buf2, " , , , , , , , , , , , , , ,%d,%d,%d,%d, , , , , , , , , , , , , , , , , , \n",
								ip_up_sum, ip_down_sum, re_in_sum, re_out_sum);
				fputs(data_buf2, fd);
			}
			else 
			{
				memset(data_buf2, 0x00, sizeof(data_buf2));
				sprintf(data_buf2, "\n");
				fputs(data_buf2, fd);
			}
		}
						
	}

	mysql_free_result(result1);
	//mysql_free_result(result2);

	stop_wait();

	cur_time = time(NULL);
	printf("\n\n**************************************************\n");
	printf("=============> ACCT Count	: %d\n", acct_cnt);
	printf("=============> UDR Count	: %d\n", udr_count);
	printf("=============> Elapsed Time	: %d Seconds\n", cur_time - offset_time);
	printf("**************************************************\n");
	fclose(fd);
	return (1);
	
}


/**
 *		arrange acct_sess_id & imsi 
 **/ 
int udr_arrange_cond_sum(int search_year,
						 int start_time,
						 int end_time,
						 char *write_file,
						 int search_opt)
{

	int				check_udr_cnt, udr_count, acct_cnt;
	int				search_key;
	int				ip_up_sum, ip_down_sum, re_in_sum, re_out_sum, acct_stat_type;
	int				ip_up, ip_down, re_in, re_out;
	int				acct_in_pkt, acct_out_pkt;
	int				data_cnt, result_cnt, abnormal_cnt;
	char			start_on, overflow_flag;
	char			query[4096];
	char			data_buf[MAX_UDR_DATA][2048], data_buf2[MAX_UDR_DATA][2048];
	char			search_acct_sess_id[32], search_min[32];
	time_t			offset_time, cur_time;
	FILE			*fd;
	MYSQL_RES		*result1, *result2;
	MYSQL_ROW		row, row1;


	udr_count = acct_cnt = data_cnt = result_cnt = abnormal_cnt = 0;
	ip_up_sum = ip_down_sum = re_in_sum = re_out_sum = 0;
	ip_up = ip_down = re_in = re_out = 0;
	acct_in_pkt = acct_out_pkt = 0;
	start_on = overflow_flag = 0;
	memset(query, 0x00, sizeof(query));


	offset_time = time(NULL);

	/** file open **/
	fd = fopen(write_file, "w");
	if (!fd)
	{
		fprintf(stderr, "FILE OPEN ERROR : %s\n", strerror(errno));
		return (-1);
	}

	printf("\n========> SEARCHING ...... WAIT\n\n");
	start_wait();
	
	sprintf(query, "SELECT * from %s WHERE (crt_year = %d AND crt_time BETWEEN %d AND %d) ORDER BY acct_sess_id, min, udr_sequence",
						ACCT_TABLE_NAME, search_year, start_time, end_time);
	if (mysql_query(conn, query) != 0)
	{
		fprintf(stderr, "Search ACCT INFO TABLE FAILURE : %s\n", mysql_error(conn));
		stop_wait();
		return (-1);
	}


	/** write title to file **/
	write_title(fd);

	result1 = mysql_store_result(conn);
	memset(search_acct_sess_id, 0x00, sizeof(search_acct_sess_id));
	memset(search_min, 0x00, sizeof(search_min));
	memset(data_buf,  0x00, sizeof(data_buf));
	memset(data_buf2, 0x00, sizeof(data_buf2));
	
	while ((row = mysql_fetch_row(result1)) != NULL)
	{
		
		sprintf(data_buf[data_cnt],  "%s,%s,%s,%s,%s,%s,%s,0x%s,0x%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,",
						row[5], row[4], row[65], row[60], row[61],
						row[8], row[62], row[56], row[57], row[13],
						row[14], row[11], row[9], row[10], row[63],
						row[52], row[20], row[21], row[15], row[22],
						row[23], row[24], row[25], row[26], row[27],
						row[17], row[31], row[38], row[51], row[33],
						row[32], row[48], row[36], row[37], row[18],
						row[40], row[41], row[42], row[43], row[44],
						row[53], row[54], row[45], row[46], row[34],
						row[35], row[47], row[59], row[38], row[49],
						row[16], row[30], row[29], row[64], row[19],
						row[39]);
						

		//fputs(data_buf, fd);
		acct_cnt++;

		if (strcmp(search_acct_sess_id, row[56]) == 0 && strcmp(search_min, row[65]) == 0)
		{
			;
		}
		else
		{
			ip_up_sum = ip_down_sum = re_in_sum = re_out_sum = 0;	
		}

		/** UDR_INFO TABLE Search **/
		strcpy(search_acct_sess_id, row[56]); 
		strcpy(search_min, row[65]);
		search_key = atoi(row[68]);
		

		/** accounting type : start/interim/stop  **/
		acct_stat_type = atoi(row[16]);
		if (acct_stat_type == 1)
		{
			start_on = 1;
			abnormal_cnt += data_cnt;
			data_cnt = 0;
			overflow_flag = 0;
		}

		else if (acct_stat_type == 2)
		{
			acct_in_pkt = atoi(row[32]);
			acct_out_pkt = atoi(row[33]);
			//start_on = 0;
		}

		memset(query, 0x00, sizeof(query));
		sprintf(query, "SELECT * from %s WHERE (acct_sess_id = '%s' AND min = '%s' AND matching_key = %d)",
						UDR_TABLE_NAME, search_acct_sess_id, search_min, search_key);

		if (mysql_real_query(conn, query, strlen(query)) != 0)
		{
			fprintf(stderr, "Search UDR INFO TABLE FAILURE : %s\n", mysql_error(conn));
			stop_wait();
			return (-1);
		}
		

		result2 = mysql_store_result(conn);

		check_udr_cnt = 0;
		while ((row1 = mysql_fetch_row(result2)) != NULL)
		{

			//if (check_udr_cnt != 0)
				//fputs(data_buf[data_cnt], fd);

			//memset(data_buf2, 0x00, sizeof(data_buf));

			if (acct_stat_type == 3)   /** accouting-interim **/
			{
				ip_up = atoi(row1[16]);
				ip_down = atoi(row1[17]);
				re_in = atoi(row1[18]);
				re_out = atoi(row1[19]);

				ip_up_sum += ip_up;
				ip_down_sum += ip_down;
				re_in_sum += re_in;
				re_out_sum += re_out;
			}


				
			if (acct_stat_type == 2 )   /* accounting-stop **/
			{

				sprintf(data_buf2[data_cnt++], "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%d,%d,%d,%d,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
								row1[3], row1[4], row1[5], row1[6], row1[7],
								row1[8], row1[9], row1[10], row1[34], row1[11],
								row1[12], row1[13], row1[14], row1[15], ip_up_sum,
								ip_down_sum, re_in_sum, re_out_sum, row1[20], row1[38],
								row1[21], row1[22], row1[23], row1[24], row1[25],
								row1[26], row1[39], row1[30], row1[31], row1[32],
								row1[33], row1[27], row1[28], row1[29], row1[35], row1[40]);

			}
			else   /** any other than accounting-stop **/
			{
				sprintf(data_buf2[data_cnt++], "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
								row1[3], row1[4], row1[5], row1[6], row1[7],
								row1[8], row1[9], row1[10], row1[34], row1[11],
								row1[12], row1[13], row1[14], row1[15], row1[16],
								row1[17], row1[18], row1[19], row1[20], row1[38],
								row1[21], row1[22], row1[23], row1[24], row1[25],
								row1[26], row1[39], row1[30], row1[31], row1[32],
								row1[33], row1[27], row1[28], row1[29], row1[35], row1[40]);

			}
			
			//fputs(data_buf2, fd);
			check_udr_cnt++;
			udr_count++;
		}

		mysql_free_result(result2);
		if (check_udr_cnt == 0)
		{
			if (acct_stat_type == 2)
			{
				//memset(data_buf2, 0x00, sizeof(data_buf2));
				sprintf(data_buf2[data_cnt++], " , , , , , , , , , , , , ,%d,%d,%d,%d, , , , , , , , , , , , , , , , , , \n",
								ip_up_sum, ip_down_sum, re_in_sum, re_out_sum);
				//fputs(data_buf2, fd);
			}
			else 
			{
				//memset(data_buf2, 0x00, sizeof(data_buf2));
				sprintf(data_buf2[data_cnt++], "\n");
				//fputs(data_buf2, fd);
			}
		}

		if (acct_stat_type == 2)
		{
			int 		i;

			switch (search_opt)
			{

				case 2 :
					if (((acct_in_pkt > ip_up_sum) || (acct_out_pkt > ip_down_sum)) 
											&& (start_on == 1) && (overflow_flag != 1))
					{
						result_cnt += data_cnt;
						for(i=0; i<data_cnt; i++)
						{
							fputs(data_buf[i], fd);
							fputs(data_buf2[i], fd);
						}
					}

					break;

				case 3 :
					if (((acct_in_pkt < ip_up_sum) || (acct_out_pkt < ip_down_sum)) 
											&& (start_on == 1) && (overflow_flag != 1))
					{
						result_cnt += data_cnt;
						for(i=0; i<data_cnt; i++)
						{
							fputs(data_buf[i], fd);
							fputs(data_buf2[i], fd);
						}
					}

					break;

				default :
					break;

				}	/* end of switch */

				memset(data_buf,  0x00, sizeof(data_buf));
				memset(data_buf2, 0x00, sizeof(data_buf2));

				if (start_on == 0 || overflow_flag == 1)
					abnormal_cnt += data_cnt;
				data_cnt = 0;
				start_on = 0;

			}		/* end of if (acct_stat_type == 2) */

		//data_cnt++;
		if (data_cnt == MAX_UDR_DATA)
		{
			//fprintf(stderr, "UDR data exceed maximum count : %d\n", MAX_UDR_DATA);

			memset(data_buf,  0x00, sizeof(data_buf));
			memset(data_buf2, 0x00, sizeof(data_buf2));
			overflow_flag = 1;
			abnormal_cnt += data_cnt;
			data_cnt = 0;
		}
						
	}   /* end of while */

	abnormal_cnt += data_cnt;

	mysql_free_result(result1);
	//mysql_free_result(result2);

	cur_time = time(NULL);

	stop_wait();
	
	printf("\n\n****************************************************************\n");
	printf("=============> Result Count 	: %d Rows\n", result_cnt);
	printf("=============> Abnormal Count 	: %d Rows\n", abnormal_cnt);
	printf("=============> Elapsed Time 	: %d Seconds\n", cur_time - offset_time);
	printf("****************************************************************\n");
	fclose(fd);
	return (1);

}


/**
 *		UDR Search Data Service Type
 **/ 
int udr_arrange_sum_dst(int search_year,
						int start_time, 
						int end_time, 
						char *write_file,
						int data_s_type)
{

	int				check_udr_cnt, udr_count, acct_cnt;
	int				result_cnt, abnormal_cnt, data_cnt;
	int				start_on, overflow_flag;
	int				search_key, i;
	int				ip_up_sum, ip_down_sum, re_in_sum, re_out_sum, acct_stat_type;
	int				ip_up, ip_down, re_in, re_out;
	int				data_service_type;
	char			query[4096];
	char			data_buf[MAX_UDR_DATA][2048], data_buf2[MAX_UDR_DATA][2048];
	char			search_acct_sess_id[32], search_min[32];
	time_t			offset_time, cur_time;
	FILE			*fd;
	MYSQL_RES		*result1, *result2;
	MYSQL_ROW		row, row1;


	udr_count = acct_cnt = result_cnt = data_cnt = abnormal_cnt = 0;
	start_on = overflow_flag = 0;
	ip_up_sum = ip_down_sum = re_in_sum = re_out_sum = 0;
	ip_up = ip_down = re_in = re_out = 0;
	memset(query, 0x00, sizeof(query));

	offset_time = time(NULL);

	/** file open **/
	fd = fopen(write_file, "w");
	if (!fd)
	{
		fprintf(stderr, "FILE OPEN ERROR : %s\n", strerror(errno));
		return (-1);
	}

	printf("\n========> SEARCHING ...... WAIT\n\n");
	start_wait();

	sprintf(query, "SELECT * from %s WHERE (crt_year = %d AND crt_time BETWEEN %d AND %d) ORDER BY acct_sess_id, min, udr_sequence",
						ACCT_TABLE_NAME, search_year, start_time, end_time);
	if (mysql_query(conn, query) != 0)
	{
		fprintf(stderr, "Search ACCT INFO TABLE FAILURE : %s\n", mysql_error(conn));
		stop_wait();
		return (-1);
	}

	/** write title to file **/
	write_title(fd);

	result1 = mysql_store_result(conn);
	memset(search_acct_sess_id, 0x00, sizeof(search_acct_sess_id));
	memset(search_min, 0x00, sizeof(search_min));
	memset(data_buf,   0x00, sizeof(data_buf));
	memset(data_buf2,  0x00, sizeof(data_buf2));
	
	while ((row = mysql_fetch_row(result1)) != NULL)
	{

		
		sprintf(data_buf[data_cnt],  "%s,%s,%s,%s,%s,%s,%s,0x%s,0x%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,", 
						row[5], row[4], row[65], row[60], row[61],
						row[8], row[62], row[56], row[57], row[13],
						row[14], row[11], row[9], row[10], row[63],
						row[52], row[20], row[21], row[15], row[22],
						row[23], row[24], row[25], row[26], row[27],
						row[17], row[31], row[38], row[51], row[33],
						row[32], row[48], row[36], row[37], row[18],
						row[40], row[41], row[42], row[43], row[44],
						row[53], row[54], row[45], row[46], row[34],
						row[35], row[47], row[59], row[38], row[49],
						row[16], row[30], row[29], row[64], row[19],
						row[39]);
						

		//fputs(data_buf, fd);
		acct_cnt++;


		if (strcmp(search_acct_sess_id, row[56]) == 0 && strcmp(search_min, row[65]) == 0)
		{
			;
		}
		else
		{
			ip_up_sum = ip_down_sum = re_in_sum = re_out_sum = 0;	
		}


		/** UDR_INFO TABLE Search **/
		strcpy(search_acct_sess_id, row[56]); 
		strcpy(search_min, row[65]);
		search_key = atoi(row[68]);
		
		/** accouting type : start/interim/stop  **/
		acct_stat_type = atoi(row[16]);
		if (acct_stat_type == 1)
		{
			start_on = 1;
			abnormal_cnt += data_cnt;
			data_cnt = 0;
			overflow_flag = 0;
		}

		memset(query, 0x00, sizeof(query));
		sprintf(query, "SELECT * from %s WHERE (acct_sess_id = '%s' AND min = '%s' AND matching_key = %d)",
						UDR_TABLE_NAME, search_acct_sess_id, search_min, search_key);

		if (mysql_real_query(conn, query, strlen(query)) != 0)
		{
			fprintf(stderr, "Search UDR INFO TABLE FAILURE : %s\n", mysql_error(conn));
			stop_wait();
			return (-1);
		}
		
		result2 = mysql_store_result(conn);

		check_udr_cnt = 0;
		while ((row1 = mysql_fetch_row(result2)) != NULL)
		{

			//if (check_udr_cnt != 0)
			//	fputs(data_buf, fd);


			if (acct_stat_type == 3)   /** accouting-interim **/
			{
				ip_up = atoi(row1[16]);
				ip_down = atoi(row1[17]);
				re_in = atoi(row1[18]);
				re_out = atoi(row1[19]);

				ip_up_sum += ip_up;
				ip_down_sum += ip_down;
				re_in_sum += re_in;
				re_out_sum += re_out;

				if (atoi(row1[3]) != 0)
					data_service_type = atoi(row1[3]);
			}
				
			if (acct_stat_type == 2 )   /* accounting-stop **/
			{

				sprintf(data_buf2[data_cnt++], "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%d,%d,%d,%d,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
								row1[3], row1[4], row1[5], row1[6], row1[7],
								row1[8], row1[9], row1[10], row1[34], row1[11],
								row1[12], row1[13], row1[14], row1[15], ip_up_sum,
								ip_down_sum, re_in_sum, re_out_sum, row1[20], row1[38],
								row1[21], row1[22], row1[23], row1[24], row1[25],
								row1[26], row1[39], row1[30], row1[31], row1[32],
								row1[33], row1[27], row1[28], row1[29], row1[35], row1[40]);
			}
			else   /** any other than accounting-stop **/
			{
				sprintf(data_buf2[data_cnt++], "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
								row1[3], row1[4], row1[5], row1[6], row1[7],
								row1[8], row1[9], row1[10], row1[34], row1[11],
								row1[12], row1[13], row1[14], row1[15], row1[16],
								row1[17], row1[18], row1[19], row1[20], row1[38],
								row1[21], row1[22], row1[23], row1[24], row1[25],
								row1[26], row1[39], row1[30], row1[31], row1[32],
								row1[33], row1[27], row1[28], row1[29], row1[35], row1[40]);

			}
			
			//fputs(data_buf2, fd);
			check_udr_cnt++;
			udr_count++;
		}

		mysql_free_result(result2);
		if (check_udr_cnt == 0)
		{
			if (acct_stat_type == 2)
			{
				sprintf(data_buf2[data_cnt++], " , , , , , , , , , , , , ,%d,%d,%d,%d, , , , , , , , , , , , , , , , , , \n",
								ip_up_sum, ip_down_sum, re_in_sum, re_out_sum);
				//fputs(data_buf2, fd);
			}
			else 
			{
				sprintf(data_buf2[data_cnt++], "\n");
				//fputs(data_buf2, fd);
			}
		}

		if (acct_stat_type == 2)
		{
			if ((data_s_type == data_service_type) && (start_on == 1) && (overflow_flag != 1))
			{
				result_cnt += data_cnt;
				for(i=0; i<data_cnt; i++)
				{
					fputs(data_buf[i], fd);
					fputs(data_buf2[i], fd);
				}
			}
			memset(data_buf,  0x00, sizeof(data_buf));
			memset(data_buf2, 0x00, sizeof(data_buf2));


			if (start_on == 0 || overflow_flag == 1)
				abnormal_cnt += data_cnt;

			data_cnt = 0;
			start_on = 0;

		}  /* end of if (acct_stat_type == 2) */

		if (data_cnt == MAX_UDR_DATA)
		{
			//fprintf(stderr, "UDR data exceed maximum count : %d\n", MAX_UDR_DATA);

			memset(data_buf,  0x00, sizeof(data_buf));
			memset(data_buf2, 0x00, sizeof(data_buf2));
			overflow_flag = 1;
			abnormal_cnt += data_cnt;
			data_cnt = 0;
		}
						
	}  /* end of while */

	mysql_free_result(result1);
	//mysql_free_result(result2);

	abnormal_cnt += data_cnt;
	stop_wait();

	cur_time = time(NULL);
	printf("\n\n****************************************************************\n");
	printf("=============> Result Count		: %d\n", result_cnt);
	printf("=============> Elapsed Time		: %d Seconds\n", cur_time - offset_time);
	printf("****************************************************************\n");
	fclose(fd);
	return (1);
	

}


/**
 *		UDR Search with time & data service type
 **/ 
int udr_arrange_cond_sum_dst(int search_year,
						 	int start_time,
						 	int end_time,
						 	char *write_file,
							int	search_opt,
						 	int data_s_type)

{

	int				check_udr_cnt, udr_count, acct_cnt;
	int				search_key;
	int				ip_up_sum, ip_down_sum, re_in_sum, re_out_sum, acct_stat_type;
	int				ip_up, ip_down, re_in, re_out;
	int				acct_in_pkt, acct_out_pkt;
	int				data_cnt, result_cnt, abnormal_cnt;
	int				data_service_type;
	char			start_on, overflow_flag;
	char			query[4096];
	char			data_buf[MAX_UDR_DATA][2048], data_buf2[MAX_UDR_DATA][2048];
	char			search_acct_sess_id[32], search_min[32];
	time_t			offset_time, cur_time;
	FILE			*fd;
	MYSQL_RES		*result1, *result2;
	MYSQL_ROW		row, row1;


	udr_count = acct_cnt = data_cnt = result_cnt = abnormal_cnt = 0;
	ip_up_sum = ip_down_sum = re_in_sum = re_out_sum = 0;
	ip_up = ip_down = re_in = re_out = 0;
	acct_in_pkt = acct_out_pkt = 0;
	start_on = overflow_flag = 0;
	memset(query, 0x00, sizeof(query));


	offset_time = time(NULL);

	/** file open **/
	fd = fopen(write_file, "w");
	if (!fd)
	{
		fprintf(stderr, "FILE OPEN ERROR : %s\n", strerror(errno));
		return (-1);
	}

	printf("\n========> SEARCHING ...... WAIT\n\n");
	start_wait();
	
	sprintf(query, "SELECT * from %s WHERE (crt_year = %d AND crt_time BETWEEN %d AND %d) ORDER BY acct_sess_id, min, udr_sequence",
						ACCT_TABLE_NAME, search_year, start_time, end_time);
	if (mysql_query(conn, query) != 0)
	{
		fprintf(stderr, "Search ACCT INFO TABLE FAILURE : %s\n", mysql_error(conn));
		stop_wait();
		return (-1);
	}


	/** write title to file **/
	write_title(fd);

	result1 = mysql_store_result(conn);
	memset(search_acct_sess_id, 0x00, sizeof(search_acct_sess_id));
	memset(search_min, 0x00, sizeof(search_min));
	memset(data_buf,  0x00, sizeof(data_buf));
	memset(data_buf2, 0x00, sizeof(data_buf2));
	
	while ((row = mysql_fetch_row(result1)) != NULL)
	{
		
		sprintf(data_buf[data_cnt],  "%s,%s,%s,%s,%s,%s,%s,0x%s,0x%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,",
						row[5], row[4], row[65], row[60], row[61],
						row[8], row[62], row[56], row[57], row[13],
						row[14], row[11], row[9], row[10], row[63],
						row[52], row[20], row[21], row[15], row[22],
						row[23], row[24], row[25], row[26], row[27],
						row[17], row[31], row[38], row[51], row[33],
						row[32], row[48], row[36], row[37], row[18],
						row[40], row[41], row[42], row[43], row[44],
						row[53], row[54], row[45], row[46], row[34],
						row[35], row[47], row[59], row[38], row[49],
						row[16], row[30], row[29], row[64], row[19],
						row[39]);
						

		//fputs(data_buf, fd);
		acct_cnt++;

		if (strcmp(search_acct_sess_id, row[56]) == 0 && strcmp(search_min, row[65]) == 0)
		{
			;
		}
		else
		{
			ip_up_sum = ip_down_sum = re_in_sum = re_out_sum = 0;	
		}

		/** UDR_INFO TABLE Search **/
		strcpy(search_acct_sess_id, row[56]); 
		strcpy(search_min, row[65]);
		search_key = atoi(row[68]);
		

		/** accounting type : start/interim/stop  **/
		acct_stat_type = atoi(row[16]);
		if (acct_stat_type == 1)
		{
			start_on = 1;
			abnormal_cnt += data_cnt;
			data_cnt = 0;
			overflow_flag = 0;
		}

		else if (acct_stat_type == 2)
		{
			acct_in_pkt = atoi(row[32]);
			acct_out_pkt = atoi(row[33]);
			//start_on = 0;
		}

		memset(query, 0x00, sizeof(query));
		sprintf(query, "SELECT * from %s WHERE (acct_sess_id = '%s' AND min = '%s' AND matching_key = %d)",
						UDR_TABLE_NAME, search_acct_sess_id, search_min, search_key);

		if (mysql_real_query(conn, query, strlen(query)) != 0)
		{
			fprintf(stderr, "Search UDR INFO TABLE FAILURE : %s\n", mysql_error(conn));
			stop_wait();
			return (-1);
		}
		

		result2 = mysql_store_result(conn);

		check_udr_cnt = 0;
		while ((row1 = mysql_fetch_row(result2)) != NULL)
		{

			//if (check_udr_cnt != 0)
				//fputs(data_buf[data_cnt], fd);

			//memset(data_buf2, 0x00, sizeof(data_buf));

			if (acct_stat_type == 3)   /** accouting-interim **/
			{
				ip_up = atoi(row1[16]);
				ip_down = atoi(row1[17]);
				re_in = atoi(row1[18]);
				re_out = atoi(row1[19]);

				ip_up_sum += ip_up;
				ip_down_sum += ip_down;
				re_in_sum += re_in;
				re_out_sum += re_out;

				data_service_type = atoi(row1[3]);
			}


				
			if (acct_stat_type == 2 )   /* accounting-stop **/
			{

				sprintf(data_buf2[data_cnt++], "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
								row1[3], row1[4], row1[5], row1[6], row1[7],
								row1[8], row1[9], row1[10], row1[34], row1[11],
								row1[12], row1[13], row1[14], row1[15], ip_up_sum,
								ip_down_sum, re_in_sum, re_out_sum, row1[20], row1[38],
								row1[21], row1[22], row1[23], row1[24], row1[25],
								row1[26], row1[39], row1[30], row1[31], row1[32],
								row1[33], row1[27], row1[28], row1[29], row1[35], row1[40]);

			}
			else   /** any other than accounting-stop **/
			{
				sprintf(data_buf2[data_cnt++], "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
								row1[3], row1[4], row1[5], row1[6], row1[7],
								row1[8], row1[9], row1[10], row1[34], row1[11],
								row1[12], row1[13], row1[14], row1[15], row1[16],
								row1[17], row1[18], row1[19], row1[20], row1[38],
								row1[21], row1[22], row1[23], row1[24], row1[25],
								row1[26], row1[39], row1[30], row1[31], row1[32],
								row1[33], row1[27], row1[28], row1[29], row1[35], row1[40]);

			}
			
			//fputs(data_buf2, fd);
			check_udr_cnt++;
			udr_count++;
		}

		mysql_free_result(result2);
		if (check_udr_cnt == 0)
		{
			if (acct_stat_type == 2)
			{
				//memset(data_buf2, 0x00, sizeof(data_buf2));
				sprintf(data_buf2[data_cnt++], " , , , , , , , , , , , , ,%d,%d,%d,%d, , , , , , , , , , , , , , , , , , \n",
								ip_up_sum, ip_down_sum, re_in_sum, re_out_sum);
				//fputs(data_buf2, fd);
			}
			else 
			{
				//memset(data_buf2, 0x00, sizeof(data_buf2));
				sprintf(data_buf2[data_cnt++], "\n");
				//fputs(data_buf2, fd);
			}
		}

		if (acct_stat_type == 2)
		{
			int 		i;

			switch (search_opt)
			{

				case 2 :
					if (((acct_in_pkt > ip_up_sum) || (acct_out_pkt > ip_down_sum)) &&
					    (data_s_type == data_service_type) && (start_on == 1) && (overflow_flag != 1))
					{
						result_cnt += data_cnt;
						for(i=0; i<data_cnt; i++)
						{
							fputs(data_buf[i], fd);
							fputs(data_buf2[i], fd);
						}
					}

					break;

				case 3 :
					if (((acct_in_pkt < ip_up_sum) || (acct_out_pkt < ip_down_sum)) &&
						(data_s_type == data_service_type) && (start_on == 1) && (overflow_flag != 1))
					{
						result_cnt += data_cnt;
						for(i=0; i<data_cnt; i++)
						{
							fputs(data_buf[i], fd);
							fputs(data_buf2[i], fd);
						}
					}

					break;

				case 4 :
					if (((acct_in_pkt == ip_up_sum) || (acct_out_pkt == ip_down_sum)) &&
						(data_s_type == data_service_type) && (start_on == 1) && (overflow_flag != 1))
					{
						result_cnt += data_cnt;
						for(i=0; i<data_cnt; i++)
						{
							fputs(data_buf[i], fd);
							fputs(data_buf2[i], fd);
						}
					}
					break;

				default :
					break;

			}	/* end of switch */

			memset(data_buf,  0x00, sizeof(data_buf));
			memset(data_buf2, 0x00, sizeof(data_buf2));

			if (start_on == 0 || overflow_flag == 1)
				abnormal_cnt += data_cnt;

			data_cnt = 0;
			start_on = 0;

		}		/* end of if (acct_stat_type == 2) */

		//data_cnt++;
		if (data_cnt == MAX_UDR_DATA)
		{
			//fprintf(stderr, "UDR data exceed maximum count : %d\n", MAX_UDR_DATA);

			memset(data_buf,  0x00, sizeof(data_buf));
			memset(data_buf2, 0x00, sizeof(data_buf2));
			overflow_flag = 1;
			abnormal_cnt += data_cnt;
			data_cnt = 0;
		}
						
	}   /* end of while */

	abnormal_cnt += data_cnt;

	mysql_free_result(result1);
	//mysql_free_result(result2);

	cur_time = time(NULL);

	stop_wait();
	
	printf("\n\n****************************************************************\n");
	printf("=============> Result Count 	: %d Rows\n", result_cnt);
	printf("=============> Elapsed Time 	: %d Seconds\n", cur_time - offset_time);
	printf("****************************************************************\n");
	fclose(fd);
	return (1);

}


/**
 *		UDR Search Procedure
 **/ 
int search_user(int cate_key, char *search_userinfo, char *write_file)
{

	int				check_udr_cnt, udr_count, acct_cnt;
	int				search_key;
	char			query[4096];
	char			data_buf[8192], data_buf2[8192];
	char			search_acct_sess_id[32], search_min[32];
	FILE			*fd;
	time_t			offset_time, cur_time;
	MYSQL_RES		*result1, *result2;
	MYSQL_ROW		row, row1;


	udr_count = acct_cnt = 0;
	memset(query, 0x00, sizeof(query));

	/** file open **/
	fd = fopen(write_file, "w");
	if (!fd)
	{
		fprintf(stderr, "FILE OPEN ERROR : %s\n", strerror(errno));
		return (-1);
	}

	offset_time = time(NULL);

	if (cate_key == 1)    	/** search for min **/
		sprintf(query, "SELECT * from %s WHERE (min = '%s')",
						ACCT_TABLE_NAME, search_userinfo);
	else if(cate_key == 2)	/** search for username  **/
		sprintf(query, "SELECT * from %s WHERE (user_name = '%s')",
						ACCT_TABLE_NAME, search_userinfo);

	else
		return (-1);

	printf("\n========> SEARCHING ...... WAIT\n\n");
	start_wait();

	if (mysql_query(conn, query) != 0)
	{
		fprintf(stderr, "Search ACCT INFO TABLE FAILURE : %s\n", mysql_error(conn));
		stop_wait();
		return (-1);
	}

	/** write tile to file **/
	write_title(fd);

	result1 = mysql_store_result(conn);
	
	while ((row = mysql_fetch_row(result1)) != NULL)
	{

		memset(data_buf, 0x00, sizeof(data_buf));
		
	
		sprintf(data_buf,  "%s,%s,%s,%s,%s, %s,%s,0x%s,0x%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,", 
						row[5], row[4], row[65], row[60], row[61],
						row[8], row[62], row[56], row[57], row[13],
						row[14], row[11], row[9], row[10], row[63],
						row[52], row[20], row[21], row[15], row[22],
						row[23], row[24], row[25], row[26], row[27],
						row[17], row[31], row[38], row[51], row[33],
						row[32], row[48], row[36], row[37], row[18],
						row[40], row[41], row[42], row[43], row[44],
						row[53], row[54], row[45], row[46], row[34],
						row[35], row[47], row[59], row[38], row[49],
						row[16], row[30], row[29], row[64], row[19],
						row[39]);
						

		fputs(data_buf, fd);
		acct_cnt++;
						
		memset(search_acct_sess_id, 0x00, sizeof(search_acct_sess_id));
		memset(search_min, 0x00, sizeof(search_min));
		strcpy(search_acct_sess_id, row[56]); 
		strcpy(search_min, row[65]);
		search_key = atoi(row[68]);
		
		memset(query, 0x00, sizeof(query));
		sprintf(query, "SELECT * from %s WHERE (acct_sess_id = '%s' AND min = '%s' AND matching_key = %d)",
						UDR_TABLE_NAME, search_acct_sess_id, search_min, search_key);

		if (mysql_real_query(conn, query, strlen(query)) != 0)
		{
			fprintf(stderr, "Search UDR INFO TABLE FAILURE : %s\n", mysql_error(conn));
			stop_wait();
			return (-1);
		}
		
		result2 = mysql_store_result(conn);

		check_udr_cnt = 0;
		while ((row1 = mysql_fetch_row(result2)) != NULL)
		{

			if (check_udr_cnt != 0)
				fputs(data_buf, fd);

			memset(data_buf2, 0x00, sizeof(data_buf));

			sprintf(data_buf2, "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
								row1[3], row1[4], row1[5], row1[6], row1[7],
								row1[8], row1[9], row1[10], row1[34], row1[11],
								row1[12], row1[13], row1[14], row1[15], row1[16],
								row1[17], row1[18], row1[19], row1[20], row1[38],
								row1[21], row1[22], row1[23], row1[24], row1[25],
								row1[26], row1[39], row1[30], row1[31], row1[32],
								row1[33], row1[27], row1[28], row1[29], row1[35], row1[40]);

			
			fputs(data_buf2, fd);
			check_udr_cnt++;
			udr_count++;
		}

		mysql_free_result(result2);
		if (check_udr_cnt == 0)
		{
			memset(data_buf2, 0x00, sizeof(data_buf2));
			sprintf(data_buf2, "\n");
			fputs(data_buf2, fd);
		}
						
	}

	mysql_free_result(result1);
	//mysql_free_result(result2);

	stop_wait();
	cur_time = time(NULL);

	printf("\n\n****************************************************************\n");
	printf("=============> ACCT Count 		: %d\n", acct_cnt);
	printf("=============> UDR Count 		: %d\n", udr_count);
	printf("=============> Elapsed Time		: %d Seconds\n", cur_time - offset_time);
	printf("****************************************************************\n");
	fclose(fd);
	return (1);

}


/**
 *		UDR Search Procedure
 **/ 
int direct_search(char *write_file, char *direct_query)
{

	int				check_udr_cnt, udr_count, acct_cnt;
	int				search_key;
	char			query[4096];
	char			data_buf[8192], data_buf2[8192];
	char			search_acct_sess_id[32], search_min[32];
	FILE			*fd;
	time_t			offset_time, cur_time;
	MYSQL_RES		*result1, *result2;
	MYSQL_ROW		row, row1;


	udr_count = acct_cnt = 0;
	memset(query, 0x00, sizeof(query));

	/** file open **/
	fd = fopen(write_file, "w");
	if (!fd)
	{
		fprintf(stderr, "FILE OPEN ERROR : %s\n", strerror(errno));
		return (-1);
	}

	offset_time = time(NULL);

	printf("\n========> SEARCHING ...... WAIT\n\n");
	start_wait();

	if (mysql_real_query(conn, direct_query, strlen(direct_query)) != 0)
	{
		fprintf(stderr, "Search ACCT INFO TABLE FAILURE : %s\n", mysql_error(conn));
		stop_wait();
		return (-1);
	}

	/** write tile to file **/
	write_title(fd);

	result1 = mysql_store_result(conn);
	
	while ((row = mysql_fetch_row(result1)) != NULL)
	{

		memset(data_buf, 0x00, sizeof(data_buf));
		
		sprintf(data_buf,  "%s,%s,%s,%s,%s, %s,%s,0x%s,0x%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,", 
						row[5], row[4], row[65], row[60], row[61],
						row[8], row[62], row[56], row[57], row[13],
						row[14], row[11], row[9], row[10], row[63],
						row[52], row[20], row[21], row[15], row[22],
						row[23], row[24], row[25], row[26], row[27],
						row[17], row[31], row[38], row[51], row[33],
						row[32], row[48], row[36], row[37], row[18],
						row[40], row[41], row[42], row[43], row[44],
						row[53], row[54], row[45], row[46], row[34],
						row[35], row[47], row[59], row[38], row[49],
						row[16], row[30], row[29], row[64], row[19],
						row[39]);
						

		fputs(data_buf, fd);
		acct_cnt++;
						
		memset(search_acct_sess_id, 0x00, sizeof(search_acct_sess_id));
		memset(search_min, 0x00, sizeof(search_min));
		strcpy(search_acct_sess_id, row[56]); 
		strcpy(search_min, row[65]);
		search_key = atoi(row[68]);
		
		memset(query, 0x00, sizeof(query));
		sprintf(query, "SELECT * from %s WHERE (acct_sess_id = '%s' AND min = '%s' AND matching_key = %d)",
						UDR_TABLE_NAME, search_acct_sess_id, search_min, search_key);

		if (mysql_real_query(conn, query, strlen(query)) != 0)
		{
			fprintf(stderr, "Search UDR INFO TABLE FAILURE : %s\n", mysql_error(conn));
			stop_wait();
			return (-1);
		}
		
		result2 = mysql_store_result(conn);

		check_udr_cnt = 0;
		while ((row1 = mysql_fetch_row(result2)) != NULL)
		{

			if (check_udr_cnt != 0)
				fputs(data_buf, fd);

			memset(data_buf2, 0x00, sizeof(data_buf));

			sprintf(data_buf2, "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
								row1[3], row1[4], row1[5], row1[6], row1[7],
								row1[8], row1[9], row1[10], row1[34], row1[11],
								row1[12], row1[13], row1[14], row1[15], row1[16],
								row1[17], row1[18], row1[19], row1[20], row1[38],
								row1[21], row1[22], row1[23], row1[24], row1[25],
								row1[26], row1[39], row1[30], row1[31], row1[32],
								row1[33], row1[27], row1[28], row1[29], row1[35], row1[40]);
			
			fputs(data_buf2, fd);
			check_udr_cnt++;
			udr_count++;
		}
		mysql_free_result(result2);
		if (check_udr_cnt == 0)
		{
			memset(data_buf2, 0x00, sizeof(data_buf2));
			sprintf(data_buf2, "\n");
			fputs(data_buf2, fd);
		}
						
	}

	mysql_free_result(result1);
	//mysql_free_result(result2);

	stop_wait();
	cur_time = time(NULL);

	printf("\n\n****************************************************************\n");
	printf("=============> ACCT Count 		: %d\n", acct_cnt);
	printf("=============> UDR Count 		: %d\n", udr_count);
	printf("=============> Elapsed Time		: %d Seconds\n", cur_time - offset_time);
	printf("****************************************************************\n");

	fclose(fd);
	return (1);

}


/**
 *		delete all acct & udr data from acct_info and udr_info table
 **/
delete_all()
{

	char			query[4096];

	memset(query, 0x00, sizeof(query));

	sprintf(query, "DELETE FROM %s", ACCT_TABLE_NAME);
	if (mysql_query(conn, query) != 0)
	{
		fprintf(stderr, "Delete ACCOUTING INFO TABLE FAILURE : %s\n", mysql_error(conn));
	}

	memset(query, 0x00, sizeof(query));

	sprintf(query, "DELETE FROM %s", UDR_TABLE_NAME);
	if (mysql_query(conn, query) != 0)
	{
		fprintf(stderr, "Delete UDR INFO TABLE FAILURE : %s\n", mysql_error(conn));
	}

	return (1);
}


/**
 *		delete time based-conditional acct & udr data 
 **/
delete_proc(int del_year, int del_start, int del_end)
{
	
	char 	query[4096];

	memset(query, 0x00, sizeof(query));
	sprintf(query, "DELETE FROM %s WHERE (crt_year = %d AND crt_time BETWEEN %d AND %d)",
						ACCT_TABLE_NAME, del_year, del_start, del_end);

	if (mysql_query(conn, query) != 0)
	{
		fprintf(stderr, "Delete ACCOUTING INFO TABLE FAILURE : %s\n", mysql_error(conn));
	}

	memset(query, 0x00, sizeof(query));
	sprintf(query, "DELETE FROM %s WHERE (crt_year = %d AND crt_time BETWEEN %d AND %d)",
						UDR_TABLE_NAME, del_year, del_start, del_end);

	if (mysql_query(conn, query) != 0)
	{
		fprintf(stderr, "Delete UDR INFO TABLE FAILURE : %s\n", mysql_error(conn));
	}

	return (1);

}
