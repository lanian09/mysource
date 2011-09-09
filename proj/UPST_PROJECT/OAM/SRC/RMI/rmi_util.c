/*******************************************************************************
			DQMS Project

	Author   :
	Section  : RMI
	SCCS ID  : @(#)rmi_util.c	1.1
	Date     : 07/21/01
	Revision History :
        '01.  7. 21     Initial
        '03.  1. 15     Initial

	Description :

	Copyright (c) uPRESTO 2005
*******************************************************************************/

/***** A.1 * File Include *******************************/
/* SYS HEADER */
#include "rmi_sock.h"
#include "rmi_util.h"
/* LIB HEADER */
/* PRO HEADER */
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
/* LOC HEADER */

static  char    mtime_str[81];

/**D.1*  Definition of Functions  *************************/
/**D.2*  Definition of Functions  *************************/

void exe_history ()
{
    strcpy (command, his[his_num%MAX_HIS].cmd);
    printf ("\n[%s] %s ", SmsName, command);
}

void dis_history ()
{
    int     idx, i;
    char    hbuf[2048];
    char    tmp_str[1024];

    sprintf (hbuf, "%s\n%s\n%s\n",
            "-------------------------------------------------",
            "NUMBER  COMMAND",
            "-------------------------------------------------");

    idx = his_num>20 ? (his_num-20)%MAX_HIS : 1;
    for (i=0; i<MAX_HIS && i<his_num; i++, idx++) {
        sprintf (tmp_str, "%6d    %-s\n", his[idx%MAX_HIS].number, his[idx%MAX_HIS].cmd);
        strcat (hbuf, tmp_str);
    }
    strcat (hbuf,
            "-------------------------------------------------\n\n");
    printf ("\n%s\n[%s] ",SmsName, hbuf);
}

int hist_func(char *cmd_buf)
{
    if (save_his (cmd_buf) < 0)
        return -1;

    if (cmd_buf[0] == '!')
        exe_history ();

    if (!strcasecmp (cmd_buf, "history") ||
        !strcasecmp (cmd_buf, "his") ||
        !strcasecmp (cmd_buf, "h"))

    if (!strcasecmp (cmd_buf, "dis-cmd-his") )
    {
        dis_history ();
        return -1;
    }
    return 1;
}

int save_his(char *mml_line)
{
    int  back, i, idx;

    his_num++;

    his[his_num%MAX_HIS].number = his_num;

    if (mml_line[0] == '!') {
        if (mml_line[1] == '!') {
            strcpy(his[his_num%MAX_HIS].cmd, his[(his_num-1)%MAX_HIS].cmd);
            strcat(his[his_num%MAX_HIS].cmd, mml_line+2);
            return 1;

        } else if(isdigit(mml_line[1])) {
            back = his_num - atoi (&mml_line[1]);
            if (back <= 0 || back > 21) {
                printf ("[General Error] Event Not Found : %s\n\n", mml_line);
                his_num--;
                return -1;
            }

            strcpy (his[his_num%MAX_HIS].cmd, his[(his_num-back)%MAX_HIS].cmd);
            strcat (his[his_num%MAX_HIS].cmd, mml_line + 2);
            return 1;

        } else {
            for (i=1; i<=his_num; i++) {
                idx = (his_num-i)%MAX_HIS;
                if (!strncmp (mml_line+1, his[idx].cmd, strlen (mml_line+1))) {
                    strcpy (his[his_num%MAX_HIS].cmd, his[idx].cmd);
                    return 1;
                }
            }
            printf ("[General Error] Event Not Found : %s\n", mml_line);
            his_num--;
            return -1;
        }
    }

    strcpy (his[his_num%MAX_HIS].cmd, mml_line);
    return 1;
}

char *mtime(void)
{
	time_t	t;

	t = time(NULL);
	strftime(mtime_str, 80, "%Y-%m-%d %T %a", localtime(&t));
	mtime_str[21]	= toupper(mtime_str[21]);
	mtime_str[22]	= toupper(mtime_str[22]);

	return mtime_str;
}

int	prn_hexa(char *msg, int dSize)
{
	int		i;

	for(i = 0; i < dSize; i++)
	{
		if( (i%20) == 0)
		{
			printf("\n");
			printf( "%04d", i);
			fflush( stdout );
		}

		if( (i%5) == 0)
			printf( " |");

		printf(" %02X", msg[i]);
	}

	return 1;
}

void UserControlledSignal2(int sign)
{
    Finishflag = sign;
	close(dSfd);
	exit(0);
}

void UserControlledSignal(int sign)
{
    Finishflag = sign;

	FinishProgram();
}

void IgnoreSignal(int sign)
{
    if(sign != SIGALRM)
    {
        printf("UNWANTED SIGNAL IS RECEIVED, signal = %d\n", sign);
    }

    signal(sign, IgnoreSignal);
}

void FinishProgram(void)
{
    printf("PROGRAM IS NORMALLY TERMINATED, Cause = %d\n", Finishflag);

	logout_win();

    send_login_sts(MI_LOGOUT, "SUCCESS");

    close(dSfd);

    exit(0);
}

int init_SIG(void)
{
    /* WANTED SIGNALS   */
    signal(SIGTERM, UserControlledSignal);
    signal(SIGINT,  UserControlledSignal2);
    signal(SIGQUIT, UserControlledSignal);
#if 0	/* 040130,poopee */
    signal(SIGSEGV, UserControlledSignal);
#endif

    /* UNWANTED SIGNALS */
    signal(SIGHUP,  IgnoreSignal);
    signal(SIGALRM, IgnoreSignal);
    signal(SIGPIPE, IgnoreSignal);
    signal(SIGPOLL, IgnoreSignal);
    signal(SIGPROF, IgnoreSignal);
    signal(SIGUSR2, IgnoreSignal);
    signal(SIGVTALRM, IgnoreSignal);

    return 1;
}
