#ifndef _SRCHMSG_H
#define _SRCHMSG_H

/* for HeadLog */
#include <ipam_headlog.h>
#include <ipam_gen.h>
#include <wap1ana.h>

#define PW_ACCOUNTING_REQUEST   4
#define PW_ACCOUNTING_RESPONSE  5
#define PW_QUALIFICATION_DIR    125
#define PW_QUALIFICATION_RET    124
#define PW_DISCONNECT_REQ       40
#define PW_ACK                  41
#define PW_NACK                 42

#define MAX_FILENAME_LEN        128
#define MAX_IMSI_LEN            15
#define MAX_MSISDN_LEN          12

#define VIEWTYPE_NORM           1
#define VIEWTYPE_LINE           2
#define FILE_EXIT       "quit"

#define SRCHTYPE_FILE   1
#define SRCHTYPE_TIME   2

#define SRCHKEY_ALL     1
#define SRCHKEY_IMSI    2
#define SRCHKEY_SEQ     3
#define SRCHKEY_LOGID   4

#define MSGTYPE_NAS             1
#define MSGTYPE_QUD             2
#define MSGTYPE_AAA             3
#define MSGTYPE_DSCP            4
#define MSGTYPE_IDR             5


#define UDRLOG_PATH             "/BSD/UDR"
#define MAX_FILE_COUNT          1024


#ifndef PASS_EXCEPTION
    #define PASS_EXCEPTION(condition, label) if ( (condition) ) \
							{ goto label; }
#endif

#ifndef PASS_CATCH
	#define PASS_CATCH(label)	goto PASS_CATCH_END_LABEL; label:
#endif

#ifndef PASS_CATCH_END
	#define PASS_CATCH_END 	PASS_CATCH_END_LABEL:
#endif


typedef struct _st_SrchInfo_t
{
    int  iViewType;
    int  iSrchType;
    int  iSrchKeyType;
    int  iFileCnt;
    char szFileInfo[MAX_FILE_COUNT][MAX_FILENAME_LEN];
    char szOutFile[MAX_FILENAME_LEN];
    char szImsiNum[MAX_IMSI_LEN+1];
    char szDirectory[MAX_FILENAME_LEN];
    time_t   tStartTime;
    time_t   tEndTime;
    unsigned int uiUdrSeq;
    unsigned int uiLogId;
} st_SrchInfo_t, *pst_SrchInfo_t;

#define MAX_LOG_MIN_LEN             16
#define MAX_LOG_IP_LEN              16
#define MAX_LOG_SUBNO_LEN           64
#define MAX_LOG_REQTIME_LEN         16
#define MAX_LOG_RESTIME_LEN         16
#define MAX_LOG_URL_LEN             601
#define MAX_LOG_METHOD_LEN          16
#define MAX_LOG_USERAGENT_LEN       64
#define MAX_LOG_STATUS_LEN          16

typedef struct _st_LogParse_t{
    unsigned char   szMin[MAX_LOG_MIN_LEN];
    unsigned int    uiSrcIp;
    unsigned int    uiSrcPort;
    unsigned char   szSubNo[MAX_LOG_SUBNO_LEN];
    time_t          tReqTime;
    time_t          tResTime;
    int             iWapReqSize;
    int             iWapResSize;
    int             iContentsLen;
    unsigned int    uiResultCode;
    unsigned char   szStatus[MAX_LOG_STATUS_LEN];
    unsigned char   szURL[MAX_LOG_URL_LEN];
    unsigned char   szMethodType[MAX_LOG_METHOD_LEN];
    unsigned char   szUserAgent[MAX_LOG_USERAGENT_LEN];
} st_LogParse_t, *pst_LogParse_t;


#define RESULT_PATH             START_PATH"/RESULT"

#endif  /* _SRCHMSG_H */
