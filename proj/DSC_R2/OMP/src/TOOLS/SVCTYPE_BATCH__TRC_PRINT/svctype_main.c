#include <svctype.h>

static char	sysname[10];
static int	syscount;

int main(int argc, char *argv[])
{
	int	portNum;
	char	peeripaddr[20], argport[10], batchfile[256];
	int	optflg, errflg = 0, hostflg = 0, userflg = 0, passflg = 0, sysflg = 0;
	int	portflg = 0;

	if(argc < 2){
		usage();
		return 1;
	}

	while((optflg = getopt(argc, argv, "r:n:s:")) != EOF)
		
	switch(optflg){
		case 'r':
			memset(peeripaddr, 0x00, sizeof(peeripaddr));
			strncpy(peeripaddr, optarg, sizeof(peeripaddr)-1);
			hostflg = 1;
			break;
		case 'n':
			memset(argport, 0x00, sizeof(argport));
			strncpy(argport, optarg, sizeof(argport)-1);
			portflg = 1;
			break;
		case 's':
			memset(sysname, 0x00, sizeof(sysname));
			strncpy(sysname, optarg, sizeof(sysname)-1);
			sysflg = 1;
			break;
		case '?':
			errflg++;
			break;
	}

	if(!hostflg){
		strcpy(peeripaddr, LOOPBACK_IP);
	}

	if(portflg)
		portNum = atoi(argport);
	else
		portNum = get_cond_port_number();

	if(portNum < 0)
		return 1;
	
	if(optind == argc){
		usage();
		return 1;
	}else
		strcpy(batchfile, argv[optind]);

	if(sysflg){
		if(!strcmp(sysname, ALL))
			syscount = 1;
		else if(!strcmp(sysname, ALL))
			syscount = 1;
		else if(!strcmp(sysname, ALL))
			syscount = 2;
		else{
			usage();
			return 1;
		}
	}else{
		strcpy(sysname, ALL);
		syscount = 2;
	}

	printf("##############################################################\n");
	printf("#                                                            #\n");
	printf("#                  SERVICE TYPE BATCH PROGRAM                #\n");
	printf("#                                                            #\n");
	printf("##############################################################\n");
	fflush(stdout);

	batch_main(peeripaddr, portNum, batchfile);

	return 0;
}

void usage(void)
{
	printf("SVCTYPE_BATCH [-r IP Address] [-n port number] -s [ALL|BSDA|BSDB] batchfile\n");
	printf("          -r\tIP Address of the server on that BSDM is running.\n");
	printf("          -n\tPort Number of the server on that BSDM is running.\n");
	printf("          -s\tSystem name, one of ALL, BSDA or BSDB.\n");
}

int get_cond_port_number(void)
{
	char	*ivhome;
	char	sysfile[128], tmp[128];

	if((ivhome = getenv(IV_HOME)) == NULL){
		fprintf(stderr, "Can't get %s from environment!\n", IV_HOME);
		return -1;
	}

	sprintf(sysfile, "%s/%s", ivhome, SYSCONF_FILE);

	if (conflib_getNthTokenInFileSection (sysfile, "SOCKET_PORT", "MMCD", 1, tmp) < 0){
		fprintf(stderr, "Can't get MMCD port from %s!\n", sysfile);
		return -1;
	}

	return strtol(tmp, 0, 0);
}

int batch_main(char *hostIp, int portNum, char *batchfile)
{

	char	answer;
	int peersock, validUploadCmd, getConfCnt, retry_q = 0;
	REC_SVCTYPE *fileSvcType[MAX_COMMAND_NUM], *cmdSvcType[MAX_COMMAND_NUM];

	if(!file_exist(batchfile))
		return 1;

	peersock = socklib_connect(hostIp, portNum);
	if(peersock < 0)
		return 1;

	if(login2mmcd(peersock))
		return 1;

	validUploadCmd = load_batch_file(batchfile, fileSvcType, MAX_COMMAND_NUM);
	if(!validUploadCmd){
		fprintf(stderr, "NO VALID COMMAND IN %s\n", batchfile);
		return 1;
	}

	fprintf(stdout, "\n\n(QUESTION)Do you want to continue to batch job?");
RETRY_Q:
	fprintf(stdout, "\n          Enter \'Y|y\' to continue, otherwise \'N|n\'(%d/3):", ++retry_q);
	answer = fgetc(stdin);
	if(answer == 'N' || answer == 'n')
		return 1;

	if(answer != 'Y' && answer != 'y'){
		if(retry_q >=3 )
			return 1;
		goto RETRY_Q;
	}


#ifdef DEBUG
	printf("====================================  READ FROM FILE =====================================\n");
	print_struct_all(fileSvcType, validUploadCmd);
#endif

	getConfCnt = get_conf_from_mmcr(peersock, cmdSvcType, MAX_COMMAND_NUM);
	if(getConfCnt <= 0){
		return 1;
	}
#ifdef DEBUG
	printf("====================================  READ FROM MMCR =====================================\n");
	print_struct_all(cmdSvcType, getConfCnt);
#endif

	processing_mmc(peersock, fileSvcType, validUploadCmd, cmdSvcType, getConfCnt);

	close(peersock);

	return 0;
}

int load_batch_file(char *batchFile, REC_SVCTYPE *svcType[], int psize)
{
	int		totalCount = 0, validCount = 0, errCount = 0, i;
	int		addCount = 0, delCount = 0, chgCount = 0;
	FILE 		*fp;
	char		tmpbuf[256], buff[256], tmp[50];
	char		*errline[50] = {NULL};
	int 		parseErr, errcause[50] = {-1};
	char		*token, *lasts;
	REC_SVCTYPE	recbuff;

	fp = fopen(batchFile, "r");
	if(!fp){
		fprintf(stderr, "[ERROR]fopen - %s\n", strerror(errno));
		return -1;
	}

	while(fgets(buff, sizeof(buff), fp) && validCount < psize){

		totalCount++;
		removeCRLFofLine(buff);

		strcpy(tmpbuf, buff);
		token = strtok_r(tmpbuf, FILE_COLUMN_TOKEN, &lasts);
		if(!token){
			errline[errCount] = strdup(buff);
			errcause[errCount++] = FPERR_WRONG_DELIMITER;
			continue;
		}

		i = 0;
		parseErr = 0;
		memset(&recbuff, 0x00, sizeof(recbuff));
		while(token && i < FILE_COLUMNS_NUM){

			memset(tmp, 0x00, sizeof(tmp));
			strtrim(token, tmp);
			switch(i){
				case 0:		// action
					if(!strcasecmp(tmp, ADD)){
						recbuff.action = ADD_ACTION;
						addCount++;
					}else if(!strcasecmp(tmp, DEL)){
						recbuff.action = DEL_ACTION;
						delCount++;
					}else if(!strcasecmp(tmp, CHG)){
						recbuff.action = CHG_ACTION;
						chgCount++;
					}else{
						errline[errCount] = strdup(buff);
						errcause[errCount++] = FPERR_UNDEF_COMMANDTYPE;
						parseErr = 1;
					}
					break;
				case 1:		// svctype id
					strcpy(recbuff.svctype, tmp);
					break;
				case 2:		// layer
					if(!strcasecmp(tmp, TCP))
						recbuff.key.layer = TCP_LAYER;
					else if(!strcasecmp(tmp, UDP))
						recbuff.key.layer = UDP_LAYER;
					else if(!strcasecmp(tmp, IP))
						recbuff.key.layer = IP_LAYER;
					else{
						errline[errCount] = strdup(buff);
						errcause[errCount++] = FPERR_UNKNOWN_LAYER;
						parseErr = 1;
					}
					break;
				case 3:		// fout
					strcpy(recbuff.fout, tmp);
					break;
				case 4:		// ip
					if(inet_pton(AF_INET, tmp, &recbuff.key.ipaddr) != 1){
						errline[errCount] = strdup(buff);
						errcause[errCount++] = FPERR_WRONG_IPV4ADDR;
						parseErr = 1;
					}
					break;
				case 5:		// netmask
					recbuff.key.netmask = atoi(tmp);
					if(recbuff.key.netmask > 32){
						errline[errCount] = strdup(buff);
						errcause[errCount++] = FPERR_INVALID_NETMASK;
						parseErr = 1;
					}
					break;
				case 6:		// port
					recbuff.key.port = atoi(tmp);
					break;
				case 7:		// block
					strcpy(recbuff.block, tmp);
					break;
				case 8:		// url_cha
					strcpy(recbuff.url_cha, tmp);
					break;
				case 9:	// first_udr
					strcpy(recbuff.first_udr,tmp);
					break;

				case 10: // alias
					strcpy(recbuff.alias, tmp);
					break;
			}

			if(parseErr) // exit while
				break;
			i++;
			token = strtok_r(NULL, FILE_COLUMN_TOKEN, &lasts);
		}

		if(!parseErr){
			svcType[validCount] = (REC_SVCTYPE *)calloc(sizeof(REC_SVCTYPE), 1);
			memcpy(svcType[validCount], &recbuff, sizeof(REC_SVCTYPE));
			validCount++;
		}
		
	}

	fclose(fp);

	printf("\n\n===================== FILE BATCH COMMAND ====================\n");
	printf(">>>> TOTAL  COMMAND COUNT : %d \n", totalCount);
	printf(">>>> ADD    COMMAND COUNT : %d \n", addCount);
	printf(">>>> DELETE COMMAND COUNT : %d \n", delCount);
	printf(">>>> CHANGE COMMAND COUNT : %d \n", chgCount);
	printf(">>>> WRONG  COMMAND COUNT : %d \n", errCount);

	for(i = 0; errline[i] != NULL; i++){
		printf("\nERROR COMMAND    : \"%s\"\n", errline[i]);
		printf("\n      ERROR DESC : %s\n", ERRTABLE[errcause[i]]);
		free(errline[i]);
	}

	return validCount;
}

int get_conf_from_mmcr(int sockfd, REC_SVCTYPE *svcType[], int psize)
{

	int     ret, rooping = 1, curpos = 0;
	SockLibMsgType  rxSockMsg;
	MMLClientResMsgType *rxCliResMsg;

	if(reqConf(sockfd))
		return -1;

	while(rooping){
		ret = socklib_action ((char*)&rxSockMsg, &sockfd);
		switch (ret) {
			case SOCKLIB_SERVER_MSG_RECEIVED:
				rxCliResMsg = (MMLClientResMsgType*)rxSockMsg.body;
				if (rxCliResMsg->head.resCode == 0) { // success
					rooping = rxCliResMsg->head.contFlag;
#ifdef DEBUG
					printf("SUCCESS\n");
					printf("%s", rxCliResMsg->body);
#endif
					curpos = conf_msg_parese(rxCliResMsg->body, svcType, curpos, psize);
				}else{
					fprintf(stderr, "While retrieving the SERVICE TYPEs from BSD's MMCR,\n");
					fprintf(stderr, "Error has been occured.\n");
					fprintf(stderr, "Please, Check status of MCDM/IXPC/MMCR\n");
					return -1;
#ifdef DEBUG
					printf("FAILED\n");
					printf("%s", rxCliResMsg->body);
#endif
				}
				break;
			case SOCKLIB_SERVER_DISCONNECTED:
				fprintf(stdout,"\n    >>> disconnected \n\n");
				return -1;
				break;
		}
	}

	return curpos;

}

int  conf_msg_parese(char *msgbody, REC_SVCTYPE *svcType[], int startpos, int maxsize)
{
	int	nd = 0;
	char 	*token, *lasts, *p;
	char	tmpdup[4096], values[12][50];
	REC_SVCTYPE		*pStr;

	strcpy(tmpdup, msgbody);

	token = strtok_r(tmpdup, "\n", &lasts);

	while( nd < 2){
		p = strstr(token, "========");
		if(p) nd++;
		token = strtok_r(NULL, "\n", &lasts);
	}

	p = strstr(token, "--------");
	while(p == NULL && startpos < maxsize){
		sscanf(token, "%s%s%s%s%s%s%s%s%s%s%s", values[0], values[1], values[2],
			   values[3], values[4], values[5], values[6], values[7], values[8],
			   values[9], values[10]);

		pStr = (REC_SVCTYPE *)calloc(sizeof(REC_SVCTYPE), 1);
		fill_struct(pStr, values);
		svcType[startpos++] = pStr;
		token = strtok_r(NULL, "\n", &lasts);
		p = strstr(token, "--------");
	}

	return startpos;
}


void fill_struct(REC_SVCTYPE *svcType, char values[][50])
{
	svcType->action = '0';

	strcpy(svcType->svctype, values[0]);			// service type id

	if(!strcasecmp(values[1], TCP))				// layer
		svcType->key.layer = TCP_LAYER;
	else if(!strcasecmp(values[1], UDP))
		svcType->key.layer = UDP_LAYER;
	else if(!strcasecmp(values[1], IP))
		svcType->key.layer = IP_LAYER;

	strcpy(svcType->fout, values[2]);			// fout

	strcpy(svcType->recordid, values[3]);			// record id

	inet_pton(AF_INET, values[4], &svcType->key.ipaddr);	// IP Address

	svcType->key.netmask = atoi(values[5]);			// netmask

	svcType->key.port = atoi(values[6]);			// port
	strcpy(svcType->block, values[7]);
	strcpy(svcType->url_cha, values[8]);			// url_cha
	strcpy(svcType->first_udr, values[9]);			// first_udr	
	strcpy(svcType->alias, values[10]);			// alias

}

void print_struct(REC_SVCTYPE *svcType)
{
	int	i;
	struct in_addr ipaddr;

	ipaddr.s_addr = svcType->key.ipaddr;
	printf("%s,%c,%s,%s,%s,%d,%d,%s,%s,%s,%s,%s", svcType->svctype, svcType->key.layer,
		   svcType->fout, svcType->recordid, inet_ntoa(ipaddr), svcType->key.netmask,
		   svcType->key.port, svcType->block, svcType->url_cha, svcType->first_udr,
		   svcType->alias, get_action_name(svcType->action));
}

void print_struct_all(REC_SVCTYPE *svcType[], int maxsize)
{
	int	i;
//	printf("================================= SVC TYPE =================================\n");
	for(i = 0; i < maxsize && svcType[i] != NULL; i++){
		print_struct(svcType[i]);
		printf("\n");
	}
}

char *get_layer_name(char layer)
{
	static char strlayer[10];

	if(layer == TCP_LAYER)
		strcpy(strlayer, TCP);
	if(layer == UDP_LAYER)
		strcpy(strlayer, UDP);
	if(layer == IP_LAYER)
		strcpy(strlayer, IP);

	return strlayer;
}

char *get_str_ipaddr(int nipaddr)
{
	struct in_addr addr;

	addr.s_addr = nipaddr;

	return inet_ntoa(addr);
}

char *get_action_name(char action)
{
	static char	act_name[10];
	if(action == ADD_ACTION)
		strcpy(act_name, "ADD");
	if(action == DEL_ACTION)
		strcpy(act_name, "DEL");
	if(action == CHG_ACTION)
		strcpy(act_name, "CHG");

	return act_name;
}



void processing_mmc(int sockfd, REC_SVCTYPE *cmdSvcType[], int size1, REC_SVCTYPE *confSvcType[], int size2)
{
	int		i,j,found, exeflag;
	char	cmd[512];
	MMC_PROC_RES	mmc_res[2];

	printf("\n\n==================== BATCH COMMAND RESULT ===================\n");
	for(i = 0; i < size1; i++){
		memset(mmc_res, 0x00, sizeof(mmc_res));

		printf("[%3d][%s](", i+1, get_action_name(cmdSvcType[i]->action));
		print_struct(cmdSvcType[i]);
		printf(")\n");
		printf("\tRESULT : ");

		found = search_svctype(cmdSvcType[i], confSvcType, size2);

		exeflag = 0;
		if(cmdSvcType[i]->action == ADD_ACTION){
			if(found >= 0){
				fprintf(stdout, "ERROR - Already same row exist!!\n");
			}else{
				sprintf(cmd, CMD_STRING_ADD, sysname, cmdSvcType[i]->svctype, get_layer_name(cmdSvcType[i]->key.layer),
				cmdSvcType[i]->fout, get_str_ipaddr(cmdSvcType[i]->key.ipaddr), cmdSvcType[i]->key.netmask,
				cmdSvcType[i]->key.port, cmdSvcType[i]->block, cmdSvcType[i]->url_cha, cmdSvcType[i]->first_udr,
				cmdSvcType[i]->alias);
				exeflag = 1;
			}
		}else if(cmdSvcType[i]->action == DEL_ACTION){
			if(found < 0){
				fprintf(stdout, "ERROR - Not found Record ID\n");
			}else{
				sprintf(cmd, CMD_STRING_DEL, sysname, confSvcType[found]->recordid);
				exeflag = 1;
			}
		}else if(cmdSvcType[i]->action == CHG_ACTION){
			if(found < 0){
				fprintf(stdout, "ERROR - Not found Record ID\n");
			}else{
				sprintf(cmd, CMD_STRING_CHG, sysname, confSvcType[found]->recordid,
			 	get_layer_name(cmdSvcType[i]->key.layer), cmdSvcType[i]->fout,
				get_str_ipaddr(cmdSvcType[i]->key.ipaddr), cmdSvcType[i]->key.netmask,
				cmdSvcType[i]->key.port, cmdSvcType[i]->block, cmdSvcType[i]->url_cha,
				cmdSvcType[i]->first_udr, cmdSvcType[i]->alias);
				exeflag = 1;
			}
		}

		if(!exeflag) continue;

		if(execute_mmc(sockfd, cmd, syscount, mmc_res) < 0){
			return ;
		}else{
			for(j = 0; j < syscount; j++){
				if(mmc_res[j].SFflag == FAIL){
					fprintf(stdout, " %s FAIL REASON-%s", mmc_res[j].sysname, mmc_res[j].FailReason);
				}else if(mmc_res[j].SFflag == SUCCESS){
					fprintf(stdout, " %s SUCCESS", mmc_res[j].sysname);
				}
				if(j < (syscount-1))
					fprintf(stdout, ",");
			}
			fprintf(stdout, "\n");
		}
	}
}

int search_svctype(REC_SVCTYPE *svcType, REC_SVCTYPE *svcTypeTB[], int tbsize)
{
	int		i, found = -1;

	for(i = 0; i < tbsize; i++){
		if(!memcmp(&svcType->key, &svcTypeTB[i]->key, sizeof(REC_SVCTYPE_KEY))){
			found = i;
			break;
		}
	}

	return found;
}
