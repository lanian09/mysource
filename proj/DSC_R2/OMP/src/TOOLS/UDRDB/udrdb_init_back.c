#include "udrdb.h"

MYSQL	sql,	*conn;
static 	int		file_cnt;
extern	int		errno;

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
	char	system_name[16];

	start_year = start_month = 0;
	start_day = start_hour = start_minute = 0;
	end_day = end_hour = end_minute = 0;
	select_num = 0;
	memset(system_name, 0x00, sizeof(system_name));


START_MENU:
	printf("\n\n=============================================================\n");
	printf("======            UDR DB Insert/Search/Delete          ======\n");
	printf("=============================================================\n\n");
	printf("[1] Select DB Operation \n");
	printf("1. Insert UDR \n");
	printf("2. Search UDR \n");
	printf("3. Delete UDR \n");
	printf("4. Exit \n");

	printf("Input ===> ");
	scanf("%d", &select_num);

	printf("\n\n");

	if (select_num == 1)
	{
		printf("-----------------------------------------------------\n");
		printf("|                 INSERT PROCEDURE                  |\n");
		printf("-----------------------------------------------------\n\n");
		printf("[1] Input System Name [BSDA/BSDB] : ");
		scanf("%s", system_name);
		if (strcasecmp(system_name, "BSDA") != 0 && strcasecmp(system_name, "BSDB") != 0)
		{
			printf("You input wrong system name : %s\n", system_name);
			goto START_MENU;
		}
		printf("\n");

		printf("[2] INPUT INSERT YEAR [2006 ~] 	: ");
		scanf("%d", &start_year);
		if (start_year < 2006 || start_year > 2100)
		{
			printf("You input wrong year info... retry : %d\n", start_year);
			goto START_MENU;
		}
		printf("\n");

		printf("[3] INPUT INSERT MONTH [1~12] 	: ");
		scanf("%d", &start_month);
		if (start_month < 1 || start_month > 12)
		{
			printf("You input wrong month info... retry : %d\n", start_month);
			goto START_MENU;
		}
		printf("\n");

		printf("[4-1] Start Day [1~31]		: ");
		scanf("%d", &start_day);
		if (start_day < 1 || start_day > 31)
		{
			printf("You input wrong start-day info... retry : %d\n", start_day);
			goto START_MENU;
		}

		printf("[4-2] Start Hour [1~24]		: ");
		scanf("%d", &start_hour);
		if (start_hour < 1 || start_hour > 24)
		{
			printf("You input wrong start-hour info... retry : %d\n", start_hour);
			goto START_MENU;
		}

		printf("[4-3] Start Minute [1~60]	: ");
		scanf("%d", &start_minute);
		if (start_minute < 1 || start_minute > 60) 
		{
			printf("You input wrong start-minute info... retry : %d\n", start_minute);
			goto START_MENU;
		}
		printf("\n");


		printf("[5-1] End Day [1~31]		: ");
		scanf("%d", &end_day);
		if (end_day < 1 || end_day > 31)
		{
			printf("You input wrong end-day info... retry : %d\n", end_day);
			goto START_MENU;
		}

		printf("[5-2] End Hour [1~24]		: ");
		scanf("%d", &end_hour);
		if (end_hour < 1 || end_hour > 24)
		{
			printf("You input wrong end-hour info... retry : %d\n", end_hour);
			goto START_MENU;
		}

		printf("[5-3] End Minute [1~60]		: ");
		scanf("%d", &end_minute);
		if (end_minute < 1 || end_minute > 60) 
		{
			printf("You input wrong end-minute info... retry : %d\n", end_minute);
			goto START_MENU;
		}
		printf("\n");

		search_dir( system_name, start_year, start_month,
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

		printf("-----------------------------------------------------\n");
		printf("|                 SEARCH PROCEDURE                  |\n");
		printf("-----------------------------------------------------\n\n");
		printf("<1> Time-Based Search\n");
		printf("<2> User-Based Search\n");
		printf("<3> Direct Query (full-SQL syntax) \n\n");
		printf("Input Search Option ===> ");

		scanf("%d", &search_sel);
		printf("\n");
		if (search_sel == 1)
		{
			printf("[1] Input Search Year [2006 ~] : ");
			scanf("%d", &search_year);
			if(search_year < 2006 || search_year > 2100)
			{
				fprintf(stderr, "Input Wrong Search Year... retry\n\n");
				goto START_MENU;
			}
			printf("\n");

			printf("[2] Input Search Start time (월일시분초, e.g. 1230153212) : ");
			scanf("%d", &start_time);
			if (start_time < 101000000 || start_time > 1300000000)
			{
				fprintf(stderr, "Input Wrong Search Start Time... retry\n\n");
				goto START_MENU;
			}
			printf("\n");

			printf("[3] Input Search End time (월일시분초, e.g. 1230153212) : ");
			scanf("%d", &end_time);
			if (end_time < 101000000 || end_time > 1300000000)
			{
				fprintf(stderr, "Input Wrong Search End Time... retry\n\n");
				goto START_MENU;
			}
			printf("\n");

			printf("[4] Input Result File Name (full-path) : ");
			scanf("%s", file_name);
			printf("\n");

			if (search_time(search_year, start_time, end_time, file_name) < 0)
				goto START_MENU;
		}

		else if (search_sel == 2)
		{
			int		sub_search_sel;
			char	file_name[32], search_min[64], search_username[32];

			memset(file_name, 		0x00, sizeof(file_name));
			memset(search_min, 		0x00, sizeof(search_min));
			memset(search_username,	0x00, sizeof(search_username));

			printf("-----------------------------------------------------\n");
			printf("|             User-Based SEARCH PROCEDURE           |\n");
			printf("-----------------------------------------------------\n\n");
			printf("<1> Search for MIN\n"); 
			printf("<2> Search for User-Nama\n\n");
			printf("Input Sub-Search Option ===> ");
			scanf("%d", &sub_search_sel);
			printf("\n");

			if (sub_search_sel == 1)
			{
				printf("[1] Input MIN : ");
				scanf("%s", search_min);
				printf("\n");

				printf("[2] Input Result File Name (full-path) : ");
				scanf("%s", file_name);
				printf("\n");

				if (search_user(sub_search_sel, search_min, file_name) < 0)
					goto START_MENU;
			}

			else if (sub_search_sel == 2)
			{
				printf("[1] Input User-Name : ");
				scanf("%s", search_username);
				printf("\n");

				printf("[2] Input Result File Name (full-path) : ");
				scanf("%s", file_name);
				printf("\n");

				if (search_user(sub_search_sel, search_username, file_name) < 0)
					goto START_MENU;
			}

			else
			{
				printf("Input Wrong Sub-Search... retry\n\n");
				goto START_MENU;
			}

		}

		else if (search_sel == 3)
		{
			gets(file_name);
			printf("Input Result File Name (full-path) > ");
			gets(file_name);

			printf("sql> ");
			gets(direct_query);

			if (direct_search(file_name, direct_query) < 0)
				goto START_MENU;
		}

		else
		{
			printf("Input Wrong Number retry...\n\n");
			goto START_MENU;
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
search_dir( char *system_name, 
			int search_year,
			int search_month,
			int start_day,
			int start_hour,
			int start_minute,
			int end_day,
			int end_hour,
			int end_minute)
{

	char 			dir_name[64];
	DIR 			*dir_pt;
	struct dirent	*dir_info;
	int				search_day, search_hour;
	int				diff_day, i;

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

	if (diff_day == 0)
	{
		search_day = start_day;	
		sprintf(dir_name, "%s/%s/%02d/%02d/%02d", REF_DIR,
										  system_name,
										  search_year,
										  search_month,
										  search_day);


		dir_pt = opendir(dir_name);

		if (dir_pt == NULL)
		{
			fprintf(stderr, "[ERR] OPEN Directory Error : %s\n", strerror(errno));
			return (-1);
		}

		fprintf(stdout, "OPEN Directory Success : %s\n", dir_name);

		while((dir_info = readdir(dir_pt)) != NULL)
		{

			if (!strcasecmp(dir_info->d_name, ".") || !strcasecmp(dir_info->d_name, ".."))
				continue;

			/** find conditional file **/
			if (find_match_file(dir_info->d_name, start_hour, start_minute,
								end_hour, end_minute) < 0)
				continue;

			/** extract file and insert process and compress file **/
			udr_file_proc(dir_name, dir_info->d_name);

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
				udr_file_proc(dir_name, dir_info->d_name);

			}  /** end of while **/

			/** close dir **/
			closedir(dir_pt);

		} /** end of for **/

	} /** end of else **/

	printf("######## file count : %d\n", file_cnt);

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

	parsing_data = strstr(file_name, "2");

	memset(hour, 0x00, sizeof(hour));
	for (i=0; i<2; i++)
	{
		hour[i] = *(parsing_data+8+i);
	}
	hour_int = atoi(hour);

	if (hour_int >= start_hour && hour_int <= end_hour)
	{

		memset(minute, 0x00, sizeof(minute));
		for (i=0; i<2; i++)
		{
			minute[i] = *(parsing_data+10+i);
		}
		min_int = atoi(minute);

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

	memset(command, 0x00, sizeof(command));
	memset(full_path, 0x00, sizeof(full_path));

	sprintf(full_path, "%s/%s", dir_path, file_name);

	/** extract compress file **/
	sprintf(command, "gunzip %s", full_path);
	system(command);


	/** file parsing and insert db **/
	parse_insert_proc(full_path);

	/** compress file **/
	memset(command, 0x00, sizeof(command));
	memset(extract_file, 0x00, sizeof(extract_file));
	memcpy(extract_file, full_path, (strlen(full_path)-3));

	sprintf(command, "gzip %s", extract_file);
	system(command);

	return (0);
}


/**
 *		write title to file
 **/
write_title(FILE *fd)
{
	
	char 	temp_buf[4096];
 
	memset(temp_buf, 0x00, sizeof(temp_buf));

	sprintf(temp_buf, "acct-session-id,min,rad-code,rad-id,username-len,nasport-id-len,udr-sequence,time-stamp,aaa-ip,udr-key,framed-ip,nas-ip,pcf-ip,ha-ip,radius-len,sess-continue,begin-sess,svc-opt,acct-stat-type,comp-tunneled,num_act,svc-type,fwd-fch-mux,fwd-fch-mux,fwd-traf-type,rev-traf-type,fch-size,fwd-fchrc,rev-fchrc,ip-tech,dcch-size,nas-port,nas-port-type,release-ind,acct-in-oct,acct-out-oct,acct-in-pkt,acct-out-pkt,event-time,acct-time,acct-sess-time,acct-delay-time,term-sdb-oct-cnt,org-sdb-oct-cnt,term-num-sdb,org-num-sdb,rcv-hdlc-oct,ip-qos,air-qos,rp-connect-id,bad-ppp-frame-cnt,acct-auth,acct-term-cause,always-on,user-id,in-mip-sig-cnt,out-mip-sig-cnt,acct-interim,correl-id,retry-flag,authen,mdn,esn,user-name,bsid,nas-port-id,data-svc-type,tran-id,req-time,res-time,session-time,dest-ip,dest-port,src-port,c-type,app-id,content-code,method-type,result-code,ip-up-size,ip-down-size,retrans-in-size,retrnas-out-size,content-len,tran-complete,tran-term-reason,url,user-agent,host,mdn,url-cha\n");

	fputs(temp_buf, fd);

	return (1);
}
