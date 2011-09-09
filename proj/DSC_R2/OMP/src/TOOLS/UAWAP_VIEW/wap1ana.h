#ifndef _WAP1ANA_DATABASE_HEADER_
#define _WAP1ANA_DATABASE_HEADER_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/mman.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>


#define SVC_TYPE_WAP1       32859
#define REDIRECT_RESULT     302

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



/* Option */
#define WAP_APP_LOG_DIR           "/BSD/APPLOG"
#define WAP_APP_LOG_FILE          "UAWAPANA"


#define TRANS_LOG_DIR           "/BSD/LOG/HEADLOG/UAWAPANA"
#define TRANS_LOG_FILE          "UAWAPANA"


#define FAIL_LOG_DIR            "/BSD/LOG/HEADLOG/UAWAPANA/ERROR"
#define FAIL_LOG_FILE           "UAWAPANA"


#define SYS_CONF_DIR            "/BSD/NEW/DATA"
#define SYS_CONF_FILE           "sysconfig"

#define LOG_INFO_DIR            "/BSD/NEW/DATA/MMAP"
#define LOG_INFO_FILE           "UAWAP_LOGINFO.dat"


#define TXN_CONF_DIR            "/BSD/NEW/DATA"
#define TXN_CONF_FILE           "UAWAP_TXN_EXT.conf"

#define GW_CONF_DIR             "/BSD/NEW/DATA"
#define GW_CONF_FILE            "UAWAP_GW.conf"

#define GLOBAL_CONF_DIR             "/BSD/NEW/DATA"
#define GLOBAL_CONF_FILE            "UAWAP_GLOBAL.conf"

#define DUP_CONF_FILE            "SDMD_DUP.conf"

#define FAILOVER    0
#define NORMAL_PROC 1

#define ERR_DECODE      0
#define ERR_IPPOOL      1
#define ERR_SESSION     2
#define ERR_SEND        3
#define ERR_REDIRECT    4

/* Return Code */
typedef enum _e_ReturnCode_t_{
    WAP1_SUCC = 0,
    WAP1_FAIL = 1
} e_ReturnCode_t, *pe_ReturnCode_t;


/* MAX Length */
#define MAX_SUBNO_SIZE          32
#define MAX_WAPGW_NUM           3
#define MAX_DB_USERNAME_LEN     16
#define MAX_DB_PASSWORD_LEN     16
#define MAX_DB_CONNSTR_LEN      64
#define MAX_CONN_NAME_LEN       64
#define MAX_DB_KEY_LEN          64
#define MAX_TLOG_LEN            1024
#define MAX_ERRMSG_LEN	        128
#define MAX_DB_IP_LEN           16
#define MAX_DB_PORT_LEN         8
#define MAX_DB_ALIAS_LEN        16
#define MAX_DB_CONF_NUM         10
#define MAX_LOGID_NUM           2000000000
#define MAX_SOCKDATA_LEN        4096

/* Database Handler */
typedef struct _st_DbConnInfo_t{
    unsigned char           sSearchKey[MAX_DB_KEY_LEN];
    unsigned char           sConnName[MAX_CONN_NAME_LEN];
} st_DbConnInfo_t,          *pst_DbConnInfo_t;

typedef struct _st_DbHandler_t{
    unsigned int            iReserved;
    st_DbConnInfo_t         stDbConnInfo[MAX_DB_CONF_NUM];
} st_DbHandler_t, *pst_DbHandler_t;



typedef struct{
    unsigned int            iLogId;
    unsigned int            iLogTime;
    unsigned char           sTLOG[MAX_TLOG_LEN+1];
    unsigned int            iFlag;
} st_DbFetchInfo_t, *pst_DbFetchInfo_t;


typedef struct{
    unsigned int            iIsThread;
	unsigned char	        sDbIpAddress[MAX_DB_IP_LEN];
	unsigned char	        sDbPort[MAX_DB_PORT_LEN];
	unsigned char	        sDbUserName[MAX_DB_USERNAME_LEN];
	unsigned char	        sDbPassword[MAX_DB_PASSWORD_LEN];
	unsigned char	        sDbAlias[MAX_DB_ALIAS_LEN];
} st_WapGwInfo_t, *pst_WapGwInfo_t;

typedef struct{
	unsigned int	        iFetchCount;
	unsigned int	        iFetchTerm;
	unsigned int	        iServiceType;
	unsigned char           sServerIp[MAX_DB_IP_LEN];
	unsigned int	        iServerPort;
	unsigned int            iPastCnt;
	unsigned int            iMySysNum;
	unsigned int            iMaxSysNum;
	unsigned int            iUrlChaFlag;
	unsigned char           sHeartbeatIp[MAX_DB_IP_LEN];
	unsigned int            iHeartbeatPort;
} st_Wap1AnaGlobal_t,       *pst_Wap1AnaGlobal_t;

typedef struct{
    unsigned char           sIpAddress[MAX_WAPGW_NUM][MAX_DB_IP_LEN];
    unsigned int            iLogId[MAX_WAPGW_NUM];
} st_LogId_t, *pst_LogId_t;

typedef struct{
    int                     iMsgQId;
    int                     iOmpRcvMsgQId;
    int                     iOmpSndMsgQId;
	st_WapGwInfo_t          wapGwInfo[MAX_WAPGW_NUM];
	st_WapGwInfo_t          wapGwInfoThread[MAX_WAPGW_NUM];
	st_Wap1AnaGlobal_t      wap1AnaGlobal;
} st_Wap1AnaConf_t, *pst_Wap1AnaConf_t;


typedef struct _st_UdpMsg_t{
    unsigned int            iGwNum;
    st_WapGwInfo_t          stWapGwInfo;
    st_DbFetchInfo_t        stDbFetchInfo;
} st_UdpMsg_t, *pst_UdpMsg_t;


unsigned int AltiOpenDb(pst_WapGwInfo_t pDbConn, char *sErrMsg);
unsigned int AltiCloseDb(pst_WapGwInfo_t pDbConn, char *sErrMsg);
unsigned int AltiInitDb();


unsigned int FetchDbData
(pst_WapGwInfo_t pst_DbConn, 
 pst_DbFetchInfo_t pst_FetchInfo,
 unsigned int iCurrentCursor,
 unsigned char *szErrMsg);

typedef struct _st_TimeVal_{
    struct tm   tm_FirstTime;
    int         transTime[4];
} st_TimeVal_t, *pst_TimeVal_t;

/* For Parsing */
#define MAX_ONEDATA_LEN 1024


#define METHOD_NUM      16
#define MAX_METHOD_LEN  16

#define METHOD_CONNECT          0x01
#define METHOD_CONNECT_REPLY    0x02
#define METHOD_REDIRECT         0x03
#define METHOD_REPLY            0x04
#define METHOD_DISCONNECT       0x05
#define METHOD_PUSH             0x06
#define METHOD_CONFIRMED_PUSH   0x07
#define METHOD_SUSPEND          0x08
#define METHOD_RESUME           0x09
#define METHOD_GET              0x40
#define METHOD_OPTIONS          0x41
#define METHOD_HEAD             0x42
#define METHOD_DELETE           0x43
#define METHOD_TRACE            0x44
#define METHOD_POST             0x60
#define METHOD_PUT              0x61

typedef struct _st_MethodType_t{
    unsigned int    iCode;
    unsigned char   sMethodStr[MAX_METHOD_LEN];
} st_MethodType_t, pst_MethodType_t;







#endif
