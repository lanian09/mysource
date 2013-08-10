/*******************************************************************************
            DQMS Project

    Author   : Jae Seung Lee
    Section  : SI_SVC
    SCCS ID  : @(#)si_svc_main.c    1.1
    Date     : 01/21/05
    Revision History :
        '05. 01. 21     Initial
        '08. 01. 07     Update By LSH for review
        '08. 01. 14     Add By LSH for IUPS NTAM        

    Description :   

    Copyright (c) uPRESTO 2005
*******************************************************************************/
    
/***** A.1 * File Include *******************************/
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

// DQMS
#include "common_stg.h"
#include "capdef.h"
#include "path.h"

// LIB
#include "typedef.h"
#include "loglib.h"
#include "utillib.h"

// TAM
#include "filter.h"

// .
#include "trace_comm.h"
#include "trace_func.h"

/** B.1* DEFINITION OF NEW CONSTANTS **********************/
#define PCAP_FILEHEADER_SIZE        24
char pcap_file_header[PCAP_FILEHEADER_SIZE] =
    {   0xD4, 0xC3, 0xB2, 0xA1,
        0x02, 0x00, 0x04, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0xFF, 0xFF, 0x00, 0x00,
        0x01, 0x00, 0x00, 0x00  };
/** C.1* DEFINITION OF NEW TYPES **************************/
typedef struct __pcap_data_header__
{
	unsigned int	sec;
	unsigned int	msec;
	unsigned int	datalen1;
	unsigned int	datalen2;
} pcap_data_header;
#define PCAP_DATAHEADER_SIZE		sizeof(pcap_data_header)

/** D.1* DECLARATION OF VARIABLES *************************/
extern pst_TraceFileList 	g_pstTraceFileList;
extern st_TraceList			*trace_tbl;

extern pst_TraceFileList 	g_pstServerTraceFileList;
extern st_TraceList			*ServerTrace_tbl;
extern stHASHOINFO			*pIRMINFO;
/** E.1* DEFINITION OF FUNCTIONS **************************/
/** E.2* DEFINITION OF FUNCTIONS **************************/
extern int PushTrace(pst_TraceFile pstTraceFile, time_t time, time_t mtime, int len, char *szData);
extern int FlushData(pst_TraceFile pstTraceFile);
extern void Init_Trace_Array( pst_TraceFile pstTraceFile);




int dTraceProc(const char* szData)
{
	pcap_data_header	pcaphdr;
	pst_TraceFile		pstTraceFile=NULL;
	char				buffer[MAX_TRACEBUF];

	int					i;
	st_TraceMsgHdr 		*pstTrace = (st_TraceMsgHdr *)szData;

	switch(pstTrace->dType)
	{
		case TRC_TYPE_ROAM_IMSI:
			dCvtfromIRMtoIMSI(pstTrace->stTraceID.szMIN);
			break;
		case TRC_TYPE_ROAM_MDN:
			dCvtfromIRMtoMDN(pstTrace->stTraceID.szMIN);
			break;
	}

	if ( pstTrace->dType==TRC_TYPE_IMSI )
	{
		for( i=0; i<MAX_TRACE_CNT; i++ )
		{
			if ( pstTrace->dType == g_pstTraceFileList->stTraceFile[i].stTraceInfo.dType )
			{
				if ( pstTrace->stTraceID.llIMSI == g_pstTraceFileList->stTraceFile[i].stTraceInfo.stTraceID.llIMSI )
				{
					pstTraceFile = &g_pstTraceFileList->stTraceFile[i];
					break;
				}
			}
		}
	}
	else if ( pstTrace->dType==TRC_TYPE_MDN || pstTrace->dType == TRC_TYPE_ROAM_IMSI || pstTrace->dType == TRC_TYPE_ROAM_MDN)
    {
        for( i=0; i<MAX_TRACE_CNT; i++ )
        {
            if ( pstTrace->dType == g_pstTraceFileList->stTraceFile[i].stTraceInfo.dType )
            {
                if ( strcmp( pstTrace->stTraceID.szMIN, 
						g_pstTraceFileList->stTraceFile[i].stTraceInfo.stTraceID.szMIN ) == 0 )
                {
                    pstTraceFile = &g_pstTraceFileList->stTraceFile[i];
                    break;
                }
            }
        }
    }
	else if ( pstTrace->dType==TRC_TYPE_SERV )
	{
		for( i=0; i<MAX_TRACE_CNT; i++ )
        {
            if ( pstTrace->dType == g_pstServerTraceFileList->stTraceFile[i].stTraceInfo.dType )
			{
				if ( pstTrace->stTraceID.uIP == g_pstServerTraceFileList->stTraceFile[i].stTraceInfo.stTraceID.uIP )
				{   
					pstTraceFile = &g_pstServerTraceFileList->stTraceFile[i];
					break;
				}
			}
        }
	}

	if ( pstTrace->dType==TRC_TYPE_SERV )
	{
		log_print(LOGN_INFO, "[SERVER] IP[%s] TIME[%d.%d]", util_cvtipaddr(NULL, pstTrace->stTraceID.uIP ), pstTrace->time, pstTrace->mtime );
	}
	else if (pstTrace->dType==TRC_TYPE_IMSI )
		log_print(LOGN_INFO, "[IMSI] IMSI[%llu] TIME[%d.%d]", pstTrace->stTraceID.llIMSI, pstTrace->time, pstTrace->mtime ); 
	else if (pstTrace->dType==TRC_TYPE_MDN )
		log_print(LOGN_INFO, "[MDN] MDN[%s] TIME[%d.%d]", pstTrace->stTraceID.szMIN, pstTrace->time, pstTrace->mtime ); 
	else if (pstTrace->dType==TRC_TYPE_ROAM_IMSI )
		log_print(LOGN_INFO, "[ROAM IMSI] IMSI[%s] TIME[%d.%d]", pstTrace->stTraceID.szMIN, pstTrace->time, pstTrace->mtime ); 
	else if (pstTrace->dType==TRC_TYPE_ROAM_MDN )
		log_print(LOGN_INFO, "[ROAM MDN] MDN[%s] TIME[%d.%d]", pstTrace->stTraceID.szMIN, pstTrace->time, pstTrace->mtime ); 
	
	if ( pstTraceFile==NULL )
	{
		return 0;
	}

	pstTraceFile->lastuptime = time(0);

	if ( FileCheck(pstTraceFile, pstTrace)<0 )
		return -1;

	pstTraceFile->pktcnt++;

#if 0
	pcaphdr.sec = htonl(pstTrace->time);
	pcaphdr.msec = htonl(pstTrace->mtime);
	pcaphdr.datalen1 = htonl(pstTrace->usDataLen);
	pcaphdr.datalen2 = htonl(pstTrace->usDataLen);
#endif
	pcaphdr.sec = pstTrace->time;
	pcaphdr.msec = pstTrace->mtime;
	pcaphdr.datalen1 = pstTrace->usDataLen;
	pcaphdr.datalen2 = pstTrace->usDataLen;

	if ( pstTrace->usDataLen > 1520 )
	{
		if ( pstTrace->dType==TRC_TYPE_IMSI )
			log_print(LOGN_DEBUG, "[IMSI] DATA SIZE TOO LARGE IMSI[%llu] SIZE[%hu]", 
					pstTrace->stTraceID.llIMSI, pstTrace->usDataLen );
		else if ( pstTrace->dType==TRC_TYPE_SERV )
			log_print(LOGN_DEBUG, "[SERVER] DATA SIZE TOO LARGE IP[%s] SIZE[%hu]", 
					util_cvtipaddr(NULL, pstTrace->stTraceID.uIP), pstTrace->usDataLen );
		else if ( pstTrace->dType==TRC_TYPE_MDN )
			log_print(LOGN_DEBUG, "[MDN] DATA SIZE TOO LARGE MDN[%s] SIZE[%hu]", 
					pstTrace->stTraceID.szMIN, pstTrace->usDataLen );
		else if ( pstTrace->dType==TRC_TYPE_ROAM_IMSI )
			log_print(LOGN_DEBUG, "[ROAM IMSI] DATA SIZE TOO LARGE IMSI[%s] SIZE[%hu]", 
					pstTrace->stTraceID.szMIN, pstTrace->usDataLen );
		else if ( pstTrace->dType==TRC_TYPE_ROAM_MDN )
			log_print(LOGN_DEBUG, "[ROAM MDN] DATA SIZE TOO LARGE MDN[%s] SIZE[%hu]", 
					pstTrace->stTraceID.szMIN, pstTrace->usDataLen );
	}

	memcpy( buffer, &pcaphdr, PCAP_DATAHEADER_SIZE );
	memcpy( buffer+PCAP_DATAHEADER_SIZE, szData+sizeof(st_TraceMsgHdr), pstTrace->usDataLen );
	
	PushTrace( pstTraceFile, pstTrace->time, pstTrace->mtime, pstTrace->usDataLen + PCAP_DATAHEADER_SIZE, buffer );

	return 1;
}

int FileCheck(st_TraceFile *pstTraceFile, st_TraceMsgHdr *pstTraceMsg)
{
	static char		subdir[80];
	static char		subdir2[80];
	static char 	szTime[100];
	static char 	szTmpFilename[128];

	struct stat 	stat_log;
	struct tm       check_time;
	struct tm       create_time;
	time_t			curtime;

	if ( pstTraceMsg == NULL )
		curtime = time(NULL);
	else
		curtime = pstTraceMsg->time;

	localtime_r( &curtime, &check_time );

	if ( pstTraceFile->createtime != 0 )
	{
		localtime_r( &pstTraceFile->createtime, &create_time );

		if ( check_time.tm_mon != create_time.tm_mon 
				|| check_time.tm_mday != create_time.tm_mday 
				|| check_time.tm_year != create_time.tm_year )
		{
			if ( pstTraceFile->fp!=NULL )
			{
				fclose(pstTraceFile->fp);
				pstTraceFile->fp=NULL;
			}

			pstTraceFile->pktcnt = 0;
			pstTraceFile->fp = NULL; 
		}
	}

	if ( pstTraceFile->szFilename[0]==0x0 )
	{
		sprintf( szTime, "%02d%02d", check_time.tm_mon+1, check_time.tm_mday );
		if( pstTraceFile->stTraceInfo.dType==TRC_TYPE_SERV)
		{
			sprintf( subdir, "%sSI%s/", TRACEDIR, util_cvtipaddr(NULL, pstTraceFile->stTraceInfo.stTraceID.uIP ) );

			sprintf( szTime, "%02d%02d%02d", check_time.tm_mon+1, check_time.tm_mday, check_time.tm_hour );
			sprintf( szTmpFilename, "%s%s_%s", subdir, util_cvtipaddr(NULL, pstTraceFile->stTraceInfo.stTraceID.uIP ), szTime );
		}
		else if( pstTraceFile->stTraceInfo.dType==TRC_TYPE_IMSI )
		{
			sprintf( subdir, "%sI%llu/", TRACEDIR, pstTraceFile->stTraceInfo.stTraceID.llIMSI );

			sprintf( szTime, "%02d%02d%02d", check_time.tm_mon+1, check_time.tm_mday, check_time.tm_hour );
			sprintf( szTmpFilename, "%s%llu_%s", 
				subdir, pstTraceFile->stTraceInfo.stTraceID.llIMSI, szTime );
		}
		else if( pstTraceFile->stTraceInfo.dType==TRC_TYPE_MDN )
		{
			sprintf( subdir, "%sM%s/", TRACEDIR, pstTraceFile->stTraceInfo.stTraceID.szMIN );

			sprintf( szTime, "%02d%02d%02d", check_time.tm_mon+1, check_time.tm_mday, check_time.tm_hour );
			sprintf( szTmpFilename, "%s%s_%s", 
				subdir, pstTraceFile->stTraceInfo.stTraceID.szMIN, szTime );
		}
		else if( pstTraceFile->stTraceInfo.dType==TRC_TYPE_ROAM_IMSI )
		{
			sprintf( subdir, "%sRI%s/", TRACEDIR, pstTraceFile->stTraceInfo.stTraceID.szMIN );

			sprintf( szTime, "%02d%02d%02d", check_time.tm_mon+1, check_time.tm_mday, check_time.tm_hour );
			sprintf( szTmpFilename, "%s%s_%s", 
				subdir, pstTraceFile->stTraceInfo.stTraceID.szMIN, szTime );
		}
		else if( pstTraceFile->stTraceInfo.dType==TRC_TYPE_ROAM_MDN )
		{
			sprintf( subdir, "%sRM%s/", TRACEDIR, pstTraceFile->stTraceInfo.stTraceID.szMIN );

			sprintf( szTime, "%02d%02d%02d", check_time.tm_mon+1, check_time.tm_mday, check_time.tm_hour );
			sprintf( szTmpFilename, "%s%s_%s", 
				subdir, pstTraceFile->stTraceInfo.stTraceID.szMIN, szTime );
		}

		sprintf( pstTraceFile->szFilename, "%s_%d.pcap", szTmpFilename, pstTraceFile->fcnt++ );

		/* Aleady File Exist */
		while( stat( pstTraceFile->szFilename, &stat_log )==0 )
		{
			/* Change FileName */
			sprintf( pstTraceFile->szFilename, "%s_%d.pcap", szTmpFilename, pstTraceFile->fcnt++ );
		}
	}
	
	if ( pstTraceFile->pktcnt>MAX_PKTCNTPERFILE )
	{
		if ( pstTraceFile->dUsedCnt > 0 )
			FlushData( pstTraceFile );

		pstTraceFile->pktcnt = 0;
		log_print(LOGN_CRI, "[TRACE] EXCESS MAX BEACON PKT CNT FILE[%s] CNT[%u]", 
				pstTraceFile->szFilename, pstTraceFile->pktcnt );
		fclose( pstTraceFile->fp );
		pstTraceFile->fp = NULL;
	}

	if ( pstTraceFile->fp == NULL /* File Not Open */
			|| ( stat(pstTraceFile->szFilename, &stat_log )!=0 ) ) /* File Opened But Can Not Find the File - File Move */
	{
		if ( pstTraceFile->stTraceInfo.dType==TRC_TYPE_SERV )
        {
			sprintf( subdir, "%sSI%s/", TRACEDIR, util_cvtipaddr(NULL,  pstTraceFile->stTraceInfo.stTraceID.uIP ) );
            sprintf( subdir2, "%sSI%s/", TRACEDIR, util_cvtipaddr(NULL,  pstTraceFile->stTraceInfo.stTraceID.uIP ) );

			sprintf( szTime, "%02d%02d%02d", check_time.tm_mon+1, check_time.tm_mday, check_time.tm_hour );
            sprintf( szTmpFilename, "%s%s_%s", subdir2, util_cvtipaddr(NULL,  pstTraceFile->stTraceInfo.stTraceID.uIP ), szTime );
        }
        else if ( pstTraceFile->stTraceInfo.dType==TRC_TYPE_IMSI )
		{
			sprintf( subdir, "%sI%llu/", TRACEDIR, pstTraceFile->stTraceInfo.stTraceID.llIMSI );
			sprintf( subdir2, "%sI%llu/", TRACEDIR, pstTraceFile->stTraceInfo.stTraceID.llIMSI );

			sprintf( szTime, "%02d%02d%02d", check_time.tm_mon+1, check_time.tm_mday, check_time.tm_hour );
			sprintf( szTmpFilename, "%s%llu_%s", 
				subdir2, pstTraceFile->stTraceInfo.stTraceID.llIMSI, szTime );
		}
        else if ( pstTraceFile->stTraceInfo.dType==TRC_TYPE_ROAM_IMSI )
		{
			sprintf( subdir, "%sRI%s/", TRACEDIR, pstTraceFile->stTraceInfo.stTraceID.szMIN );
			sprintf( subdir2, "%sRI%s/", TRACEDIR, pstTraceFile->stTraceInfo.stTraceID.szMIN );

			sprintf( szTime, "%02d%02d%02d", check_time.tm_mon+1, check_time.tm_mday, check_time.tm_hour );
			sprintf( szTmpFilename, "%s%s_%s", 
				subdir2, pstTraceFile->stTraceInfo.stTraceID.szMIN, szTime );
		}
        else if ( pstTraceFile->stTraceInfo.dType==TRC_TYPE_ROAM_MDN )
		{
			sprintf( subdir, "%sRM%s/", TRACEDIR, pstTraceFile->stTraceInfo.stTraceID.szMIN );
			sprintf( subdir2, "%sRM%s/", TRACEDIR, pstTraceFile->stTraceInfo.stTraceID.szMIN );

			sprintf( szTime, "%02d%02d%02d", check_time.tm_mon+1, check_time.tm_mday, check_time.tm_hour );
			sprintf( szTmpFilename, "%s%s_%s", 
				subdir2, pstTraceFile->stTraceInfo.stTraceID.szMIN, szTime );
		}
        else
		{
			sprintf( subdir, "%sM%s/", TRACEDIR, pstTraceFile->stTraceInfo.stTraceID.szMIN );
			sprintf( subdir2, "%sM%s/", TRACEDIR, pstTraceFile->stTraceInfo.stTraceID.szMIN );

			sprintf( szTime, "%02d%02d%02d", check_time.tm_mon+1, check_time.tm_mday, check_time.tm_hour );
			sprintf( szTmpFilename, "%s%s_%s", 
				subdir2, pstTraceFile->stTraceInfo.stTraceID.szMIN, szTime );
		}

		sprintf( pstTraceFile->szFilename, "%s_%d.pcap", szTmpFilename, pstTraceFile->fcnt++ );

        /* Aleady File Exist */
        while( stat( pstTraceFile->szFilename, &stat_log )==0 )
        {
			/* Change FileName */
			sprintf( pstTraceFile->szFilename, "%s_%d.pcap", szTmpFilename, pstTraceFile->fcnt++ );
        }

		/* File is not Exist */
		if ( pstTraceFile->fp!=NULL )
		{
			log_print(LOGN_WARN, "FileCheck: File Closed [%s]", pstTraceFile->szFilename);
			fclose( pstTraceFile->fp);
			pstTraceFile->fp=NULL;
		}
		
        mkdir(TRACEDIR, 0777 );

        mkdir(subdir, 0777);

        mkdir(subdir2, 0777);

		if ( ( pstTraceFile->fp=fopen(pstTraceFile->szFilename, "wb" ) ) == NULL )
		{
			log_print(LOGN_CRI, "FileCheck: Fail Open File [%s] [%s]", pstTraceFile->szFilename, strerror(errno) );
			return -1;
		}

		pstTraceFile->createtime = curtime;

		pstTraceFile->pktcnt = 0;

		log_print(LOGN_DEBUG, "FileCheck: File Open [%s]", pstTraceFile->szFilename);

		fwrite( pcap_file_header, PCAP_FILEHEADER_SIZE, 1, pstTraceFile->fp );
		fflush( pstTraceFile->fp );
	}

	return 1;
}


int Check_TraceInfo()
{
	short i, j;
	char stop, new;
    st_TraceInfo* pstTraceInfo;
    st_TraceInfo* pstTraceFile;

	/* Check Stopping Trace */
    for( i=0; i<MAX_TRACE_CNT; i++ )
    {
		pstTraceFile = &g_pstTraceFileList->stTraceFile[i].stTraceInfo;
		
		if ( pstTraceFile->dType!=TRC_TYPE_IMSI && pstTraceFile->dType!=TRC_TYPE_MDN 
				&& pstTraceFile->dType!=TRC_TYPE_ROAM_IMSI && pstTraceFile->dType!=TRC_TYPE_ROAM_MDN )
			continue;

		stop=1;

		/* dType == TRC_TYPE_IP or TRC_TYPE_IMSI or TRC_TYPE_MDN*/
		for (j=0; j<trace_tbl->count; j++ )
		{
        	pstTraceInfo = &(trace_tbl->stTraceInfo[j]);

        	if ( pstTraceInfo->dType==pstTraceFile->dType )
			{
        		if ( pstTraceInfo->dType==TRC_TYPE_IP )
				{
					if( pstTraceInfo->stTraceID.uIP==pstTraceFile->stTraceID.uIP )
					{
						stop = 0;
						break;
					}
				}
        		else
				{
					if( strncmp(pstTraceInfo->stTraceID.szMIN, pstTraceFile->stTraceID.szMIN, MIN_SIZE )==0 )
					{
						stop = 0;
						break;
					}
				}
			}
		}

		if ( stop==1 )
			StopTrace( &g_pstTraceFileList->stTraceFile[i] );
    }

	/* Check Startting Trace */
	for( j=0; j<trace_tbl->count; j++ )
	{
        pstTraceInfo = &(trace_tbl->stTraceInfo[j]);

		new = 1;

		/* dType == TRC_TYPE_IP or TRC_TYPE_IMSI or TRC_TYPE_MDN */
		for (i=0; i<MAX_TRACE_CNT; i++ )
		{
			pstTraceFile = &g_pstTraceFileList->stTraceFile[i].stTraceInfo;
			
			if ( pstTraceFile->dType!=TRC_TYPE_IP 
					&& pstTraceFile->dType!=TRC_TYPE_IMSI 
					&& pstTraceFile->dType!=TRC_TYPE_MDN 
					&& pstTraceFile->dType!=TRC_TYPE_ROAM_IMSI 
					&& pstTraceFile->dType!=TRC_TYPE_ROAM_MDN )
				continue;

        	if ( pstTraceInfo->dType==pstTraceFile->dType )
			{
				if ( pstTraceFile->dType==TRC_TYPE_IP )
				{
					if( pstTraceInfo->stTraceID.uIP==pstTraceFile->stTraceID.uIP )
					{
						new = 0;
						break;
					}
				}
				else
				{
					if( strncmp(pstTraceInfo->stTraceID.szMIN, pstTraceFile->stTraceID.szMIN, MIN_SIZE )==0 )
					{
						new = 0;
						break;
					}
				}
			}
		}
		
		if ( new==1 )
			StartTrace( pstTraceInfo );
	}

	return 1;
}

int Check_ServTraceInfo()
{
    short i, j;
    char stop, new;
    st_TraceInfo* pstTraceInfo;
    st_TraceInfo* pstTraceFile;

    /* Check Stopping Trace */
    for( i=0; i<MAX_TRACE_CNT; i++ )
    {
        pstTraceFile = &g_pstServerTraceFileList->stTraceFile[i].stTraceInfo;

        if ( pstTraceFile->dType!=TRC_TYPE_SERV )
            continue;

        stop=1;

        /* dType == TRC_TYPE_SERV */
        for (j=0; j<ServerTrace_tbl->count; j++ )
        {
            pstTraceInfo = &(ServerTrace_tbl->stTraceInfo[j]);

            if ( pstTraceInfo->dType==pstTraceFile->dType )
            {
                if ( pstTraceInfo->dType==TRC_TYPE_SERV )
                {
                    if( pstTraceInfo->stTraceID.uIP==pstTraceFile->stTraceID.uIP )
                    {
                        stop = 0;
                        break;
                    }
                }
            }
        }
        if ( stop==1 )
            StopTrace( &g_pstServerTraceFileList->stTraceFile[i] );
    }

    /* Check Startting Trace */
    for( j=0; j<ServerTrace_tbl->count; j++ )
    {
        pstTraceInfo = &(ServerTrace_tbl->stTraceInfo[j]);

        new = 1;

        /* dType == TRC_TYPE_SERV */
        for (i=0; i<MAX_TRACE_CNT; i++ )
        {
            pstTraceFile = &g_pstServerTraceFileList->stTraceFile[i].stTraceInfo;

            if ( pstTraceFile->dType!=TRC_TYPE_SERV )
                continue;

            if ( pstTraceInfo->dType==pstTraceFile->dType )
            {
                if ( pstTraceFile->dType==TRC_TYPE_SERV )
                {
                    if( pstTraceInfo->stTraceID.uIP==pstTraceFile->stTraceID.uIP )
                    {
                        new = 0;
                        break;
                    }
                }
            }
        }

        if ( new==1 )
            StartServerTrace( pstTraceInfo );
    }

    return 1;
}


void StopTrace(pst_TraceFile pstTraceFile)
{
	if ( pstTraceFile->stTraceInfo.dType==TRC_TYPE_IMSI )
		log_print(LOGN_DEBUG, "StopTrace: IMSI [%s]", pstTraceFile->stTraceInfo.stTraceID.szMIN );
	else if ( pstTraceFile->stTraceInfo.dType==TRC_TYPE_MDN )
		log_print(LOGN_DEBUG, "StopTrace: MDN [%s]", pstTraceFile->stTraceInfo.stTraceID.szMIN );
	else if ( pstTraceFile->stTraceInfo.dType==TRC_TYPE_ROAM_IMSI )
		log_print(LOGN_DEBUG, "StopTrace: ROAM IMSI [%s]", pstTraceFile->stTraceInfo.stTraceID.szMIN );
	else if ( pstTraceFile->stTraceInfo.dType==TRC_TYPE_ROAM_MDN )
		log_print(LOGN_DEBUG, "StopTrace: ROAM MDN [%s]", pstTraceFile->stTraceInfo.stTraceID.szMIN );
	else
		log_print(LOGN_DEBUG, "StopTrace: IP [%u]", pstTraceFile->stTraceInfo.stTraceID.uIP );

	if ( pstTraceFile->fp!=NULL )
	{
		log_print(LOGN_CRI, "FileCheck: File Closed [%s]", pstTraceFile->szFilename);
		fclose( pstTraceFile->fp );
		pstTraceFile->fp=NULL;
	}

	memset( pstTraceFile, 0, sizeof( st_TraceFile ) );

/*
	DisTrace( LOG_INFO );
*/
}

void StartTrace(st_TraceInfo *sourceInfo)
{
	static char busyflag=0;		/* Protect Log Overflow */

	int i;
	st_TraceInfo* targetInfo;
	
	for ( i=0; i<MAX_TRACE_CNT; i++ )
	{
		targetInfo = &g_pstTraceFileList->stTraceFile[i].stTraceInfo;

		if ( targetInfo->dType!=TRC_TYPE_IMSI && targetInfo->dType!=TRC_TYPE_IP 
			&& targetInfo->dType!=TRC_TYPE_MDN && targetInfo->dType!=TRC_TYPE_ROAM_IMSI && targetInfo->dType!=TRC_TYPE_ROAM_MDN )
		{
			memcpy( targetInfo, sourceInfo, sizeof(st_TraceInfo) );
		
			if ( sourceInfo->dType==TRC_TYPE_IMSI )
			{
				targetInfo->stTraceID.llIMSI = atoll(targetInfo->stTraceID.szMIN);
				log_print(LOGN_DEBUG, "StartTrace: IMSI [%s]", targetInfo->stTraceID.szMIN );
			}
			else if ( sourceInfo->dType==TRC_TYPE_MDN )
			{
				log_print(LOGN_DEBUG, "StartTrace: MDN [%s]", targetInfo->stTraceID.szMIN );
			}
			else if ( sourceInfo->dType==TRC_TYPE_ROAM_IMSI )
			{
				log_print(LOGN_DEBUG, "StartTrace: ROAM IMSI [%s]", targetInfo->stTraceID.szMIN );
			}
			else if ( sourceInfo->dType==TRC_TYPE_ROAM_MDN )
			{
				log_print(LOGN_DEBUG, "StartTrace: ROAM MDN [%s]", targetInfo->stTraceID.szMIN );
			}
			else
				log_print(LOGN_DEBUG, "StartTrace: IP [%u]", targetInfo->stTraceID.uIP );

			if ( busyflag==1 )
				log_print(LOGN_DEBUG, "StartTrace: TRACE IS AVAILABLE" );
			busyflag=0;

			Init_Trace_Array( &g_pstTraceFileList->stTraceFile[i] );

			return;
		}
	}
	
	/* Trace is Not Available now */
	if (busyflag==0 )
	{
		log_print(LOGN_DEBUG, "StartTrace: TRACE IS BUSY" );
		busyflag=1;
	}
}

void StartServerTrace(st_TraceInfo *sourceInfo)
{
    static char busyflag=0;     /* Protect Log Overflow */

    int i;
    st_TraceInfo* targetInfo;
   
    for ( i=0; i<MAX_TRACE_CNT; i++ )
    {
        targetInfo = &g_pstServerTraceFileList->stTraceFile[i].stTraceInfo;

        if ( targetInfo->dType!=TRC_TYPE_SERV )
        {
            memcpy( targetInfo, sourceInfo, sizeof(st_TraceInfo) );
   
            if ( sourceInfo->dType==TRC_TYPE_SERV )
                log_print(LOGN_DEBUG, "StartServerTrace: IP [%u]", targetInfo->stTraceID.uIP );

            if ( busyflag==1 )
                log_print(LOGN_DEBUG, "StartServerTrace: TRACE IS AVAILABLE" );
            busyflag=0;

			Init_Trace_Array( &g_pstServerTraceFileList->stTraceFile[i] );
/*
            DisServerTrace( LOG_INFO );
*/

            return;
        }
    }

    /* Trace is Not Available now */
    if (busyflag==0 )
    {
        log_print(LOGN_DEBUG, "StartServerTrace: TRACE IS BUSY" );
        busyflag=1;
    }
}

void DisTrace(int level)
{
	pst_TraceFile	pstTraceFile;
	st_TraceInfo* printInfo;

	int i;

	log_print(level, " ");
	log_print(level, "---------------------------------------- MIN TRACE ------------------------------------------");
	for ( i=0; i<MAX_TRACE_CNT; i++ )
	{
		printInfo = &g_pstTraceFileList->stTraceFile[i].stTraceInfo;
		pstTraceFile = &g_pstTraceFileList->stTraceFile[i];

		if ( printInfo->dType==TRC_TYPE_IMSI )
			log_print( level, "DisTrace: IMSI[%s]  W_PKT[%d] BUF[%d]  FILE [%s]", 
				printInfo->stTraceID.szMIN, pstTraceFile->pktcnt, pstTraceFile->dUsedCnt, pstTraceFile->szFilename );
		else if ( printInfo->dType==TRC_TYPE_MDN )
			log_print( level, "DisTrace: MDN[%s]  W_PKT[%d] BUF[%d]  FILE [%s]", 
				printInfo->stTraceID.szMIN, pstTraceFile->pktcnt, pstTraceFile->dUsedCnt, pstTraceFile->szFilename );
		else if ( printInfo->dType==TRC_TYPE_ROAM_IMSI )
			log_print( level, "DisTrace: ROAM IMSI[%s]  W_PKT[%d] BUF[%d]  FILE [%s]", 
				printInfo->stTraceID.szMIN, pstTraceFile->pktcnt, pstTraceFile->dUsedCnt, pstTraceFile->szFilename );
		else if ( printInfo->dType==TRC_TYPE_ROAM_MDN )
			log_print( level, "DisTrace: ROAM MDN[%s]  W_PKT[%d] BUF[%d]  FILE [%s]", 
				printInfo->stTraceID.szMIN, pstTraceFile->pktcnt, pstTraceFile->dUsedCnt, pstTraceFile->szFilename );
		else if ( printInfo->dType==TRC_TYPE_IP )
		{
			log_print( level, "DisTrace: IP[%s]  W_PKT[%d] BUF[%d] FILE [%s]", 
				util_cvtipaddr(NULL,  printInfo->stTraceID.uIP), pstTraceFile->pktcnt, pstTraceFile->dUsedCnt, pstTraceFile->szFilename );
		}
	}
	log_print(level, "--------------------------------------------------------------------------------------------");
}

void DisServerTrace(int level)
{
    pst_TraceFile   pstTraceFile;
    st_TraceInfo* printInfo;

    int i;

	log_print(level, " ");
	log_print(level, "-------------------------------------- SERVER TRACE ----------------------------------------");
    for ( i=0; i<MAX_TRACE_CNT; i++ )
    {
        printInfo = &g_pstServerTraceFileList->stTraceFile[i].stTraceInfo;
        pstTraceFile = &g_pstServerTraceFileList->stTraceFile[i];

		if( printInfo->dType==TRC_TYPE_SERV )
        {
            log_print( level, "DisServerTrace: IP [%s]  W_PKT [%d] BUF[%d] FILE [%s]",
                util_cvtipaddr(NULL,  printInfo->stTraceID.uIP ), pstTraceFile->pktcnt, pstTraceFile->dUsedCnt, pstTraceFile->szFilename );
        }
    }
	log_print(level, "--------------------------------------------------------------------------------------------");
}

S32 dMakeIRMHash()
{
	int		dLine;
	FILE	*fa;
	char	szBuf[1024];
	char	szInfo_1[64], szInfo_2[64], szInfo_3[64];

	st_IRMHash_Key		stIRMHashKey;
	st_IRMHash_Key		*pKey = &stIRMHashKey;
	st_IRMHash_Data		stIRMHashData;
	st_IRMHash_Data		*pData = &stIRMHashData;
	stHASHONODE			*pHASHNODE;

	/* reset hash */
	hasho_reset(pIRMINFO);

	fa = fopen(FILE_IRM, "r");
	if(fa == NULL) {
		log_print(LOGN_CRI,"dMakeIRMHash : FILE=%s OPEN FAIL (%s)", FILE_IRM, strerror(errno));
		return -1;
	}

	dLine = 0;
	while(fgets(szBuf,1024,fa) != NULL)
	{
		if(szBuf[0] != '#') {
			log_print(LOGN_CRI,"dMakeIRMHash : File=%s:%d ROW FORMAT ERR", FILE_IRM, dLine+1);
			fclose(fa);
			return -2;
		}

		if(szBuf[1] == '#')
			continue;
		else if(szBuf[1] == 'E')
			break;
		else if(szBuf[1] == '@') {
			if(sscanf(&szBuf[2],"%s %s %s", szInfo_1, szInfo_2, szInfo_3) == 3) {
				log_print(LOGN_CRI, "READ IRM DATA i=%d INFO1=%s INFO2=%s INFO3=%s", dLine, szInfo_1, szInfo_2, szInfo_3);

				memset(pKey, 0x00, DEF_IRMHASH_KEY_SIZE);
				memset(pData, 0x00, DEF_IRMHASH_DATA_SIZE);	

				memcpy(pKey->szIRM, szInfo_2, DEF_IRM_PATTERN_LEN);
				pKey->szIRM[DEF_IRM_PATTERN_LEN] = 0x00;

				memcpy(pData->szIMSI, szInfo_1, DEF_IRM_PATTERN_LEN);
				pData->szIMSI[DEF_IRM_PATTERN_LEN] = 0x00;

				memcpy(pData->szPrefix, szInfo_3, DEF_IRM_PREFIX_LEN);
				pData->szPrefix[DEF_IRM_PREFIX_LEN] = 0x00;

				if((pHASHNODE = hasho_add(pIRMINFO, (U8 *)pKey, (U8 *)pData)) != NULL) {
					log_print(LOGN_CRI, "HASH ADD IRM i=%d IRM=%s IMSI=%s PREFIX=%s", 
						dLine, pKey->szIRM, pData->szIMSI, pData->szPrefix);
				}
				else {
					log_print(LOGN_CRI, "HASH ADD FAIL IRM i=%d IRM=%s IMSI=%s PREFIX=%s", 
						dLine, pKey->szIRM, pData->szIMSI, pData->szPrefix);
				}
			}
		}
		dLine++;
	}

	fclose(fa);

	return 0;	
}

S32 dCvtfromIRMtoIMSI(U8 *szIMSI)
{
	int					len, offset;
	U8					szRst[MAX_MIN_SIZE];

	st_IRMHash_Key		stIRMHashKey;
	st_IRMHash_Key		*pKey = &stIRMHashKey;
	st_IRMHash_Data		*pData;
	stHASHONODE			*pHASHNODE;

	len = strlen(szIMSI);
	len = (len > MAX_MIN_LEN) ? MAX_MIN_LEN : len;	

	if(len >= DEF_IRM_MIN_LEN)
	{
		memset(pKey, 0x00, DEF_IRMHASH_KEY_SIZE);
		offset = len - DEF_IRM_MIN_LEN;
		memcpy(pKey->szIRM, &szIMSI[offset], DEF_IRM_PATTERN_LEN);
		pKey->szIRM[DEF_IRM_PATTERN_LEN] = 0x00;
		offset = offset + DEF_IRM_PATTERN_LEN;

		if((pHASHNODE = hasho_find(pIRMINFO, (U8 *)pKey)) != NULL)
		{
			pData = (st_IRMHash_Data *)nifo_ptr(pIRMINFO, pHASHNODE->offset_Data);
			sprintf(szRst, "%s%s%s", pData->szPrefix, pData->szIMSI, &szIMSI[offset]);
			log_print(LOGN_INFO, "CVTIRM IMSI CVT IRM=%s IMSI=%s", szIMSI, szRst);
			memcpy(szIMSI, szRst, MAX_MIN_LEN);
			szIMSI[MAX_MIN_LEN] = 0x00;
		}
		else 
		{
			log_print(LOGN_CRI, "CVTIRM IMSI hasho_find NULL IMSI=%s IRM=%s", szIMSI, pKey->szIRM);
		}
	}
	else
	{
		log_print(LOGN_INFO, "CVTIRM IMSI SHORT LENGTH=%d IMSI=%s", len, szIMSI);
	}

	return 0;
}

S32 dCvtfromIRMtoMDN(U8 *szIMSI)
{
	int					len, offset;
	U8					szRst[MAX_MIN_SIZE];

	st_IRMHash_Key		stIRMHashKey;
	st_IRMHash_Key		*pKey = &stIRMHashKey;
	st_IRMHash_Data		*pData;
	stHASHONODE			*pHASHNODE;

	len = strlen(szIMSI);
	len = (len > MAX_MIN_LEN) ? MAX_MIN_LEN : len;	

	if(len >= DEF_IRM_MIN_LEN)
	{
		memset(pKey, 0x00, DEF_IRMHASH_KEY_SIZE);
		offset = len - DEF_IRM_MIN_LEN;
		memcpy(pKey->szIRM, &szIMSI[offset], DEF_IRM_PATTERN_LEN);
		pKey->szIRM[DEF_IRM_PATTERN_LEN] = 0x00;
		offset = offset + DEF_IRM_PATTERN_LEN;

		if((pHASHNODE = hasho_find(pIRMINFO, (U8 *)pKey)) != NULL)
		{
			pData = (st_IRMHash_Data *)nifo_ptr(pIRMINFO, pHASHNODE->offset_Data);
			if(pData->szIMSI[0] == '1') {
				sprintf(szRst, "0%s%s", pData->szIMSI, &szIMSI[offset]);
			}
			else {
				sprintf(szRst, "%s%s", pData->szIMSI, &szIMSI[offset]);
			}
			log_print(LOGN_INFO, "CVTIRM MDN CVT IRM=%s IMSI=%s", szIMSI, szRst);
			memcpy(szIMSI, szRst, MAX_MIN_LEN);
			szIMSI[MAX_MIN_LEN] = 0x00;
		}
		else 
		{
			log_print(LOGN_CRI, "CVTIRM MDN hasho_find NULL IMSI=%s IRM=%s", szIMSI, pKey->szIRM);
		}
	}
	else
	{
		log_print(LOGN_INFO, "CVTIRM MDN SHORT LENGTH=%d IMSI=%s", len, szIMSI);
	}

	return 0;
}

