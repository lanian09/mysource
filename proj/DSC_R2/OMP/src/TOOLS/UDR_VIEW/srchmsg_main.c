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
#include "srchmsg.h"
extern char     *optarg;


#define UDR_FILE_LEN 36
#define UDR_GZFILE_LEN 39

#define DEFAULT_FILE_STR "0"

char    VIEWTYPE_NAME[3][5] = {"", "NORM", "LINE"};

int             XFLAG=0;

void init_sig();
void sig_fatal(int signal);
int chk_number(char *str_val, int max_len); time_t get_timet(char *str_val);
void usage();


#define FILE_EXIT       "quit"

st_SrchInfo_t   gSrchInfo;

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

        
        
int isUdrDump(char *file_buf)
{
    int  iRet = 0;
    char *ptr = NULL;

    ptr = strrchr(file_buf, '/');
    
    ptr += 1;

    if( strlen(ptr) == 36 )
    {
        iRet = 0;
    }
    else
    {
        fprintf(stdout, "File is Not Dump File [%s] \n", file_buf);
        iRet = -1;
    }

    return iRet;
}

int
GetViewType()
{
    int         iMainSelect = 0;
    char        input_buf[256];

    fprintf(stdout, "=============================================\n");
    fprintf(stdout, "================== View Type ================\n");
    fprintf(stdout, "=============================================\n");
    fprintf(stdout, "1. Trace Type \n");
    fprintf(stdout, "2. Excel Type \n");
    fprintf(stdout, "9. Goto Main Menu \n");
    fprintf(stdout, "0. EXIT \n");
    fprintf(stdout, "=============================================\n");
    fprintf(stdout, "Input : ");
    
    while (fgets(input_buf, sizeof(input_buf), stdin) == NULL)
    {
    }
    input_buf[strlen(input_buf)-1] = 0;

    iMainSelect = strtol(input_buf, NULL,10);

    switch(iMainSelect)
    {
        case 1:
            gSrchInfo.iViewType = VIEWTYPE_NORM;
            break;
        case 2:
            gSrchInfo.iViewType = VIEWTYPE_LINE;
            break;
        case 9:
            StartMainMenu();
            break;

        case 0:
        default:
            fprintf(stdout, "Program Terminated\n");
            exit(-1);
            break;
    }

    return 0;

}

int
GetOutputFileName()
{
    int flag = 1;
    int i = 0;
    int iCnt = 0;

    char    file_buf[MAX_FILENAME_LEN];

    
    while(1)
    {
        memset(file_buf, 0x00, MAX_FILENAME_LEN);
        fprintf(stdout, "Enter Dump OutFile :");
        while (fgets(file_buf, sizeof(file_buf), stdin) == NULL)
        {
        }
        
        file_buf[strlen(file_buf)-1] = 0;
        if( file_buf[0] == '\n')
        {
            fprintf(stdout, "Please Enter OutFile!!!\n");
            continue;
        }
        else
        {
            break;
        }

    }

    snprintf(gSrchInfo.szOutFile, MAX_FILENAME_LEN, "%s", file_buf);
    fprintf(stderr,"     RESULT-FILE       = %s\n", gSrchInfo.szOutFile);

    return 0;
}

int
GetDirectoryName()
{
    int flag = 1;
    int i = 0;
    int iCnt = 0;

    char    dir_buf[MAX_FILENAME_LEN];

    
    while(1)
    {
        memset(dir_buf, 0x00, MAX_FILENAME_LEN);
        fprintf(stdout, "Enter Search Directory : ");
        while (fgets(dir_buf, sizeof(dir_buf), stdin) == NULL)
        {
        }
        
        dir_buf[strlen(dir_buf)-1] = 0;
        if( dir_buf[0] == '\n')
        {
            fprintf(stdout, "Please Enter Search Directory !!!\n");
            continue;
        }
        else
        {
            break;
        }

    }

    snprintf(gSrchInfo.szDirectory, MAX_FILENAME_LEN, "%s", dir_buf);

    return 0;
}

int
GetInputFileName()
{
    int flag = 1;
    int i = 0;
    int iCnt = 0;
    char *tmpChar = NULL;
    char    file_buf[MAX_FILENAME_LEN];
    
    while(1)
    {
        memset(file_buf, 0x00, MAX_FILENAME_LEN);
        fprintf(stdout, "Enter Dump File (FileName OR quit) [%d]: ", iCnt + 1);
        i = 0;
        while (fgets(file_buf, sizeof(file_buf), stdin) == NULL)
        {
        }
        file_buf[strlen(file_buf)-1] = 0;
        if( !strcasecmp(file_buf, FILE_EXIT) )
        {
            break;
        }
        /* File is udr dump?? */
		/*
        if( isUdrDump(file_buf) != 0 )
        {
            continue;
        }
		*/

        snprintf(gSrchInfo.szFileInfo[iCnt], MAX_FILENAME_LEN, "%s", file_buf);

        iCnt++;
    }

    gSrchInfo.iFileCnt = iCnt;
    gSrchInfo.iSrchType = SRCHTYPE_FILE;

    return 0;
}

int
GetImsiNum()
{
    int flag = 1;
    int i = 0;
    int iCnt = 0;

    char    file_buf[MAX_FILENAME_LEN];

    
    while(1)
    {
        memset(file_buf, 0x00, MAX_FILENAME_LEN);
        fprintf(stdout, "Enter IMSI Number : ");
        while (fgets(file_buf, sizeof(file_buf), stdin) == NULL)
        {
        }
        
        file_buf[strlen(file_buf)-1] = 0;
        if( file_buf[0] == '\n')
        {
            fprintf(stdout, "Please Enter IMSI NUMBER !!!\n");
            continue;
        }
        else
        {
            break;
        }

    }

    snprintf(gSrchInfo.szImsiNum, MAX_IMSI_LEN+1, "%s", file_buf);

    return 0;
}




int
ProcessFileDump()
{
    int i = 0;
    FILE *Outfd = 0;
	char	*tmpChar = NULL;
	char	szTmpCmdName[256];

    GetInputFileName();
    GetOutputFileName();
    PASS_EXCEPTION((Outfd = fopen(gSrchInfo.szOutFile, "w")) == NULL,
        FOPEN_FAIL);
    GetViewType();
    
    /* Write Title */
    if( gSrchInfo.iViewType == VIEWTYPE_LINE )
    {
        PASS_EXCEPTION(write_title(Outfd, &gSrchInfo) != 0, FILE_WRITE_FAILURE);
    }

    for(i=0; i<gSrchInfo.iFileCnt; i++)
    {
        fprintf(stdout, "%s Dump Start\n", gSrchInfo.szFileInfo[i]);
        fflush(stdout);
		tmpChar = strrchr(gSrchInfo.szFileInfo[i], '/');
		tmpChar++;
		if( strlen(tmpChar) == UDR_GZFILE_LEN )
		{
			snprintf(szTmpCmdName, 256, "gunzip %s", gSrchInfo.szFileInfo[i]);
			gSrchInfo.szFileInfo[i][strlen(gSrchInfo.szFileInfo[i])-3] = '\0';
			system(szTmpCmdName);
			srch_msg(gSrchInfo.szFileInfo[i], Outfd, &gSrchInfo);
			snprintf(szTmpCmdName, 256, "gzip %s", gSrchInfo.szFileInfo[i]);
			system(szTmpCmdName);
		}
		else
		{
			srch_msg(gSrchInfo.szFileInfo[i], Outfd, &gSrchInfo); 
		}

    }

    PASS_CATCH(FOPEN_FAIL)
    fprintf(stdout, "OutFile(%s) Open Failure\n", gSrchInfo.szOutFile);
    fflush(stdout);
    exit(-1);
    
    PASS_CATCH(FILE_WRITE_FAILURE)
    fprintf(stdout, "OutFile(%s) Write Failure\n", gSrchInfo.szOutFile);
    fflush(stdout);
    exit(-1);

    PASS_CATCH_END;

    fclose(Outfd);
    return 0;
}


int
GetInputTime()
{
    int flag = 1;
    int i = 0;
    int iCnt = 0;

    char    time_buf[MAX_FILENAME_LEN];

    /* Start Time */
    while(1)
    {
        memset(time_buf, 0x00, MAX_FILENAME_LEN);
        fprintf(stdout, "Enter Start Time (YYMMDDhhmm): ");
        while (fgets(time_buf, sizeof(time_buf), stdin) == NULL)
        {
        }
        
        time_buf[strlen(time_buf)-1] = 0;
        if( time_buf[0] == '\n')
        {
            fprintf(stdout, "Please Enter Start Time!!!\n");
            continue;
        }
        else
        {
            gSrchInfo.tStartTime = get_timet(time_buf);
            if(gSrchInfo.tStartTime > 0)
            {
                break;
            }
        }

    }
    
    /* End Time */
    while(1)
    {
        memset(time_buf, 0x00, MAX_FILENAME_LEN);
        fprintf(stdout, "Enter End Time (YYMMDDhhmm): ");
        while (fgets(time_buf, sizeof(time_buf), stdin) == NULL)
        {
        }
        
        time_buf[strlen(time_buf)-1] = 0;
        if( time_buf[0] == '\n')
        {
            fprintf(stdout, "Please Enter End Time!!!\n");
            continue;
        }
        else
        {
            gSrchInfo.tEndTime = get_timet(time_buf);
            if(gSrchInfo.tEndTime > 0)
            {
                break;
            }
        }

    }
    gSrchInfo.tEndTime = get_timet(time_buf);
    
    gSrchInfo.iSrchType = SRCHTYPE_TIME;

    return 0;
}



int
ProcessTimeDump()
{
    int i = 0;
    FILE *Outfd = 0;

    GetInputTime();
    GetOutputFileName();
    PASS_EXCEPTION((Outfd = fopen(gSrchInfo.szOutFile, "w")) == NULL,
        FOPEN_FAIL);
    GetViewType();
    
    /* Write Title */
    if( gSrchInfo.iViewType == VIEWTYPE_LINE )
    {
        PASS_EXCEPTION(write_title(Outfd, &gSrchInfo) != 0, FILE_WRITE_FAILURE);
    }
    
    /*
    while(1)
    {
        GetDirectoryName();
        if( srch_file(&gSrchInfo) == 0 )
        {
            break;
        }
    }

    for(i=0; i<gSrchInfo.iFileCnt; i++)
    {
        fprintf(stdout, "%s Dump Start\n", gSrchInfo.szFileInfo[i]);
        fflush(stdout);
        srch_msg(gSrchInfo.szFileInfo[i], Outfd, &gSrchInfo);
    }
    */

    srch_file(&gSrchInfo, Outfd);

    PASS_CATCH(FOPEN_FAIL)
    fprintf(stdout, "OutFile(%s) Open Failure\n", gSrchInfo.szOutFile);
    fflush(stdout);
    exit(-1);
    
    PASS_CATCH(FILE_WRITE_FAILURE)
    fprintf(stdout, "OutFile(%s) Write Failure\n", gSrchInfo.szOutFile);
    fflush(stdout);
    exit(-1);

    PASS_CATCH_END;

    fclose(Outfd);
    return 0;
}


int
FileView()
{
    int iMainSelect = 0;
    char    input_buf[256];

    fprintf(stdout, "==========================================================\n");
    fprintf(stdout, "======================== File View =======================\n");
    fprintf(stdout, "==========================================================\n");
    fprintf(stdout, "1. File View From File     (Search From File) \n");
    fprintf(stdout, "2. File View Search Time   (Search From Time) \n");
    fprintf(stdout, "9. Goto Main Menu \n");
    fprintf(stdout, "0. EXIT \n");
    fprintf(stdout, "==========================================================\n");
    fprintf(stdout, "Input : ");
    fflush(stdout);
    
    while (fgets(input_buf, sizeof(input_buf), stdin) == NULL)
    {
    }
    input_buf[strlen(input_buf)-1] = 0;

    iMainSelect = strtol(input_buf, NULL,10);
    switch(iMainSelect)
    {
        case 1:
            ProcessFileDump();
            break;
        case 2:
            ProcessTimeDump();
            break;
        case 9:
            StartMainMenu();
            break;

        case 0:
        default:
            fprintf(stdout, "Program Terminated [%d]\n", iMainSelect);
            fflush(stdout);
            exit(-1);
            break;
    }

    return 0;
}

int
ProcessImsiFile()
{
    int i = 0;
    int iViewType = 0;

    GetImsiNum();
    /* Set SrchKeyType */
    gSrchInfo.iSrchKeyType = SRCHKEY_IMSI;
    
    ProcessFileDump();

    return 0;
}

int
ProcessImsiTime()
{
    int i = 0;
    int iViewType = 0;

    GetImsiNum();
    /* Set SrchKeyType */
    gSrchInfo.iSrchKeyType = SRCHKEY_IMSI;
    
    ProcessTimeDump();

    return 0;
}


int
ImsiView()
{
    int iMainSelect = 0;
    char    input_buf[256];

    fprintf(stdout, "=============================================\n");
    fprintf(stdout, "================== IMSI View ================\n");
    fprintf(stdout, "=============================================\n");
    fprintf(stdout, "1. IMSI VIEW Search File \n");
    fprintf(stdout, "2. IMSI VIEW Search Time \n");
    fprintf(stdout, "9. Goto Main Menu \n");
    fprintf(stdout, "0. EXIT \n");
    fprintf(stdout, "=============================================\n");
    fprintf(stdout, "Input : ");
    fflush(stdout);
    
    while (fgets(input_buf, sizeof(input_buf), stdin) == NULL)
    {
    }
    input_buf[strlen(input_buf)-1] = 0;

    iMainSelect = strtol(input_buf, NULL,10);
    switch(iMainSelect)
    {
        case 1:
            ProcessImsiFile();
            break;
        case 2:
            ProcessImsiTime();
            break;
        case 9:
            StartMainMenu();
            break;

        case 0:
        default:
            fprintf(stdout, "Program Terminated [%d]\n", iMainSelect);
            fflush(stdout);
            exit(-1);
            break;
    }

    return 0;

}


int
GetSeqNum()
{
    int flag = 1;
    int i = 0;
    int iCnt = 0;

    char    file_buf[MAX_FILENAME_LEN];

    
    while(1)
    {
        memset(file_buf, 0x00, MAX_FILENAME_LEN);
        fprintf(stdout, "Enter SEQUENCE Number : ");
        while (fgets(file_buf, sizeof(file_buf), stdin) == NULL)
        {
        }
        
        file_buf[strlen(file_buf)-1] = 0;
        if( file_buf[0] == '\n')
        {
            fprintf(stdout, "Please Enter SEQUENCE NUMBER !!!\n");
            continue;
        }
        else
        {
            break;
        }

    }
    
    gSrchInfo.uiUdrSeq = strtol(file_buf, NULL, 10);
    return 0;
}


int
ProcessSeqFile()
{
    int i = 0;
    int iViewType = 0;

    GetSeqNum();
    /* Set SrchKeyType */
    gSrchInfo.iSrchKeyType = SRCHKEY_SEQ;
    
    ProcessFileDump();

    return 0;
}

int
ProcessSeqTime()
{
    int i = 0;
    int iViewType = 0;

    GetSeqNum();
    /* Set SrchKeyType */
    gSrchInfo.iSrchKeyType = SRCHKEY_SEQ;
    
    ProcessTimeDump();

    return 0;
}


int
SequenceView()
{
    int iMainSelect = 0;
    char    input_buf[256];

    fprintf(stdout, "=============================================\n");
    fprintf(stdout, "================ SEQUENCE View ==============\n");
    fprintf(stdout, "=============================================\n");
    fprintf(stdout, "1. SEQUENCE VIEW Search File \n");
    fprintf(stdout, "2. SEQUENCE VIEW Search Time \n");
    fprintf(stdout, "9. Goto Main Menu \n");
    fprintf(stdout, "0. EXIT \n");
    fprintf(stdout, "=============================================\n");
    fprintf(stdout, "Input : ");
    fflush(stdout);
    
    while (fgets(input_buf, sizeof(input_buf), stdin) == NULL)
    {
    }
    input_buf[strlen(input_buf)-1] = 0;

    iMainSelect = strtol(input_buf, NULL,10);
    switch(iMainSelect)
    {
        case 1:
            ProcessSeqFile();
            break;
        case 2:
            ProcessSeqTime();
            break;
        case 9:
            StartMainMenu();
            break;

        case 0:
        default:
            fprintf(stdout, "Program Terminated [%d]\n", iMainSelect);
            fflush(stdout);
            exit(-1);
            break;
    }

    return 0;

}





int
StartMainMenu()
{
    int iMainSelect = 0;
    char    input_buf[256];
    
    while(1)
    {
        memset(&gSrchInfo, 0x00, sizeof(st_SrchInfo_t));

        fprintf(stdout, "==========================================================\n");
        fprintf(stdout, "================ UDR VIEWER R2.1.7 (08.01.08) ============\n");
        fprintf(stdout, "==========================================================\n");
        fprintf(stdout, "1. File View       (Only File Dump No Search Rule) \n");
        fprintf(stdout, "2. IMSI View       (File Dump by IMSI Search Rule)\n");
        fprintf(stdout, "3. SEQUENCE View   (File Dump By UDRSEQUENCE Search Rule) \n");
        fprintf(stdout, "0. EXIT \n");
        fprintf(stdout, "==========================================================\n");
        fprintf(stdout, "Input : ");
        fflush(stdout);
        
        while (fgets(input_buf, sizeof(input_buf), stdin) == NULL)
        {
        }
        input_buf[strlen(input_buf)-1] = 0;

        iMainSelect = strtol(input_buf, NULL,10);
        switch(iMainSelect)
        {
            case 1:
                FileView();
                break;
            case 2:
                ImsiView();
                break;
            case 3:
                SequenceView();
                break;

            case 0:
            default:
                fprintf(stdout, "Program Terminated\n");
                fflush(stdout);
                exit(-1);
                break;
        }
    }

    return 0;
}

int ProcessOriProcess()
{
    int iRet = 0;
    FILE *Outfd = 0;
    int i = 0;
	char	*tmpChar = NULL;
	char	szTmpCmdName[256];
    
    PASS_EXCEPTION((Outfd = fopen(gSrchInfo.szOutFile, "w")) == NULL,
        FOPEN_FAIL);
    
    /* Write Title */
    if( gSrchInfo.iViewType == VIEWTYPE_LINE )
    {
        PASS_EXCEPTION(write_title(Outfd, &gSrchInfo) != 0, FILE_WRITE_FAILURE);
    }

    if( gSrchInfo.iSrchType == SRCHTYPE_TIME )
    {
        PASS_EXCEPTION( srch_file(&gSrchInfo, Outfd) != 0, DIR_SRCH_FAIL);
    }

    if( gSrchInfo.iSrchType == SRCHTYPE_FILE )
    {
        for(i=0; i<gSrchInfo.iFileCnt; i++)
        {
			fprintf(stdout, "%s Dump Start\n", gSrchInfo.szFileInfo[i]);
			fflush(stdout);
			tmpChar = strrchr(gSrchInfo.szFileInfo[i], '/');
			tmpChar++;
			if( strlen(tmpChar) == UDR_GZFILE_LEN )
			{
				snprintf(szTmpCmdName, 256, "gunzip %s", gSrchInfo.szFileInfo[i]);
				gSrchInfo.szFileInfo[i][strlen(gSrchInfo.szFileInfo[i])-3] = '\0';
				system(szTmpCmdName);
				srch_msg(gSrchInfo.szFileInfo[i], Outfd, &gSrchInfo);
				snprintf(szTmpCmdName, 256, "gzip %s", gSrchInfo.szFileInfo[i]);
				system(szTmpCmdName);
			}
			else
			{
				srch_msg(gSrchInfo.szFileInfo[i], Outfd, &gSrchInfo); 
			}


        }
    }


    PASS_CATCH(FOPEN_FAIL)
    fprintf(stdout, "Fopen Failure [%s]\n", gSrchInfo.szOutFile);
    iRet = -1;
    
    PASS_CATCH(DIR_SRCH_FAIL)
    fprintf(stdout, "Directory Search Failure [%s]\n", gSrchInfo.szDirectory);
    iRet = -1;
    
    PASS_CATCH(FILE_WRITE_FAILURE)
    fprintf(stdout, "File Write Failure [%s]\n", gSrchInfo.szOutFile);
    iRet = -1;
    

    PASS_CATCH_END;

    return iRet;

}


/*------------------------------------------------------------------------------
* FUNCTIONS   : main
* SYNOPSIS    :
* PARAMETERS  :
* RETURNS     :
* REMARKS     :
* ----------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    int iRet = 0;

    /* 20040819,poopee */
    init_sig();
    if( argc > 1 )
    {
        memset(&gSrchInfo, 0x00, sizeof(st_SrchInfo_t));
        PASS_EXCEPTION(get_para(argc,argv) < 0, END_PROCESS);
        ProcessOriProcess();
    }
    else
    {
        StartMainMenu();
    }


    PASS_CATCH(END_PROCESS)
    usage();
    exit(0);
    
    PASS_CATCH(SEARCH_FAIL)
    fprintf(stderr,"SEARCH ERROR!! \n");
    fflush(stdout);
    iRet = -1;
    
    PASS_CATCH_END;

    return iRet;
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
    /*
    fclose(SRCHINFO.fp);
    fclose(SRCHINFO.err_fp);        
    */
    /* 20040923,poopee */
    exit(signal == SIGTERM ? 0 : 1);
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


/*------------------------------------------------------------------------------
* FUNCTIONS   :
* SYNOPSIS    :
* PARAMETERS  :
* RETURNS     :
* REMARKS     :
* ----------------------------------------------------------------------------*/
void usage()
{
    fprintf(stderr,"SRCHMSG ((-s START_TIME -e END_TIME) | -f INPUT_FILE) [-i IMSI] -v VIEW_TYPE [-o RESULT_FILE]\n");

    fprintf(stderr,"       VERSION  : R1.0.0\n");
    fprintf(stderr,"       TIME     : YYMMDDhhmm\n");
    fprintf(stderr,"       VIEW_TYPE: NORM | LINE\n");
    
}

int get_para(int cnt, char *para[])
{
    int             arg_val;
    char            start[11], end[11], udrSeq[11];
    time_t          timet_val;
    char            *ptr;
    int             iRet = 0;

    PASS_EXCEPTION(cnt == 1, INVALID_PARA);

    start[0] = 0; end[0] = 0;
    
    while ((arg_val=getopt(cnt,para,"s:e:f:i:u:t:v:o:x")) != EOF)
    {
        switch (arg_val)
        {
            case 's':
                fprintf (stderr ,"\n");
                PASS_EXCEPTION((gSrchInfo.tStartTime = get_timet(optarg)) == 0, GET_START_TIME_ERR);
                strcpy(start,optarg);
                gSrchInfo.iSrchType = SRCHTYPE_TIME;
                break;
            
            case 'e':
                PASS_EXCEPTION((gSrchInfo.tEndTime = get_timet(optarg)) == 0, GET_END_TIME_ERR);
                strcpy(end,optarg);
                break;

            case 'f':
                PASS_EXCEPTION(strlen(optarg) > MAX_FILENAME_LEN, FILENAME_LEN_ERR);
                snprintf(gSrchInfo.szFileInfo[0], MAX_FILENAME_LEN, "%s", optarg);
                gSrchInfo.iFileCnt = 1;
                gSrchInfo.iSrchType = SRCHTYPE_FILE;
                break;

            case 'i':
                PASS_EXCEPTION(chk_number(optarg,MAX_IMSI_LEN) < 0, 
                        IMSI_LEN_ERR);
                strcpy(gSrchInfo.szImsiNum,optarg);
                gSrchInfo.iSrchKeyType = SRCHKEY_IMSI;
                break;
            

            case 'v':
                if (!strcasecmp(optarg,"NORM"))
                    gSrchInfo.iViewType = VIEWTYPE_NORM;
                else if (!strcasecmp(optarg,"LINE"))
                    gSrchInfo.iViewType = VIEWTYPE_LINE;
                else
                {
                    PASS_EXCEPTION(1, UNKNOWN_VIEW_TYPE);
                }

                break;

            case 'o':
                PASS_EXCEPTION(strlen(optarg) > MAX_FILENAME_LEN, MAX_FILENAME_ERR);
                strcpy(gSrchInfo.szOutFile,optarg);
                break;
            
            case 'u':
                PASS_EXCEPTION(chk_number(optarg,11) < 0, 
                        SEQ_LEN_ERR);
                gSrchInfo.uiUdrSeq = strtol(optarg, 0, 10);
                gSrchInfo.iSrchKeyType = SRCHKEY_IMSI;
                break;

            default:
                PASS_EXCEPTION(1, INVALID_OPT);

        }
    }

    /* 필수 파라미터 컴사 */
    PASS_EXCEPTION(gSrchInfo.iViewType == 0 , MANDATORY_PARA_ERR);

    /* 파일 검색조건이 입력되어있는지 검사 : 파일명 또는 시간*/
    /* 조건이 없는 경우 */
    PASS_EXCEPTION(gSrchInfo.iSrchType == 0, 
        TIME_OR_FILE_NOT_FOUND);

    /* 파일명과 시간 조건이 모두 입력된 경우 */

    /* 파일 검색 조건이 시간일 경우 시작과 종료 시간이 입력되었는지 검사*/
    PASS_EXCEPTION( (gSrchInfo.iSrchType == SRCHTYPE_TIME) &&
        ( gSrchInfo.tStartTime == 0 || gSrchInfo.tEndTime == 0 ), 
            START_OR_END_TIME_NOT_FOUND);

    
    /* 가입자 검색 조건 MSID */
    
    /* 출력 파일명이 입력되지 않았을경우
       default파일명을 지정 */
    if ( strlen(gSrchInfo.szOutFile) == 0)
    {
        if (gSrchInfo.iSrchType != SRCHTYPE_FILE)
        {
            if( gSrchInfo.iSrchType == SRCHTYPE_TIME )
            {
                sprintf(gSrchInfo.szOutFile, "%s/SRCHMSG_%s_%s_%s.RSLT",
                    RESULT_PATH,VIEWTYPE_NAME[gSrchInfo.iViewType],
                    start,end);
            }
		
            if( gSrchInfo.iSrchKeyType == SRCHKEY_IMSI )
            {
                sprintf(gSrchInfo.szOutFile, "%s/SRCHMSG_%s_%s_%s_IMSI%s.RSLT",
                    RESULT_PATH,VIEWTYPE_NAME[gSrchInfo.iViewType],
                    start,end,
                    gSrchInfo.szImsiNum);
            }
            else if( gSrchInfo.iSrchKeyType == SRCHKEY_SEQ )
            {
                sprintf(gSrchInfo.szOutFile,"%s/SRCHMSG_%s_%s_%s_SEQ%d.RSLT",
                    RESULT_PATH,VIEWTYPE_NAME[gSrchInfo.iViewType],
                    start,end,
                    gSrchInfo.uiUdrSeq);
            }
        }
        else
        {
            if ((ptr=strrchr(gSrchInfo.szFileInfo[0],'/')) == NULL)
                ptr = gSrchInfo.szFileInfo[0];    /* logical file path */
            else
                ptr += 1;

            sprintf(gSrchInfo.szOutFile,"%s/SRCHMSG_%s_%s.RSLT",
                RESULT_PATH,VIEWTYPE_NAME[gSrchInfo.iViewType],
                ptr);

        }


    }
    fprintf(stderr,"     : RESULT-FILE       = %s\n",gSrchInfo.szOutFile);

    if( gSrchInfo.iSrchType == SRCHTYPE_TIME )
    {
#ifdef __UDRCOLL__
        //MakeDirectoryName(gSRchInfo.szDirectory, gSrchInfo.start);
#else
        snprintf(gSrchInfo.szDirectory, MAX_FILENAME_LEN, "%s", UDRLOG_PATH);
#endif
    }

    PASS_CATCH(INVALID_PARA)
    iRet = -1;
    
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
                
    PASS_CATCH(INVALID_OPT)
    fprintf(stderr,"ERROR: invalid option(%d)!!!\n",arg_val); 
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


