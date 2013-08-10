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
#include <ipaf_svc.h>
#include <ipam_headlog.h>
#include "srchmsg_aaa.h"

char szSYSNAME[5];

extern int              errno;
extern SRCH_INFO        SRCHINFO;
extern int              XFLAG;

/* challa(20060922) ---> */
extern int              dDirectory;
extern char             szDate[5];
/* <--- */


void 	proc_srchmsg();
void 	srch_file(char *dirpath);
void 	srch_msg(char *filename);
void 	write_title();
int 	is_headlog(char *filename);
int 	write_msg(st_LogDataHead *data_head, st_MsgQ *qmsg);
time_t 	get_ctime(char *filename);

/* SYSNAME BSDA/BSDB (20060921,challa) */
int 	dFindSysName( );

extern char *str_time(time_t t);

/*------------------------------------------------------------------------------
* FUNCTIONS   :
* SYNOPSIS    :
* PARAMETERS  :
* RETURNS     :
* REMARKS     :
* ----------------------------------------------------------------------------*/
void proc_srchmsg()
{
    char        path[MAX_FILENAME_LEN+1], targetDir[MAX_FILENAME_LEN+1];
    char        e_month, e_day;
    time_t      tmp_timet;
    struct tm   *bdtime;
    int         fd;
	int         dRet;
    struct stat statbuf;

	/* 20060921, add(challa) divide for SysName */
	dRet = dFindSysName ( );
	if( dRet == 0 )
		sprintf(path,"%sAAA/%s",HEADLOG_PATH, szSYSNAME);


    if ((SRCHINFO.fp=fopen(SRCHINFO.outfile,"w")) == NULL)
    {
        fprintf(stderr,"ERROR: file(%s) open error(%s)!!!\n",
            SRCHINFO.outfile,strerror(errno));
        exit(1);
    }

    if ((SRCHINFO.err_fp=fopen(SRCHINFO.errfile,"w")) == NULL)
    {
        fprintf(stderr,"ERROR: file(%s) open error(%s)!!!\n",
            SRCHINFO.errfile,strerror(errno));
        exit(1);
    }

    if (SRCHINFO.viewtype == VIEWTYPE_LINE) 
        write_title();
    if (SRCHINFO.srchtype == SRCHTYPE_FILE)
        srch_msg(SRCHINFO.filename);
    else
    {
        bdtime    = localtime(&SRCHINFO.endtime);
        e_month   = bdtime->tm_mon + 1;
        e_day     = bdtime->tm_mday;

        tmp_timet = SRCHINFO.starttime;
        bdtime    = localtime(&tmp_timet);
        while (1)
        {
            sprintf(targetDir,"%s/%02d%02d",path, bdtime->tm_mon+1, bdtime->tm_mday);
            fprintf(stderr,"     : DIRECTORY         : %s\n",targetDir);

            srch_file(targetDir);

            tmp_timet += 86400; 
            bdtime = localtime(&tmp_timet);
            
            if (tmp_timet > SRCHINFO.endtime) break;
        }
    }

    fclose(SRCHINFO.fp);
    fclose(SRCHINFO.err_fp);

    if (stat(SRCHINFO.errfile,&statbuf) != -1)
        if (statbuf.st_size == 0) unlink(SRCHINFO.errfile);

    fprintf(stderr,"     : ------------------------------------------------------------------\n");

    fprintf(stderr,"     : FINAL RESULT      = FILE:%d MESSAGE:%d FOUND:%d WRITTEN:%d\n",
        SRCHINFO.tot_file,SRCHINFO.tot_msg,SRCHINFO.tot_found,SRCHINFO.tot_written);

    if (SRCHINFO.tot_written > 65535)
        fprintf (stderr, "[SPLIT RESULT] split -l 65535 [RESULT_FILE]");

}


/*------------------------------------------------------------------------------
* FUNCTIONS   :
* SYNOPSIS    :
* PARAMETERS  :
* RETURNS     :
* REMARKS     :
* ----------------------------------------------------------------------------*/

int dFindSysName( )
{
	if( dDirectory == 1 )
	{
		strncpy( szSYSNAME, "BSDA", 4 );
		szSYSNAME[4] = '\0';
	}
	else if( dDirectory == 2 )
	{
		strncpy( szSYSNAME, "BSDB", 4 );
		szSYSNAME[4] = '\0';
	}
	else
		return -1;

	return 0;
}

	

void srch_file(char *dirpath)
{
    DIR         *dirp;
    struct dirent   *direntp;
    char        filename[MAX_FILENAME_LEN+1];

    if ((dirp=opendir(dirpath)) == NULL)
    {
        fprintf(stderr,"ERROR: directory(%s) open error(%s)!!!\n",dirpath,strerror(errno));
        return;
    }

    while ((direntp=readdir(dirp)) != NULL)
    {
        if (!strcmp(direntp->d_name,".") || !strcmp(direntp->d_name,"..")) 
            continue;

        if (is_headlog(direntp->d_name) < 0) 
            continue;

        sprintf(filename,"%s/%s",dirpath,direntp->d_name);

        srch_msg(filename);
    }

    closedir(dirp);
}

/*------------------------------------------------------------------------------
* FUNCTIONS   :
* SYNOPSIS    :
* PARAMETERS  :
* RETURNS     :
* REMARKS     :
* ----------------------------------------------------------------------------*/

int is_headlog(char *filename)
{
    int         len, i, length;
    char        *ptr = 0;

	char        tmp_file[16];


	/* FILE_NAME Length */
    ptr = strstr(filename,".gz");
    if (ptr != NULL && !strcmp(ptr,".gz"))
        length = 22;
    else
        length = 19;

    len = strlen(filename);
    if (len != length)      /* AAA_YYYYmmdd_HHMMSS */
    {
        fprintf(stderr,"ERROR: invalid headlog filename(%s) length(%d)!!!\n",filename,len);
        return -1;
    }

    if (len > 19) len = 19;

	/* TIME SELECT :challa(20060921) */
	strncpy( tmp_file, &filename[4], 15 );
	tmp_file[15] = '\0';
	len = strlen(tmp_file);
	/* <--- */

    for (i=0; i<len; i++)
    {
        if (!isdigit(tmp_file[i]) && tmp_file[i] != '_')
        {
            fprintf(stderr,"ERROR: invalid headlog filename(%s)!!!\n",filename);
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

time_t get_ctime(char *filename)
{
    int         len, i, tmp_val;
    char        tmp_str[5];
    struct tm   bdtime;
	char        *tmp_filename = NULL;
	char        *ptr = NULL;

	/*
    ptr = strrchr(filename,'/');
    ptr += 1;
	*/

	ptr = strchr(filename,'_');   /* first pointer of '_'  return */
	ptr += 1;

	tmp_filename = ptr;
	memcpy(tmp_str,tmp_filename,4);      // year

    tmp_str[4] = 0;
    tmp_val = atoi(tmp_str);
    if (tmp_val < 1970 || tmp_val > 9999)
    {
        //fprintf(stderr,"ERROR: invalid year value(%d)!!!\n",tmp_val);
        return 0;
    }
    bdtime.tm_year = tmp_val - 1900;

    memcpy(tmp_str,tmp_filename+4,2);    // month
    tmp_str[2] = 0;
    tmp_val = atoi(tmp_str);
    if (tmp_val < 1 || tmp_val > 12)
    {
        fprintf(stderr,"ERROR: invalid month value(%d)!!!\n",tmp_val);
        return 0;
    }
    bdtime.tm_mon = tmp_val - 1;

    memcpy(tmp_str,tmp_filename+6,2);    // day
    tmp_str[2] = 0;
    tmp_val = atoi(tmp_str);
    if (tmp_val < 1 || tmp_val > 31)
    {
        fprintf(stderr,"ERROR: invalid day value(%d)!!!\n",tmp_val);
        return 0;
    }
    bdtime.tm_mday = tmp_val;

    memcpy(tmp_str,tmp_filename+8+1,2);    // hour
    tmp_str[2] = 0;
    tmp_val = atoi(tmp_str);
    if (tmp_val < 0 || tmp_val > 23)
    {
       fprintf(stderr,"ERROR: invalid hour value(%d)!!!\n",tmp_val);
       return 0;
    }
    bdtime.tm_hour = tmp_val;

    memcpy(tmp_str,tmp_filename+10+1,2);   // min
    tmp_str[2] = 0;
    tmp_val = atoi(tmp_str);
    if (tmp_val < 0 || tmp_val > 59)
    {
        fprintf(stderr,"ERROR: invalid min value(%d)!!!\n",tmp_val);
        return 0;
    }
    bdtime.tm_min = tmp_val;

    memcpy(tmp_str,tmp_filename+12+1,2);   // sec
    tmp_str[2] = 0;
    tmp_val = atoi(tmp_str);
    if (tmp_val < 0 || tmp_val > 59)
    {
        //fprintf(stderr,"ERROR: invalid sec value(%d)!!!\n",tmp_val);
        return 0;
    }

    bdtime.tm_sec = tmp_val;
    bdtime.tm_isdst = 0;

    return (mktime(&bdtime));

}



/*------------------------------------------------------------------------------
* FUNCTIONS   :
* SYNOPSIS    :
* PARAMETERS  :
* RETURNS     :
* REMARKS     :
* ----------------------------------------------------------------------------*/

void srch_msg(char *filename)
{
    int         fd, len=0, gz_flag = 0, ret;
    int         tot_msg=0, tot_found=0, tot_written=0;
    char        tmp_filename[MAX_FILENAME_LEN+1], command[128], *ptr;

    st_LogFileHead  file_head;
    st_LogDataHead  data_head;
    st_MsgQ         qmsg;
    st_AAAREQ       stAAA;

    struct stat statbuf;
    time_t      start_time;

    memset( &qmsg, 0x00, sizeof(st_MsgQ) );
    
    if (stat(filename,&statbuf) < 0)
    {
        fprintf(stderr,"ERROR: stat(%s) error(%s)!!!\n",filename,strerror(errno));
        return;
    }

    if (SRCHINFO.srchtype == SRCHTYPE_TIME)
    {
        if (XFLAG == 0 && statbuf.st_mtime < SRCHINFO.starttime)
        {
			/*
            fprintf(stderr,"DEBUG: %s, file end(%d) before start time(%d)\n",
                    filename,statbuf.st_mtime,SRCHINFO.starttime);
			*/
            return;
        }

        if ((start_time=get_ctime(filename)) > SRCHINFO.endtime)
        {
			/*
            fprintf(stderr,"DEBUG: %s, file start(%d) after end time(%d), 1\n",
                    filename,start_time,SRCHINFO.starttime);
			*/

            return;
        }
    }

    len = strlen(filename);
    if (!strcmp(filename+len-3,".gz"))
    {
        strncpy(tmp_filename,filename,len-3);
        tmp_filename[len-3] = 0;

        sprintf(command,"gunzip %s.gz",tmp_filename);
        system(command);
        gz_flag = 1;
    }
    else
        strcpy(tmp_filename,filename);



    if ((fd=open(tmp_filename,O_RDONLY)) < 0)
    {
        fprintf(stderr,"ERROR: file(%s) open error(%s)!!!\n",tmp_filename,strerror(errno));
        if (gz_flag == 1)
        {
            sprintf(command,"gzip %s",tmp_filename);
            system(command);
        }
        return;
    }
    
    if ((len=read(fd,(void *)&file_head,sizeof(st_LogFileHead))) <= 0)
    {
        fprintf(stderr,"ERROR: file(%s) read error(%s)!!!\n",tmp_filename,strerror(errno));
        close(fd);
        if (gz_flag == 1)
        {
            sprintf(command,"gzip %s",tmp_filename);
            system(command);
        }
        return;
    }



    if (SRCHINFO.srchtype == SRCHTYPE_TIME && XFLAG == 1 && 
        file_head.tStartTime > SRCHINFO.endtime)
    {
		/*
        fprintf(stderr,"DEBUG: %s, file start(%d) after end time(%d), 2\n",
            filename,file_head.tStartTime,SRCHINFO.endtime);
		*/

        return;
    }

/*
    if (SRCHINFO.srchtype == SRCHTYPE_TIME && file_head.tStartTime > SRCHINFO.endtime)
    {
        //fprintf(stderr,"DEBUG: %s, file start(%d) after end time(%d), 2\n",filename,file_head.tStartTime,SRCHINFO.endtime);
        return;
    }
*/

    SRCHINFO.tot_file++;
    fprintf(stderr,"     : FILE              : %s\n",filename);

    while(1)
    {
        len = read(fd,(void *)&data_head,sizeof(st_LogDataHead));
        if (len <= 0)
        {
            if (len < 0)
                fprintf(stderr,"ERROR_1: file(%s) read error(%s)!!!\n",
                        filename,strerror(errno));
            close(fd);
            break;
        }

        len = read(fd, (void*)&qmsg, CVT_INT_CP(data_head.uiLogLen));
        if (len <= 0 || len != CVT_INT_CP(data_head.uiLogLen))
        {
        	if (len < 0)
            	fprintf(stderr,"ERROR_2: file(%s) read error(%s)!!!\n", filename,strerror(errno));
            else if (len != CVT_INT_CP(data_head.uiLogLen))
                fprintf(stderr,"ERROR: file(%s) read length mismatch!!!\n",filename);

            close(fd);
            break;
        }

		/*
        if (CVT_SHORT_CP(data_head.usMType) == DEF_SVC)
        {

            len = read(fd, (void*)&qmsg, CVT_INT_CP(data_head.uiLogLen));
            if (len <= 0 || len != CVT_INT_CP(data_head.uiLogLen))
            {
                if (len < 0)
                    fprintf(stderr,"ERROR_2: file(%s) read error(%s)!!!\n",
                            filename,strerror(errno));
                else if (len != CVT_INT_CP(data_head.uiLogLen))
                    fprintf(stderr,"ERROR: file(%s) read length mismatch!!!\n",filename);

                close(fd);
                break;
            }
        }
        else
        {
            fprintf(stderr,"ERROR: datahead's MType(%d) is not DEF_SVC time(%s)!!!\n",
                    CVT_SHORT_CP(data_head.usMType), 
                    str_time(CVT_INT(data_head.stTmval.tv_sec)));
            continue;
        }
		*/

        SRCHINFO.tot_msg++;
        tot_msg++;

        if (SRCHINFO.srchtype == SRCHTYPE_TIME)
        {
            if ((CVT_INT_CP(data_head.stTmval.tv_sec) < SRCHINFO.starttime)
                || (CVT_INT_CP(data_head.stTmval.tv_sec) > SRCHINFO.endtime))

            continue;
        }


        switch (SRCHINFO.numtype)
        {
        case NUMTYPE_IMSI:
			/*
            if (qmsg.szMIN[0] == 0) 
                fprintf(stderr,"ERROR: MSID not exist in log header\n");
			*/

            if (!strcmp( SRCHINFO.imsi, (char *)qmsg.szMIN) )
            {
                SRCHINFO.tot_found++;
                tot_found++;
                if ((ret=write_msg(&data_head,&qmsg)) == 0)
                {
                    SRCHINFO.tot_written++;
                    tot_written++;
                }
                else if (ret == -1)
                {
                    fprintf(SRCHINFO.err_fp,"FILENAME: %s",filename);
                    //print_hex_log(SRCHINFO.err_fp, qmsg.szBody, qmsg.usBodyLen);
                }
            }

            break;

        case NUMTYPE_MSISDN:
            if (qmsg.szExtra[0] == 0) 
                    fprintf(stderr,"ERROR: MDN not exist in log header\n");

            if (!strcmp(SRCHINFO.msisdn, (char *)qmsg.szExtra))
            {
                SRCHINFO.tot_found++;
                tot_found++;
                if ((ret=write_msg(&data_head,&qmsg)) == 0)
                {
                    SRCHINFO.tot_written++;
                    tot_written++;
                }
                else if (ret == -1)
                {
                    fprintf(SRCHINFO.err_fp,"FILENAME: %s",filename);
                    //print_hex_log(SRCHINFO.err_fp, qmsg.szBody, qmsg.usBodyLen);
                }
            }
            break;

        default:
            SRCHINFO.tot_found++;
            tot_found++;

            if ((ret=write_msg(&data_head,&qmsg)) == 0)
            {
                SRCHINFO.tot_written++;
                tot_written++;
            }
            else if (ret == -1)
            {
                fprintf(SRCHINFO.err_fp,"FILENAME: %s",filename);
                //print_hex_log(SRCHINFO.err_fp, qmsg.szBody, qmsg.usBodyLen);
            }

            break;
        }
    }

    close(fd);

    if (gz_flag == 1)
    {
        sprintf(command,"gzip %s",tmp_filename);
        system(command);
    }

    if ((ptr=strrchr(filename,'/')) == NULL)
        ptr = filename;
    else
        ptr += 1;
    fprintf(stderr,"     : %-15s   : MESSAGE=%d FOUND=%d WRITTEN=%d\n",
            ptr,tot_msg,tot_found,tot_written);           

    return;
}


/*------------------------------------------------------------------------------
* FUNCTIONS   :
* SYNOPSIS    :
* PARAMETERS  :
* RETURNS     :
* REMARKS     :
* ----------------------------------------------------------------------------*/
void write_title()
{
    char        buffer[1024*20];
    int         len;

	sprintf(buffer,"Timestamp< System< Message-Type< CODE< ID< LEN< AUTHENTICATOR< TIMESTAMP< UDR-SEQUENCE<ACCT-RESULT-REASON<");
   	len = strlen(buffer);

	sprintf(&buffer[len],"CALLING-STATION-ID< ESN< FRAMED-IP-ADDRESS< USER-NAME< ACCT-SESSION-ID< CORRELATION-ID<");
   	len = strlen(buffer);

	sprintf(&buffer[len],"SESSION-CONT< BEGINNING-SESSION< HA-IP-ADDR< NAS-IP-ADDRESS< PCF-IP-ADDR< BSID< USER-ID< ");
   	len = strlen(buffer);

	sprintf(&buffer[len],"F_FCH_MUX< R_FCH_MUX< SERVICE-OPTION< FTYPE< RTYPE< ");
   	len = strlen(buffer);

	sprintf(&buffer[len],"FFSIZE< FRC< RRC< IP-TECH< COMP-FLAG< REASON-IND< DFSIZE< ");
   	len = strlen(buffer);

	sprintf(&buffer[len],"ALWAYS-ON< ACCT-OUTPUT-OCTETS< ACCT-INPUT-OCTETS< BAD-FRAME-COUNT<EVENT-TIMESTAMP< ACTIVE-TIME< ");
   	len = strlen(buffer);

	sprintf(&buffer[len],"NUM-ACTIVE< SDB-INPUT-OCTETS< SDB-OUTPUT-OCTETS< NUMSDB-INPUT< NUMSDB-OUTPUT< NUM-BYTES-RECEIVED-TOTAL< ");
   	len = strlen(buffer);

	sprintf(&buffer[len],"MIP-SIGNALING-INBOUND-COUNT< MIP-SIGNALING-OUTBOUND-COUNT< IP-QOS< AIR-PRIORITY< ACCT-INPUT-PACKETS< ACCT-OUTPUT-PACKETS< ");
   	len = strlen(buffer);

	sprintf(&buffer[len],"R-P-CONNECTION-ID< ACCT-AUTHENTIC< ACCT-SESSION-TIME< ACCT-TERMINATE-CAUSE< ACCT-STATUS-TYPE< ");
   	len = strlen(buffer);

    sprintf (&buffer[len], "NAS-PORT-TYPE< NAS-PORT< NAS-PORT-ID< SERVICE-TYPE< ACCT-DELAY-TIME< C23-BIT< H-BTI< RETRY_FLAG< ");
    len = strlen(buffer);

	sprintf(&buffer[len],"DATA-SERVICE-TYPE< TRANSACTION-ID< REQUEST-TIME< RESPONSE-TIME< SESSION-TIME< SERVER-IP-ADDRESS< SERVER-PORT< TERMINAL-PORT< ");
    len = strlen(buffer);

	sprintf(&buffer[len],"URL< DOWNLOAD-TYPE< APPLICATION-ID< CONTENT-ID< METHOD-TYPE< RESULT-CODE< IP-LAYER-UPLOAD-SIZE< IP-LAYER-DOWMLOAD-SIZE< TCP-LAYER-RETRANS-INPUT-SIZE< TCP-LAYER-RETRANS-OUTPUT-SIZE< USE-COUNT< USE-TIME< TOTAL-SIZE< TOTAL-TIME<" );
    len = strlen(buffer);

	/* 20070104 Add: NEW UDR */
	sprintf(&buffer[len],"AUDIO-UPLOAD-SIZE< AUDIO-DOWNLOAD-SIZE< VIDEO-UPLOAD-SIZE< VIDEO-DOWNLOAD-SIZE< CALLEE-MIN< CALLER-MIN< ");
	len = strlen(buffer);
	/* <--- */

	sprintf(&buffer[len],"TRANSACTION-CONTENT-LENGTH< TRANSACTION-COMPLETENESS< UDR-GENERATION-REASON< USER-AGENT< DOWNLOAD-INFO< \n");
   	len = strlen(buffer);

#if 0	// 070530, poopee
	sprintf(&buffer[len],"DATA-SERVICE-TYPE< TRANSACTION-ID< REQUEST-TIME< RESPONSE-TIME< SESSION-TIME< SERVER-IP-ADDRESS< SERVER-PORT< TERMINAL-PORT< ");
    len = strlen(buffer);

	sprintf(&buffer[len],"URL< DOWNLOAD-TYPE< APPLICATION-ID< CONTENT-ID< METHOD-TYPE< RESULT-CODE< IP-LAYER-UPLOAD-SIZE< IP-LAYER-DOWMLOAD-SIZE< TCP-LAYER-RETRANS-INPUT-SIZE< TCP-LAYER-RETRANS-OUTPUT-SIZE< USE-COUNT< USE-TIME< TOTAL-SIZE< TOTAL-TIME<" );
   	len = strlen(buffer);

	/* 20070104 Add: NEW UDR */
	sprintf(&buffer[len],"AUDIO-UPLOAD-SIZE< AUDIO-DOWNLOAD-SIZE< VIDEO-UPLOAD-SIZE< VIDEO-DOWNLOAD-SIZE< ");
	len = strlen(buffer);
	/* <--- */

	sprintf(&buffer[len],"TRANSACTION-CONTENT-LENGTH< TRANSACTION-COMPLETENESS< UDR-GENERATION-REASON< USER-AGENT< DOWNLOAD-INFO< CALLEE-MIN< CALLER-MIN< ");
    len = strlen(buffer);

	sprintf(&buffer[len],"DATA-SERVICE-TYPE< TRANSACTION-ID< REQUEST-TIME< RESPONSE-TIME< SESSION-TIME< SERVER-IP-ADDRESS< SERVER-PORT< TERMINAL-PORT< ");
    len = strlen(buffer);

	sprintf(&buffer[len],"URL< DOWNLOAD-TYPE< APPLICATION-ID< CONTENT-ID< METHOD-TYPE< RESULT-CODE< IP-LAYER-UPLOAD-SIZE< IP-LAYER-DOWMLOAD-SIZE< TCP-LAYER-RETRANS-INPUT-SIZE< TCP-LAYER-RETRANS-OUTPUT-SIZE< USE-COUNT< USE-TIME< TOTAL-SIZE< TOTAL-TIME<" );
   	len = strlen(buffer);

	/* 20070104 Add: NEW UDR */
	sprintf(&buffer[len],"AUDIO-UPLOAD-SIZE< AUDIO-DOWNLOAD-SIZE< VIDEO-UPLOAD-SIZE< VIDEO-DOWNLOAD-SIZE< ");
	len = strlen(buffer);
	/* <--- */

	sprintf(&buffer[len],"TRANSACTION-CONTENT-LENGTH< TRANSACTION-COMPLETENESS< UDR-GENERATION-REASON< USER-AGENT< DOWNLOAD-INFO< CALLEE-MIN< CALLER-MIN\n ");
	len = strlen(buffer);
#endif


    if (fprintf(SRCHINFO.fp,"%s",buffer) < 0)
    {
        fprintf(stderr,"ERROR: file(%s) write error(%s)!!!\n",
                SRCHINFO.outfile,strerror(errno));
        fclose(SRCHINFO.fp);
        fclose(SRCHINFO.err_fp);    
        exit(1);
    }

    return;
}




/*------------------------------------------------------------------------------
* FUNCTIONS   :
* SYNOPSIS    :
* PARAMETERS  :
* RETURNS     :
* REMARKS     :
* ----------------------------------------------------------------------------*/
int write_msg(st_LogDataHead *data_head, st_MsgQ *qmsg )
{
    st_AAAREQ   aaa_msg;

    int         ret, adr;
    int         dUDRCnt = 0;
	int         dRcvLen = 0;
    char        IMSI[16];
	int         dTime;

    memset( &aaa_msg, 0x00, sizeof(st_AAAREQ) );

   	data_head->usINFType 	= CVT_SHORT_CP(data_head->usINFType);
	qmsg->usBodyLen 		= CVT_SHORT_CP(qmsg->usBodyLen );
    dUDRCnt 				= (int)data_head->usINFType;


	if ((ret=ParsingRadiusAAA( qmsg->szBody, qmsg->usBodyLen, 0, &aaa_msg, dUDRCnt )) < 0)
    {
    	fprintf(stderr,"ERROR: ParsingRadiusAAA() error(ret=%d)\n",ret);
        return -1;
	}

    if (print_aaa_msg(data_head,&aaa_msg) < 0)
    {
		fprintf(stderr,"ERROR: AAA message print error");
        return -1;
	}


    return 0;
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
