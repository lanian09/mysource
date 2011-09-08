#ifndef __LOGM_H_
#define __LOGM_H_

#include <stdio.h>
#include <time.h>
#include <dirent.h>
#include <sys/types.h>


/** SDMD LOG Information  **/
//#define         DUMP_FILE_PATH     	"/BSDM/CDR"
#define         DUMP_FILE_PATH     	"/DSCM/CDR"

#define			COMPRESS_FILE		1
#define			UNCOMPRESS_FILE		2

/**  LOGM Init **/
typedef struct
{
	unsigned int	max_dump_cnt;
} st_LOGMConfig;

 

/**  CDR -> LOGM LOG Structure **/
typedef struct
{
    unsigned char   		szIMSI[16];
    unsigned long long   	llAcctSessID;
    unsigned int   			uiSrcIP;
    int   					dSvcOpt;
    time_t   				tCreateTime;
    time_t   				tLastTime;
	unsigned int 			uiDestIP;
	unsigned short 			usDestPort;
	unsigned short 			usSecPort;
	int						dSvcType;
	int						dReserved;
	unsigned int			uiUPIPFrames;
	unsigned int			uiDownIPFrames;
	unsigned int			uiUPIPBytes;
	unsigned int			uiDownIPBytes;
	unsigned int			uiUPTCPREFrames;
	unsigned int			uiDownTCPREFrames;
	unsigned int			uiUPTCPREBytes;
	unsigned int			uiDownTCPREBytes;
} st_CDRSessLog, *pst_CDRSessLog;


/** CDR -> LOGM LOG List Structure **/
typedef struct
{
    int	    			dCount;
    int    				dReserved;
	st_CDRSessLog		cdr_sess_log[40];
} st_CDRSessLogList, *pst_CDRSessLogList;


typedef struct
{
	time_t				mkcrt_time;
	unsigned	int		log_cnt;
	unsigned	int		fd;
	unsigned	char	fileName[32];
} st_CDRDumpInfo, *pst_CDRDumpInfo;


/** error reason **/
typedef enum
{
	ERR_LOGM_SUCCESS    =   0,
    ERR_LOGM_RCV_UNKNOWN_MSG,
    ERR_LOGM_RCV_WRONG_LENGTH,
} e_err_logm;


/** Service-Type registered IP & Port List **/
typedef struct 
{
    char    dest_ip_addr[16];
    int     dest_port;
} st_svc_register;

#endif
