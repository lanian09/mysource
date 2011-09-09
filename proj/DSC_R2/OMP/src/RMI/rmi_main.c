#include "rmi_proto.h"
#include "ctype.h"

int		sockFd,hisBufIndex=0;
char	prompt[32];
char	hisBuf[RMI_NUM_HISTORY_BUFF][1024];
char	logFname[256];
FILE	*logFp=NULL;
pthread_mutex_t	rmi_mutex;
int		batchResCode, batchResFlag, batchend=0;
char    end;
char	passwd[32];

#if 1 /* jhnoh : 030815 */
int 	cliReqId = 0;
#endif

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main (int ac, char *av[])
{
	char	input[1024],*ptr;

	// mmcd로 connect한다.
	if (rmi_initial(ac,av) < 0)
		return -1;

	// login name과 password를 입력 받아 mmcd와 login 절차를 수행한다.
	rmi_login2mmcd ();

	// 주기적으로 interrupt에 의해 MMCD로부터의 수신 데이터를 확인한다.
	signal (SIGALRM, rmi_receiveResult);
	ualarm (50000, 0);

	while (1)
	{
		fprintf(stderr,"\n    [%s] ", prompt);

// log-out을 하지않고 alt-f4등으로 강제 종료로 인해 tty가 사라지면 blocking 되지 않고 무한루프로 빠짐.
// ttyname(0) 추가
		while (fgets(input, sizeof(input), stdin) == NULL) 
			if(ttyname(0) == NULL) return 0;	

		// 앞쪽 white-space를 지운다.
		ptr = input;
		for (; isspace(*ptr); ptr++);

		// 맨끝 \n을 지운다.
		input[strlen(input)-1] = 0;

		// 그냥 Enter를 입력한 경우
		if (!strlen(ptr)) {
			//fprintf(stderr,"\n[%s] ", prompt);
			continue;
		}

		// 자신이 직접 처리해야 하는 command인 경우
		if (rmi_isBuiltInCmd(ptr)) {
			//fprintf(stderr,"\n[%s] ", prompt);
			continue;
		}

		// mmcd로 명령어를 보낸다.
		//
#if 0 /* jhnoh : 030815 */
		rmi_send2mmcd(ptr, RMI_REQID_GENERAL_CMD, -1);
#else
		rmi_send2mmcd(ptr, -1, 0);
#endif
		fprintf(stderr,"\n    [%s] ", prompt);
	}

	return 0;

} //----- End of main -----//




//------------------------------------------------------------------------------
// mmcd로부터의 메시지를 수신하여 화면에 출력한다.
//------------------------------------------------------------------------------
void rmi_receiveResult (int sigId)
{
	int		ret,actFd;
	char choice[32];
	SockLibMsgType	rxSockMsg;
	MMLClientResMsgType	*rxCliResMsg;
	struct timeval t;
	fd_set fDS;
#define STDIN_FD 0
	int fD_POS;	

	t.tv_sec = 0;
	t.tv_usec = 200000;

	if (batchend==1){
		signal (SIGALRM, rmi_receiveResult);
		ualarm (50000, 0);
		return;
	}

	do {
		ret = socklib_action ((char*)&rxSockMsg, &actFd);
		switch (ret) {
			case SOCKLIB_SERVER_MSG_RECEIVED:
				rxCliResMsg = (MMLClientResMsgType*)rxSockMsg.body;
				
				// sjjeon : network order
				rxCliResMsg->head.cliReqId	= ntohl(rxCliResMsg->head.cliReqId);
				rxCliResMsg->head.confirm	= ntohl(rxCliResMsg->head.confirm);
				rxCliResMsg->head.batchFlag	= ntohl(rxCliResMsg->head.batchFlag);

#if 1 /* jhnoh : 030817 */
				if (rxCliResMsg->head.confirm == 1) {
					/*fprintf(stderr, "\n\n  Do you continue to execute this command ?\n  (%s) [Y/N] ? ", rxCliResMsg->body );

					choice = 0x00;
					while (!(choice=='y' || choice=='Y' || choice=='N' || choice=='n')) {
						FD_ZERO (&fDS);
						FD_SET (STDIN_FD, &fDS );

						ret = select (STDIN_FD+1, &fDS, 0, 0, &t);
						if (ret > 0) {
							if (FD_ISSET (STDIN_FD, &fDS) ) {
								gets(&choice);
								switch(choice) {
									case 'y':
 									case 'Y':
										rmi_send2mmcd(rxCliResMsg->body, 1, rxCliResMsg->head.batchFlag);
										break;
									case 'n':
									case 'N':
										rmi_send2mmcd(rxCliResMsg->body, 0, rxCliResMsg->head.batchFlag);
										break;
									default:
										fprintf(stderr, "\n\n  Do you continue to execute this command ?\n  (%s) [Y/N] ? ", rxCliResMsg->body );
										break;
								}
							}
						}
					}*/

					fprintf(stderr, "\n\n  Input The Password : ");
					
					system ("stty -echo");

					memset(choice, 0, 32);
					while (1) {
						FD_ZERO (&fDS);
						FD_SET (STDIN_FD, &fDS );

						ret = select (STDIN_FD+1, &fDS, 0, 0, &t);
						if (ret > 0) {
							if (FD_ISSET (STDIN_FD, &fDS) ) {
								gets (choice);
								system ("stty echo");
								if (!strcasecmp(choice, passwd)) {
									rmi_send2mmcd(rxCliResMsg->body, 1, rxCliResMsg->head.batchFlag);
									break;
								}
								else {
									rmi_send2mmcd(rxCliResMsg->body, 0, rxCliResMsg->head.batchFlag);
									break;
								}
							}
						}
					}

					
				}
#endif
				if (rxCliResMsg->head.confirm != 1) {
					fprintf(stderr,"%s",rxCliResMsg->body);

					// log file이 열려 있으면 기록한다.
					if (logFp != NULL)
						fprintf(logFp,"%s",rxCliResMsg->body);

					//rxCliResMsg->head.cliReqId = ntohl(rxCliResMsg->head.cliReqId);
					if (rxCliResMsg->head.batchFlag == 1) {
						if (rxCliResMsg->head.contFlag == 0) {
							batchResCode = rxCliResMsg->head.resCode;
							batchResFlag = 1;
						}
					}

                    /* exit (CPU OVERLOAD) && (CANC-USR) */
                    if(strstr(rxCliResMsg->body,"    BYE...") != NULL) 
                        rmi_terminate (1);

				}

				fprintf(stderr,"\n    [%s] ", prompt);
				break;

			case SOCKLIB_SERVER_DISCONNECTED:
				fprintf(stderr,"\n    >>> disconnected \n\n");
				exit(0);
				break;

			case SOCKLIB_NO_EVENT:
				break;

			default:
				break;
		}
	} while (ret == SOCKLIB_SERVER_MSG_RECEIVED) ;

	signal (SIGALRM, rmi_receiveResult);
	ualarm (50000, 0);

	return;

} //----- End of rmi_receiveResult -----//


#if 0 /* jhnoh : 030815 */
static int	cliReqId=1;
#endif

//------------------------------------------------------------------------------
// mmcd로 명령어를 보낸다.
// history buffer에 기록한다.
//------------------------------------------------------------------------------
#if 0 /* jhnoh : 030815 */
int rmi_send2mmcd (char *input, int cliReqId, char confirm)
#else
int rmi_send2mmcd (char *input, char confirm, int batchFlag)
#endif
{
	int		txLen, nConfirm;
	SockLibMsgType		txSockMsg;
	MMLClientReqMsgType	*txCliReqMsg;

	txCliReqMsg = (MMLClientReqMsgType*)txSockMsg.body;

	txCliReqMsg->head.cliReqId = htonl(cliReqId++);
//	txCliReqMsg->head.batchFlag = batchFlag; 
	txCliReqMsg->head.batchFlag = htonl(batchFlag);  // network order : sjjeon
	//txCliReqMsg->head.confirm = confirm;
	nConfirm =(int) confirm;
	txCliReqMsg->head.confirm = htonl(nConfirm);     // network order : sjjeon
	txCliReqMsg->head.clientType = 0; /* RMI */

#if 0 /*Debuf*/
printf("\n H2N cliReqId : %d\n", txCliReqMsg->head.cliReqId);
printf(" H2N batchFlag: %d\n", txCliReqMsg->head.batchFlag);
printf(" H2N confirm : %d\n", txCliReqMsg->head.confirm);
printf(" H2N clientType: %d\n", txCliReqMsg->head.clientType);
#endif

	strcpy (txCliReqMsg->body, input);

	txSockMsg.head.bodyLen = sizeof(txCliReqMsg->head) + strlen(input);
	txLen = sizeof(txSockMsg.head) + txSockMsg.head.bodyLen;

	//pthread_mutex_lock(&rmi_mutex);
//	if (socklib_sndMsg (sockFd, (char*)&txSockMsg, txLen) < 0) {
	if (socklib_sndMsg_hdr_chg (sockFd, (char*)&txSockMsg, txLen) < 0) {
		fprintf(stderr,"\n    >>> socklib_sndMsg fail \n\n");
		rmi_terminate (0);
	}

	//pthread_mutex_unlock(&rmi_mutex);

	if (!strncasecmp(input,"log-in",6))
		return 1;

	// history buffer에 기록한다.
	// 단 confirm이 아닌경우만
	if ( confirm < 0 ) rmi_saveInputHistory (input);

	batchend=0;

	return 1;

} //----- End of rmi_send2mmcd -----//
