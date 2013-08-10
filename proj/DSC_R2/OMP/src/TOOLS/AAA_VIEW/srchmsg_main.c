/*
*  _  _   _  ____ ____    _   __     __ _    _    _    _____  __
* | || \ | ||  __|  _ \  / \  \ \   / // \  | |  | |  |  _\ \/ /
* | ||  \| || |__| |_) |/ _ \  \ \ / // _ \ | |  | |  | |__\  /
* | || \   ||  __|    // ___ \  \   // ___ \| |__| |__|  __ | |
* |_||_|\__||_|  |_|\_\_/   \_\  \_//_/   \_\____|____|____||_|
*
* Copyright 2004 Infravalley, Inc. All Rights Reserved
* |_||_|\__||_|  |_|\_\_/   \_\  \_//_/   \_\____|____|____||_|
*
* ------------------------------------------------------------------------------
* MODULE NAME : srchmsg_main.c
* DESCRIPTION :
* REVISION    : DATE       VER NAME                   DESCRIPTION
*               2004/06/08 1.0 poopee                 Created
* COMMENTS    :
* ------------------------------------------------------------------------------
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>
#include "srchmsg_aaa.h"

extern char     *optarg;

/* change NAME (challa) 20060816 */
char   	VIEWTYPE_NAME[3][6] = {"", "TRACE", "EXCEL"};
char    MSGTYPE_NAME[5]  	= "AAA";


SRCH_INFO       SRCHINFO;
int             XFLAG=0;
/* add(challa,20060922) ---> */ 
int             dDirectory = 0;
/* <--- */


void 	usage();
void 	init_sig();
void 	sig_fatal(int signal);
int 	get_para(int cnt, char *para[]);
int 	chk_number(char *str_val, int max_len);
time_t 	get_timet(char *str_val);

extern 	void proc_srchmsg();

/*------------------------------------------------------------------------------
* FUNCTIONS   : main
* SYNOPSIS    :
* PARAMETERS  :
* RETURNS     :
* REMARKS     :
* ----------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    /* 20040819,poopee */
    init_sig();

    /* 검색 조건 검색 및 초기화 */
    if (get_para(argc,argv) < 0)
    {
        usage();
        exit(0);
    }

    /* head-log 파일 검색 */
    proc_srchmsg();

    return 1;
}


/*------------------------------------------------------------------------------
* FUNCTIONS   : signal
* SYNOPSIS    :
* PARAMETERS  :
* RETURNS     :
* REMARKS     :
* ----------------------------------------------------------------------------*/
void init_sig()
{
    signal(SIGINT, sig_fatal);      /* interrupt key(delete or Cntl-C) */
    signal(SIGQUIT, sig_fatal);     /* terminal quit key(Cntl-\) */
    signal(SIGTRAP, sig_fatal);     /* implementation defined H/W fault */
    signal(SIGIOT, sig_fatal);      /* implementatino defined H/W fault */
    signal(SIGTERM, sig_fatal);     /* termination by kill(1) */
    signal(SIGKILL, sig_fatal);     /* termination by kill(1) */
}


void sig_fatal(int signal)
{
    fprintf(stderr,"signal(%d)\n",signal);
    fclose(SRCHINFO.fp);
    fclose(SRCHINFO.err_fp);        /* 20040923,poopee */
    exit(signal == SIGTERM ? 0 : 1);
}


/*------------------------------------------------------------------------------
* FUNCTIONS   : get_para
* SYNOPSIS    :
* PARAMETERS  :
* RETURNS     :
* REMARKS     :
* ----------------------------------------------------------------------------*/
int get_para(int cnt, char *para[])
{
    int             arg_val;
    char            vflag=0, tflag=0;     
    char            sflag=0, eflag=0, fflag=0, iflag=0, mflag=0, oflag=0, dflag=0;
    char            start[11], end[11];
	int             number;
    time_t          timet_val;
    char            *ptr;

    if (cnt == 1) return -1; 


    memset(&SRCHINFO, 0x00, sizeof(SRCHINFO));
    start[0] = 0; end[0] = 0;

    while ((arg_val=getopt(cnt,para,"s:e:f:i:m:t:v:o:x")) != EOF)
    {
        switch (arg_val)
        {
            case 's':

				/* BSDM HEADLOG PATH : BSDA/BSDB INSERT */
                //fprintf (stderr ,"     : INSERT SYS_NUMBER: (1)BSDA (2)BSDB\n");
                fprintf (stderr ,"     : INSERT SYS_NUMBER: (1)DSCA (2)DSCB\n");

				scanf( "%d", &number);
				if( number == 1 || number == 2 )
				{
					dDirectory = number;
				}	
				else
				{
					fprintf(stderr,"NOT FOUND SYS_NUMBER!!!\n");
					exit(0);
				}


                if ((SRCHINFO.starttime=get_timet(optarg)) == 0)
                {
                    fprintf(stderr,"ERROR: invalid STARTTIME(%s)!!!\n",optarg);
                    return -1;
                }

                strcpy(start,optarg);
                sflag = 1;
                break;

            case 'e':
                if ((SRCHINFO.endtime=get_timet(optarg)) == 0)
                {
                    fprintf(stderr,"ERROR: invalid ENDTIME(%s)!!!\n",optarg);
                    return -1;
                }
                strcpy(end,optarg);
                eflag = 1;
                break;

            case 'f':
                if (strlen(optarg) > MAX_FILENAME_LEN)
                {
                     fprintf(stderr,"ERROR: too long filename(%s)!!!\n",optarg);
                     return -1;
                }
                strcpy(SRCHINFO.filename,optarg);
                fflag = 1;

                break;

            case 'i':
                if (chk_number(optarg,MAX_IMSI_LEN) < 0)
                {
                     fprintf(stderr,"ERROR: invalid IMSI(%s)!!!\n",optarg);
                     return -1;
                }

                strcpy(SRCHINFO.imsi,optarg);
                iflag = 1;
                break;

            case 'm':
                if (chk_number(optarg,MAX_MSISDN_LEN) < 0)
                {
                    fprintf(stderr,"ERROR: invalid MSISDN(%s)!!!\n",optarg);
                    return -1;
                }

                strcpy(SRCHINFO.msisdn,optarg);
                mflag = 1;
                break;


            case 'v':
                if (!strcasecmp(optarg,"TRACE"))
                    SRCHINFO.viewtype = VIEWTYPE_NORM;
                else if (!strcasecmp(optarg,"EXCEL"))
                    SRCHINFO.viewtype = VIEWTYPE_LINE;
                else
                {
                    fprintf(stderr,"ERROR: invalid VIEWTYPE(%s)!!!\n",optarg);
                    return -1;
                }

                vflag = 1;
                break;

            case 'o':
                if (strlen(optarg) > MAX_FILENAME_LEN)
                {
                    fprintf(stderr,"ERROR: too long outfile(%s)!!!\n",optarg);
                    return -1;
                }
                strcpy(SRCHINFO.outfile,optarg);
                oflag = 1;
                break;

            case 'x':
                 XFLAG = 1;
                 break;

            default:
                fprintf(stderr,"ERROR: invalid option(%d)!!!\n",arg_val); 
                return -1;

        }
    }

    /* 필수 파라미터 컴사 */
    if (vflag == 0 )
    {
        fprintf(stderr,"ERROR: mandatory parameter missed!!!\n");
        return -1;
    }

    /* 파일 검색조건이 입력되어있는지 검사 : 파일명 또는 시간*/
    /* 조건이 없는 경우 */
    if (fflag == 0 && sflag == 0 && eflag == 0)
    {
        fprintf(stderr,"ERROR: time or file not specified!!!\n");
        return -1;
    }

    /* 파일명과 시간 조건이 모두 입력된 경우 */
    if (fflag == 1 && (sflag == 1 || eflag == 1))
    {
        fprintf(stderr,"ERROR: time and file used at the same time!!!\n");
        return -1;
    }

    /* 파일 검색 조건이 시간일 경우 시작과 종료 시간이 입력되었는지 검사*/
    if ((sflag == 1 || eflag == 1) && ((sflag + eflag) != 2))
    {
        fprintf(stderr,"ERROR: start and end time not specified at the same time!!!\n");
        return -1;
    }


    if (fflag == 1) 
        SRCHINFO.srchtype = SRCHTYPE_FILE;
    else 
        SRCHINFO.srchtype = SRCHTYPE_TIME;

    /* 가입자 검색 조건 MSID, MDN */
    if (iflag == 1 && mflag == 1)
    {
       fprintf(stderr,"ERROR: IMSI and MSISDN used at the same time!!!\n");
       return -1;
    }

    if (iflag == 1) 
        SRCHINFO.numtype = NUMTYPE_IMSI;
    else if (mflag == 1) 
        SRCHINFO.numtype = NUMTYPE_MSISDN;
    else 
        SRCHINFO.numtype = NUMTYPE_ALL;

    /* 출력 파일명이 입력되지 않았을경우
       default파일명을 지정 */
    if (oflag == 0)
    {
        if (fflag != 1)
        {
			if( iflag == 1 ) 
			{
            	sprintf(SRCHINFO.outfile,"%s/SRCHMSG_%s_%s_%s_%s.RSLT",
                	RESULT_PATH,VIEWTYPE_NAME[SRCHINFO.viewtype],start,end,SRCHINFO.imsi);
			}
			else
				/* TIME으로 search 할 경우 OUTFILE NAME */
            	sprintf(SRCHINFO.outfile,"%s/SRCHMSG_%s_%s_%s.RSLT",
                	RESULT_PATH,VIEWTYPE_NAME[SRCHINFO.viewtype],start,end);
        }
        else
        {
            if ((ptr=strrchr(SRCHINFO.filename,'/')) == NULL)
                ptr = SRCHINFO.filename;    /* logical file path */
            else
                ptr += 1;

			/* 특정 IMSI로 search */
			if (iflag == 1 )
			{
            	sprintf(SRCHINFO.outfile,"%s/SRCHMSG_%s_%s_%s.RSLT",
                	RESULT_PATH,VIEWTYPE_NAME[SRCHINFO.viewtype],SRCHINFO.imsi,ptr);
			}
			else
				/* 기본 search */
            	sprintf(SRCHINFO.outfile,"%s/SRCHMSG_%s_%s.RSLT",
                	RESULT_PATH,VIEWTYPE_NAME[SRCHINFO.viewtype],ptr);

        }

        fprintf(stderr,"     : RESULT-FILE       = %s\n",SRCHINFO.outfile);

        strcpy(SRCHINFO.errfile, SRCHINFO.outfile);
        strcat(SRCHINFO.errfile,".ERROR");

    }

    return 0;
}


/*------------------------------------------------------------------------------
* FUNCTIONS   :
* SYNOPSIS    :
* PARAMETERS  :
* RETURNS     :
* REMARKS     :
* ----------------------------------------------------------------------------*/
int chk_number(char *str_val, int max_len)
{
    int     len, i;

    len = strlen(str_val);
    if (len > max_len)
    {
        fprintf(stderr,"ERROR: invalid length(%d)!!!\n",len);
        return -1;
    }

    for (i=0; i<len; i++)
    {
        if (!isdigit(str_val[i]))
        {
            fprintf(stderr,"ERROR: not digit value(%s)!!!\n",str_val);
            return -1;
        }
    }

    return 0;
}


/*------------------------------------------------------------------------------
* FUNCTIONS   :
* SYNOPSIS    :
* PARAMETERS  :
* RETURNS     :
* REMARKS     :
* ----------------------------------------------------------------------------*/
time_t get_timet(char *str_val)
{
    int         len, i, tmp_val;
    char        tmp_str[3];
    time_t      ctime;
    struct tm   bdtime, *bdtime_p;
    time_t      ret_time;

    len = strlen(str_val);
    if (len != 10)
    {
        fprintf(stderr,"ERROR: invalid length(%d)!!!\n",len);
        return -1;
    }

    for (i=0; i<len; i++)
    {
        if (!isdigit(str_val[i]))
        {
            fprintf(stderr,"ERROR: not digit value(%s)!!!\n",str_val);
            return 0;
        }
    }


    ctime = time(&ctime);
    bdtime_p = localtime((time_t *) &ctime);
    bdtime.tm_year = bdtime_p->tm_year;

    // year
    memcpy(tmp_str,str_val,2);
    tmp_str[2] = 0;
    tmp_val = atoi(tmp_str);

    bdtime.tm_year = tmp_val+2000-1900;

    memcpy(tmp_str,str_val+2,2);    // month
    tmp_str[2] = 0;
    tmp_val = atoi(tmp_str);
    if (tmp_val < 1 || tmp_val > 12)
    {
        fprintf(stderr,"ERROR: invalid month value(%d)!!!\n",tmp_val);
        return 0;
    }
    bdtime.tm_mon = tmp_val-1;

    memcpy(tmp_str,str_val+4,2);    // day
    tmp_str[2] = 0;
    tmp_val = atoi(tmp_str);
    if (tmp_val <= 0 || tmp_val > 31)
    {
        fprintf(stderr,"ERROR: invalid day value(%d)!!!\n",tmp_val);
        return 0;
    }
    bdtime.tm_mday = tmp_val;

/* add(challa) 0814 ---> */

    memcpy(tmp_str,str_val+6,2);    // hour
    tmp_str[2] = 0;
    tmp_val = atoi(tmp_str);
    if (tmp_val < 0 || tmp_val > 23)
    {
        fprintf(stderr,"ERROR: invalid day value(%d)!!!\n",tmp_val);
        return 0;
    }
    bdtime.tm_hour = tmp_val;

/* <---- */


    memcpy(tmp_str,str_val+8,2);    // min
    tmp_str[2] = 0;
    tmp_val = atoi(tmp_str);
    if (tmp_val < 0 || tmp_val > 59)
    {
        fprintf(stderr,"ERROR: invalid MIN value(%d)!!!\n",tmp_val);
        return 0;
    }
    bdtime.tm_min = tmp_val;

    bdtime.tm_sec = 0;
    bdtime.tm_isdst = 0;

    return mktime(&bdtime);

}


/*------------------------------------------------------------------------------
* FUNCTIONS   :
* SYNOPSIS    :
* PARAMETERS  :
* RETURNS     :
* REMARKS     :
* ----------------------------------------------------------------------------*/
void usage()
{
    //fprintf(stderr,"AAA_VIEW ((-s START_TIME -e END_TIME) | -f INPUT_FILE) [-i IMSI] -v VIEW_TYPE [-o RESULT_FILE]\n");
    fprintf(stderr,"AAA_VIEW ((-s START_TIME -e END_TIME) | -f INPUT_FILE) [-i IMSI] -v VIEW_TYPE\n");

    fprintf(stderr,"       VERSION  : R2.1.7 (08.01.08)\n");
    fprintf(stderr,"       TIME     : YYMMDDhhmm (ex: 0609221600)\n");
    fprintf(stderr,"       VIEW_TYPE: TRACE | EXCEL\n");
    
}
