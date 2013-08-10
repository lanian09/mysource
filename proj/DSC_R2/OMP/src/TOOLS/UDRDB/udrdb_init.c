#include "udrdb.h"

extern	int		errno;
MYSQL	sql,	*conn;
static 	int		file_cnt;
static 	int		prev_search_year, prev_start_time, prev_end_time;

/**
 *	mysql initialize
 **/
mysql_initialize()
{

    int     key, shmId;
    int     i;

    mysql_init (&sql);
    if ((conn = mysql_real_connect(&sql, "localhost", UDRDB_ACCOUNT, UDRDB_PASSWORD, 
        						   UDR_DATABASE, 0, 0, 0)) == NULL) 
    {
        fprintf(stderr,"[MYSQL] mysql_real_connect fail; err=%s\n", mysql_error(&sql));
        return (-1);
    }

    return 1;
}

/**
 *
 **/
udrdb_proc()
{
	
	int		start_year, start_month;
	int		start_day, start_hour, start_minute;
	int		end_day, end_hour, end_minute;
	int		select_num, del_sel;

	start_year = start_month = 0;
	start_day = start_hour = start_minute = 0;
	end_day = end_hour = end_minute = 0;
	select_num = 0;


START_MENU:
	printf("\n\n=============================================================\n");
	printf("======            UDR DB INSERT/SELECT/DELETE          ======\n");
	printf("=============================================================\n\n");
	printf("[1] CHOICE UDR-DB OPERATION \n");
	printf("1. INSERT UDR \n");
	printf("2. SELECT UDR \n");
	printf("3. DELETE UDR \n");
	printf("4. EXIT \n\n");

	printf("INPUT ===> ");
	scanf("%d", &select_num);

	printf("\n\n");

	if (select_num == 1)
	{

INSERT_PROC :
		printf("-----------------------------------------------------\n");
		printf("|                 INSERT PROCEDURE                  |\n");
		printf("|              [ -1 : GOTO TOP MENU]                |\n");
		printf("-----------------------------------------------------\n\n");

		printf("[1] INPUT INSERT YEAR [2006 ~] : ");
		scanf("%d", &start_year);
		if (start_year < 2006 || start_year > 2100)
		{
			if (start_year == -1)
				goto START_MENU;

			printf("You input wrong year info... retry : %d\n", start_year);
			goto INSERT_PROC;
		}
		printf("\n");

		printf("[2] INPUT INSERT MONTH [1~12] 	: ");
		scanf("%d", &start_month);
		if (start_month < 1 || start_month > 12)
		{
			if (start_month == -1)
				goto START_MENU;

			printf("You input wrong month info... retry : %d\n", start_month);
			goto INSERT_PROC;
		}
		printf("\n");

		printf("[3-1] Start Day [1~31]		: ");
		scanf("%d", &start_day);
		if (start_day < 1 || start_day > 31)
		{
			if (start_day == -1)
				goto START_MENU;

			printf("You input wrong start-day info... retry : %d\n", start_day);
			goto INSERT_PROC;
		}

		printf("[3-2] Start Hour [1~24]		: ");
		scanf("%d", &start_hour);
		if (start_hour < 1 || start_hour > 24)
		{
			if (start_hour == -1)
				goto START_MENU;

			printf("You input wrong start-hour info... retry : %d\n", start_hour);
			goto INSERT_PROC;
		}

		printf("[3-3] Start Minute [1~60]	: ");
		scanf("%d", &start_minute);
		if (start_minute < 0 || start_minute > 60) 
		{
			if (start_minute == -1)
				goto START_MENU;

			printf("You input wrong start-minute info... retry : %d\n", start_minute);
			goto INSERT_PROC;
		}
		printf("\n");


		printf("[4-1] End Day [1~31]		: ");
		scanf("%d", &end_day);
		if (end_day < 1 || end_day > 31)
		{
			if (end_day == -1)
				goto START_MENU;

			printf("You input wrong end-day info... retry : %d\n", end_day);
			goto INSERT_PROC;
		}

		printf("[4-2] End Hour [1~24]		: ");
		scanf("%d", &end_hour);
		if (end_hour < 1 || end_hour > 24)
		{
			if (end_hour == -1)
				goto START_MENU;

			printf("You input wrong end-hour info... retry : %d\n", end_hour);
			goto INSERT_PROC;
		}

		printf("[4-3] End Minute [1~60]		: ");
		scanf("%d", &end_minute);
		if (end_minute < 0 || end_minute > 60) 
		{
			if (end_minute == -1)
				goto START_MENU;

			printf("You input wrong end-minute info... retry : %d\n", end_minute);
			goto INSERT_PROC;
		}
		printf("\n");

		search_dir( start_year, start_month,
					start_day, start_hour, start_minute,
					end_day, end_hour, end_minute);
	}

	else if (select_num == 2)
	{
		int		search_sel;
		int		search_year, start_time, end_time;
		char	file_name[64];
		char	direct_query[512];

		memset(file_name, 	0x00, sizeof(file_name));
		memset(direct_query,0x00, sizeof(direct_query));

SEARCH_PROC :
		printf("-----------------------------------------------------\n");
		printf("|                 SEARCH PROCEDURE                  |\n");
		printf("|              [ -1 : GOTO TOP MENU ]               |\n");
		printf("|         [ 1 : SAME VALUE PREVIOUS INPUT ]         |\n");
		printf("-----------------------------------------------------\n\n");

		printf("[1] Input Search Year [2006 ~] : ");
		scanf("%d", &search_year);
		if(search_year < 2006 || search_year > 2100)
		{
			if (search_year == -1)
				goto START_MENU;
			else if(search_year == 1)
				search_year = prev_search_year;
			else
			{
				fprintf(stderr, "Input Wrong Search Year... retry\n\n");
				goto SEARCH_PROC;
			}
		}
		prev_search_year = search_year;
		printf("\n");

		printf("[2] Input Search Start time (월일시분초, e.g. 1230153212) : ");
		scanf("%d", &start_time);
		if (start_time < 101000000 || start_time > 1300000000)
		{
			if (start_time == -1)
				goto START_MENU;
			else if(start_time == 1)
				start_time = prev_start_time;
			else
			{
				fprintf(stderr, "Input Wrong Search Start Time... retry\n\n");
				goto SEARCH_PROC;
			}
		}
		prev_start_time = start_time;
		printf("\n");

		printf("[3] Input Search End time (월일시분초, e.g. 1230153212) : ");
		scanf("%d", &end_time);
		if (end_time < 101000000 || end_time > 1300000000)
		{
			if (end_time == -1)
				goto START_MENU;
			else if(end_time == 1)
				end_time = prev_end_time;
			else
			{
				fprintf(stderr, "Input Wrong Search End Time... retry\n\n");
				goto SEARCH_PROC;
			}
		}
		prev_end_time = end_time;
		printf("\n");

		printf("[4] Input Result File Name : ");
		scanf("%s", file_name);
		if (atoi(file_name) == -1)
			goto START_MENU;
		printf("\n");

SUB_SEARCH:
		printf("-----------------------------------------------------\n");
		printf("|                 SUB-SEARCH PROCEDURE              |\n");
		printf("-----------------------------------------------------\n\n");
		printf("<1> User-Based Search\n");
		printf("<2> Direct Query (full-SQL syntax) \n");
		printf("<3> UDR Arrangement (Order by Correlation-Id) \n");
		printf("<4> UDR Arrangement & Conditional Search \n");
		printf("<5> Goto Previou MENU \n");
		printf("<6> Goto TOP MENU \n\n");
		printf("Input Sub-Search Option ===> ");

		scanf("%d", &search_sel);
		printf("\n");

		if (search_sel == 1)
		{
			int		sub_search_sel;
			char	search_min[64], search_username[32];

			memset(search_min, 		0x00, sizeof(search_min));
			memset(search_username,	0x00, sizeof(search_username));

USER_SEARCH :
			printf("-----------------------------------------------------\n");
			printf("|             User-Based SEARCH PROCEDURE           |\n");
			printf("-----------------------------------------------------\n\n");
			printf("<1> Search for MIN\n"); 
			printf("<2> Search for User-Nama\n");
			printf("<3> Goto Previous MENU \n");
			printf("<4> Goto TOP MENU \n\n");
			printf("Input Sub-Search Option ===> ");
			scanf("%d", &sub_search_sel);
			printf("\n");

			if (sub_search_sel == 1)
			{
				printf("[1] Input MIN : ");
				scanf("%s", search_min);
				printf("\n");

				if (search_user(sub_search_sel, search_min, file_name) < 0)
					goto START_MENU;
			}

			else if (sub_search_sel == 2)
			{
				printf("[1] Input User-Name : ");
				scanf("%s", search_username);
				printf("\n");

				if (search_user(sub_search_sel, search_username, file_name) < 0)
					goto START_MENU;
			}

			else if (sub_search_sel == 3)
			{
				goto SUB_SEARCH;
			}

			else if (sub_search_sel == 4)
			{
				goto START_MENU;
			}

			else
			{
				printf("Input Wrong Sub-Search... retry\n\n");
				goto USER_SEARCH;
			}

		}

		else if (search_sel == 2)
		{

			gets(direct_query);

			printf("sql> ");
			gets(direct_query);

			if (direct_search(file_name, direct_query) < 0)
				goto START_MENU;
		}

		else if (search_sel == 3)
		{
			udr_arrange(search_year, start_time, end_time, file_name);
		}

		else if (search_sel == 4)
		{

			int		packet_search_sel;

VOL_DST_SEARCH :
			printf("--------------------------------------------------------------------\n");
			printf("|       Packet-Volume  or Data-Service-Type SEARCH PROCEDURE       |\n");
			printf("--------------------------------------------------------------------\n\n");
			printf("<1> Packet Volume based Search\n");
			printf("<2> Data Service Type based Search\n");
			printf("<3> Goto Previous MENU \n");
			printf("<4> Goto TOP MENU \n\n");
			printf("Input Sub-Search Option ===> ");
			scanf("%d", &packet_search_sel);
			printf("\n");

			if (packet_search_sel == 1)
			{
				int		volume_search_sel;

VOL_SEARCH :
				printf("------------------------------------------------------------\n");
				printf("|             Packet Volume Based SEARCH PROCEDURE         |\n");
				printf("------------------------------------------------------------\n\n");
				printf("<1> ALL UDR SEARCH & SUM \n"); 
				printf("<2> PDSN Up-Packet(or Down-Packet) more than BSD \n");
				printf("<3> PDSN Up-Packet(or Down-Packet) less than BSD \n");
				printf("<4> Goto Previous MENU \n");
				printf("<5> Goto TOP MENU \n\n");
				printf("Input Sub-Search Option ===> ");
				scanf("%d", &volume_search_sel);
				printf("\n");

				switch(volume_search_sel)
				{
					case 1 :
						udr_arrange_sum(search_year, start_time, end_time, file_name);
						break;
					case 2 :
					case 3 :
						udr_arrange_cond_sum(search_year,
										 start_time,
										 end_time,
										 file_name,
										 volume_search_sel);
						break;
					case 4 :
						goto VOL_DST_SEARCH;	
					case 5 :
						goto START_MENU;
					default :
						fprintf(stderr, " Input Wrong Number... retry \n\n");
						goto VOL_SEARCH;
				}
			}  /* end of if (packet_search_sel == 1) */

			else if (packet_search_sel == 2)
			{
				int		dst_search_sel;
				int		data_svc_type;

DST_SEARCH :
				printf("-------------------------------------------------------------\n");
				printf("|            Data Service Type Based SEARCH PROCEDURE       |\n");
				printf("-------------------------------------------------------------\n\n");
				printf("<1> ALL Specific Data-Service-Type UDR SEARCH & SUM \n"); 
				printf("<2> Specific Data-Service-Type and PDSN Up-Packet(or Down-Packet) more than BSD \n");
				printf("<3> Specific Data-Service-Type and PDSN Up-Packet(or Down-Packet) less than BSD \n");
				printf("<4> Goto Previous MENU \n");
				printf("<5> Goto TOP MENU \n\n");
				printf("Input Sub-Search Option ===> ");
				scanf("%d", &dst_search_sel);
				printf("\n");
				if (dst_search_sel == 4)
					goto VOL_DST_SEARCH;
				if (dst_search_sel == 5)
					goto START_MENU;

				printf("Input Data Service Type ===> ");
				scanf("%d", &data_svc_type);
				printf("\n");

				switch(dst_search_sel)
				{
					case 1 :
						udr_arrange_sum_dst(search_year,
											start_time,
											end_time, 
											file_name,
											data_svc_type);
						break;
					case 2 :
					case 3 :
						udr_arrange_cond_sum_dst(search_year,
										 start_time,
										 end_time,
										 file_name,
										 dst_search_sel,
										 data_svc_type);
						break;
					default :
						fprintf(stderr, " Input Wrong Number... retry \n\n");
						goto DST_SEARCH;
				}
				
			} /* end of else if (packet_search_sel == 2) */

			else if (packet_search_sel == 3)
			{
				goto SUB_SEARCH;
			}
			else if (packet_search_sel == 4)
			{
				goto START_MENU;
			}
			else 
			{
				printf("Input Wrong Number...retry..\n");
				goto VOL_DST_SEARCH;
			}

		} /* end of else if(search_sel == 4) */

		else if (search_sel == 5)
		{
			goto SEARCH_PROC;
		}
		else if (search_sel == 6)
		{
			goto START_MENU;
		}
		else
		{
			printf("Input Wrong Number retry...\n\n");
			goto SUB_SEARCH;
		}
	}

	else if (select_num == 3)
	{
		int		del_year;
		int		start_time, end_time;

		printf("-----------------------------------------------------\n");
		printf("|                 DELETE PROCEDURE                  |\n");
		printf("-----------------------------------------------------\n\n");
		printf("<1> Delete All acct_info & udr_info data \n");
		printf("<2> Delete conditional data for time \n\n");
		printf("Input Delete Option ===> ");
		scanf("%d", &del_sel);

		printf("\n");
		if (del_sel == 1)
		{
			delete_all();
			printf("******************************************\n");
			printf("**      Delete All Data Success !!!     **\n");
			printf("******************************************\n");

		}
		else if (del_sel == 2)
		{
			printf("[1] Input Delete Year [2006 ~] : ");
			scanf("%d", &del_year);
			if(del_year < 2006 || del_year > 2100)
			{
				fprintf(stderr, "Input Wrong Search Year [%d]... retry\n\n", del_year);
				goto START_MENU;
			}
			printf("\n");

			printf("[2] Input Delete Start time (월일시분초, e.g. 1008041224) : ");
			scanf("%d", &start_time);
			if (start_time < 101000000 || start_time > 1300000000)
			{
				fprintf(stderr, "Input Wrong Delete Start Time... retry\n\n");
				goto START_MENU;
			}
			printf("\n");

			printf("[3] Input Delete End time (월일시분초, e.g. 1008042050) : ");
			scanf("%d", &end_time);
			if (end_time < 101000000 || end_time > 1300000000)
			{
				fprintf(stderr, "Input Wrong Delete End Time... retry\n\n");
				goto START_MENU;
			}
			printf("\n");

			delete_proc(del_year, start_time, end_time);
			printf("***********************************************\n");
			printf("**      Delete Required Data Success !!!     **\n");
			printf("***********************************************\n");
		}
			
		else
		{
			printf("Input Wrong Number retry...\n\n");
			goto START_MENU;
		}
	}

	else if (select_num == 4)
		exit(1);

	else
	{
		printf("Input Wrong Number!!  retry....\n\n");
		goto START_MENU;
	}

	printf("\n\n");
	goto START_MENU;

}


/**
 *		search directory & file
 **/
int
search_dir( int search_year,
			int search_month,
			int start_day,
			int start_hour,
			int start_minute,
			int end_day,
			int end_hour,
			int end_minute)
{

	int				search_day, search_hour;
	int				diff_day, i, cnt;
	char 			system_name[8], dir_name[64];
	DIR 			*dir_pt;
	struct dirent	*dir_info;


	/** initialize variables **/
	dir_pt = NULL;
	search_day = search_hour = 0;
	diff_day = i = 0;
	file_cnt = 0;

	if (start_day > end_day)
	{
		fprintf(stderr, "You input wrong day...retry\n");
		return (-1);
	}

	diff_day = end_day - start_day;

	for (cnt = 0; cnt < 2; cnt++)
	{
		memset(system_name, 0x00, sizeof(system_name));
		if (cnt == 0)
			strcpy(system_name, "BSDA");
		else
			strcpy(system_name, "BSDB");

		if (diff_day == 0)
		{
			search_day = start_day;	
			memset(dir_name, 0x00, sizeof(dir_name));
			sprintf(dir_name, "%s/%s/%02d/%02d/%02d", REF_DIR,
										  system_name,
										  search_year,
										  search_month,
										  search_day);


			dir_pt = opendir(dir_name);

			if (dir_pt == NULL)
			{
				fprintf(stderr, "[ERR] OPEN Directory Error : %s\n", strerror(errno));
				continue;
				//return (-1);
			}

			fprintf(stdout, "OPEN Directory Success : %s\n", dir_name);

			while((dir_info = readdir(dir_pt)) != NULL)
			{
				if (!strcasecmp(dir_info->d_name, ".") || !strcasecmp(dir_info->d_name, "..") ||
					!strcasecmp(dir_info->d_name, "BAK"))
					continue;

				/** find conditional file **/
				if (find_match_file(dir_info->d_name, start_hour, start_minute,
								end_hour, end_minute) < 0)
					continue;

				printf("end find_match_file...\n");

				/** extract file and insert process and compress file **/
				if (udr_file_proc(dir_name, dir_info->d_name) < 0)
					fprintf(stderr, "[ERR] FILE NAME FORMAT WRONG\n");

				printf("end udr_file_proc...\n");

			} /** end of while **/

		} /** end of if **/

		else 
		{
			/** loop for start day ~ end day **/
			for(i=0; i <= diff_day; i++) 
			{
				search_day = start_day + i;
				search_hour = start_hour;

				sprintf(dir_name, "%s/%s/%02d/%02d/%02d", REF_DIR,
										  system_name,
										  search_year,
										  search_month,
										  search_day);


				dir_pt = opendir(dir_name);
				if (dir_pt == NULL)
				{
					fprintf(stderr, "[ERR] OPEN Directory Error : %s\n", strerror(errno));
					continue;
				}

				fprintf(stdout, "OPEN Directory Success : %s\n", dir_name);

				if (i == 0)	
					search_hour = 24;
				else
				{
					start_hour = 1;
					if (i == diff_day)
						search_hour = end_hour;
				}

				while((dir_info = readdir(dir_pt)) != NULL)
				{

					if (!strcasecmp(dir_info->d_name, ".") || !strcasecmp(dir_info->d_name, ".."))
						continue;
			
					/** find conditional file **/
					if (find_match_file(dir_info->d_name, start_hour, start_minute,
									search_hour, end_minute) < 0)
						continue;

					/** extract file and insert process and compress file **/
					if (udr_file_proc(dir_name, dir_info->d_name) < 0)
						fprintf(stderr, "[ERR] FILE NAME FORMAT WRONG\n");

				}  /** end of while **/

				/** close dir **/
				closedir(dir_pt);

			} /** end of for **/

		} /** end of else **/

	} /** end of for (cnt = 0; cnt < 2; cnt++) **/

	printf("######## FILE COUNT : %d\n", file_cnt);

}


/** 
 *   Find file for give condition 
 **/
int
find_match_file( char *file_name,
				 int start_hour,
				 int start_minute,
				 int end_hour,
				 int end_minute)
{

	char	*parsing_data;
	char	hour[4], minute[4];
	int		hour_int, min_int;
	int		i;
	
	/** variables initialize **/
	hour_int = min_int = 0;

	printf("find_match 1...%s\n", file_name);
	parsing_data = strstr(file_name, "2");

	memset(hour, 0x00, sizeof(hour));
	printf("find_match 1-2...\n");
	for (i=0; i<2; i++)
	{
		hour[i] = *(parsing_data+8+i);
	}
	hour_int = atoi(hour);

	printf("find_match 2...\n");
	if (hour_int >= start_hour && hour_int <= end_hour)
	{

		memset(minute, 0x00, sizeof(minute));
		for (i=0; i<2; i++)
		{
			minute[i] = *(parsing_data+10+i);
		}
		min_int = atoi(minute);

		printf("find_match 3...\n");
		if (hour_int == start_hour && hour_int == end_hour)
		{
			if (min_int >= start_minute && min_int <= end_minute)
			{
				file_cnt++;
				return (1);
			}
			else
				return (-1);

		}

		else if (hour_int == start_hour && hour_int != end_hour)
		{
			printf("find_match 4...\n");
			if (min_int >= start_minute)
			{
				file_cnt++;
				return (1);
			}
			else
				return (-1);
		}
		else if (hour_int == end_hour && hour_int != start_hour)
		{
			if (min_int <= end_minute)
			{
				file_cnt++;
				return (1);
			}
			else
				return (-1);
			printf("find_match 5...\n");
		}

		else  /**  searched hour is between start hour and end hour **/
		{
			file_cnt++;
			return(1);	
		}

	}
	else
	{
		return (-1);
	}
}


/** 
 *   FILE extract and get data and compress
 **/ 
int 
udr_file_proc(char *dir_path, char *file_name)
{

	char command[256];	
	char full_path[256];	
	char extract_file[256];	
	int	 file_format;

	/** initialize variables **/
	file_format = 0;
	memset(full_path, 0x00, sizeof(full_path));
	sprintf(full_path, "%s/%s", dir_path, file_name);

	if (strlen(file_name) == 36)
	{
		/** file parsing and insert db **/
		file_format = UNCOMPRESS_FILE;
		parse_insert_proc(full_path, file_format);
	}

	else if (strlen(file_name) == 39)
	{
		/** extract compress file **/
		memset(command, 0x00, sizeof(command));
		sprintf(command, "gunzip %s", full_path);
		system(command);

		/** file parsing and insert db **/
		file_format = COMPRESS_FILE;
		parse_insert_proc(full_path, file_format);

		/** compress file **/
		memset(command, 0x00, sizeof(command));
		memset(extract_file, 0x00, sizeof(extract_file));
		memcpy(extract_file, full_path, (strlen(full_path)-3));

		sprintf(command, "gzip %s", extract_file);
		system(command);
	}
	else
		return (-1);

	return (0);

}


/**
 *		write title to file
 **/
write_title(FILE *fd)
{
	
	char 	temp_buf[4096];
 
	memset(temp_buf, 0x00, sizeof(temp_buf));


	sprintf(temp_buf, "TIME-STAMP,UDR-SEQUENCE,CALLING-STATION-ID,MDN,ESN,FRAMED-IP-ADDRESS,USER-NAME,ACCT-SESSION-ID,CORRELATION-ID,SESSION-CONT,BEGINNING-SESSION,HA-IP-ADDR,NAS-IP-ADDRESS,PCF-IP-ADDR,BSID,USER-ID,F-FCH-MUX,R-FCH-MUX,SERVICE-OPTION,FTYPE,RTYPE,FCH-FRAME-SIZE,FORWARD-FCH-RC,REVERSE-FCH-RC,IP-TECHNOLOGY,COMPULSORY-TUNNEL-INDICATOR,RELEASE-INDICATOR,DCCH-FRAME-SIZE,ALWAYS-ON,ACCT-OUTPUT-OCTETS,ACCT-INPUT-OCTETS,BAD-FRAME-COUNT,EVENT-TIMESTAMP,ACTIVE-TIME,NUM-ACTIVE,SDB-INPUT-OCTEST,SDB-OUTPUT-OCTETS,NUMSDB-INPUT,NUMSDB-OUPUT,NUM-BYTES-RECEIVED-TOTAL,MIP-SIGNALING-INBOUND-COUNT,MIP-SIGNALING-OUTBOUND-COUNT,IP-QOS,AIR-PRIORITY,ACCT-INPUT-PACKETS,ACCT-OUTPUT-PACKETS,R-P-CONNECTION-ID,ACCT-AUTHENTIC,ACCT-SESSION-TIME,ACCT-TERMINATE-CAUSE,ACCT-STATUS-TYPE,NAS-PORT-TYPE,NAS-PORT,NAS-PORT-ID,SERVICE-TYPE,ACCT-DELAY-TYPE,DATA-SEVICE-TYPE,TRANSACTION-ID,REQUEST-TIME,RESPONSE-TIME,SESSION-TIME,SERVER-IP-ADDRESS,SERVER-PORT,TERMINAL-PORT,URL,DOWNLOAD-TYPE,APPLICATION-ID,CONTENT-CODE,METHOD-TYPE,RESULT-CODE,IP-LAYER-UPLOAD-SIZE,IP-LAYER-DOWNLOAD-SIZE,TCP-LAYER-RETRANS-INPUT-SIZE,TCP-LAYER-RETRANS-OUTPUT-SIZE,CP-CODE,PHONE-NUMBER,USE-COUNT,USE-TIME,TOTAL-SIZE,TOTAL-TIME,BILLCOM-HEADER-COUNT,GATEWAY-HEADER-COUNT,HANDSET-MODEL,AUDIO-UPLOAD-SIZE,AUDIO-DOWNLOAD-SIZE,VIDEO-UPLOAD-SIZE,VIDEO-DOWNLOAD-SIZE,TRANSACTION-CONTENT-LENGTH,TRANSACTION-COMPLETENESS,TRANSACTION-TERM-REASON,USER-AGENT,PACKET-COUNTER\n");
	fputs(temp_buf, fd);

	return (1);
}
