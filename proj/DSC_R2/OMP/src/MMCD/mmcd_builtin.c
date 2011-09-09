#include "mmcd_proto.h"
#include <crypt.h>

extern int		ixpcQid, mmcLogId, MML_NUM_CMD, msgId4Nmsib;
extern int      	overloadFlag;
extern time_t		currentTime;
extern char		trcBuf[4096], trcTmp[1024];
extern char		mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN], sysLabel[COMM_MAX_NAME_LEN];
extern MMLCmdContext		*mmlCmdTbl;
extern MMLHelpContext		*mmlHelpTbl;
extern MmcdJobTblContext	*mmcdJobTbl;
extern MmcdUserTblContext	*mmcdUserTbl;
extern MmcdCliTblContext	*mmcdCliTbl;

extern MMcdUserIPTblContext	*mmcdIPTbl;		// 2009.07.17 by sjs

extern MmcdBuiltInCmdVector	mmcdBuiltInCmdVector[MMCD_MAX_BUILTIN_CMD];
extern int			trcFlag, trcLogFlag;
extern char			inputErrBuf[1024];

MmcdBuiltInCmdVector	mmcdBuiltInCmdVector[MMCD_MAX_BUILTIN_CMD] =
{
    {"log-in",				mmcd_builtin_log_in},
    {"log-out",				mmcd_builtin_log_out},
    {"rebuild-mml-tbl",		mmcd_builtin_rebuild_mml_tbl},
    {"dis-usr-info",		mmcd_builtin_dis_usr_info},
    {"add-usr",				mmcd_builtin_add_usr},
    {"canc-usr",			mmcd_builtin_canc_usr},
    {"del-usr",				mmcd_builtin_del_usr},
    {"dis-exe-cmd",			mmcd_builtin_dis_exe_cmd},
    {"dis-help",			mmcd_builtin_cmd_help},
    {"dis-cls-lst",			mmcd_builtin_grade_help},
    {"chg-pwd",				mmcd_builtin_chg_passwd},
    {"dis-cur-usr",			mmcd_builtin_dis_cur_usr},
    {"dis-his",				mmcd_builtin_dis_cmd_his},
    {"canc-exe-cmd",		mmcd_builtin_canc_exe_cmd},
    {"stat-cmd-canc",		mmcd_builtin_stat_cmd_canc},
    {"cmd-help",			mmcd_builtin_cmd_help},
    {"add-ipaddr",			mmcd_builtin_add_ipaddr},
    {"del-ipaddr",			mmcd_builtin_del_ipaddr},
	{"dis-ipaddr-info", 	mmcd_builtin_dis_ipaddr_info}
//    {"alive", 				mmcd_builtin_heart_beat}
};
int	MMCD_NUM_BUILTIN_CMD=25;


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int mmcd_isBuiltInCmd (MMLInputCmdInfo *inputCmdInfo)
{
    int	i, cmdIndex, size;
    char	resBuf[4096];
    MmcdBuiltInCmdVector	*builtin;
    
    if( ( builtin = ( MmcdBuiltInCmdVector* ) bsearch ( inputCmdInfo->cmdName,
             mmcdBuiltInCmdVector, MMCD_NUM_BUILTIN_CMD,
             sizeof( MmcdBuiltInCmdVector ), mmcd_bsrchCmp ) ) == NULL ) 
	{
        return 0;
    }
    
    // client�� Accept �޽����� ������. log-in, log-out�� ������ �ʴ´�.
    if (strcasecmp (inputCmdInfo->cmdName, "LOG-IN") &&
        strcasecmp (inputCmdInfo->cmdName, "LOG-OUT") )
    {
        mmcd_sendInputAccepted2Client (inputCmdInfo);
    }
    
    /* DIS-HLEP */
    if (!strcasecmp (inputCmdInfo->cmdName, "DIS-HELP") ) 
    { 
        // ��ɾ� �̸��� ������ ��ġ�ϴ� ���� �ִ��� ���� ã�ƺ���.
        if ((cmdIndex = mmcd_getCmdIndex (inputCmdInfo->paraInfo[0].paraVal)) >= 0) 
        {
            int byte=0,j,send=0;
            
            size = strlen(mmlHelpTbl[cmdIndex].cmdHelp);
            for (i=0; i<size; i++)
            {
                resBuf[byte] = mmlHelpTbl[cmdIndex].cmdHelp[i];
                byte++;	
                
                /* 
                ** 3500 byte�̻��̸� \n�� ���ö����� ã�Ƽ� ������
                ** \n�� ������ 4000byte������ �о� ������.
                */
                if ( byte > 3500 )
                {
                    send=0;
                    for (j=1; j<500; j++)
                    {
                        resBuf[byte] = mmlHelpTbl[cmdIndex].cmdHelp[i+j];
                        byte++;
                        if (  mmlHelpTbl[cmdIndex].cmdHelp[i+j] == '\n' ) 
                        {
                            resBuf[byte]=NULL;
                            mmcd_builtin_send_result (resBuf, 0, 1, inputCmdInfo);
                            byte=0;	
                            resBuf[byte]='\n';
                            byte++;
                            send=1;
                            break;
                        }
                    }
                    /* \n�� ������ */
                    if ( send == 0 )
                    {
                        resBuf[byte]=NULL;
                        mmcd_builtin_send_result (resBuf, 0, 1, inputCmdInfo);
                        byte=0;	
                        resBuf[byte]='\n';
                        byte++;
                    }
                    i = i+j;
                }
            }
            resBuf[byte]=NULL;
            mmcd_builtin_send_result (resBuf, 0, 0, inputCmdInfo);
            
            return 1;
        }
        else 
        {
            sprintf(inputErrBuf,"NOT_REGISTERED_COMMAND");
            mmcd_sendInputError2Client(inputCmdInfo);
        }
        return 1;
    }

    // ó�� function�� ȣ���Ѵ�.
    //
    (int)(*(builtin->func)) (inputCmdInfo);
    
    return 1;
} //----- End of mmcd_isBuiltInCmd -----//



//------------------------------------------------------------------------------
// client�� socket ���� �� �� ó�� ������ log-in ������ �����Ѵ�.
// - user table�� Ȯ���Ͽ� ��ϵ� userName�� passwd�� Ȯ���Ѵ�.
// - Ȯ�� �� client table�� �߰��ϰ� user tabel�� login-time ���� update�Ѵ�.
//------------------------------------------------------------------------------
int mmcd_builtin_log_in (
    MMLInputCmdInfo *inputCmdInfo	// �Էµ� ��ɾ� ������ ����ִ�.
    )
{
    int			userIndex, k, ipIndex;
    char		welcome[64];
//    unsigned int 	connIP;
    char 		ConnIP[15];
    
    // user_name�� �Է����� ���� ���

#if 0 /*debug*/ 
    fprintf(stderr, "\nID=[%s], [%s]", inputCmdInfo->paraInfo[0].paraVal, inputCmdInfo->paraInfo[1].paraVal);
#endif
//        connIP = GetClientIPhl( inputCmdInfo->cliSockFd );
    
    memset( ConnIP, 0, 15 );
    strcpy( ConnIP, GetClientIPstr( inputCmdInfo->cliSockFd ) );

    ipIndex = mmcd_check_ipaddr( ConnIP );
    if( ipIndex < 0 )
    {
        sprintf(trcBuf,"[mmcd_builtin_log_in] ipaddress missing\n");
        trclib_writeLog (FL, trcBuf);
        inputCmdInfo->errorCode = 7;
        return (mmcd_builtin_send_result ("\n        IPADDRESS_MISSING\n", -1, 0, inputCmdInfo));
    }

    if(!strlen(inputCmdInfo->paraInfo[0].paraVal)) 
    {
        sprintf(trcBuf,"[mmcd_builtin_log_in] username missing\n");
        trclib_writeLog (FL, trcBuf);
        inputCmdInfo->errorCode = 1;
        return (mmcd_builtin_send_result ("\n        USERNAME_MISSING\n", -1, 0, inputCmdInfo));
    }
    
    /* jhnoh : 030916 : limit of passwd length */
    // passwd�� �Է����� ���� ���
    if (strlen(inputCmdInfo->paraInfo[1].paraVal) > 15) 
    {
        sprintf(trcBuf,"[mmcd_builtin_log_in] password missing\n");
        fprintf (stderr," passwd=%s\n", inputCmdInfo->paraInfo[1].paraVal );
        trclib_writeLog (FL, trcBuf);
        inputCmdInfo->errorCode = 2;
        return (mmcd_builtin_send_result ("\n        PASSWORD_IS_TOO_LONG\n", -1, 0, inputCmdInfo));
    }
    
    // passwd�� �Է����� ���� ���
    if (!strlen(inputCmdInfo->paraInfo[1].paraVal)) 
    {
        sprintf(trcBuf,"[mmcd_builtin_log_in] password missing\n");
        trclib_writeLog (FL, trcBuf);
        inputCmdInfo->errorCode = 3;
        return (mmcd_builtin_send_result ("\n        PASSWORD_MISSING\n", -1, 0, inputCmdInfo));
    }
    
    // if not registered user 
    if ((userIndex = mmcd_getUserIndex (inputCmdInfo->paraInfo[0].paraVal)) < 0) {
        sprintf(trcBuf,"[mmcd_builtin_log_in] unknown user; userName=%s\n", inputCmdInfo->paraInfo[0].paraVal);
        trclib_writeLog (FL, trcBuf);
        inputCmdInfo->errorCode = 4;
        return (mmcd_builtin_send_result ("\n        NOT_REGISTERD_USER\n", -1, 0, inputCmdInfo));
    }
    
    if ( strcasecmp(inputCmdInfo->paraInfo[0].paraVal,OMDSYS_USER_NAME) &&
         mmcdUserTbl[userIndex].loginCnt != 0 &&
         strcasecmp(inputCmdInfo->paraInfo[0].paraVal, "yy")){		//yhshin
        
        sprintf(trcBuf,"[mmcd_builtin_log_in] the same user is log-in already\n");
        trclib_writeLog (FL, trcBuf);
        inputCmdInfo->errorCode = 5;
        return (mmcd_builtin_send_result ("\n        ALREADY THE SAME USER LOGINED\n", -1, 0, inputCmdInfo));
    }	
    
    // password error
    if (strcmp (mmcdUserTbl[userIndex].passwd,
                (char*)crypt(inputCmdInfo->paraInfo[1].paraVal, inputCmdInfo->paraInfo[1].paraVal))) {
        sprintf(trcBuf,"[mmcd_builtin_log_in] mismatch passwd\n");
        trclib_writeLog (FL, trcBuf);
        inputCmdInfo->errorCode = 6;
        return (mmcd_builtin_send_result ("\n        PASSWORD_MISMATCH\n", -1, 0, inputCmdInfo));
    }
    
    for (k=0; k<MML_NUM_TP_CLIENT_TBL; k++) {
        if (mmcdCliTbl[k].cliSockFd != inputCmdInfo->cliSockFd)
            continue;
        mmcd_builtin_log_out(inputCmdInfo);
    }
    
    // check cpu overload
    // read SFDB if CPU overload occured
    
    //if(overloadFlag && (mmcdUserTbl[userIndex].privilege !=1)) {
    if(overloadFlag && (mmcdUserTbl[userIndex].privilege > 1)) {
        sprintf(trcBuf,"[mmcd_builtin_log_in] access denied.. in cpu overload\n");
        trclib_writeLog (FL, trcBuf);
        return (mmcd_builtin_send_result ("\n\n    BYE...\n    DISCONNECTED IN OVERLOAD\n    (SUPER USER ONLY)\n", -1, 0, inputCmdInfo));
    }
    
    // insert client table
    
    for (k=0; k<MML_NUM_TP_CLIENT_TBL; k++) 
    {
        if( mmcdCliTbl[k].cliSockFd )
            continue;
        mmcdCliTbl[k].cliSockFd = inputCmdInfo->cliSockFd;
        strcpy (mmcdCliTbl[k].userName, inputCmdInfo->paraInfo[0].paraVal);
        mmcdCliTbl[k].privilege = mmcdUserTbl[userIndex].privilege;
        mmcdCliTbl[k].nmsibFlag = mmcdUserTbl[userIndex].nmsibFlag;
        mmcdCliTbl[k].userIndex = userIndex;

        mmcdCliTbl[k].useIPListIndex = ipIndex;		//2009.07.16 by sjs
        mmcdIPTbl[ipIndex].useFlag = 1;			//	ip����Ʈ�� ��������� ǥ��
        if( inputCmdInfo->clientType == 1 )		
        {
            mmcdCliTbl[k].lastHeartBeatTime = currentTime;
            sprintf( trcBuf, "[mmcd_builtin_log_in] currentTime = %d lastHeartBeatTime = %d\n", 
                     ( int )currentTime, ( int )mmcdCliTbl[k].lastHeartBeatTime );
            trclib_writeLog (FL, trcBuf);
        }
        break;
    }
    
    // update user table
    //(mmcdUserTbl[userIndex].loginCnt)++;
    mmcdUserTbl[userIndex].loginCnt = 1; // yhshin
    mmcdUserTbl[userIndex].lastLoginTime = currentTime;
//        mmcdUserTbl[userIndex].connIP = connIP;		//2009.07.16 by sjs
    strcpy( mmcdUserTbl[userIndex].ConnIP, ConnIP );		//2009.07.16 by sjs
    
    // lastLoginTime�� passwd file�� �ݿ��ϱ� ����
    mmcd_savePasswdFile();
    
    sprintf(trcBuf,"[mmcd_builtin_log_in] user log-in; user=%s, fd=%d, ip = %s\n",
            mmcdUserTbl[userIndex].userName, inputCmdInfo->cliSockFd, mmcdUserTbl[userIndex].ConnIP );
    trclib_writeLogErr (FL, trcBuf);
    
#if 1 /* jhnoh : 030813 */
    inputCmdInfo->nmsibFlag = mmcdUserTbl[userIndex].nmsibFlag;
#endif
    
#if 1 /* jhnoh : 030813  to eliminate "WELCOME" message */
    strcat(inputCmdInfo->inputString, "?");
#endif
    
    sprintf ( welcome,"\n    WELCOME %s USER\n",mmcd_printUserFullClass(mmcdUserTbl[userIndex].privilege) );
    // client�� ����� ������.
    return (mmcd_builtin_send_result (welcome, 0, 0, inputCmdInfo));
    
} //----- End of mmcd_builtin_log_in -----//

// client�κ��� log-out �޽����� ���� ��� ȣ��Ǿ� log-out ������ �����Ѵ�.
// - �ش� client�� ���� �������� job�� ������ application���� cancel �޽����� ������
//	job table���� �����Ѵ�.
// -  client table������ �����Ѵ�.
//		-> log-in�� client table�� �߰��ǰ� log-out�Ǹ� �����ȴ�.
//			socket connection ����/���ܵǴ� ������ �ƴϴ�.
//------------------------------------------------------------------------------
int mmcd_builtin_log_out (
		MMLInputCmdInfo *inputCmdInfo	// �Էµ� ��ɾ� ������ ����ִ�.
		)
{
    int		cliIndex,userIndex,jobNo,ret;
    
    if (inputCmdInfo->clientType != 2) {
        // application���� cancel �޽����� ������ job table�� �����Ѵ�.
        for (jobNo=0; jobNo<MML_NUM_TP_JOB_TBL; jobNo++) {
            if (mmcdJobTbl[jobNo].tpInd &&
                mmcdJobTbl[jobNo].cliSockFd == inputCmdInfo->cliSockFd) {
                mmcd_sendCancMsg2App (jobNo);
                memset ((void*)&mmcdJobTbl[jobNo], 0, sizeof(MmcdJobTblContext));
            }
        }
        
        // search client table
        if ((cliIndex = mmcd_getCliIndex (inputCmdInfo->cliSockFd)) < 0) {
            sprintf(trcBuf,"[mmcd_builtin_log_out] mmcd_getCliIndex fail; fd=%d\n", inputCmdInfo->cliSockFd);
            trclib_writeLog (FL, trcBuf);
            return (mmcd_builtin_send_result ("    INTERNAL_ERROR (mmcd_getCliIndex fail)\n", -1, 0, inputCmdInfo));
        }
        
        userIndex = mmcdCliTbl[cliIndex].userIndex;
        
        // update user table
        (mmcdUserTbl[userIndex].loginCnt)--;
        mmcdUserTbl[userIndex].lastLogoutTime = currentTime;
        
        // lastLogoutTime�� passwd file�� �ݿ��ϱ� ����
        mmcd_savePasswdFile();

        mmcdIPTbl[mmcdCliTbl[cliIndex].useIPListIndex].useFlag = 0;			//	ip����Ʈ off
        
        // client�� ����� ������.
        ret = mmcd_builtin_send_result ("\n    BYE...\n", 0, 0, inputCmdInfo);
	
        // delete client table
        memset ((void*)&mmcdCliTbl[cliIndex], 0, sizeof(MmcdCliTblContext));
        
        sprintf(trcBuf,"[mmcd_builtin_log_in] user log-out; user=%s, fd=%d\n",
                mmcdUserTbl[userIndex].userName, inputCmdInfo->cliSockFd);
        trclib_writeLogErr (FL, trcBuf);
    }
    else 
    {
        // application���� cancel �޽����� ������ job table�� �����Ѵ�.
        for (jobNo=0; jobNo<MML_NUM_TP_JOB_TBL; jobNo++) {
            if (mmcdJobTbl[jobNo].tpInd &&
                mmcdJobTbl[jobNo].cliSockFd == inputCmdInfo->cliSockFd) {
                mmcd_sendCancMsg2App (jobNo);
                memset ((void*)&mmcdJobTbl[jobNo], 0, sizeof(MmcdJobTblContext));
            }
        }
        
        // search client table
        if ((cliIndex = mmcd_getCliIndex (inputCmdInfo->cliSockFd)) < 0) {
            sprintf(trcBuf,"[mmcd_builtin_log_out] mmcd_getCliIndex fail; fd=%d\n", inputCmdInfo->cliSockFd);
            trclib_writeLog (FL, trcBuf);
            return (mmcd_builtin_send_result ("    INTERNAL_ERROR (mmcd_getCliIndex fail)\n", -1, 0, inputCmdInfo));
        }
        
        userIndex = mmcdCliTbl[cliIndex].userIndex;
        
        // client�� ����� ������.
        ret = mmcd_builtin_send_result ("\n    BYE...\n", 0, 0, inputCmdInfo);
	
        // delete client table
        memset ((void*)&mmcdCliTbl[cliIndex], 0, sizeof(MmcdCliTblContext));
    }
    return ret;
} //----- End of mmcd_builtin_log_out -----//


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int mmcd_builtin_rebuild_mml_tbl (MMLInputCmdInfo *inputCmdInfo)
{
    int		newCmdCnt, cmdIndex;
    char	resBuf[1024], tmpBuf[256], errBuf[256];
    
    cmdIndex = inputCmdInfo->cmdIndex;
    sprintf(resBuf, "\n    %s %s %s\n    %s\n",
            sysLabel, commlib_printTStamp(), inputCmdInfo->userName,
            mmlHelpTbl[cmdIndex].cmdSlogan);
    
    if ((newCmdCnt = mmcd_rebuildCmdTbl (errBuf)) < 0) {
        sprintf(tmpBuf,"    RESULT = FAIL\n    %s\n", errBuf);
        strcat (resBuf,tmpBuf);
    } else {
        sprintf(tmpBuf,"    RESULT = SUCCESS\n    REBUILT %d COMMANDs SUCCESSFULLY\n    COMPLETED\n\n", newCmdCnt);
        strcat (resBuf,tmpBuf);
    }
    
    // dis-msg-his mmc ������� ��ȸ�� log file�� ����Ѵ�.
    logPrint (mmcLogId,FL, "%s\n", resBuf);
    
    return (mmcd_builtin_send_result (resBuf, 0, 0, inputCmdInfo));
    
} //----- End of mmcd_builtin_rebuild_mml_tbl -----//



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int mmcd_builtin_cmd_help (MMLInputCmdInfo *inputCmdInfo)
{
    int		i,cmdIndex,matchCnt=0,size;
    char	resBuf[4096],tmp[128],cmdName[32],tmpBuf[32],*ptr;
    
    
    cmdIndex = inputCmdInfo->cmdIndex;
    sprintf(resBuf, "\n    %s %s %s\n    %s\n",
            sysLabel, commlib_printTStamp(), inputCmdInfo->userName,
            mmlHelpTbl[cmdIndex].cmdSlogan);	
    
    if (inputCmdInfo->nmsibFlag) {
        mmcd_sendInputAccepted2Client (inputCmdInfo);
    }
    
    // '?'�� �� ������ �κ��� �߶󳽴�.
    ptr = strstr(inputCmdInfo->cmdName, "?");
    
    if (ptr == NULL) {
        // dis ?�� �������� ?�� cmd�� �и����� para�κп� ���� ���
        strcpy(cmdName, inputCmdInfo->cmdName);
    } else if (inputCmdInfo->cmdName[0] == '?') {
        // ?dis �� �������� ?�� �տ� ���� ��� -> ? ������ cmd�� �Ϻη� �߶󳽴�.
        strcpy (cmdName, ptr+1);
    } else {
        // dis? �� �������� ?�� �ڿ� ���� ��� -> ? ������ cmd�� �Ϻη� �߶󳽴�.
        strncpy(cmdName, inputCmdInfo->cmdName, (ptr - inputCmdInfo->cmdName));
        cmdName[ptr - inputCmdInfo->cmdName] = 0;
    }
    
    // ��ɾ� �̸��� ������ ��ġ�ϴ� ���� �ִ��� ���� ã�ƺ���.
    if ((cmdIndex = mmcd_getCmdIndex (cmdName)) >= 0) {
        
        int byte=0,j,send=0;
        // help_table�� �ִ� �ش� ��ɾ��� help�� ������.
        // ������ ¥���� help�� ���� �� �¾Ƽ� 
        
        size = strlen(mmlHelpTbl[cmdIndex].cmdHelp);
        resBuf[0]='\n';
        byte++;
        for (i=0; i<size; i++){
            
            
            ///		if ((!strncmp(cmdName, "set-rule-sce", 12)) || (!strncmp(cmdName, "SET-RULE-SCE", 12))) {
            //			return;
            //		}
            if(!strncasecmp(cmdName, "set-rule-sce", 12))
                return 0;  //warnning
            
            resBuf[byte] = mmlHelpTbl[cmdIndex].cmdHelp[i];
            byte++;	
            
            /* 
            ** 3500 byte�̻��̸� \n�� ���ö����� ã�Ƽ� ������
            ** \n�� ������ 4000byte������ �о� ������.
            */
            if ( byte > 3500 ){
                send=0;
                for (j=1; j<500; j++){
                    resBuf[byte] = mmlHelpTbl[cmdIndex].cmdHelp[i+j];
                    byte++;
                    if (  mmlHelpTbl[cmdIndex].cmdHelp[i+j] == '\n' ) {
                        resBuf[byte]=NULL;
                        mmcd_builtin_send_result (resBuf, 0, 1, inputCmdInfo);
                        byte=0;	
                        resBuf[byte]='\n';
                        byte++;
                        send=1;
                        break;
                    }
                }
                /* \n�� ������ */
                if ( send == 0 ){
                    resBuf[byte]=NULL;
                    mmcd_builtin_send_result (resBuf, 0, 1, inputCmdInfo);
                    byte=0;	
                    resBuf[byte]='\n';
                    byte++;
                }
                i = i+j;
            }
        }
        
        resBuf[byte]=NULL;
        mmcd_builtin_send_result (resBuf, 0, 0, inputCmdInfo);
        return 1;
    }
    
    // ��ҹ��� �������� �ʰ� ���ϱ� ����
    for (i=0; i<strlen(cmdName); i++)
        cmdName[i] = toupper(cmdName[i]);
    
    // ��ɾ� �̸��� �Ϻΰ� ��ġ�ϴ� ��� ��ɾ� ����Ʈ�� ������.
    strcpy (resBuf, "\n    ");
    
    for (cmdIndex=0; cmdIndex < MML_NUM_CMD; cmdIndex++)
    {
        // ��ҹ��� �������� �ʰ� ���ϱ� ����
        for (i=0; i<strlen(mmlCmdTbl[cmdIndex].cmdName); i++)
            tmpBuf[i] = toupper(mmlCmdTbl[cmdIndex].cmdName[i]);
        tmpBuf[i] = 0;
        
        if (strstr(mmlCmdTbl[cmdIndex].cmdName, cmdName) == NULL &&
            strstr(tmpBuf, cmdName) == NULL)
            continue;
        if (!strcasecmp(mmlCmdTbl[cmdIndex].cmdName, "log-in"))
            continue;
            
        if (!strcasecmp(mmlCmdTbl[cmdIndex].cmdName, "set-rule-sce")) {
            continue;
        }
            
        // ��ɾ� �̸��� �빮�� 
        sprintf(tmp, "%-23s  ", tmpBuf );
//		sprintf(tmp, "%-16s  ", mmlCmdTbl[cmdIndex].cmdName);
        
        strcat (resBuf, tmp);
        if (++matchCnt%4==0) strcat (resBuf, "\n    ");
        
        if (matchCnt%120==0) { // 30��(4*30=120)�� ������ ������.
            if (mmcd_builtin_send_result (resBuf, 0, 1, inputCmdInfo) < 0)
                return -1;
            strcpy (resBuf, "\n    "); 
            commlib_microSleep(5000);
        }
    }
    
    if (matchCnt == 0) {
        strcat (resBuf,"NOT_FOUND_MATCHED_COMMAND");
    }
    
    strcat (resBuf, "\n");
    return (mmcd_builtin_send_result (resBuf, 0, 0, inputCmdInfo));
    
} //----- End of mmcd_builtin_cmd_help -----//



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int mmcd_builtin_dis_usr_info (MMLInputCmdInfo *inputCmdInfo)
{
    int		userIndex,cmdIndex;
    char	resBuf[8192],tmp[128];
    
    cmdIndex = inputCmdInfo->cmdIndex;
    sprintf(resBuf, "\n    %s %s %s\n    %s\n",
            sysLabel, commlib_printTStamp(), inputCmdInfo->userName,
            mmlHelpTbl[cmdIndex].cmdSlogan);
    strcat (resBuf,"    RESULT = SUCCESS\n");
	strcat (resBuf,"    ==========================================================================================================\n");
	strcat (resBuf,"    USER              NAME              CLASS     LOGIN        LAST_LOGIN       LAST_LOGOUT                 IP\n");
	strcat (resBuf,"    ==========================================================================================================\n");
//    strcat (resBuf,"    =======================================================================================\n");
//    strcat (resBuf,"    USER              NAME              CLASS     LOGIN        LAST_LOGIN       LAST_LOGOUT\n");
//    strcat (resBuf,"    =======================================================================================\n");
    
    
    for (userIndex=1; userIndex < MML_NUM_TP_USER_TBL; userIndex++) 
    {
        if (strcmp (mmcdUserTbl[userIndex].userName, "")) 
        {
#if 1 /* jhnoh : 030819 */
            if (strncmp(mmcdUserTbl[userIndex].userName, "ROOT", 4) && 
                strncmp(mmcdUserTbl[userIndex].userName, "INS_MNG", 7)) 
            {
				if( mmcdUserTbl[userIndex].name[0] == 0 )
				{
					strcpy( mmcdUserTbl[userIndex].name, "Unknown" ); 
				}
				if( mmcdUserTbl[userIndex].loginCnt == 0 )
				{
					strcpy( mmcdUserTbl[userIndex].ConnIP, "Not_Connected" );
				}
#endif
                //sprintf(tmp,"  %-16s  %-8s  %-8d  %-13s  %-13s %-16s\n", -->�ѹ��� �ι� call�ϸ� �ڿ� ������ ���� ������.
                sprintf(tmp,"    %-16s  %-16s  %5s  %8d  %16s",
                        mmcdUserTbl[userIndex].userName, mmcdUserTbl[userIndex].name,
                        mmcd_printUserClass(mmcdUserTbl[userIndex].privilege),
                        mmcdUserTbl[userIndex].loginCnt,
                        mmcd_printTimeMMDDHHMMSS(mmcdUserTbl[userIndex].lastLoginTime) );
                strcat (resBuf, tmp);
	            sprintf(tmp,"  %16s", mmcd_printTimeMMDDHHMMSS(mmcdUserTbl[userIndex].lastLogoutTime));
                strcat (resBuf, tmp);
				sprintf(tmp,"  %16s\n", mmcdUserTbl[userIndex].ConnIP );
				strcat (resBuf, tmp);
            }
        }
    }

	strcat (resBuf,"    ==========================================================================================================\n");
//    strcat (resBuf,"    =======================================================================================\n");
    strcat (resBuf,"    COMPLETED\n\n");
    
    // dis-msg-his mmc ������� ��ȸ�� log file�� ����Ѵ�.
    logPrint (mmcLogId,FL, "%s\n", resBuf);
#ifdef DEBUG
	sprintf(trcBuf," === dis-usr-info result ===\n>>>> strlen=%d \n%s\n", strlen(resBuf), resBuf);
	trclib_writeLogErr (FL,trcBuf);
#endif
    
    // client�� ��� �޽����� ������.
    return (mmcd_builtin_send_result (resBuf, 0, 0, inputCmdInfo));
    
} //----- End of mmcd_builtin_dis_usr_info -----//


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int mmcd_builtin_add_usr (MMLInputCmdInfo *inputCmdInfo)
{
    int		userIndex,cmdIndex;
    char	resBuf[4096],tmp[128],userName[32],passwd[32], name[32];
    
    cmdIndex = inputCmdInfo->cmdIndex;
    sprintf(resBuf, "\n    %s %s %s\n    %s\n",
            sysLabel, commlib_printTStamp(), inputCmdInfo->userName,
            mmlHelpTbl[cmdIndex].cmdSlogan);
    
    // get parameters (name, passwd)
    strcpy (userName, inputCmdInfo->paraInfo[0].paraVal);
    strcpy (passwd, inputCmdInfo->paraInfo[1].paraVal);
    strcpy( name, inputCmdInfo->paraInfo[3].paraVal );
    
    sprintf(tmp, "    ID = %s NAME   = %s\n", userName, name);
    strcat (resBuf, tmp);
    
    // �̹� ��ϵ� user���� Ȯ���Ѵ�.
    if (mmcd_getUserIndex (userName) >= 0) 
    {
        strcat (resBuf,"    RESULT = FAIL\n    REASON = ALREADY_EXIST_USER\n    COMPLETED\n\n");
        logPrint (mmcLogId,FL, "%s\n", resBuf);
        return (mmcd_builtin_send_result (resBuf, -1, 0, inputCmdInfo));
    }
    
    // user table�� �߰��Ѵ�.
    for (userIndex=0; userIndex < MML_NUM_TP_USER_TBL; userIndex++) 
    {
        if (!strcmp (mmcdUserTbl[userIndex].userName, "")) 
        {
            strcpy (mmcdUserTbl[userIndex].userName, userName);
            strcpy( mmcdUserTbl[userIndex].name, name );
            strcpy (mmcdUserTbl[userIndex].passwd, (char*)crypt(passwd,passwd));
            mmcdUserTbl[userIndex].privilege = mmcd_setCmdClass2CmdTbl(inputCmdInfo->paraInfo[2].paraVal);
            break;
        }
    }
    
    // user table full ���� Ȯ��.
    if (userIndex >= MML_NUM_TP_USER_TBL) 
    {
        strcat (resBuf,"    RESULT = FAIL\n    REASON = USER TABLE is FULL(MAX=32)\n    COMPLETED\n\n");
        logPrint (mmcLogId,FL, "%s\n", resBuf);
        return (mmcd_builtin_send_result (resBuf, -1, 0, inputCmdInfo));
    }
    
    // passwd ���Ͽ� �����Ѵ�.
    mmcd_savePasswdFile();
    
    strcat (resBuf,"    RESULT = SUCCESS\n    COMPLETED\n\n");
    
    // dis-msg-his mmc ������� ��ȸ�� log file�� ����Ѵ�.
    logPrint (mmcLogId,FL, "%s \n>>> strlen=%d\n", resBuf, strlen(resBuf));
    
    // client�� ��� �޽����� ������.
    return (mmcd_builtin_send_result (resBuf, 0, 0, inputCmdInfo));
    
} //----- End of mmcd_builtin_add_usr -----//

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int mmcd_builtin_del_usr (MMLInputCmdInfo *inputCmdInfo)
{
    int		userIndex, cmdIndex;
    char	resBuf[4096],tmp[128],userName[32];
    
    cmdIndex = inputCmdInfo->cmdIndex;
    sprintf(resBuf, "\n    %s %s %s\n    %s\n",
            sysLabel, commlib_printTStamp(), inputCmdInfo->userName,
            mmlHelpTbl[cmdIndex].cmdSlogan);
    
    // get parameters (name)
    strcpy (userName, inputCmdInfo->paraInfo[0].paraVal);
    
    sprintf(tmp, "    NAME   = %s\n", userName);
    strcat (resBuf, tmp);
    
    // ��ϵǾ� �ִ� user���� Ȯ���Ѵ�.
    if ((userIndex = mmcd_getUserIndex (inputCmdInfo->paraInfo[0].paraVal)) < 0) 
    {
        strcat (resBuf,"    RESULT = FAIL\n    REASON = NOT_REGISTERED_USER\n    COMPLETED\n\n");
        logPrint (mmcLogId,FL, "%s\n", resBuf);
        return (mmcd_builtin_send_result (resBuf, -1, 0, inputCmdInfo));
    }
    else 
    {
        if (!(strncmp(inputCmdInfo->paraInfo[0].paraVal, "ROOT", 4)))
        {
            strcat (resBuf,"    RESULT = FAIL\n    REASON = CANNOT DELETE ROOT USER\n    COMPLETED\n\n");
            logPrint (mmcLogId,FL, "%s\n", resBuf);
            return (mmcd_builtin_send_result (resBuf, -1, 0, inputCmdInfo));
        }
        if (!(strncmp(inputCmdInfo->paraInfo[0].paraVal, "INS_MNG", 7)))
        {
            strcat (resBuf,"    RESULT = FAIL\n    REASON = CANNOT DELETE NMSIB USER\n    COMPLETED\n\n");
            logPrint (mmcLogId,FL, "%s\n", resBuf);
            return (mmcd_builtin_send_result (resBuf, -1, 0, inputCmdInfo));
        }
    }

    // ���� log-in�Ǿ� �ִ��� Ȯ���Ѵ�.
    if (mmcdUserTbl[userIndex].loginCnt) 
    {
        strcat (resBuf,"    RESULT = FAIL\n    REASON = USER_LOGIN_STATE\n    COMPLETED\n\n");
        logPrint (mmcLogId,FL, "%s\n", resBuf);
        return (mmcd_builtin_send_result (resBuf, -1, 0, inputCmdInfo));
    }
    
    // user table���� �����Ѵ�.
    memset ((void*)&mmcdUserTbl[userIndex], 0, sizeof(MmcdUserTblContext));
    
    // passwd ���Ͽ� �����Ѵ�.
    mmcd_savePasswdFile();
    
    strcat (resBuf,"    RESULT = SUCCESS\n    COMPLETED\n\n");
    
    // dis-msg-his mmc ������� ��ȸ�� log file�� ����Ѵ�.
    logPrint (mmcLogId,FL, "%s\n", resBuf);
    
    // client�� ��� �޽����� ������.
    return (mmcd_builtin_send_result (resBuf, 0, 0, inputCmdInfo));
    
} //----- End of mmcd_builtin_del_usr -----//


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int mmcd_builtin_chg_passwd (MMLInputCmdInfo *inputCmdInfo)
{
    int     userIndex, cmdIndex, chg_own;
    char    resBuf[4096], userName[32], oldpswd[32], newpswd[32];

    cmdIndex = inputCmdInfo->cmdIndex;
    sprintf(resBuf, "\n    %s %s %s\n    %s\n",
            sysLabel, commlib_printTStamp(), inputCmdInfo->userName,
            mmlHelpTbl[cmdIndex].cmdSlogan);

    // get parameters (userName,passwd)
    if (strcmp (inputCmdInfo->paraInfo[0].paraVal, "")) {
        strcpy (userName, inputCmdInfo->paraInfo[0].paraVal);
        if (!strcmp(userName, inputCmdInfo->userName))
            chg_own = 1;
        else
            chg_own = 0;
    } else {
        // �Է����� �ʾ����� �ڽ��� ���� �ٲ۴�.
        chg_own = 1;
        strcpy (userName, inputCmdInfo->userName);
    }
    strcpy (oldpswd, inputCmdInfo->paraInfo[1].paraVal);
    strcpy (newpswd, inputCmdInfo->paraInfo[2].paraVal);

#if 1 /* jhnoh : 030916 : limit of passwd length */
	if (strlen(oldpswd) > 10) {
        strcat (resBuf,"    RESULT = FAIL\n    REASON = OLD PASSWORD IS TOO LONG\n    COMPLETED\n\n");
        logPrint (mmcLogId,FL, "%s\n", resBuf);
        return (mmcd_builtin_send_result (resBuf, -1, 0, inputCmdInfo));
	}
	if (strlen(newpswd) > 10) {
        strcat (resBuf,"    RESULT = FAIL\n    REASON = NEW PASSWORD IS TOO LONG\n    COMPLETED\n\n");
        logPrint (mmcLogId,FL, "%s\n", resBuf);
        return (mmcd_builtin_send_result (resBuf, -1, 0, inputCmdInfo));
	}
#endif

    // ��ϵǾ� �ִ� user���� Ȯ���Ѵ�.
    if ((userIndex = mmcd_getUserIndex (userName)) < 0) {
        strcat (resBuf,"    RESULT = FAIL\n    REASON = NOT_REGISTERED_USER\n    COMPLETED\n\n");
        logPrint (mmcLogId,FL, "%s\n", resBuf);
        return (mmcd_builtin_send_result (resBuf, -1, 0, inputCmdInfo));
    }

    // SU, NU Check 
    if (mmcdCliTbl[inputCmdInfo->cliIndex].privilege > MML_PRIVILEGE_SU && chg_own != 1) {
    // Password  Ȯ���Ѵ�.

	if (!chg_own) {
       		strcat(resBuf,"    RESULT = FAIL\n    REASON = NORMAL USER CHANGE PASSWORD ERROR\n    COMPLETED\n\n");
        	logPrint (mmcLogId,FL, "%s\n", resBuf);
  	   	return (mmcd_builtin_send_result (resBuf, -1, 0, inputCmdInfo));
	}
	if (!(strcmp(inputCmdInfo->paraInfo[1].paraVal, ""))) {
       		strcat(resBuf,"    RESULT = FAIL\n    REASON = OLDPASSWORD MISSING ERROR\n    COMPLETED\n\n");
        	logPrint (mmcLogId,FL, "%s\n", resBuf);
   	    	return (mmcd_builtin_send_result (resBuf, -1, 0, inputCmdInfo));
	}

    }
    // �ٸ� user���� �ٲٷ��� ��� �ڽ��� SU�̿��� �Ѵ�.
    if (!chg_own) {
        // �ڽ��� userIndex�� ã�´�.
        userIndex = mmcd_getUserIndex (inputCmdInfo->userName);
        if (mmcdUserTbl[userIndex].privilege != MML_PRIVILEGE_SU) { // �ڽ��� ����� Ȯ���Ѵ�.
            strcat (resBuf,"    RESULT = FAIL\n    REASON = PRIVILEGE_VIOLATION\n    COMPLETED\n\n");
            logPrint (mmcLogId,FL, "%s\n", resBuf);
            return (mmcd_builtin_send_result (resBuf, -1, 0, inputCmdInfo));
        }
        // �ٲٷ��� user�� userIndex�� ã�´�.
        if ((userIndex = mmcd_getUserIndex (userName)) < 0) {
            strcat (resBuf,"    RESULT = FAIL\n    REASON = NOT_REGISTERED_USER\n    COMPLETED\n\n");
            logPrint (mmcLogId,FL, "%s\n", resBuf);
            return (mmcd_builtin_send_result (resBuf, -1, 0, inputCmdInfo));
        }
    } else {
        // �ڽ��� userIndex�� ã�´�.
        userIndex = mmcd_getUserIndex (inputCmdInfo->userName);
    }

	if (strcmp (mmcdUserTbl[userIndex].passwd,
		(char*)crypt(inputCmdInfo->paraInfo[1].paraVal, inputCmdInfo->paraInfo[1].paraVal))) {
		strcat(resBuf,"    RESULT = FAIL\n    REASON = PASSWORD MISMATCH ERROR\n    COMPLETED\n\n");
		logPrint (mmcLogId,FL, "%s\n", resBuf);
			return (mmcd_builtin_send_result (resBuf, -1, 0, inputCmdInfo));
	}

    // passwd�� �����Ѵ�.
    strcpy (mmcdUserTbl[userIndex].passwd, (char*)crypt(newpswd,newpswd));

    // passwd ���Ͽ� �����Ѵ�.
    mmcd_savePasswdFile();

    strcat (resBuf,"    RESULT = SUCCESS\n    COMPLETED\n\n");

    // dis-msg-his mmc ������� ��ȸ�� log file�� ����Ѵ�.
    logPrint (mmcLogId,FL, "%s\n", resBuf);

    // client�� ��� �޽����� ������.
    return (mmcd_builtin_send_result (resBuf, 0, 0, inputCmdInfo));
} //----- End of mmcd_builtin_chg_passwd -----//


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int mmcd_builtin_dis_exe_cmd (MMLInputCmdInfo *inputCmdInfo)
{
	int		jobNo, cmdIndex;
	char	resBuf[4096],resHead[1024], tmp[1024];

	cmdIndex = inputCmdInfo->cmdIndex;
	sprintf(resHead, "\n    %s %s %s\n    %s\n",
			sysLabel, commlib_printTStamp(), inputCmdInfo->userName,
			mmlHelpTbl[cmdIndex].cmdSlogan);

	// �������� job�� �Ѱ��� �ִ��� Ȯ���Ѵ�.
	for (jobNo=0; jobNo<MML_NUM_TP_JOB_TBL; jobNo++)
		if (mmcdJobTbl[jobNo].tpInd) break;
	if (jobNo >= MML_NUM_TP_JOB_TBL) {
	//	strcat (resHead,"    RESULT = FAIL\n    REASON = NOT_FOUND_EXE_CMD\n    COMPLETED\n\n");
		strcat (resHead,"    RESULT = SUCCESS\n    REASON = NOT_FOUND_EXE_CMD\n    COMPLETED\n\n");
		logPrint (mmcLogId,FL, "%s\n", resHead);
		return (mmcd_builtin_send_result (resHead, -1, 0, inputCmdInfo));
	}

	// �������� job list�� �����.
	strcat (resHead,"    ==========================================================================\n");
	strcat (resHead,"    JOB_NO  COMMAND                                         USER      DEADLINE\n");
	strcat (resHead,"    ==========================================================================\n");

	strcpy (resBuf, resHead);

	for (jobNo=0; jobNo<MML_NUM_TP_JOB_TBL; jobNo++) {
		if (mmcdJobTbl[jobNo].tpInd == 0)
			continue;
		sprintf(tmp,"    %3d     %-45s   %-8s  %s\n", jobNo, mmcdJobTbl[jobNo].inputString,
				mmcdJobTbl[jobNo].userName,
				mmcd_printTimeHHMMSS(mmcdJobTbl[jobNo].deadlineTime));
		strcat (resBuf, tmp);
		if (strlen(resBuf) > 3000) {
			strcat (resBuf,"    RESULT = SUCCESS\n    CONTINUED\n\n");
			mmcd_builtin_send_result (resBuf, 0, 1, inputCmdInfo);
			strcpy (resBuf, resHead);
		}
	}
	strcat (resBuf,"    ==========================================================================\n");
	strcat (resBuf,"    RESULT = SUCCESS\n    COMPLETED\n\n");

	// dis-msg-his mmc ������� ��ȸ�� log file�� ����Ѵ�.
	logPrint (mmcLogId,FL, "%s\n", resBuf);

	// client�� ��� �޽����� ������.
	return (mmcd_builtin_send_result (resBuf, 0, 0, inputCmdInfo));

} //----- End of mmcd_builtin_dis_exe_cmd -----//


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int mmcd_builtin_canc_exe_cmd (MMLInputCmdInfo *inputCmdInfo)
{
    int		jobNo, cmdIndex;
    char	resBuf[4096],tmp[128];
    
    cmdIndex = inputCmdInfo->cmdIndex;
    sprintf(resBuf, "\n    %s %s %s\n    %s\n",
            sysLabel, commlib_printTStamp(), inputCmdInfo->userName,
            mmlHelpTbl[cmdIndex].cmdSlogan);
    
    // get parameter (job number)
    jobNo = strtol (inputCmdInfo->paraInfo[0].paraVal, 0, 0);
    
    sprintf(tmp, "    JOB_NO = %d\n", jobNo);
    strcat (resBuf, tmp);
    
    if (mmcdJobTbl[jobNo].tpInd == 0) { // job ���� �� ���� Ȯ��
        //strcat (resBuf, "    RESULT = FAIL\n    REASON = NOT_FOUND_JOB\n    COMPLETED\n\n");
        strcat (resBuf, "    RESULT = SUCCESS\n    REASON = NOT_FOUND_JOB\n    COMPLETED\n\n");
        logPrint (mmcLogId,FL, "%s\n", resBuf);
        return (mmcd_builtin_send_result (resBuf, -1, 0, inputCmdInfo));
    }
    else if (mmcdJobTbl[jobNo].cliSockFd != inputCmdInfo->cliSockFd) { // owner Ȯ�� --> �ڽ��� �Է��� ��ɸ� cancel�� �� �ִ�.
        strcat (resBuf, "    RESULT = FAIL\n    REASON = NOT_OWNER\n    COMPLETED\n\n");
        logPrint (mmcLogId,FL, "%s\n", resBuf);
        return (mmcd_builtin_send_result (resBuf, -1, 0, inputCmdInfo));
    }
    
    // application���� cancel �޽����� ������ job table�� �����Ѵ�.
    mmcd_sendCancMsg2App (jobNo);
    memset ((void*)&mmcdJobTbl[jobNo], 0, sizeof(MmcdJobTblContext));
    strcat (resBuf, "    RESULT = SUCCESS\n    COMPLETED\n\n");
    
    // dis-msg-his mmc ������� ��ȸ�� log file�� ����Ѵ�.
    logPrint (mmcLogId,FL, "%s\n", resBuf);
    
    // client�� ��� �޽����� ������.
    return (mmcd_builtin_send_result (resBuf, 0, 0, inputCmdInfo));
    
} //----- End of mmcd_builtin_canc_exe_cmd -----//


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int mmcd_builtin_canc_usr (MMLInputCmdInfo *inputCmdInfo)
{
    int     i, cmdIndex, userIndex;
    char    resBuf[6000];
    MMLInputCmdInfo retCmdInfo;

    cmdIndex = inputCmdInfo->cmdIndex;
    sprintf(resBuf, "\n    %s %s %s\n    %s\n",
            sysLabel, commlib_printTStamp(), inputCmdInfo->userName,
            mmlHelpTbl[cmdIndex].cmdSlogan);

    for (i=0; i<MML_NUM_TP_CLIENT_TBL; i++) {
        if (!strcmp(mmcdCliTbl[i].userName,inputCmdInfo->paraInfo[0].paraVal))
            break;
    }

    if(i >= MML_NUM_TP_CLIENT_TBL) {
        strcat (resBuf,"    RESULT = FAIL\n    REASON = NOT_CONNECTED_USER\n    COMPLETED\n\n");
        logPrint (mmcLogId,FL, "%s\n", resBuf);
        return (mmcd_builtin_send_result (resBuf, -1, 0, inputCmdInfo));
    }

    userIndex = mmcdCliTbl[i].userIndex;

    // update user table
    //(mmcdUserTbl[userIndex].loginCnt)--;
    mmcdUserTbl[userIndex].loginCnt = 0;	// yhshin
    mmcdUserTbl[userIndex].lastLogoutTime = currentTime;

    // lastLogoutTime�� passwd file�� �ݿ��ϱ� ����
    mmcd_savePasswdFile();

    // client�� ����� ������.
    // make message(retmdInfo) to inform to be disconnected by canc-usr mmc
    retCmdInfo.nmsibFlag = 0;
    retCmdInfo.cliReqId = 0;
    retCmdInfo.cliSockFd=mmcdCliTbl[i].cliSockFd;
    strcpy(retCmdInfo.inputString,"");
    mmcd_builtin_send_result ("\n    BYE...\n    DISCONNECTED BY SUPER USER \n", -1, 0,&retCmdInfo);


    // delete client table
    memset ((void*)&mmcdCliTbl[i], 0, sizeof(MmcdCliTblContext));

    sprintf(trcBuf,"[mmcd_builtin_log_out] user log-out; user=%s\n",
            mmcdUserTbl[userIndex].userName);
    trclib_writeLogErr (FL, trcBuf);

    strcat (resBuf,"    RESULT = SUCCESS\n    COMPLETED\n\n");

    // client�� ��� �޽����� ������.
    return (mmcd_builtin_send_result (resBuf, 0, 0, inputCmdInfo));

} //----- End of mmcd_builtin_canc_usr -----//


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int mmcd_builtin_send_result (char *data, char resCode, char contFlag, MMLInputCmdInfo *inputCmdInfo)
{
    int					txLen, msgId;
    char				segFlag=0, seqNo=1;
    SockLibMsgType		txSockMsg;
    MMLClientResMsgType	*txCliResMsg;
    
    txCliResMsg = (MMLClientResMsgType*)txSockMsg.body;
    memset ((void*)txCliResMsg, 0, sizeof(txCliResMsg->head));
    
    if (inputCmdInfo->nmsibFlag) {
        txCliResMsg->head.contFlag = MTYPE_MMC_RESPONSE;
    } else {
        txCliResMsg->head.contFlag = contFlag;
    } 
    
    txCliResMsg->head.cliReqId = htonl(inputCmdInfo->cliReqId);
    txCliResMsg->head.errCode = htonl(inputCmdInfo->errorCode); //yhshin htonl
    txCliResMsg->head.resCode  = resCode;
    txCliResMsg->head.segFlag  = segFlag;
    txCliResMsg->head.seqNo    = seqNo;
    
    if (data == NULL)
        strcpy (txCliResMsg->body, "");
    else
        strcpy (txCliResMsg->body, data);
    
    mmcd_send2COND (txCliResMsg->body);
    
    txSockMsg.head.bodyLen = strlen(txCliResMsg->body) + sizeof(txCliResMsg->head);
    txLen = sizeof(txSockMsg.head) + txSockMsg.head.bodyLen;
    
//	if (socklib_sndMsg(inputCmdInfo->cliSockFd, (char*)&txSockMsg, txLen) < 0) {
    if (socklib_sndMsg_hdr_chg(inputCmdInfo->cliSockFd, (char*)&txSockMsg, txLen) < 0) {
        sprintf(trcBuf,"[mmcd_builtin_send_result] socklib_sndMsg fail Error: %s\n", strerror(errno));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
//printf("mmcd_builtin_send_result End -[%d] %s : %s\n", inputCmdInfo->nmsibFlag, inputCmdInfo->cmdName,inputCmdInfo->userName);
    if (strstr(inputCmdInfo->inputString, "?") == NULL) 
    {
        msgId = msgId4Nmsib = NEXT(msgId4Nmsib,MML_NUM_TP_JOB_TBL);
        if (inputCmdInfo->nmsibFlag == 0)
        {
            if (txCliResMsg->head.resCode >= 0) 
            {
                mmcd_send2Nmsib(txCliResMsg->body,
                                MTYPE_MMC_RESPONSE,
                                segFlag,
                                seqNo,
                                msgId);
            }
        }
    }
    
    return 1;

} //----- End of mmcd_builtin_send_result -----//


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int mmcd_builtin_dis_cur_usr (MMLInputCmdInfo *inputCmdInfo)
{
    int		userIndex,cmdIndex, found=0;
    char	resBuf[4096],tmp[128];
    
    cmdIndex = inputCmdInfo->cmdIndex;
    sprintf(resBuf, "\n    %s %s %s\n    %s\n",
            sysLabel, commlib_printTStamp(), inputCmdInfo->userName,
            mmlHelpTbl[cmdIndex].cmdSlogan);
    
    strcat (resBuf,"    ======================================================================\n");
    strcat (resBuf,"    USER              CLASS     LOGIN         LAST_LOGIN       LAST_LOGOUT\n");
    strcat (resBuf,"    ======================================================================\n");
    
    for (userIndex=0; userIndex < MML_NUM_TP_USER_TBL; userIndex++) {
        if ((strcmp (mmcdUserTbl[userIndex].userName, "")) && (mmcdUserTbl[userIndex].loginCnt != 0) ) {
#if 1 /* jhnoh : 030819 */
            if (strncmp(mmcdUserTbl[userIndex].userName, "ROOT", 4) && strncmp(mmcdUserTbl[userIndex].userName, "INS_MNG", 7)) {
#endif
                //sprintf(tmp,"  %-16s  %-8s  %-8d  %-13s  %-13s\n", -->�ѹ��� �ι� call�ϸ� �ڿ� ������ ���� ������.
                sprintf(tmp,"      %-16s  %5s  %8d  %16s",
                        mmcdUserTbl[userIndex].userName,
                        mmcd_printUserClass(mmcdUserTbl[userIndex].privilege),
                        mmcdUserTbl[userIndex].loginCnt,
                        mmcd_printTimeMMDDHHMMSS(mmcdUserTbl[userIndex].lastLoginTime));
                strcat (resBuf, tmp);
                sprintf(tmp,"  %16s\n", mmcd_printTimeMMDDHHMMSS(mmcdUserTbl[userIndex].lastLogoutTime));
                strcat (resBuf, tmp);
                found = 1;
            }
        }
    }
    
    if (found == 0) {
        sprintf(resBuf, "\n    %s %s %s\n    %s\n",
                sysLabel, commlib_printTStamp(), inputCmdInfo->userName,
                mmlHelpTbl[cmdIndex].cmdSlogan);
        strcat(resBuf, "\n    NO USER IN CURRENT OMP SYSTEM \n");
    }
    strcat (resBuf,"    ======================================================================\n");
    strcat (resBuf,"    RESULT = SUCCESS\n    COMPLETED\n\n");
    
    // dis-msg-his mmc ������� ��ȸ�� log file�� ����Ѵ�.
    logPrint (mmcLogId,FL, "%s\n", resBuf);
    
    // client�� ��� �޽����� ������.
    return (mmcd_builtin_send_result (resBuf, 0, 0, inputCmdInfo));
    
} //----- End of mmcd_builtin_dis_usr_info -----//



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int mmcd_builtin_dis_cmd_his (MMLInputCmdInfo *inputCmdInfo)
{
    int		userIndex,cliIndex, cmdIndex, hisIndex, i, isnot=0;
    char	resBuf[4096],tmp[128];
    
    memset(resBuf, 0x00, sizeof(resBuf));
    
    cmdIndex = inputCmdInfo->cmdIndex;
    sprintf(resBuf, "\n    %s %s %s\n    %s\n",
            sysLabel, commlib_printTStamp(), inputCmdInfo->userName,
            mmlHelpTbl[cmdIndex].cmdSlogan);
    
    strcat (resBuf,"\n    NO  COMMAND");
    strcat (resBuf,"\n    ============================================================");
    cliIndex = inputCmdInfo->cliIndex;
    hisIndex = mmcdCliTbl[cliIndex].hisIndex;
    for (userIndex=hisIndex, i=1; userIndex != hisIndex-1; userIndex++) {
        memset(tmp, 0x00, sizeof(tmp));
        if (strcmp(mmcdCliTbl[cliIndex].history[userIndex], "")) {
            sprintf(tmp, "\n    %2d  %s", i, mmcdCliTbl[cliIndex].history[userIndex]);
            i++;
            isnot=1;
        }	
        strcat (resBuf, tmp);
        if (userIndex == MMCD_NUM_HISTORY_BUFF)
            userIndex = -1;
    }
    strcat (resBuf,"\n    RESULT = SUCCESS\n    COMPLETED\n\n");
    
    if (isnot==0) 
        sprintf(resBuf, "\n\n    RESULT = FAIL\n    REASON = NO HISTORY\n");

    return (mmcd_builtin_send_result (resBuf, 0, 0, inputCmdInfo));
    
} //----- End of mmcd_builtin_dis_usr_info -----//



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int mmcd_builtin_stat_cmd_canc (MMLInputCmdInfo *inputCmdInfo)
{
    int		jobNo,cmdIndex;
    char	resBuf[4096],tmp[128];
    
    cmdIndex = inputCmdInfo->cmdIndex;
    sprintf(resBuf, "\n    %s %s %s\n    %s\n",
            sysLabel, commlib_printTStamp(), inputCmdInfo->userName,
            mmlHelpTbl[cmdIndex].cmdSlogan);
    
    // get parameter (job number)
    jobNo = strtol (inputCmdInfo->paraInfo[0].paraVal, 0, 0);
    
    sprintf(tmp, "    JOB_NO = %d\n", jobNo);
    strcat (resBuf, tmp);
    
    if (mmcdJobTbl[jobNo].tpInd == 0) { // job ���� �� ���� Ȯ��
        strcat (resBuf, "    RESULT = FAIL\n    REASON = NOT_FOUND_JOB\n    COMPLETED\n\n");
        logPrint (mmcLogId,FL, "%s\n", resBuf);
        return (mmcd_builtin_send_result (resBuf, -1, 0, inputCmdInfo));
    }
    else if (strcmp(mmcdJobTbl[jobNo].dstAppName, "STMD")) {
        strcat (resBuf, "    RESULT = FAIL\n    REASON = NOT STATISTICS JOB NUMBER\n    COMPLETED\n\n");
        logPrint (mmcLogId,FL, "%s\n", resBuf);
        return (mmcd_builtin_send_result (resBuf, -1, 0, inputCmdInfo));
    }
    else if (mmcdJobTbl[jobNo].cliSockFd != inputCmdInfo->cliSockFd) { // owner Ȯ�� --> �ڽ��� �Է��� ��ɸ� cancel�� �� �ִ�.
        strcat (resBuf, "    RESULT = FAIL\n    REASON = NOT_OWNER\n    COMPLETED\n\n");
        logPrint (mmcLogId,FL, "%s\n", resBuf);
        return (mmcd_builtin_send_result (resBuf, -1, 0, inputCmdInfo));
    }
    
    // application���� cancel �޽����� ������ job table�� �����Ѵ�.
    mmcd_sendCancMsg2App (jobNo);
    memset ((void*)&mmcdJobTbl[jobNo], 0, sizeof(MmcdJobTblContext));
    strcat (resBuf, "    RESULT = SUCCESS\n    COMPLETED\n\n");
    
    // dis-msg-his mmc ������� ��ȸ�� log file�� ����Ѵ�.
    logPrint (mmcLogId,FL, "%s\n", resBuf);
    
    // client�� ��� �޽����� ������.
    return (mmcd_builtin_send_result (resBuf, 0, 0, inputCmdInfo));
    
} //----- End of mmcd_builtin_canc_exe_cmd -----//


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int mmcd_builtin_grade_help (MMLInputCmdInfo *inputCmdInfo)
{
    int		i, cmdIndex, matchCnt = 0;
    char	resBuf[4096], tmp[128], tmpBuf[32];
    char    input;
    
    if (!(strncmp(inputCmdInfo->paraInfo[0].paraVal, "SU", 2)))
        input = MML_PRIVILEGE_SU;
    else if (!(strncmp(inputCmdInfo->paraInfo[0].paraVal, "NU", 2)))
        input = MML_PRIVILEGE_NU;
    else if (!(strncmp(inputCmdInfo->paraInfo[0].paraVal, "GUEST", 5)))
        input = MML_PRIVILEGE_GUEST;
    else if (!(strncmp(inputCmdInfo->paraInfo[0].paraVal, "su", 2)))
        input = MML_PRIVILEGE_SU;
    else if (!(strncmp(inputCmdInfo->paraInfo[0].paraVal, "nu", 2)))
        input = MML_PRIVILEGE_NU;
    else if (!(strncmp(inputCmdInfo->paraInfo[0].paraVal, "guest", 5)))
        input = MML_PRIVILEGE_GUEST;
    
    strcpy (resBuf, "\n  ");
    for (cmdIndex=0; cmdIndex < MML_NUM_CMD; cmdIndex++)
    {
        for (i=0; i<strlen(mmlCmdTbl[cmdIndex].cmdName); i++)
            tmpBuf[i] = toupper(mmlCmdTbl[cmdIndex].cmdName[i]);
        tmpBuf[i] = 0;
        
        if (mmlCmdTbl[cmdIndex].privilege == input)	
        {
            sprintf(tmp, "    %-16s  ", mmlCmdTbl[cmdIndex].cmdName);
            strcat (resBuf, tmp);
            if (++matchCnt%4==0) strcat (resBuf, "\n  ");
            
            if (matchCnt%120==0) 
            { // 30��(4*30=120)�� ������ ������.
                if (mmcd_builtin_send_result (resBuf, 0, 1, inputCmdInfo) < 0)
                    return -1;
                strcpy (resBuf, "\n  ");
                commlib_microSleep(5000);
            }
        }
    }
    
    strcat (resBuf, "\n");
    return (mmcd_builtin_send_result (resBuf, 0, 0, inputCmdInfo));
    
} //----- End of mmcd_builtin_cmd_help -----//


//2009.07.17 by sjs

int mmcd_builtin_heart_beat( MMLInputCmdInfo *inputCmdInfo )
{
    mmcdCliTbl[inputCmdInfo->cliIndex].lastHeartBeatTime = currentTime;
#if 0
    sprintf(trcBuf,"[mmcd_builtin_heart_beat] alive; user=%s, fd=%d\n",
            inputCmdInfo->userName, inputCmdInfo->cliSockFd);
    trclib_writeLogErr (FL, trcBuf);
#endif
    
    return (mmcd_builtin_send_result( "\n       SUCCESS\n", 0, 0, inputCmdInfo ) );
}

int mmcd_builtin_add_ipaddr( MMLInputCmdInfo *inputCmdInfo )
{
    int		tblIndex = 0;
	char 	userName[COMM_MAX_NAME_LEN], objIP[IPLEN];
	char    resBuf[4096];
    int     ipListNum = 0;

	memset( resBuf, 0, sizeof( resBuf ) );
	memset( userName, 0, sizeof( userName ) );
    memset( objIP, 0, sizeof( objIP ) );
    
    strcpy( userName, inputCmdInfo->userName );
    strcpy( objIP, inputCmdInfo->paraInfo[0].paraVal );
	if( ( strlen( objIP ) <= 6 ) || ( check_ipaddr( objIP ) == -1 ) )
	{
		strcat (resBuf, "    RESULT = FAIL\n    REASON = INPUT ERROR\n    COMPLETED\n\n");
		logPrint (mmcLogId,FL, "%s\n", resBuf);
		return (mmcd_builtin_send_result (resBuf, -1, 0, inputCmdInfo));
	}

    if( mmcd_check_ipaddr( objIP ) == 0 )		//�̹� �����ǰ� ��ϵ� ���
    {
        strcat (resBuf, "    RESULT = FAIL\n    REASON = OVERLAPPED IP\n    COMPLETED\n\n");
        logPrint (mmcLogId,FL, "%s\n", resBuf);
        return (mmcd_builtin_send_result (resBuf, -1, 0, inputCmdInfo));
    }
#if 0
    if( mmcdIPTbl[MML_NUM_IP_TBL - 1].listFlag == 1 )	//������ ����Ʈ�� Ǯ�� ���
    {
        strcat (resBuf, "    RESULT = FAIL\n    REASON = FULL IP LIST\n    COMPLETED\n\n");
        logPrint (mmcLogId,FL, "%s\n", resBuf);
        return (mmcd_builtin_send_result (resBuf, -1, 0, inputCmdInfo));
    }
#endif
    for( tblIndex = 0; tblIndex < MML_NUM_IP_TBL; tblIndex++ )
    {
		if( mmcdIPTbl[tblIndex].listFlag != 0 ) {
			++ipListNum;
			if (ipListNum >= MML_NUM_IP_TBL) {
				strcat (resBuf, "    RESULT = FAIL\n    REASON = FULL IP LIST\n    COMPLETED\n\n");
				logPrint (mmcLogId,FL, "%s\n", resBuf);
				return (mmcd_builtin_send_result (resBuf, -1, 0, inputCmdInfo));
			}
			continue;
		}
        mmcdIPTbl[tblIndex].listFlag = 1;
        strcpy( mmcdIPTbl[tblIndex].ipAddress, objIP );
        strcpy( mmcdIPTbl[tblIndex].userName, userName );
		break;
    }

    mmcd_saveIPFile();
    strcat (resBuf, "    RESULT = SUCCESS\n    COMPLETED\n\n");
    logPrint (mmcLogId,FL, "%s\n", resBuf);
    return (mmcd_builtin_send_result (resBuf, -1, 0, inputCmdInfo));

}

int mmcd_builtin_dis_ipaddr_info( MMLInputCmdInfo *inputCmdInfo )
{
    int		ipIndex,cmdIndex;
    char	resBuf[4096],tmp[128];
    
    cmdIndex = inputCmdInfo->cmdIndex;
    sprintf(resBuf, "\n    %s %s %s\n    %s\n",
            sysLabel, commlib_printTStamp(), inputCmdInfo->userName,
            mmlHelpTbl[cmdIndex].cmdSlogan);
    strcat (resBuf,"    RESULT = SUCCESS\n");
    strcat (resBuf,"    ==========================================\n");
    strcat (resBuf,"    ADD USER              IP            ON/OFF\n" );
    strcat (resBuf,"    ==========================================\n");
    
    
    for( ipIndex = 0; ipIndex < MML_NUM_IP_TBL; ipIndex++) 
    {
        if( mmcdIPTbl[ipIndex].listFlag == 1 )
        {
            sprintf(tmp,"    %-16s  %-16s  %d\n", mmcdIPTbl[ipIndex].userName, 
                    mmcdIPTbl[ipIndex].ipAddress, mmcdIPTbl[ipIndex].useFlag );
            strcat (resBuf, tmp);
        }
    }
    strcat (resBuf,"    ==========================================\n");
    strcat (resBuf,"    COMPLETED\n\n");
    
    // dis-msg-his mmc ������� ��ȸ�� log file�� ����Ѵ�.
    logPrint (mmcLogId,FL, "%s\n", resBuf);
    
    // client�� ��� �޽����� ������.
    return (mmcd_builtin_send_result (resBuf, 0, 0, inputCmdInfo));
}

int mmcd_builtin_del_ipaddr( MMLInputCmdInfo *inputCmdInfo )
{
    int 	ipIndex;
    char	objIP[IPLEN];
	char    resBuf[4096];

	memset( resBuf, 0, sizeof( resBuf ) );
    strcpy( objIP, inputCmdInfo->paraInfo[0].paraVal );
    
    ipIndex = mmcd_check_ipaddr( objIP );
    if( ipIndex < 0 )				// ip�� ����Ʈ�� ���°��
    {
        strcat (resBuf, "    RESULT = FAIL\n    REASON = NOT FOUND IP\n    COMPLETED\n\n");
        logPrint (mmcLogId,FL, "%s\n", resBuf);
        return (mmcd_builtin_send_result (resBuf, -1, 0, inputCmdInfo));
    }
    else if( mmcdIPTbl[ipIndex].useFlag == 1 ) // ip�� ���� ������� ���
    {
        strcat (resBuf, "    RESULT = FAIL\n    REASON = USE IP\n    COMPLETED\n\n");
        logPrint (mmcLogId,FL, "%s\n", resBuf);
        return (mmcd_builtin_send_result (resBuf, -1, 0, inputCmdInfo));
    }

    mmcdIPTbl[ipIndex].listFlag = 0;
    mmcd_saveIPFile();

    strcat (resBuf, "    RESULT = SUCCESS\n    COMPLETED\n\n");
    logPrint (mmcLogId,FL, "%s\n", resBuf);
    return (mmcd_builtin_send_result (resBuf, -1, 0, inputCmdInfo));

}

int mmcd_check_ipaddr( const char *clientIP )
{
    int tblIndex;
    for( tblIndex = 0; tblIndex < MML_NUM_IP_TBL; tblIndex++ )
    {
        if( !strcmp( mmcdIPTbl[tblIndex].ipAddress, clientIP ) )
        {
			if( mmcdIPTbl[tblIndex].listFlag == 1 )
				return tblIndex;
        }
    }
    return -1;
}

int check_ipaddr( const char *clientIP )
{
	char 	buf[IPLEN];
	int 	ipSize;
	int		i, dotCnt = 0;;
	strcpy( buf, clientIP );
	
	ipSize = strlen( buf );
	for( i = 0; i < ipSize; i++ )
	{
		if( !isdigit( buf[i] ) )
		{
			if( buf[i] != '.' )
			{
				return -1;
			}
			dotCnt++;
			if( dotCnt >= 4 )
			{
				return -1;
			}
		}
	}
	if( dotCnt != 3 )
	{
		return -1;
	}
	return 0;
}
