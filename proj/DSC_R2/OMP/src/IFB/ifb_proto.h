#ifndef __IFB_PROTO_H__
#define __IFB_PROTO_H__

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/procfs.h>
#include <sys/signal.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <dirent.h>
#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>
#include <stropts.h>
#include <commlib.h>
#include <comm_msgtypes.h>
#include <sysconf.h>
#include <proc_version.h>
#include <crypt.h>

#define PROC_DIR		"/proc"
#define PASSWD_FILE		"/etc/passwd"
#define KILLPRC_LOG_FILE       "LOG/OAM/killprc_log"
#define KILLPRC_ERRLOG_FILE    "LOG/OAM/killprc_err"
#define STARTPRC_LOG_FILE       "LOG/OAM/startprc_log"
#define STARTPRC_ERRLOG_FILE    "LOG/OAM/startprc_err"
#define RETRY_COUNT     	3
#define PASSWORD_FILE   	"DATA/mml_passwd"

typedef struct {
	char	procName[16];
	char	exeName[256]; // 실행화일의 full path name
	int		msgQkey;
	int		runCnt;
	pid_t	pid;
	char	startTime[32];
	char    procVersion[10];
} IFB_ProcInfoContext;


extern int errno;

extern int disprc_getArgs (int ac, char *av[]);

extern int startprc_getArgs (int ac, char *av[]);
extern void startprc_startupProc (int);
extern int startprc_printProcListPrompt (void);
extern void startprc_interrupt (int);

extern int killprc_getArgs (int ac, char *av[]);
extern int killprc_shutdownProc (int);
extern int killprc_getSAMDqid (void);
extern void killprc_notify2samd (int);

extern int ifb_getOpAccount (char*);
extern int ifb_checkLogin (void);
extern int ifb_getProcIndex (char*);
extern int ifb_getProcStatus (void);
extern int ifb_getGuiStatus (void);
extern void ifb_printProcStatus (void);
extern int ifb_setConfProcTbl (void);
extern int ifb_promptYesNo (void);
extern int ifb_promptYesNo2 (char*);
extern int ifb_killProc (pid_t);
extern int ifb_clearQ (int, int);

extern int check_user_valid(FILE *, char *, char *);
extern int interact_w(char *);

#endif //__IFB_PROTO_H__
