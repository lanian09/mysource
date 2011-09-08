/**
	@file		cmd_user.h
	@author
	@version
	@date		2011-07-18
	@brief		cmd_user.c 헤더파일
*/
#ifndef __CMD_USER_H__
#define __CMD_USER_H__

/**
  Include headers
 */
// .
#define COMMAND_FILE_PATH		START_PATH"/DATA/MMC_COM"

typedef struct _In_Arg
{ 
	char    name[32];
	char    value[64];
} In_Arg;

typedef struct {
#define	USER_NAME_LEN				16
#define	USER_PASS_LEN				16
	char			szUserName[USER_NAME_LEN];
	char			szUserPass[USER_PASS_LEN];
	int				dUserLevel;
	int				dIPFlag;
	time_t			tLastLogin;
	time_t			tLastLogout;
	unsigned int	lLastLoginIP;
	unsigned int	lRegIP;
} T_ADMIN_DATA;

typedef struct {
#define	MAX_USER_CNT				200
	unsigned short	usUseFlag;
	unsigned short	usTotalCnt;
	int				dReserved;
	int				dConnectFlag[MAX_USER_CNT];
	T_ADMIN_DATA	stUserList[MAX_USER_CNT];
} T_ADMIN_LIST;

/**
	Declare functions
*/
extern int name_cmp_sort( const void *a, const void *b );
extern int name_cmp( const void *a, const void *b );
extern char *szCut_string(char *str);
extern int usr_login( In_Arg Par[], int sfd, mml_msg *ml );
extern int dis_admin_info( In_Arg Par[], int sfd, mml_msg *ml );
extern int set_admin_info( In_Arg Para[], int sfd, mml_msg *ml );
extern int usr_logout( In_Arg Para[], int sfd, mml_msg *ml );
extern int kill_user( In_Arg Para[], int sfd, mml_msg *ml );
extern int dis_his_cmd( In_Arg Para[], int sfd, mml_msg *ml );
extern int GetUserInfo( char *UserName, int Type );
extern int InitUserInfo( int dIndex, int dCntFlag );
extern int dAdminInfoInit(void);
extern int bin_print( char *outbuf, unsigned short usLen, int sfd, short sRet, short cont_flag, int dConTblIdx  );
extern int dis_cmd_exe( In_Arg Par[], int sfd, mml_msg *ml );
extern int del_cmd_exe( In_Arg Par[], int sfd, mml_msg *ml );
extern int dGetCOMString(In_Arg Par[], int sfd, mml_msg *ml);
extern int send_text_user (int osfd, char *sBuf, int endFlag, mml_msg *ml, int dResult, int dConTblIdx );
extern int send_text_login (int osfd, char *sBuf, int dUserLevel, int endFlag, mml_msg *ml, int dConTbl );
extern int	dGetConTblIdx( int dSockfd );
extern char *time_short( time_t time );
extern char *cvt_ipaddr(unsigned int uiIP);
extern char *time_str( time_t time );
extern void clear_my_tmr( short i);
extern int send_text_his (int osfd, char *sBuf, int curpage, int totpage, mml_msg *ml, int dConTblIdx );
extern int dSendMessage( int dSsfd, int dMsgLen, char *szSmsg, int stConIdx );
extern int dSetSockBlock( int dReturn, int dSockFD, int stConIdx );

#endif /* __CMD_USER_H__ */
