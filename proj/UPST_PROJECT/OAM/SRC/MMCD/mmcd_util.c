/**
	@file		mmcd_util.h
	@author
	@version
	@date		2011-07-14
	@brief		mmcd_util.c 헤더파일
*/

/**
	Include headers
*/

/* SYS HEADER */
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
/* LIB HEADER */
#include "commdef.h"
#include "config.h"
#include "loglib.h"
#include "utillib.h"
/* PRO HEADER */
#include "path.h"
#include "mmcdef.h"
#include "msgdef.h"
#include "procid.h"
#include "sockio.h"
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
/* LOC HEADER */
#include "cmd_get.h"
#include "mmcd_mem.h"
#include "mmcd_util.h"

/**
	Define constants
*/
#define MMC_LOG               		LOG_PATH"MMC"
#define DIS_TREND_PARA_DELIMITER	","
#define MASK_VALUE					128

/**
	Declare variables
*/
static char				mtime_str[32];

extern st_MngPkt		cli_msg;			/*< mmcd_main.h */
extern T_ADMIN_LIST		stAdmin;			/*< mmcd_main.h */
extern RUN_TBL     		*run_tbl;			/*< mmcd_main.h */
extern int				gdClient;			/*< mmcd_main.h */
extern int 				gdNumfds;			/*< mmcd_main.h */
extern int 				gdNumWfds;			/*< mmcd_main.h */
extern int 				gdSvrSfd;			/*< mmcd_main.h */
extern st_ConTbl		stConTbl[];			/*< mmcd_main.h */
extern fd_set 			gstReadfds;			/*< mmcd_main.h */
extern fd_set			gstWritefds;		/*< mmcd_main.h */

/**
 *	Declare extern func.
 */
extern int dSendMessage(int dSsfd, int dMsgLen, char *szSmsg, int stConIdx);
extern int dSetSockBlock(int dReturn, int dSockFD, int stConIdx);
extern int GetUserInfo(char *UserName, int Type);
extern int dGetConTblIdx(int dSockfd);
extern int dInsert_MMCD_Result(st_MngPkt *pstMngPkt, short tmrID, char *szBuf, int dSockfd);
extern void clear_my_tmr(short i);

static int SaveInputCmd(char *input_comm);

/**
 *	Implement func.
 */
/*******************************************************************************
 ExceptionCommand for GET-CMD-INFO
*******************************************************************************/
int dIsExceptionCmd(char *cmd)
{
    char *szExCmd[] = { "DIS-TREND-INFO", "DIS-DEFECT-INFO", "ADD-FLT-CLT", "DEL-FLT-CLT", "ADD-FLT-SVR", "DEL-FLT-SVR",
                        "ADD-EQUIP-INFO", "DEL-EQUIP-INFO", "ADD-FLT-SCTP", "DEL-FLT-SCTP" };
    int  i, len = sizeof(szExCmd)/sizeof(char *);

    for( i = 0; i < len; i++ ){
        if( !strncasecmp(szExCmd[i], cmd, sizeof(szExCmd[i])) )
            return i+1;
    }
    return 0;

}

/*******************************************************************************
 ExceptionCommand @help, ?
*******************************************************************************/
int dIsExceptionCmdSe(char *cmd)
{
    char *szExCmd[] = { "USER-LOGIN", "USER-LOGOUT", "KILL-USER", "GET-CMD-INFO" };
	int  dRet, i, len = sizeof(szExCmd)/sizeof(char *);

	for( i = 0; i < len; i++ ){
		//if( !strcasecmp(szExCmd[i], cmd) )
        if( !strncasecmp(szExCmd[i], cmd, sizeof(szExCmd[i])) )
			return i+1;
	}

	dRet = dIsExceptionCmd(cmd);
	return !dRet? dRet: dRet +len;
}

/*******************************************************************************
 MAKE HEAD
*******************************************************************************/
char  *ComposeHead()
{
    static char  buf[200];   /* buf must be static type */

    strcpy (buf, (char *)mtime());
	strcat (buf, "\n");
    //strcat (buf, " ON MP\n");
    buf[strlen (buf)] = 0;

    return buf;
}

/*******************************************************************************
 SEND RESULT IN dRcvPkt_Handle FUNCTION (mmcd_mem.c)
*******************************************************************************/
int send_text_form (int osfd, char *sBuf, int endFlag, int dConTblIdx )
{
	int			dRet, dRet1, dLen;
	st_MngPkt	smesg;

	memset (&smesg, 0, sizeof (st_MngPkt));

	dLen = strlen(sBuf);
	log_print(LOGN_INFO, "SFD:[%d] LEN:[%d] BODY[%s]", osfd, dLen, sBuf);

	if(dLen >= MAX_MNGPKT_BODY_SIZE) {
		log_print(LOGN_WARN, "SEND COM DATA BUFFER OVERFLOW MAX=%d:CUR=%d", MAX_MNGPKT_BODY_SIZE, dLen);
		dLen = MAX_MNGPKT_BODY_SIZE - 1;
	}

    memcpy(smesg.data, sBuf, dLen);
	smesg.data[dLen] = 0x00;

	smesg.head.llMagicNumber = MAGIC_NUMBER;
	smesg.head.llIndex = cli_msg.head.llIndex;
	smesg.head.usResult = endFlag;
	smesg.head.usBodyLen = dLen;
	strcpy(smesg.head.userName, cli_msg.head.userName);
	memcpy(smesg.head.TimeStamp, cli_msg.head.TimeStamp,
								sizeof (cli_msg.head.TimeStamp));

	memcpy(smesg.head.ucmmlid, cli_msg.head.ucmmlid,
								sizeof (cli_msg.head.ucmmlid));

	smesg.head.ucBinFlag = cli_msg.head.ucBinFlag;
	smesg.head.usSrcProc = cli_msg.head.usSrcProc;
	smesg.head.usTotLen = MNG_PKT_HEAD_SIZE + smesg.head.usBodyLen;

	smesg.head.ucSvcID = SID_MML;
	smesg.head.ucMsgID = MID_MML_RST;

	smesg.head.usTotPage = 1;
    smesg.head.usCurPage = 1;
	smesg.head.usStatFlag = 0;

	memcpy(smesg.head.userName, cli_msg.head.userName, MAX_USER_NAME_LEN );

	dRet = dSendMessage( osfd, smesg.head.usTotLen, (char*)&smesg, dConTblIdx );
	dRet1 = dSetSockBlock( dRet, osfd, dConTblIdx );

	return 1;
}

/*******************************************************************************
 SEND RESULT IN MMCD COMMAND CONTROL FUNCTION (cmd_*.c)
*******************************************************************************/
int send_text_user (int osfd, char *sBuf, int endFlag, mml_msg *ml, int dResult, int dConTblIdx )
{
	int			dRet, dRet1, dLen, dNmsSfd, dNmsUserIdx, dConTbl;
    st_MngPkt   smesg;

    memset (&smesg, 0, sizeof (st_MngPkt));

	dLen = strlen(sBuf);
	log_print(LOGN_INFO, "SFD:[%d] LEN:[%d] BODY[%s]", osfd, dLen, sBuf);

	if(dLen >= MAX_MNGPKT_BODY_SIZE) {
		log_print(LOGN_WARN, "SEND COM DATA BUFFER OVERFLOW MAX=%d:CUR=%d", MAX_MNGPKT_BODY_SIZE, dLen);
		dLen = MAX_MNGPKT_BODY_SIZE - 1;
	}

    memcpy(smesg.data, sBuf, dLen);
	smesg.data[dLen] = 0x00;

    smesg.head.llMagicNumber = MAGIC_NUMBER;
    smesg.head.llIndex = cli_msg.head.llIndex;
    smesg.head.usResult = endFlag;
    smesg.head.usBodyLen = dLen;
    strcpy(smesg.head.userName, cli_msg.head.userName);
    memcpy(smesg.head.TimeStamp, cli_msg.head.TimeStamp,
                                sizeof (cli_msg.head.TimeStamp));

    memcpy(smesg.head.ucmmlid, cli_msg.head.ucmmlid,
                                sizeof (cli_msg.head.ucmmlid));

    smesg.head.ucBinFlag = cli_msg.head.ucBinFlag;
    smesg.head.usSrcProc = cli_msg.head.usSrcProc;
    smesg.head.usTotLen = MNG_PKT_HEAD_SIZE + smesg.head.usBodyLen;

    smesg.head.ucSvcID = SID_MML;
    smesg.head.ucMsgID = MID_MML_RST;

	smesg.head.usTotPage = 1;
    smesg.head.usCurPage = 1;
	smesg.head.usStatFlag = 0;

	memcpy(smesg.head.userName, cli_msg.head.userName, MAX_USER_NAME_LEN );

	dNmsSfd = dGetNMSsfd();
	dNmsUserIdx = GetUserInfo( "ntasnms", 1 );

    dRet = dSendMessage( osfd, smesg.head.usTotLen, (char*)&smesg, dConTblIdx );
	dRet1 = dSetSockBlock( dRet, osfd, dConTblIdx );

	if( dNmsSfd > 0 && dNmsSfd != osfd && stAdmin.dConnectFlag[dNmsUserIdx] == 1 ) {
		dConTbl = dGetConTblIdx( dNmsSfd );
		dRet = dSendMessage( dNmsSfd, smesg.head.usTotLen, (char*)&smesg, dConTbl );
		dRet1 = dSetSockBlock( dRet, dNmsSfd, dConTbl );
	}
	dRet = dInsert_MMCD_Result(&smesg, ml->cmd_id, sBuf, osfd);

    return 1;
}

/*******************************************************************************
 SEND RESULT IN HISTORY FUNCTION
*******************************************************************************/
int send_text_his (int osfd, char *sBuf, int curpage, int totpage, mml_msg *ml, int dConTblIdx )
{
	int			dRet, dRet1, dLen, dNmsSfd, dNmsUserIdx, dConTbl;
    st_MngPkt   smesg;

    memset (&smesg, 0, sizeof (st_MngPkt));

	dLen = strlen(sBuf);
	log_print(LOGN_INFO, "SFD:[%d] LEN:[%d] BODY[%s]", osfd, dLen, sBuf);

	if(dLen >= MAX_MNGPKT_BODY_SIZE) {
		log_print(LOGN_WARN, "SEND COM DATA BUFFER OVERFLOW MAX=%d:CUR=%d", MAX_MNGPKT_BODY_SIZE, dLen);
		dLen = MAX_MNGPKT_BODY_SIZE - 1;
	}

    memcpy(smesg.data, sBuf, dLen);
	smesg.data[dLen] = 0x00;

    smesg.head.llMagicNumber = MAGIC_NUMBER;
    smesg.head.llIndex = cli_msg.head.llIndex;

	if( curpage == totpage )
		smesg.head.usResult = DBM_CONTINUE;
	else
		smesg.head.usResult = DBM_END;

    smesg.head.usBodyLen = dLen;
    strcpy(smesg.head.userName, cli_msg.head.userName);
    memcpy(smesg.head.TimeStamp, cli_msg.head.TimeStamp,
                                sizeof (cli_msg.head.TimeStamp));

    memcpy(smesg.head.ucmmlid, cli_msg.head.ucmmlid,
                                sizeof (cli_msg.head.ucmmlid));

    smesg.head.ucBinFlag = cli_msg.head.ucBinFlag;
    smesg.head.usSrcProc = cli_msg.head.usSrcProc;
    smesg.head.usTotLen = MNG_PKT_HEAD_SIZE + smesg.head.usBodyLen;

    smesg.head.ucSvcID = SID_MML;
    smesg.head.ucMsgID = MID_MML_RST;

    smesg.head.usTotPage = totpage;
    smesg.head.usCurPage = curpage;
	smesg.head.usStatFlag = 0;

    memcpy(smesg.head.userName, cli_msg.head.userName, MAX_USER_NAME_LEN );

    dRet = dSendMessage( osfd, smesg.head.usTotLen, (char*)&smesg, dConTblIdx );
	dRet1 = dSetSockBlock( dRet, osfd, dConTblIdx );

    dNmsSfd = dGetNMSsfd();
	dNmsUserIdx = GetUserInfo( "ntasnms", 1 );

    if( dNmsSfd > 0 && dNmsSfd != osfd && stAdmin.dConnectFlag[dNmsUserIdx] == 1 )
    {
		dConTbl = dGetConTblIdx( dNmsSfd );

		log_print(LOGN_INFO, "NMS SEND SFD[%d] CONTBL[%d]", dNmsSfd, dConTbl );

        dRet = dSendMessage( dNmsSfd, smesg.head.usTotLen, (char*)&smesg, dConTbl );

		log_print(LOGN_INFO, "NMS SEND END");
		dRet1 = dSetSockBlock( dRet, dNmsSfd, dConTbl );
    }
	dRet = dInsert_MMCD_Result(&smesg, ml->cmd_id, sBuf, osfd);

    return 1;
}

/*******************************************************************************
 SEND RESULT TO NMS IN GRAMMER CHECK
*******************************************************************************/
int send_text_ack (int osfd, char *sBuf, int endFlag, int dConTblIdx )
{
	int			dRet, dRet1, dLen;
    st_MngPkt   smesg;

    memset (&smesg, 0, sizeof (st_MngPkt));

	dLen = strlen(sBuf);
	log_print(LOGN_INFO, "SFD:[%d] LEN:[%d] BODY[%s]", osfd, dLen, sBuf);

	if(dLen >= MAX_MNGPKT_BODY_SIZE) {
		log_print(LOGN_WARN, "SEND COM DATA BUFFER OVERFLOW MAX=%d:CUR=%d", MAX_MNGPKT_BODY_SIZE, dLen);
		dLen = MAX_MNGPKT_BODY_SIZE - 1;
	}

    memcpy(smesg.data, sBuf, dLen);
	smesg.data[dLen] = 0x00;

    smesg.head.llMagicNumber = MAGIC_NUMBER;
    smesg.head.llIndex = cli_msg.head.llIndex;
    smesg.head.usResult = endFlag;
    smesg.head.usBodyLen = dLen;
    strcpy(smesg.head.userName, cli_msg.head.userName);
    memcpy(smesg.head.TimeStamp, cli_msg.head.TimeStamp,
                                sizeof (cli_msg.head.TimeStamp));

    memcpy(smesg.head.ucmmlid, cli_msg.head.ucmmlid,
                                sizeof (cli_msg.head.ucmmlid));

    smesg.head.ucBinFlag = cli_msg.head.ucBinFlag;
    smesg.head.usSrcProc = cli_msg.head.usSrcProc;
    smesg.head.usTotLen = MNG_PKT_HEAD_SIZE + smesg.head.usBodyLen;

    smesg.head.ucSvcID = SID_MML;
    smesg.head.ucMsgID = MID_MML_ACK;

	smesg.head.usTotPage = 1;
    smesg.head.usCurPage = 1;
	smesg.head.usStatFlag = 0;

	memcpy(smesg.head.userName, cli_msg.head.userName, MAX_USER_NAME_LEN );

    dRet = dSendMessage( osfd, smesg.head.usTotLen, (char*)&smesg, dConTblIdx );
	dRet1 = dSetSockBlock( dRet, osfd, dConTblIdx );

    return 1;
}

/*******************************************************************************
 SEND RESULT IN LOGIN
*******************************************************************************/
int send_text_login (int osfd, char *sBuf, int dUserLevel, int endFlag, mml_msg *ml, int dConTblIdx )
{
	int			dRet, dRet1, dLen, dNmsSfd, dNmsUserIdx, dConTbl;
    st_MngPkt   smesg;

    memset (&smesg, 0, sizeof (st_MngPkt));

	dLen = strlen(sBuf);
	log_print(LOGN_INFO, "SFD:[%d] LEN:[%d] BODY[%s]", osfd, dLen, sBuf);

	if(dLen >= MAX_MNGPKT_BODY_SIZE) {
		log_print(LOGN_WARN, "SEND COM DATA BUFFER OVERFLOW MAX=%d:CUR=%d", MAX_MNGPKT_BODY_SIZE, dLen);
		dLen = MAX_MNGPKT_BODY_SIZE - 1;
	}

    memcpy(smesg.data, sBuf, dLen);
	smesg.data[dLen] = 0x00;

    smesg.head.llMagicNumber = MAGIC_NUMBER;
    smesg.head.llIndex = cli_msg.head.llIndex;
    smesg.head.usResult = endFlag;
    smesg.head.usBodyLen = dLen;
    strcpy(smesg.head.userName, cli_msg.head.userName);
    memcpy(smesg.head.TimeStamp, cli_msg.head.TimeStamp,
                                sizeof (cli_msg.head.TimeStamp));

    memcpy(smesg.head.ucmmlid, cli_msg.head.ucmmlid,
                                sizeof (cli_msg.head.ucmmlid));

    smesg.head.ucBinFlag = cli_msg.head.ucBinFlag;
    smesg.head.usSrcProc = cli_msg.head.usSrcProc;
    smesg.head.usTotLen = MNG_PKT_HEAD_SIZE + smesg.head.usBodyLen;

	smesg.head.usReserved = dUserLevel;

    smesg.head.ucSvcID = SID_MML;
    smesg.head.ucMsgID = MID_MML_RST;

	smesg.head.usStatFlag = 0;

	memcpy(smesg.head.userName, cli_msg.head.userName, MAX_USER_NAME_LEN );

    dRet = dSendMessage( osfd, smesg.head.usTotLen, (char*)&smesg, dConTblIdx );
	dRet1 = dSetSockBlock( dRet, osfd, dConTblIdx );

	dNmsSfd = dGetNMSsfd();
	dNmsUserIdx = GetUserInfo( "ntasnms", 1 );

    if( dNmsSfd > 0 && dNmsSfd != osfd && stAdmin.dConnectFlag[dNmsUserIdx] == 1 )
    {
		dConTbl = dGetConTblIdx( dNmsSfd );

        dRet = dSendMessage( dNmsSfd, smesg.head.usTotLen, (char*)&smesg, dConTbl );
		dRet1 = dSetSockBlock( dRet, dNmsSfd, dConTbl );
    }

	sprintf( smesg.data, "%s\n", smesg.data );
	smesg.head.usBodyLen = strlen(smesg.data);
	smesg.head.usTotLen = MNG_PKT_HEAD_SIZE + smesg.head.usBodyLen;

	dRet = dInsert_MMCD_Result(&smesg, ml->cmd_id, sBuf, osfd);

    return 1;
}

int send_com_data (int osfd, char *sBuf, int endFlag, mml_msg *ml, int dResult, int dConTblIdx )
{
    int         dRet, dRet1, dLen;
    st_MngPkt   smesg;

	dLen	= 0;
    memset (&smesg, 0, sizeof (st_MngPkt));

	log_print(LOGN_INFO, "SFD:[%d] LEN:[%d] BODY[%s]", osfd, dLen, sBuf);

	dLen = strlen(sBuf);
	if(dLen >= MAX_MNGPKT_BODY_SIZE) {
		log_print(LOGN_WARN, "SEND COM DATA BUFFER OVERFLOW MAX=%d:CUR=%d", MAX_MNGPKT_BODY_SIZE, dLen);
		dLen = MAX_MNGPKT_BODY_SIZE - 1;
	}

    memcpy(smesg.data, sBuf, dLen);
	smesg.data[dLen] = 0x00;

    smesg.head.llMagicNumber = MAGIC_NUMBER;
    smesg.head.llIndex = cli_msg.head.llIndex;
    smesg.head.usResult = endFlag;
    smesg.head.usBodyLen = dLen;
    strcpy(smesg.head.userName, cli_msg.head.userName);
    memcpy(smesg.head.TimeStamp, cli_msg.head.TimeStamp,
                                sizeof (cli_msg.head.TimeStamp));

    memcpy(smesg.head.ucmmlid, cli_msg.head.ucmmlid,
                                sizeof (cli_msg.head.ucmmlid));

    smesg.head.ucBinFlag = cli_msg.head.ucBinFlag;
    smesg.head.usSrcProc = cli_msg.head.usSrcProc;
    smesg.head.usTotLen = MNG_PKT_HEAD_SIZE + smesg.head.usBodyLen;

    smesg.head.ucSvcID = SID_MML;
    smesg.head.ucMsgID = MID_MML_RST;

    smesg.head.usTotPage = 1;
    smesg.head.usCurPage = 1;
    smesg.head.usStatFlag = 0;

    memcpy(smesg.head.userName, cli_msg.head.userName, MAX_USER_NAME_LEN );

	log_print( LOGN_INFO, "BODY LENGTH:[%d] TOT LENGTH:[%d]",
		smesg.head.usBodyLen, smesg.head.usTotLen );

    dRet = dSendMessage( osfd, smesg.head.usTotLen, (char*)&smesg, dConTblIdx );
    dRet1 = dSetSockBlock( dRet, osfd, dConTblIdx );

	log_print( LOGN_INFO, "SEND DQMS_COM DATA");

    return 1;
}

/*******************************************************************************

*******************************************************************************/
int output_log (char *pbuf)
{
    int     err_code;

    err_code = SaveInputCmd (pbuf);

    if (err_code < 0)
	{
        log_print(LOGN_DEBUG,"[General Error] Fail To Save Command History\n\n");
        return -1;
    }
    return 1;
}

/*******************************************************************************
 CONVERT CURRENT TIME TO STRING
*******************************************************************************/
char *mtime()
{
    time_t t;
    t = time(&t);
    strftime(mtime_str, 32, "%Y-%m-%d %T %a", localtime((time_t *)&t));
    mtime_str[21] = toupper(mtime_str[21]);
    mtime_str[22] = toupper(mtime_str[22]);
    return mtime_str;
}

/*******************************************************************************
 CONVERT TIME TO STRING
*******************************************************************************/
char *time_str( time_t time )
{
	strftime(mtime_str, 32, "%Y/%m/%d %T %a", localtime((time_t *)&time));
	mtime_str[21] = toupper(mtime_str[21]);
    mtime_str[22] = toupper(mtime_str[22]);
    return mtime_str;
}

char *time_short( time_t time )
{
	strftime(mtime_str, 32, "%Y/%m/%d %T %a", localtime((time_t *)&time));
    mtime_str[21] = toupper(mtime_str[21]);
    mtime_str[22] = toupper(mtime_str[22]);

	memcpy( mtime_str, &mtime_str[5], 15 );
	mtime_str[14] = 0x00;
    return mtime_str;
}

/*******************************************************************************
 STRING COMPARE
*******************************************************************************/
int strcasecmp(const char *a, const char *b)
{
	while (*a && *b)
	{
		if (toupper(*a) - toupper(*b))
			return 1;
		a++, b++;
	}

	if (*a == *b)
		return 0;
	return 1;
}

/*******************************************************************************
 STRING COMPARE BY N
*******************************************************************************/
int strncasecmp(const char *a, const char *b, size_t n)
{
	int	i;

	for (i = 0;*a && *b && i < n;i++)
	{
		if (toupper(*a) - toupper(*b))
			return 1;
		a++, b++;
	}

	if (i == n || *a == *b)
		return 0;
	return 1;
}

/*******************************************************************************

*******************************************************************************/
int SaveInputCmd(char *input_comm)
{
    FILE    *fptr;
    char    dir_path[80], mesg_path[80];
    struct	stat stat_log;
    time_t  nowtime;
    struct  tm  check_time, log_time;

    time(&nowtime);
    localtime_r(&nowtime, &check_time);

    sprintf(mesg_path, "%s%d/%d.%d", MMC_LOG, check_time.tm_mon + 1,
            check_time.tm_mday, check_time.tm_hour);

    if (stat(mesg_path, &stat_log) == 0) {
        localtime_r(&stat_log.st_atime, &log_time);
        if (check_time.tm_year != log_time.tm_year) {
            log_print(LOGN_CRI,"[MMCD] DELETE MMC LOG FILE = %s \n\n", mesg_path);
            unlink(mesg_path);
        }
    } else {
        sprintf(dir_path, "%s%d", MMC_LOG, check_time.tm_mon + 1);
        mkdir(dir_path, 0777);
    }

    if ((fptr = fopen(mesg_path, "a+")) == NULL)
        return -1;

    fprintf (fptr, "%s\n\n", input_comm);

    fclose (fptr);
    return 1;
}

/*******************************************************************************
 GET INDEX FOR RUN TABLE
*******************************************************************************/
int dGetRunTblIdx( int sfd )
{
	int		i;

	for(i=0; i<MAX_TMR_REC; i++)
	{
		if( sfd == run_tbl->sfd[i] )
			return i;
	}

	return -1;
}

/*******************************************************************************

*******************************************************************************/
int dMake_Dir( struct tm *Tm, char *szLogPath )
{
	int		dRet;
	char	szLogFilePath[128];

	sprintf( szLogFilePath, "%s", DEF_HIS_PATH );

	/*
    * MAKE MONTH PATH
    */
    sprintf( szLogFilePath, "%s/%02d", szLogFilePath, Tm->tm_mon+1 );

    dRet = mkdir( szLogFilePath, 0755 );
    if( dRet < 0 )
    {
        if( errno != EEXIST )
        {
            log_print(LOGN_CRI,"[ERROR] MAKE MON DIRECTORY [%d] [%s]", errno, strerror(errno) );

            return -1;
        }
    }

    /*
    * MAKE DAY PATH
    */
    sprintf( szLogFilePath, "%s/%02d", szLogFilePath, Tm->tm_mday );

    dRet = mkdir( szLogFilePath, 0755 );
    if( dRet < 0 )
    {
        if( errno != EEXIST )
        {
            log_print(LOGN_CRI,"[ERROR] MAKE MON DIRECTORY [%d] [%s]", errno, strerror(errno) );

            return -1;
        }
    }

	/*
    * MAKE LAST LOG PATH
    */
    sprintf( szLogFilePath, "%s/%s", szLogFilePath, szLogPath );

    dRet = mkdir( szLogFilePath, 0755 );
    if( dRet < 0 )
    {
        if( errno != EEXIST )
        {
            log_print(LOGN_CRI,"[ERROR] MAKE LAST LOG[%s] DIRECTORY [%d] [%s]", szLogPath, errno, strerror(errno) );

            return -1;
        }
    }

	return 1;
}

/*******************************************************************************
 CHECK FOR NMS CONNECTION
*******************************************************************************/
int dCheckNMS( int dSfd )
{
    int         i;

    for(i=0; i<gdClient; i++)
    {
        if( dSfd == stConTbl[i].dSfd )
        {
            if( !strcmp(stConTbl[i].adminid, "ntasnms") )
                return 1;
            else
                return -1;
        }
    }

    if( i == MAX_FDSET2 )
        return -1;

	return 0;
}

/*******************************************************************************
 GET NMS SOCKET FD
*******************************************************************************/
int dGetNMSsfd()
{
    int         i;
    int         dSfd;

    for(i=0,dSfd = 0 ; i<gdClient; i++)
    {
        if( !strcmp( stConTbl[i].adminid, "ntasnms" ) )
        {
            dSfd = stConTbl[i].dSfd;
			break;
        }
    }

    if( i == MAX_FDSET2 )
        dSfd = -1;

    return dSfd;
}

int dGetConTblIdx(int dSockfd)
{
	int		i;

	for(i = 0; i < gdClient; i++)
	{
		if(dSockfd == stConTbl[i].dSfd)
			break;
	}

	if(i == gdClient)
		return -1;
	else
		return i;
}

/*******************************************************************************

*******************************************************************************/
void CheckClient( int dReloadFlag )
{
	int				i, j, dSfd, dAdminIdx, dRet;
	unsigned int	dReloadSOLIBFlag = 99;
	st_MngPkt		smesg;
	fd_set			stWd;

    memset(&smesg, 0x00, sizeof(st_MngPkt));

    smesg.head.llMagicNumber	= MAGIC_NUMBER;
    smesg.head.llIndex			= 0;
    smesg.head.usResult			= DBM_SUCCESS;
    smesg.head.ucSvcID			= SID_MML;
    smesg.head.ucMsgID			= MID_MML_CHK;

	/*
	* CONFORM TO OMP ReloadSOLIB
	*/
	if(dReloadFlag != 0)
		memcpy(&smesg.head.ucmmlid[0], &dReloadSOLIBFlag, sizeof(unsigned int));

	smesg.head.usBodyLen	= 0;
	smesg.head.usSrcProc	= SEQ_PROC_MMCD;
	smesg.head.usTotLen		= MNG_PKT_HEAD_SIZE + smesg.head.usBodyLen;

	/*	EVENT INVOKE	*/
	for(i = 0; i < gdClient; i++)
	{
		if( (stConTbl[i].dSfd>0) && (stConTbl[i].cSockBlock==0x00))
		{
			log_print(LOGN_INFO, "CHECK CLIENT NAME[%s]", stConTbl[i].adminid);

			dSfd	= stConTbl[i].dSfd;
			strcpy(smesg.head.userName, stConTbl[i].adminid);

			if( (dRet = send(dSfd, (char*)&smesg, smesg.head.usTotLen, MSG_NOSIGNAL)) < 0)
			{
				log_print(LOGN_DEBUG, "F=%s:%s.%d: ERROR IN send(dSfd[%d]) errno[%d-%s]", __FILE__, __FUNCTION__, __LINE__, dSfd, errno, strerror(errno));

				if(errno == EAGAIN)
					continue;
				else
				{
					/*	CLOSE SOCKET	*/
					close(dSfd);
					FD_CLR(dSfd, &gstReadfds);

					log_print(LOGN_DEBUG, "GDCLIENT INFO: [%d]", gdClient);

					if(gdClient == 1)
					{
						gdClient--;
						gdNumfds = gdSvrSfd;
						log_print(LOGN_DEBUG, "GDNumfds INFO 3: [%d]", gdNumfds);
					}
					else if( (gdClient-1) == i)
					{
						gdClient--;

						while(1)
						{
							if(stConTbl[gdClient-1].dSfd > 0)
							{
								gdNumfds = dGetMaxFds();
								break;
							}
							else
							{
								gdClient--;

								if(gdClient == 0)
								{
									log_print(LOGN_DEBUG, "GDNumfds INFO 4: [%d]", gdNumfds);
									gdNumfds = gdSvrSfd;
									break;
								}
							}
						}
					}

					memcpy((char*)&stWd, (char*)&gstWritefds, sizeof(fd_set));
					if(FD_ISSET(dSfd, &stWd))
					{
						log_print(LOGN_DEBUG, "CLEAR WRITEFDS FOR SOCKET CLOSE SFD[%d]", dSfd);
						FD_CLR(dSfd, &gstWritefds);
						gdNumWfds--;
						stConTbl[i].cSockBlock = 0x00;
					}

					log_print(LOGN_DEBUG, "GDCLIENT INFO: [%d]", gdClient);

					stConTbl[i].dSfd	= -1;
					stConTbl[i].dBufLen	= 0;
					stConTbl[i].uiCliIP	= 0;

					dAdminIdx = GetUserInfo(stConTbl[i].adminid, 1);
					stAdmin.dConnectFlag[dAdminIdx] = 0;

					stConTbl[i].adminid[0] = 0x00;
					for(j = 0; j < MAX_TMR_REC; j++)
					{
						if(dSfd == run_tbl->sfd[j])
							clear_my_tmr(j);
					}
				}
			}
			else
			{
				if(dRet != smesg.head.usTotLen)
				{
					log_print(LOGN_CRI, "WRITE MESSAGE SIZE ERROR [%d] [%d] BUFFER FULL IN CLIENT CHECK", dRet, smesg.head.usTotLen);

					/*	ADD WRITE FD_SET	*/
					FD_SET(dSfd, &gstWritefds);
					gdNumWfds++;

					stConTbl[i].cSockBlock = 0x01;

					stConTbl[i].dBufLen = smesg.head.usTotLen - dRet;

					memcpy(&stConTbl[i].szSendBuffer[0], (char*)&smesg+dRet, (smesg.head.usTotLen-dRet));
					stConTbl[i].dBufLen = (smesg.head.usTotLen-dRet);
				}
			}
		}
	}
}

/*******************************************************************************
 SET FIDB VALUE IN MASK STATUS
*******************************************************************************/
void SetFIDBValue( unsigned char *ucFIDB, unsigned char ucNEW )
{
    unsigned char ucTmp;

    ucTmp = *ucFIDB;

    if( ucTmp >= MASK_VALUE ) {
        ucTmp = ucNEW;
        ucTmp |= 0x80;
    }
    else {
        ucTmp = ucNEW;
    }

    *ucFIDB = ucTmp;
}

/*******************************************************************************
 GET MAXIMUM FDS, SET gdNumfds.
*******************************************************************************/
int dGetMaxFds()
{
	int			i;
	int			dMaxFds = 0;

	for( i=0; i<gdClient; i++)
	{
		log_print(LOGN_DEBUG, "[dGetMaxFds] stConTbl IDX[%d], SFD[%d], IP[%s]", i, stConTbl[i].dSfd, util_cvtipaddr(NULL, stConTbl[i].uiCliIP) );
		if( dMaxFds < stConTbl[i].dSfd )
			dMaxFds = stConTbl[i].dSfd;
	}

	/*
    * CHECK MAX FDS INCLUDING SERVER FDS
    * ADDED 2003.09.23
    */
    if( gdSvrSfd > dMaxFds )
    	dMaxFds = gdSvrSfd;

	log_print(LOGN_DEBUG, "[dGetMaxFds] MAX SFD[%d]", dMaxFds );

	return dMaxFds;
}

int cCheckTrendCmd(char cType, char *sCmdStr)
{
	char			sCmd[BUF_SIZE], sTmpStr[BUF_SIZE], *pStr;
	size_t			szCurStr, szCmdStr;
	st_TrendInfo	stTrendInfo;

	szCurStr	= 0;
	szCmdStr	= strlen(sCmdStr);
	memset(sTmpStr, 0x00, BUF_SIZE);
	sprintf(sTmpStr, "%s", sCmdStr);

	pStr = sTmpStr;
	pStr = strtok(pStr, " ");
	sprintf(sCmd, "%s", pStr);

	if( ((szCurStr += strlen(pStr)+1) < szCmdStr) && ((pStr = &sCmdStr[szCurStr]) != NULL) && (strtok(pStr, DIS_TREND_PARA_DELIMITER) != NULL))
		stTrendInfo.ucOfficeID	= (unsigned char)atoi(pStr);
	else
		return -1;

	if( ((szCurStr += strlen(pStr)+1) < szCmdStr) && ((pStr = &sCmdStr[szCurStr]) != NULL) && (strtok(pStr, DIS_TREND_PARA_DELIMITER) != NULL))
		stTrendInfo.ucSysType	= (unsigned char)atoi(pStr);
	else
		return -2;

	if( ((szCurStr += strlen(pStr)+1) < szCmdStr) && ((pStr = &sCmdStr[szCurStr]) != NULL) && (strtok(pStr, DIS_TREND_PARA_DELIMITER) != NULL))
		stTrendInfo.ucSubType	= (unsigned char)atoi(pStr);
	else
		return -3;

	if( ((szCurStr += strlen(pStr)+1) < szCmdStr) && ((pStr = &sCmdStr[szCurStr]) != NULL) && (strtok(pStr, DIS_TREND_PARA_DELIMITER) != NULL))
		stTrendInfo.usL4Code	= (unsigned short)atoi(pStr);
	else
		return -4;

	if( ((szCurStr += strlen(pStr)+1) < szCmdStr) && ((pStr = &sCmdStr[szCurStr]) != NULL) && (strtok(pStr, DIS_TREND_PARA_DELIMITER) != NULL))
		stTrendInfo.uiIP		= (unsigned int)atoi(pStr);
	else
		return -5;

	if( ((szCurStr += strlen(pStr)+1) < szCmdStr) && ((pStr = &sCmdStr[szCurStr]) != NULL) && (strtok(pStr, DIS_TREND_PARA_DELIMITER) != NULL))
		stTrendInfo.ucSYSID		= (unsigned char)atoi(pStr);
	else
		return -6;

	if( ((szCurStr += strlen(pStr)+1) < szCmdStr) && ((pStr = &sCmdStr[szCurStr]) != NULL) && (strtok(pStr, DIS_TREND_PARA_DELIMITER) != NULL))
		stTrendInfo.ucBSCID		= (unsigned char)atoi(pStr);
	else
		return -7;

	if( ((szCurStr += strlen(pStr)+1) < szCmdStr) && ((pStr = &sCmdStr[szCurStr]) != NULL) && (strtok(pStr, DIS_TREND_PARA_DELIMITER) != NULL))
		stTrendInfo.usBTSID		= (unsigned short)atoi(pStr);
	else
		return -8;

	if( ((szCurStr += strlen(pStr)+1) < szCmdStr) && ((pStr = &sCmdStr[szCurStr]) != NULL) && (strtok(pStr, DIS_TREND_PARA_DELIMITER) != NULL))
		stTrendInfo.ucFAID		= (unsigned char)atoi(pStr);
	else
		return -9;

	if( ((szCurStr += strlen(pStr)+1) < szCmdStr) && ((pStr = &sCmdStr[szCurStr]) != NULL) && (strtok(pStr, DIS_TREND_PARA_DELIMITER) != NULL))
		stTrendInfo.ucSECID		= (unsigned char)atoi(pStr);
	else
		return -10;

	if(cType == TYPE_DEFECT_INFO)
	{
		if( ((szCurStr += strlen(pStr)+1) < szCmdStr) && ((pStr = &sCmdStr[szCurStr]) != NULL) && (strtok(pStr, DIS_TREND_PARA_DELIMITER) != NULL))
			stTrendInfo.ucDefectCode	= (unsigned char)atoi(pStr);
		else
			return -11;

		sprintf(sTmpStr, "%s %hu,%hu,%hu,%hu,%s,%hu,%hu,%hu,%hu,%hu,%hu", sCmd, stTrendInfo.ucOfficeID, stTrendInfo.ucSysType, stTrendInfo.ucSubType,
			stTrendInfo.usL4Code, util_cvtipaddr(NULL, stTrendInfo.uiIP), stTrendInfo.ucSYSID, stTrendInfo.ucBSCID, stTrendInfo.usBTSID, stTrendInfo.ucFAID,
			stTrendInfo.ucSECID, stTrendInfo.ucDefectCode);
	}
	else
	{
		sprintf(sTmpStr, "%s %hu,%hu,%hu,%hu,%s,%hu,%hu,%hu,%hu,%hu", sCmd, stTrendInfo.ucOfficeID, stTrendInfo.ucSysType, stTrendInfo.ucSubType,
			stTrendInfo.usL4Code, util_cvtipaddr(NULL, stTrendInfo.uiIP), stTrendInfo.ucSYSID, stTrendInfo.ucBSCID, stTrendInfo.usBTSID, stTrendInfo.ucFAID, stTrendInfo.ucSECID);
	}

	strcpy(sCmdStr, sTmpStr);

	return 0;
}
