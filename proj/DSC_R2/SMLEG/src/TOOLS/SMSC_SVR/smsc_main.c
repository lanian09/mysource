#include "smsc_proto.h"

char	mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN];
int		ixpcQid, ixpcPortNum;
char	trcBuf[4096], trcTmp[1024];



/*------------------------------------------------------------------------------*/
int main (int ac, char *av[])
{
	SockLibMsgType	rxSockMsg;
	int		ret, actFd;


	if (smsc_initial() < 0) {
		fprintf(stderr,">>>>>> ixpc_initial fail\n");
		return -1;
	}

	/* clear previous queue messages
	*/
	while (1)
	{
		/* remote 시스템들과 연결된 socket port들을 확인하여 메시지를 처리한다.
		*/
		ret = socklib_action ((char*)&rxSockMsg, &actFd);

		switch (ret)
		{
			case SOCKLIB_NEW_CONNECTION:
				smsc_newConnEvent (actFd); 					/* remote 가 접속해온 경우 */
				break;

			case SOCKLIB_CLIENT_MSG_RECEIVED:
				smsc_recvEventRxPort (actFd, &rxSockMsg); 	/* remote 로부터 data를 수신한 경우 */
				break;

			case SOCKLIB_SERVER_MSG_RECEIVED:
				smsc_recvEventTxPort (actFd); 				/* remote로 접속한 port로 data가 들어온 경우 */
				break;

			case SOCKLIB_CLIENT_DISCONNECTED:
				smsc_disconnEventRxPort (actFd); 			/* remote i가 접속해온 fd가 끊어진 경우 */
				break;

			case SOCKLIB_SERVER_DISCONNECTED:
				smsc_disconnEventTxPort (actFd); 			/* remote 로 접속한 fd가(송신port) 끊어진 경우 */
				break;

			case SOCKLIB_NO_EVENT:
				break;

			default:
				break;
		} /* end of socklib_action */

	} /* end of while(1) */

	return 1;

} /** End of main **/
