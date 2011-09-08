#ifndef __IFB_PROTO_H__
#define __IFB_PROTO_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
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
#include <crypt.h>
#include <commlib.h>
#include <comm_msgtypes.h>
#include <comm_proc.h>
#include <sysconf.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
//#include <asm/param.h>
#include <sfm_msgtypes.h>
#include <ipaf_names.h>
#include <ctype.h>

#define	PROC_DIR			"/proc"
#define	PASSWD_FILE			"/etc/passwd"
#define	RETRY_COUNT			3
#define	PASSWORD_FILE		"NEW/DATA/mml_passwd"
#define	IV_HOME				"IV_HOME"

#ifndef PARENT_PATH
#define PARENT_PATH			".."
#endif
#ifndef HOME_PATH
#define HOME_PATH			"."
#endif


typedef struct {
	char	procName[16];
	char	exeName[256]; // 실행화일의 full path name
	int		msgQkey;
	int		runCnt;
	pid_t	pid;
	char	startTime[32];
    char    procVersion[10];
} IFB_ProcInfoContext;

// version seq count
/*
#define SEQ_PROC_IXPC                   0
#define SEQ_PROC_SAMD                   1
#define SEQ_PROC_MMCR                   2
#define SEQ_PROC_STMM                   3
#define SEQ_PROC_CAPD                   4
#define SEQ_PROC_ANA                    5
#define SEQ_PROC_CDR                    6
#define SEQ_PROC_TRCDR                  7
#define SEQ_PROC_WAP1ANA                8
#define SEQ_PROC_UAWAPANA               9
#define SEQ_PROC_WAP2ANA                10
#define SEQ_PROC_HTTPANA                11
#define SEQ_PROC_UDRGEN                 22
#define SEQ_PROC_AAAIF                  23
#define SEQ_PROC_SDMD                   24
*/

typedef struct{
    char    *name;
    int     index;
}VersionIndexTable;

extern VersionIndexTable vit[];
extern int errno;
extern SFM_SysCommMsgType   *loc_sadb;

extern int disprc_getArgs (int ac, char *av[]);

extern int startprc_getSAMDqid();
extern int startprc_getArgs (int ac, char *av[]);
extern void startprc_startupProc (int);
extern int startprc_printProcListPrompt (void);
extern void startprc_interrupt (int);
extern void startprc_notify2samd (int);

extern int killprc_getArgs (int ac, char *av[]);
extern int killprc_shutdownProc (int);
extern int killprc_getSAMDqid (void);
extern void killprc_notify2samd (int);

extern int ifb_getOpAccount (char*);
extern int ifb_checkLogin (void);
extern int ifb_getProcIndex (char*);
extern int ifb_getProcStatus (int );
extern pid_t ifb_getPid (char*);
extern void ifb_printProcStatus (void);
extern int ifb_setConfProcTbl (void);
extern int ifb_promptYesNo (void);
extern int ifb_promptYesNo2 (char choice);
extern int ifb_killProc (pid_t);
extern int ifb_clearQ (int, int);

extern int init_ver_shm();
extern int init_sadb_shm (void);
extern int detatch_ver_shm();
extern void get_version(int prc_idx, char *ver);
extern char * get_ver_str(char *procname);

extern int check_user_valid(FILE *, char *, char *);
extern int interact_w(char *);


#endif //__IFB_PROTO_H__
