#ifndef __SMPP_H__
#define __SMPP_H__

#include <stdio.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/msg.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <sysconf.h>
#include <errno.h>
#include <comm_msgtypes.h>
#include <mysql.h>
#include <define.h>
#include <ipaf_names.h>
#include <comm_smpp_if.h>
#include <utillib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "ipaf_define.h"
#include "comm_timer.h"
#include "comm_trace.h"
#include "ipaf_stat.h"
#include "shm.h"
#include "commlib.h"

#define SMPP_EMAIL_LOG_FILE		"APPLOG/SMPP/"
#define SMPP_INTERFACE_LOG		"APPLOG/SMPP/smpp_his"
#define SMPP_LOG_FILE       	"APPLOG/SMPP/smpp_log"
#define SMPP_ERRLOG_FILE    	"APPLOG/SMPP/smpp_err"
#define SMPP_CONF_FILE 			"NEW/DATA/smpp.conf"
#define SMPP_OP_FILE			"NEW/DATA/smpp_operator"
#define TIMEOUT_FILE            "NEW/DATA/TIMEOUT.conf"

#define SHM_CREATE  					1
#define SHM_EXIST   					2

#define PREFIX_MAX						100000
#define MAX_SMPP_CID					5000
#define MAX_SMPP_DATA_LEN   			2048

#define MAX_SMC_INFO					1		/* SMSC SMS Server number,
								 		 * server connection 을 늘리려면 define을 증가 시킨다. */
#define ONE_SMC_INFO					0		/* SMSC SMS Server number */
#define MAX_SMPPQ_TUPLE 				10

#define SMC_SIZE (sizeof(T_SMC_QUEUE) - 4 - sizeof(T_SMC_TUPLE) * 10)
#define SMC_TUPLE_SIZE  sizeof(T_SMC_TUPLE)
#define SMC_QUEUE_SIZE  sizeof(T_SMC_QUEUE)

#define COM_ID_BIND_TM					0x2
#define COM_ID_BIND_TM_RESP				0x80000002
#define COM_ID_SUBMIT_SM				0x4
#define COM_ID_SUBMIT_SM_RESP			0x80000004

#define SMPP_NO_EVENT       			1
#define	SEND_TO_SMSC					1 
#define RCV_FROM_SMSC					2

/** Trace **/
#define     TRACE_METHOD_IMSI           1
#define     TRACE_METHOD_IP             2

#define     TRACE_TYPE_BLK_INFO			1
#define     TRACE_TYPE_DELIVER			2
#define     TRACE_TYPE_DELIVER_ACK		3
#define     TRACE_TYPE_REPORT			4
#define     TRACE_TYPE_REPORT_ACK		5
////////////////////////////////////////////////////////////
/** MMC 명령어 **/
typedef struct {
	char    cmdName[24];
	int     (*func)(IxpcQMsgType*);
} MmcHdlrVector;

#define MML_MAX_MMC_HANDLER			30

/*** set-smc-entry dst, pfx, smc_id, [last_pfx] ***/
#define a_Update_SMC_ENTRY 	100

/*** set-smc-info dst, smc_id, ip_addr, system_id, passwd, system_type, port_no ***/
#define a_Update_SMC_INFO	101

/*** dis-smc-entry dst , [pfx], [smc_id] ***/
#define a_Select_SMC_ENTRY	102

/*** dis-smc-info dst, smc_id ***/
#define a_Select_SMC_INFO	103

/*** del-smc-entry dst, pfx, smc_id, [last_pfx] ***/
#define a_Delete_SMC_ENTRY	104

/*** del-smc-info dst, smc_id ***/
#define a_Delete_SMC_INFO   105

/*** 추가해야할 명령어 ***/
#define a_Add_Operator_INFO	106
#define a_Del_Operator_INFO	107
#define a_Chg_Operator_INFO	108

#define HEAD_READ_STS   1
#define BODY_READ_STS   2

/** ERROR CODE **/
#define eP_ConTimeover      			-150
#define eP_ConError         			-151
#define eP_SockFdErr        			-152
#define eP_SockConn        				-153
#define eP_NoConn           			-154
#define eP_MsgLenOver       			-155
#define eP_Sock_Disconnect  			-156
#define eP_datalen          			-157
#define eP_myProc           			-158
#define eP_pairProc        			 	-159
#define eP_DataErr          			-160
#define eP_SelctDB          			-161
#define eP_WriteSockErr     			-162
#define eP_StsBuffer        			-163
#define eP_FDSts            			-164
#define eP_WriteSockErr2    			-165
#define eP_Protocol         			-166
#define eP_CID                          -167
#define eP_mtype                        -168
#define eP_smcid                        -169
#define eP_CidAlloc                     -170
#define eP_FILE_NOT                     -171
#define eP_Data_Diff            		-172
#define eP_Data_Err                     -173
#define eP_idx                          -174
#define eP_Chg_IP                       -175
#define eP_Sock_Disconnect2     		-176
#define eP_Timeout                      -177
#define eP_CallbackError        		-178

//////////////////////////////////////////////////////////	
// SMSC INTERFACE
//////////////////////////////////////////////////////////	
/* SMSC와 주고받는 Packet Format */
#define SMPP_HEAD_LEN   				8
#define SMPP_MSG_ID_LEN					16
#define SMPP_MSG_PWD_LEN				16
#define SMPP_MSG_PRIFIX_LEN				16
#define SMPP_MSG_ADDR_LEN				32
#define SMPP_MSG_TEXT_LEN				160
#define SMPP_MSG_DELIVER_TIME_LEN		20
#define SMPP_MSG_DEST_CODE_LEN			12

//////////////////////////////////////////////////////////	
enum { SMPP_ERR_NORMAL = 100
	 , SMPP_ERR_WRITE  = 200
	 , SMPP_ERR_ETC    = 300};

typedef struct {
    char    msg_flag;	/* HEAD_READ_STS/BODY_READ_STS */
    int     rcv_len;	/* Buffer에 들어있는 data Length */
    int     body_len;	/* Header에서 읽어온 bodyLen 값 */
#define MAX_SMPP_DATA_LEN       		2048
    char    buf[MAX_SMPP_DATA_LEN];
} SMPP_RBUF;

#define SOCK_BIND_TRY_STS   			1
typedef struct {
    long    type;		/* IP_Address를 long형으로 변환한 값 */
    int     fd;
    int     bind;		/* SMSC와 Bind 되었는 지 여부 */
    int     bindtry;	/* bind retry한 상태인지 아닌지 */
    int     prototype;	/* not used */   
	int		port_no;
	int     writeFailCnt;		/* 2005-12-03 EKYANG */
    SMPP_RBUF   rbuf;
} SMPP_CON_DATA;

typedef struct {
    int         cur_con;
    int         max_con;
    int         maxfd;
    SMPP_CON_DATA   condata[MAX_SMC_INFO];
} SMPP_FD_TBL;

#define MAX_SMPP_CID 5000
typedef struct {
    int     	cid;
    int     	conidx;
    int     	prototype;
    time_t  	starttime;
} SMPP_CID;

typedef struct {
    int        	port_no;
    char       	ip_addr[32];
    char       	user_id[16];
    char       	passwd[16];
    char       	smsc_id[16];
    char       	use_scm[16];
} SMC_INF;

typedef struct {
    SMC_INF    smc_info[MAX_SMC_INFO];
    char       mc_entry[PREFIX_MAX];
} SMC_TABLE;

typedef struct {
    int         pfx;
    int         port_no;
    int         reserved;
    char        smc_id;
    char        ipaddr[32];
    char        systemid[16];
    char        password[16];
    char        sysmtype[16];
} T_SMC_TUPLE;

typedef struct {
    long        mtype;
    int         client_id;
    int         acc_type;
    int         cid;        
    int         acc_result;
    int         tuple_cnt; 
    T_SMC_TUPLE smc_tuple[MAX_SMPPQ_TUPLE]; 
} T_SMC_QUEUE;


#define SMS_TBL						"sms_history"		/* sms history table name */ 
typedef struct {
#define	SMPP_DB_IP_MAX_SIZE			16
#define	SMPP_DB_NAME_MAX_SIZE		8
#define	SMPP_DB_USER_MAX_SIZE		16
#define	SMPP_DB_PASSWD_MAX_SIZE		16
#define	SMPP_DB_TABLE_MAX_SIZE		16
	char		ipaddr[SMPP_DB_IP_MAX_SIZE];
	char		name[SMPP_DB_NAME_MAX_SIZE];
	char		user[SMPP_DB_USER_MAX_SIZE];
	char		passwd[SMPP_DB_PASSWD_MAX_SIZE];
	char		table[SMPP_DB_TABLE_MAX_SIZE];
} SMS_DB_INFO;

////////////////////////////////////////////////////////////////////

extern char    sysLabel[COMM_MAX_NAME_LEN], mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN];
extern SMC_TABLE	*smc_tbl;
extern SMPP_FD_TBL 	client;
extern int smppQid, ixpcQid;

extern int smpp_mmc_set_smc_info (IxpcQMsgType *rxIxpcMsg);
extern int smpp_mmc_dis_smc_info (IxpcQMsgType *rxIxpcMsg);
extern int smpp_mmc_del_smc_info (IxpcQMsgType *rxIxpcMsg);

extern int smpp_mmcHdlrVector_bsrchCmp (const void *a, const void *b);
extern int smpp_mmcHdlrVector_qsortCmp (const void *a, const void *b);

extern int smpp_initLog (void);
extern int smpp_txMMLResult (IxpcQMsgType *rxIxpcMsg, char *buff, char resCode,
			char contFlag, unsigned short extendTime, char segFlag, char seqNo);

extern int smpp_mkDir(char *dirName);
extern void smpp_makePFX(int index, char * spfx );

/* smpp_init.c */
extern int smpp_init(int pid);
extern int smpp_get_smpp_conf (void);
extern int smpp_get_smc_info (void);
extern int init_smpp_shm(void);
extern void smpp_finProc(void);
extern int smpp_dLoadTimeOut(void);
extern void smpp_dLogTimeOut(int level);
extern void SetUpSignal(void);
extern void UserControlledSignal (int sign);
extern void IgnoreSignal (int sign);

extern int dGetTraceData(pst_SESSInfo pstInfo);
extern int GetConfData(char *sParseStr, char **ppstData, int *iCount);
extern char * TreamNullData(char *sParseStr);
extern void Get1Data(char *sParseStr, char *getData);

/* smpp_log.c */
extern int smpp_log (int type, char *cmd_type, int smcid, int cid, SMS_HIS *sms_hist, SMPP_MSG_INFO *msg_info );
/* smpp_msgQ.c */
extern int send_sms_toSmc(int smcid, SMS_HIS *sms_hist);
extern void smpp_dumpSmppMsg(SMS_INFO *rcv_msg);
//extern void recv_smpp_cmd(SMS_INFO *rcv_msg);
extern void recv_smpp_cmd(IxpcQMsgType *rxIxpcMsg);
extern void proc_msgQ_data(void);
/* smpp_proto.c */
//extern int  make_smpp_DELIVER(SMS_HIS *sms_hist, SMPP_MSG_INFO *msg_info,  SMPP_DELIVER *smpp);
//extern void make_smpp_BIND(int idx, SMPP_BIND *smpp);
//extern void make_smpp_REPORT_ACK(int code, SMPP_REPORT_ACK *smpp);

extern int protocol_bind_try(int idx);
extern int read_sock_data_to_buffer(int idx);
extern int check_smpp_head_msg(int idx);
extern int read_complete_msg(int idx);
extern int check_smpp_ack(int idx);
/* smpp_socket.c */
extern void cut_socket (int idx) ;
extern int read_socket(int idx, int msg_len, char *msg);
extern void init_client_data (void);
extern int add_fd_tbl(int idx, int sockfd);
extern int check_socket(int *active_fd);
extern int write_sock_data(int idx, char *data, int len);
extern int read_proto_data(int idx);
extern void proc_socket_data (void);
extern int tcp_connect(int idx);
extern void manage_con_sts(void);
/* smpp_sql.c */
extern int keepaliveDB(void);
extern int smpp_connectDB(void );
extern int smpp_selectDB(char *subs_id);
extern int smpp_insertDB(SMS_HIS *his);
extern int smpp_updateDB(SMS_HIS *his, int update_type);
extern void sample_data(SMS_HIS *his);
/* smpp_util.c */
extern void ClearCid(int cid);
extern void audit_call(time_t  now);
extern int getAllocCidCnt(void);
extern void smpp_makePFX(int index, char * spfx);
/* smpp_trace */
extern void Trace_BLKMSG (SMS_INFO *rcvBlkMsg);
extern void Trace_DELIVER_MSG (SMS_HIS *sms_his, SMPP_DELIVER *deliver);
extern int Send_TrcMsg_BlkMsg_SMPP2COND (SMS_INFO *rcvBlkMsg, int trace_type);
extern int Send_TrcMsg_DelivMsg_SMPP2COND (SMS_HIS *sms_his, SMPP_DELIVER *deliver, int trace_type);


extern int recv_mmc (IxpcQMsgType *rxIxpcMsg);
extern void dSetCurTrace(NOTIFY_SIG *pNOTISIG);
extern void dSetCurTIMEOUT(NOTIFY_SIG *pNOTISIG);
extern int NewCid();
extern void FreeCid(int cid);
extern int make_smpp_DELIVER(SMS_HIS*, SMPP_MSG_INFO*,  SMPP_DELIVER*);
extern int  set_version(int prc_idx, char *ver);
extern int init_smpp_shm (void);
extern void InitCid(int start, int end);

#endif /* __SFMCONF_H__ */
