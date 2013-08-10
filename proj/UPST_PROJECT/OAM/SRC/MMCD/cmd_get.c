/**
	@file		cmd_get.c
	@author
	@date		2011-07-14
	@version	
	@brief		cmd_get.c
*/

/**
 *	Include headers
 */

/* SYS HEADER */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* LIB HEADER */
#include "config.h"
#include "loglib.h"
/* PRO HEADER */
#include "mmcdef.h"
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
/* LOC HEADER */
#include "cmd_get.h"

/**
 *	Define cons.
 */
#define MML_PV_MC					0
#define DEF_STAT_TIME				600
#define SECOND_FOR_DELAY			3
#define Two2Int(a)					(((a)[0] - '0') * 10 + (a)[1] - '0')

/**
	Declare var.
*/
tENUMLIST	enumlist[MAX_ENUM_LIST_ELEM];		/* help_proc.h */
extern st_MngPkt	cli_msg;							/* mmcd_main.h */
extern RUN_TBL		*run_tbl;							/* mmcd_main.h */
extern LIB_TBL		lib_tbl[MAX_LIB_TABLE];				/* mmcd_main.h */

/**
	Declare functions
*/
extern int			get_para_idx(char *str);
extern long long	convert_int64(char *p);
extern char			*get_string(char *str);
extern COM_TBL		*get_cmd_tbl(char *str);
extern char			*ComposeHead();
extern int			GetUserInfo(char *UserName, int Type);
extern void			clear_my_tmr(short i);
extern char			*time_short(time_t time);

/**
 *	Implement func.
 */
char *pv_range_check(char *str, PARA_CONT *para)
{
	int i;
	char    szDecimal[16];

	long long key;
	int min, hour, day, mon, year;

	switch (para->type) {
	case VT_DECSTR :
		key = strlen(str);
		for (i = 0;i < key;i++)
			if (isdigit(str[i]) == 0 && str[i] != '-')
				return "DECSTR VALUE ERROR";
		break;

	case VT_NONABS :
	case VT_STRING :
		key = strlen(str);
		break;

	case VT_HTIME  :
		if (strlen(str) != 4)
			return "HOUR-MIN VALUE ERROR";

		hour= Two2Int(&str[0]);
		min = Two2Int(&str[2]);
		if (hour > 23 || min > 59)
			return "HOUR-MIN VALUE ERROR";

		return NULL;

	case VT_YTIME  :
		if (strlen(str) != 10)
			return "DATE VALUE ERROR";

		year = Two2Int(&str[0]) * 100 + Two2Int(&str[2]);
		mon  = Two2Int(&str[4]);
	    day  = Two2Int(&str[6]);
	    hour = Two2Int(&str[8]);
/*
	    min  = Two2Int(&str[10]);
*/
	    if (year > 2030 || year < 1998 || mon > 12 || day > 31 || hour > 23 )
			return "DATE VALUE ERROR";

		return NULL;

	case VT_ENUM:
	case VT_EPOS:
		key = get_para_idx(str);
		for (i = para->start;i < para->end;i++)
			if (key == enumlist[i].code)
				return NULL;

		return "NOT INCLUDED IN ENUMLIST";

	case VT_HEXA   :
		key = strtol(str, NULL, 16);

		/* modified by uamyd 'key' -> '(int)key' */
		sprintf( szDecimal, "%d", (int)key );

        if( strcmp( szDecimal, str ) != 0 )
            return "HEXA VALUE ERROR";

		for (i = 0;str[i];i++)
			if (isxdigit(str[i]) == 0)
				return "HEXA VALUE ERROR";
		break;

	case VT_HEXSTR :
		key = strlen(str);
		for (i = 0;i < key;i++)
			if (isxdigit(str[i]) == 0)
				return "HEXA_STRING VALUE ERROR";
		break;

	case VT_DECIMAL:
		key = atoi(str);

		/* modified by uamyd 'key' -> '(int)key' */
		sprintf( szDecimal, "%d", (int)key );

        if( strcmp( szDecimal, str ) != 0 )
            return "DECIMAL VALUE ERROR";

		for (i = 0;str[i];i++)
			if (isdigit(str[i]) == 0 && str[i] != '-')
				return "DECIMAL VALUE ERROR";
		break;

	case VT_LONGDECIMAL:
		key = convert_int64(str);

		for( i=0; str[i]; i++ )
			if( isxdigit(str[i]) == 0 )
				return "LONGDECIMAL VALUE ERROR";
		break;

	case VT_MTIME:
		if (strlen(str) != 12)
			return "DATE VALUE ERROR";

		year = Two2Int(&str[0]) * 100 + Two2Int(&str[2]);
		mon  = Two2Int(&str[4]);
	    day  = Two2Int(&str[6]);
	    hour = Two2Int(&str[8]);

	    min  = Two2Int(&str[10]);

	    if (year > 2030 || year < 1998 || mon > 12 || day > 31 || hour > 23 || min > 59 )
			return "DATE VALUE ERROR";

		return NULL;


	default:
		return "UNKNOWN PARA TYPE";
	}

	if (key >= para->start && key <= para->end)
		return NULL;

	return "RANGE ERROR";
}


/*******************************************************************************
 PROCESSING COMMAND (PARSING)
*******************************************************************************/
char *cmd_get(In_Arg in_para[], char *outstr, mml_msg *mml, COM_TBL **cmd_ret)
{
	char			tmpstr[256];
	char			*sp;
	int				num_para, cmd_style;
	COM_TBL 		*cmd_ptr;
	int				i, j, k;
	int				dupf, idx, key, slen = 0;
	char			valstr[MAX_PAR_IN_COM][MAX_PARSTR_LEN];
	char			*strp;
	unsigned int 	userLevel;

	if( (sp = get_string(NULL)) == NULL)
		return "INPUT FORMAT ERROR";

	if( (cmd_ptr = get_cmd_tbl(sp)) == NULL)
		return "UNKNOWN COMMAND";

	*cmd_ret	= cmd_ptr;
	cmd_style	= 0;
	num_para	= 0;
	outstr[0]	= 0;

	memset(&in_para[0], 0, sizeof(In_Arg)*30);

	for(i = 0; i < MAX_PAR_IN_COM;i++)
		valstr[i][0] = 0;

	while( (sp = get_string(tmpstr)) != NULL)
	{
		if(num_para == cmd_ptr->num_para)
			return "INVALID PARAMETER COUNT";

		if(sp[0] == ',')
		{
			if(cmd_style == 0)
				cmd_style = 2;
			else if(cmd_style != 2)
				return "DELIMITER ERROR";

			num_para++;
			continue;
		}

		sp = get_string(NULL);

		if( (sp == NULL)||(sp[0] == ','))
		{
			if (cmd_style == 0)
				cmd_style = 2;
			else if (cmd_style != 2)
				return "DELIMITER ERROR";

			if( (strp = pv_range_check(tmpstr, &cmd_ptr->para[num_para])) != NULL)
				return strp;

			strcpy(valstr[num_para], tmpstr);
			num_para++;

			if(sp == NULL)
				break;

		}
		else if(sp[0] == '=')
		{
			if(cmd_style == 0)
				cmd_style = 1;
			else if(cmd_style != 1)
				return "DELIMITER ERROR";

			if( (idx = get_para_idx(tmpstr)) < 0)
				return "UNKNOWN PARAMETER";

			for(i = 0, dupf = 0; i < cmd_ptr->num_para; i++)
			{
				if(cmd_ptr->para[i].idx == idx)
				{
					if(valstr[i][0] == 0)
						break;
					else
						dupf = 1;
				}
			}

			if(i >= cmd_ptr->num_para)
			{
				if(dupf == 1)
					return "DUPLICATED PARA";
				else
					return "UNKNOWN PARAMETER";
			}

			if( (sp = get_string(NULL)) == NULL)
				return "PARA VALUE ERROR";

			if( (strp = pv_range_check(sp, &cmd_ptr->para[i])) != NULL)
				return strp;

			strcpy(valstr[i], sp);
			num_para++;
		}
		else
			return "DELIMITER ERROR";
	}

	for(i = 0; i < cmd_ptr->p_man; i++)
	{
		log_print(LOGN_DEBUG, "MANDATORY INFO [%s]", valstr[i] );
		if(valstr[i][0] == 0)
		{
			return "MANDATORY MISSED";
		}
	}

	memset(mml, 0, sizeof(mml_msg));

	sprintf(outstr, "\n%s  INPUT : %s  ", (char *)ComposeHead(), cmd_ptr->com_name);
	slen = strlen(outstr);

	for(i = 0, j = 0;i < MAX_PAR_IN_COM;i++)
	{
		if(valstr[i][0] == 0)
			continue;

		sprintf(&outstr[slen], "%s=%s ", cmd_ptr->para[i].para_name, valstr[i]);
		slen += strlen(&outstr[slen]);

		strcpy(in_para[j].name, cmd_ptr->para[i].para_name);
		strncpy(in_para[j].value, valstr[i], MMLCONTENTS);

		mml->msg_body[j].para_id	= cmd_ptr->para[i].idx;
		mml->msg_body[j].para_type	= cmd_ptr->para[i].type;
		mml->msg_body[j].para_cont[MMLNUMOFPARA - 1] = 0;
		if(cmd_ptr->para[i].type == VT_ENUM)
		{
			sprintf(mml->msg_body[j].para_cont, "%d", get_para_idx(valstr[i]));
		}
		else if(cmd_ptr->para[i].type == VT_EPOS)
		{
			key = get_para_idx(valstr[i]);
			for(k = cmd_ptr->para[i].start; k < cmd_ptr->para[i].end; k++)
			{
				if(key == enumlist[k].code)
				{
					/* modified by uamyd ''k-cmd_ptr->para[i].start' -> '(int)(k-cmd_ptr...start)' */
					sprintf(mml->msg_body[j].para_cont, "%d", (int)(k - cmd_ptr->para[i].start));
					break;
				}
			}
		}
		else
		{
			strncpy(mml->msg_body[j].para_cont, valstr[i], MMLCONTENTS);
		}
		log_print(LOGN_DEBUG, "[%d] %s = %s", i, cmd_ptr->para[i].para_name, mml->msg_body[j].para_cont);	/* 20040421,poopee */

		j++;
	}
	sprintf(&outstr[slen], "\nACCEPTED\n");

	mml->num_of_para	= num_para;

	log_print(LOGN_DEBUG, "PARAMETER COUNT : J:[%d] NUM_PARA:[%d]", j, num_para);

	mml->msg_id		= cmd_ptr->code;
	mml->msg_len	= j * sizeof(mml_body) + 4;
	mml->dst_proc	= MML_PV_MC;
	mml->dst_func	= cmd_ptr->dest;

	strcpy(mml->adminid, cli_msg.head.userName);

	log_print(LOGN_INFO, "USER NAME[%s]", cli_msg.head.userName);

	/*** CHECK USER LEVEL FOR COMMAND *****************************************/
	if(mml->msg_id != MI_USER_LOGIN)
		userLevel = GetUserInfo(cli_msg.head.userName, 2);
	else
		userLevel = 1;

	if(userLevel > (cmd_ptr->access-1))
	{
		log_print(LOGN_INFO, "DENY");
		return "PERMISSION DENIED";
	}

	return NULL;
}

/*******************************************************************************

*******************************************************************************/
int SetTimer(int esfd, mml_msg *snd_msg, COM_TBL *cmdptr, In_Arg in_para[])
{
	    int  i, j, stat=0;
    int  dPrdNum;
    time_t  tNow;
    time_t  tStatCheck;

    tNow = time(&tNow);

    /*  TIMER Setting  */
    for(i = 1; i<= MAX_TMR_REC; i++)
    {
        if(run_tbl->cmd_id[i] == 0)
        {
            run_tbl->cmd_id[i] = i;
            run_tbl->msg_id[i] = cmdptr->code;

            run_tbl->period[i] = DEF_STAT_TIME;
            run_tbl->stat_tot_cnt[i] = 0;
            run_tbl->stat_cur_cnt[i] = 0;
            run_tbl->stat_TOTAL[i] = 1;
            run_tbl->stat_TOT_NUM[i] = 1;

            for(j = 0; j < snd_msg->num_of_para; j++)
            {
                /*** #mprd : 통계 수집 주기 #mtim : 통계 수집 회수 ************/
                if (strcasecmp (in_para[j].name, "prd") == 0)
                {
                    stat = 1;
                    dPrdNum = atoi(in_para[j].value);

                    /*** 통계 명령에 대한 유효성 체크 *************************/
                    if( (dPrdNum%5) != 0 )
                    {
                        clear_my_tmr(i);
                        return -1;
                    }

                    run_tbl->period[i] = dPrdNum * 60;
                }
                else if(strcasecmp (in_para[j].name, "cnt") == 0)
                {
                    run_tbl->stat_TOTAL[i] = atoi(in_para[j].value);
                    run_tbl->stat_TOT_NUM[i] = atoi(in_para[j].value);

                    /*if (run_tbl->stat_tot_cnt[i] != (-1))*/
                    /*  run_tbl->stat_tot_cnt[i]++; */
                }
                log_print(LOGN_DEBUG, "STAT COMMAND : PARAMETER[%d]:[%s]", j, in_para[j].value);
            }

            if (isalpha(in_para[0].value[0]))
                strcpy (run_tbl->type[i], in_para[0].value);
            else
                run_tbl->type[i][0] = 0;

            sprintf( run_tbl->time_stamp[i], "%s", time_short(tNow) );
            strncpy (run_tbl->user_name[i], cli_msg.head.userName, MAX_USER_NAME_LEN-1);
            run_tbl->inv_proc[i] = cli_msg.head.usSrcProc;
            run_tbl->llIndex[i] = cli_msg.head.llIndex;
            run_tbl->sfd[i] = esfd;
            run_tbl->ucbinflag[i] = cli_msg.head.ucBinFlag;

            run_tbl->exe_proc[i] = cli_msg.head.usResult;

            run_tbl->blockcode[i] = lib_tbl[snd_msg->msg_id % MAX_LIB_TABLE].block;

            memcpy( &run_tbl->usmmlid[i], &cli_msg.head.ucmmlid[0], sizeof(unsigned short));

            memcpy( run_tbl->szCommandString[i], cli_msg.data, cli_msg.head.usBodyLen );
            run_tbl->szCommandString[i][cli_msg.head.usBodyLen] = 0x00;

            if(stat == 1)
            {
                /*** STATISTIC COMMAND ****************************************/

                tStatCheck = (tNow/run_tbl->period[i])*run_tbl->period[i] + run_tbl->period[i];

                run_tbl->time[i] = TIME_OUT + (tStatCheck - tNow);

                run_tbl->stat_check_time[i] = tStatCheck + SECOND_FOR_DELAY;
				log_print(LOGN_DEBUG, "STATCHECK:[%ld] TIME:[%d]", tStatCheck, run_tbl->time[i] );
            }
            else
            {
                run_tbl->stat_check_time[i] = 0;
                run_tbl->time[i] = TIME_OUT;
            }

            break;
        }
    }

    snd_msg->cmd_id = i;

    memcpy( &run_tbl->stMML_MSG[i], snd_msg, sizeof(mml_msg) );

    return 1;

} /* End of SetTimer() */

/* 20040421,poopee */
int isdigitstr(char *str)
{
	int		i, len;

	len = strlen(str);
	for (i=0; i<len; i++)
		if (!isdigit(str[i]))
			return 0;
	return 1;
}

