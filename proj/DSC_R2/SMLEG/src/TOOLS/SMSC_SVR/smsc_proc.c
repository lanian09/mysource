#include "smsc_proto.h"

extern int					ixpcPortNum;
extern char					trcBuf[4096], trcTmp[1024];
extern int					trcFlag, trcLogFlag;
extern ClientSockFdContext	clientSockFdTbl[SOCKLIB_MAX_CLIENT_CNT];


/*------------------------------------------------------------------------------*/
int smsc_newConnEvent (int fd)
{
	int		i,j;
	char	remoteAddr[32];

	/* socklib에서 관리하는 clientSockFdTbl을 검색한다.
	*/
	for (i=0; i<SOCKLIB_MAX_CLIENT_CNT; i++) {
		if (clientSockFdTbl[i].fd == fd) {
			break;
		}
	}
	if (i >= SOCKLIB_MAX_CLIENT_CNT) {
		sprintf(trcBuf,"[smsc_newConnEvent] not found fd[%d](rx_port) in clientSockFdTbl\n", fd);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	/* 접속해온 remote 시스템의 ip address를 얻는다.
	*/
	sprintf (trcBuf, "[smsc_newConnEvent] remode address = %s\n"
			, inet_ntoa(clientSockFdTbl[i].cliAddr.sin_addr));
	trclib_writeLogErr (FL,trcBuf);

	return 0;
}


int smsc_recvEventRxPort (int fd, SockLibMsgType *rxSockMsg)
{
	int result=0, ret=0;
	int msg_type = rxSockMsg->head.mtype;


	switch(msg_type)
	{
		case SMPP_BIND_MSG:
			{
			SMPP_BIND 		*recvMsg; 
			SMPP_BIND_ACK 	sendMsg;

			recvMsg = (SMPP_BIND *)rxSockMsg;

			fprintf(stderr, " @@@ RECV SMPP_BIND_MSG\n");
			//smsc_user_input(msg_type, &result);
			/* send BIND ACK MSG */
			result = 0;
			smsc_make_BIND_ACK(result, recvMsg,  &sendMsg);
			if ((ret = socklib_sndMsg(fd, (char*)&sendMsg, sizeof(SMPP_BIND_ACK))) < 0) {
				sprintf(trcBuf,"[smsc_recvEventRxPort] SMPP_BIND_ACK msg send faild(fd=%d)\n", fd);
				trclib_writeLogErr (FL,trcBuf);

			}else{
				fprintf(stderr, " @@@ SEND OK SMPP_BIND_ACK MSG\n");
			}

			}
			break;
		case SMPP_DELIVER_MSG:
			{
			SMPP_DELIVER 		*recvMsg; 
			SMPP_DELIVER_ACK 	sendMsg;
			SMPP_REPORT 		sendRptMsg; 

			recvMsg = (SMPP_DELIVER *)rxSockMsg;

			fprintf(stderr, " @@@ RECV SMPP_DELIVER_MSG\n");
			//smsc_user_input(msg_type, &result);
			result=0;
			/* send DELIVER ACK MSG */
			smsc_make_DELIVER_ACK(result, recvMsg,  &sendMsg);
			if ((ret = socklib_sndMsg(fd, (char*)&sendMsg, sizeof(SMPP_DELIVER_ACK))) < 0) {
				sprintf(trcBuf,"[smsc_recvEventRxPort] SMPP_DELIVER_ACK msg send faild(fd=%d)\n", fd);
				trclib_writeLogErr (FL,trcBuf);
			}
			fprintf(stderr, " @@@ SEND OK SMPP_DELIVER_ACK MSG\n");
			
			sleep(1);

			//smsc_user_input(msg_type, &result);
			result=0;
			/* send REPORT MSG */
			smsc_make_REPORT(result, recvMsg,  &sendRptMsg);
			if ((ret = socklib_sndMsg(fd, (char*)&sendRptMsg, sizeof(SMPP_REPORT))) < 0) {
				sprintf(trcBuf,"[smsc_recvEventRxPort] SMPP_REPORT msg send faild(fd=%d)\n", fd);
				trclib_writeLogErr (FL,trcBuf);
			}else{
				fprintf(stderr, " @@@ SEND OK SMPP_REPORT MSG\n");
			}
	
			}
			break;
		case SMPP_REPORT_MSG:
			break;
		case SMPP_REPORT_ACK_MSG:
			fprintf(stderr, " @@@ RECV SMPP_REPORT_ACK_MSG\n");
			sprintf(trcBuf,"[smsc_recvEventRxPort] SMPP_REPORT_ACK_MSG received(fd=%d)\n", fd);
			trclib_writeLogErr (FL,trcBuf);
			break;
		default:
			sprintf(trcBuf,"[smsc_recvEventRxPort] unknown msg type=%d\n"
					, rxSockMsg->head.mtype);
			trclib_writeLogErr (FL,trcBuf);
			break;
	}

	return 0;
}


int smsc_recvEventTxPort (int fd)
{
	sprintf(trcBuf,"[smsc_recvEventTxPort] unexpected msg received tx_port; unknown fd=%d\n", fd);
	trclib_writeLogErr (FL,trcBuf);

	return 0;
} 


int smsc_disconnEventRxPort (int fd)
{

	socklib_disconnectSockFd (fd); /* disconnect */
	sprintf(trcBuf,"[smsc_disconnEventRxPort] disconnected rx_port; unknown fd=%d\n", fd);
	trclib_writeLogErr (FL,trcBuf);

	return 0;
}


int smsc_disconnEventTxPort (int fd)
{
	socklib_disconnectSockFd (fd); /* disconnect rx_port also */
	sprintf(trcBuf,"[smsc_disconnEventTxPort] disconnected tx_port; unknown fd=%d\n", fd);
	trclib_writeLogErr (FL,trcBuf);

	return -1;
} 

int	smsc_make_BIND_ACK(int result, SMPP_BIND *recvMsg,  SMPP_BIND_ACK *sendMsg)
{
	int     i, slen;

	//////////////////////////
	// SMPP BODY
	//////////////////////////
	/* RESULT */
	sendMsg->result = result;
	/* PREFIX */
	strcpy(sendMsg->prefix, "019");

	//////////////////////////                                                                                        
	// SMPP HEADER                                                                                                    
	//////////////////////////                                                                                        
	sendMsg->header.type = SMPP_BIND_ACK_MSG;
	sendMsg->header.len = htonl(sizeof(SMPP_BIND_ACK)-sizeof(SMPP_MSG_H));

	return 0;
}

int	smsc_make_DELIVER_ACK(int result, SMPP_DELIVER *recvMsg,  SMPP_DELIVER_ACK *sendMsg)
{
	int     i, slen;

	//////////////////////////
	// SMPP BODY
	//////////////////////////
	/* RESULT */
	sendMsg->result = result;
	/* ORGADDR */
	strcpy(sendMsg->org_addr, recvMsg->dst_addr);
	/* DSTADDR */
	strcpy(sendMsg->dst_addr, recvMsg->org_addr);
	/* SERIAL NUMBER */
	//sendMsg->sn = htonl(recvMsg->sn);                                                                                  
	sendMsg->sn = recvMsg->sn;                                                                                  

	//////////////////////////                                                                                        
	// SMPP HEADER                                                                                                    
	//////////////////////////                                                                                        
	sendMsg->header.type = SMPP_DELIVER_ACK_MSG;
	sendMsg->header.len = htonl(sizeof(SMPP_DELIVER_ACK)-sizeof(SMPP_MSG_H));

	return 0;
}

int	smsc_make_REPORT(int result, SMPP_DELIVER *recvMsg,  SMPP_REPORT *sendMsg)
{
	int     i, slen;
	char 	tims[SMPP_MSG_DELIVER_TIME_LEN];
	time_t	cur_t;

	cur_t = time(NULL);
	//////////////////////////
	// SMPP BODY
	//////////////////////////
	/* RESULT */
	sendMsg->result = result;
	/* ORGADDR */
	strcpy(sendMsg->org_addr, recvMsg->dst_addr);
	/* DSTADDR */
	strcpy(sendMsg->dst_addr, recvMsg->org_addr);
	/* SERIAL NUMBER */
	//sendMsg->sn = htonl(recvMsg->sn);
	sendMsg->sn = recvMsg->sn;
	/* REPORT TIME */
	strftime(tims, sizeof(tims), "%Y-%m-%d %H:%M:%S", localtime((time_t *)&cur_t));
	strcpy(sendMsg->deliver_t, tims) ;
	strcpy(sendMsg->dest_code, "01990f");

	//////////////////////////
	// SMPP HEADER                                                                                                    
	//////////////////////////
	sendMsg->header.type = SMPP_REPORT_MSG;
	sendMsg->header.len = htonl(sizeof(SMPP_REPORT)-sizeof(SMPP_MSG_H));

	return 0;
}


void smsc_printf_menu(int msg_type)
{
	switch(msg_type)
	{
	case SMPP_BIND_MSG:
		fprintf(stderr,"\n   0. E_OK\n");
		fprintf(stderr,"\n   1. E_SYSFIAL\n");
		fprintf(stderr,"\n   2. E_AUTH_FAIL\n");
		fprintf(stderr,"\n   3. E_FORMAT_ERR\n");
		break;
	case SMPP_DELIVER_MSG:
		break;
	case SMPP_DELIVER_ACK_MSG:
		fprintf(stderr,"\n   0. E_OK\n");
		fprintf(stderr,"\n   1. E_SYSFIAL\n");
		fprintf(stderr,"\n   2. E_AUTH_FAIL\n");
		fprintf(stderr,"\n   3. E_FORMAT_ERR\n");
		fprintf(stderr,"\n   4. E_NOT_BOUND\n");
		fprintf(stderr,"\n   5. E_NO_DESTIN\n");
		fprintf(stderr,"\n   6. E_SENT\n");
		fprintf(stderr,"\n   7. E_EXPIRED\n");
		fprintf(stderr,"\n  11. E_NVALID_TERM\n");
		fprintf(stderr,"\n  12. E_OVERFLOW\n");
		break;
	case SMPP_REPORT_MSG:
		break;
	case SMPP_REPORT_ACK_MSG:
		fprintf(stderr,"\n   0. E_OK\n");
		fprintf(stderr,"\n   1. E_SYSFIAL\n");
		fprintf(stderr,"\n   4. E_NOT_BOUND\n");
		break;
	default:
		fprintf(stderr,"\n   #. UNKNOWN MSG TYPE\n");
		break;
	}
}

int smsc_user_input(int msg_type, int *result)
{
	/* USER INPUT */
	smsc_printf_menu(msg_type);

	fprintf(stderr,"\n   - choice result code :");
	scanf("%d", result);
	if (!smsc_check_input(msg_type, *result))
		exit(0);
		//smsc_user_input(msg_type, result);

	return 0;
}

#if 0
int smsc_check_user_input(result)
{
	if (result <0 || result >3) return -1;
	return 0;
}
#endif


int smsc_check_input(int type, int result)
{
	switch(type)
	{
	case SMPP_BIND_MSG:
		if((result < 0 ) || (result>3)) break;
		return 1;
	case SMPP_BIND_ACK_MSG:
		break;
	case SMPP_DELIVER_MSG:
		if((result < 0 ) || (result>12)) break;
		return 1;
	case SMPP_DELIVER_ACK_MSG:
	case SMPP_REPORT_MSG:
	case SMPP_REPORT_ACK_MSG:
	default:
		break;
	}
	return 0;
}

char *smsc_get_msgtype(int type)
{
	switch(type)
	{
	case SMPP_BIND_MSG:
		return "BIND";
	case SMPP_DELIVER_MSG:
		return "DELIVER";
	case SMPP_DELIVER_ACK_MSG:
		return "DELIVER_ACK";
	case SMPP_REPORT_MSG:
		return "REPORT";
	case SMPP_REPORT_ACK_MSG:
		return "REPORT_ACK";
	default:
		return "UNKOWN TYPE";
	}
}
