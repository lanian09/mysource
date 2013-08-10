//#include "dagapi.h"
//#include "dagutil.h"
#include <sys/shm.h>
#include <errno.h>
#include <signal.h>

#include <shmutil.h>
#include "capd_def.h"
#include <utillib.h>
#include <init_shm.h>

#include <mysql.h>
#include <pthread.h>

//#include "mems.h"
#include "nifo.h"
#include "rdrheader.h"
#include "capd_msgtypes.h"
#include "capd_global.h"
#include "comm_trace.h"	// Trace Info 구조체 
#include "sysconf.h"		// TRACE_INFO_FILE define 
#include "conflib.h"

/* Function Time Check */
#include "func_time_check.h"
#include "comm_session.h"
#include "rdrheader.h"



/** D. DECLARATION OF VARIABLES ***********************************************/
extern int      JiSTOPFlag;

extern SFM_SysCommMsgType   *loc_sadb;

extern int 				dMyQid;
extern int 				dCAPDQid;
extern int 				dSmppQid;
extern int 				dIxpcQid;
extern int 				dMmcrQid;
//extern int 				dCondQid;

extern MYSQL			sql, *conn;
extern SCE_LIST         g_stSce[MAX_SCE_NUM];
// TRACE_INFO.conf 구조체 
extern st_SESSInfo			*gpTrcList[DEF_SET_CNT];
extern st_SESSInfo			*gpCurTrc; // CURRENT OPERATION pointer

extern st_NOTI				*gpIdx;


extern RDRMmcHdlrVector mmcHdlrVector[MAX_MMC_HANDLER];
extern int numMmcHdlr;

extern char            sysLable[COMM_MAX_NAME_LEN], mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN];

int conflib_getNthTokenInFileSection ( char *fname, char *section, char *keyword, int n, char *string );

extern int 	InitSHM_TRACE(void);
extern int rdrana_mmcHdlrVector_qsortCmp (const void *a, const void *b);
extern int readSmsFile(void);
extern rad_sess_body *find_rad_sess (rad_sess_key *pkey);
extern char * find_imsi_rad_sess (rad_sess_key *pkey);
/* DEL : by june, 2010-10-03
 * DESC: TRACE 에 사용되는 SESSION 정보 참조 파트 주석 처리
extern void init_session (int shm_key);
*/
void init_RuleEntry(void);
int select_Sce(void);

extern unsigned char           ruleSetList[MAX_PKG_ID_NUMBER];
extern RuleSetList				g_stRule[MAX_RULE_SET_LIST];
extern RuleEntryList			g_stEntry[MAX_RULE_ENTRY_LIST];

// Trace Info 구조체 
extern st_SESSInfo			g_stTrcInfo;

extern SMS_DB_INFO			sms_db;

int	MakeRuleSetList(char *fname);


void CatchSignal(int sig)
{
	JiSTOPFlag = 0;
	dAppLog(LOG_CRI, "%s: SIGNAL[%d] CATCH. FINISH PROGRAM", __FUNCTION__, sig);

	exit(0);
}

void IgnoreSignal(int sig)
{
	if( sig != SIGALRM )
		dAppLog(LOG_INFO, "SIGNAL[%d] RECEIVED.", sig);
	signal(SIGALRM, IgnoreSignal);
}

void SetupSignal(void)
{

	JiSTOPFlag = 1;

    /* EXIT SIGNALS   */
	//signal(SIGSEGV, CatchSignal);
	signal(SIGHUP, CatchSignal);
	signal(SIGINT, CatchSignal);
	signal(SIGTERM, CatchSignal);
	signal(SIGPIPE, CatchSignal);
	signal(SIGQUIT, CatchSignal);

    /* IGNORE SIGNALS */
    signal(SIGALRM, IgnoreSignal);
    signal(SIGPOLL, IgnoreSignal);
    signal(SIGPROF, IgnoreSignal);
    signal(SIGUSR1, IgnoreSignal);
    signal(SIGUSR2, IgnoreSignal);
    signal(SIGVTALRM, IgnoreSignal);
    signal(SIGCLD, SIG_IGN);
}

int sms_get_conf (void)
{
	int     lNum, ret;
	char    *env;
	char    iv_home[64];
	char    getBuf[256], token[9][64];
	FILE    *fp=NULL;
	char    module_conf[256];

	if( (env = getenv(IV_HOME)) == NULL) {
		dAppLog (LOG_DEBUG, "[%s:%d] not found %s environment name", __FUNCTION__, __LINE__, IV_HOME);
		return -1;
	}
	strcpy(iv_home, env);
	sprintf(module_conf, "%s/%s", iv_home, SMPP_CONF_FILE);

	if ((fp = fopen(module_conf, "r")) == NULL) {
		dAppLog (LOG_DEBUG, "[%s] fopen fail[%s]; err=%d(%s)", module_conf, errno, strerror(errno));
		return -1;
	}

	/* [DB_INFO] sectionA¸·I AIμ¿ */
	if ((lNum = conflib_seekSection (fp,"DB_INFO")) < 0) {
		return -1;
	}

	/* μi·IμE ½A½ºAUμeAC AI¸§°u IP_ADDRESS¸| AuAaCN´U. */
	while (fgets(getBuf,sizeof(getBuf),fp) != NULL)
	{
		if (getBuf[0] == '[') /* end of section */
			break;
		if (getBuf[0]=='#' || getBuf[0]=='\n') /* comment line or empty */
			continue;

		if ((ret = sscanf(getBuf,"%s%s%s%s%s",token[0],token[1],token[2],token[3], token[4])) < 5) {
			dAppLog (LOG_DEBUG, "[sms_get_conf][DB_INFO] syntax error; file=%s, lNum=%d, ret = %d"
					, module_conf , lNum, ret);
			fclose(fp);
			return -1;
		}

		strcpy(sms_db.name, token[0]);
		strcpy(sms_db.ipaddr, token[2]);
		strcpy(sms_db.user, token[3]);
		strcpy(sms_db.passwd, token[4]);
		strcpy(sms_db.table, SMS_TBL);
		dAppLog(LOG_DEBUG, "### DB INFO : NAME:%s  IP:%s  USER:%s  PASS:%s  TABLE:%s"
				, sms_db.name
				, sms_db.ipaddr
				, sms_db.user
				, sms_db.passwd
				, sms_db.table);
	}

	fclose(fp);
	return 0;
}

int INIT_CAPD_IPCS(void)
{
	int     key, ret;
	char    tmp[64], fname[256];
	char   	szLogBuf[1024];

	sprintf(fname,"%s", DEF_SYSCONF_FILE );

	/* GET SYSTEM LABEL */
	if (conflib_getNthTokenInFileSection (fname, "[GENERAL]", "SYSTEM_LABEL", 1, tmp) < 0) 
	{
		sprintf( szLogBuf, "CAN'T GET SYSTEM LABEL err=%s", strerror(errno));
		dAppWrite( LOG_CRI, szLogBuf );
		return -1;
	}
	else 
	{
		strcpy(sysLable, tmp);
		strcpy(mySysName, sysLable);
		strcpy(myAppName, "RDRANA");
		dAppLog(LOG_CRI, "SYSTEM LABEL=[%s]", sysLable);
	}

    /* INIT_SHM: SHM_LOC_SADB */
    if (conflib_getNthTokenInFileSection (fname, "[SHARED_MEMORY_KEY]", "SHM_LOC_SADB", 1, tmp) < 0) {
        sprintf( szLogBuf, "CAN'T GET SHM KEY OF SHM_LOC_SADB err=%s", strerror(errno));
        dAppWrite( LOG_CRI, szLogBuf );
        return -1;
    }
    else
        key = strtol(tmp, 0, 0);
    ret = shmget( key, sizeof(SFM_SysCommMsgType), 0666|IPC_CREAT|IPC_EXCL); 
    if( ret < 0 )
    {
        if(errno == EEXIST)                                                  
        {                                                                    
            ret = shmget( key, sizeof(SFM_SysCommMsgType), 0666|IPC_CREAT);  
            if(ret < 0)                                                      
                return -2;

            if((long)(loc_sadb = (SFM_SysCommMsgType *)shmat( ret, (char*)0, 0)) == -1)                                                                            
                return -3;
        }
        else                                                                 
            return -4;                                                       
    }
    if((long)(loc_sadb = (SFM_SysCommMsgType *)shmat( ret, (char*)0, 0)) == -1)
        return -5;

	/* MMCR NOTI Shared Memory Init */
	if( InitSHM_TRACE() < 0 ) {
		dAppLog( LOG_CRI, "ERROR InitSHM_TRACE() FAIL");
		return -6;
	}


	if (conflib_getNthTokenInFileSection (fname, "[APPLICATIONS]", "RDRANA", 1, tmp) < 0) {
		dAppLog( LOG_CRI, "CAN'T GET MSGQ KEY OF RDRANA err=%s", strerror(errno));
		return -1;
	}
	else
	{
		key = strtol(tmp, 0, 0);
    	if ((dMyQid = msgget(key,IPC_CREAT|0666)) < 0) 
		{
			dAppLog( LOG_CRI, "dMyQid msgget fail; key=0x%x,err=%d(%s)", key, errno, strerror(errno));
        	fprintf(stderr,"[rdrana_init] msgget fail; key=0x%x,err=%d(%s)\n",key,errno,strerror(errno));
        	return -1;
		}
	}

	if (conflib_getNthTokenInFileSection (fname, "[APPLICATIONS]", "RDRCAPD", 1, tmp) < 0) {
		dAppLog( LOG_CRI, "CAN'T GET MSGQ KEY OF RDRANA err=%s", strerror(errno));
		return -1;
	}
	else
	{
		key = strtol(tmp, 0, 0);
    	if ((dCAPDQid = msgget(key,IPC_CREAT|0666)) < 0) 
		{
			dAppLog( LOG_CRI, "dMyQid msgget fail; key=0x%x,err=%d(%s)", key, errno, strerror(errno));
        	fprintf(stderr,"[rdrana_init] msgget fail; key=0x%x,err=%d(%s)\n",key,errno,strerror(errno));
        	return -1;
		}
	}


	if (conflib_getNthTokenInFileSection (fname, "[APPLICATIONS]", "SMPP", 1, tmp) < 0) {
		dAppLog( LOG_CRI, "CAN'T GET MSGQ KEY OF SMPP err=%s", strerror(errno));
		return -1;
	}
	else
	{
		key = strtol(tmp, 0, 0);
    	if ((dSmppQid = msgget(key,IPC_CREAT|0666)) < 0) 
		{
			dAppLog( LOG_CRI, "SMPP msgget fail; key=0x%x,err=%d(%s)", key, errno, strerror(errno));
        	fprintf(stderr,"[rdrana_init] msgget fail; key=0x%x,err=%d(%s)\n",key,errno,strerror(errno));
        	return -1;
		}
	}

	if (conflib_getNthTokenInFileSection (fname, "[APPLICATIONS]", "IXPC", 1, tmp) < 0) {
		dAppLog( LOG_CRI, "CAN'T GET MSGQ KEY OF IXPC err=%s", strerror(errno));
		return -1;
	}
	else
	{
		key = strtol(tmp, 0, 0);
    	if ((dIxpcQid = msgget(key,IPC_CREAT|0666)) < 0) 
		{
			dAppLog( LOG_CRI, "IXPC msgget fail; key=0x%x,err=%d(%s)\n", key, errno, strerror(errno));
        	fprintf(stderr,"[rdrana_init] msgget fail; key=0x%x,err=%d(%s)\n",key,errno,strerror(errno));
        	return -1;
		}
	}

	if (conflib_getNthTokenInFileSection (fname, "[APPLICATIONS]", "MMCR", 1, tmp) < 0) {
		dAppLog( LOG_CRI, "CAN'T GET MSGQ KEY OF MMCR err=%s", strerror(errno));
		return -1;
	}
	else
	{
		key = strtol(tmp, 0, 0);
    	if ((dMmcrQid = msgget(key,IPC_CREAT|0666)) < 0) 
		{
			dAppLog( LOG_CRI, "MMCR msgget fail; key=0x%x,err=%d(%s)\n", key, errno, strerror(errno));
        	fprintf(stderr,"[rdrana_init] msgget fail; key=0x%x,err=%d(%s)\n",key,errno,strerror(errno));
        	return -1;
		}
	}

    if (conflib_getNthTokenInFileSection (fname, "[SHARED_MEMORY_KEY]", "SHM_LEG_SESS", 1, tmp) < 0) {    
        dAppLog( LOG_CRI, "CAN'T GET SHM KEY OF S_SSHM_NIFO err=%s", strerror(errno));     
        return -24;                                                                        
    }                                                                                      
    else                                                                                   
        key = strtol(tmp, 0, 0);                                                           


/* DEL : by june, 2010-10-03
 * DESC: TRACE 에 사용되는 SESSION 정보 참조 파트 주석 처리
	// WAITING RLEG HASHO CONSTRUCT COMPLETE. Don't Remove sleep(2)
	sleep(2);
	init_session (key);
 */

	// Trace Info 파일 Init  제거 -> 공유 메모리로 변경 

	// RULESET_LIST.conf 파일에서 PKG ID와 SMS On/Off 를 셋팅한다. 
	memset(fname, 0x00, sizeof(fname));
	sprintf(fname, "%s", PBTABLE_PATH);
	ret = MakeRuleSetList(fname);
	if( ret < 0 )
	{
		dAppLog(LOG_CRI, "MakeRuleSetList() fail...");
		return -1;
	}

	mysql_init(&sql);
	if ((conn = mysql_real_connect (&sql, "localhost", "root", "mysql", "mysql", 0, 0, 0)) == NULL)
	{
		dAppLog( LOG_CRI, "mysql connection fail", key, errno, strerror(errno));
		return -1;
	}

	dAppLog( LOG_CRI, "mysql local connection success", key, errno, strerror(errno));

    qsort ( (void*)mmcHdlrVector,
            numMmcHdlr,
            sizeof(RDRMmcHdlrVector),
            rdrana_mmcHdlrVector_qsortCmp );
	
	if( sms_get_conf() < 0 )
	{
		dAppLog( LOG_DEBUG, "[%s:%d] sms db connect configuration fail", __FUNCTION__, __LINE__ );
		return -1;
	}

	if( readSmsFile() < 0 )
	{
		dAppLog( LOG_DEBUG, "[%s:%d] sms  config file read fail", __FUNCTION__, __LINE__ );
		return -1;
	}

	select_Sce();

	init_RuleEntry();

	return 0;
}

void FinishProgram(void)
{
	dAppLog(LOG_CRI, "PROGRAM IS NORMALLY TERMINATED.");

	exit(0);
}

int select_Sce(void)
{
	MYSQL_RES   *res = NULL;
	MYSQL_ROW   row;
	char query[256];

	int i = 0;

	// SCE LIST
	sprintf(query,"select se_ip, value_key from INI_VALUES where value_type = 5");


	if( mysql_query(conn, query) != 0 )
	{
		dAppLog(LOG_DEBUG, "SELECT SCE Query Fail.");
		return -1;
	}

	res = mysql_store_result(conn);

	while( (row = mysql_fetch_row(res)) != NULL )
	{
		if(i > 2)
		{
			dAppLog(LOG_DEBUG, "NUMBER OF SCE is Over 2... Abnormal INI_VALUES table.");
			break;
		}
		sprintf(g_stSce[i].sce_ip, row[0]);
		sprintf(g_stSce[i].sce_name, row[1]);
		i++;
	}
	mysql_free_result(res);

	return 0;
}

void init_RuleEntry(void)
{
	int i = 0;

	for(i=0; i < MAX_RULE_ENTRY_LIST; i++)
	{
		memset(g_stEntry[i].entName,0x00,sizeof(g_stEntry[i].entName));
		g_stEntry[i].entId = -1;
	}
}

int	MakeRuleSetList(char *fname)
{
	char buf[64] = {0,};
	int No, Pbit, Hbit, PkgNo, RedNo, SmsOnOff;
	FILE *fp = NULL;
	char *pRuleSetList = ruleSetList;

    if( (fp = fopen(fname, "r")) != NULL )
    {
        while( fgets(buf, sizeof(buf), fp) )
        {
            if( buf[0] == '#' )
			{
                continue;
			}
            else
            {
			//  # No    PBIT    HBIT    PKGNO   REDNO   SMS
                sscanf( buf, "%d %d %d %d %d %d", &No, &Pbit, &Hbit, &PkgNo, &RedNo, &SmsOnOff );

				g_stRule[PkgNo].pkgNo = PkgNo;
				g_stRule[PkgNo].pBit = Pbit;
				g_stRule[PkgNo].hBit = Hbit;
                if( SmsOnOff == 1 )
				{
					pRuleSetList[PkgNo] = SmsOnOff;
					dAppLog(LOG_CRI, "SmsOnOff : %d , PkgNo ; %d, %02d %02d", SmsOnOff, \
							PkgNo, Pbit, Hbit);
				}
                else
					pRuleSetList[PkgNo] = SmsOnOff;
            }
        }
    }
    else
	{	
        dAppLog(LOG_CRI, "%s File Open fail....", PBTABLE_PATH);
		return -1;
	}

    fclose(fp);

    return 0;
}

int dGetSmsMessage (void)
{
    FILE    *fp;
    int     dIndex = 0;
    char    szBuf[1024];
    char    ppstData[64][64];
    //int     iRet = 0;
    int     iCount = 0;
    char    fileName[64] = {0,}, *env = NULL;


    if ((env = getenv(IV_HOME)) == NULL) {
        dAppLog(LOG_DEBUG,"dGetSmsMessage not found %s environment name", IV_HOME);
        return -1;
    }
    sprintf (fileName, "%s/%s", env, SMS_MSG_FILE);

    fp = fopen(fileName, "r");
    if( fp == NULL ) {
		dAppLog(LOG_DEBUG, "[ERROR] GET %s FILE NOT EXIST[%s][%s]",
				SMS_MSG_FILE, errno, strerror(errno));
		return -1;
    }

    while( fgets(szBuf, 1024, fp) != NULL )
    {
        if( dIndex >= MAX_TRACE_NUM )
            break;
        if(szBuf[0] == '#' || szBuf[0] == '\t'
            || szBuf[0] == ' ' || szBuf[0] == '\n')
            continue;

        if(!strcmp(ppstData[0], "@START")) {                                                                     
            continue;                                                         
        }   
        else if(!strcmp(ppstData[0], "@END")) {
            break;
        }


        if( iCount < 3 ) {
            return -1;
        }

        dIndex++;
    }

    fclose(fp);

    return 0;
}

int InitSHM_TRACE(void)
{
	int 	dRet;
	int  	key;
	char 	tmp[64], fname[256];

	sprintf(fname,"%s", DEF_SYSCONF_FILE );

	if (conflib_getNthTokenInFileSection (fname, "SHARED_MEMORY_KEY", "SHM_TRACE", 1, tmp) < 0)
		return -1;

	key = strtol(tmp,0,0);
	dRet = Init_shm(key, sizeof(st_SESSInfo), (void **)&gpTrcList[0]);
	if( dRet < 0 )
	{
		dAppLog( LOG_CRI, "FAIL %s(0) dRet=%d", __FUNCTION__, dRet);
		return -1;
	}

	if (conflib_getNthTokenInFileSection (fname, "SHARED_MEMORY_KEY", "SHM_TRACE1", 1, tmp) < 0)
		return -1;

	key = strtol(tmp,0,0);
	dRet = Init_shm(key, sizeof(st_SESSInfo), (void **)&gpTrcList[1]);
	if( dRet < 0 )
	{
		dAppLog( LOG_CRI, "FAIL %s(1) dRet=%d", __FUNCTION__, dRet);
		return -1;
	}
	return 0;
}
