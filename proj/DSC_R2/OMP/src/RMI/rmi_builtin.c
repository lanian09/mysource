#include "rmi_proto.h"
#include <sys/types.h>
#include <dirent.h>
#include <sys/ipc.h>

extern FILE		*logFp;
extern char		logFname[256];
extern int		hisBufIndex;
extern char		hisBuf[RMI_NUM_HISTORY_BUFF][1024];
extern char		prompt[32];
extern int      batchResCode, batchResFlag;
extern int 		batchend;
extern char		passwd[32];

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int rmi_isBuiltInCmd (char *input)
{
	char	*ptr, tmp[1024], *cmdName,*para, *para2;
    int idx;

	// command name을 잘라낸다.
	strcpy (tmp, input);
	cmdName = (char*)strtok_r(tmp," \t",&para);
	if (para == NULL) { // 파라미터가 없는 경우
		cmdName = input;
	}

	if (!strcasecmp (cmdName, "exit")) {
		rmi_terminate (1);
		return 1;
	} else if (!strcasecmp (cmdName, "open-log")) {
		rmi_builtin_open_log (para);
		return 1;
	} else if (!strcasecmp (cmdName, "close-log")) {
		rmi_builtin_close_log (para);
		return 1;
	} else if (!strcasecmp (cmdName, "act-cmd-file")) {
		rmi_builtin_act_cmd_file (para);
		return 1;
	} else if (!strcasecmp (cmdName, "file_exe")) {
		rmi_builtin_file_exe (para);
		return 1;
	} else if (!strcasecmp (cmdName, "dis-cmd-file")) {
		rmi_builtin_dis_cmd_file (para);
		return 1;
	} else if (cmdName[0] == '!') {
		rmi_builtin_history (input);
		return 1;
	} else if (!strcasecmp (cmdName, "h") || !strcasecmp (cmdName, "history")) {
		rmi_builtin_dis_history ();
		return 1;
	}

	return 0;

} //----- End of rmi_isBuiltInCmd -----//



//------------------------------------------------------------------------------
// mmcd로 log-out 명령을 보내고, 로그파일이 열려 있으면 close한 후 종료된다.
//------------------------------------------------------------------------------
int rmi_terminate (int logoutFlag)
{
	if (logoutFlag)
#if 0 /* jhnoh : 030815 */
		rmi_send2mmcd ("log-out", RMI_REQID_LOGOUT, -1);
#else
		rmi_send2mmcd ("log-out", -1, 0);
#endif

	if (logFp != NULL)
		fclose(logFp);
	
	exit(0);

} //----- End of rmi_terminate -----//



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int rmi_builtin_open_log (char *para)
{
	char	*env;
	char    msgBuf[1024];

	if (logFp != NULL) {
		fprintf(stderr,"\n    ALREADY_OPENED [%s]\n", logFname);
		return -1;
	}

	if ((env = getenv(IV_HOME)) == NULL) {
		sprintf(msgBuf,"\n    not found environment name [%s]\n", IV_HOME);
		return -1;
	}
	sprintf (logFname, "%s/log/rmi_%s", env, commlib_printTime());
	if ((logFp = fopen (logFname, "w")) == NULL) {
		sprintf(msgBuf,"\n    not fopen fail[%s]; err=%d(%s)\n", logFname, errno, strerror(errno));
		return -1;
	}

	sprintf(msgBuf,"\n      LOG_FILE [%s] OPENED\n", logFname);

	fprintf(stderr,"\n  [%s] ", prompt);

	return 1;

} //----- End of rmi_builtin_open_log -----//



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int rmi_builtin_close_log (char *para)
{
	char msgBuf[1024];

	if (logFp == NULL) {
		sprintf(msgBuf,"\n      LOG_FILE NOT OPENED\n", logFname);
		return -1;
	}

	sprintf(msgBuf,"\n      LOG_FILE [%s] CLOSED\n", logFname);

	fclose(logFp);
	strcpy (logFname, "");
	fprintf(stderr,"\n    [%s] ", prompt);

	return 1;

} //----- End of rmi_builtin_close_log -----//


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int rmi_builtin_history (char *inputString)
{
	int		i,num;
	char	input[1024], tmp[1024], *ptr1, *ptr2;
	
	if (inputString[1] == '!') {
		sprintf (input, "%s %s",
				hisBuf[(hisBufIndex+RMI_NUM_HISTORY_BUFF-1) % RMI_NUM_HISTORY_BUFF],
				&inputString[2]);
		fprintf(stderr,"\n     %s\n", input);
#if 0 /* jhnoh : 030815 */
		rmi_send2mmcd (input, RMI_REQID_GENERAL_CMD, -1);
#else
		rmi_send2mmcd (input, -1, 0);
#endif
		return 1;
	} else {
		if (isdigit(inputString[1])) {
			num = atoi(&inputString[1]);
			if (num < hisBufIndex) {
				// 숫자뒤에 뭔가 더 붙어 있으면 이전 명령어 뒤에 붙이기 위해 잘라낸다.
				strcpy (tmp, inputString);
				ptr1 = (char*)strtok_r(tmp," \t",&ptr2);
				if (ptr2 == NULL) {
					strcpy (input, hisBuf[num]);
				} else {
					sprintf (input, "%s %s", hisBuf[num], ptr2);
				}

				fprintf(stderr,"\n    %s\n", input);
#if 0 /* jhnoh : 030815 */
				rmi_send2mmcd (input, RMI_REQID_GENERAL_CMD, -1);
#else
				rmi_send2mmcd (input, -1, 0);
#endif
				return 1;
			}
		} else {
			// 뭔가 더 붙어 있으면 이전 명령어 뒤에 붙이기 위해 잘라낸다.
			strcpy (tmp, inputString);
			ptr1 = (char*)strtok_r(tmp," \t",&ptr2);
			for (i=hisBufIndex-1; i>=0; i--) {
#if 0
				if (strstr(hisBuf[i], &ptr1[1]) != NULL) {
#else 
				if (!(strncmp(hisBuf[i], &ptr1[1], strlen(&ptr1[1])))) {
#endif
					if (ptr2 == NULL) {
						strcpy (input, hisBuf[i]);
					} else {
						sprintf (input, "%s %s", hisBuf[i], ptr2);
					}

					fprintf(stderr,"\n    %s\n", input);
#if 0 /* jhnoh : 030815 */
					rmi_send2mmcd (input, RMI_REQID_GENERAL_CMD, -1);
#else 
					rmi_send2mmcd (input, -1, 0);
#endif
					return 1;
				}
			}
		}
	}

	fprintf(stderr,"\n      not_found_command\n");
	fprintf(stderr,"\n    [%s] ", prompt);

	return -1;

} //----- End of rmi_builtin_history -----//



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int rmi_builtin_dis_history (void)
{
	int		i;
	char msgBuf[1024];

	fprintf(stderr,"\n");
	for (i=0; i<RMI_NUM_HISTORY_BUFF; i++) {
		if (strcasecmp(hisBuf[i], "")) {
			sprintf(msgBuf,"  %2d    %s\n", i, hisBuf[i]);
			fprintf(stderr,"     %s", msgBuf);
		}
	}
	fprintf(stderr,"\n    [%s] ", prompt);

	return 1;

} //----- End of rmi_builtin_dis_history -----//



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int rmi_saveInputHistory (char *input)
{
	int	i;

	// 이전것들을 하나씩 앞으로 당긴다.
	if (hisBufIndex >= RMI_NUM_HISTORY_BUFF) {
		for (i=1; i<hisBufIndex; i++) {
			strcpy (hisBuf[i-1], hisBuf[i]);
		}
	}

	if (hisBufIndex < RMI_NUM_HISTORY_BUFF)
	    hisBufIndex++;

	strcpy (hisBuf[hisBufIndex-1], input);

	return 1;

} //----- End of rmi_builtin_dis_history -----//

// MMCD로 부터 COMPLETED 가 오면 return 1
//             그 외의 값이 오면 return 0
int rmi_WaitResFromMMCD()
{
	int		ret,actFd;
	char choice[32];
	SockLibMsgType	rxSockMsg;
	MMLClientResMsgType	*rxCliResMsg;
	struct timeval t;
	fd_set fDS;
#define STDIN_FD 0
	int fD_POS;	

	int end_flag = 0;

	t.tv_sec = 0;
	t.tv_usec = 200000;

	if (batchend==1){
		signal (SIGALRM, rmi_receiveResult);
		ualarm (50000, 0);
		return 0;
	}

	do {
		ret = socklib_action ((char*)&rxSockMsg, &actFd);
		switch (ret) {
			case SOCKLIB_SERVER_MSG_RECEIVED:
				rxCliResMsg = (MMLClientResMsgType*)rxSockMsg.body;

				if (rxCliResMsg->head.confirm == 1) {
					fprintf(stderr, "\n\n      Input The Password : %s.", passwd);
					
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

				if (rxCliResMsg->head.confirm != 1) {
					fprintf(stderr,"%s",rxCliResMsg->body);

					// log file이 열려 있으면 기록한다.
					if (logFp != NULL)
						fprintf(logFp,"%s",rxCliResMsg->body);

					rxCliResMsg->head.cliReqId = ntohl(rxCliResMsg->head.cliReqId);
					if (rxCliResMsg->head.batchFlag == 1) {
						if (rxCliResMsg->head.contFlag == 0) {
							batchResCode = rxCliResMsg->head.resCode;
							batchResFlag = 1;
						}
					}

                    // exit (CPU OVERLOAD) && (CANC-USR)
                    if(strstr(rxCliResMsg->body,"BYE...") != NULL) 
                        rmi_terminate (1);

				}

				//fprintf(stderr,"\n[%s] ", prompt);
				if(strstr(rxCliResMsg->body, "FAIL") != NULL ||
					strstr(rxCliResMsg->body, "ERROR") != NULL) {
					sleep(1);
					signal (SIGALRM, rmi_receiveResult);
					ualarm (50000, 0);
					return 0;
				}
				else if(strstr(rxCliResMsg->body, "SUCCESS") != NULL ||
					strstr(rxCliResMsg->body, "COMPLETED") != NULL) {
					sleep(1);
					signal (SIGALRM, rmi_receiveResult);
					ualarm (50000, 0);
					return 1;
				}
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
		
		sleep(1);
	} while(1);
	//while (ret == SOCKLIB_SERVER_MSG_RECEIVED) ;

	signal (SIGALRM, rmi_receiveResult);
	ualarm (50000, 0);

	return 0;
}

int rmi_builtin_file_exe (char *para)
{
	char	fname[256],lineBuf[1024], msgBuf[1024], end;
	int     ret,actFd,cmdCnt=0, idx, idx1, idx2, confirmCheck;
	FILE	*batchFp;
	char    choice;
	SockLibMsgType  rxSockMsg;
	MMLClientResMsgType *rxCliResMsg;
	struct stat		statBuf;
    char    para1[128], para2[128], para3[128]; 

	struct timeval t;
    fd_set fDS;
#define STDIN_FD 0
    int fD_POS;
    
    memset(para1, 0x00, sizeof(para1));
    memset(para2, 0x00, sizeof(para2));
    memset(para3, 0x00, sizeof(para3));

    t.tv_sec = 0;
    t.tv_usec = 200000;

    if(para == NULL) {
        sprintf(msgBuf,"\n      TYPE THE FILE NAME\n");
        printf("%s", msgBuf);
        return 1;
    }

    strncpy(para3, para, strlen(para)); 
    for (idx=0; isspace(para3[idx]); idx++);
    for (; para3[idx]!=0 && para3[idx]!=','; idx++) {
       para1[idx] = para3[idx];
    }

    if (idx !=0) {
        idx2=idx-1;
        for ( ; isspace(para1[idx2]); idx2--) 
            para1[idx2] = 0;
        para1[idx] = 0;
    } 
    
    if(para1 == NULL) {
        sprintf(msgBuf,"\n TYPE THE FILE NAME\n");
        printf("%s", msgBuf);
        return 1;
    }

    if (para3[idx] != 0) {
        for (idx++; isspace(para3[idx]); idx++);
        for (idx1=0; para3[idx] != 0; idx++, idx1++) {
            para2[idx1] = para3[idx];
        }
        if (idx1 != 0) {
            idx2=idx1-1;
            for ( ; isspace(para2[idx2]); idx2--) 
                para2[idx2] = 0;
            para2[idx1] = 0;
        }

        if (!strcasecmp(para2, "on"))
            confirmCheck = 1;
        else if (!strcasecmp(para2, "off"))
            confirmCheck = 0;
        else if (!strcasecmp(para2, ""))
            confirmCheck = 1;
        else {
            sprintf(msgBuf,"\n      parameter error [%s]\n act-cmd-file file-name on/off\n", para2);
			printf("%s", msgBuf);
            return -1; 
        }
    }
    else {
        confirmCheck = 1;
    }

    strcpy(fname, para1);

	if (!strcasecmp(para1, "")) {
        sprintf(msgBuf,"\n      TYPE THE FILE NAME\n");
        printf("%s", msgBuf);
		return -1;
    }

	if (stat (fname, &statBuf) < 0) {
		sprintf(msgBuf,"\n      CAN'T ACCESS TO FILE[%s]; err=%d(%s)\n", para1, errno, strerror(errno));
        printf("%s", msgBuf);
		return -1;
	}

	if (!S_ISREG(statBuf.st_mode)) {
		sprintf(msgBuf,"\n      NOT REGULAR FILE[%s]\n", para1);
        printf("%s", msgBuf);
		return -1;
	}

	if ((batchFp = fopen (fname, "r")) == NULL) {
		sprintf(msgBuf,"\n      NOT FOPEN FAIL[%s]; err=%d(%s)\n", para1, errno, strerror(errno));
        printf("%s", msgBuf);
		return -1;
	}

	while ((fgets(lineBuf, sizeof(lineBuf), batchFp)) != NULL)
	{
		end = ' ';

		if (lineBuf[0]=='#' || lineBuf[0]=='\n')
			continue;

		lineBuf[strlen(lineBuf)-1] = 0;
#if 0 /* jhnoh : 030917 */
		rmi_send2mmcd (lineBuf, -1, 1);
#else
		rmi_send2mmcd (lineBuf, 1, 1); /* no confirm when batch mode */
#endif
		if(rmi_WaitResFromMMCD () <= 0)
			break;

		if (batchResCode != 0 && confirmCheck==1) 
			break;

		cmdCnt++;

	}
	fclose(batchFp);

	if (batchResCode != 0  && confirmCheck==1) {
		sprintf(msgBuf,"\n      >>> STOPPED BATCH-JOB DUE TO RESULT_FAILURE\n");
        printf("%s", msgBuf);
	}
	sprintf(msgBuf,"\n      %d COMMANDS EXECUTED SUCCESSFULLY\n", cmdCnt);
    printf("%s", msgBuf);

	sprintf(msgBuf,"\n    [%s] ", prompt);

	return 1;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int rmi_builtin_act_cmd_file (char *para)
{
	char	*env, fname[256],lineBuf[1024], msgBuf[1024], end;
	int     ret,actFd,cmdCnt=0, idx, idx1, idx2, confirmCheck;
	FILE	*batchFp;
	char    choice;
	SockLibMsgType  rxSockMsg;
	MMLClientResMsgType *rxCliResMsg;
	struct stat		statBuf;
    char    para1[128], para2[128], para3[128]; 

	struct timeval t;
    fd_set fDS;
#define STDIN_FD 0
    int fD_POS;
    
    memset(para1, 0x00, sizeof(para1));
    memset(para2, 0x00, sizeof(para2));
    memset(para3, 0x00, sizeof(para3));

    t.tv_sec = 0;
    t.tv_usec = 200000;

    if(para == NULL) {
        sprintf(msgBuf,"\n      TYPE THE FILE NAME\n");
        printf("%s", msgBuf);
        return 1;
    }

    strncpy(para3, para, strlen(para)); 
    for (idx=0; isspace(para3[idx]); idx++);
    for (; para3[idx]!=0 && para3[idx]!=','; idx++) {
       para1[idx] = para3[idx];
    }

    if (idx !=0) {
        idx2=idx-1;
        for ( ; isspace(para1[idx2]); idx2--) 
            para1[idx2] = 0;
        para1[idx] = 0;
    } 
    
    if(para1 == NULL) {
        sprintf(msgBuf,"\n      TYPE THE FILE NAME\n");
        printf("%s", msgBuf);
        return 1;
    }

    if (para3[idx] != 0) {
        for (idx++; isspace(para3[idx]); idx++);
        for (idx1=0; para3[idx] != 0; idx++, idx1++) {
            para2[idx1] = para3[idx];
        }
        if (idx1 != 0) {
            idx2=idx1-1;
            for ( ; isspace(para2[idx2]); idx2--) 
                para2[idx2] = 0;
            para2[idx1] = 0;
        }

        if (!strcasecmp(para2, "on"))
            confirmCheck = 1;
        else if (!strcasecmp(para2, "off"))
            confirmCheck = 0;
        else if (!strcasecmp(para2, ""))
            confirmCheck = 1;
        else {
            sprintf(msgBuf,"\n      parameter error [%s]\n act-cmd-file file-name on/off\n", para2);
			printf("%s", msgBuf);
            return -1; 
        }
    }
    else {
        confirmCheck = 1;
    }

    if ((env = getenv(IV_HOME)) == NULL) {
        sprintf(msgBuf,"\n      not found environment name [%s]\n", IV_HOME);
        printf("%s", msgBuf);
        return -1;
    }

    sprintf(fname,"%s/DATA/cmd/%s",env,para1);

	if (!strcasecmp(para1, "")) {
        sprintf(msgBuf,"\n      TYPE THE FILE NAME\n");
        printf("%s", msgBuf);
		return -1;
    }

	if (stat (fname, &statBuf) < 0) {
		sprintf(msgBuf,"\n      CAN'T ACCESS TO FILE[%s]; err=%d(%s)\n", para1, errno, strerror(errno));
        printf("%s", msgBuf);
		return -1;
	}

	if (!S_ISREG(statBuf.st_mode)) {
		sprintf(msgBuf,"\n      NOT REGULAR FILE[%s]\n", para1);
        printf("%s", msgBuf);
		return -1;
	}

	if ((batchFp = fopen (fname, "r")) == NULL) {
		sprintf(msgBuf,"\n      NOT FOPEN FAIL[%s]; err=%d(%s)\n", para1, errno, strerror(errno));
        printf("%s", msgBuf);
		return -1;
	}

	while ((fgets(lineBuf, sizeof(lineBuf), batchFp)) != NULL)
	{
		end = ' ';

		if (lineBuf[0]=='#' || lineBuf[0]=='\n')
			continue;

		lineBuf[strlen(lineBuf)-1] = 0;
#if 0 /* jhnoh : 030917 */
		rmi_send2mmcd (lineBuf, -1, 1);
#else
		rmi_send2mmcd (lineBuf, 1, 1); /* no confrim when batch mode */
#endif
		if (batchResCode != 0 && confirmCheck==1) 
			break;

		cmdCnt++;
	}
	fclose(batchFp);

	if (batchResCode != 0  && confirmCheck==1) {
		sprintf(msgBuf,"\n       >>> STOPPED BATCH-JOB DUE TO RESULT_FAILURE\n");
        printf("%s", msgBuf);
	}
	sprintf(msgBuf,"\n      %d COMMANDS EXECUTED SUCCESSFULLY\n", cmdCnt);
    printf("%s", msgBuf);

	sprintf(msgBuf,"\n    [%s] ", prompt);

	return 1;

} //----- End of rmi_builtin_act_cmd_file -----//


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int rmi_builtin_dis_cmd_file (char *para)
{
	char	*env, fname[256],lineBuf[1024], name[256], count=0;
	DIR *dirp;
	struct dirent *dp;
    if ((env = getenv(IV_HOME)) == NULL) {
        fprintf(stderr,"\n      not found environment name [%s]\n", IV_HOME);
        return -1;
    }
    sprintf(fname,"%s/DATA/cmd",env);
	dirp = opendir(fname);

	sprintf(lineBuf, "\n    == [BATCH JOB FILE LIST] =============================================\n");

	while ((dp = readdir(dirp)) != NULL) {
		if (strcasecmp(dp->d_name, ".") && strcasecmp(dp->d_name, "..")) {
			sprintf(&lineBuf[strlen(lineBuf)], "\n      %s", dp->d_name);
			count++;
		}
	}
	
	if (count == 0) {
		sprintf(lineBuf, "\n\n      NOT EXIST BATCH JOB FILE \n");
		printf("%s", lineBuf);
	}
	else {
		strcat(lineBuf, "\n    ====================================================================== \n");
		sprintf(&lineBuf[strlen(lineBuf)], "      TOTAL = %d\n", count);
		printf("%s", lineBuf);
	}

	closedir(dirp);

	return 1;

} //----- End of rmi_builtin_dis_cmd_file -----//

