#include <svctype.h>

char	username[50], password[50];
int	cliReqId = 0;

int login2mmcd (int sockfd)
{
    	char    msgBuf[1024];
    	int     ret;
    	char	msgbody[256];
    	SockLibMsgType  rxSockMsg;
    	MMLClientResMsgType *rxCliResMsg;

	fprintf(stdout, "\n\nPlease Enter the LOGIN ID & PASSWORD for service type batch job!!\n");
login_prompt:
    
	fprintf(stdout,"\n\tLOGIN ID : ");
    	fgets (username, sizeof(username), stdin);
    	username[strlen(username)-1] = 0;
    	fprintf(stdout, "\n\tPASSWORD : ");
    	system ("stty -echo");
    	fgets (password, sizeof(password), stdin);
    	system ("stty echo");
    	password[strlen(password)-1] = 0;

    	sprintf(msgbody,"log-in %s,%s",username, password);

    	if(sendmsg2mmcd (sockfd, msgbody, -1, 0) < 0)
		return 1;

    	while (1) {
        	ret = socklib_action ((char*)&rxSockMsg, &sockfd);
        	switch (ret) {
            
			case SOCKLIB_SERVER_MSG_RECEIVED:
                		rxCliResMsg = (MMLClientResMsgType*)rxSockMsg.body;
                
				if (rxCliResMsg->head.resCode == 0) { // success
                    			fprintf(stdout,"\n");
                   			 return 0;
                		}
                		sleep(1);
                		fprintf(stdout,"\n    >>> login incorrect <<<\n");
                		fprintf(stdout,"\n%s", rxCliResMsg->body);
                		goto login_prompt;

            		case SOCKLIB_SERVER_DISCONNECTED:
                		fprintf(stdout,"\n    >>> disconnected \n\n");
				return 1;
                		break;
        	}
    	}

	fprintf(stdout, "Login Success!!\n");

    	return 0;

}

int reqConf(int sockfd)
{
	int		ret = 0;
    	char	msgbody[256];
	SockLibMsgType  rxSockMsg;
	MMLClientResMsgType *rxCliResMsg;

	sprintf(msgbody, CMD_STRING_DIS, BSDA);
    	if(sendmsg2mmcd (sockfd, msgbody, -1, 0) < 0)
		return 1;

	ret = socklib_action ((char*)&rxSockMsg, &sockfd);
	switch (ret) {
		case SOCKLIB_SERVER_MSG_RECEIVED:
			rxCliResMsg = (MMLClientResMsgType*)rxSockMsg.body;
			if (rxCliResMsg->head.resCode == 0) { // success
				ret = 0;
			}else{
				printf("[%s]", rxCliResMsg->body);
				ret = 1;
			}
			break;
		case SOCKLIB_SERVER_DISCONNECTED:
			fprintf(stdout,"\nMMCD disconnected \n\n");
			ret = 1;
			break;
	}


	return ret;
}

int execute_mmc(int sockfd, char *cmd, int sysCount, MMC_PROC_RES *mmcRes)
{
	int		i, ret;
    	char	msgbody[256], *sys, *rsn, *res;
	SockLibMsgType  rxSockMsg;
	MMLClientResMsgType *rxCliResMsg;

	sprintf(msgbody, cmd);
    	if(sendmsg2mmcd (sockfd, msgbody, 1, 0) < 0)
		return -1;

	//1. MMC ACCEPTION RESPONSE from MMCD
	ret = socklib_action ((char*)&rxSockMsg, &sockfd);
	switch (ret) {
		case SOCKLIB_SERVER_MSG_RECEIVED:
			rxCliResMsg = (MMLClientResMsgType*)rxSockMsg.body;
			if (rxCliResMsg->head.resCode != 0) {	// fail
				fprintf(stdout, "%s", rxCliResMsg->body);
				return -1;
			}
			break;
		case SOCKLIB_SERVER_DISCONNECTED:
			fprintf(stdout,"\nMMCD disconnected \n\n");
			return -1;
	}

	//2. MMC RESULT RESPONSE from MMCD(error), MMCR(BSDA, BDSB)
	i = 0;
	while(i < sysCount){
		ret = socklib_action ((char*)&rxSockMsg, &sockfd);
		switch (ret) {
			case SOCKLIB_SERVER_MSG_RECEIVED:

				rxCliResMsg = (MMLClientResMsgType*)rxSockMsg.body;
				sys = strstr(rxCliResMsg->body, "SYSTEM =");
				if(!sys){			// MMCD return Error!
					fprintf(stdout, "NOT FOUND SYSTEM NAME");
					return -1;
				}
				sys = strstr(sys, "BSD");

				strncpy(mmcRes[i].sysname, sys, SYSNAME_SIZE);
				if (rxCliResMsg->head.resCode == 0) {	// success
					mmcRes[i].SFflag = SUCCESS;
				}else{									// FAIL
					mmcRes[i].SFflag = FAIL;
					rsn = strstr(rxCliResMsg->body, REASON);
					copyFeq2CR(mmcRes[i].FailReason, rsn);
				}
				i++;
				break;
			case SOCKLIB_NO_EVENT:
				commlib_microSleep(100000);
				break;
			case SOCKLIB_SERVER_DISCONNECTED:
				fprintf(stdout,"\nMMCD disconnected \n\n");
				return -1;
		}

	} // end of while

	return 0;

}

int sendmsg2mmcd (int sockfd, char *msgbody, char confirm, int batchFlag)
{
    	int     txLen;
    	SockLibMsgType      txSockMsg;
    	MMLClientReqMsgType *txCliReqMsg;

    	txCliReqMsg = (MMLClientReqMsgType*)txSockMsg.body;

    	txCliReqMsg->head.cliReqId = htonl(cliReqId++);
    	txCliReqMsg->head.batchFlag = batchFlag;

    	txCliReqMsg->head.confirm = confirm;

   	txCliReqMsg->head.clientType = 0; /* RMI */

    	strcpy (txCliReqMsg->body, msgbody);

    	txSockMsg.head.bodyLen = sizeof(txCliReqMsg->head) + strlen(msgbody);
    	txLen = sizeof(txSockMsg.head) + txSockMsg.head.bodyLen;

    	if (socklib_sndMsg (sockfd, (char*)&txSockMsg, txLen) < 0) {
        	fprintf(stderr,"\n    >>> socklib_sndMsg fail \n\n");
        	return 1;
    	}

    	return 0;
}

int file_exist(char *file)
{
	struct stat     st;

	if(stat(file, &st) < 0){
		fprintf(stderr, "[ERROR] %s file does not exist!\n", file);
		return 0;
	}

	if(S_IFREG & st.st_mode)
		return 1;
	else{
		fprintf(stderr, "[ERROR] %s is not a regular file!\n", file);
		return 0;
	}
}


char *current_time(void)
{
	static char		sNow[50];
	struct tm		tmNow;
	time_t			tNow = time(NULL);

	localtime_r(&tNow, &tmNow);
	sprintf(sNow, "%4d-%02d-%02d %02d:%02d:%02d", tmNow.tm_year+1900,
			tmNow.tm_mon+1, tmNow.tm_mday, tmNow.tm_hour, tmNow.tm_min,
			tmNow.tm_sec);

	return sNow;
}

void strtrim(const char *s1, char *s2)
{
	char *p = s1;
	while(*p != 0x00){
		if(*p != ' ' && *p != '\t'){
			*s2 = *p;
			s2++;
		}
		p++;
	}
}

void copyFeq2CR(char *s1, char *s2)
{
	char *p ;

	p = strchr(s2, '=');
	p++;

	while(isspace(*p)) p++;

	while(*p != '\n'){
		*s1 = *p;
		s1++;
		p++;
	}
}

void removeCRLFofLine(char *s1)
{
	while(*s1 != 0x00){
		if(*s1 == '\r' || *s1 == '\n')
			*s1 = 0x00;
		s1++;
	}
}
