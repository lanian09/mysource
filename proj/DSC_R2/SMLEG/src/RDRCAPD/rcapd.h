#ifndef __RCAPD_H__
#define __RCAPD_H__

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "ipaf_define.h"
#include "ipaf_svc.h"
#include "comm_msgtypes.h"
#include "utillib.h"
#include "keepalivelib.h"
#include "mmc_hld.h"
#include "rcapd_file.h"


#define     HIPADDR(d)      ((d>>24)&0xff),((d>>16)&0xff),((d>>8)&0xff),(d&0xff)
#define     NIPADDR(d)      (d&0xff),((d>>8)&0xff),((d>>16)&0xff),((d>>24)&0xff)

/* Connection munber with SM */

#define _SYS_NAME_MPA						"SCMA"
#define _SYS_NAME_MPB						"SCMB"

#define CSV_PATH                    		"/CM/scmscm/cm/adapters/CSVAdapter/csvfiles/"
#define TRANSACTION_RDR_PATH        		CSV_PATH"4042321936/"
#define BLOCK_RDR_PATH              		CSV_PATH"4042321984/"
//#define TRANSACTION_RDR_PATH        		CSV_PATH"4042321001/"
//#define BLOCK_RDR_PATH              		CSV_PATH"4042321002/"

#define MMC_MSG_SIZE              			4096
#define FILE_PATH_SHORT_LEN					128
#define FILE_PATH_LONG_LEN					256	

enum { RDR_TYPE_TRANSACTION = 1, RDR_TYPE_BLOCK = 2 };
enum { RDR_EXIST_TRS = 1, RDR_EXIST_BLK, RDR_EXIST_BOTH };
/* DEBUGGING ¿ë */

/////////////////////////////////////////////////////////////////////////////////////
/* global variable definition */
extern char sysLable[COMM_MAX_NAME_LEN], mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN];
extern int  dMyQid, dIxpcQid, dANAQid;
extern int  dANAQid;
extern int	JiSTOPFlag, FinishFlag;
extern TAIL    *g_pTail[2];
/////////////////////////////////////////////////////////////////////////////////////
/* main.c */

/* init.c */
extern int check_my_run_status (char *procname);
extern void SetUpSignal(void);
extern int initProc(void);
extern void finProc(void);
extern int dGetConfig_LEG (void);

/* msgq.c */
extern int msgQRead	(int tid, pst_MsgQ pstMsgQ);
extern void makeCpsData (void);
extern void report_LEGStatData2STMD  (int idx);
extern void report_LOGONStatData2STMD  (int idx);
extern void makeStatData(int idx);
extern void report_CPSData2FIMD  (void);
extern int branchMessage (GeneralQMsgType *prxGenQMsg);
extern void dSendMsg_RDRANA(int qid, int msg_type, char *data, int dataLen);
extern char *getStrRdrType (int type);

/* file.c */
extern TAIL *openTail (char *fname);
extern int readTail (TAIL **RTAIL, size_t size, int sec, int isRdrExist);
extern void closeTail (TAIL *OTAIL);
extern int findTargetFile (int rdrtype, char *fname);

/* proc.c */
extern void theApp (void);
extern unsigned getLineBufLen ( char * p, char * end );
extern int getBackupFileName (char *fname, char *bkname);
extern int	procRdrStatus (int result, int rdrtype, TAIL *pTail);

#endif /* __RCAPD_H__ */

