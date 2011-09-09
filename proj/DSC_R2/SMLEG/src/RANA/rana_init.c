/** A. FILE INCLUSION *********************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <unistd.h>

#include <ipaf_shm.h>
#include <ipaf_define.h>
#include <ipaf_error.h>
#include <define.h>
#include <init_shm.h>
#include <ipaf_stat.h>

#include <sys/types.h>
#include <arpa/inet.h>

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stropts.h>
#include <string.h>
#include <fcntl.h>
#include <sys/errno.h>
#include "conflib.h"
#include "rana.h"

/** B. DEFINITION OF NEW CONSTANTS ********************************************/

/** C. DEFINITION OF NEW TYPES ************************************************/

/** D. DECLARATION OF VARIABLES ***********************************************/
extern int      JiSTOPFlag;
extern int      FinishFlag;

//extern  T_CAP   *shm_cap;

/** E.1 DEFINITION OF FUNCTIONS ***********************************************/
extern int log_write( char *fmt, ... );
extern int dAppLog(int dIndex, char *fmt, ...);

/** E.2 DEFINITION OF FUNCTIONS ***********************************************/

/*******************************************************************************

*******************************************************************************/
void FinishProgram()
{
    dAppLog( LOG_CRI, "PROGRAM IS NORMALLY TERMINATED, Cause = %d", FinishFlag);
    exit(0);
}

/*******************************************************************************

*******************************************************************************/
void UserControlledSignal(int sign)
{
	dAppLog(LOG_CRI, "DEFINED SIGNAL IS RECEIVED, signal = %d", sign);
    JiSTOPFlag = 0;
    FinishFlag = sign;

    FinishProgram();
}

/*******************************************************************************

*******************************************************************************/
void IgnoreSignal(int sign)
{
    if (sign != SIGALRM)
        dAppLog( LOG_CRI, "UNWANTED SIGNAL IS RECEIVED, signal = %d", sign);
    signal(sign, IgnoreSignal);
}

/*******************************************************************************

*******************************************************************************/
void SetUpSignal()
{
    JiSTOPFlag = 1;

    /* WANTED SIGNALS   */
    signal(SIGTERM, UserControlledSignal);
    signal(SIGINT,  UserControlledSignal);
    signal(SIGQUIT, UserControlledSignal);

    /* UNWANTED SIGNALS */
    signal(SIGHUP,  IgnoreSignal);
    signal(SIGALRM, IgnoreSignal);
    signal(SIGPIPE, IgnoreSignal);
    signal(SIGPOLL, IgnoreSignal);
    signal(SIGPROF, IgnoreSignal);
    signal(SIGUSR1, IgnoreSignal);
    signal(SIGUSR2, IgnoreSignal);
    signal(SIGVTALRM, IgnoreSignal);
    signal(SIGCLD, SIG_IGN);

	dAppLog(LOG_CRI, "SIGNAL HANDLER WAS INSTALLED");
}

int dGetConfig_LEG (void)
{
	int     lNum, rowCnt=0, ret;
	char    *env;
	char    iv_home[64];
	char    module_conf[64];
	char    getBuf[256], token[9][64];
	FILE    *fp=NULL;

	if( (env = getenv(IV_HOME)) == NULL) {
		dAppLog(LOG_CRI, "[%s:%s:%d] not found %s environment name", __FILE__, __FUNCTION__, __LINE__, IV_HOME);
		return -1;
	}
	strcpy(iv_home, env);
	sprintf(module_conf, "%s", DEF_SYSCONF_FILE);

	if ((fp = fopen(module_conf, "r")) == NULL) {
		dAppLog(LOG_CRI,"[dGetConfig_LEG] fopen fail[%s]; err=%d(%s)", module_conf, errno, strerror(errno));
		return -1;
	}

	/* [RLEG_CONFIG] section으로 이동 */
	if ((lNum = conflib_seekSection (fp,"RLEG_CONFIG")) < 0) {
		dAppLog(LOG_CRI,"[dGetConfig_LEG] Secsion not found");
		return -1;
	}

	/* 등록된 시스템들의 이름과 IP_ADDRESS를 저장한다. */
	while (fgets(getBuf,sizeof(getBuf),fp) != NULL)
	{
		if (rowCnt >= MAX_SM_NUM) break;
		if (getBuf[0] == '[') /* end of section */
			break;
		if (getBuf[0]=='#' || getBuf[0]=='\n') /* comment line or empty */
			continue;

		if ((ret = sscanf(getBuf,"%s%s%s%s%s%s%s%s%s"
						, token[0],token[1],token[2],token[3],token[4],token[5],token[6],token[7],token[8])) < 9)
		{
			dAppLog(LOG_CRI,"[dGetConfig_LEG] config syntax error; file=%s, lNum=%d, ret = %d"
					, module_conf , lNum, ret);
			fclose(fp);
			return -1;
		}

		strcpy(gSCM[rowCnt].ip, token[2]);
		gSCM[rowCnt].port = atoi(token[3]);

		gSCM[rowCnt].svc_type[0] = atoi(token[4]);
		gSCM[rowCnt].svc_type[1] = atoi(token[5]);
		gSCM[rowCnt].svc_type[2] = atoi(token[6]);
		gSCM[rowCnt].svc_type[3] = atoi(token[7]);
		gSCM[rowCnt].svc_type[4] = atoi(token[8]);

		dAppLog(LOG_INFO, "[dGetConfig_LEG] gSCM[%d] [IP:%s][PORT:%d][SVC_OPT1:%d][SVC_OPT2:%d][SVC_OPT3:%d][SVC_OPT4:%d][SVC_OPT5:%d]"
				, rowCnt, gSCM[rowCnt].ip, gSCM[rowCnt].port
				, gSCM[rowCnt].svc_type[0], gSCM[rowCnt].svc_type[1], gSCM[rowCnt].svc_type[2], gSCM[rowCnt].svc_type[3], gSCM[rowCnt].svc_type[4]);

		rowCnt++;
	}
	fclose(fp);
	return 0;
}
                
int dCheck_SvcOpt(int svc_opt)
{   
	int i, dIdx=0;

	if (!strncmp(mySysName, _SYS_NAME_MPA, 4)) {
		dIdx=0;
	}           
	else if (!strncmp(mySysName, _SYS_NAME_MPB, 4)) {
		dIdx=1;
	}
	else {
		dAppLog(LOG_DEBUG, "my system name is wrong");
		return -1;
	}   

	for (i=0; i<MAX_SVCOPT_COUNT; i++) {
		if (svc_opt  == gSCM[dIdx].svc_type[i]) {
			return 0;
		}
		else
			return (-2);
	}
	return (-3);
}

int dSend_CpsOverLoadReport(unsigned char over_flag)
{
	loc_sadb->cps_over_alm_flag = over_flag;
	dAppLog(LOG_DEBUG, "CPS OVERLOAD FLAG = %d", over_flag);
	return 0;
}

int g_cps_overload_count = 0;

int dCheck_CpsOverLoad(int cur_cps, CPS_OVLD_CTRL *ctrl)
{
	if(ctrl->over_cps > cur_cps) 	goto StndLoad;


	if(!ctrl->over_flag) {
		// TODO: 	Enter Overload Status.
		//			Send Failure Message to FIMD
		ctrl->over_flag = 1;
		g_cps_overload_count = 0;
		dAppLog(LOG_CRI, "CPS] CUR=%d [FLAG:%d CPS:%d RATE:%d"
			, cur_cps, ctrl->over_flag,ctrl->over_cps,ctrl->over_rate);
		dAppLog(LOG_CRI, "CPS] DO NOT LOGIN [CPS: %d", cur_cps);
		dSend_CpsOverLoadReport (ctrl->over_flag);
	}
	g_cps_overload_count += 1;
  //  if(!(g_cps_overload_count % (cur_cps / (100 / ctrl->over_rate)))) {
    if(!(g_cps_overload_count % ((100 / ctrl->over_rate)))) {
        dAppLog(LOG_CRI, "CPS] DO NOT LOGIN [CNT: %d, CPS:%d, RATE: %d", g_cps_overload_count, cur_cps, ctrl->over_rate);
        dSend_CpsOverLoadReport (ctrl->over_flag);
        return 1;
    }

	dAppLog(LOG_DEBUG, "CPS] DO LOGIN [CNT: %d, CPS:%d, RATE: %d", g_cps_overload_count, cur_cps, ctrl->over_rate);
	return 0;

StndLoad:
	if(ctrl->over_flag) {
		// TODO: 	Enter Normal Status.
		//			Send Recovery Message to FIMD
		ctrl->over_flag = 0;
		dAppLog(LOG_DEBUG, "CPS] CUR=%d [FLAG:%d CPS:%d RATE:%d"
			, cur_cps, ctrl->over_flag,ctrl->over_cps,ctrl->over_rate);
		dAppLog(LOG_DEBUG, "CPS] DO LOGIN [CPS: %d", cur_cps);
		dSend_CpsOverLoadReport (ctrl->over_flag);
	}
	return 0;
}

void dump_sce_login_info (int tidx, PSUBS_INFO pSI)
{
	dAppLog(LOG_INFO, "[dGetSubsInfo] GET SUBSCRIBER-NAME		   	:%s", pSI->szMIN);
	dAppLog(LOG_INFO, "[dGetSubsInfo] GET SUBSCRIBER-IP			 	:%s", pSI->szFramedIP);
	dAppLog(LOG_INFO, "[dGetSubsInfo] GET SUBSCRIBER-DOMAIN		 	:%s", pSI->szDomain);
	//dAppLog(LOG_INFO, "[dGetSubsInfo] GET SUBSCRIBER-PROPERTY-NAME1 :%s", pSI->prop_key[0]);
	//dAppLog(LOG_INFO, "[dGetSubsInfo] GET SUBSCRIBER-PROPERTY-VAL2  :%s", pSI->prop_val[0]);
	//dAppLog(LOG_INFO, "[dGetSubsInfo] GET SUBSCRIBER-PROPERTY-NAME2 :%s", pSI->prop_key[1]);
	//dAppLog(LOG_INFO, "[dGetSubsInfo] GET SUBSCRIBER-PROPERTY-KEY2  :%s", pSI->prop_val[1]);
}

void makeSubsRecords (pst_RADInfo pstRADInfo, PSUBS_INFO psi)
{
	sprintf (psi->szMIN, "%s", pstRADInfo->szMIN);
	sprintf(psi->szFramedIP, "%s", CVT_INT2STR_IP(pstRADInfo->uiFramedIP));
	sprintf(psi->szDomain, "subscribers");
	psi->type = IP_RANGE;
	psi->uiCBit = pstRADInfo->uiCBIT;
	psi->uiPBit = pstRADInfo->uiPBIT;
	psi->uiHBit = pstRADInfo->uiHBIT;
}

char * TreamNullData(char *sParseStr)
{
    int i = 0;

    for(i = 0; i<(int)strlen(sParseStr); i++)
    {
        if( sParseStr[i] == ' ' || sParseStr[i] == '\t' )
        {
            continue;
        }
        else
        {
            break;
        }
    }
    return (sParseStr + i);
}

void Get1Data(char *sParseStr, char *getData)
{
    int     i = 0;


    for(i = 0; i<(int)strlen(sParseStr); i++)
    {
        if( sParseStr[i] == ' ' || sParseStr[i] == '\t'
            || sParseStr[i] == '\0' || sParseStr[i] == '\n')
        {
            break;
        }
        else
        {
            getData[i] = sParseStr[i];
        }
    }
    getData[i] = '\0';

    return;
}
 

int GetConfData(char *sParseStr, char ppstData[64][64], int *iCount)                                                                                      
{                                                                                                                                                         
	int     e_Ret = 0;                                                                                                                                    
	int i = 0;                                                                                                                                            
	char *tmpStr = NULL;                                                                                                                                  
	char getData[64];                                                                                                                                     

	tmpStr = sParseStr;                                                                                                                                   
	for(i = 0; i<64; i++)                                                                                                                                 
	{
		memset(getData, 0x00, 64);
		tmpStr = TreamNullData(tmpStr);
		Get1Data(tmpStr, getData);
		if(getData[0] == '\0')                                                                                                                            
		{                                                                                                                                                 
			break;                                                                                                                                        
		}                                                                                                                                                 
		snprintf(ppstData[i], 64, "%s", getData);                                                                                                         
		tmpStr += strlen(ppstData[i]);                                                                                                                    
	}                                                                                                                                                     

	*iCount = i;                                                                                                                                          

	return e_Ret;                                                                                                                                         
} 
