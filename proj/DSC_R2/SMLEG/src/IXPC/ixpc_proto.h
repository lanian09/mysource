#ifndef __IXPC_H__
#define __IXPC_H__

#include <sys/errno.h>
#include <sys/msg.h>
#include <sys/sysinfo.h>
#include <sys/shm.h>

#include "socklib.h"
#include <comm_msgtypes.h>
#include <sfm_msgtypes.h>
#include <sysconf.h>
#include <commlib.h>
#include <comm_almsts_msgcode.h>


#define IXPC_LOG_FILE			"APPLOG/OAM/ixpc_log"
#define IXPC_ERRLOG_FILE		"APPLOG/OAM/ixpc_err"
#define IXPC_RECONNECT_INTERVAL		3
#define IXPC_CONN_CHECK_PERIOD		10
#define IXPC_AUDIT_TIMER			60
#define IXPC_PING_TEST_PERIOD		30


/* �ý��� ���ο� �ִ� application���� message queue�� ���� �����ϱ� ����
**	�� application�鿡 ���� ������ ����Ǵ� structure�̴�.
*/
typedef struct {
	int		pres;			/* present(1) or not_used(0) */
	int		msgQkey;		/* message queue key */
	char	appName[32];	/* application name */
} IXPC_MsgQRoutTable;

typedef struct {
	int		pres;			/* present(1) or not_used(0) */
	int		sockTxFd;		/* remote system���� ������ �۽ſ� socket fd */
	int		sockRxFd;		/* remote system���� ������ ���ſ� socket fd */
	time_t	disconnTime;	/* ������ ������ �ð� -> �������ϱ� ���� interval�� �α����� */
	time_t	lastTxTime;		/* ���������� �޽����� ���� �ð� -> �ֱ����� connection check�� ���� */
	time_t	lastRxTime;		/* ���������� �޽����� ������ �ð� -> �ֱ����� connection check�� ���� */
	char	sysType[32];	/* remote system type */
	char	sysGroup[32];	/* remote system group name */
	char	sysName[32];	/* remote system name */
	char	ipAddrPri[32];	/* primary ip address of remote system */
	char	ipAddrSec[32];	/* secondary ip address of remote system */
} IXPC_SockRoutTable;

extern int g_dStopFlag;
extern void UserControlledSignal(int);
extern void IgnoreSignal(int);
extern void SetUpSignal(void);
extern void FinishProgram(void);
extern void bind_socket_release();

extern int	errno;

extern int ixpc_initial (void);
extern int ixpc_initMsgQRoutTbl (char*);
extern int ixpc_initSockRoutTbl (char*);
extern int ixpc_initLog (void);
extern int ixpc_newConnEvent (int);
extern int ixpc_recvEventRxPort (int, SockLibMsgType*);
extern int ixpc_recvEventTxPort (int);
extern int ixpc_disconnEventRxPort (int);
extern int ixpc_disconnEventTxPort (int);
extern int ixpc_exeRxQMsg (GeneralQMsgType*);
extern int ixpc_intRoute (GeneralQMsgType*);
extern int ixpc_extRoute (GeneralQMsgType*);
extern int ixpc_extRouteSend (GeneralQMsgType*, int);
extern int ixpc_isMyMsg (GeneralQMsgType*);
extern int ixpc_exeMyMsg (GeneralQMsgType*);
extern int ixpc_checkConnections (void);
extern int ixpc_connect2Remote (int);
extern int ixpc_txConnCheckMsg (int);
extern int ixpc_isSysTypeRoute (char*);

extern int set_version(int prc_idx, char *ver);
extern int check_my_run_status (char *procname);
#endif /*__IXPC_H__*/
