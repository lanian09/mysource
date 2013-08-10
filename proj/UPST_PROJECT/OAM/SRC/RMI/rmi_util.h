#ifndef _RMI_UTIL_H_
#define _RMI_UTIL_H_

/**
 *	INCLUDE HEADER FILES
 */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

// OAM/INC
#include "mmcdef.h"

// .

#define MAX_HIS			21

typedef struct {
    int     number;
    char    cmd[256];
} his_t;

/**
 *	DECLARE VARIABLES
 */
int     his_num;

extern  char   *mtime(void);

char    command[1024];
extern his_t   his[];
extern int		dSfd;
extern int Finishflag;
extern char SmsName[];

/**
 *	DELCARE FUNCTIONS
 */
int		save_his(char *mml_line);
void	FinishProgram(void);

extern void	logout_win(void);
extern int	send_login_sts (int msgtype, char *result);
extern int dSockInit(char *szIPAddr);


#endif	/* _RMI_UTIL_H_ */
