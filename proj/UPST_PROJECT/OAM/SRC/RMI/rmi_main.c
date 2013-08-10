/*******************************************************************************
			DQMS Project

	Author   :
	Section  : RMI
	SCCS ID  : @(#)rmi_main.c	1.1
	Date     : 07/21/01
	Revision History :
        '01.  7. 21     Initial
        '03.  1. 15     Initial

	Description :

	Copyright (c) uPRESTO 2005
*******************************************************************************/

/***** A.1 * File Include *******************************/

/* SYS HEADER */
/* LIB HEADER */
/* PRO HEADER */
#include "mmcdef.h"
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
/* LOC HEADER */
#include "gethostlib.h"
#include "rmi_util.h"
#include "rmi_sock.h"

his_t   his[MAX_HIS];
st_MngPkt cli_msg;

char    my_login[20], my_passwd[20];
char    SmsName[20];
int     rmi_ID;
int		Finishflag;
time_t	tCheck_time;
char szPriAddr[64];
char	szLogbuf[2048];
long long gllTid=0;

extern  int dSfd;

extern char    command[];

/**D.1*  Definition of Functions  *************************/
int		get_smsname(void);
int		login_win(void);
void	logout_win(void);
int		send_login_sts(int msgtype, char *result);
int     dSockInit(char *szIPAddr);

extern int init_SIG(void);
extern int hist_func(char *cmd_buf);
extern int dSendMessage(int dSsfd, int dMsgLen, char *szSmsg);

/**D.2*  Definition of Functions  *************************/
int main(int argc, char *argv[])
{
	int			i, ret, smsg_len, his_res, input_len;
	char		szCheck[24];
	st_MngPkt	mml;

	signal(SIGTTIN, SIG_IGN);
	gllTid = 0;

	get_smsname();

	if( (ret = dGetHostIP(szPriAddr)) < 0)
	{
		printf("dGetConnectIP Error\n");
		exit(0);
	}

	if( (ret = dSockInit(szPriAddr)) < 0)
	{
		printf("dSockInit Error\n");
		exit(0);
	}

	init_SIG();

    /*
    ** MMCD SERVER¿¡ loginÇÑ´Ù.
    */
	tCheck_time = time(NULL);

	if(login_win() < 0)
	{
		sprintf (szLogbuf, "\n[RMI] LOGIN FAILURE\n");
		printf("%s", szLogbuf );

		send_login_sts(MI_LOGIN, "FAIL");

		exit (0);
	}

/*
    send_login_sts(MI_LOGIN, "SUCCESS");
*/

    signal (SIGALRM, SockCheck);
    alarm (1);

    //printf ("\n[%s] ", SmsName);

    for ( ; ; ) {
        memset (&mml, 0, sizeof (st_MngPkt));

        smsg_len = his_res = input_len =  0;

        while (fgets (command, 1024, stdin) == NULL) {
            if (errno == EINTR)
                continue;
            logout_win();

            send_login_sts(MI_LOGOUT, "SUCCESS");

            close(dSfd);
            printf("[%s] LogOut. Good bye !!!\n\n", SmsName);
            exit (0);
        }

		time( &tCheck_time );

        command[strlen (command)-1] = '\0';

		sscanf( command, "%s", szCheck );

        if( !strcasecmp( szCheck, "user-login" ) && strcmp (command, "\0") )
        {
            printf("\n       CANNOT EXECUTING THIS COMMAND\n ");

            printf ("\n[%s] ", SmsName);

            continue;
        }

        printf ("\n[%s] ", SmsName);
        input_len = strlen (command);
        if (input_len == 0 ||
            !strcmp (command, "\0")) {
            continue;
        }

        for (i=0; i<input_len; i++) {
            if (!isspace (command[i]))  break;
        }
        if (i == input_len){
            continue;
        }

        if (!strcasecmp (command, "exit")) {
            logout_win();

            send_login_sts(MI_LOGOUT, "SUCCESS");

            close(dSfd);
            printf("[%s] LogOut. Good bye !!!\n\n", SmsName);
            exit (0);
        }

        his_res = hist_func(command);

        if(his_res < 0) continue;

		mml.head.llMagicNumber = MAGIC_NUMBER;
		mml.head.llIndex = gllTid++;

		mml.head.usResult = DBM_SUCCESS;

		mml.head.usSrcProc = 1;
		mml.head.ucBinFlag = 0;
		sprintf( mml.head.TimeStamp ,"%s", mtime() );


        mml.head.usBodyLen = strlen(command);
		mml.head.usTotLen = mml.head.usBodyLen + MNG_PKT_HEAD_SIZE;

        sprintf (mml.head.userName, "%s", my_login);
        sprintf (mml.data, "%s", command);

        ret = dSendMessage (dSfd, mml.head.usTotLen, (char *)&mml);

		if( ret < 0 )
		{
			printf("[%s] Program Ended !!!!\n", SmsName);
			exit(0);
		}
    }
}

int login_win(void)
{
	int			i, j, dLogCnt, mlen, rvalue;
	char		szSmsg[MNG_PKT_HEAD_SIZE + MAX_MMCD_MSG_SIZE];
	st_MngPkt	login_msg;

	dLogCnt	= 0;
	for(i = 0; i < 5; i++)
	{
		mlen	= 0;
		rvalue	= 0;
		memset(&login_msg, 0x00, MNG_PKT_HEAD_SIZE + MAX_MMCD_MSG_SIZE);

		printf("\nWelcome to %s Central System  ", SmsName);
		printf("\nLOGIN: ");
		scanf("%s", my_login);
		printf("PASSWORD: ");
		system("stty -echo");
		scanf("%s", my_passwd);
		system("stty echo");
		puts("");

		login_msg.head.llMagicNumber	= MAGIC_NUMBER;
		login_msg.head.llIndex			= gllTid++;

		login_msg.head.usResult			= DBM_SUCCESS;

		login_msg.head.usSrcProc		= 1;
		sprintf(login_msg.head.TimeStamp, "%s", mtime());
		sprintf(login_msg.data, "user-login %s,%s", my_login, my_passwd);

		login_msg.head.usBodyLen	= strlen(login_msg.data);
		login_msg.head.usTotLen		= strlen(login_msg.data) + MNG_PKT_HEAD_SIZE;

		strcpy(login_msg.head.userName, my_login);
		memcpy(&szSmsg[0], &login_msg, sizeof(st_MngPkt));
		if(dSendMessage(dSfd, login_msg.head.usTotLen, &szSmsg[0]) < 0)
			continue;
		else
		{
			for(j = 0; j < 5; j++)
			{
				memset(&cli_msg, 0x00, sizeof(cli_msg));
				if( (rvalue = dSockCheck()) == 0)
				{
					if(!strncmp(cli_msg.data, "SUCCESS", strlen("SUCCESS")))
						return 1;
					else
					{
						if(cli_msg.head.usBodyLen == 0)
						{
							dLogCnt++;

							if(dLogCnt > 2)
								return -1;
						}
						else
							break;
					}
				}
			}

			if(j == 5)
			{
				puts("\nCOMMUNICATION_TIMEOUT\n");
				continue;
			}
		}
	}/* end of while */

	return -1;
}

void logout_win(void)
{
	st_MngPkt	logout_msg;

	memset(&logout_msg, 0, sizeof (st_MngPkt));
	logout_msg.head.llMagicNumber	= MAGIC_NUMBER;
	logout_msg.head.llIndex			= gllTid++;
	logout_msg.head.usResult		= DBM_SUCCESS;
	logout_msg.head.usSrcProc		= 1;
	sprintf(logout_msg.head.TimeStamp ,"%s", mtime());
	sprintf(logout_msg.data, "user-logout %s", my_login);
	sprintf(logout_msg.head.userName, "%s", my_login);
	logout_msg.head.usBodyLen		= strlen(logout_msg.data);
	logout_msg.head.usTotLen		= strlen(logout_msg.data) + MNG_PKT_HEAD_SIZE;

	dSendMessage(dSfd, logout_msg.head.usTotLen, (char *)&logout_msg);
}


int send_login_sts (int msgtype, char *result)
{
    char    buf[3000];
    int     slen;

    if (msgtype == MI_LOGIN) {
        sprintf (buf, "\n%s %s ON MP", SmsName, mtime());
        slen = strlen(buf);
        if (!strcmp (result, "SUCCESS")) {
            sprintf (&buf[slen],
                "\nS1606  USER LOGIN\n  USER_NAME = %s  PORT = %d\n  RESULT = SUCCESS\nCOMPLETED\n\n",
                my_login, rmi_ID+1);
        }
        else {
            sprintf (&buf[slen], "\nS1608  USER LOGIN\n  \n  RESULT = FAIL\nCOMPLETED\n\n");
        }
    }
    else if( msgtype == MI_LOGOUT)
    {
        sprintf (buf, "\n%s %s ON MP", SmsName, mtime());
        slen = strlen(buf);
        sprintf (&buf[slen],
            "\nS1607  USER LOGOUT\n  USER_NAME = %s  PORT = %d\n  RESULT = SUCCESS\nCOMPLETED\n\n",
            my_login, rmi_ID+1);
    }
    else
    {
        sprintf(buf, "%s", result );
    }

    printf("%s\n", buf);

    return 1;
}

int get_smsname(void)
{
    sprintf (SmsName, "%s", "DQMS");

    return 1;
}
