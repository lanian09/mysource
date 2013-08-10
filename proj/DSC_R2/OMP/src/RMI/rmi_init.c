#include "rmi_proto.h"
#include <sys/ipc.h>

extern int	sockFd;
extern char	prompt[32];
int  omdsysQid;
extern char passwd[32];

int rmi_initial (int ac, char *av[])
{
	int		port=0;
	char	*env,fname[256],tmp[32],token[8][CONFLIB_MAX_TOKEN_LEN];
	char	key, ipAddr[32], mySysName[32];

	if ((env = getenv(MY_SYS_NAME)) == NULL) {
		fprintf(stderr,"not found %s environment name\n", MY_SYS_NAME);
		return -1;
	}
	strcpy (mySysName, env);

	if ((env = getenv(IV_HOME)) == NULL) {
		printf("not found %s environment name\n", IV_HOME);
		return -1;
	}
	sprintf(fname,"%s/%s", env, SYSCONF_FILE);

	// argument로 서버(mmcd)의 주소와 port가 들어 왔으면 받아 setting한다.
	rmi_getArgs (ac, av, ipAddr, &port);

	// ipAddr를 지정하지 않았으면 config 파일에서 OMP의 주소를 읽는다.
	if (!strcasecmp(ipAddr, "")) {
		if (conflib_getNthTokenInFileSection (fname, "ASSOCIATE_SYSTEMS", "DSCM", 3, ipAddr) < 0)
			return -1;
	}

	// port를 지정하지 않았으면 config 파일에서 mmcd의 port를 읽는다.
	if (!port) {
		if (conflib_getNthTokenInFileSection (fname, "SOCKET_PORT", "MMCD", 1, tmp) < 0)
			return -1;
		port = strtol(tmp,0,0);
	}

	if (conflib_getNthTokenInFileSection (fname, "GENERAL", "SYSTEM_LABEL", 1, prompt) < 0)
		return -1;

	// mmcd에 connect한다.
	if ((sockFd = socklib_connect (ipAddr, port)) < 0) {
		printf("    >>> can't connect [%s:%d]....\n\n", ipAddr, port);
		return -1;
	}

	fprintf(stderr,"\n\n    CONNECT SUCCESS [IP=%s  PORT=%d]\n\n", ipAddr, port);

	return 1;

} //----- End of rmi_init -----//


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int rmi_getArgs (int ac, char *av[], char *ipAddr, int *port)
{
	int		a;
	char	*usage =
"  rmi [-a address] [-p port] \n\
        -a : destination ip_address \n\
        -p : port number \n\
        -h : display this message\n\n";

	strcpy (ipAddr,"");
	*port = 0;

	while ((a = getopt(ac,av,"a:p:h")) != EOF)
	{
		switch (a) {
			case 'a': // 접속할 서버(mmcd)의 ip_address
				strcpy (ipAddr, optarg);
				break;
			case 'p': // 접속할 서버의 port_number
				*port = strtol (optarg,0,0);
				break;
			case 'h': // help
				printf("%s",usage);
				exit(0);
		}
	}

	return 1;

} //----- End of rmi_getArgs -----//



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int rmi_login2mmcd (void)
{
	time_t	now;
	char    temp[20];
	char    msgBuf[1024];

	int		ret,actFd;
	char	userName[32], input[256];
	SockLibMsgType	rxSockMsg;
	MMLClientResMsgType	*rxCliResMsg;

	now = time(NULL);
	strftime(temp, 100, "%Y-%m-%d %H:%M:%S", localtime(&now));
	sprintf(msgBuf,"\n\n    Welcome to DSC [%s]\n", temp);
login_prompt:
	fprintf(stderr,"\n    Login  : ");
	fgets (userName, sizeof(userName), stdin);
	userName[strlen(userName)-1] = 0;
	fprintf(stderr,"    Passwd : ");
	system ("stty -echo");
	fgets (passwd, sizeof(passwd), stdin);
	system ("stty echo");
	passwd[strlen(passwd)-1] = 0;

	sprintf(input,"log-in %s,%s",userName,passwd);

#if 0	/* jhnoh : 030815 */	
	rmi_send2mmcd (input, RMI_REQID_LOGIN, -1);
#else
	rmi_send2mmcd (input, -1, 0);
#endif

	while (1) {
		ret = socklib_action ((char*)&rxSockMsg, &actFd);
		switch (ret) {
			case SOCKLIB_SERVER_MSG_RECEIVED:
				rxCliResMsg = (MMLClientResMsgType*)rxSockMsg.body;
				if (rxCliResMsg->head.resCode == 0) { // success
					fprintf(stderr,"\n");
					return 1;
				}
				sleep(1);
				fprintf(stderr,"\n    >>> login incorrect <<<\n");
				fprintf(stderr,"\n%s", rxCliResMsg->body);
				goto login_prompt;

			case SOCKLIB_SERVER_DISCONNECTED:
				fprintf(stderr,"\n    >>> disconnected \n\n");
				rmi_terminate (0);
				break;
		}
	}

	return 1;

} //----- End of rmi_login2mmcd -----//
