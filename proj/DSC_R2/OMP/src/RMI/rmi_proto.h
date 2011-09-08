#ifndef __RMI_PROTO_H__
#define __RMI_PROTO_H__

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <pthread.h>
#include <comm_msgtypes.h>
#include <sysconf.h>
#include <commlib.h>

#define RMI_NUM_HISTORY_BUFF	20
#define RMI_REQID_LOGIN			100
#define RMI_REQID_LOGOUT		101
#define RMI_REQID_GENERAL_CMD	102
#define RMI_REQID_BATCH_CMD		103
#define NEXT(a,b)	((a+1)%b)


extern int errno;

extern int rmi_initial (int ac, char *av[]);
extern int rmi_getArgs (int ac, char *av[], char*, int*);
extern int rmi_terminate (int);
#if 0 /* jhnoh : 030815 */
//extern int rmi_send2mmcd (char*, int, char);
#else
extern int rmi_send2mmcd (char*, char, int);
#endif
extern int rmi_saveInputHistory (char*);
extern int rmi_isBuiltInCmd (char*);
extern int rmi_builtin_open_log (char*);
extern int rmi_builtin_close_log (char*);
extern int rmi_builtin_act_cmd_file (char*);
extern int rmi_builtin_file_exe (char*);
extern int rmi_builtin_history (char*);
extern int rmi_builtin_dis_history (void);
extern void rmi_receiveResult (int);
extern int rmi_WaitResFromMMCD (void);

extern int rmi_login2mmcd(void);

#endif //__RMI_PROTO_H__
