#include "cdrlog.h"

int add_dest_port(unsigned short);

extern		int			errno;
static		int			print_flag;
extern		int			total_cdr_cnt;
extern		int			dup_cnt;
int			cdrlog_cnt;

typedef struct _SVC_LIST
{
	int					svc_opt;
	struct _SVC_LIST	*next_svc_opt;
} st_svc_opt_list;
typedef st_svc_opt_list *pst_svc_opt_list;

typedef struct _IP_LIST
{
	unsigned int		dest_ip;
	struct _IP_LIST		*next_dest_ip;
} st_ip_list;
typedef st_ip_list *pst_ip_list;

typedef struct _PORT_LIST
{
	unsigned short		dest_port;
	struct _PORT_LIST	*next_dest_port;
} st_port_list;
typedef st_port_list *pst_port_list;

pst_svc_opt_list        pst_svc_opt_list_head;
pst_svc_opt_list        pst_svc_opt_list_tail;
pst_ip_list             pst_ip_list_head;
pst_ip_list             pst_ip_list_tail;
pst_port_list           pst_port_list_head;
pst_port_list           pst_port_list_tail;

int check_dup_value(int svc_opt, unsigned int dest_ip, unsigned short dest_port);
int search_dest_port(pst_port_list search_list, unsigned short dest_port);
/**
 *
 */
int
find_dir_file_proc(char	*system_name, 
				   int	search_year,
				   int 	search_month,
				   int 	search_day,
				   int	start_hour,
				   int	start_min,
				   int	end_hour,
				   int	end_min,
				   int	write_fd,
				   int	exist_min,
				   char *min,
				   int	exist_ip,
				   char *ip,
				   int	exist_port1,
				   int	port1,
				   int	exist_port2,
				   int	port2)
{

	char	dir_name[128];
	DIR		*dir_pt;
	struct	dirent	*dir_info;

	memset(dir_name, 0x00, sizeof(dir_name));
	
	sprintf(dir_name, "%s/%s/%02d/%02d/%02d", DUMP_FILE_PATH,
						  					system_name,
											search_year,
										    search_month,
											search_day);


	dir_pt = opendir(dir_name);

	if (dir_pt == NULL)
	{
		fprintf(stderr, "[ERR] OPEN Directory Error : %s [%s]\n",
						strerror(errno), dir_name);
		return (-1);
	}

	fprintf(stdout, "OPEN Directory Success : %s\n", dir_name);

	while((dir_info = readdir(dir_pt)) != NULL)
	{

		if (!strcasecmp(dir_info->d_name, ".") || !strcasecmp(dir_info->d_name, ".."))
			continue;

		/** find conditional file **/
		if (find_match_file(dir_info->d_name, start_hour, start_min,
											  end_hour, end_min) < 0)
			continue;

		/** extract file and insert process and compress file **/
		if (cdr_file_proc(dir_name, dir_info->d_name, write_fd, exist_min, min, exist_ip, ip, exist_port1, port1, exist_port2, port2) < 0)
			fprintf(stderr, "[ERR] FILE NAME FORMAT WRONG...\n");

	} /* end of while */

}


/**
 *		CDR LOG FILE parsing and write to file..
 **/ 
int parse_write_proc(char *read_file,
					 FILE *write_fd,
					 int file_format,
					 int min_flag,
					 char *search_min,
					 int ip_flag,
					 char *ip,
					 int port1_flag,
					 int port1,
					 int port2_flag,
					 int port2)
{

	int					i;
	int					year_int, time_int;
	long long			temp_long;
	char 				temp_file[256];
	char				year_data[4], time_data[10];
	char				*date_str;
	char				acct_sess_id[16];
	char				buffer[2048];
	char				src_ip[16], dest_ip[16];
	char				create_time[64], end_time[64];
	time_t				temp_cvt_crt_time, temp_cvt_last_time;
	time_t				cvt_crt_time, cvt_last_time;
	FILE				*fd;
	struct	tm			*crt_time, *last_time;
	st_CDRSessLog		cdrlog_info;
	st_CDRDumpInfo		cdrlog_head;
	int 			reg_cnt;


	/** initialize variables **/
	reg_cnt = 0;
	memset(&cdrlog_head, 	0x00, sizeof(st_CDRDumpInfo));
	memset(temp_file, 	0x00, sizeof(temp_file));
	memset(year_data, 	0x00, sizeof(year_data));
	memset(time_data, 	0x00, sizeof(time_data));

	if (file_format == COMPRESS_FILE)
		memcpy(temp_file, read_file, (strlen(read_file)-3));
	else
		memcpy(temp_file, read_file, strlen(read_file));

	date_str = strstr(temp_file, "2");
	//date_str = strstr(date_str, "2");

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
	if ((fread(&cdrlog_head, sizeof(st_CDRDumpInfo), 1, fd)) <= 0)
	{
		fprintf(stderr, "FILE READ ERROR : %s [FILE : %s]\n",
						strerror(errno), temp_file);	
	}

	/** print head info **/
	print_flag = 0;

	/** read CDR Sess Log data **/
	while(1)
	{

		memset(&cdrlog_info, 	0x00, sizeof(st_CDRSessLog));

		if ((fread(&cdrlog_info, sizeof(st_CDRSessLog), 1, fd)) <= 0)
		{
			//fprintf(stderr, "END READ FILE : %s\n", strerror(errno));
			break;
		}

		print_flag++;

		memset(acct_sess_id, 		0x00, sizeof(acct_sess_id));

		CVT_INT64_CP(&temp_long, cdrlog_info.llAcctSessID); 
		//sprintf(acct_sess_id, "%lld", temp_long);
		conv_id(temp_long, acct_sess_id);

		/** print sample **/
		/*
		if (print_flag == 1)
		{
			printf("############ PRINT CDR LOG DATA ###############\n\n");

			printf("IMSI 				: %s\n", 	cdrlog_info.szIMSI);
			printf("Acct Session ID 		: %s\n", 	acct_sess_id);
			printf("Source-IP-Address		: %s\n", 	
							cvt_ipaddr(CVT_INT_CP(cdrlog_info.uiSrcIP)));
			printf("Service-Option			: %d\n", CVT_INT_CP(cdrlog_info.dSvcOpt));
			printf("Create-Time			: %d\n", CVT_INT_CP(cdrlog_info.tCreateTime));
			printf("Last-Time			: %d\n", CVT_INT_CP(cdrlog_info.tLastTime));
			printf("Source-Port			: %d\n", cdrlog_info.usSecPort);
			printf("Destination-IP-Address		: %s\n", 
							cvt_ipaddr(CVT_INT_CP(cdrlog_info.uiDestIP)));
			printf("Destination-Port		: %d\n", cdrlog_info.usDestPort);
			printf("Service-Type			: %d\n", CVT_INT_CP(cdrlog_info.dSvcType));
			printf("IP-Up-Frame			: %d\n", CVT_INT_CP(cdrlog_info.uiUPIPFrames));
			printf("IP-Down-Frame			: %d\n", CVT_INT_CP(cdrlog_info.uiDownIPFrames));
			printf("IP-Up-Byte			: %d\n", CVT_INT_CP(cdrlog_info.uiUPIPBytes));
			printf("IP-Down-Byte			: %d\n", CVT_INT_CP(cdrlog_info.uiDownIPBytes));
			printf("ReTx-Up-Frame			: %d\n", CVT_INT_CP(cdrlog_info.uiUPTCPREFrames));
			printf("ReTx-Down-Frame			: %d\n", CVT_INT_CP(cdrlog_info.uiDownTCPREFrames));
			printf("ReTx-Up-Byte			: %d\n", CVT_INT_CP(cdrlog_info.uiUPTCPREBytes));
			printf("ReTx-Down-Byte			: %d\n", CVT_INT_CP(cdrlog_info.uiDownTCPREBytes));

			printf("################################################\n\n");
		}
		*/

		memset(src_ip, 		0x00, sizeof(src_ip));
		memset(dest_ip,		0x00, sizeof(dest_ip));
		memset(create_time,	0x00, sizeof(create_time));
		memset(end_time,	0x00, sizeof(end_time));

		strcpy(src_ip, (char *)cvt_ipaddr(CVT_INT_CP(cdrlog_info.uiSrcIP)));
		strcpy(dest_ip, (char *)cvt_ipaddr(CVT_INT_CP(cdrlog_info.uiDestIP)));

		CVT_INT_CP_CDR(cdrlog_info.tCreateTime, (int *)&cvt_crt_time);
		CVT_INT_CP_CDR(cdrlog_info.tLastTime, (int *)&cvt_last_time);

/*
		printf("=======>  cvt_crt_time : %d, %p\n", cvt_crt_time, &cvt_crt_time);
		printf("=======>  cvt_last_time : %d, %p\n", cvt_last_time, &cvt_last_time);
*/

		crt_time 	= localtime(&cvt_crt_time);
		sprintf(create_time, "%d-%02d-%02d %02d:%02d:%02d",
								crt_time->tm_year+1900,
								crt_time->tm_mon+1,
								crt_time->tm_mday,
								crt_time->tm_hour,
								crt_time->tm_min,
								crt_time->tm_sec);

		last_time 	= localtime(&cvt_last_time);
		sprintf(end_time, "%d-%02d-%02d %02d:%02d:%02d",
								last_time->tm_year+1900,
								last_time->tm_mon+1,
								last_time->tm_mday,
								last_time->tm_hour,
								last_time->tm_min,
								last_time->tm_sec);

		memset(buffer, 0x00, sizeof(buffer));


		if (check_register(dest_ip, cdrlog_info.usDestPort) < 0)
		{
			reg_cnt++;
			continue;
		}

		if (min_flag == 1)
		{
			if (!strcmp(search_min, cdrlog_info.szIMSI))
			{
				sprintf(buffer, "%s,0x%s,%s,%d,%s,%s,%u,%s,%u,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",	// 070913,poopee, ushort for port
					cdrlog_info.szIMSI,
					acct_sess_id,
					src_ip,
					CVT_INT_CP(cdrlog_info.dSvcOpt),
					create_time,
					end_time,
					CVT_SHORT_CP(cdrlog_info.usSecPort) & 0x0000FFFF,	// 070913,poopee
					dest_ip,	
					CVT_SHORT_CP(cdrlog_info.usDestPort) & 0x0000FFFF,	// 070913,poopee
					CVT_INT_CP(cdrlog_info.dSvcType),
					CVT_INT_CP(cdrlog_info.uiUPIPFrames),
					CVT_INT_CP(cdrlog_info.uiDownIPFrames),
					CVT_INT_CP(cdrlog_info.uiUPIPBytes),
					CVT_INT_CP(cdrlog_info.uiDownIPBytes),
					CVT_INT_CP(cdrlog_info.uiUPTCPREFrames),
					CVT_INT_CP(cdrlog_info.uiDownTCPREFrames),
					CVT_INT_CP(cdrlog_info.uiUPTCPREBytes),
					CVT_INT_CP(cdrlog_info.uiDownTCPREBytes));

				fputs(buffer, write_fd);
				cdrlog_cnt++;
			}
		}
		else if (min_flag == 2)
		{
			sprintf(buffer, "%s,0x%s,%s,%d,%s,%s,%d,%s,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
					cdrlog_info.szIMSI,
					acct_sess_id,
					src_ip,
					CVT_INT_CP(cdrlog_info.dSvcOpt),
					create_time,
					end_time,
					CVT_SHORT_CP(cdrlog_info.usSecPort) & 0x0000FFFF,	// 070913,poopee
					dest_ip,	
					CVT_SHORT_CP(cdrlog_info.usDestPort) & 0x0000FFFF,	// 070913,poopee
					CVT_INT_CP(cdrlog_info.dSvcType),
					CVT_INT_CP(cdrlog_info.uiUPIPFrames),
					CVT_INT_CP(cdrlog_info.uiDownIPFrames),
					CVT_INT_CP(cdrlog_info.uiUPIPBytes),
					CVT_INT_CP(cdrlog_info.uiDownIPBytes),
					CVT_INT_CP(cdrlog_info.uiUPTCPREFrames),
					CVT_INT_CP(cdrlog_info.uiDownTCPREFrames),
					CVT_INT_CP(cdrlog_info.uiUPTCPREBytes),
					CVT_INT_CP(cdrlog_info.uiDownTCPREBytes));

			fputs(buffer, write_fd);
			cdrlog_cnt++;
		}
		else if (min_flag == 3)
		{
			int 	ret_code;
			
			ret_code = check_dup_value( cdrlog_info.dSvcOpt, 
										cdrlog_info.uiDestIP,
										cdrlog_info.usDestPort);
			if(ret_code < 0)
			{
				dup_cnt++;
				continue;
			}
			else if (ret_code == 2)
			{
				add_dest_port(cdrlog_info.usDestPort);
			}
			else if (ret_code == 3)
			{
				add_dest_port(cdrlog_info.usDestPort);
				add_dest_ip(cdrlog_info.uiDestIP);
			}
			else if (ret_code == 4)
			{
				add_dest_port(cdrlog_info.usDestPort);
				add_dest_ip(cdrlog_info.uiDestIP);
				add_service_opt(cdrlog_info.dSvcOpt);
			}

			sprintf(buffer, "%s,0x%s,%s,%d,%s,%s,%d,%s,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
					cdrlog_info.szIMSI,
					acct_sess_id,
					src_ip,
					CVT_INT_CP(cdrlog_info.dSvcOpt),
					create_time,
					end_time,
					CVT_SHORT_CP(cdrlog_info.usSecPort) & 0x0000FFFF,	// 070913,poopee
					dest_ip,	
					CVT_SHORT_CP(cdrlog_info.usDestPort) & 0x0000FFFF,	// 070913,poopee
					CVT_INT_CP(cdrlog_info.dSvcType),
					CVT_INT_CP(cdrlog_info.uiUPIPFrames),
					CVT_INT_CP(cdrlog_info.uiDownIPFrames),
					CVT_INT_CP(cdrlog_info.uiUPIPBytes),
					CVT_INT_CP(cdrlog_info.uiDownIPBytes),
					CVT_INT_CP(cdrlog_info.uiUPTCPREFrames),
					CVT_INT_CP(cdrlog_info.uiDownTCPREFrames),
					CVT_INT_CP(cdrlog_info.uiUPTCPREBytes),
					CVT_INT_CP(cdrlog_info.uiDownTCPREBytes));
			
			fputs(buffer, write_fd);
			cdrlog_cnt++;
		}
		else 
		{
			if (ip_flag == 1)
			{
				if (!strcmp(ip, src_ip) || !strcmp (ip, dest_ip))
				{
					sprintf(buffer, "%s,0x%s,%s,%d,%s,%s,%u,%s,%u,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",	// 070913,poopee, ushort for port
						cdrlog_info.szIMSI,
						acct_sess_id,
						src_ip,
						CVT_INT_CP(cdrlog_info.dSvcOpt),
						create_time,
						end_time,
						CVT_SHORT_CP(cdrlog_info.usSecPort) & 0x0000FFFF,	// 070913,poopee
						dest_ip,	
						CVT_SHORT_CP(cdrlog_info.usDestPort) & 0x0000FFFF,	// 070913,poopee
						CVT_INT_CP(cdrlog_info.dSvcType),
						CVT_INT_CP(cdrlog_info.uiUPIPFrames),
						CVT_INT_CP(cdrlog_info.uiDownIPFrames),
						CVT_INT_CP(cdrlog_info.uiUPIPBytes),
						CVT_INT_CP(cdrlog_info.uiDownIPBytes),
						CVT_INT_CP(cdrlog_info.uiUPTCPREFrames),
						CVT_INT_CP(cdrlog_info.uiDownTCPREFrames),
						CVT_INT_CP(cdrlog_info.uiUPTCPREBytes),
						CVT_INT_CP(cdrlog_info.uiDownTCPREBytes));

					fputs(buffer, write_fd);
					cdrlog_cnt++;
				}
			}
			if (port1_flag == 1)
			{
				if (port1 == CVT_SHORT_CP(cdrlog_info.usSecPort) & 0x0000FFFF || port1 == CVT_SHORT_CP(cdrlog_info.usDestPort) & 0x0000FFFF)
				{
					sprintf(buffer, "%s,0x%s,%s,%d,%s,%s,%u,%s,%u,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",	// 070913,poopee, ushort for port
						cdrlog_info.szIMSI,
						acct_sess_id,
						src_ip,
						CVT_INT_CP(cdrlog_info.dSvcOpt),
						create_time,
						end_time,
						CVT_SHORT_CP(cdrlog_info.usSecPort) & 0x0000FFFF,	// 070913,poopee
						dest_ip,	
						CVT_SHORT_CP(cdrlog_info.usDestPort) & 0x0000FFFF,	// 070913,poopee
						CVT_INT_CP(cdrlog_info.dSvcType),
						CVT_INT_CP(cdrlog_info.uiUPIPFrames),
						CVT_INT_CP(cdrlog_info.uiDownIPFrames),
						CVT_INT_CP(cdrlog_info.uiUPIPBytes),
						CVT_INT_CP(cdrlog_info.uiDownIPBytes),
						CVT_INT_CP(cdrlog_info.uiUPTCPREFrames),
						CVT_INT_CP(cdrlog_info.uiDownTCPREFrames),
						CVT_INT_CP(cdrlog_info.uiUPTCPREBytes),
						CVT_INT_CP(cdrlog_info.uiDownTCPREBytes));

					fputs(buffer, write_fd);
					cdrlog_cnt++;
				}
			}
			if (port2_flag == 1)
			{
				if (port2 == CVT_SHORT_CP(cdrlog_info.usSecPort) & 0x0000FFFF || port2 == CVT_SHORT_CP(cdrlog_info.usDestPort) & 0x0000FFFF)
				{
					sprintf(buffer, "%s,0x%s,%s,%d,%s,%s,%u,%s,%u,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",	// 070913,poopee, ushort for port
						cdrlog_info.szIMSI,
						acct_sess_id,
						src_ip,
						CVT_INT_CP(cdrlog_info.dSvcOpt),
						create_time,
						end_time,
						CVT_SHORT_CP(cdrlog_info.usSecPort) & 0x0000FFFF,	// 070913,poopee
						dest_ip,	
						CVT_SHORT_CP(cdrlog_info.usDestPort) & 0x0000FFFF,	// 070913,poopee
						CVT_INT_CP(cdrlog_info.dSvcType),
						CVT_INT_CP(cdrlog_info.uiUPIPFrames),
						CVT_INT_CP(cdrlog_info.uiDownIPFrames),
						CVT_INT_CP(cdrlog_info.uiUPIPBytes),
						CVT_INT_CP(cdrlog_info.uiDownIPBytes),
						CVT_INT_CP(cdrlog_info.uiUPTCPREFrames),
						CVT_INT_CP(cdrlog_info.uiDownTCPREFrames),
						CVT_INT_CP(cdrlog_info.uiUPTCPREBytes),
						CVT_INT_CP(cdrlog_info.uiDownTCPREBytes));

					fputs(buffer, write_fd);
					cdrlog_cnt++;
				}
			}
		}
	} 	/** end of while **/

	total_cdr_cnt += cdrlog_cnt;
	printf("=======> CDR LOG COUNT : %d\n", cdrlog_cnt);
	printf("=======> check register cnt : %d\n", reg_cnt);
	cdrlog_cnt = 0;
	fclose(fd);
									
	return (1);

}


/** 
 *   check service option/Server IP/Server Port duplication 
 **/
int
check_dup_value(int svc_opt, unsigned int dest_ip, unsigned short dest_port)
{

    int     i, j, k;


    //if (svc_opt == svc_opt_list[i])
    if (search_service_opt(pst_svc_opt_list_head, svc_opt) > 0)
    {
        //if (dest_ip == dest_ip_list[j])
        if (search_dest_ip(pst_ip_list_head, dest_ip) > 0)
        {
            //if (dest_port == dest_port_list[k])
            if (search_dest_port(pst_port_list_head, dest_port) > 0)
            {
                return(-1);
            }
            else
                return(2);

        }
        else
            return(3);

    }
    else
        return(4);

}


/**
 *      search service option list
 **/
int
search_service_opt(pst_svc_opt_list search_list, int svc_opt)
{

    if (search_list == NULL)
        return(-1);

    while(svc_opt != search_list->svc_opt)
    {
        search_list = search_list->next_svc_opt;

        if (search_list == NULL)
            return(-1);

    }
    return (1);

}


/**
 *      search destination ip list
 **/
int search_dest_ip(pst_ip_list search_list, unsigned int dest_ip)
{

    if (search_list == NULL)
        return(-1);

    while(dest_ip != search_list->dest_ip)
    {
        search_list = search_list->next_dest_ip;

        if (search_list == NULL)
            return(-1);

    }
    return (1);

}


/**
 *      search destination ip list
 **/
int
search_dest_port(pst_port_list search_list, unsigned short dest_port)
{

    if (search_list == NULL)
        return(-1);

    while(dest_port != search_list->dest_port)
    {
        search_list = search_list->next_dest_port;

        if (search_list== NULL)
            return(-1);

    }
    return (1);

}


/**
 *      add service option
 **/
int
add_service_opt(int svc_opt)
{

    pst_svc_opt_list    new_svc_opt;

    new_svc_opt = (pst_svc_opt_list)calloc(1, sizeof(st_svc_opt_list));
    if (new_svc_opt == NULL)
    {
        fprintf(stderr, "MEMORY ALLOCATION FAILURE\n");
        return (-1);
    }

    new_svc_opt->svc_opt = svc_opt;

    if (pst_svc_opt_list_head == NULL)
        pst_svc_opt_list_head = new_svc_opt;
    else
        pst_svc_opt_list_tail->next_svc_opt= new_svc_opt;

    new_svc_opt->next_svc_opt = NULL;
    pst_svc_opt_list_tail = new_svc_opt;

    return(0);
}


/**
 *      add destination ip
 **/
int
add_dest_ip(unsigned int dest_ip)
{

    pst_ip_list     new_ip;

    new_ip = (pst_ip_list) calloc(1, sizeof(st_ip_list));
    if (new_ip == NULL)
    {
        fprintf(stderr, "MEMORY ALLOCATION FAILURE\n");
        return (-1);
    }

    new_ip->dest_ip = dest_ip;

    if (pst_ip_list_head == NULL)
        pst_ip_list_head = new_ip;
    else
        pst_ip_list_tail->next_dest_ip = new_ip;

    new_ip->next_dest_ip = NULL;
    pst_ip_list_tail = new_ip;

    return(0);

}



/**
 *      add destination port
 **/
int
add_dest_port(unsigned short dest_port)
{

    pst_port_list       new_port;

    new_port = (pst_port_list) calloc(1, sizeof(st_port_list));
    if (new_port == NULL)
    {
        fprintf(stderr, "MEMORY ALLOCATION FAILURE\n");
        return (-1);
    }

    new_port->dest_port = dest_port;

    if (pst_port_list_head == NULL)
        pst_port_list_head = new_port;
    else
        pst_port_list_tail->next_dest_port = new_port;

    new_port->next_dest_port = NULL;
    pst_port_list_tail = new_port;

    return(0);

}



/**
 *
 **/
int 
get_day_per_month(int cur_month)
{
	switch(cur_month)
	{
		case 1:
			return (31);
		case 2:
			return (28);
		case 3:
			return (31);
		case 4:
			return (30);
		case 5:
			return (31);
		case 6:
			return (30);
		case 7:
		case 8:
			return (31);
		case 9:
			return (30);
		case 10:
			return (31);
		case 11:
			return (30);
		case 12:
			return (31);
		default :
			return (30);

	}

}
