#ifndef __MMCR_H__
#define __MMCR_H__


#include <stdio.h>
#include <stdlib.h>
//#include <unistd.h>
#include <ctype.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <pthread.h>
#include <sys/statvfs.h>

#include <sys/procfs.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/file.h>
#include <sys/swap.h>
#include <sys/sysinfo.h>

#include "sysconf.h"
#include "comm_msgtypes.h"
#include "commlib.h"
#include "sfm_msgtypes.h"
#include "stm_msgtypes.h"
#include "comm_almsts_msgcode.h"
#include "mmcr_msgtype.h"
#include "ipaf_shm2.h"
#include "mmdb_destip.h"
#include "mmdb_destport.h"
#include "comm_loglevel.h"
#include "comm_timer.h"
#include "pps_conf.h"

#include "ippool_bitarray.h"

#include "mmc_hld.h"

/* for debug */
#include "mmcr_debug.h"
#include "hasho.h"
//#include "init_shm.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define DEF_OFF_FLAG	0
#define DEF_ON_FLAG		1

#define MMCR_LOG_FILE       "APPLOG/OAM/mmcr_log"
#define MMCR_ERRLOG_FILE    "APPLOG/OAM/mmcr_err"

#define MDBMGR_MSG_KEY		8014  /*0x1F4E*/
#define AAAIF_MSG_KEY		8043  /*0x1F6B*/

#define AAA_INFO_FILE        "NEW/DATA/AAAIF.conf"
#define AN_AAA_INFO_FILE     "NEW/DATA/AN_AAAIF.conf" // by helca 2007.01.05
#define SERVICE_TYPE_FILE    "NEW/DATA/SERVICE_TYPE.conf"
#define SERVICE_OPT_FILE    "NEW/DATA/SVC_OPT.conf"
#define IP_POOL_FILE         "NEW/DATA/IP_POOL.conf"
#define TRC_INFO_FILE        "NEW/DATA/TRACE_INFO.conf"
#define UAWAPANA_GW_FILE		 "NEW/DATA/UAWAP_GW.conf"
#define UAWAPANA_TXT_EXT_FILE "NEW/DATA/UAWAP_TXN_EXT.conf"
#define PSDN_FILE		     "NEW/DATA/PDSN.conf"
#define SDMD_INFO_FILE		 "NEW/DATA/SDMD_DUP.conf"
#define SDMD_STS_FILE        "NEW/DATA/SDMD_STS.conf"
#define SDMD_SWITCH_FILE	 "NEW/DATA/SDMD_SWITCH_OVER.conf"
#define UDR_CATEGORY_FILE	 "NEW/DATA/UDR_CATEGORY.conf"
#define UDR_DUMP_FILE		 "NEW/DATA/UDR_DUMP.conf"
#define UDR_DUMP_CONF_FILE	 "NEW/DATA/UDR_DUMP_CONF.conf"
#define UDR_FILE			 "NEW/DATA/AAAPARA.conf"
#define UDR_TXC_FILE         "NEW/DATA/UDR_TXC.conf"
#define TIMER_FILE			 "NEW/DATA/TIMER.conf"
#define PPS_FILE		 "NEW/DATA/PPS.conf"
#define CALL_LOAD_FILE		 "NEW/DATA/CALL_LOAD.conf"
#define SESS_INFO_FILE		 "NEW/DATA/SESS_INFO.conf"
#define LOG_LEVEL_FILE		 "NEW/DATA/LOG_LEVEL.conf"
#define TIMER_FILE		     "NEW/DATA/TIMER.conf"
#define UAWAP_GLOBAL_FILE    "NEW/DATA/UAWAP_GLOBAL.conf"
#define UDR_CDR_INFO_FILE    "NEW/DATA/UDR_CDRINFO.conf"
#define ICMP_FILE		     "NEW/DATA/ICMP.conf"
#define BCAST_FILE		     "NEW/DATA/BCAST.conf"		// 071011, poopee
#define PSDN_FILE                           "NEW/DATA/PDSN.conf"
#define RULESET_USED_FILE                   "NEW/DATA/RULESET_USED.conf"
#define CALL_OVER_CTRL_FILE                 "NEW/DATA/CALL_OVER_CTRL.conf"

#define SESS_TMP_FILE			"/tmp/"

#define SESS_INFO_SHM		10300
#define MAX_SVCTYPE_NUM     300
#define UCHAR unsigned char
#define MSGQ_MAX_SIZE		100  // 15 -> 100
#define CONF_START_LINE     "@START"
#define CONF_END_LINE       "@END"
#define MAX_COL_COUNT       20
#define MAX_COL_SIZE        100

#define TOK_UNKNOWN             0
#define TOK_COMMENT             1
#define TOK_EMPTY_LINE          2
#define TOK_CONF_LINE           3
#define TOK_START_LINE          4
#define TOK_END_LINE            5

#define COMM_FRM_MODE            0
#define AAA_FRM_MODE             1
#define URL_FRM_MODE             2
#define TRC_FRM_MODE             3
#define PDSN_FRM_MODE             4
#define ETC_FRM_MODE             5 

#define IPTYPE_IPPOOL           0
#define IPTYPE_PDSN             1
#define BUFSIZE                 1024
#define MMCMSGSIZE              4096
#define FILESIZE                256
// jjinri
#define PAGE_CNT				100  // jjinri UI 화면상의 한페이지 ROWS 수 

//yhshin #define RSYNC                   "/usr/bin/rsync"
#define RSYNC                   "/usr/local/bin/rsync"

#define DAY2SEC                             24*60*60
#define DAY2HOUR                            24
#define HOUR2SEC                            60*60
#define HOUR2MILISEC                        60*60*1000
enum {CMD_USE_OFF=0, CMD_USE_ON};

#define DELETE_TRACE_TIME		(60*60)*6
typedef struct _ConfLine{
    char    gubun;
    char    columns[MAX_COL_COUNT][MAX_COL_SIZE];
    char    line[BUFSIZ];
}ConfLine, *pConfLine;

typedef struct _DLinkedList{
    struct _DLinkedList *prev;
    struct _DLinkedList *next;
    ConfLine    item;
}DLinkedList, *pDlinkedList;

typedef void (*MMC_FUNC)(IxpcQMsgType*, void *);
typedef int (*MMC_PRC)( void * ,IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,
                        void * ,MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail,
                        char columns[MAX_COL_COUNT][MAX_COL_SIZE], char fname[FILESIZE] );

#define STR_RULE_NAME_LEN   48
            
#define	MAX_RULE_SET_LIST	5000
int	g_TotalRuleSetCnt;
typedef struct __RuleSetList__ {
	unsigned short	pBit;
	unsigned short	hBit;
	unsigned short	pkgNo;
} RuleSetList;

#define MMCR_MAX_MMC_HANDLER	70
typedef struct _MmcFuncTable{
	char cmdName[20];
	MMC_FUNC mmcFunc;
    MMC_PRC mmcPrc;
	char *confFile;
	int cmdType;
	int	paraCnt;
	int mode;
    int frmMode;
	int keyCnt;
    int startIndex;
} MmcFuncTable, *pMmcFuncTable;

#define MMC_MEMBER_MAX 20
typedef struct _MmcMemberTable{
    char cmdName[20];
    char paraName[MAX_COL_COUNT][MAX_COL_SIZE];
    int printFormat[MAX_COL_COUNT];
    int paraCnt;
} MmcMemberTable, *pMmcMemberTable;

typedef struct _enumTable{
    char    name[MAX_COL_SIZE];
    int     value;
}stEnumTable;

extern int mmcr_mmcHdlrVector_qsortCmp (const void *a, const void *b);
extern int mmcr_mmcHdlrVector_bsrchCmp (const void *a, const void *b);

extern int mmcr_mmcMemberHdlrVector_qsortCmp (const void *a, const void *b);
extern int mmcr_mmcMemberHdlrVector_bsrchCmp (const void *a, const void *b);

/* mmcr_init.c */
extern int InitSys();
extern int mmcr_initLog (void);
extern int UnBlockSignal(int SIGNAL);

/* mmcr_rxtxmsg.c */
extern int HandleRxMsg (void);
extern int MMCReqBypassSnd (IxpcQMsgType *rxIxpcMsg);
extern int MMCResSnd (IxpcQMsgType *rxIxpcMsg, char *resBuf, char resCode, char contFlag);
extern int SendAppNoty( GeneralQMsgType *sndGenQMsg );
extern int oldSendAppNoty(int appKind, USHORT sType, UCHAR sSvcID, UCHAR sMsgID);
extern int findMsgQID( int type );
extern char * convertNulltoDollar( char * item );


/* mmcr_mmchdl.c */
extern void doDisInfo(IxpcQMsgType *rxIxpcMsg, void *pt);
extern void doAddInfo(IxpcQMsgType *rxIxpcMsg, void *pt);
extern void doDelInfo(IxpcQMsgType *rxIxpcMsg, void *pt);
extern void doChgInfo(IxpcQMsgType *rxIxpcMsg, void *pt);
extern void doSetInfo(IxpcQMsgType *rxIxpcMsg, void *pt);
extern void doDisSvcInfo(IxpcQMsgType *rxIxpcMsg, void *pt);
extern void set_dup_conf(IxpcQMsgType *rxIxpcMsg, void *pt);
extern void headMessage ( pMmcMemberTable pmmcHdlr, char result[BUFSIZE] );
extern void tailMessage ( pMmcMemberTable mmcHdlr,char result[BUFSIZE], int cnt );
extern void conTailMessage ( pMmcMemberTable mmcHdlr,char result[BUFSIZE] );
extern void bodyLineMessage ( pMmcMemberTable mmcHdlr,DLinkedList *node, char result[BUFSIZE]);

extern int doDisCommProcess( void *,IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,void *,MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE], char fname[FILESIZE] );
extern int doAddCommProcess( void *,IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,void *, MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE], char fname[FILESIZE]);
extern int doChgCommProcess( void *,IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,void *, MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE], char fname[FILESIZE]);
extern int doDelCommProcess( void *,IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,void *,MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE], char fname[FILESIZE] );
extern int doSetCommProcess( void *,IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,void *,MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE], char fname[FILESIZE] );
extern int doAddSvcType(void *tbl, IxpcQMsgType *rxIxpcMsg,
                  GeneralQMsgType genQMsgType, void *pft,
                  MpConfigCmd *mcc, DLinkedList *head, DLinkedList *tail,
                  char columns[MAX_COL_COUNT][MAX_COL_SIZE],char  fname[FILESIZE]);

extern int doAddLimitProcess(  void *,IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,void *, MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE],char  fname[FILESIZE]);
extern int doAddTrcProcess(  void *tbl,IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,void *pft, MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE],char  fname[FILESIZE]);
extern int doChgTrcProcess(  void *tbl,IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,void *pft, MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE],char  fname[FILESIZE]);
extern int doDisTrcProcess( void *tbl ,IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,void *pft,MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE], char fname[FILESIZE] );
extern int doDelTrcProcess( void * tbl, IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,void *pft,MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE], char fname[FILESIZE] );
extern int doSetLogLevel( void *tbl, IxpcQMsgType *rxIxpcMsg,
                       GeneralQMsgType genQMsgType,void *pft,
                       MpConfigCmd *mcc, DLinkedList *head,
                       DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE],
                       char fname[FILESIZE] );
extern int doSetTimer( void *tbl, IxpcQMsgType *rxIxpcMsg,
                       GeneralQMsgType genQMsgType,void *pft,
                       MpConfigCmd *mcc, DLinkedList *head,
                       DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE],
                       char fname[FILESIZE] );
extern int doSetOneProcess( void *tbl, IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,void *pft,MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE], char fname[FILESIZE] );
extern int doSetPpsProcess( void *tbl, IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,
                     void *pft,MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail,
                     char columns[MAX_COL_COUNT][MAX_COL_SIZE], char fname[FILESIZE] );
extern int doSetIcmpProcess( void *tbl, IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,
                     void *pft,MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail,
                     char columns[MAX_COL_COUNT][MAX_COL_SIZE], char fname[FILESIZE] );
extern int doSetBcastProcess( void *tbl, IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,
		             void *pft,MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail,
					 char columns[MAX_COL_COUNT][MAX_COL_SIZE], char fname[FILESIZE] );
extern int doSetDupProcesss( void *tbl, IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,void *pft,MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE], char fname[FILESIZE] );
extern int doAddPdsnProcess(  void *tbl,IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,void *pft, MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE],char  fname[FILESIZE]);
extern int doDisPdsnProcess( void *tbl ,IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,void *pft,MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE], char fname[FILESIZE] );
extern int doDelPdsnProcess( void * tbl, IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,void *pft,MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE], char fname[FILESIZE] );
extern int doChgPdsnProcess( void * tbl, IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,void *pft,MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE], char fname[FILESIZE] );
extern int doDelSvcProcess( void * tbl, IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,void *pft,MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE], char fname[FILESIZE] );
extern int doChgSvcProcess( void * tbl, IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,void *pft,MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE], char fname[FILESIZE] );
extern int doSetUdrConfProcess(  void *tbl,IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,void *pft, MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE],char  fname[FILESIZE]);
extern int doSetDupProcesss( void *tbl, IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,void *pft,MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE], char fname[FILESIZE] );
extern int doDisSvcProcess( void *tbl ,IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,void *pft,MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE], char fname[FILESIZE] );
extern int doSetSwtProcess( void *tbl, IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,void *pft,MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE], char fname[FILESIZE] );
extern int doAddIpProcess(  void *tbl,IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,void *pft, MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE],char  fname[FILESIZE]);
extern int doDelIpProcess( void *tbl ,IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType,void *pft,MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE], char fname[FILESIZE] );

extern int doAddPdsnInfoShm(void *tbl, IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType, void *pft, MpConfigCmd *mcc, DLinkedList *head, DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE],char  fname[FILESIZE]);
extern int doDelPdsnInfoShm(void *tbl, IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType, void *pft, MpConfigCmd *mcc, DLinkedList *head, DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE],char  fname[FILESIZE]);
extern int doChgPdsnInfoShm(void *tbl, IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType, void *pft, MpConfigCmd *mcc, DLinkedList *head, DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE],char  fname[FILESIZE]);
extern int doAddIpPoolInfoShm(void *tbl, IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType, void *pft, MpConfigCmd *mcc, DLinkedList *head, DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE],char  fname[FILESIZE]);
extern int doDelIpPoolInfoShm(void *tbl, IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType, void *pft, MpConfigCmd *mcc, DLinkedList *head, DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE],char  fname[FILESIZE]);
extern int doChgIpPoolInfoShm(void *tbl, IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType, void *pft, MpConfigCmd *mcc, DLinkedList *head, DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE],char  fname[FILESIZE]);
extern int doAddSvcTypeShm(void *tbl, IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType, void *pft, MpConfigCmd *mcc, DLinkedList *head, DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE],char  fname[FILESIZE]);
extern int doDelSvcTypeShm(void *tbl, IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType, void *pft, MpConfigCmd *mcc, DLinkedList *head, DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE],char  fname[FILESIZE]);
extern int doChgSvcTypeShm(void *tbl, IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType, void *pft, MpConfigCmd *mcc, DLinkedList *head, DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE],char  fname[FILESIZE]);
extern int doDisUrlProcess( void *,IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType, void *, MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE], char fname[FILESIZE] );
extern int doDisAaaProcess( void *,IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType, void *, MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE], char fname[FILESIZE] );
extern int doDisPpsProcess( void *tbl ,IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType, void *pft, MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE], char fname[FILESIZE] );
extern int doDisIcmpProcess( void *tbl ,IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType, void *pft, MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE], char fname[FILESIZE] );
extern int doDisBcastProcess( void *tbl ,IxpcQMsgType *rxIxpcMsg, GeneralQMsgType genQMsgType, void *pft, MpConfigCmd *mcc, DLinkedList *head,DLinkedList *tail, char columns[MAX_COL_COUNT][MAX_COL_SIZE], char fname[FILESIZE] );

//file처리 함수

extern int open_conf_file(char *file, FILE **fp, char *mode);
extern int create_new_conf_list(DLinkedList *head);
extern int make_conf_list(char *fname, DLinkedList *head);
extern DLinkedList *new_node(DLinkedList *prev);
extern int seperate_columns(char *buff, char wbuff[MAX_COL_COUNT][MAX_COL_SIZE]);
extern void trim_trail_line(char *line);
extern DLinkedList *search_list_node(char srchStr[MAX_COL_COUNT][MAX_COL_SIZE],int startIndex, int compsize, DLinkedList *head );
extern DLinkedList *search_node(char *srchStr, int pos, DLinkedList *head);
extern DLinkedList *search_node2(char *srchStr, int pos, DLinkedList *head);
extern DLinkedList *search_node3(char *srchStr, int pos, DLinkedList *head);
extern DLinkedList *search_start_conf(DLinkedList *head);
extern DLinkedList *search_end_conf(DLinkedList *tail);
extern DLinkedList *insert_list_node(char insitem[MAX_COL_COUNT][MAX_COL_SIZE], int colsize, DLinkedList *tail);
extern void delete_list_node(DLinkedList *node);
extern DLinkedList *next_conf_node(DLinkedList *node);
extern DLinkedList *next_conf_node_compare(DLinkedList * node,char srchStr[MAX_COL_COUNT][MAX_COL_SIZE], int startIndex, int compsize, int paraCnt );
extern DLinkedList *delete_search_node(int startIndex, DLinkedList *head); // 2007.01.29 by helca

extern void fill_line_buff(char rbuff[MAX_COL_COUNT][MAX_COL_SIZE], char *wbuff, int rsize);
extern int flush_list_to_file(DLinkedList *head, char *fname);
extern void print_list_node(DLinkedList *head);
extern int getTotalNodeCnt();
extern char * convertNulltoDollar( char * item );
extern void sort_asc_conf_nodes(DLinkedList *head);
extern int compare_nodes(const DLinkedList *node1, const DLinkedList *node2);

// only for svc-type
extern DLinkedList *find_same_svctype_key(char srchStr[MAX_COL_COUNT][MAX_COL_SIZE], DLinkedList *head);
extern DLinkedList *insert_svctype_node(char insitem[MAX_COL_COUNT][MAX_COL_SIZE], int colsize, DLinkedList *head);
extern DLinkedList *search_jump_node(DLinkedList *head, int posSeq, int *seq);
extern int compare_strIPAddr(char *addr1, char *addr2);
extern unsigned char get_protocol_enum(char *proto);
extern void strtoupper( char headName[20] );


// mmcr_util.c
void conf_file_sync(char *filename);
char * strtoupper2(char *s1);
int deleteTime();
int callTraceDelete(time_t now);

// 
void dis_all_svc_info(IxpcQMsgType *rxIxpcMsg);
void dis_partly_svc_info(IxpcQMsgType *rxIxpcMsg, char *srchStr);
void compose_svc_info_header(char *buff);
void compose_svc_info_line(char *buff, char bodyPara[MAX_COL_COUNT][MAX_COL_SIZE],
                           char txcPara[MAX_COL_COUNT][MAX_COL_SIZE], int  type, void *vptr);
int make_svc_body(char *svcid, DLinkedList *svctype,
                   DLinkedList *urlcat, DLinkedList *udrtxc,
                   IxpcQMsgType *rxIxpcMsg);
int make_udr_body(char *svcid, DLinkedList *udrinfo,
                   DLinkedList *svctype, DLinkedList *udrtxc,
                   IxpcQMsgType *rxIxpcMsg);
int make_cdr_body(char *cdrsvcid, char *svcid, DLinkedList *svctype,
                   DLinkedList *udrtxc, IxpcQMsgType *rxIxpcMsg);
void uniq_svc_id(DLinkedList *head, char *svcidarr[], int pos);

//
int InitShm_IpPool_svcType();

//
void loadLogLevel(void);
void loadTimer(void);
int mdb_init(void);
void loadPpsIcmpConf(void);
int set_version(int prc_idx, char *ver);
int dInit_IPPOOLBIT( int dKey );
int dSetIPPOOLList( UINT uiIPAddress, UINT uiNetMask, UCHAR ucStatus, pst_IPPOOLLIST pstIPPOOL );
extern int check_my_run_status (char *procname);
void set_head_tail(DLinkedList *head, DLinkedList *tail);
void delete_all_list_node(DLinkedList *head, DLinkedList *tail);
void bodyLineUrlMessage ( pMmcMemberTable mmcHdlr,DLinkedList *node, char result[BUFSIZE] );
void bodyLineAaaMessage ( pMmcMemberTable mmcHdlr,DLinkedList *node, char result[BUFSIZE] );
void bodyLineTrcMessage ( pMmcMemberTable mmcHdlr,DLinkedList *node, char result[BUFSIZE] );
void bodyLinePdsnMessage ( pMmcMemberTable mmcHdlr,DLinkedList *node, char result[BUFSIZE] );
void bodyLineSvcMessage ( pMmcMemberTable mmcHdlr,DLinkedList *node, char result[BUFSIZE] );
void addBodyLineIcmpMessage(pMmcMemberTable mmcHdlr, char columns[MAX_COL_COUNT][MAX_COL_SIZE], char result[BUFSIZE], int type );
void addHeadMessage ( pMmcMemberTable pmmcHdlr, char result[BUFSIZE] );
void addBodyLineMessage ( pMmcMemberTable mmcHdlr,char columns[MAX_COL_COUNT][MAX_COL_SIZE],char result[BUFSIZE], int type );
void addBodyLineAaaMessage ( pMmcMemberTable mmcHdlr,char columns[MAX_COL_COUNT][MAX_COL_SIZE], char result[BUFSIZE] , int type);
void addBodyLineUrlMessage ( pMmcMemberTable mmcHdlr,char columns[MAX_COL_COUNT][MAX_COL_SIZE], char result[BUFSIZE], int type );
void addBodyLineTrcMessage ( pMmcMemberTable mmcHdlr,char columns[MAX_COL_COUNT][MAX_COL_SIZE], char result[BUFSIZE],int type );
void addTailMessage ( pMmcMemberTable mmcHdlr,char result[BUFSIZE] );
int Init_MMDBDESTIP();
int Init_MMDBDESTPORT();
int Init_MMDBSVCOPT();
void addBodyLinePdsnMessage ( pMmcMemberTable mmcHdlr,char columns[MAX_COL_COUNT][MAX_COL_SIZE], char result[BUFSIZE], int type );
void addBodyLinePpsMessage(pMmcMemberTable mmcHdlr, char columns[MAX_COL_COUNT][MAX_COL_SIZE], char result[BUFSIZE], int type );
void addBodyLineSvcMessage ( pMmcMemberTable mmcHdlr,char columns[MAX_COL_COUNT][MAX_COL_SIZE], char result[BUFSIZE], int type );
stHASHONODE *hasho_add_new(stHASHOINFO *pstHASHOINFO, U8 *pKey, U8 *pData, S32 *pRet );
int mmcr_dLoadTimeOut(void);
void mmcr_dLogTimeOut(void);
int mmcr_dWriteTimeOut(void);

void UserControlledSignal(int sign);
void handleChildProcess(int sign);
#endif //  __MMCR_H_

