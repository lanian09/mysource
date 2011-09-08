#ifndef _SRCHMSG_H
#define _SRCHMSG_H

#include <ipaf_svc.h>
#include <udrgen_define.h>

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

#define MSGTYPE_NAS             1
#define MSGTYPE_QUD             2
#define MSGTYPE_AAA             3
#define MSGTYPE_DSCP            4
#define MSGTYPE_IDR             5


#define UDRLOG_PATH             "/BSDM/UDR"
#define MAX_FILE_COUNT          2048


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
} st_SrchInfo_t, *pst_SrchInfo_t;



#define RESULT_PATH             "/BSDM/RESULT"

#endif  /* _SRCHMSG_H */
