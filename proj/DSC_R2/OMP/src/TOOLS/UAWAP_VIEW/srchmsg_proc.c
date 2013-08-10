/*
*  _  _   _  ____ ____    _   __     __ _    _    _    _____  __
* | || \ | ||  __|  _ \  / \  \ \   / // \  | |  | |  |  _\ \/ /
* | ||  \| || |__| |_) |/ _ \  \ \ / // _ \ | |  | |  | |__\  /
* | || \   ||  __|    // ___ \  \   // ___ \| |__| |__|  __ | |
* |_||_|\__||_|  |_|\_\_/   \_\  \_//_/   \_\____|____|____||_|
*
* Copyright 2004 Infravalley, Inc. All Rights Reserved
*
* ------------------------------------------------------------------------------
* MODULE NAME : srchmsg_proc.c
* DESCRIPTION :
* REVISION    : DATE       VER NAME                   DESCRIPTION
*               2004/06/08 1.0 poopee                 Created
* COMMENTS    :
*
*
* ------------------------------------------------------------------------------
*/

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include "srchmsg.h"
extern int              errno;

#define LOG_FILE_LEN    21
int srch_file(st_SrchInfo_t *pSrchInfo, FILE *Outfd);
int is_headlog(char *filename);
time_t get_ctime(char *filename);
int srch_msg(char *filename, FILE *OutFd, st_SrchInfo_t   *pSrchInfo);
int write_title(FILE *OutFd, st_SrchInfo_t *pSrchInfo);
//int write_msg(st_AAAREQ *pAAAReq, SRCH_INFO *pSRCHINFO);

extern char *str_time(time_t t);



/*------------------------------------------------------------------------------
* FUNCTIONS   :
* SYNOPSIS    :
* PARAMETERS  :
* RETURNS     :
* REMARKS     :
* ----------------------------------------------------------------------------*/

int srch_msg(char *filename, FILE *OutFd, st_SrchInfo_t   *pSrchInfo)
{
    int         fd, len, ret;
    int         tot_msg=0, tot_found=0, tot_written=0;
    char        tmp_filename[MAX_FILENAME_LEN+1], command[128], *ptr;

    st_MsgQ             qmsg;
    st_AAAREQ           stAAA;
    int                 iRet = 0;
    char                tmpStart[256];
    char                tmpStop[256];
    struct stat         statbuf;
    time_t              start_time;
    st_LogFileHead      file_head;
    st_LogDataHead      data_head;
    st_DbFetchInfo_t    stDbFetch;
    st_LogParse_t       stLogParse;

    memset( &qmsg, 0x00, sizeof(st_MsgQ) );
    memset(&file_head, 0x00, sizeof(st_LogFileHead));
    memset(&data_head, 0x00, sizeof(st_LogDataHead));
    memset(&stDbFetch, 0x00, sizeof(st_DbFetchInfo_t));
    memset(&stLogParse, 0x00, sizeof(st_LogParse_t));

    PASS_EXCEPTION(stat(filename,&statbuf) < 0, STAT_ERROR);
    

    len = strlen(filename);
    strcpy(tmp_filename,filename);

    PASS_EXCEPTION((fd=open(tmp_filename,O_RDONLY)) < 0, FILE_OPEN_ERR);
    
    PASS_EXCEPTION((len=read(fd,(void *)&file_head,sizeof(st_LogFileHead))) <= 0,
        FILE_READ_ERR);
    
    if (pSrchInfo->iSrchType == SRCHTYPE_TIME)
    {
        PASS_EXCEPTION(statbuf.st_mtime < pSrchInfo->tStartTime,
            FILE_END_ERR);

        PASS_EXCEPTION((start_time = get_ctime(filename)) > pSrchInfo->tEndTime,
            FILE_START_ERR);
    }
    
    while(1)
    {
        memset(&data_head, 0x00, sizeof(st_LogDataHead));
        memset(&stDbFetch, 0x00, sizeof(st_DbFetchInfo_t));
        memset(&stLogParse, 0x00, sizeof(st_LogParse_t));
        len = read(fd, (void *)&data_head, sizeof(st_LogDataHead));
		
        if( len <= 0 )
        {
            if( len < 0 )
            {
                fprintf(stdout,"ERROR: file(%s) read error(%s)!!!\n",
                    filename,strerror(errno));
            }
            close(fd);
            break;

        }
        len = read(fd, (void *)&stDbFetch, sizeof(st_DbFetchInfo_t));
        if( pSrchInfo->iSrchType == SRCHTYPE_TIME )
        {
            if( (CVT_INT_CP(data_head.stTmval.tv_sec) < pSrchInfo->tStartTime)
                || (CVT_INT_CP(data_head.stTmval.tv_sec) > pSrchInfo->tEndTime))
            {
                continue;
            }
        }

        tot_msg++;
        
        ParsingDbData(stDbFetch.sTLOG, &stLogParse);

        switch (pSrchInfo->iSrchKeyType)
        {
            case SRCHKEY_LOGID:
				if (stDbFetch.iLogId == 0)
                {
					fprintf(stdout,"ERROR: MSID not exist in log header\n");
                }
				else if (stDbFetch.iLogId == pSrchInfo->uiLogId )
				{
                    tot_found++;
                    if ((ret = print_txnlog_msg(&stDbFetch, &stLogParse, OutFd, pSrchInfo)) == 0)
                    {
                        tot_written++;
                    }
				}
                break;

            case SRCHKEY_ALL:
            default:

            tot_found++;
            if ((ret = print_txnlog_msg(&stDbFetch, &stLogParse, OutFd, pSrchInfo)) == 0)
            {
                tot_written++;
            }
            break;

        }

    }


    close(fd);


    if ((ptr=strrchr(filename,'/')) == NULL)
    {
        ptr = filename;
    }
    else
    {
        ptr += 1;
    }
    fprintf(stdout,"     : %-15s   : MESSAGE=%d FOUND=%d WRITTEN=%d\n",
            ptr,tot_msg,tot_found,tot_written);           
    
    
    PASS_CATCH(STAT_ERROR)
    fprintf(stdout,"ERROR: stat(%s) error(%s)!!!\n",filename,strerror(errno));
    iRet = -1;
    
    PASS_CATCH(FILE_OPEN_ERR)
    fprintf(stdout,"ERROR: file(%s) open error(%s)!!!\n",tmp_filename,strerror(errno));
    iRet = -1;
    
    PASS_CATCH(FILE_READ_ERR)
    fprintf(stdout,"ERROR: file(%s) read error(%s)!!!\n",tmp_filename,strerror(errno));
    iRet = -1;
        
    PASS_CATCH(FILE_END_ERR)
    fprintf(stdout,"ERROR: filestat.st_mtime(%d) is Smaller than pSrchInfo->tStartTime(%d)!!\n",
        statbuf.st_mtime, pSrchInfo->tStartTime);
    iRet = -1;
    
    PASS_CATCH(FILE_START_ERR)
    fprintf(stdout,"ERROR: get_ctime(filename) (%d) is Bigger than pSrchInfo->tEndTime (%d)!!\n",
        start_time, pSrchInfo->tEndTime);
    iRet = -1;
    
    
    PASS_CATCH_END;

    return iRet;
}


/*------------------------------------------------------------------------------
* FUNCTIONS   :
* SYNOPSIS    :
* PARAMETERS  :
* RETURNS     :
* REMARKS     :
* ----------------------------------------------------------------------------*/
int write_title(FILE *OutFd, st_SrchInfo_t *pSrchInfo)
{
    char        buffer[1024*20];
    int         len;
    int         iRet = 0;
    
    sprintf(buffer,"LOG-ID; LOG-TIME; ");
    len = strlen(buffer);

    sprintf(&buffer[len],"FLAG; ");
    len = strlen(buffer);
    
    sprintf(&buffer[len],"MIN; TERMINAL-IP-ADDRESS; TERMINAL-PORT; ");
    len = strlen(buffer);
    
    sprintf(&buffer[len],"SUB-NUMBER; REQUEST-TIME; RESPONSE-TIME; ");
    len = strlen(buffer);
    
    sprintf(&buffer[len],"WAP-REQUEST-SIZE; WAP-RESPONSE-SIZE; CONTENTS-LENGTH; ");
    len = strlen(buffer);

    sprintf(&buffer[len],"RESULT-CODE; STATUS; URL; ");
    len = strlen(buffer);
    
    sprintf(&buffer[len],"METHOD-TYPE; USER-AGENT; TLOG; \n");
    len = strlen(buffer);




    PASS_EXCEPTION(fprintf(OutFd,"%s",buffer) < 0, FWRITE_FAIL);
    

    PASS_CATCH(FWRITE_FAIL)
    fprintf(stdout,"ERROR: file(%s) write error(%s)!!!\n",
            pSrchInfo->szOutFile,strerror(errno));
    
    iRet = -1;
    
    PASS_CATCH_END;

    return iRet;
}


void print_hex_log(FILE* fp, char *buffer, int len)
{
    int     i;

    for (i=0; i<len; i++)
    {
        if (i % 10 == 0) fprintf(fp,"\n");
        fprintf(fp,"%02x ",0x000000FF & buffer[i]);
    }
    fprintf(fp,"\n");

    return;
}

/*------------------------------------------------------------------------------
* FUNCTIONS   :
* SYNOPSIS    :
* PARAMETERS  :
* RETURNS     :
* REMARKS     :
* ----------------------------------------------------------------------------*/
time_t get_ctime(char *filename)
{
    int         len, i, tmp_val;
    char        *ptr, tmp_str[5];
    struct tm   bdtime;
    char        *tmp_filename = NULL;

    /* YYYYmmdd_HHMMSS */
    ptr = strrchr(filename,'/');
    ptr += 1;
    
    //tmp_filename = ptr + 8;
    tmp_filename = ptr + 6;
    
    memcpy(tmp_str,tmp_filename,4);      // year
    tmp_str[4] = 0;
    tmp_val = atoi(tmp_str);
    if (tmp_val < 1970 || tmp_val > 9999)
    {
        fprintf(stdout,"ERROR: invalid year value(%d)!!!\n",tmp_val);
        return 0;
    }
    bdtime.tm_year = tmp_val - 1900;

    memcpy(tmp_str,tmp_filename+4,2);    // month
    tmp_str[2] = 0;
    tmp_val = atoi(tmp_str);
    if (tmp_val < 1 || tmp_val > 12)
    {
        fprintf(stdout,"ERROR: invalid month value(%d)!!!\n",tmp_val);
        return 0;
    }
    bdtime.tm_mon = tmp_val - 1;

    memcpy(tmp_str,tmp_filename+6,2);    // day
    tmp_str[2] = 0;
    tmp_val = atoi(tmp_str);
    if (tmp_val < 1 || tmp_val > 31)
    {
        fprintf(stdout,"ERROR: invalid day value(%d)!!!\n",tmp_val);
        return 0;
    }
    bdtime.tm_mday = tmp_val;

    memcpy(tmp_str,tmp_filename+8+1,2);    // hour
    tmp_str[2] = 0;
    tmp_val = atoi(tmp_str);
    if (tmp_val < 0 || tmp_val > 23)
    {
       fprintf(stdout,"ERROR: invalid hour value(%d)!!!\n",tmp_val);
       return 0;
    }
    bdtime.tm_hour = tmp_val;

    memcpy(tmp_str,tmp_filename+10+1,2);   // min
    tmp_str[2] = 0;
    tmp_val = atoi(tmp_str);
    if (tmp_val < 0 || tmp_val > 59)
    {
        fprintf(stdout,"ERROR: invalid min value(%d)!!!\n",tmp_val);
        return 0;
    }
    bdtime.tm_min = tmp_val;

    memcpy(tmp_str,tmp_filename+12+1,2);   // sec
    tmp_str[2] = 0;
    tmp_val = atoi(tmp_str);
    if (tmp_val < 0 || tmp_val > 59)
    {
        fprintf(stdout,"ERROR: invalid sec value(%d)!!!\n",tmp_val);
        return 0;
    }

    bdtime.tm_sec = tmp_val;

    return (mktime(&bdtime));

}

#if 0
int srch_file(st_SrchInfo_t *pSrchInfo)
{
    DIR         *dirp;
    struct dirent   *direntp;
    char        filename[MAX_FILENAME_LEN+1];
    int         iRet = 0;
    int         iFileCnt = 0;
    
    PASS_EXCEPTION((dirp=opendir(pSrchInfo->szDirectory)) == NULL, OPEN_DIR_FAIL);
    
    direntp = readdir(dirp);

    while ((direntp=readdir(dirp)) != 0)
    {
        if (!strcmp(direntp->d_name,".") || !strcmp(direntp->d_name,"..")) 
            continue;
        
        /*
        if (is_headlog(direntp->d_name) < 0) 
            continue;
        */
        snprintf(pSrchInfo->szFileInfo[iFileCnt++], MAX_FILENAME_LEN, "%s/%s",pSrchInfo->szDirectory,direntp->d_name);
    }

    pSrchInfo->iFileCnt = iFileCnt;
    
    PASS_CATCH(OPEN_DIR_FAIL)
    fprintf(stdout,"ERROR: directory(%s) open error(%s)!!!\n",pSrchInfo->szDirectory,strerror(errno));
    iRet = -1;

    PASS_CATCH_END;

    closedir(dirp);

    return iRet;
}
#endif

int srch_file(st_SrchInfo_t *pSrchInfo, FILE *Outfd)
{
    DIR         *dirp;
    struct dirent   *direntp;
    char        filename[MAX_FILENAME_LEN+1];
    int         iRet = 0;
    int         iFileCnt = 0;
    struct tm   tmStart, tmEnd, tmInc;
    char        keepDir[15], endDir[15];
    char        startDate[15], endDate[15];
    char        searchDir[128], keepDate[15];
	char		sysSide;
    time_t      incrDay = 0;
    struct stat         statbuf;
    int			i = 0;
	char		szTmpCmdName[256];

    localtime_r(&pSrchInfo->tStartTime, &tmStart);
    localtime_r(&pSrchInfo->tEndTime, &tmEnd);
    
    sprintf(endDir, "%02d%02d", tmEnd.tm_mon+1, tmEnd.tm_mday);
    sprintf(endDate, "20%02d%02d%02d_%02d%02d%02d", tmEnd.tm_year-100, tmEnd.tm_mon+1, tmEnd.tm_mday,
        tmEnd.tm_hour, tmEnd.tm_min, tmEnd.tm_sec);
    sprintf(startDate, "20%02d%02d%02d_%02d%02d%02d", tmStart.tm_year-100, tmStart.tm_mon+1, tmStart.tm_mday,
        tmStart.tm_hour, tmStart.tm_min, tmStart.tm_sec);
    
    
	for( i=0; i<2; i++)
	{
		if( i == 0 )
		{
			sysSide = 'A';
		}
		else
		{
			sysSide = 'B';
		}
		sprintf(keepDir, "%02d%02d", tmStart.tm_mon+1, tmStart.tm_mday);
		incrDay = pSrchInfo->tStartTime - (pSrchInfo->tStartTime % 86400 );


		while ( strcmp(endDir, keepDir) >= 0 )
		{
			memset(szTmpCmdName, 0x00, 256);
			sprintf(searchDir, "%s/BSD%c/%s", "/BSDM/HEADLOG/UAWAP", sysSide, keepDir);
			fprintf(stdout, "Search Dir[%s]\n", searchDir);
			dirp = NULL;
			dirp = opendir(searchDir);
			while( dirp && (direntp = readdir(dirp)) != 0 )
			{

				if (!strcmp(direntp->d_name,".") || !strcmp(direntp->d_name,"..")) 
					continue;
				if ( !(strlen(direntp->d_name) == LOG_FILE_LEN ||
						strlen(direntp->d_name) == TXN_GZFILE_LEN) ) 
					continue;
				/*
				if ( strncmp(startDate, &direntp->d_name[6], 15) > 0 )
					continue;
				if ( strncmp(endDate, &direntp->d_name[6], 15) < 0 )
					break;
				*/
				memset( filename, 0x00, sizeof(filename));
				if( strlen(direntp->d_name) == TXN_GZFILE_LEN )
				{
					snprintf(szTmpCmdName, 256, "gunzip %s/%s", searchDir, direntp->d_name);
					snprintf(filename, 256, "%s/%s", searchDir, direntp->d_name);
					filename[strlen(filename)-3] = '\0';
					system(szTmpCmdName);
					srch_msg(filename, Outfd, pSrchInfo);
					snprintf(szTmpCmdName, 256, "gzip %s", filename);
					system(szTmpCmdName);
				}
				else
				{
					sprintf(filename, "%s/%s", searchDir, direntp->d_name);
					srch_msg(filename, Outfd, pSrchInfo);
				}
			}
			if( dirp )
			{
				closedir(dirp);
			}
			incrDay = pSrchInfo->tStartTime + 86400;
			localtime_r(&incrDay, &tmInc);
			sprintf(keepDir, "%02d%02d", tmInc.tm_mon+1, tmInc.tm_mday);
		}
	}

    
    PASS_CATCH(OPEN_DIR_FAIL)
    fprintf(stdout,"ERROR: directory(%s) open error(%s)!!!\n",pSrchInfo->szDirectory,strerror(errno));
    iRet = -1;
    
    PASS_CATCH(STAT_ERROR)
    iRet = -1;

    PASS_CATCH_END;

    return iRet;
}



