#include "mmcd_proto.h"

int		mmcdQid, condQid, ixpcQid, nmsibQid, MML_NUM_CMD, cmdLogId, mmcLogId, msgId4Nmsib=0;
int     overloadFlag=0,tmpFlag=0;
time_t	currentTime;
char	inputErrBuf[1024];
char	trcBuf[4096], trcTmp[1024];
char	mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN], sysLabel[COMM_MAX_NAME_LEN];
MMLHelpContext		*mmlHelpTbl;
MmcdJobTblContext	*mmcdJobTbl;
MmcdUserTblContext	*mmcdUserTbl;
MmcdCliTblContext	*mmcdCliTbl;
MMLCmdContext		*mmlCmdTbl;
MMcdUserIPTblContext	*mmcdIPTbl;		// 2009.07.17 by sjs
extern int	trcFlag, trcLogFlag;
SFM_sfdb     *sfdb;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main (int ac, char *av[])
{
    GeneralQMsgType	rxGenQMsg;
    SockLibMsgType	rxSockMsg;
    
    int		ret,actFd,loopCnt=0;
    int		tickPerSec=100;
    int     check_Index;
    
#if 1
    if((check_Index = check_my_run_status("MMCD")) < 0)
    	exit(0);
#endif
    
    if (mmcd_initial() < 0)
        return -1;
    // clear previous queue messages
    //
    while (msgrcv(mmcdQid, &rxGenQMsg, sizeof(rxGenQMsg), 0, IPC_NOWAIT) > 0);
    memset ((void*)&rxGenQMsg, 0, sizeof(rxGenQMsg));
    
    while (1)
    {
        // client��� ����� socket port�� Ȯ���Ͽ� ���� ����/���� �޽��� ���� ��
        //	event�� ó���Ѵ�.
        ret = socklib_action ((char*)&rxSockMsg, &actFd);
        
        switch (ret)
        {
        case SOCKLIB_CLIENT_MSG_RECEIVED:
            mmcd_exeMmcReqMsg (&rxSockMsg, actFd);
            break;
            
        case SOCKLIB_NEW_CONNECTION: // client ����
            // ���Ӹ� �̷���� �����̹Ƿ� �ƹ��� ó���� �� �ʿ䰡 ����.
            // - ���� �� login ������ ���ľ� ��ɾ �Է��� �� �����Ƿ�...
            // - client table�� �߰��Ǵ� ������ log-in�� ������ �����̴�.
            sprintf(trcBuf,"[mmcd_main] new connection fd=%d\n", actFd);
            trclib_writeLogErr (FL,trcBuf);
            break;
            
        case SOCKLIB_CLIENT_DISCONNECTED:
            // client ���� ��������, �ش� client�� ���� �������� mmc�� job table����
            //	ã�� cancel �޽����� ������ �����Ѵ�.
            // client table�� �����ϰ� user table�� update�Ѵ�.
            mmcd_exeClientDisconn(actFd);
            break;
            
        default:
            break;
        } //end of socklib_action
        
        
        // �ڽ��� msgQ�� ������ �޽����� �����Ѵ�.
        // - application���κ��� ��ɾ� ó�� ����� ixpc�� ���� ���´�.
        // - �ڽ��� ���� ó���ؾ� �ϴ� built-in ����� ���, syntax Ȯ�� �� �ڽ���
        //	msgQ�� ���¾��� ���� ������ ����̴�.
        //
        if (msgrcv(mmcdQid, &rxGenQMsg, sizeof(rxGenQMsg), 0, IPC_NOWAIT) > 0) 
        {
            switch (rxGenQMsg.mtype) 
            {
            case MTYPE_STAT_REPORT_SHORT_TERM:
            case MTYPE_STAT_REPORT_LONG_TERM:
            case MTYPE_MMC_RESPONSE: // ��ɾ� ó�� ��� ����
                mmcd_exeMmcResMsg (&rxGenQMsg);
                break;
            case MTYPE_SETPRINT:
                trclib_exeSetPrintMsg ((TrcLibSetPrintMsgType*)&rxGenQMsg);
            }
            memset ((void*)&rxGenQMsg, 0, sizeof(rxGenQMsg));
        }
        
        
        // - �ֱ���(0.5��)���� mmcdJobTbl�� �˻��� application���κ��� ������ ����
        //	timer�� ����� ���� ã�� timeout ó���Ѵ�.
        // - currentTime�� update�Ѵ�.
        if (((++loopCnt) % (tickPerSec/2)) == 0) 
        {
            mmcd_scanJobTbl (); 
            proc_check_heart_beat();	// 2009.07.15 by sjs
        }
        
        // read SFDB if CPU overload occured
        if (((loopCnt) % (tickPerSec*2)) == 0) 
        {
            proc_overload ();
        }
    } //-- end of while(1) --//
    return 1;
} //----- End of main -----//

int proc_overload ()
{
    int 			tmpFlag = overloadFlag;
    int 			i,j,k,cpu_usage,userIndex;
    MMLInputCmdInfo retCmdInfo;
    
    for ( k=0; k<SYSCONF_MAX_ASSO_SYS_NUM; k++)
    {
        //jean if ( strcasecmp ( sfdb->sys[k].commInfo.type, SYSCONF_SYSTYPE_SMS ) ) continue;
        if ( strcasecmp ( sfdb->sys[k].commInfo.type, SYSCONF_SYSTYPE_BSD ) ) 
            continue;
        overloadFlag = 0;
        for (j=0; j<sfdb->sys[k].commInfo.cpuCnt; j++) 
        {
            if (sfdb->sys[k].commInfo.cpuInfo.level[j] == SFM_ALM_MASKED)
                continue;
            cpu_usage = sfdb->sys[k].commInfo.cpuInfo.usage[j]/10;
            
#if 0// ��� �������  ������ ó��.
            if ( cpu_usage > sfdb->sys[k].commInfo.cpuInfo.minLimit)
#else
                if ( sfdb->sys[k].commInfo.cpuInfo.level[j] > SFM_ALM_NORMAL )
#endif
                    overloadFlag = 1;
        }
        if (!tmpFlag && overloadFlag) 
        {
            // update user table
            for (i=0; i<MML_NUM_TP_CLIENT_TBL; i++) 
            {
                if (mmcdCliTbl[i].privilege>1) 
                {
                    userIndex = mmcdCliTbl[i].userIndex;
                    
                    //update user table
                    (mmcdUserTbl[userIndex].loginCnt)--;
                    mmcdUserTbl[userIndex].lastLogoutTime = currentTime;
                    
                    // save lastLogoutTime at passwd file�� 
                    mmcd_savePasswdFile();
                    
                    // make message(inputCmdInfo) to inform to be disconnected by cpu_overload 
                    // send to client 
                    retCmdInfo.nmsibFlag = 0;
                    retCmdInfo.cliReqId = 0;
                    retCmdInfo.cliSockFd=mmcdCliTbl[i].cliSockFd;
                    strcpy(retCmdInfo.inputString,"");
                    
                    mmcd_builtin_send_result ("\n    BYE...\n    CPU OVERLOAD, SUPER USER ONLY \n", 
                                              -1, 0, &retCmdInfo);
                    
                    // connection�� ���´�.
                    socklib_disconnectClientFd ( mmcdCliTbl[i].cliSockFd );
                    
                    // delete client table
                    memset ((void*)&mmcdCliTbl[i], 0, sizeof(MmcdCliTblContext));
                    
                    sprintf(trcBuf,"log-out because of cpu_overload, user = %s \n",
                            mmcdUserTbl[userIndex].userName);
                    trclib_writeLogErr (FL, trcBuf);
                }
            }
        }
    }
     return 1;
}

// 2009.07.15 by sjs
int proc_check_heart_beat()
{
    int i;
    time_t curTime;
    curTime = time( NULL );
    for ( i = 0 ; i < MML_NUM_TP_CLIENT_TBL; i++ ) 
    {
        if( ( curTime - mmcdCliTbl[i].lastHeartBeatTime ) > 30 
            && ( mmcdCliTbl[i].lastHeartBeatTime != ( time_t )0 ) ) 
        {
            socklib_disconnectClientFd( mmcdCliTbl[i].cliSockFd );
            sprintf( trcBuf, "heartbeat check disconnect, user = %s table = %d heartbeat = %d\n",
                     mmcdCliTbl[i].userName, i, ( int )mmcdCliTbl[i].lastHeartBeatTime );
            trclib_writeLogErr (FL, trcBuf);
            
            mmcd_exeClientDisconn( mmcdCliTbl[i].cliSockFd );
			return 1;
        }
    }
	return 0;
}

