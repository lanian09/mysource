/**
	@file		mmcd_init.h
	@author
	@version
	@date		2011-07-14
	@brief		mmcd_init.c 헤더파일
*/

#ifndef __MMCD_INIT_H__
#define	__MMCD_INIT_H__

#include <errno.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <mysql/mysql.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/socket.h>
#include <unistd.h>

#include "nifo.h"
#include "cifo.h"
#include "gifo.h"
#include "mems.h"

#include "dblib.h"
#include "filelib.h"
#include "loglib.h"
#include "sockio.h"
#include "verlib.h"

#include "commdef.h"
#include "path.h"
#include "mmcdef.h"
#include "msgdef.h"
#include "sshmid.h"

#include "cmd_get.h"
#include "mmcd_db.h"


/**
 * Define constants
 */
#define MMCD_LOG					LOG_PATH"MMCD"
#define MAX_STRING_BUFFER			262144
#define CLIENT_CHECK_TIME			5
#define DEF_SYS_MMCD				7
#define MAX_COM_SYMBOL				512



/**
 * Define structures
 */

/**
 * Declare variables
 */
stMEMSINFO		*gpRECVMEMS;
stCIFO          *pCIFO;

unsigned short	g_usSeq;
char            g_cmd_line[256];

/* MySQL을 사용하기 위한 변수 */
MYSQL			stMySQL;
st_ConnInfo		stConnInfo;
char       		szIP[16], szName[32], szPass[32], szAlias[32];

/* mmcd_global 에서 옮김 */
st_MngPkt		cli_msg;
RUN_TBL         *run_tbl;
LIB_TBL			lib_tbl[MAX_LIB_TABLE];
COM_TBL			cmd_tbl[MAX_COM_SYMBOL];
T_ADMIN_LIST	stAdmin;
st_ConTbl		stConTbl[MAX_FDSET2+1];
fd_set			gstReadfds;
fd_set			gstWritefds;
st_MSG_BUF		gstMsgBuf[MAX_FDSET2];

char			SmsName[20];
char			JCstr_buff[MAX_STRING_BUFFER];
int				ptr_JCstr;
int				gdNumfds;
int				gdNumWfds;
int				gdSvrSfd;
int				num_cmd_tbl;
int				num_enumlist;
int				gdStopFlag;
int				myBlockCode;
int				num_para_tbl;
int				gdIndex;


/**
	Declare functions
*/
extern int dInsert_MMCDMsg(MYSQL *pstMySQL, st_SysMMCDMsg *pstData);
extern int dCreate_MMCDMsg(MYSQL *pstMySQL);

extern void SetUpSignal();
extern void FinishProgram();
extern int dTcpSvrModeInit();
extern int dInitIPCs();
extern int GetUserInfo(char *UserName, int Type);
extern int set_version(key_t key, int prc_idx, char *ver);
extern int dIsRcvedMessage();
extern char* util_cvtipaddr(char* szIP, unsigned int uiIP);
extern int dRecvMessage(int dRsfd, char *szTcpRmsg, int* dRecvLen);
extern int dGetMaxFds();
extern int dMergeBuffer(int dIdx, int dSfd, char* szTcpRmsg, int dRecvLen);
extern int dSendBlockMessage(int dSsfd, int dMsgLen, char *szSmsg, int stConIdx);
extern int dSetSockBlock(int dReturn, int dSockFD, int stConIdx);
extern int Init_tbl();
extern int dAdminInfoInit();
extern char *ComposeHead();
extern int dGetConTblIdx(int dSockfd);
extern int dSendMessage(int dSsfd, int dMsgLen, char *szSmsg, int stConIdx);
extern int dGetNMSsfd();
extern int SendToPROC(st_MsgQ *in_msg, int MsgID);
extern void util_makenid(unsigned char ucSysType, long long *pllNID);

extern int SendToOMP(dbm_msg_t*, unsigned short);
extern void clear_my_tmr(short i);
extern int out_print(char *outbuf, dbm_msg_t *msg, unsigned short usLen, short tmrID, short sRet, short cont_flag);
extern void Timer(time_t tTime);
extern int dCheckStatFlag(short StatFlag);
extern int dInsert_MMCD_Result(st_MngPkt *pstMngPkt, short tmrID, char *szBuf, int dSockfd);
extern int init_mmc_server();
extern int dSocketCheck();
extern void CheckClient(int dReloadFlag);

/**
	Declare functions
*/
extern int dInitIPCs();
extern void UserControlledSignal(int sign);
extern void FinishProgram(void);
extern void IgnoreSignal(int sign);

#endif	/* __MMCD_INIT_H__ */
