#include "cdrlog.h"

static 	int		file_cnt;
extern	int		errno;
extern	int		cdrlog_cnt;
int				total_cdr_cnt;
int				dup_cnt;


/**
 *
 **/
main()
{
	FILE 	*fp;

	load_config_data();

	if (cdrlog_proc() < 0)
	{
		fprintf(stdout, "CDR LOG VIEWER ERROR...QUIT");
		exit(1);
	}

}


/**
 *
 **/
cdrlog_proc()
{
	
	int		start_year, start_month;
	int		start_day, start_hour, start_minute;
	int		end_year, end_month, end_day, end_hour, end_minute;
	int		select_num, del_sel;
	int		is_min_exist, offset, i;
	int		exist_ip, exist_port1, exist_port2;
	int		port1, port2;
	char	ip [16];
	char	start_time[16], end_time[16];
	char	search_min[16],file_name[64];
	char	start_year_str[5],start_month_str[4],start_day_str[4];
	char	start_hour_str[5],start_minute_str[4];
	char	end_year_str[5],end_month_str[4],end_day_str[4];
	char	end_hour_str[5],end_minute_str[4];

	start_year = start_month = 0;
	start_day = start_hour = start_minute = 0;
	end_day = end_hour = end_minute = 0;
	select_num = 0;
	total_cdr_cnt = dup_cnt = 0;
	exist_ip = exist_port1 = exist_port2 = 0;

	memset(search_min, 	0x00, sizeof(search_min));
	memset(file_name, 	0x00, sizeof(file_name));
	memset(ip,			0x00, sizeof(ip));


START_MENU:
	printf("\n\n====================================================\n");
	printf("======             CDR LOG VIEWER             ======\n");
	printf("====================================================\n\n");
	printf("1. SEARCH CDR INFO (TIME & IMSI SEARCH)\n");
	printf("2. SEARCH CDR INFO (TIME)\n");
	printf("3. SEARCH CDR INFO (TIME & SKIP DUPLICATION(Service-Option/Dest-IP/Dest-Port) SEARCH))\n");
	printf("4. SEARCH CDR INFO (TIME & IP)\n");
	printf("5. SEARCH CDR INFO (TIME & Port)\n");
	printf("6. Exit \n");

	printf("Input ===> ");
	scanf("%d", &select_num);

	printf("\n\n");

	if (select_num == 1 || select_num == 2 || select_num == 3 || select_num == 4 || select_num == 5)
	{
		offset = 0;
		memset(start_time, 	0x00, sizeof(start_time));
		memset(end_time, 	0x00, sizeof(end_time));

		printf("[1] INPUT START TIME [YYYYMMDDhhmm, (e.g. 200612150810)]  : ");
		scanf("%s", &start_time);

		memset(start_year_str, 0x00, sizeof(start_year_str));
		for(i=0; i<4; i++)
		{
			 start_year_str[i] = start_time[i];
		}
		offset += i;
		start_year = atoi(start_year_str);

		if (start_year < 2006 || start_year > 2100)
		{
			printf("You input wrong year info... retry : %d\n", start_year);
			goto START_MENU;
		}

		memset(start_month_str,	0x00, sizeof(start_month_str));
		for(i=0; i<2; i++)
		{
			 start_month_str[i] = start_time[offset+i];
		}
		offset += i;
		start_month = atoi(start_month_str);

		if (start_month < 1 || start_month > 12)
		{
			printf("You input wrong month info... retry : %d\n", start_month);
			goto START_MENU;
		}

		memset(start_day_str, 0x00, sizeof(start_day_str));
		for(i=0; i<2; i++)
		{
			 start_day_str[i] = start_time[offset+i];
		}
		offset += i;
		start_day = atoi(start_day_str);

		if (start_day < 1 || start_day > 31)
		{
			printf("You input wrong start-day info... retry : %d\n", start_day);
			goto START_MENU;
		}

		memset(start_hour_str, 0x00, sizeof(start_hour_str));
		for(i=0; i<2; i++)
		{
			 start_hour_str[i] = start_time[offset+i];
		}
		offset += i;
		start_hour = atoi(start_hour_str);

		if (start_hour < 0 || start_hour > 24)
		{
			printf("You input wrong start-hour info... retry : %d\n", start_hour);
			goto START_MENU;
		}

		memset(start_minute_str, 0x00, sizeof(start_minute_str));
		for(i=0; i<2; i++)
		{
			 start_minute_str[i] = start_time[offset+i];
		}
		offset += i;
		start_minute = atoi(start_minute_str);

		if (start_minute < 0 || start_minute > 60) 
		{
			printf("You input wrong start-minute info... retry : %d\n", start_minute);
			goto START_MENU;
		}

		printf("[2] INPUT END TIME [YYYYMMDDhhmm, (e.g. 200612150930)]  : ");
		scanf("%s", &end_time);
		offset = 0;

		memset(end_year_str, 0x00, sizeof(end_year_str));
		for(i=0; i<4; i++)
		{
			 end_year_str[i] = end_time[offset+i];
		}
		offset += i;
		end_year = atoi(end_year_str);

		if (end_year < 2006 || end_year > 2100)
		{
			printf("You input wrong year info... retry : %d\n", end_year);
			goto START_MENU;
		}

		memset(end_month_str, 0x00, sizeof(end_month_str));
		for(i=0; i<2; i++)
		{
			 end_month_str[i] = end_time[offset+i];
		}
		offset += i;
		end_month = atoi(end_month_str);

		if (end_month < 1 || end_month > 12)
		{
			printf("You input wrong month info... retry : %d\n", end_month);
			goto START_MENU;
		}

		memset(end_day_str, 0x00, sizeof(end_day_str));
		for(i=0; i<2; i++)
		{
			 end_day_str[i] = end_time[offset+i];
		}
		offset += i;
		end_day = atoi(end_day_str);

		if (end_day < 1 || end_day > 31)
		{
			printf("You input wrong end-day info... retry : %d\n", end_day);
			goto START_MENU;
		}

		memset(end_hour_str, 0x00, sizeof(end_hour_str));
		for(i=0; i<2; i++)
		{
			 end_hour_str[i] = end_time[offset+i];
		}
		offset += i;
		end_hour = atoi(end_hour_str);

		if (end_hour < 0 || end_hour > 24)
		{
			printf("You input wrong end-hour info... retry : %d\n", end_hour);
			goto START_MENU;
		}

		memset(end_minute_str, 0x00, sizeof(end_minute_str));
		for(i=0; i<2; i++)
		{
			 end_minute_str[i] = end_time[offset+i];
		}
		offset += i;
		end_minute = atoi(end_minute_str);

		if (end_minute < 0 || end_minute > 60) 
		{
			printf("You input wrong end-minute info... retry : %d\n", end_minute);
			goto START_MENU;
		}
		printf("\n");

		if (select_num == 1)
		{
			printf("[3] INPUT IMSI INFO : ");
			scanf("%s", search_min);
			is_min_exist = 1;

			//printf("\n");

			//printf("[6] RESULT FIlE NAME : ");
			sprintf(file_name, "%d%02d%02d%02d%02d_%d%02d%02d%02d%02d_CDRLOG",
									 start_year,
									 start_month,
									 start_day,
									 start_hour,
									 start_minute,
									 end_year,
									 end_month,
									 end_day,
									 end_hour,
									 end_minute);
			printf("\n\n");
		}

		else if (select_num == 2)
		{
			sprintf(file_name, "%d%02d%02d%02d%02d_%d%02d%02d%02d%02d_CDRLOG",
									 start_year,
									 start_month,
									 start_day,
									 start_hour,
									 start_minute,
									 end_year,
									 end_month,
									 end_day,
									 end_hour,
									 end_minute);
			is_min_exist = 2;
			printf("\n\n");
		}

		else if (select_num == 3)
		{
			sprintf(file_name, "%d%02d%02d%02d%02d_%d%02d%02d%02d%02d_CDRLOG",
									 start_year,
									 start_month,
									 start_day,
									 start_hour,
									 start_minute,
									 end_year,
									 end_month,
									 end_day,
									 end_hour,
									 end_minute);
			is_min_exist = 3;
			printf("\n\n");
		}

		if (select_num == 4)
		{
			printf("[3] INPUT IP INFO : ");
			scanf("%s", ip);
			exist_ip = 1;
			is_min_exist = 0;

			//printf("\n");

			//printf("[6] RESULT FIlE NAME : ");
			sprintf(file_name, "%d%02d%02d%02d%02d_%d%02d%02d%02d%02d_CDRLOG",
									 start_year,
									 start_month,
									 start_day,
									 start_hour,
									 start_minute,
									 end_year,
									 end_month,
									 end_day,
									 end_hour,
									 end_minute);
			printf("\n\n");
		}

		if (select_num == 5)
		{
			printf("[3] INPUT PORT1 INFO (0:ignore or Port no) : ");
			scanf("%d", &port1);
			if (port1 != 0) exist_port1 = 1;

			printf("[3] INPUT PORT2 INFO (0:ignore or Port no) : ");
			scanf("%d", &port2);
			if (port2 != 0) exist_port2 = 1;

			is_min_exist = 0;

			//printf("\n");

			//printf("[6] RESULT FIlE NAME : ");
			sprintf(file_name, "%d%02d%02d%02d%02d_%d%02d%02d%02d%02d_CDRLOG",
									 start_year,
									 start_month,
									 start_day,
									 start_hour,
									 start_minute,
									 end_year,
									 end_month,
									 end_day,
									 end_hour,
									 end_minute);
			printf("\n\n");
		}

		search_dir( start_year, start_month, start_day,
					start_hour, start_minute,
					end_year, end_month, end_day,
					end_hour, end_minute, file_name,
					is_min_exist, search_min,
					exist_ip, ip,
					exist_port1, port1, exist_port2, port2);
	}


	else if (select_num == 6)
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
search_dir( int start_year,
			int start_month,
			int start_day,
			int start_hour,
			int start_minute,
			int	end_year,
			int	end_month,
			int end_day,
			int end_hour,
			int end_minute,
			char *write_file,
			int exist_min,
			char *min,
			int exist_ip,
			char *ip,
			int exist_port1,
			int port1,
			int exist_port2,
			int port2)
{

	char 			dir_name[64];
	DIR 			*dir_pt;
	struct dirent	*dir_info;
	int				search_month, search_day, search_hour, search_minute;
	int				diff_day, diff_month, i, j;
	char			system_name[8];
	FILE 			*write_fd=(FILE *) NULL;

	/** initialize variables **/
	dir_pt = NULL;
	search_month = search_day = search_hour = 0;
	diff_day = 0;
	file_cnt = 0;

	cdrlog_cnt = 0;

	write_fd = fopen(write_file, "w");
	if (!write_fd)
	{
		fprintf(stderr, "FILE OPEN ERROR : %s [%s]", strerror(errno), write_file);
		return (-1);
	}

	/** write file head **/
	write_title(write_fd);
	if (start_year != end_year)
	{
		fprintf(stderr, "You input wrong year [start year : %d][end year : %d]\n",
								start_year, end_year);
		fprintf(stderr, "PLEASE INPUT SAME YEAR .....\n");
		return (-1);
	}

	for(j=0; j<2; j++)
	{
		if (j == 0)
			strcpy(system_name, "DSCA");
		else
			strcpy(system_name, "DSCB");

		diff_month = end_month - start_month;
		if (diff_month == 0) 
		{
			if (start_day > end_day)
			{
				fprintf(stderr, "You input wrong month [start day : %d][end day : %d]\n",
												        start_day, end_day);
				return (-1);
			}

			diff_day = end_day - start_day;

			if (diff_day == 0)
			{
				/** find matched-directory and file **/
				find_dir_file_proc(system_name,
								   start_year,
								   start_month,
								   start_day,
								   start_hour,
								   start_minute,
								   end_hour,
								   end_minute,
								   write_fd,
								   exist_min,
								   min,
								   exist_ip,
								   ip,
								   exist_port1,
								   port1,
								   exist_port2,
								   port2);
			}
			else
			{
				/** loop for start day ~ end day **/
				for (i = 0; i <= diff_day; i++)
				{
					search_day = start_day + i;

					if (i == 0)
					{
						search_hour 	= 24;
						search_minute 	= 60;
					}
					else
					{
						start_hour 		= 1;
						start_minute 	= 1;
						if (i == diff_day)
						{
							search_hour 	= end_hour;
							search_minute 	= end_minute;
						}
					}
					find_dir_file_proc(system_name,
								   	   start_year,
								       start_month,
								       search_day,
								       start_hour,
								       start_minute,
								       search_hour,
								       search_minute,
									   write_fd,
									   exist_min,
									   min,
									   exist_ip,
									   ip,
									   exist_port1,
									   port1,
									   exist_port2,
									   port2);
				} /* enf of for */

			}  /* end of else */

		}  /* enf of if (diff_month == 0) */

		else /** diff_month != 0 **/
		{
			for (i = 0; i <= diff_month; i++)
			{
				search_month = start_month + i;

				if (i == 0)
				{
					for(j = 0; j <= get_day_per_month(search_month) - start_day; j++)
					{
						search_day 	= start_day + j;
						if (j == 0)
						{
							search_hour		= 24;
							search_minute	= 60;
						}
						else
						{
							start_hour		= 1;
							start_minute	= 1;
							if (j == get_day_per_month(search_month) - start_day)
							{
								search_hour 	= end_hour;
								search_minute 	= end_minute;
							}
						}

						find_dir_file_proc(system_name,
									   	   start_year,
									       search_month,
									       search_day,
									       start_hour,
									       start_minute,
									       search_hour,
									       search_minute,
										   write_fd,
										   exist_min,
										   min,
										   exist_ip,
										   ip,
										   exist_port1,
										   port1,
										   exist_port2,
										   port2);

				    } /** enf of for () **/

				}
				else if (0 < i && i < diff_month) 
				{
					for(j = 0; j <= get_day_per_month(search_month); j++)
					{
						search_day 		= start_day + j;
						start_hour		= 1;
						start_minute	= 1;
						search_hour		= 24;
						search_minute	= 60;

						find_dir_file_proc(system_name,
									   	   start_year,
									       search_month,
									       search_day,
									       start_hour,
									       start_minute,
									       search_hour,
									       search_minute,
										   write_fd,
										   exist_min,
										   min,
										   exist_ip,
										   ip,
										   exist_port1,
										   port1,
										   exist_port2,
										   port2);

				    } /** enf of for () **/
				}
				else  /** i == diff_month **/
				{
					for(j = 0; j <= end_day; j++)
					{
						search_day = start_day + j;

						start_hour 		= 1;
						start_minute 	= 1;
						search_hour		= 24;
						search_minute	= 60;

						if (j == end_day)
						{
							search_hour		= end_hour;
							search_minute	= end_minute;
						}

						find_dir_file_proc(system_name,
										   start_year,
										   search_month,
										   search_day,
										   start_hour,
										   start_minute,
										   search_hour,
										   search_minute,
										   write_fd,
										   exist_min,
										   min,
										   exist_ip,
										   ip,
										   exist_port1,
										   port1,
										   exist_port2,
										   port2);
					}  /** end of for **/

				} /** end of else **/

			} /** end of for **/

		} /* end of else (diff_month != 0) */

	}  /** end of for(i=0; i<2; i++) **/

	if (total_cdr_cnt != 0)
	{
		printf("\n############################################################\n");
		printf("##   FILE COUNT       : %d\n", file_cnt);
		printf("##   TOTAL CDR COUNT  : %d\n", total_cdr_cnt);
		printf("##   DUP COUNT        : %d\n", dup_cnt);
		printf("##   OUTPUT FILE NAME : %s\n", write_file);
		printf("############################################################\n");
	}
	else
	{
		printf("\n############################################################\n");
		printf("##   FILE COUNT       : %d\n", file_cnt);
		printf("##   TOTAL CDR COUNT  : %d\n", total_cdr_cnt);
		printf("############################################################\n");
		remove(write_file);
	}

	fclose(write_fd);

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

	//parsing_data = strstr(file_name, "2");
	parsing_data = &(file_name[8]);

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
cdr_file_proc(char *dir_path,
			  char *file_name,
			  FILE *write_fd,
			  int  is_exist_min,
			  char *min,
			  int  exist_ip,
			  char *ip,
			  int  exist_port1,
			  int  port1,
			  int  exist_port2,
			  int  port2)
{

	char 	command[256];	
	char 	full_path[256];	
	char 	extract_file[256];	
	int		file_format;

	file_format = 0;
	memset(full_path, 	0x00, sizeof(full_path));
	sprintf(full_path, "%s/%s", dir_path, file_name);

	if (strlen(file_name) == 36)
	{
		file_format = UNCOMPRESS_FILE;
		parse_write_proc(full_path, write_fd, file_format);
	}
	else if (strlen(file_name) == 39)
	{
		/** extract compress file **/
		memset(command, 	0x00, sizeof(command));
		sprintf(command, "gunzip %s", full_path);
		system(command);

		/** file parsing and insert db **/
		file_format = COMPRESS_FILE;
		parse_write_proc(full_path, write_fd, file_format, is_exist_min, min, exist_ip, ip, exist_port1, port1, exist_port2, port2);

		/** compress file **/
		memset(command, 0x00, sizeof(command));
		memset(extract_file, 0x00, sizeof(extract_file));
		memcpy(extract_file, full_path, (strlen(full_path)-3));

		sprintf(command, "gzip %s", extract_file);
		system(command);
	}
	else
		return (1);

	return (0);
}


/**
 *		write title to file
 **/
static	char 	temp_buf[1024];
write_title(FILE *fd)
{
	
 
	memset(temp_buf, 0x00, sizeof(temp_buf));
	sprintf(temp_buf, "IMSI,ACCT-SESSSION-ID,SOURCE-IP-ADDRESS,SERVICE-OPTION,CREATE-TIME,LAST-TIME,SOURCE-PORT,DESTINATION-IP-ADDRESS,DESTINATION-PORT,SERVICE-TYPE,IP-LAYER-UPLOAD-FRAMES,IP-LAYER-DOWNLOAD-FRAMES,IP-LAYER-UPLOAD-BYTES,IP-LAYER-DOWNLOAD-BYTES,TCP-RETRANS-UPLOAD-FRAMES,TCP-RETRANS-DOWNLOAD-FRAMES,TCP-RETRNAS-UPLOAD-BYTES,TCP-RETRANS-DOWNLOAD-BYTES\n");

	fputs(temp_buf, fd);

	return (1);
}
