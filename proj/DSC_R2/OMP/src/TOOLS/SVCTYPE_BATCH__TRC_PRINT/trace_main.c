#include <trace_proto.h>

int main(int argc, char *argv[])
{
	int		portNum;
	char	peerIpAddr[20], argPort[10], imsi[16], outpath[128];
	int		optflg, errflg = 0, imsiflg = 0, hostflg = 0, outflg = 0, dirflg = 0;
	int		portflg = 0;

	while((optflg = getopt(argc, argv, "wm:d:r:p:")) != EOF)
		switch(optflg){
			case 'w':
				outflg = 1;
				break;
			case 'm':
				memset(imsi, 0x00, sizeof(imsi));
				strncpy(imsi, optarg, sizeof(imsi)-1);
				imsiflg = 1;
				break;
			case 'd':
				memset(outpath, 0x00, sizeof(outpath));
				strncpy(outpath, optarg, sizeof(outpath)-1);
				dirflg = 1;
				break;
			case 'r':
				memset(peerIpAddr, 0x00, sizeof(peerIpAddr));
				strncpy(peerIpAddr, optarg, sizeof(peerIpAddr)-1);
				hostflg = 1;
				break;
			case 'p':
				memset(argPort, 0x00, sizeof(argPort));
				strncpy(argPort, optarg, sizeof(argPort)-1);
				portflg = 1;
				break;
			case '?':
				errflg++;
				break;
		}

	if(!imsiflg || errflg || (dirflg && !outflg)){
		usage();
		exit(1);
	}

	if(!hostflg){
		strcpy(peerIpAddr, LOOPBACK_IP);
	}

	if(dirflg){
		if(!is_dirpath(outpath)){
			printf("%s is not a directory\n", outpath);
			exit(1);
		}
	}else{
		strcpy(outpath, "./");
	}

	if(portflg){
		portNum = atoi(argPort);
	}else{
		portNum = get_cond_port_number();
	}
	if(portNum < 0)
		return 1;

	printf("******* Press Q(q) to exit  ********\n");
	trace_main(peerIpAddr, portNum, imsi, outflg, outpath);

	return 0;
}

void usage(void)
{
	printf("TRC_PRINT [-w] [-d path] [-h IP Address] [-p port number] -m IMSI\n");
	printf("          -w\tAutomatically save trace information to file below current directory.\n");
	printf("            \tBut when you use \'-d\', trace file is saved below the specified directory\n");
	printf("            \tIf you don't use this option, trace information is not saved automatically\n");
	printf("          -d\tDirectory name for trace file\n");
	printf("            \tWhen you use this option, use this with \'-w\'\n");
	printf("          -r\tIP Address of the server on that BSDM is running.\n");
	printf("          -p\tPort Number of the server on that BSDM is running.\n");
	printf("          -m\tIMSI Number to be traced to\n");
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

	if (conflib_getNthTokenInFileSection (sysfile, "SOCKET_PORT", "COND", 1, tmp) < 0){
		fprintf(stderr, "Can't get COND port from %s!\n", sysfile);
		return -1;
	}

	return strtol(tmp, 0, 0);
}

int connect_to_peer(char *ipAddr, int portNum)
{
	int				   peersock;
	struct sockaddr_in peeraddr;

	peersock = socket(AF_INET, SOCK_STREAM, 0);
	if(peersock < 0){
		fprintf(stderr, "Socket Creation Error - %s\n", strerror(errno));
		return -1;
	}

	bzero(&peeraddr, sizeof(peeraddr));
	peeraddr.sin_addr.s_addr = inet_addr(ipAddr);
	peeraddr.sin_family = AF_INET;
	peeraddr.sin_port = htons(portNum);

	if(connect(peersock, (struct sockaddr *)&peeraddr, sizeof(peeraddr)) < 0){
		fprintf(stderr, "Connect Error - %s\n", strerror(errno));
		return -1;
	}

	socklib_setNonBlockMode(peersock);

	return peersock;
}

int read_from_fd(int fd, char *buff, int readsize)
{
	return read(fd, buff, readsize);
}

void parse_sock_header(SockLibHeadType *sockHeader, int *sizeToRead, int *traceFlag)
{
	*sizeToRead = ntohl(sockHeader->bodyLen);
	if(sockHeader->mapType == 1) //trace
		*traceFlag = 1;
	else
		traceFlag = 0;

#ifdef DEBUG_1
fprintf(stdout, "MAPTYPE [%d], BODY LEN [%d]\n", sockHeader->mapType, *sizeToRead);
#endif
}

#define	IMSI_TOKEN		"IMSI:"
int	if_imsi_match(char *buff, char *imsi, char **posTrace)
{
	char 	*p, *p1;

	p = strstr(buff, IMSI_TOKEN);

	if(!p)
		return 0;

	p += strlen(IMSI_TOKEN);
	if(strncmp(p, imsi, 15))
		return 0;

	p1 = strchr(p, '\n');
	if(!p1){
		*posTrace = buff;
	}else{
		p1++;
		*posTrace = p1;
	}

	return 1;
	
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

int read_trace(int sock, FILE *fp, char *imsi)
{
	char	buff[1024], *start = NULL, tmpB[128];
	int		readsize, traceFlag, sizeToRead, matchFlag;
	int		readBytes, restBytes;

	readsize = sizeof(SockLibHeadType);
	if(read_from_fd(sock, buff, readsize) < 0)
		return -1;

	parse_sock_header((SockLibHeadType *)buff, &sizeToRead, &traceFlag);

	if(readsize == 0)
		return 0;

	if(!traceFlag){
		while(read_from_fd(sock, buff, sizeof(buff)) > 0);
		return 0;
	}

	readsize = (sizeToRead > (sizeof(buff)-1))?(sizeof(buff)-1):sizeToRead;

	readBytes = 0;
	memset(buff, 0x00, sizeof(buff));
	if((readBytes = read_from_fd(sock, buff, readsize)) < 0)
		return -1;

	if(!readBytes)
		return 0;


	if(if_imsi_match(buff, imsi, &start)){
		restBytes = sizeToRead - readBytes;
		sprintf(tmpB, "\n\n[ ************************ TRACE RECEIVERD TIME : "
					  "%s ************************ ]\n", current_time());
		if(fp){
			if(fputs(tmpB, fp) == EOF){
				fprintf(stderr, "[ERROR] fputs error - %s\n", strerror(errno));
				fclose(fp);
				exit(0);
			}
			if(!fwrite(start, 1, strlen(start), fp)){
				fprintf(stderr, "[ERROR] fwrite error - %s\n", strerror(errno));
				fclose(fp);
				exit(0);
			}
		}
		
		// print trace out to STDOUT
		puts(tmpB);
		fwrite(start, 1, strlen(start), stdout);

		while(restBytes > 0){
			readsize = (restBytes > sizeof(buff))?sizeof(buff):restBytes;
			if((readBytes = read_from_fd(sock, buff, readsize)) < 0)
			        return -1;

			if(fp){
				if(!fwrite(buff, 1, readBytes, fp)){
					fprintf(stderr, "[ERROR] fwrite error - %s\n", strerror(errno));
					fclose(fp);
					exit(0);
				}
			}
			fwrite(buff, 1, readBytes, stdout);

			restBytes -= readBytes;
		}

		puts("\n\n");
		if(fp){
			fputs("\n", fp);
			fflush(fp);
		}
		fflush(stdout);
		
	}else{
		while(read_from_fd(sock, buff, sizeof(buff)) > 0);
#ifdef DEBUG_1
		puts(buff);
#endif
		return 0;
	}

	return 1;

}

FILE *open_trace_file(char *path, char *imsi)
{
	char  outfile[512], tmpB[100];
	struct tm	tmNow;
	FILE		*fp;
	int	  len;
	time_t		tNow = time(NULL);

	memset(outfile, 0x00, sizeof(outfile));
	strcpy(outfile, path);
	len = strlen(outfile);
	if(outfile[len-1] == '/')
		outfile[len-1] = 0x00;

	localtime_r(&tNow, &tmNow);
	sprintf(tmpB, "/CALL_TRACE_%s_T%04d%02d%02d%02d%02d%02d.TRC",
				  imsi, tmNow.tm_year+1900, tmNow.tm_mon+1, tmNow.tm_mday,
				  tmNow.tm_hour, tmNow.tm_min, tmNow.tm_sec);
	strcat(outfile, tmpB);

	fp = fopen(outfile, "w");
	if(!fp){
		fprintf(stderr, "[ERROR]fopen - %s\n", strerror(errno));
		return NULL;
	}

	return fp;

}

int trace_main(char *hostIp, int portNum, char *imsi, int outflag, char *path)
{
	int				fd_stdin, peersock, maxfd, status, readsize;
	int				selStatus;
	fd_set			fdset_Read;
	FILE			*fp = NULL;
	char			buff[128];
	struct timeval	timeout;

	fd_stdin = fileno(stdin);

	peersock = connect_to_peer(hostIp, portNum);
	if(peersock < 0)
		return 1;

	if(outflag){
		fp = open_trace_file(path, imsi);
		if(!fp)
			exit(1);
	}

	timeout.tv_sec = 0;
	timeout.tv_usec = 100;

	while(1){
		FD_ZERO(&fdset_Read);
		FD_SET(fd_stdin, &fdset_Read);
		FD_SET(peersock, &fdset_Read);
		maxfd = (fd_stdin >= peersock)?fd_stdin:peersock;
		maxfd += 1;

		selStatus = select(maxfd, &fdset_Read, NULL, NULL, &timeout);
		if(selStatus < 0){
			fprintf(stderr, "[ERROR] select - %s\n", strerror(errno));
		}else if(selStatus > 0){
			if(FD_ISSET(fd_stdin, &fdset_Read)){
				memset(buff, 0x00, sizeof(buff));
				read_from_fd(fd_stdin, buff, 1);
				if(buff[0] == 'q' || buff[0] == 'Q'){
					close(peersock);
					fclose(fp);
					return 1;
				}
			}
			if(FD_ISSET(peersock, &fdset_Read)){
				status = read_trace(peersock, fp, imsi);
				if(status < 0){
					// try to reconnect to peer.
					peersock = connect_to_peer(hostIp, portNum);
					if(peersock < 0)
						return 1;
				}
			}
		}
#ifdef DEBUG_1
		else{
			printf(".");
		}
#endif
	}

	return 0;

}
