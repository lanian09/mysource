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
// 07.22 jjinri #include <srchmsg.h>

// 07.22 jjinri int CheckPara(SRCH_INFO *pSRCHINFO, char *szResultBuf);
// 07.22 jjinri extern int proc_srchmsg(SRCH_INFO *pSRCHINFO, char *szResultBuf);

char    VIEWTYPE_NAME[3][5] = {"", "NORM", "LINE"};

/*------------------------------------------------------------------------------
* FUNCTIONS   : main
* SYNOPSIS    :
* PARAMETERS  :
* RETURNS     :
* REMARKS     :
* ----------------------------------------------------------------------------*/
#if 0 // 07.22 jjinri 
int InitUdrSearch(SRCH_INFO *SRCH_INFO)
{
    int iRet = 0;

	PASS_EXCEPTION(SRCH_INFO == NULL, END_PROCESS);
    /* head-log 파일 검색 */
    

    PASS_CATCH(END_PROCESS)
    iRet = -1;
    
    PASS_CATCH_END;

    return iRet;
}


int CheckPara(SRCH_INFO *pSRCHINFO, char *szResultBuf)
{
    int             arg_val;
    char            *ptr;
    int             iRet = 0;
	char			starttime[256];
	char			endtime[256];

	memset(starttime, 0x00, 256);
	memset(endtime, 0x00, 256);
    /* 필수 파라미터 컴사 */
    PASS_EXCEPTION(pSRCHINFO->ucViewF == 0 , MANDATORY_PARA_ERR);

    /* 파일 검색조건이 입력되어있는지 검사 : 파일명 또는 시간*/
    /* 조건이 없는 경우 */
    PASS_EXCEPTION(pSRCHINFO->ucSearchFileF == 0 && pSRCHINFO->ucStartF == 0 && pSRCHINFO->ucEndF == 0, 
        TIME_OR_FILE_NOT_FOUND);

    /* 파일명과 시간 조건이 모두 입력된 경우 */
    PASS_EXCEPTION(pSRCHINFO->ucSearchFileF == 1 && (pSRCHINFO->ucStartF == 1 || pSRCHINFO->ucEndF == 1), 
        TIME_AND_FILE_ALL_FOUND);

    /* 파일 검색 조건이 시간일 경우 시작과 종료 시간이 입력되었는지 검사*/
    PASS_EXCEPTION((pSRCHINFO->ucStartF == 1 || pSRCHINFO->ucEndF == 1) 
        && ((pSRCHINFO->ucStartF + pSRCHINFO->ucEndF) != 2),
            START_OR_END_TIME_NOT_FOUND);

    if (pSRCHINFO->ucSearchFileF == 1) 
    {
        pSRCHINFO->srchtype = SRCHTYPE_FILE;
	}
    else
    {
        pSRCHINFO->srchtype = SRCHTYPE_TIME;
		iRet = cftime(starttime, "%Y%m%d%H%M", (time_t *)&pSRCHINFO->starttime);
		iRet = cftime(endtime, "%Y%m%d%H%M", (time_t *)&pSRCHINFO->endtime);
    }
    /* 가입자 검색 조건 MSID */

    PASS_EXCEPTION(pSRCHINFO->ucImsiF != 1 && pSRCHINFO->ucSearchFileF != 1 
        && pSRCHINFO->ucUdrSeqF != 1, NO_IMSI_NUM);

    if( pSRCHINFO->ucImsiF == 1 )
    {
        pSRCHINFO->numtype = NUMTYPE_IMSI;
    }
    if( pSRCHINFO->ucUdrSeqF == 1 )
    {
        pSRCHINFO->numtype = NUMTYPE_UDRSEQ;
    }
    /* 출력 파일명이 입력되지 않았을경우
       default파일명을 지정 */
    if (pSRCHINFO->ucOutfileF == 0)
    {
        if (pSRCHINFO->ucSearchFileF != 1)
        {
            if( pSRCHINFO->ucImsiF == 1 )
            {
                sprintf(pSRCHINFO->outfile, "%s/SRCHMSG_%s_%s_%s_IMSI%s.DUMP",
                    RESULT_PATH,VIEWTYPE_NAME[pSRCHINFO->viewtype],
                    starttime,endtime,
                    pSRCHINFO->imsi);
            }
            else if( pSRCHINFO->ucUdrSeqF == 1 )
            {
                sprintf(pSRCHINFO->outfile, "%s/SRCHMSG_%s_%s_%s_SEQ%d.DUMP",
                    RESULT_PATH,VIEWTYPE_NAME[pSRCHINFO->viewtype],
                    starttime, endtime,
                    pSRCHINFO->uiUdrSeq);
            }
        }
        else
        {
            if ((ptr=strrchr(pSRCHINFO->filename,'/')) == NULL)
                ptr = pSRCHINFO->filename;    /* logical file path */
            else
                ptr += 1;

            sprintf(pSRCHINFO->outfile,"%s/SRCHMSG_%s_%s.DUMP",
                RESULT_PATH,VIEWTYPE_NAME[pSRCHINFO->viewtype],
                ptr);

        }


    }
    sprintf(szResultBuf,"==========================================================================\n"
						"    RESULT FILE  = %s\n",pSRCHINFO->outfile);
    strcpy(pSRCHINFO->errfile, pSRCHINFO->outfile);
    strcat(pSRCHINFO->errfile,".RESULT");
//    strcat(pSRCHINFO->errfile,".ERROR");
	
	iRet = 0;
    PASS_CATCH(GET_START_TIME_ERR)
    fprintf(stderr,"ERROR: invalid STARTTIME(%s)!!!\n",optarg);
    iRet = -1;
    
    PASS_CATCH(GET_END_TIME_ERR)
    fprintf(stderr,"ERROR: invalid ENDTIME(%s)!!!\n",optarg);
    iRet = -1;
    
    PASS_CATCH(FILENAME_LEN_ERR)
    fprintf(stderr,"ERROR: too long filename(%s)!!!\n",optarg);
    iRet = -1;
    
    PASS_CATCH(IMSI_LEN_ERR)
    fprintf(stderr,"ERROR: invalid IMSI(%s)!!!\n",optarg);
    iRet = -1;
    
    
    PASS_CATCH(UNKNOWN_MSG_TYPE)
    fprintf(stderr,"ERROR: invalid MSGTYPE(%s)!!!\n",optarg);
    iRet = -1;
                    
    PASS_CATCH(UNKNOWN_VIEW_TYPE)
    fprintf(stderr,"ERROR: invalid VIEWTYPE(%s)!!!\n",optarg);
    iRet = -1;
                
    PASS_CATCH(MAX_FILENAME_ERR)
    fprintf(stderr,"ERROR: too long outfile(%s)!!!\n",optarg);
    iRet = -1;
                
    
    PASS_CATCH(MANDATORY_PARA_ERR)
    fprintf(stderr,"ERROR: mandatory parameter missed!!!\n");
    iRet = -1;
    
    PASS_CATCH(TIME_OR_FILE_NOT_FOUND)
    fprintf(stderr,"ERROR: time or file not specified!!!\n");
    iRet = -1;
    
    PASS_CATCH(TIME_AND_FILE_ALL_FOUND)
    fprintf(stderr,"ERROR: time and file used at the same time!!!\n");
    iRet = -1;
    
    PASS_CATCH(START_OR_END_TIME_NOT_FOUND)
    fprintf(stderr,"ERROR: start and end time not specified at the same time!!!\n");
    iRet = -1;
    
    PASS_CATCH(DUAL_SEARCH_OPT)
    fprintf(stderr,"ERROR: IMSI and MSISDN used at the same time!!!\n");
    iRet = -1;
    
    PASS_CATCH(NO_IMSI_NUM)
    fprintf(stderr,"ERROR: IMSI Is Not Exist !!!\n");
    iRet = -1;

    PASS_CATCH(SEQ_LEN_ERR)
    fprintf(stderr,"ERROR: Sequence Is Not Number !!!\n");
    iRet = -1;
    
    PASS_CATCH_END;

    return iRet;
}


int FindUdrSearch(SRCH_INFO *pSRCHINFO, char *szResultBuf)
{
	int iRet = 0;

	PASS_EXCEPTION(CheckPara(pSRCHINFO, szResultBuf) !=0, CHECK_PARA_FAILURE);
	PASS_EXCEPTION(proc_srchmsg(pSRCHINFO, szResultBuf) !=0, PROC_SRCH_FAILURE);


	PASS_CATCH(CHECK_PARA_FAILURE);
	iRet = -1;
	
	PASS_CATCH(PROC_SRCH_FAILURE);
	iRet = -1;

	PASS_CATCH_END;
	return iRet;
}
#endif

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

    bdtime.tm_year = tmp_val+100;

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
    if (tmp_val < 0 || tmp_val > 31)
    {
        fprintf(stderr,"ERROR: invalid day value(%d)!!!\n",tmp_val);
        return 0;
    }
    bdtime.tm_mday = tmp_val;
    
    memcpy(tmp_str,str_val+6,2);    // hour
    tmp_str[2] = 0;
    tmp_val = atoi(tmp_str);
    if (tmp_val < 0 || tmp_val > 23)
    {
        fprintf(stderr,"ERROR: invalid day value(%d)!!!\n",tmp_val);
        return 0;
    }
    bdtime.tm_hour = tmp_val;
    

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


