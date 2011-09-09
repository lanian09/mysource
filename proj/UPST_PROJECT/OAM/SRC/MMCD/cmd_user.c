/**
	@file		cmd_user.c
	@author
	@version
	@date		2011-07-18
	@brief		cmd_user.c
*/

/**
 *	Include headers
 */

/* SYS HEADER */
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <mysql/mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
/* LIB HEADER */
#include "commdef.h"
#include "loglib.h"
/* PRO HEADER */
#include "msgdef.h"
#include "path.h"
#include "mmcdef.h"
#include "sockio.h"
#include "config.h"
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
/* LOC HEADER */
#include "cmd_load.h"
#include "cmd_user.h"
#include "cmd_get.h"

/**
	Define constants
*/
#define DEF_COM_LOG_PATH            "COM/"

#define END							0
#define MAX_TOT_RECORD				2000
#define	MAX_ROW_CNT					15
#define DAY_TIME_T					86400
#define WEEK_TIME_T					604800

/**
	Define structures
*/
typedef struct _st_HisData
{
	char		szUserID[16];
	time_t		tTime;
	int			dResult;
	char		szCommand[256];
} st_HisData, *pst_HisData;

/**
	Declare variables
*/
char					prn_buf[2048];
int			         	gdClient;			

extern MYSQL			stMySQL;
extern T_ADMIN_LIST		stAdmin;			/*< mmcd_main.h */

extern st_ConTbl		stConTbl[];			/*< mmcd_main.h */
extern char				SmsName[];			/*< mmcd_main.h */
extern LIB_TBL			lib_tbl[];			/*< mmcd_main.h */
extern fd_set			gstReadfds;			/*< mmcd_main.h */
extern fd_set			gstWritefds;		/*< mmcd_main.h */
extern int				gdNumfds;			/*< mmcd_main.h */
extern int				gdNumWfds;			/*< mmcd_main.h */
extern int				gdSvrSfd;			/*< mmcd_main.h */
extern RUN_TBL			*run_tbl;			/*< mmcd_main.h */
extern st_MngPkt		cli_msg;			/*< mmcd_main.h */

/**
 *	Declare extern func.
 */
extern char *util_cvtipaddr(char* szIP, unsigned int uiIP);
extern int dGetUserInfo(MYSQL *pstMySQL, st_UserAdd *pstData, int *dCnt);
extern int dUser_Login_Update(MYSQL *pstMySQL, char *sUserName, time_t tLoginTime);
extern char *ComposeHead();
extern int dGetMaxFds(void);
extern int kill_user( In_Arg Para[], int sfd, mml_msg *ml );
extern int dIsExceptionCmd(char *cmd);
extern int send_com_data (int osfd, char *sBuf, int endFlag, mml_msg *ml, int dResult, int dConTblIdx );

/**
 *	Implement func.
 */
int name_cmp_sort( const void *a, const void *b )
{
    return strcmp( ((T_ADMIN_DATA *)a)->szUserName, ((T_ADMIN_DATA *)b)->szUserName );
}

int name_cmp( const void *a, const void *b )
{
    return strcmp( (char *)a, ((T_ADMIN_DATA *)b)->szUserName );
}

T_ADMIN_DATA *Get_User(char *str)
{
    return (T_ADMIN_DATA *)bsearch(str, stAdmin.stUserList, stAdmin.usTotalCnt, sizeof(T_ADMIN_DATA), name_cmp );
}

char *szCut_string(char *str)
{
    int     i;
    int     dLen;
    static char temp_str[1024];

    sprintf(temp_str, "%s", str);

    dLen = strlen(temp_str);

    for(i = dLen - 1; i > 0; i--)
    {
        if((temp_str[i] != ' ') && (i == dLen - 1))
            break;

        if(temp_str[i] != ' ')
        {
            temp_str[i + 1] = '\0';
            break;
        }
    }
    return temp_str;
}

int usr_login( In_Arg Par[], int sfd, mml_msg *ml )
{
    int             i, dLen, dRet, dResult, dResultFlag, dIndex, dConTblIdx;
	time_t				tLogin;
	T_ADMIN_DATA		*stData;

	dResult		= 0;
	dResultFlag	= 0;
	dConTblIdx	= 0;

    memset(prn_buf, 0x00, sizeof(prn_buf));
    log_print(LOGN_DEBUG, "[CHECK] LOGIN START");
    dAdminInfoInit();	/*	Refresh	*/

	/*	05/11/04 HWH ADD		*/
	/*	USER LOG-IN CHECK		*/
	/*	Curr Log-In User Check	*/
	for(i = 0; i < gdClient; i++)
	{
		/* ADMIN ID Compare */
		if(strcmp(Par[0].value, stConTbl[i].adminid) == 0)
		{
				/* ALREADY LOGIN */
				dConTblIdx = i;
				log_print(LOGN_DEBUG, "F=%s:%s.%d: [ALREADY LOGIN ID] dConTblIdx[%d] adminid[%s] INPUT_ID[%s]", __FILE__, __FUNCTION__, __LINE__,
					i, stConTbl[i].adminid, Par[0].value);

				sprintf(prn_buf, "\n%s %sM%04d  %s\n  RESULT = FAIL",
					SmsName, (char*)ComposeHead(), lib_tbl[MI_USER_LOGIN].mcode, lib_tbl[MI_USER_LOGIN].msg_header);

				dLen = strlen(prn_buf);
				strcpy(&prn_buf[dLen], "\n  REASON = ALREADY LOGIN USER ID");

				dLen += strlen(&prn_buf[dLen]);
				sprintf(&prn_buf[dLen], "\nCOMPLETED\n");

				dLen += strlen(&prn_buf[dLen]);
				send_text_user(sfd, (char*)prn_buf, DBM_FAILURE, ml, 0, dConTblIdx);
				stConTbl[i].cRetryCnt++;

				return 1;
			}
	}

	for(i = 0; i < gdClient; i++)
	{
		if(stConTbl[i].dSfd == sfd)
		{
			dConTblIdx = i;
			break;
		}
	}

	/*	GET INDEX	*/
	dIndex = GetUserInfo(Par[0].value, 1);

	/*	GET USER NAME	*/
	stData = Get_User(Par[0].value);

	if(stData == NULL)
	{
		/* NOT USER NAME */
		sprintf(prn_buf, "\n%s %sM%04d  %s\n  RESULT = FAIL", SmsName, (char*)ComposeHead(), lib_tbl[MI_USER_LOGIN].mcode, lib_tbl[MI_USER_LOGIN].msg_header);
		dLen = strlen(prn_buf);

		strcpy(&prn_buf[dLen], "\n  REASON = CANNOT FIND USER ID");
		dLen += strlen(&prn_buf[dLen]);

		sprintf(&prn_buf[dLen], "\nCOMPLETED\n");
		dLen += strlen(&prn_buf[dLen]);
		log_print(LOGN_DEBUG, "F=%s:%s.%d: stData[NULL]", __FILE__, __FUNCTION__, __LINE__);
		send_text_user(sfd, (char*)prn_buf, DBM_FAILURE, ml, 0, dConTblIdx);
	}
	else
	{
		log_print(LOGN_DEBUG, "[PASS][%s] [INPUT PASS][%s]", stData->szUserPass, Par[1].value);
		if(strcmp(stData->szUserPass , Par[1].value) == 0)
		{
			/*	IP ADDRESS CHECK	*/
			if(stData->lLastLoginIP != 0)
			{
				if(stData->lLastLoginIP == ntohl(stConTbl[dConTblIdx].uiCliIP))
				{
					dResult = 2;
					sprintf(prn_buf, "SUCCESS");
					log_print(LOGN_DEBUG, "LOGIN RESULT 2, dIndex[%d] SUCCESS", dIndex);

					/* 05/11/04 HWH ADD  */
					/* USER INFO SETTING */
					time(&tLogin);
					stAdmin.stUserList[dIndex].tLastLogin	= tLogin;
					stAdmin.dConnectFlag[dIndex]			= 1;

					/* ADMIN ID SETTING */
					strcpy(stConTbl[dConTblIdx].adminid, Par[0].value);

					if( (dRet = dUser_Login_Update(&stMySQL, stData->szUserName, tLogin)) < 0)
						log_print(LOGN_WARN, "F=%s:%s.%d: FAILED IN dUser_Login_Update() dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
				}
				else
				{
					/*	FAIL: IP ADDRESS ERROR	*/
					sprintf(prn_buf, "\n%s %sM%04d  %s\n  RESULT = FAIL",
						SmsName, (char*)ComposeHead(), lib_tbl[MI_USER_LOGIN].mcode, lib_tbl[MI_USER_LOGIN].msg_header);

					dLen = strlen(prn_buf);
					strcpy(&prn_buf[dLen], "\n  REASON = NOT PERMISSION IP ADDRESS");

					dLen += strlen(&prn_buf[dLen]);
					sprintf(&prn_buf[dLen], "\nCOMPLETED\n");

					dLen += strlen(&prn_buf[dLen]);
					dResultFlag = DBM_FAILURE;
				}
			}
			else
			{
				dResult = 2;
				sprintf(prn_buf, "SUCCESS");
				log_print(LOGN_DEBUG, "LOGIN RESULT 2, dIndex[%d] SUCCESS", dIndex);

				/* 05/11/04 HWH ADD  */
				/* USER INFO SETTING */
				time(&tLogin);
				stAdmin.stUserList[dIndex].tLastLogin	= tLogin;
				stAdmin.dConnectFlag[dIndex]			= 1;

				/* ADMIN ID SETTING */
				strcpy(stConTbl[dConTblIdx].adminid, Par[0].value);

				if( (dRet = dUser_Login_Update(&stMySQL, stData->szUserName, tLogin)) < 0)
					log_print(LOGN_WARN, "F=%s:%s.%d: FAILED IN dUser_Login_Update() dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
			}
		}
		else
		{
			sprintf(prn_buf, "\n%s %sM%04d  %s\n  RESULT = FAIL",
				SmsName, (char*)ComposeHead(), lib_tbl[MI_USER_LOGIN].mcode, lib_tbl[MI_USER_LOGIN].msg_header);

			dLen = strlen(prn_buf);
			strcpy(&prn_buf[dLen], "\n  REASON = INCORRECT PASSWORD");

			dLen += strlen(&prn_buf[dLen]);
			sprintf(&prn_buf[dLen], "\nCOMPLETED\n");

			dLen += strlen(&prn_buf[dLen]);
			dResultFlag = DBM_FAILURE;
		}

		log_print(LOGN_DEBUG, "Send_Text_login USERLEVEL[%d] SFD[%d] dResultFlag[%d] dConTblIdx[%d]", stData->dUserLevel, sfd, dResultFlag, dConTblIdx);
		send_text_login(sfd, (char*)prn_buf, stData->dUserLevel, dResultFlag, ml, dConTblIdx);
	}
	log_print(LOGN_DEBUG," sfd[%d] DATA=[%s]", sfd,  prn_buf);

	return dResult;
}

/*******************************************************************************
 DISPLAY USER INFORMATION
*******************************************************************************/
int dis_admin_info( In_Arg Par[], int sfd, mml_msg *ml )
{
	int 	dLen;
	int		dConnect = 0 ;
	int		dConTblIdx;
	struct in_addr	inaddr;
	T_ADMIN_DATA	*stData;
	T_ADMIN_DATA	*stUser;

	int		i;

    memset( prn_buf, 0x00, sizeof(prn_buf) );

	dConTblIdx = dGetConTblIdx( sfd );

	stUser = Get_User( ml->adminid );

	/*** GET USER INFO FOR USER NAME*******************************************/
	stData = Get_User( Par[0].value );
	if( stData != NULL) /*** GET SUCCESS **************************************/
	{
		for(i=0; i<stAdmin.usTotalCnt;i++)
		{
			if( !strcmp( Par[0].value, stAdmin.stUserList[i].szUserName ) )
			{
				dConnect = stAdmin.dConnectFlag[i];
				break;
			}
		}

		sprintf( prn_buf, "\n%s %sM%04d  %s\n  RESULT = SUCCESS",
        	SmsName, (char*)ComposeHead(), lib_tbl[MI_DIS_USER_INFO].mcode,
        	lib_tbl[MI_DIS_USER_INFO].msg_header );
		dLen = strlen( prn_buf );

		sprintf(&prn_buf[dLen],
			"\n  ------------------------------------------------------------------------------");
    	dLen += strlen( &prn_buf[dLen] );

    	sprintf(&prn_buf[dLen],
        	"\n  USR ID          LEVEL  LOGIN  LAST_LOGIN_TIME CONNECT_IP       IP_ADDRESS");
    	dLen += strlen(&prn_buf[dLen]);

    	sprintf(&prn_buf[dLen],
			"\n  ------------------------------------------------------------------------------");
    	dLen += strlen(&prn_buf[dLen]);

		inaddr.s_addr = stData->lRegIP;

		if( stData->tLastLogin != 0 )
		{
        	sprintf(&prn_buf[dLen],
        		"\n  %-16s  %-3s    %-3s  %s  %-16s  %-16s",
            	stData->szUserName,
#if 0	/* 20040518,poopee */
            	(stData->dUserLevel == 0)?"S":((stData->dUserLevel==1)?"N":"G" ),
#else
            	(stData->dUserLevel == 0)?"S":"N",
#endif
				((dConnect == 0)?"N":"Y"),
				time_short(stData->tLastLogin),
				((dConnect == 0)?"-":util_cvtipaddr(NULL, stData->lLastLoginIP)),
				(stData->lRegIP==0)?"-" :inet_ntoa(inaddr) );

			dLen += strlen(&prn_buf[dLen]);
		}
		else
		{
			sprintf(&prn_buf[dLen],
				"\n  %-16s  %-3s    %-3s  -               %-16s  %-16s",
            	stData->szUserName,
#if 0	/* 20040518,poopee */
            	(stData->dUserLevel == 0)?"S":((stData->dUserLevel==1)?"N":"G" ),
#else
            	(stData->dUserLevel == 0)?"S":"N",
#endif
				((dConnect == 0)?"N":"Y"),
				((dConnect == 0)?"-":util_cvtipaddr(NULL, stData->lLastLoginIP)),
            	(stData->lRegIP==0)?"-" :inet_ntoa(inaddr) );

        	dLen += strlen(&prn_buf[dLen]);
		}

		sprintf(&prn_buf[dLen],
            "\n  ..............................................................................");
        dLen += strlen(&prn_buf[dLen]);

        sprintf(&prn_buf[dLen],
            "\n  TOTAL COUNT = 1");
        dLen += strlen(&prn_buf[dLen]);

    	sprintf(&prn_buf[dLen],
			"\n  ------------------------------------------------------------------------------");
    	dLen += strlen(&prn_buf[dLen]);

		sprintf(&prn_buf[dLen], "\nCOMPLETED\n");
    	dLen += strlen( &prn_buf[dLen] );

		/*** SEND RESULT USING TCP/IP *********************************************/
    	send_text_user( sfd, (char *)prn_buf, END, ml, 1, dConTblIdx );

		//dHistoryMMCD( ml, 1 );
	}
	else	/*** GET FAILURE **************************************************/
	{
		sprintf( prn_buf, "\n%s %sM%04d  %s\n  RESULT =  FAIL",
        	SmsName, (char*)ComposeHead(), lib_tbl[MI_DIS_USER_INFO].mcode,
        	lib_tbl[MI_DIS_USER_INFO].msg_header );
		dLen = strlen( prn_buf );

		sprintf(&prn_buf[dLen],
			"\n  REASON = NOT FOUND USER ID [%s]", Par[0].value );
		dLen += strlen( &prn_buf[dLen] );

		sprintf(&prn_buf[dLen], "\nCOMPLETED\n");
    	dLen += strlen( &prn_buf[dLen] );

		/*** SEND RESULT USING TCP/IP *********************************************/
    	send_text_user( sfd, (char *)prn_buf, DBM_FAILURE, ml, 0, dConTblIdx );

		//dHistoryMMCD( ml, 0 );
	}

    log_print(LOGN_DEBUG, prn_buf );

	return 1;
}


#if 0
/*******************************************************************************
 SET ADMIN INFORMATION
*******************************************************************************/
int set_admin_info( In_Arg Para[], int sfd, mml_msg *ml )
{
	int					i;
	int                 dRet, dLen;
	int					dIndex;
	int					dExecuteUserIdx;
	int					dRunTblIdx;
	int					dConTblIdx;
	struct in_addr  	inaddr;
	T_ADMIN_DATA        stAdminOld, cmd_para;

#if 0	/* 20040518,poopee */
	log_print(LOGN_DEBUG, "[%s] [%s] [%d] [%d]",
			Para[0].value, Para[1].value, atoi(Para[2].value), atoi(Para[3].value) );
#else
	cmd_para.szUserName[0] = 0;
	cmd_para.szUserPass[0] = 0;
	cmd_para.dUserLevel = -1;
	cmd_para.dIPFlag = -1;
	for (i=0; i<4; i++)
	{
		log_print(LOGN_DEBUG,"[%s:%s]",Para[i].name,Para[i].value);
		if (!strcmp(Para[i].name,"USR"))
			strcpy(cmd_para.szUserName,Para[i].value);
		else if (!strcmp(Para[i].name,"PASSWD"))
			strcpy(cmd_para.szUserPass,Para[i].value);
		else if (!strcmp(Para[i].name,"USERLEVEL"))
			cmd_para.dUserLevel = atoi(Para[i].value);
		else if (!strcmp(Para[i].name,"IP"))
		{
			cmd_para.dIPFlag = 1;
			cmd_para.lRegIP = inet_addr(Para[i].value);
		}
	}
#endif

	memset( prn_buf, 0x00, sizeof(prn_buf) );
	memset( &stAdminOld, 0x00, sizeof(T_ADMIN_DATA) );

	dConTblIdx = dGetConTblIdx( sfd );

	dIndex = GetUserInfo( cmd_para.szUserName, 1 ); /*** GET INDEX FOR USER NAME ****/
	dExecuteUserIdx = GetUserInfo( ml->adminid, 1 );

	if( dIndex < 0 )	/*** INVALID INDEX ************************************/
	{
		sprintf( prn_buf, "\n%s %sM%04d  %s\n  RESULT = FAIL",
        SmsName, (char*)ComposeHead(), lib_tbl[MI_CHG_USER_INFO].mcode,
        lib_tbl[MI_CHG_USER_INFO].msg_header );

        strcat( prn_buf, tmp_buf );
        sprintf( tmp_buf, "\n  REASON = CANNOT FIND USER ID[%s]", cmd_para.szUserName );
        strcat( prn_buf, "\nCOMPLETED\n");

        log_print(LOGN_DEBUG, prn_buf );

        send_text_user( sfd, (char *)prn_buf, DBM_FAILURE, ml, 0, dConTblIdx );

		//dHistoryMMCD( ml, 0 );

        return dIndex;
	}
	else if( stAdmin.dConnectFlag[dIndex] == 1 ) /*** LOGIN STATUS ************/
    {
		// ANOTHER USER
		if( strcasecmp( stAdmin.stUserList[dIndex].szUserName, ml->adminid ) != 0 )
		{
			sprintf( prn_buf, "\n%s %sM%04d  %s\n  RESULT = FAIL",
            	SmsName, (char*)ComposeHead(), lib_tbl[MI_CHG_USER_INFO].mcode,
            	lib_tbl[MI_CHG_USER_INFO].msg_header );

        	sprintf( tmp_buf, "\n  REASON = CURRENT LOGIN STATUS" );
        	strcat( prn_buf, tmp_buf );
        	strcat( prn_buf, "\nCOMPLETED\n");

        	log_print(LOGN_DEBUG, prn_buf );

        	send_text_user( sfd, (char *)prn_buf, DBM_FAILURE, ml, 0, dConTblIdx );

        	//dHistoryMMCD( ml, 0 );

        	return dIndex;
		}
    }

	if( stAdmin.stUserList[dExecuteUserIdx].dUserLevel > 0 )
	{
		if( strcasecmp(stAdmin.stUserList[dIndex].szUserName, ml->adminid ) != 0 )
		{
			sprintf( prn_buf, "\n%s %sM%04d  %s\n  RESULT = FAIL",
        		SmsName, (char*)ComposeHead(), lib_tbl[MI_CHG_USER_INFO].mcode,
        		lib_tbl[MI_CHG_USER_INFO].msg_header );

        	sprintf( tmp_buf, "\n  REASON = THIS USER CANNOT CHANGE ANOTHER USER INFORMATION" );
        	strcat( prn_buf, tmp_buf );
        	strcat( prn_buf, "\nCOMPLETED\n");

        	log_print(LOGN_DEBUG, prn_buf );

        	send_text_user( sfd, (char *)prn_buf, DBM_FAILURE, ml, 0, dConTblIdx );

        	//dHistoryMMCD( ml, 0 );

        	return dIndex;
		}
#if 0	/* 20040518,pooee */
		else if( strlen(Para[2].value) != 0 || atoi(Para[3].value) != 0 )
#else
		else if( cmd_para.dUserLevel != -1 || cmd_para.dIPFlag != -1)
#endif
		{
			sprintf( prn_buf, "\n%s %sM%04d  %s\n  RESULT = FAIL",
                SmsName, (char*)ComposeHead(), lib_tbl[MI_CHG_USER_INFO].mcode,
                lib_tbl[MI_CHG_USER_INFO].msg_header );

            sprintf( tmp_buf, "\n  REASON = THIS USER CANNOT CHANGE IP ADDRESS AND USER LEVEL" );
            strcat( prn_buf, tmp_buf );
            strcat( prn_buf, "\nCOMPLETED\n");

            log_print(LOGN_DEBUG, prn_buf );

            send_text_user( sfd, (char *)prn_buf, DBM_FAILURE, ml, 0, dConTblIdx );

            //dHistoryMMCD( ml, 0 );

            return dIndex;
		}
	}


	/*** SAVE ORIGINAL VALUE IN stAdminOld ************************************/
	memcpy( &stAdminOld, &stAdmin.stUserList[dIndex], sizeof(T_ADMIN_DATA) );

#if 0	/* 20040520,poopee */
	memcpy( &stAdmin,&stAdminOld,sizeof(T_ADMIN_DATA));
#else
	memcpy( &stAdmin.stUserList[dIndex],&stAdminOld,sizeof(T_ADMIN_DATA));
#endif
	strcpy( stAdmin.stUserList[dIndex].szUserName, cmd_para.szUserName );

	if( strlen(cmd_para.szUserPass) != 0 )
		strcpy( stAdmin.stUserList[dIndex].szUserPass, cmd_para.szUserPass );

	if( cmd_para.dUserLevel != -1 )
		stAdmin.stUserList[dIndex].dUserLevel = cmd_para.dUserLevel - 1;

	if( cmd_para.dIPFlag != -1)
	{
		stAdmin.stUserList[dIndex].dIPFlag = 1;
		stAdmin.stUserList[dIndex].lRegIP = cmd_para.lRegIP;
	}

	dRunTblIdx = dGetRunTblIdx( sfd );

	dRet = dWriteAdminFile( 0 ) ;

	if( dRet < 0 )
	{
		memcpy( &stAdmin.stUserList[dIndex], &stAdminOld, sizeof(T_ADMIN_DATA) );

		sprintf( prn_buf, "\n%s %sM%04d  %s\n  RESULT = FAIL",
        	SmsName, (char*)ComposeHead(), lib_tbl[MI_CHG_USER_INFO].mcode,
        	lib_tbl[MI_CHG_USER_INFO].msg_header );

        sprintf( tmp_buf, "\n  REASON = SET INFORMATION OF ADMINISTRATOR [%d]", dRet );
        strcat( prn_buf, tmp_buf );
        strcat( prn_buf, "\nCOMPLETED\n");

        log_print(LOGN_DEBUG, prn_buf );

        send_text_user( sfd, (char *)prn_buf, DBM_FAILURE, ml, 0, dConTblIdx );

		//dHistoryMMCD( ml, 0 );
	}
	else
	{
		sprintf( prn_buf, "\n%s %sM%04d  %s\n  RESULT = SUCCESS",
        	SmsName, (char*)ComposeHead(), lib_tbl[MI_CHG_USER_INFO].mcode,
        	lib_tbl[MI_CHG_USER_INFO].msg_header );

#if 0	/* 20040420,poopee */
		sprintf( tmp_buf, "\n  CHANGE COMPELTE");
        strcat( prn_buf, tmp_buf );

		sprintf( tmp_buf,
            "\n  --------------------------------------------------------------------");
		strcat( prn_buf, tmp_buf );

        sprintf( tmp_buf,
            "\n  USR ID          LEVEL  LOGIN_STATUS  LAST_LOGIN_TIME IP_ADDRESS");
		strcat( prn_buf, tmp_buf );

        sprintf( tmp_buf,
            "\n  --------------------------------------------------------------------");
		strcat( prn_buf, tmp_buf );

		inaddr.s_addr = stAdmin.stUserList[dIndex].lRegIP;

		if( stAdmin.stUserList[dIndex].tLastLogin != 0 )
        {
        sprintf( tmp_buf,
            "\n  %-16s  %-3s  %6s        %s  %-16s",
            stAdmin.stUserList[dIndex].szUserName,
            (stAdmin.stUserList[dIndex].dUserLevel == 0)?"S":((stAdmin.stUserList[dIndex].dUserLevel==1)?"N":"G" ),
            ((stAdmin.dConnectFlag[dIndex] == 0)?"N":"Y"),
            time_short(stAdmin.stUserList[dIndex].tLastLogin),
            (stAdmin.stUserList[dIndex].lRegIP==0)?"-" :inet_ntoa(inaddr) );
			strcat( prn_buf, tmp_buf );
        }
        else
        {
        sprintf( tmp_buf,
            "\n  %-16s  %-3s  %6s        -               %s",
            stAdmin.stUserList[dIndex].szUserName,
            (stAdmin.stUserList[dIndex].dUserLevel == 0)?"S":((stAdmin.stUserList[dIndex].dUserLevel==1)?"N":"G" ),
            ((stAdmin.dConnectFlag[dIndex] == 0)?"N":"Y"),
            (stAdmin.stUserList[dIndex].lRegIP==0)?"-" :inet_ntoa(inaddr) );
			strcat( prn_buf, tmp_buf );
        }


		sprintf( tmp_buf,
            "\n  --------------------------------------------------------------------");
		strcat( prn_buf, tmp_buf );
#else
		sprintf(tmp_buf,       "\n  ----------------------------------------------------");
		dLen = strlen(tmp_buf);
		sprintf(&tmp_buf[dLen],"\n        USR ID          PASSWD  LEVEL  IP_ADDRESS");
		dLen = strlen(tmp_buf);
		sprintf(&tmp_buf[dLen],"\n  ----------------------------------------------------");
		dLen = strlen(tmp_buf);

		inaddr.s_addr = stAdminOld.lRegIP;
		sprintf(&tmp_buf[dLen],"\n  PRE   %-16s******    %s    %-16s",
			stAdminOld.szUserName,
#if 0	/* 20040518,poope */
			(stAdminOld.dUserLevel == 0)? "S":((stAdminOld.dUserLevel==1)?"N":"G"),
#else
			(stAdminOld.dUserLevel == 0)? "S":"N",
#endif
			(stAdminOld.lRegIP==0)? "-":inet_ntoa(inaddr));
		dLen = strlen(tmp_buf);

		inaddr.s_addr = stAdmin.stUserList[dIndex].lRegIP;
		sprintf(&tmp_buf[dLen],"\n  CUR   %-16s******    %s    %-16s",
			stAdmin.stUserList[dIndex].szUserName,
#if 0	/* 20040518,poopee */
			(stAdmin.stUserList[dIndex].dUserLevel == 0)? "S":((stAdmin.stUserList[dIndex].dUserLevel==1)?"N":"G"),
#else
			(stAdmin.stUserList[dIndex].dUserLevel == 0)? "S":"N",
#endif
			(stAdmin.stUserList[dIndex].lRegIP==0)? "-":inet_ntoa(inaddr));
		dLen = strlen(tmp_buf);

		sprintf(&tmp_buf[dLen],"\n  ----------------------------------------------------");
		dLen = strlen(tmp_buf);
#endif
		strcat( prn_buf, tmp_buf );
        strcat( prn_buf, "\nCOMPLETED\n");

		send_text_user( sfd, (char *)prn_buf, END, ml, 1, dConTblIdx );

		//dHistoryMMCD( ml, 1 );

		//qsort( (void *)stAdmin.stUserList, stAdmin.usTotalCnt, sizeof(T_ADMIN_DATA), name_cmp_sort );
	}

	return dRet;
}
#endif


#if 0
/*******************************************************************************
 ADD ADMIN INFORMATION
*******************************************************************************/
int add_admin_info( In_Arg Para[], int sfd, mml_msg *ml )
{
	int			i;
	int			dRet, dLen;
	int			dIndex;
	int			dNewIdx;
	int			dNewIndex;
	int			dRunTblIdx;
	int			dConTblIdx;
	int			dOldContFlag[MAX_USER_CNT];
	T_ADMIN_DATA		*stUser, cmd_para;
	struct in_addr	inaddr;

	cmd_para.szUserName[0] = 0;
	cmd_para.szUserPass[0] = 0;
	cmd_para.dUserLevel = -1;
	cmd_para.dIPFlag = -1;
	for (i=0; i<4; i++)
	{
		log_print(LOGN_DEBUG,"[%s:%s]",Para[i].name,Para[i].value);
		if (!strcmp(Para[i].name,"USR"))
			strcpy(cmd_para.szUserName,Para[i].value);
		else if (!strcmp(Para[i].name,"PASSWD"))
			strcpy(cmd_para.szUserPass,Para[i].value);
		else if (!strcmp(Para[i].name,"USERLEVEL"))
			cmd_para.dUserLevel = atoi(Para[i].value);
		else if (!strcmp(Para[i].name,"IP"))
		{
			cmd_para.dIPFlag = 1;
			cmd_para.lRegIP = inet_addr(Para[i].value);
		}
	}

	memset( prn_buf, 0x00, sizeof(prn_buf) );

	dConTblIdx = dGetConTblIdx( sfd );

	stUser = Get_User( ml->adminid );
	if( stUser->dUserLevel > cmd_para.dUserLevel-1 )		/* #### ? */
	{
		sprintf( prn_buf, "\n%s %sM%04d  %s\n  RESULT = FAIL",
        SmsName, (char*)ComposeHead(), lib_tbl[MI_ADD_USER].mcode,
        lib_tbl[MI_ADD_USER].msg_header );

        sprintf( tmp_buf, "\n  REASON = CANNOT CREATE THIS USER LEVEL" );
        strcat( prn_buf, tmp_buf );
        strcat( prn_buf, "\nCOMPLETED\n");

        log_print(LOGN_DEBUG, prn_buf );

        send_text_user( sfd, (char *)prn_buf, DBM_FAILURE, ml, 0, dConTblIdx );

		//dHistoryMMCD( ml, 0 );

        return 1;
	}

	dIndex = GetUserInfo( cmd_para.szUserName, 1 );
	if( dIndex >= 0 )	/*** USER ID ALREADY EXIST ****************************/
	{
		sprintf( prn_buf, "\n%s %sM%04d  %s\n  RESULT = FAIL",
        	SmsName, (char*)ComposeHead(), lib_tbl[MI_ADD_USER].mcode,
        	lib_tbl[MI_ADD_USER].msg_header );

        sprintf( tmp_buf, "\n  REASON = ALREADY EXIST ADMINISTRATOR" );
        strcat( prn_buf, tmp_buf );
        strcat( prn_buf, "\nCOMPLETED\n");

        log_print(LOGN_DEBUG, prn_buf );

        send_text_user( sfd, (char *)prn_buf, DBM_FAILURE, ml, 0, dConTblIdx );

		//dHistoryMMCD( ml, 0 );

		return dIndex;
	}

	dNewIndex = stAdmin.usTotalCnt;

	if( dNewIndex == MAX_USER_CNT )
	{
		sprintf( prn_buf, "\n%s %sM%04d  %s\n  RESULT = FAIL",
        	SmsName, (char*)ComposeHead(), lib_tbl[MI_ADD_USER].mcode,
        	lib_tbl[MI_ADD_USER].msg_header );

        sprintf( tmp_buf, "\n  REASON = MAX USER COUNT" );
        strcat( prn_buf, tmp_buf );
        strcat( prn_buf, "\nCOMPLETED\n");

        log_print(LOGN_DEBUG, prn_buf );

        send_text_user( sfd, (char *)prn_buf, DBM_FAILURE, ml, 0, dConTblIdx );

		//dHistoryMMCD( ml, 0 );

        return dIndex;
	}

	stAdmin.usTotalCnt += 1;
	strcpy( stAdmin.stUserList[dNewIndex].szUserName, cmd_para.szUserName );
	strcpy( stAdmin.stUserList[dNewIndex].szUserPass, cmd_para.szUserPass );
	stAdmin.stUserList[dNewIndex].dUserLevel  = cmd_para.dUserLevel - 1;
	stAdmin.stUserList[dNewIndex].tLastLogin  = 0;
	stAdmin.stUserList[dNewIndex].tLastLogout = 0;

	stAdmin.dConnectFlag[dNewIndex] = 0;

	if( cmd_para.dIPFlag == -1 )
	{
		stAdmin.stUserList[dNewIndex].dIPFlag = 0;
		stAdmin.stUserList[dNewIndex].lRegIP = 0;
	}
	else
	{
		stAdmin.stUserList[dNewIndex].dIPFlag = 1;
		stAdmin.stUserList[dNewIndex].lRegIP = cmd_para.lRegIP;;
	}

	dRunTblIdx = dGetRunTblIdx( sfd );

	dRet = dWriteAdminFile( 0 );

	if( dRet < 0 )
	{
		InitUserInfo( dNewIndex, 1 );

		sprintf( prn_buf, "\n%s %sM%04d  %s\n  RESULT = FAIL",
        	SmsName, (char*)ComposeHead(), lib_tbl[MI_ADD_USER].mcode,
        	lib_tbl[MI_ADD_USER].msg_header );

        sprintf( tmp_buf, "\n  REASON = ADD ADMINISTRATOR FAIL[%d]", dRet );
        strcat( prn_buf, tmp_buf );
        strcat( prn_buf, "\nCOMPLETED\n");

        log_print(LOGN_DEBUG, prn_buf );

        send_text_user( sfd, (char *)prn_buf, DBM_FAILURE, ml, 0, dConTblIdx );

		return 1;
	}
	else
	{
		sprintf( prn_buf, "\n%s %sM%04d  %s\n  RESULT = SUCCESS",
        	SmsName, (char*)ComposeHead(), lib_tbl[MI_ADD_USER].mcode,
        	lib_tbl[MI_ADD_USER].msg_header );

#if 0	/* 20040420,poopee */
        sprintf( tmp_buf, "\n  NEW USER REGISTRATION COMPLETE" );
#else
		inaddr.s_addr = stAdmin.stUserList[dNewIndex].lRegIP;

		sprintf(tmp_buf,       "\n  --------------------------------------");
		dLen = strlen(tmp_buf);
		sprintf(&tmp_buf[dLen],"\n  USR ID          LEVEL  IP_ADDRESS");
		dLen = strlen(tmp_buf);
		sprintf(&tmp_buf[dLen],"\n  --------------------------------------");
		dLen = strlen(tmp_buf);
		sprintf(&tmp_buf[dLen],"\n  %-16s  %s    %-16s",
			stAdmin.stUserList[dNewIndex].szUserName,
#if 0	/* 20040518,poopee */
			(stAdmin.stUserList[dNewIndex].dUserLevel == 0)? "S":((stAdmin.stUserList[dNewIndex].dUserLevel==1)?"N":"G"),
#else
			(stAdmin.stUserList[dNewIndex].dUserLevel == 0)? "S":"N",
#endif
			(stAdmin.stUserList[dNewIndex].lRegIP==0)? "-":inet_ntoa(inaddr));
		dLen = strlen(tmp_buf);
		sprintf(&tmp_buf[dLen],"\n  --------------------------------------");
		dLen = strlen(tmp_buf);
#endif
        strcat( prn_buf, tmp_buf );
        strcat( prn_buf, "\nCOMPLETED\n");

        log_print(LOGN_DEBUG, prn_buf );

        send_text_user( sfd, (char *)prn_buf, END, ml, 1, dConTblIdx );


		qsort( (void *)stAdmin.stUserList, stAdmin.usTotalCnt, sizeof(T_ADMIN_DATA), name_cmp_sort );
	}

	for(i=0; i<MAX_USER_CNT; i++)
		dOldContFlag[i] = stAdmin.dConnectFlag[i];

	dNewIdx = 0;
	for( i=0 ; i<stAdmin.usTotalCnt; i++ )
	{
		if( strcasecmp( stAdmin.stUserList[i].szUserName, cmd_para.szUserName ) == 0 ) {
			stAdmin.dConnectFlag[i] = 0;

			dNewIdx++;
			continue;
		}

		stAdmin.dConnectFlag[i] = dOldContFlag[i-dNewIdx];
	}

	return dRet;
}


/*******************************************************************************
 DELETE ADMIN INFORMATION
*******************************************************************************/
int del_admin_info( In_Arg Para[], int sfd, mml_msg *ml )
{
	int				i;
	int				dRet, dLen;
	int				dIndex;
	int				dNewIdx;
	int				dRunTblIdx;
	int				dConTblIdx;
	T_ADMIN_DATA	stAdminOld;
	struct in_addr	inaddr;

	memset( prn_buf, 0x00, sizeof(prn_buf) );

	dConTblIdx = dGetConTblIdx( sfd );

	dIndex = GetUserInfo( Para[0].value, 1 );
	if( dIndex < 0 )
	{
		sprintf( prn_buf, "\n%s %sM%04d  %s\n  RESULT = FAIL",
        	SmsName, (char*)ComposeHead(), lib_tbl[MI_DEL_USER].mcode,
        	lib_tbl[MI_DEL_USER].msg_header );

        sprintf( tmp_buf, "\n  REASON = NOT EXIST USER ID" );
        strcat( prn_buf, tmp_buf );
        strcat( prn_buf, "\nCOMPLETED\n");

        log_print(LOGN_DEBUG, prn_buf );

        send_text_user( sfd, (char *)prn_buf, DBM_FAILURE, ml, 0, dConTblIdx );

        return dIndex;
	}
	else if( !strcmp(Para[0].value, "ntasnms") )
	{
		sprintf( prn_buf, "\n%s %sM%04d  %s\n  RESULT = FAIL",
 			SmsName, (char*)ComposeHead(), lib_tbl[MI_DEL_USER].mcode,
 			lib_tbl[MI_DEL_USER].msg_header );

    	sprintf( tmp_buf, "\n  REASON = CANNOT DELETE THIS USER" );
 		strcat( prn_buf, tmp_buf );
 		strcat( prn_buf, "\nCOMPLETED\n");

        log_print(LOGN_DEBUG, prn_buf );

        send_text_user( sfd, (char *)prn_buf, DBM_FAILURE, ml, 0, dConTblIdx );

		//dHistoryMMCD( ml, 0 );

        return dIndex;

	}
	else if( stAdmin.dConnectFlag[dIndex] == 1 )
	{
		sprintf( prn_buf, "\n%s %sM%04d  %s\n  RESULT = FAIL",
        	SmsName, (char*)ComposeHead(), lib_tbl[MI_DEL_USER].mcode,
        	lib_tbl[MI_DEL_USER].msg_header );

        sprintf( tmp_buf, "\n  REASON = CURRENT LOGIN STATUS" );
        strcat( prn_buf, tmp_buf );
        strcat( prn_buf, "\nCOMPLETED\n");

        log_print(LOGN_DEBUG, prn_buf );

        send_text_user( sfd, (char *)prn_buf, DBM_FAILURE, ml, 0, dConTblIdx );

		//dHistoryMMCD( ml, 0 );

        return dIndex;
	}

	memcpy( &stAdminOld, &stAdmin.stUserList[dIndex], sizeof(T_ADMIN_DATA) );

	InitUserInfo( dIndex, 1 );

	dRunTblIdx = dGetRunTblIdx( sfd );

	dRet = dWriteAdminFile( 1 );
	if( dRet < 0 )
	{
		memcpy( &stAdmin.stUserList[dIndex], &stAdminOld, sizeof(T_ADMIN_DATA) );

		sprintf( prn_buf, "\n%s %sM%04d  %s\n  RESULT = FAIL",
        	SmsName, (char*)ComposeHead(), lib_tbl[MI_DEL_USER].mcode,
        	lib_tbl[MI_DEL_USER].msg_header );

        sprintf( tmp_buf, "\n  REASON = ERROR DELETE ADMINISTRATOR[%d]", dRet );
        strcat( prn_buf, tmp_buf );
        strcat( prn_buf, "\nCOMPLETED\n");

        log_print(LOGN_DEBUG, prn_buf );

        send_text_user( sfd, (char *)prn_buf, DBM_FAILURE, ml, 0, dConTblIdx );

		return 1;
	}
	else
	{
		sprintf( prn_buf, "\n%s %sM%04d  %s\n  RESULT = SUCCESS",
        	SmsName, (char*)ComposeHead(), lib_tbl[MI_DEL_USER].mcode,
        	lib_tbl[MI_DEL_USER].msg_header );

#if 0	/* 20040420,poopee */
        sprintf( tmp_buf, "\n  DELETE USER COMPLETE" );
#else
		inaddr.s_addr = stAdminOld.lRegIP;

		sprintf(tmp_buf,       "\n  --------------------------------------");
		dLen = strlen(tmp_buf);
		sprintf(&tmp_buf[dLen],"\n  USR ID          LEVEL  IP_ADDRESS");
		dLen = strlen(tmp_buf);
		sprintf(&tmp_buf[dLen],"\n  --------------------------------------");
		dLen = strlen(tmp_buf);
		sprintf(&tmp_buf[dLen],"\n  %-16s  %s    %-16s",
			stAdminOld.szUserName,
			(stAdminOld.dUserLevel == 0)? "S":((stAdminOld.dUserLevel==1)?"N":"G"),
			(stAdminOld.lRegIP==0)? "-":inet_ntoa(inaddr));
		dLen = strlen(tmp_buf);
		sprintf(&tmp_buf[dLen],"\n  --------------------------------------");
		dLen = strlen(tmp_buf);
#endif
        strcat( prn_buf, tmp_buf );
        strcat( prn_buf, "\nCOMPLETED\n");

        log_print(LOGN_DEBUG, prn_buf );

        send_text_user( sfd, (char *)prn_buf, END, ml, 1, dConTblIdx );

	}

	dAdminInfoInit();

	dNewIdx = 0;
	for( i = 0; i<stAdmin.usTotalCnt; i++)
	{
		if( i == dIndex )
			dNewIdx++;

		stAdmin.dConnectFlag[i] = stAdmin.dConnectFlag[i+dNewIdx];
	}

	InitUserInfo( stAdmin.usTotalCnt, 0 );

	qsort( (void *)stAdmin.stUserList, stAdmin.usTotalCnt, sizeof(T_ADMIN_DATA), name_cmp_sort );

	return 1;
}

#endif


/*******************************************************************************
 USER LOGOUT
*******************************************************************************/
int usr_logout( In_Arg Para[], int sfd, mml_msg *ml )
{
	    int     i, j;
    int     dLen;
    int     dIndex;
    int     dConTblIdx;
    time_t  tLogout;
    fd_set  stWd;

    dConTblIdx = dGetConTblIdx( sfd );

    if( strcmp( Para[0].value, ml->adminid ) ) /*** NOT MATCH *****************/
    {
        sprintf( prn_buf, "\n%s %sM%04d  %s\n  RESULT = FAIL",
            SmsName, (char*)ComposeHead(), lib_tbl[MI_USER_LOGOUT].mcode,
            lib_tbl[MI_USER_LOGOUT].msg_header );
        dLen = strlen(prn_buf);

        sprintf( &prn_buf[dLen],
            "\n  REASON = CANNOT LOGOUT ANOTHER USER");
        dLen += strlen(&prn_buf[dLen]);

        sprintf( &prn_buf[dLen],
            "\nCOMPLETED\n");
        dLen += strlen(&prn_buf[dLen]);

        log_print(LOGN_DEBUG, prn_buf );

        send_text_user( sfd, (char *)prn_buf, DBM_FAILURE, ml, 0, dConTblIdx );

        /*dHistoryMMCD( ml, 0 );*/

        return 0;
    }

    if( sfd != stConTbl[dConTblIdx].dSfd ) {
        sprintf( prn_buf, "\n%s %sM%04d  %s\n  RESULT = FAIL",
            SmsName, (char*)ComposeHead(), lib_tbl[MI_USER_LOGOUT].mcode,
            lib_tbl[MI_USER_LOGOUT].msg_header );
        dLen = strlen(prn_buf);
        sprintf( &prn_buf[dLen],
            "\n  REASON = CANNOT LOGOUT ANOTHER USER");
        dLen += strlen(&prn_buf[dLen]);

        sprintf( &prn_buf[dLen],
            "\nCOMPLETED\n");
        dLen += strlen(&prn_buf[dLen]);

        log_print(LOGN_DEBUG, prn_buf );

        send_text_user( sfd, (char *)prn_buf, DBM_FAILURE, ml, 0, dConTblIdx );

        /*dHistoryMMCD( ml, 0 );*/

        return 0;
    }


    log_print(LOGN_DEBUG,"user_logout [%s]", Para[0].value);

    /*** SET LAST LOGOUT TIME *************************************************/
    dIndex = GetUserInfo( Para[0].value, 1 );

    tLogout = time(&tLogout);
    stAdmin.stUserList[dIndex].tLastLogout = tLogout;
    stAdmin.dConnectFlag[dIndex] = 0;


    for( i=0; i<gdClient; i++ )
    {
        if( !strcmp( Para[0].value, stConTbl[i].adminid ) )
        {
            sprintf( prn_buf, "\n%s %sM%04d  %s\n  RESULT = SUCCESS",
                SmsName, (char*)ComposeHead(), lib_tbl[MI_USER_LOGOUT].mcode,
                lib_tbl[MI_USER_LOGOUT].msg_header );
                dLen = strlen(prn_buf);

            sprintf( &prn_buf[dLen],
                "\n  LOGOUT SUCCESS");
            dLen += strlen(&prn_buf[dLen]);

            sprintf(&prn_buf[dLen], "\nCOMPLETED\n");
            dLen += strlen( &prn_buf[dLen] );

            log_print(LOGN_DEBUG, prn_buf );

            send_text_user( sfd, (char *)prn_buf, END, ml, 1, dConTblIdx );

            /*dHistoryMMCD( ml, 1 );*/

            close(sfd);
            FD_CLR( sfd, &gstReadfds);

            log_print(LOGN_DEBUG, "GDCLIENT INFO : [%d]", gdClient );

            if( gdClient == 1 ) {
                gdClient--;
                gdNumfds = gdSvrSfd;
            }
            else if( (gdClient-1) == i )
            {
                gdClient--;

                while(1)
                {
                    if( stConTbl[gdClient-1].dSfd > 0 )
                    {
                        /*gdNumfds = stConTbl[gdClient-1].dSfd;*/
                        gdNumfds = dGetMaxFds();
                        break;
                    }
                    else
                    {
                        gdClient--;

                        if( gdClient == 0 ) {
                            gdNumfds = gdSvrSfd;
                            break;
                        }
                    }
                }
            }

            /*** ADD 2003.05.27. ***/
            memcpy((char*)&stWd, (char*)&gstWritefds, sizeof(fd_set));
            if( FD_ISSET( sfd, &stWd ) )
            {
                log_print(LOGN_DEBUG,"CLEAR WRITEFDS FOR SOCKET CLOSE SFD[%d]", sfd );

                FD_CLR( sfd, &gstWritefds );

                gdNumWfds--;
                stConTbl[i].cSockBlock = 0x00;
            }

            log_print(LOGN_DEBUG, "GDCLIENT INFO : [%d]", gdClient );

            stConTbl[i].dSfd = -1;
            stConTbl[i].dBufLen = 0;
            stConTbl[i].uiCliIP = 0;
            stConTbl[i].cRetryCnt = 0;
            stConTbl[i].adminid[0] = 0x00;

            for( j=0; j<MAX_TMR_REC; j++)
            {
                if( !strcasecmp( Para[0].value, run_tbl->user_name[j] ) )
                {
                    clear_my_tmr(j);
                }
            }

            break;
        }
    }

    return 1;
}

int kill_user( In_Arg Para[], int sfd, mml_msg *ml )
{

	int     i, j;
    int     dLen;
    int     dIndex;
    int     dSfd;
    time_t  tLogout;
    fd_set  stWd;

    log_print(LOGN_DEBUG,"user_logout [%s]", Para[0].value);

    /*** SET LAST LOGOUT TIME *************************************************/
    dIndex = GetUserInfo( Para[0].value, 1 );

    tLogout = time(&tLogout);
    stAdmin.stUserList[dIndex].tLastLogout = tLogout;
    stAdmin.dConnectFlag[dIndex] = 0;


    for( i=0; i<gdClient; i++ )
    {
        if( !strcmp( Para[0].value, stConTbl[i].adminid ) )
        {
            sprintf( prn_buf, "\n%s %sM%04d  %s\n  RESULT = SUCCESS",
                SmsName, (char*)ComposeHead(), lib_tbl[MI_KILL_USER].mcode,
                lib_tbl[MI_KILL_USER].msg_header );
                dLen = strlen(prn_buf);

            sprintf( &prn_buf[dLen], "\n  LOGOUT SUCCESS");
            dLen += strlen(&prn_buf[dLen]);

            sprintf(&prn_buf[dLen], "\nCOMPLETED\n");
            dLen += strlen( &prn_buf[dLen] );

            log_print(LOGN_DEBUG, prn_buf );

            /*send_text_user( sfd, (char *)prn_buf, END, ml, 1 ); */

            /*dHistoryMMCD( ml, 1 );*/
            dSfd = stConTbl[i].dSfd;

            close(dSfd);
            FD_CLR( dSfd, &gstReadfds);

            log_print(LOGN_DEBUG, "GDCLIENT INFO : [%d]", gdClient );

            if( gdClient == 1 ) {
                gdClient--;
                gdNumfds = gdSvrSfd;
            }
            else if( (gdClient-1) == i )
            {
                gdClient--;

                while(1)
                {
                    if( stConTbl[gdClient-1].dSfd > 0 )
                    {
                        /*gdNumfds = stConTbl[gdClient-1].dSfd;*/
                        gdNumfds = dGetMaxFds();
                        break;
                    }
                    else
                    {
                        gdClient--;

                        if( gdClient == 0 ) {
                            gdNumfds = gdSvrSfd;
                            break;
                        }
                    }
                }
            }

            /*** ADD 2003.05.27. ***/
            memcpy((char*)&stWd, (char*)&gstWritefds, sizeof(fd_set));
            if( FD_ISSET( dSfd, &stWd ) )
            {
                log_print(LOGN_DEBUG,"CLEAR WRITEFDS FOR SOCKET CLOSE SFD[%d]", dSfd );
                FD_CLR( dSfd, &gstWritefds );

                gdNumWfds--;
                stConTbl[i].cSockBlock = 0x00;
            }

            log_print(LOGN_DEBUG, "GDCLIENT INFO : [%d]", gdClient );

            stConTbl[i].dSfd = -1;
            stConTbl[i].dBufLen = 0;
            stConTbl[i].uiCliIP = 0;
            stConTbl[i].cRetryCnt = 0;
            memset( stConTbl[i].adminid, 0x00, 21 );

            for( j=0; j<MAX_TMR_REC; j++)
            {
                if( !strcasecmp( Para[0].value, run_tbl->user_name[j] ) )
                {
                    clear_my_tmr(j);
                }
            }

            break;
        }
    }

    return 1;
}


#if 0
/*******************************************************************************

*******************************************************************************/
int dis_all_user_info( In_Arg Par[], int sfd, mml_msg *ml )
{
    int             i;
    int             dLen;
	int				dConTblIdx;
	struct in_addr 	inaddr;
    T_ADMIN_DATA    *stData;

    memset(prn_buf, 0x00, sizeof(prn_buf) );

	dConTblIdx = dGetConTblIdx( sfd );

	if( cli_msg.head.ucBinFlag == 1 )
	{
		memcpy( &prn_buf[0], &stAdmin, sizeof(T_ADMIN_LIST) );

		bin_print( &prn_buf[0], sizeof(T_ADMIN_LIST), sfd, DBM_SUCCESS, END, dConTblIdx );

		return 1;
	}

	//log_print(LOGN_DEBUG,"ADMIN SIZE[%d] [%d]", sizeof(T_ADMIN_LIST), stAdmin.usTotalCnt );

	sprintf( prn_buf, "\n%s %sM%04d  %s\n  RESULT = SUCCESS",
        SmsName, (char*)ComposeHead(), lib_tbl[MI_DIS_USER_INFO].mcode,
        lib_tbl[MI_DIS_USER_INFO].msg_header );
    dLen = strlen(prn_buf);

    sprintf(&prn_buf[dLen],
		"\n  ------------------------------------------------------------------------------");
    dLen += strlen(&prn_buf[dLen]);

    sprintf(&prn_buf[dLen],
		"\n  USR ID          LEVEL  LOGIN  LAST_LOGIN_TIME CONNECT_IP       IP_ADDRESS");
    dLen += strlen(&prn_buf[dLen]);

    sprintf(&prn_buf[dLen],
		"\n  ------------------------------------------------------------------------------");
    dLen += strlen(&prn_buf[dLen]);

    for( i=0; i<stAdmin.usTotalCnt; i++)
    {
		inaddr.s_addr = stAdmin.stUserList[i].lRegIP;

		if( stAdmin.stUserList[i].tLastLogin != 0 )
		{
        	sprintf(&prn_buf[dLen],
				"\n  %-16s  %-3s    %-3s  %s  %-16s  %-16s",
            	stAdmin.stUserList[i].szUserName,
#if 0	/* 20040518,poopee */
            	(stAdmin.stUserList[i].dUserLevel == 0)?"S":((stAdmin.stUserList[i].dUserLevel==1)?"N":"G" ),
#else
            	(stAdmin.stUserList[i].dUserLevel == 0)?"S":"N",
#endif
				((stAdmin.dConnectFlag[i] == 0)?"N":"Y"),
				time_short(stAdmin.stUserList[i].tLastLogin),
				((stAdmin.dConnectFlag[i] == 0)?"-":util_cvtipaddr(NULL, stAdmin.stUserList[i].lLastLoginIP)),
				(stAdmin.stUserList[i].lRegIP==0)?"-" :inet_ntoa(inaddr) );
		}
		else
		{
			sprintf(&prn_buf[dLen],
				"\n  %-16s  %-3s    %-3s  -               %-16s  %-16s",
                stAdmin.stUserList[i].szUserName,
#if 0	/* 20040518,poopee */
                (stAdmin.stUserList[i].dUserLevel == 0)?"S":((stAdmin.stUserList[i].dUserLevel==1)?"N":"G" ),
#else
            	(stAdmin.stUserList[i].dUserLevel == 0)?"S":"N",
#endif
				((stAdmin.dConnectFlag[i] == 0)?"N":"Y"),
				((stAdmin.dConnectFlag[i] == 0)?"-":util_cvtipaddr(NULL, stAdmin.stUserList[i].lLastLoginIP)),
				(stAdmin.stUserList[i].lRegIP==0)?"-" :inet_ntoa(inaddr));
		}
        dLen += strlen(&prn_buf[dLen]);
    }

	sprintf(&prn_buf[dLen],
        "\n  ..............................................................................");
    dLen += strlen(&prn_buf[dLen]);

    sprintf(&prn_buf[dLen],
        "\n  TOTAL COUNT = %d", stAdmin.usTotalCnt );
    dLen += strlen(&prn_buf[dLen]);

    sprintf(&prn_buf[dLen],
		"\n  ------------------------------------------------------------------------------");
    dLen += strlen(&prn_buf[dLen]);

	sprintf( &prn_buf[dLen], "\nCOMPLETED\n");
    dLen += strlen(&prn_buf[dLen]);

    log_print(LOGN_DEBUG, prn_buf );

    /*** SEND RESULT USING TCP/IP *********************************************/
    send_text_user( sfd, (char *)prn_buf, END, ml, 1, dConTblIdx );

	//dHistoryMMCD( ml, 1 );

    return 1;
}
#endif


/*******************************************************************************

*******************************************************************************/
int dis_his_cmd( In_Arg Para[], int sfd, mml_msg *ml )
{
	FILE 	*fp;
	int		dLen;
	int		dResult;
	int		dTime;
	int		dSpaceCnt;
	int		dRecordCnt;
	int		dDftRecordCnt;
	int		dConTblIdx;
	char	szBuffer[1024];
	char	szName[24];
	char	szCommandName[32];
	char	*szCommand = NULL;
	char	szYear[4];
	char	szMon[2];
	char	szDay[2];
	char	szHour[2];
	time_t		tStart, tEnd, tCheck;
	struct tm	*stStartTM;
	struct tm 	*stEndTM;
	struct tm	*stCheckTM;

	int		i;
	int		dCurPage;
	int		dTotPage;
	st_HisData	stHisData[MAX_TOT_RECORD];

	time_t	tNow;
	struct tm	*stTM;
	char		szLogFile[128];

	szBuffer[0] = 0x00;

	memset( prn_buf, 0x00, sizeof(prn_buf) );

	tNow = time(&tNow);
	stTM = localtime(&tNow);
	stStartTM = localtime(&tNow);
	stEndTM = localtime(&tNow);
	stCheckTM = localtime(&tNow);

	dConTblIdx = dGetConTblIdx( sfd );

	if( ml->num_of_para == 0 )
	{
		// sprintf( szLogFile, "%s%02d/%02d/%02d/COMMAND_LOG.dat", DEF_HIS_LOG_PATH, stTM->tm_year-100, stTM->tm_mon+1, stTM->tm_mday );
		sprintf( szLogFile, "%s%02d/%02d/%sCOM_HIS.dat",
            DEF_HIS_PATH, stTM->tm_mon+1, stTM->tm_mday, DEF_COM_LOG_PATH );

		if( (fp = fopen( szLogFile, "r")) == NULL)
		{
			sprintf( prn_buf, "\n%s %sM%04d  %s\n  RESULT = FAIL",
        		SmsName, (char*)ComposeHead(), lib_tbl[MI_DIS_HIS_CMD].mcode,
        		lib_tbl[MI_DIS_HIS_CMD].msg_header );

        	dLen = strlen( prn_buf );

        	sprintf( &prn_buf[dLen], "\n  REASON = CANNOT OPEN FILE" );
        	dLen += strlen( &prn_buf[dLen] );

			sprintf( &prn_buf[dLen], "\nCOMPLETED\n");
			dLen += strlen( &prn_buf[dLen] );

        	send_text_user( sfd, (char *)prn_buf, DBM_FAILURE, ml, 0, dConTblIdx );

			//dHistoryMMCD( ml, 0 );

			log_print(LOGN_CRI, "[ERROR] FILE OPEN[%s]", szLogFile );

        	return 0;
		}

		dTotPage = 1;
		dRecordCnt = 0;

		sprintf( prn_buf, "\n%s %sM%04d  %s\n  RESULT = SUCCESS",
        	SmsName, (char*)ComposeHead(), lib_tbl[MI_DIS_HIS_CMD].mcode,
        	lib_tbl[MI_DIS_HIS_CMD].msg_header );
    	dLen = strlen(prn_buf);

		sprintf(&prn_buf[dLen],
			"\n    ------------------------------------------------------------------");
		dLen += strlen(&prn_buf[dLen]);

		sprintf(&prn_buf[dLen],
			"\n    USER ID          TIME                      RESULT    COMMAND ");
		dLen += strlen(&prn_buf[dLen]);

		sprintf(&prn_buf[dLen],
            "\n    ------------------------------------------------------------------");
        dLen += strlen(&prn_buf[dLen]);

		while( fgets(szBuffer, 1024, fp) != NULL )
		{
			szBuffer[strlen(szBuffer)-1] = 0x00;

        	if( szBuffer[0] != '#' )
            	break;
        	else if( szBuffer[1] == 'C' )
        	{
            	sscanf( &szBuffer[2], "%s %d %d", szName, &dResult, &dTime );
				strcpy(stHisData[dRecordCnt].szUserID, szName );
				stHisData[dRecordCnt].dResult = dResult;
				stHisData[dRecordCnt].tTime = dTime;

				if( strcmp( szName, ml->adminid ) )
				{
					continue;
				}
            	szCommand = strstr( szBuffer, " " );
            	*szCommand++;
            	dSpaceCnt = 0;
            	while(dSpaceCnt < 3)
            	{
                	szCommand = strstr( szCommand, " " );
					*szCommand++;
                	dSpaceCnt++;
            	}
        	}

			strcpy(stHisData[dRecordCnt].szCommand, szCommand );

			dRecordCnt++;

			//if( dRecordCnt >= MAX_DAY_RECORD_CNT )
			//	break;
		}

		if( dRecordCnt > 50 )
			dDftRecordCnt = 50;
		else
			dDftRecordCnt = dRecordCnt;

		dTotPage = ((dDftRecordCnt%MAX_ROW_CNT) == 0) ? dDftRecordCnt/MAX_ROW_CNT : dDftRecordCnt/MAX_ROW_CNT + 1;

		dCurPage = 1;

		for(i=0; i<dDftRecordCnt; i++)
		{
			// HIDE PASSWORD
			sscanf( stHisData[dRecordCnt-(i+1)].szCommand, "%s", szCommandName );

			if( strcasecmp( szCommandName, "user-login" ) == 0 || strcasecmp( szCommandName, "add-user-info" ) == 0 ||
				strcasecmp( szCommandName, "chg-user-info" ) == 0 )
			{
				stHisData[dRecordCnt-(i+1)].szCommand[strlen(szCommandName)] = 0x00;
			}

			sprintf(&prn_buf[dLen],
                "\n    %-16s %s   %s   %-s",
                stHisData[dRecordCnt-(i+1)].szUserID, time_str(stHisData[dRecordCnt-(i+1)].tTime),
                ((stHisData[dRecordCnt-(i+1)].dResult==1)?"SUCCESS":"FAILURE"),
                stHisData[dRecordCnt-(i+1)].szCommand );
            dLen +=strlen(&prn_buf[dLen]);

			if( (i+1)%MAX_ROW_CNT == 0 )
			{
				if( i != (dDftRecordCnt-1) )
				{
				sprintf(&prn_buf[dLen],
            		"\n    ------------------------------------------------------------------");
        		dLen += strlen(&prn_buf[dLen]);

				sprintf( &prn_buf[dLen], "\nCONTINUE \n");

                send_text_his( sfd, (char *)prn_buf, dCurPage, dTotPage, ml, dConTblIdx );

log_print(LOGN_DEBUG, "[INFO] [TOT PAGE][%d], [CUR PAGE][%d]", dTotPage, dCurPage );

                dLen = 0;
				dCurPage++;

				sprintf(&prn_buf[dLen],
            		"\n    ------------------------------------------------------------------");
        		dLen += strlen(&prn_buf[dLen]);

        		sprintf(&prn_buf[dLen],
            		"\n    USER ID          TIME                      RESULT    COMMAND ");
        		dLen += strlen(&prn_buf[dLen]);

        		sprintf(&prn_buf[dLen],
            		"\n    ------------------------------------------------------------------");
        		dLen += strlen(&prn_buf[dLen]);
				}
				else
				{
					break;
				}
			}
		}

		sprintf(&prn_buf[dLen],
            "\n    ..................................................................");
        dLen += strlen(&prn_buf[dLen]);

        sprintf(&prn_buf[dLen],
            "\n    TOTAL COUNT = %d", dDftRecordCnt );
        dLen += strlen(&prn_buf[dLen]);

		sprintf(&prn_buf[dLen],
            "\n    ------------------------------------------------------------------");
        dLen += strlen(&prn_buf[dLen]);

		sprintf( &prn_buf[dLen], "\nCOMPLETED\n");
        dLen += strlen(&prn_buf[dLen]);

		send_text_his( sfd, (char *)prn_buf, dCurPage, dTotPage, ml, dConTblIdx );

		//dHistoryMMCD( ml, 1 );

		log_print(LOGN_DEBUG, "[DEFAULT DIS_HIS_CMD] [TOTAL PAGE] [%d] [CUR PAGE] [%d]  [TOTAL RECORD] [%d] [REAL] [%d]",
							dTotPage, dCurPage,  dRecordCnt, dDftRecordCnt );

		fclose(fp);
	}
	else if( ml->num_of_para == 2 )
	{
		memset( stStartTM, 0x00, sizeof(struct tm) );
		memset( stEndTM, 0x00, sizeof(struct tm) );
		memset( stCheckTM, 0x00, sizeof(struct tm) );

		memcpy( szYear, &Para[0].value[0], 4 );
		stStartTM->tm_year = atoi(szYear)-1900;

		memcpy( szMon, &Para[0].value[4], 2 );
        stStartTM->tm_mon = atoi(szMon)-1;

		memcpy( szDay, &Para[0].value[6], 2 );
        stStartTM->tm_mday = atoi(szDay);

		memcpy( szHour, &Para[0].value[8], 2 );
        stStartTM->tm_hour = atoi(szHour);

		tStart = mktime( stStartTM );


		memcpy( szYear, &Para[1].value[0], 4 );
        stEndTM->tm_year = atoi(szYear)-1900;

		memcpy( szMon, &Para[1].value[4], 2 );
        stEndTM->tm_mon = atoi(szMon)-1;

		memcpy( szDay, &Para[1].value[6], 2 );
        stEndTM->tm_mday = atoi(szDay);

		memcpy( szHour, &Para[1].value[8], 2 );
		stEndTM->tm_hour = atoi(szHour);

		tEnd = mktime( stEndTM );

		log_print(LOGN_DEBUG,"[%s] [%s]", Para[0].value, Para[1].value );
		log_print(LOGN_DEBUG,"[START] [%d] [%d] [%d] [%d]", stStartTM->tm_year, stStartTM->tm_mon, stStartTM->tm_mday, stStartTM->tm_hour);
		log_print(LOGN_DEBUG,"[END]   [%d] [%d] [%d] [%d]", stEndTM->tm_year, stEndTM->tm_mon, stEndTM->tm_mday, stEndTM->tm_hour);
		log_print(LOGN_DEBUG,"START[%ld] END[%ld] NOW[%ld]", tStart, tEnd, tNow );

		if( (tEnd - tStart) > WEEK_TIME_T ) /*** IN ONE WEEK ******************/
		{
			sprintf( prn_buf, "\n%s %sM%04d  %s\n  RESULT = FAIL",
            	SmsName, (char*)ComposeHead(), lib_tbl[MI_DIS_HIS_CMD].mcode,
            	lib_tbl[MI_DIS_HIS_CMD].msg_header );

            dLen = strlen( prn_buf );

            sprintf( &prn_buf[dLen], "\n  REASON = EXCEED TIME PERIOD (MAX : 7 DAYS)" );
            dLen += strlen( &prn_buf[dLen] );

			sprintf( &prn_buf[dLen], "\nCOMPLETED\n");
			dLen += strlen( &prn_buf[dLen] );

            send_text_user( sfd, (char *)prn_buf, DBM_FAILURE, ml, 0, dConTblIdx );

            return 0;

		}
		else if( (tEnd - tStart) < 0 ) /*** End Time FASTER THAN Start Time ***/
		{
			sprintf( prn_buf, "\n%s %sM%04d  %s\n  RESULT = FAIL",
            	SmsName, (char*)ComposeHead(), lib_tbl[MI_DIS_HIS_CMD].mcode,
            	lib_tbl[MI_DIS_HIS_CMD].msg_header );

            dLen = strlen( prn_buf );

            sprintf( &prn_buf[dLen], "\n  REASON = START TIME BIGGER THAN END TIME" );
            dLen += strlen( &prn_buf[dLen] );

			sprintf( &prn_buf[dLen], "\nCOMPLETED\n");
            dLen += strlen( &prn_buf[dLen] );

            send_text_user( sfd, (char *)prn_buf, DBM_FAILURE, ml, 0, dConTblIdx );

            return 0;

		}
		else if( tNow < tStart ) /*** NOW TIME FASTER THAN START TIME *********/
		{
			sprintf( prn_buf, "\n%s %sM%04d  %s\n  RESULT = FAIL",
            	SmsName, (char*)ComposeHead(), lib_tbl[MI_DIS_HIS_CMD].mcode,
            	lib_tbl[MI_DIS_HIS_CMD].msg_header );

            dLen = strlen( prn_buf );

            sprintf( &prn_buf[dLen], "\n  REASON = START TIME BIGGER THAN NOW TIME" );
            dLen += strlen( &prn_buf[dLen] );

			sprintf( &prn_buf[dLen], "\nCOMPLETED\n");
            dLen += strlen( &prn_buf[dLen] );

            send_text_user( sfd, (char *)prn_buf, DBM_FAILURE, ml, 0, dConTblIdx );

            return 0;

		}
		else
		{
			if( tEnd > tNow )
				tEnd = tNow;

			tCheck = tStart;

			dLen = 0;

			dTotPage = 1;
        	dRecordCnt = 0;

			sprintf( prn_buf, "\n%s %sM%04d  %s\n  RESULT = SUCCESS",
            	SmsName, (char*)ComposeHead(), lib_tbl[MI_DIS_HIS_CMD].mcode,
            	lib_tbl[MI_DIS_HIS_CMD].msg_header );
        	dLen = strlen(prn_buf);

			sprintf(&prn_buf[dLen],
				"\n    ------------------------------------------------------------------");
            dLen += strlen(&prn_buf[dLen]);

            sprintf(&prn_buf[dLen],
            	"\n    USER ID          TIME                      RESULT    COMMAND ");
            dLen += strlen(&prn_buf[dLen]);

            sprintf(&prn_buf[dLen],
            	"\n    ------------------------------------------------------------------");
            dLen += strlen(&prn_buf[dLen]);

			while(1)
			{
				stCheckTM = localtime( &tCheck );

				//sprintf( szLogFile, "%s%02d/%02d/%02d/COMMAND_LOG.dat",
                //	DEF_HIS_LOG_PATH, stCheckTM->tm_year-100, stCheckTM->tm_mon+1, stCheckTM->tm_mday );

				sprintf( szLogFile, "%s%02d/%02d/%sCOM_HIS.dat",
                    DEF_HIS_PATH, stCheckTM->tm_mon+1, stCheckTM->tm_mday, DEF_COM_LOG_PATH );

log_print(LOGN_DEBUG, "FILE[%s]", szLogFile );

				if( (fp = fopen( szLogFile, "r")) == NULL)
        		{
					log_print(LOGN_DEBUG,"[DIS_HIS_CMD] CANNOT FILE OPEN : NO HISTORY DATA ");

					tCheck += DAY_TIME_T;

					if( (tCheck - tEnd) > DAY_TIME_T )
                    {
						break;
					}

            		continue;
        		}

				while( fgets(szBuffer, 1024, fp) != NULL )
        		{
            		szBuffer[strlen(szBuffer)-1] = 0x00;

            		if( szBuffer[0] != '#' )
                		break;
            		else if( szBuffer[1] == 'C' )
            		{
                		sscanf( &szBuffer[2], "%s %d %d", szName, &dResult, &dTime );
						strcpy(stHisData[dRecordCnt].szUserID, szName );
                		stHisData[dRecordCnt].dResult = dResult;
                		stHisData[dRecordCnt].tTime = dTime;

						if( dTime > tEnd )
							break;

                		if( strcmp( szName, ml->adminid ) )
                		{
                    		continue;
                		}

						if( dTime < tStart )
							continue;

                		szCommand = strstr( szBuffer, " " );
                		*szCommand++;

                		dSpaceCnt = 0;
                		while(dSpaceCnt < 3)
                		{
                    		szCommand = strstr( szCommand, " " );
							*szCommand++;
                    		dSpaceCnt++;
                		}
            		}

					strcpy(stHisData[dRecordCnt].szCommand, szCommand );

					dRecordCnt++;

            		//if( (dRecordCnt+1)%MAX_ROW_CNT == 0 )
                	//	dTotPage++;

        		} /*** FILE READ WHILE END ***/

				fclose( fp );

				tCheck += DAY_TIME_T;
//log_print(LOGN_DEBUG,"[CHECK][%d] END[%d]", tCheck, tEnd );
				if( (tCheck -  tEnd) > DAY_TIME_T )
                {
					break;
				}
			} /*** WHILE(1) END ***/

			if( dRecordCnt > 50 )
                dRecordCnt = 50;
            else
                dRecordCnt = dRecordCnt;

			dTotPage = ((dRecordCnt%MAX_ROW_CNT) == 0) ? dRecordCnt/MAX_ROW_CNT : dRecordCnt/MAX_ROW_CNT + 1;

			dCurPage = 1;

			for(i=0; i<dRecordCnt; i++)
        	{
				// HIDE PASSWORD
            	sscanf( stHisData[i].szCommand, "%s", szCommandName );

            	if( strcasecmp( szCommandName, "user-login" ) == 0 || strcasecmp( szCommandName, "add-user-info" ) == 0 ||
                	strcasecmp( szCommandName, "chg-user-info" ) == 0 )
            	{
                	stHisData[i].szCommand[strlen(szCommandName)] = 0x00;
            	}

            	sprintf(&prn_buf[dLen],
                	"\n    %-16s %s   %s   %-s",
                	stHisData[i].szUserID, time_str(stHisData[i].tTime),
                	((stHisData[i].dResult==1)?"SUCCESS":"FAILURE"),
                	stHisData[i].szCommand );
            	dLen +=strlen(&prn_buf[dLen]);

            	if( (i+1)%MAX_ROW_CNT == 0 )
            	{
					if( i != (dRecordCnt-1) )
					{
						sprintf(&prn_buf[dLen], "\n    ------------------------------------------------------------------");
	                	dLen += strlen(&prn_buf[dLen]);
	                	sprintf( &prn_buf[dLen], "\nCONTINUE \n");
	                	send_text_his( sfd, (char *)prn_buf, dCurPage, dTotPage, ml, dConTblIdx );
	                	dLen = 0;
						dCurPage++;
						sprintf(&prn_buf[dLen], "\n    ------------------------------------------------------------------");
	                	dLen += strlen(&prn_buf[dLen]);
	                	sprintf(&prn_buf[dLen], "\n    USER ID          TIME                      RESULT    COMMAND ");
	                	dLen += strlen(&prn_buf[dLen]);
	                	sprintf(&prn_buf[dLen], "\n    ------------------------------------------------------------------");
	                	dLen += strlen(&prn_buf[dLen]);
					}
					else
						break;
            	}
        	}

			sprintf(&prn_buf[dLen],
                "\n    ..................................................................");
            dLen += strlen(&prn_buf[dLen]);

            if( (i+1)%MAX_ROW_CNT == 0 ) {
                sprintf(&prn_buf[dLen], "\n    TOTAL COUNT = %d", i+1 );
                dLen += strlen(&prn_buf[dLen]);
            }
            else {
                sprintf(&prn_buf[dLen], "\n    TOTAL COUNT = %d", i );
                dLen += strlen(&prn_buf[dLen]);
            }

        	sprintf(&prn_buf[dLen],
            	"\n    ------------------------------------------------------------------");
        	dLen += strlen(&prn_buf[dLen]);

        	sprintf( &prn_buf[dLen], "\nCOMPLETED\n");
        	dLen += strlen(&prn_buf[dLen]);

			send_text_his( sfd, (char *)prn_buf, dCurPage, dTotPage, ml, dConTblIdx );

			//dHistoryMMCD( ml, 1 );

        	log_print(LOGN_DEBUG, "[DEFAULT DIS_HIS_CMD] [TOTAL PAGE] [%d] [CUR PAGE] [%d] [TOTAL RECORD] [%d]",
                            	dTotPage, dCurPage, dRecordCnt );
		}
	}
	else
	{
		sprintf( prn_buf, "\n%s %sM%04d  %s\n  RESULT = FAIL",
        	SmsName, (char*)ComposeHead(), lib_tbl[MI_DIS_HIS_CMD].mcode,
        	lib_tbl[MI_DIS_HIS_CMD].msg_header );

        dLen = strlen( prn_buf );

        sprintf( &prn_buf[dLen], "\n  REASON = PARAMETER NOT MATCH" );
        dLen += strlen( &prn_buf[dLen] );

		sprintf( &prn_buf[dLen], "\nCOMPLETED\n");
		dLen += strlen( &prn_buf[dLen] );

        send_text_user( sfd, (char *)prn_buf, DBM_FAILURE, ml, 0, dConTblIdx );

        return 0;
	}

	return 1;
}

/*******************************************************************************
 Type=1:Index, =2:User Level, =3:IP Flag
*******************************************************************************/
int GetUserInfo( char *UserName, int Type )
{
	int		i;
	int		dRet = -1, dRet2;

	dRet2 = dAdminInfoInit();

	for( i=0; i<stAdmin.usTotalCnt; i++)
    {
        if( !strcmp( UserName, stAdmin.stUserList[i].szUserName ) )
        {
			if( Type == 1 )
			{
				/*** ADMIN INFORMATION INDEX **********************************/
				dRet = i;
			}
			else if( Type == 2 )
			{
				/*** ADMIN LEVEL **********************************************/
				dRet = stAdmin.stUserList[i].dUserLevel;
log_print(LOGN_INFO, "GetUserInfo, type[%d], dRet[%d]", Type, dRet);
			}
			else if( Type == 3 )
			{
				/*** ADMIN IP FLAG ********************************************/
				dRet = stAdmin.stUserList[i].dIPFlag;
			}
			else
			{
				return dRet;
			}

            break;
        }
    }

	return dRet;
}

/*******************************************************************************

*******************************************************************************/
int InitUserInfo( int dIndex, int dCntFlag )
{
	stAdmin.usTotalCnt = stAdmin.usTotalCnt - dCntFlag;
	stAdmin.dConnectFlag[dIndex] = 0;

	stAdmin.stUserList[dIndex].szUserName[0] = 0x00;
	stAdmin.stUserList[dIndex].szUserPass[0] = 0x00;
	stAdmin.stUserList[dIndex].dUserLevel = 0;
	stAdmin.stUserList[dIndex].dIPFlag = 0;
	stAdmin.stUserList[dIndex].tLastLogin = 0;
	stAdmin.stUserList[dIndex].tLastLogout = 0;
	stAdmin.stUserList[dIndex].lLastLoginIP = 0;
	stAdmin.stUserList[dIndex].lRegIP = 0;

	return 1;
}


/*******************************************************************************

*******************************************************************************/
int dAdminInfoInit(void)
{
	int			i, dRet, dCount;
	st_UserAdd	stUserInfo[MAX_USER];

	dRet	= 0;
	memset(stUserInfo, 0x00, sizeof(st_UserAdd)*MAX_USER);
	if( (dRet = dGetUserInfo(&stMySQL, stUserInfo, &dCount)) < 0)
		return -1;

    stAdmin.usTotalCnt = dCount;
    for(i = 0; i < stAdmin.usTotalCnt; i++)
    {

        sprintf(stAdmin.stUserList[i].szUserName, "%s", szCut_string(stUserInfo[i].szUserName));
        sprintf(stAdmin.stUserList[i].szUserPass, "%s", szCut_string(stUserInfo[i].szPassword));
        stAdmin.stUserList[i].dUserLevel = stUserInfo[i].sSLevel;
        stAdmin.stUserList[i].tLastLogin = stUserInfo[i].uLastLoginTime;
        stAdmin.stUserList[i].lLastLoginIP = stUserInfo[i].uConnectIP;

    }

    qsort( (void*)stAdmin.stUserList, stAdmin.usTotalCnt, sizeof(T_ADMIN_DATA), name_cmp_sort);

	return 0;
}

/*******************************************************************************

*******************************************************************************/
int bin_print( char *outbuf, unsigned short usLen, int sfd, short sRet, short cont_flag, int dConTblIdx  )
{
	int			dRet, dRet1;
    st_MngPkt  	output;

    char  buf[2048], head[100];

    memset( &output, 0x00, sizeof(st_MngPkt) );
    memset( &buf[0], 0x00, sizeof(buf) );

    if( cli_msg.head.ucBinFlag == 0x00 )
    {
        strcpy( head, (char*)ComposeHead() );
        head[strlen(head)-1] = '\0';

        sprintf( buf, "\n%s\n%s", head, outbuf );

        if( cont_flag == DBM_CONTINUE )
            strcat( buf, "\nCONTINUED\n");
        else
            strcat( buf, "\n\nCOMPLETED\n");

        strcpy( output.data, buf );
        output.head.usBodyLen = strlen(buf);
    }
    else
    {
        log_print(LOGN_DEBUG,"BinResult %d", cli_msg.head.ucBinFlag );
        //LOGN_hexa( &outbuf[0], usLen );
        memcpy( &output.data[0], &outbuf[0], usLen );
        output.head.usBodyLen = usLen;
    }

	output.head.ucBinFlag = cli_msg.head.ucBinFlag;

    output.head.llMagicNumber = MAGIC_NUMBER;
    output.head.llIndex = cli_msg.head.llIndex;

    if( sRet == DBM_SUCCESS )
    {
        output.head.usResult = cont_flag;
    }
    else
    {
        output.head.usResult = sRet;
    }

	output.head.usTotLen = output.head.usBodyLen+MNG_PKT_HEAD_SIZE;

    output.head.usSrcProc = cli_msg.head.usSrcProc;
    memcpy( output.head.TimeStamp, cli_msg.head.TimeStamp,
            sizeof( cli_msg.head.TimeStamp) );
    memcpy( output.head.userName, cli_msg.head.userName, MAX_USER_NAME_LEN );


    output.head.ucSvcID = SID_MML;
    output.head.ucMsgID = MID_MML_RST;
    memcpy( &output.head.ucmmlid[0], &cli_msg.head.ucmmlid[0], sizeof(unsigned short) );

	log_print(LOGN_DEBUG,"Ret Data\n%s", output.data );

    dRet = dSendMessage( sfd, output.head.usTotLen, (char*)&output, dConTblIdx );
	dRet1 = dSetSockBlock( dRet, sfd, dConTblIdx );

    return 1;

    return 0;
}

/*******************************************************************************

*******************************************************************************/
int dis_cmd_exe( In_Arg Par[], int sfd, mml_msg *ml )
{
	int		i;
	int		dLen;
	int		dCount = 0;
	int		dConTblIdx;
	time_t	tNow, tRemain;
	char	szDate[16];
	char	szCommand[24];

	prn_buf[0] = 0x00;

	dConTblIdx = dGetConTblIdx( sfd );

	tNow = time( &tNow );

	sprintf( prn_buf, "\n%s %sM%04d  %s\n  RESULT = SUCCESS",
    		SmsName, (char*)ComposeHead(), lib_tbl[MI_DIS_CMD_EXE].mcode,
    		lib_tbl[MI_DIS_CMD_EXE].msg_header );
    dLen = strlen( prn_buf );

	sprintf(&prn_buf[dLen],
		"\n  ------------------------------------------------------------------------------");
    dLen += strlen(&prn_buf[dLen]);

    sprintf(&prn_buf[dLen],
		"\n  JOBID USR NAME         EXECUTE TIME   REMAIN/TOTAL REMAIN(SEC) COMMAND");
    dLen += strlen(&prn_buf[dLen]);

    sprintf(&prn_buf[dLen],
        "\n  ------------------------------------------------------------------------------");
    dLen += strlen(&prn_buf[dLen]);

	for( i=0; i<MAX_TMR_REC; i++ )
	{
		if( run_tbl->time[i] > 0 )
		{
			sscanf( &run_tbl->szCommandString[i][0], "%s", szCommand );
			szCommand[strlen(szCommand)] = 0x00;

			memcpy( szDate, &run_tbl->time_stamp[i][5], 15 );
			szDate[14] = 0x00;

			//tRemain = run_tbl->period[i] - ( tNow - run_tbl->stat_check_time[i] );
			tRemain = ( run_tbl->stat_check_time[i] - tNow );
			if( tRemain < 0 )
				tRemain = 0;

			/*** 현재 실행중인 명령어 *****************************************/
			sprintf( &prn_buf[dLen],
				"\n  %3d   %-16s %-15s%6d/%-3d   %6d      %-24s",
				//i+1, run_tbl->user_name[i], run_tbl->time_stamp[i],
				i, run_tbl->user_name[i], szDate,
				run_tbl->stat_TOTAL[i], run_tbl->stat_TOT_NUM[i], (int)tRemain, szCommand );
			dLen += strlen(&prn_buf[dLen]);

			dCount++;
		}
	}

	if( dCount == 0 )
	{
		sprintf(&prn_buf[dLen],
        	"\n  NO CURRENT COMMAND");
    	dLen += strlen(&prn_buf[dLen]);
	}

	sprintf(&prn_buf[dLen],
		"\n  ------------------------------------------------------------------------------");
    dLen += strlen(&prn_buf[dLen]);

	sprintf(&prn_buf[dLen],
        "\nCOMPLETED\n");
    dLen += strlen(&prn_buf[dLen]);

	send_text_user( sfd, (char *)prn_buf, END, ml, 1, dConTblIdx );

	//dHistoryMMCD( ml, 1 );

    return 1;

}


/*******************************************************************************

*******************************************************************************/
int del_cmd_exe( In_Arg Par[], int sfd, mml_msg *ml )
{
	int 	dLen;
	int		dIndex;
	int		dConTblIdx;

	prn_buf[0] = 0x00;

	dConTblIdx = dGetConTblIdx( sfd );

	dIndex = atoi( Par[0].value );

	if( run_tbl->cmd_id[dIndex] == 0 && run_tbl->msg_id[dIndex] == 0 )
	{
		 sprintf( prn_buf, "\n%s %sM%04d  %s\n  RESULT = FAILURE",
        	SmsName, (char*)ComposeHead(), lib_tbl[MI_DEL_CMD_EXE].mcode,
           	lib_tbl[MI_DEL_CMD_EXE].msg_header );
    	dLen = strlen( prn_buf );

    	sprintf( &prn_buf[dLen], "\n  REASON : INVALID INDEX ");
    	dLen += strlen(&prn_buf[dLen]);

    	sprintf( &prn_buf[dLen], "\nCOMPLETED\n");
    	dLen += strlen(&prn_buf[dLen]);

    	send_text_user( sfd, (char *)prn_buf, DBM_FAILURE, ml, 0, dConTblIdx );

    	//dHistoryMMCD( ml, 0 );

		return 1;
	}

	clear_my_tmr( dIndex );

	sprintf( prn_buf, "\n%s %sM%04d  %s\n  RESULT = SUCCESS",
       	SmsName, (char*)ComposeHead(), lib_tbl[MI_DEL_CMD_EXE].mcode,
           lib_tbl[MI_DEL_CMD_EXE].msg_header );
   	dLen = strlen( prn_buf );

	sprintf( &prn_buf[dLen], "\n    MMC JOB [%d] CANCELED",dIndex);
	dLen += strlen(&prn_buf[dLen]);

	sprintf( &prn_buf[dLen], "\nCOMPLETED\n");
    dLen += strlen(&prn_buf[dLen]);

	send_text_user( sfd, (char *)prn_buf, END, ml, 1, dConTblIdx );

	return 1;
}

/*
*
*/
int dGetCOMString(In_Arg Par[], int sfd, mml_msg *ml)
{
	FILE	*fp;
	int		dConTblIdx;
	size_t	szLen, szStrLen;
	char	sBuf[BUF_SIZE*2], sSendBuf[MAX_MNGPKT_BODY_SIZE];

	szLen		= 0;
	szStrLen	= 0;
	prn_buf[0]	= 0x00;
	dConTblIdx = dGetConTblIdx(sfd);

	if( (fp = fopen(COMMAND_FILE_PATH, "r")) == NULL)
	{
		log_print(LOGN_CRI, "%s NOT FOUND, errno=%d\n", COMMAND_FILE_PATH, errno);

		sprintf(prn_buf, "\n%s %sM%04d  %s\n  RESULT = FAILURE", SmsName, (char*)ComposeHead(), lib_tbl[MI_GET_COM].mcode, lib_tbl[MI_GET_COM].msg_header);
		szLen = strlen(prn_buf);

		sprintf(&prn_buf[szLen], "\n  REASON : FILE NOT FOUND DQMS_COM");
		szLen += strlen(&prn_buf[szLen]);

		sprintf(&prn_buf[szLen], "\nCOMPLETED\n");
		szLen += strlen(&prn_buf[szLen]);

		send_text_user(sfd, (char*)prn_buf, DBM_FAILURE, ml, 0, dConTblIdx);
	}

	while(fgets(sBuf, BUF_SIZE*2, fp) != NULL)
	{
		szLen = strlen(sBuf);
		while(isspace(sBuf[szLen-1]))
			sBuf[--szLen] = 0x00;

		sBuf[szLen++] = '\n';

		if( !strncasecmp(sBuf, "#@D", 3) )
			continue;

		if( dIsExceptionCmd(sBuf) ){

				while(fgets(sBuf, BUF_SIZE*2, fp) != NULL)
				{
					szLen = strlen(sBuf);
					while(isspace(sBuf[szLen-1]))
						sBuf[--szLen] = 0x00;

					if(strlen(sBuf) == 0)
						break;
					else
						log_print(LOGN_DEBUG, "F=%s:%s.%d: sBuf[%s]", __FILE__, __FUNCTION__, __LINE__, sBuf);
				}
			continue;
		} 
			
		if( (szStrLen + szLen) > MAX_MNGPKT_BODY_SIZE)
		{
			/*	SEND DQMS_COM INFORMATION	*/
			send_com_data(sfd, (char *)sSendBuf, DBM_CONTINUE, ml, 0, dConTblIdx);
			szStrLen = 0;
		}

		if(strncasecmp(sBuf, "#@1", 3) == 0)
		{
			strcpy(&sSendBuf[szStrLen], &sBuf[4]);
			szStrLen += strlen(&sBuf[4]);
		}
		else
		{
			strcpy(&sSendBuf[szStrLen], &sBuf[0]);
			szStrLen += strlen(sBuf);
		}
	}
	fclose(fp);

	/*	SEND DQMS_COM INFORMATION	*/
	if(szStrLen != 0)	/* 20040619,poopee */
		send_com_data(sfd, (char*)sSendBuf, END, ml, 0, dConTblIdx);
	else
		send_com_data(sfd, "", END, ml, 0, dConTblIdx);

	return 1;
}
