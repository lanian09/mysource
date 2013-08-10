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
		/* remote �ý��۵�� ����� socket port���� Ȯ���Ͽ� �޽����� ó���Ѵ�.
		*/
		ret = socklib_action ((char*)&rxSockMsg, &actFd);

		switch (ret)
		{
			case SOCKLIB_NEW_CONNECTION:
				smsc_newConnEvent (actFd); 					/* remote �� �����ؿ� ��� */
				break;

			case SOCKLIB_CLIENT_MSG_RECEIVED:
				smsc_recvEventRxPort (actFd, &rxSockMsg); 	/* remote �κ��� data�� ������ ��� */
				break;

			case SOCKLIB_SERVER_MSG_RECEIVED:
				smsc_recvEventTxPort (actFd); 				/* remote�� ������ port�� data�� ���� ��� */
				break;

			case SOCKLIB_CLIENT_DISCONNECTED:
				smsc_disconnEventRxPort (actFd); 			/* remote i�� �����ؿ� fd�� ������ ��� */
				break;

			case SOCKLIB_SERVER_DISCONNECTED:
				smsc_disconnEventTxPort (actFd); 			/* remote �� ������ fd��(�۽�port) ������ ��� */
				break;

			case SOCKLIB_NO_EVENT:
				break;

			default:
				break;
		} /* end of socklib_action */

	} /* end of while(1) */

	return 1;

} /** End of main **/
