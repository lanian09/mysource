#include "fimd_proto.h"

extern int	fimdQid, ixpcQid, condQid, eqSysCnt;
extern int	dataPortNum, eventPortNum;
extern time_t   currentTime, ixpc_rec_Time[SYSCONF_MAX_ASSO_SYS_NUM], samd_rec_Time[SYSCONF_MAX_ASSO_SYS_NUM];
extern char	mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN], sysLabel[COMM_MAX_NAME_LEN];
extern char	trcBuf[4096], trcTmp[1024], sfdbFileName[256];
extern int	trcLogId, trcErrLogId, trcFlag, trcLogFlag;
extern SFM_sfdb	*sfdb;
extern int	numMmcHdlr;
extern FimdMmcHdlrVector	mmcHdlrVector[FIMD_MAX_MMC_HANDLER];
extern FimdKeepAlive		fimdKeepAlive[MAX_KEEPALIVE];
extern int fimd_checkProcState(int sysIndex, char *procName, int procIndex);

extern int stsIndex[3];
void rshellResult_dumHdlr (int signo); // connect hang add by helca 2007.06.10
void fimd_keepAliveIncrease(int msgId, char *sysName)
{
	int		sys, i;

	if ( (sys = fimd_getSysIndexByName ( sysName ) ) < 0 ) {
		sprintf(trcBuf,"[fimd_keepAliveIncrease] ERR sysName[%s], msgId=%d\n", sysName, msgId);
		trclib_writeLogErr (FL,trcBuf);
		return ;
	}

	for (i=0; i < MAX_KEEPALIVE; i++) {
		if (fimdKeepAlive[i].category == msgId) {
			if ( fimdKeepAlive[i].sysIdx == sys ){
				fimdKeepAlive[i].cnt++;
				break;
			}
		}
	}
	return;
}

/*
* 현재 시스템의 ixpc또는 samd 가 죽으면 마지막 상태가 OMP에 남아 있는데 
* 아래의 함수에서 일정시간 (10초) 동안 받은 메세지가 없다면 그 시스템의 ixpc와 samd 를 dead로 바꾼다
*/
/* SDMD requests to change 10 seconds into 5  */
#define MP_PROC_CHECK_TIME	10
void fimd_checkSysAlive()
{
	int i, flag = 0;

	for(i = 0; i < SYSCONF_MAX_ASSO_SYS_NUM; i++){
		if( (time(0) - ixpc_rec_Time[i]) > MP_PROC_CHECK_TIME)
		{
#if 1
	sprintf(trcBuf,"[fimd_checkSysAlive] >>> 1.IXPC[%d] (%d:%d). \n",i, (int)time(0), (int)ixpc_rec_Time[i] );
	trclib_writeLogErr (FL,trcBuf);
#endif
			/* DSC Down Active info 초기화 */	
			//			sfdb->sys[i].commInfo.systemDup.myStatus = 3;

			if ( stsIndex[i] == 0) linktest(i);	
			flag = 1;
			//review_linktest(i);
			// if "break" is inserted at line, SAMD's status cannot be checked.
			// So comment out
			//			break;
		} 
		else if((time(0) - samd_rec_Time[i]) > MP_PROC_CHECK_TIME) { 
#if 1
	sprintf(trcBuf,"[fimd_checkSysAlive] >>> 2.SAMD[%d].(%d:%d) \n" , i, (int)time(0), (int)samd_rec_Time[i]);
	trclib_writeLogErr (FL,trcBuf);
#endif

			/* DSC Down Active info 초기화 */
			//			sfdb->sys[i].commInfo.systemDup.myStatus = 3;

			if ( stsIndex[i] == 0) linktest(i);
			flag = 1;
		}else {
#if 0
	sprintf(trcBuf,"[fimd_checkSysAlive] >>> 3.ETC[%d].(%d:%d:%d) \n" , i, (int)time(0), (int)ixpc_rec_Time[i], (int)samd_rec_Time[i]);
	trclib_writeLogErr (FL,trcBuf);
#endif
			stsIndex[i] = 0;	
			continue;
		}	
	}

//	if(flag){
//		fimd_broadcastAlmEvent2Client();
//	}
}

void fimd_checkKeepAlive()
{
	int i,/*j,*/ sysIndex, changeFlag=0;
	int	over_cnt=0;

	for (i=0; i<MAX_KEEPALIVE; i++) {
		if (!fimdKeepAlive[i].category)	continue;

		if (fimdKeepAlive[i].cnt == fimdKeepAlive[i].oldcnt) {
			// 20041202.mnpark 
			if(fimdKeepAlive[i].retry != -1)
				fimdKeepAlive[i].retry++;
			else
				fimdKeepAlive[i].retry +=2;
		}else
			fimdKeepAlive[i].retry = INIT_KEEPALIVE_RETRY;

		fimdKeepAlive[i].oldcnt = fimdKeepAlive[i].cnt;

		if (fimdKeepAlive[i].retry >= (KEEPALIVE_CHECK_CNT+over_cnt)) 
		{

			switch (fimdKeepAlive[i].category) 
			{
				case MSGID_SYS_COMM_STATUS_REPORT:
					sysIndex = fimdKeepAlive[i].sysIdx;	

					if(sysIndex < 0) break;

					logPrint(trcLogId,FL,"[%s-SAMD]retryCnt %d\n",sfdb->sys[sysIndex].commInfo.name, fimdKeepAlive[i].retry);
					break;
#if 0
				case MSGID_SYS_SPEC_HW_STATUS_REPORT:
					sysIndex = fimdKeepAlive[i].sysIdx;	
					if(sysIndex < 0) break;
					logPrint(trcLogId,FL,"[%s-SHMD]retryCnt %d\n",sfdb->sys[sysIndex].commInfo.name, fimdKeepAlive[i].retry);

					for (j=0; j < sfdb->sys[sysIndex].commInfo.procCnt; j++) {
						if (!strcasecmp(sfdb->sys[sysIndex].commInfo.procInfo[j].name, "SHMD")) {

							// mask된 놈은 상태관리에서 제외한다.
							if (sfdb->sys[sysIndex].commInfo.procInfo[j].mask == SFM_ALM_MASKED) continue;

							sfdb->sys[sysIndex].commInfo.procInfo[j].prevStatus = sfdb->sys[sysIndex].commInfo.procInfo[j].status;
							//sfdb->sys[sysIndex].commInfo.procInfo[j].status 	= SFM_STATUS_DEAD;
							// keepAlive경우 fimd_lndbOpenCallMon의 rsh명령에 의해 만들어진 프로세스 상태를 참고한다.
							sfdb->sys[sysIndex].commInfo.procInfo[j].status 	= fimd_checkProcState(sysIndex,"SHMD",j);

							if (sfdb->sys[sysIndex].commInfo.procInfo[j].prevStatus != sfdb->sys[sysIndex].commInfo.procInfo[j].status) {
								fimd_hdlProcAlm(sysIndex, j, 1);
								changeFlag = 1;
							}
							break;
						}
					}
					break;
#endif
				case MSGID_SYS_SPEC_CONN_STATUS_REPORT:
					break;
			}

			fimdKeepAlive[i].retry = -10;
		}
	}

	if (changeFlag) {
		for (sysIndex=0; sysIndex <eqSysCnt ; sysIndex++) {
			fimd_updateSysAlmInfo(sysIndex);
		}
		fimd_broadcastAlmEvent2Client();
	}
}



int ping_test_with_ip (char *ipaddr)
{
    int sts,j;
    char    result[16];

    j = 0;

    sprintf(result, "%s\n",(pingtest(ipaddr, 0, 1,500000)>0)? "alive":"die");

    if (!memcmp (result, "alive", 5)) {
        sts = SFM_LAN_CONNECTED;
    } else {

        sts = SFM_LAN_DISCONNECTED;
        do {
            commlib_microSleep(100000);
            sprintf(result, "%s\n",(pingtest(ipaddr, 0, 1,500000)>0)? "alive":"die");
            if (!memcmp (result, "alive", 5)) {
                sts = SFM_LAN_CONNECTED;
                break;
            }
            j++;
        } while(sts == SFM_LAN_DISCONNECTED && j != 2);
    }

    return sts;
}

#define SYS_OMP		0
#define SYS_SCMA	1
#define SYS_SCMB	2
// 2007.01.12 by helca
void linktest(int sysIndex)
{
    int             i, j;
	char			checkip[2][16];
	int				sts, dRet = 0;


	/**<
	  Link 확인.
	  1. Primary ip 로 확인하고 안되면 secondary ip로 확인 한다.
	  2. ping 이 되는 ip로 rsh 실행.
	  3. ping 이 둘다 안되면 all dead 처리
	**/

	switch (sysIndex) {
	case SYS_OMP:	/**< ip 정보는 DSCA.lan 정보에서 얻는다. */
		snprintf (checkip[0], sizeof(checkip[0]), "%s", sfdb->sys[SYS_SCMA].commInfo.lanInfo[0].targetIp);
		snprintf (checkip[1], sizeof(checkip[1]), "%s", sfdb->sys[SYS_SCMA].commInfo.lanInfo[1].targetIp);
		break;
	case SYS_SCMA:	/**< ip 정보는 OMP.lan 정보에서 얻는다. */
		snprintf (checkip[0], sizeof(checkip[0]), "%s", sfdb->sys[SYS_OMP].commInfo.lanInfo[0].targetIp);
		snprintf (checkip[1], sizeof(checkip[1]), "%s", sfdb->sys[SYS_OMP].commInfo.lanInfo[1].targetIp);
		break;
	case SYS_SCMB:	/**< ip 정보는 OMP.lan 정보에서 얻는다. */
		snprintf (checkip[0], sizeof(checkip[0]), "%s", sfdb->sys[SYS_OMP].commInfo.lanInfo[2].targetIp);
		snprintf (checkip[1], sizeof(checkip[1]), "%s", sfdb->sys[SYS_OMP].commInfo.lanInfo[3].targetIp);
		break;
	default:
		return;
		break;
	}

	for (i=0; i < 2/*lancnt*/; i++)  {
		sts = ping_test_with_ip (checkip[i]);
		if (sts == SFM_LAN_CONNECTED) {
			rshellResult(sysIndex, checkip[i]);
			break;
		}
			
	}
/* DEBUG: by june, 2010-10-07
 * DESC : LOG ADD
 */
#if 0
			sprintf (trcBuf, ">>>>>>> sysIdex[%d][%s] checkip[%s][%s] STS[%s]\n"
					, sysIndex
					, sfdb->sys[sysIndex].commInfo.name
					, checkip[0], checkip[1]
					, (sts==0)?"CONNECT":"DISCONNECT" 
					);
			trclib_writeLogErr (FL,trcBuf);
#endif
	if ( sts == SFM_LAN_DISCONNECTED) {		// ping  fail all
		for(j=0; j < sfdb->sys[sysIndex].commInfo.procCnt; j++){
			sfdb->sys[sysIndex].commInfo.procInfo[j].status = SFM_STATUS_DEAD;
			sfdb->sys[sysIndex].commInfo.procInfo[j].pid =  0;
			sfdb->sys[sysIndex].commInfo.procInfo[j].uptime =  0;

			if(sfdb->sys[sysIndex].commInfo.procInfo[j].prevStatus != sfdb->sys[sysIndex].commInfo.procInfo[j].status){
				fimd_hdlProcAlm (sysIndex, j, 0);
				fimd_updateSysAlmInfo (sysIndex);
				fimd_broadcastAlmEvent2Client ();
			}

			sfdb->sys[sysIndex].commInfo.procInfo[j].prevStatus = sfdb->sys[sysIndex].commInfo.procInfo[j].status;
		}
		logPrint(trcLogId,FL,"[%s] ALL PROCESS DEAD~!!\n",sfdb->sys[sysIndex].commInfo.name);
		return;
	}else{

	//	signal (SIGALRM, rshellResult_dumHdlr);	
	//	alarm(7);	
	//	fprintf(stderr, "checkip[i]: %d, %s\n", i, checkip[i]);
		dRet = rshellResult(sysIndex, checkip[i]);
	//	alarm(0);	
		if (dRet > 0){
			stsIndex[sysIndex] = 0;	
			return;	
		} 
	}

    return;
}

void review_linktest(int sysIndex)
{
    int             j,con=SFM_LAN_DISCONNECTED;
    char            command[256], fname[256], lineBuf[1024];
    FILE            *fp;

    //sprintf (fname, "/tmp/Linktest_BSD_sysIndex");
    sprintf (fname, "/tmp/Linktest_DSC_sysIndex");
    sprintf (command, "/usr/sbin/ping %s 1 > %s", sfdb->sys[sysIndex].commInfo.name, fname);
    system (command);
//    system ("sync");
    
    if ((fp = fopen (fname, "r")) == NULL) {
        sprintf (trcBuf, "[ping_test] fopen fail[%s]; err=%d(%s)\n", fname, errno, strerror(errno));
        trclib_writeLogErr (FL,trcBuf);
        return;
    }

    while (fgets (lineBuf, sizeof(lineBuf), fp) != NULL) {
        if (strstr (lineBuf, "alive") != NULL) {
            con = SFM_LAN_CONNECTED;
            break;
        }
    }
    if(fp)fclose(fp);

    if ( con == SFM_LAN_DISCONNECTED ){
        for(j=0; j < sfdb->sys[sysIndex].commInfo.procCnt; j++){
            sfdb->sys[sysIndex].commInfo.procInfo[j].status = SFM_STATUS_DEAD;
        }

    } 
    logPrint(trcLogId,FL,"[%s] ALL PROCESS DEAD~!!\n",sfdb->sys[sysIndex].commInfo.name);
    return;
}

int rshellResult(int sysIndex, char *checkip)
{
    int     i, readStart = 0, readEnd =0, procCnt = 0;
    char    *env;

    char    shellCommand[256], shellPort_file[50], getBuf[256];
    char    token[12][10];
    FILE    *fp;
	int 	dret = -1;

    memset(shellCommand, 0x00, sizeof(shellCommand));
    memset(shellPort_file, 0x00, sizeof(shellPort_file));
    memset(getBuf, 0x00, sizeof(getBuf));

    if ((env = getenv(IV_HOME)) == NULL) {
        sprintf(trcBuf, "[rshellResult] not found %s environment name\n", IV_HOME);
    	trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    sprintf(shellPort_file, "%s/DATA/shellPort_result", env);

    sprintf(shellCommand, "/usr/bin/rsh -l root %s /DSC/NEW/BIN/disprc 2> %s",
            checkip, shellPort_file);
    sprintf(shellCommand, "/DSC/SCRIPT/rsh_test.sh %s > %s", checkip, shellPort_file);
	strcpy(trcBuf, shellCommand);
	strcat(trcBuf,"\n");
   	trclib_writeLogErr (FL,trcBuf);

    dret = system(shellCommand);
	if (dret < 0) {
        sprintf(trcBuf, "[rshellResult] system command failed[%s]; err=%d(%s)\n", shellCommand, errno, strerror(errno));
    	trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

    if ((fp = fopen(shellPort_file,"r")) == NULL) {
        sprintf(trcBuf, "[rshellResult] fopen fail[%s]; err=%d(%s)\n", shellPort_file, errno, strerror(errno));
    	trclib_writeLogErr (FL,trcBuf);
        return -1;
    }

    while (fgets (getBuf, sizeof(getBuf), fp) != NULL){
        if(!strncmp(getBuf, "=======",7) && readEnd == 1){
            break;
        }
        if (!strncmp(getBuf, "-------", 7)){
            readStart = 1;
            continue;
        }
        if(readStart){
            sscanf(getBuf, " %s %s %s %s %s %s %s %s %s %s %s %s",
                  token[0], token[1], token[2], token[3], token[4], token[5], token[6], token[7],
                  token[8], token[9], token[10], token[11]);
            readEnd =1;

			if ( (!strcasecmp(token[0], "CM")) || (!strcasecmp(token[0], "SM")) 
			 ||	(!strcasecmp(token[6], "CM")) || (!strcasecmp(token[6], "SM")) ) {
				/* 관리 대상 제외 */
				continue;
			 }

            strcpy(sfdb->sys[sysIndex].commInfo.procInfo[procCnt].name, token[0]);
            if(strcasecmp(token[1], "-")){
                sfdb->sys[sysIndex].commInfo.procInfo[procCnt].pid = atoi(token[1]);
                sfdb->sys[sysIndex].commInfo.procInfo[procCnt].status = SFM_STATUS_ALIVE;

                strcpy(sfdb->sys[sysIndex].commInfo.procInfo[procCnt+1].name, token[6]);
                if(strcasecmp(token[7], "-")){
                    sfdb->sys[sysIndex].commInfo.procInfo[procCnt+1].pid = atoi(token[7]);
                    sfdb->sys[sysIndex].commInfo.procInfo[procCnt+1].status = SFM_STATUS_ALIVE;
                }else{
                    sfdb->sys[sysIndex].commInfo.procInfo[procCnt+1].status = SFM_STATUS_DEAD;
                    sfdb->sys[sysIndex].commInfo.procInfo[procCnt+1].pid = 0;
                }
            }else{
                sfdb->sys[sysIndex].commInfo.procInfo[procCnt].status = SFM_STATUS_DEAD;
                sfdb->sys[sysIndex].commInfo.procInfo[procCnt].pid = 0;
                strcpy(sfdb->sys[sysIndex].commInfo.procInfo[procCnt+1].name, token[6]);
                if(strcasecmp(token[7], "-")){
                    sfdb->sys[sysIndex].commInfo.procInfo[procCnt+1].pid = atoi(token[7]);
                    sfdb->sys[sysIndex].commInfo.procInfo[procCnt+1].status = SFM_STATUS_ALIVE;
                }
                else{
                    sfdb->sys[sysIndex].commInfo.procInfo[procCnt+1].status = SFM_STATUS_DEAD;
                    sfdb->sys[sysIndex].commInfo.procInfo[procCnt+1].pid = 0;
                }
            }
#if 0
            fprintf(stderr, ">>> RSH] sysindex:%d procName:%s  pid:%ld procName:%s pid:%ld\n"
					, sysIndex
                    , sfdb->sys[sysIndex].commInfo.procInfo[procCnt].name
					, sfdb->sys[sysIndex].commInfo.procInfo[procCnt].pid
                    , sfdb->sys[sysIndex].commInfo.procInfo[procCnt+1].name
					, sfdb->sys[sysIndex].commInfo.procInfo[procCnt+1].pid);
#endif
           	
			if(sfdb->sys[sysIndex].commInfo.procInfo[procCnt].prevStatus != sfdb->sys[sysIndex].commInfo.procInfo[procCnt].status){
/* DEBUG: by june, 2010-10-07
 * DESC : LOG ADD
 */
#if 0
				sprintf (trcBuf, "[rshellResult] sys[%d] procCnt[%d] procName[%s] status[%d:%d]\n"
						, sysIndex
						, procCnt
						, sfdb->sys[sysIndex].commInfo.procInfo[procCnt].name
						, sfdb->sys[sysIndex].commInfo.procInfo[procCnt].prevStatus
						, sfdb->sys[sysIndex].commInfo.procInfo[procCnt].status);
				trclib_writeLogErr (FL,trcBuf);
#endif
				fimd_hdlProcAlm (sysIndex, procCnt, 0);
				fimd_updateSysAlmInfo (sysIndex);
				for(i=0; i<2; i++) fimd_updatePDAlmInfo(i);
				fimd_broadcastAlmEvent2Client ();
				sfdb->sys[sysIndex].commInfo.procInfo[procCnt].prevStatus = sfdb->sys[sysIndex].commInfo.procInfo[procCnt].status;
			}
/* DEBUG: by june, 2010-10-07
 * DESC :
 */
			if(sfdb->sys[sysIndex].commInfo.procInfo[procCnt+1].prevStatus != sfdb->sys[sysIndex].commInfo.procInfo[procCnt+1].status){
#if 0
				sprintf (trcBuf, "[rshellResult] sys[%d] procCnt+1[%d] procName[%s] status[%d:%d]\n"
						, sysIndex
						, procCnt+1
						, sfdb->sys[sysIndex].commInfo.procInfo[procCnt+1].name
						, sfdb->sys[sysIndex].commInfo.procInfo[procCnt+1].prevStatus
						, sfdb->sys[sysIndex].commInfo.procInfo[procCnt+1].status);
				trclib_writeLogErr (FL,trcBuf);
#endif
				fimd_hdlProcAlm (sysIndex, procCnt+1, 0);
				fimd_updateSysAlmInfo (sysIndex);
				for(i=0; i<2; i++) fimd_updatePDAlmInfo(i);
				fimd_broadcastAlmEvent2Client ();
				sfdb->sys[sysIndex].commInfo.procInfo[procCnt+1].prevStatus = sfdb->sys[sysIndex].commInfo.procInfo[procCnt+1].status;
			}
			//sfdb->sys[sysIndex].commInfo.procInfo[procCnt].prevStatus = sfdb->sys[sysIndex].commInfo.procInfo[procCnt].status;
			//sfdb->sys[sysIndex].commInfo.procInfo[procCnt+1].prevStatus = sfdb->sys[sysIndex].commInfo.procInfo[procCnt+1].status;
			procCnt = procCnt + 2;
        }
    }

    if(fp)fclose(fp);
    return 1;
}

void rshellResult_dumHdlr (int signo)
{
	int             i;
	char            command[256], fname[256], getBuf[256], token[3][10];
	FILE            *fp;	

	sprintf (fname, "/tmp/rsh_stat");
	sprintf (command, "lsof -i |grep TCP|grep 5186 > %s", fname);
	system (command);
//	system ("sync");	

	if ((fp = fopen (fname, "r")) == NULL) {
    		sprintf (trcBuf, "[rsh_stat] fopen fail[%s]; err=%d(%s)\n", fname, errno, strerror(errno));
    		trclib_writeLogErr (FL,trcBuf);
    		return;
	}

	while (fgets (getBuf, sizeof(getBuf), fp) != NULL){

		sscanf(getBuf, " %s %s %s ", token[0], token[1], token[2]);
		
		if (strcasecmp(token[0], "rsh")) continue;

		if(!strcasecmp(token[0], "rsh")){
			sprintf (command, "kill -9 %s", token[1]);
			system (command);
			//system ("sync");	
			//sprintf(trcBuf,"kill rsh because BSD hang \n");
			sprintf(trcBuf,"kill rsh because DSC hang \n");
			trclib_writeLogErr (FL,trcBuf);	
			break;	
		}
	}

	if(fp)fclose(fp);

	for(i = 0; i < SYSCONF_MAX_ASSO_SYS_NUM; i++){	
		stsIndex[i] = 1;
	}

	//sprintf(trcBuf,"[rshellResult_dumHdlr] BSD Connect hang \n");
	sprintf(trcBuf,"[rshellResult_dumHdlr] DSC Connect hang \n");
	trclib_writeLogErr (FL,trcBuf);
	return ;
}



