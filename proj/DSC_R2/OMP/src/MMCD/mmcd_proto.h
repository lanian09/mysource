#ifndef __MMCD_PROTO_H__
#define __MMCD_PROTO_H__

#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <strings.h>
#include <time.h>
#include <ctype.h>
//#include <mysql.h> by helca
#include <mysql.h>

#include <comm_msgtypes.h>
#include <sysconf.h>
#include <commlib.h>
#include <omp_filepath.h>
#include <sfm_msgtypes.h>
#include <stm_msgtypes.h>
#include <mysql_db_tables.h>
#include <proc_version.h>

#ifdef ORG
#define MML_NUM_TP_CMD_TBL		100  // mmlCmdTbl�� tuple ���� yslim
#endif
#define MML_NUM_TP_CMD_TBL		500  // mmlCmdTbl�� tuple ���� yslim

#define MML_NUM_TP_JOB_TBL		256  // mmcdJobTbl�� tuple ����
#define MML_NUM_TP_USER_TBL		SOCKLIB_MAX_CLIENT_CNT // mmcdUserTbl�� tuple ����
#define MML_NUM_TP_CLIENT_TBL	SOCKLIB_MAX_CLIENT_CNT // mmcdClientTbl�� tuple ����


#define MML_NUM_IP_TBL		       	50	//mmcdIPTbl �� tuple ����	// 2009.07.17 by sjs

#define MML_DEFAULT_RES_TIMER	45 // mmcd���� application���κ��� ������ ��ٸ��� �ð�
#define MMCD_MAX_BUILTIN_CMD	32

#define MML_PTYPE_DECIMAL	1  // �Ķ���� ���� - 10����
#define MML_PTYPE_HEXA		2  // �Ķ���� ���� - 16����
#define MML_PTYPE_STRING	3  // �Ķ���� ���� - string
#define MML_PTYPE_ENUM		4  // �Ķ���� ���� - enumeration
#define MML_PTYPE_FIXSTR	5  // �Ķ���� ���� - fixed string
#define MML_PTYPE_DECSTR	6
#define MML_PTYPE_FIXDEC	7

#define MML_PRIVILEGE_SU	1
#define MML_PRIVILEGE_NU	2
#define MML_PRIVILEGE_GUEST	3

#define MML_PRIVILEGE_SU_STR		"SU"
#define MML_PRIVILEGE_NU_STR		"NU"
#define MML_PRIVILEGE_GUEST_STR		"GUEST"
#define MML_PRIVILEGE_SUPER_STR		"SUPER"
#define MML_PRIVILEGE_NORMAL_STR	"NORMAL"

#define MML_SET_CMD_NEXT		0
#define MML_SET_CMD_COMMAND		1
#define MML_SET_CMD_PARAMETER	2
#define MML_SET_CMD_SLOGAN		3
#define MML_SET_CMD_HELP		4

//
// command file�� ��ϵ� ��ɾ���� ������ ����Ǵ� table
//
typedef struct {
	char		enumStr[COMM_MAX_NAME_LEN]; // enum_string 
	char		enumStr2[COMM_MAX_NAME_LEN]; // enum_string2 
} EnumPara;

typedef struct {
    char		paraName[COMM_MAX_NAME_LEN];
    char		mandFlag; // mandatory(1) or optional(0) or optional2 (2)
    char		paraType; // �Ķ���ͺ� type; ex) decimal, hexa, string, enum ��
    int			minVal;   // minimum value; �Է� ������ �ּҰ�
    int			maxVal;   // maximum value; �Է� ������ �ִ밪
#define FIXED_VALUE_NUM	5
    int			fixVal[FIXED_VALUE_NUM];
#define MML_MAX_ENUM_ITEM 	120
    EnumPara	enumList[MML_MAX_ENUM_ITEM];
} MMLParaContext;

typedef struct {
    char			cmdName[COMM_MAX_NAME_LEN*2]; // �ݵ�� ù��° field�� �״�� �� ��. -> qsort/bsearch�� ������ ����Եȴ�.
    char			dstSysName[COMM_MAX_NAME_LEN]; // ����� ó���� application�� �ִ� ���μ���
    char			dstAppName[COMM_MAX_NAME_LEN]; // ����� ó���� application
    char			privilege; // ����� �Է��� �� �ִ� user ��� -> �������� ������ ����
    char			confirm;
    char			category[50];
    char			paraCnt;   // �Ķ���� ����; �ʼ�/���� ���� ��ü ����
    MMLParaContext	paraInfo[MML_MAX_PARA_CNT];
} MMLCmdContext;

typedef struct {
    char	cmdName[COMM_MAX_NAME_LEN*2]; // �ݵ�� ù��° field�� �״�� �� ��. -> qsort/bsearch�� ������ ����Եȴ�.
    char	cmdSlogan[100];
    char	cmdHelp[10000];
} MMLHelpContext;

//
// client�κ��� ������ input string�� �м��� ������ ����ȴ�.
// - syntax check ���� �� application���� request�� ������ ���� �������� �ӽ� �����ȴ�.
//
typedef struct {
    char	paraName[COMM_MAX_NAME_LEN];  // �Է¹��� parameter name
    char	paraVal[COMM_MAX_VALUE_LEN];  // �Է¹��� parameter value; "aaa=123" ���·� �Է��� �� �����Ƿ� ũ�� ��ƾ� �Ѵ�.
} MMLInputParaContext;

typedef struct 
{
    int			cliSockFd; // ����� ���� client socket fd
    int			confirm;
    int			batchFlag;
    int			cliReqId;  // client���� �Ҵ��� key��
    int			cmdIndex;  // mmlCmdTbl������ index
    int			cliIndex;  // mmcdCliTbl������ index
    int			clientType;
    int			errorCode;
    char		nmsibFlag; // nmsib���� ���� ������� ǥ���Ѵ�.
    char		inputString[256]; // �Է¹��� string ����
    char		cmdName[COMM_MAX_NAME_LEN*2]; // �Է¹��� string���� command name �κ�
    char		userName[COMM_MAX_NAME_LEN]; // ����� ���� client�� user name
    char		paraCnt; // �Է¹��� string���� paramter �κ��� delimeter�� �����Ͽ� �߶� ����
    MMLInputParaContext	paraInfo[MML_MAX_PARA_CNT];
} MMLInputCmdInfo;

//
// �������� ��ɾ� ������ ����Ǵ� table
// - syntax check�� �ش� application���� request �޽��� ó���� ���� �� ������ ���
//	������ ���� ������ �����ȴ�.
// - dis-exe-cmd ������� ������ ��ɾ� ��ȸ�� ���⿡ ��ϵ� ������ ����Ѵ�.
//
typedef struct 
{
    int			tpInd;        // tuple_indicator; ����� ǥ��; empty(0), busy(1)
    int			cliSockFd;    // ��ɸ� ���� client�� ����� socket fd
    int			cliReqId;  // client���� �Ҵ��� key��
    int 		batchFlag;
    int			cmdIndex;     // mmlCmdTbl������ index
    time_t		deadlineTime; // Application���κ��� respose�� ������ ���� ��ٸ��� �ð� -> request�� ���� �ð� + ���ð�
    int			msgId4Nmsib;
    int 		clientType;
    char		nmsibFlag;    // nmsib���� ���ӵ� user���� ǥ���Ѵ�.
    char		inputString[256]; // �Է¹��� ��ɾ� string
    char		cmdName[COMM_MAX_NAME_LEN*2]; // command_name
    char		userName[COMM_MAX_NAME_LEN]; // ����� �Է��� user_name
    char		dstSysName[COMM_MAX_NAME_LEN]; // application�� �ִ� ���μ���
    char		dstAppName[COMM_MAX_NAME_LEN]; // application name
} MmcdJobTblContext;


//
// user ������ ����Ǵ� table ����
// - dis-usr-info ������� ��ȸ�� ���⿡ ��ϵ� ������ ����Ѵ�.
//
typedef struct 
{
    char		userName[COMM_MAX_NAME_LEN];
    char 		name[COMM_MAX_NAME_LEN];
    char		passwd[COMM_MAX_NAME_LEN]; /* password -> encrypt�Ǿ� ����ȴ�.*/
    char		privilege;  // user ����,���
    int			loginCnt;   // ���� id�� ������������ login�� �� �����Ƿ� ���� login�Ǿ� �ִ� session ����
    char		nmsibFlag;  // nmsib���� �����Ҷ� ���Ǵ� userName���� ǥ���Ѵ�.
    time_t		lastLoginTime;  // �ֱ� login �ð�
    time_t		lastLogoutTime; // ������ logout �ð�
#define IPLEN 16				// by june, 2010-03-09, 12�ڸ� ip ó�� ���� ����
    char		ConnIP[IPLEN];
//    unsigned int	connIP;
} MmcdUserTblContext;

//
// ���ӵǾ� �ִ� client���� ������ ����Ǵ� table ����
// - ���� id�� ������������ login�� �� �����Ƿ� user table�� ���� �����ȴ�.
//
typedef struct 
{
    int		cliSockFd;
    int		userIndex;  // �ش� user�� user_table������ index
    char	userName[COMM_MAX_NAME_LEN];
    char	privilege;  // user ����,���
    char	nmsibFlag;  // nmsib���� ���ӵ� ������ ǥ���Ѵ�.
    char	hisIndex;  // history buffer�� index
#define MMCD_NUM_HISTORY_BUFF	30
    char	etcFlag;   // ��Ÿ ���
    char	history[MMCD_NUM_HISTORY_BUFF][256];
    time_t	lastHeartBeatTime; /* ���������� heartbeat�� ���� �ð� */ // 2009.07.15 by sjs
    int 	useIPListIndex;
} MmcdCliTblContext;


// mmcd���� ���� ó���ؾ� �ϴ� built-in ��ɾ� ����Ʈ�� ó�� function�� ����ϴ� table
//
typedef struct {
	char	cmdName[COMM_MAX_NAME_LEN*2];
	int		(*func)(MMLInputCmdInfo*);
} MmcdBuiltInCmdVector;

//2009.07.17  by sjs
typedef struct
{
    char 		ipAddress[IPLEN];			// �߰��� �������ּ�
    char		userName[COMM_MAX_NAME_LEN];		// �����Ǹ� �߰��� ����� ID
    char		listFlag;				// ����Ʈ�� ������� Ȯ��
    char		useFlag;
}MMcdUserIPTblContext;



#define NEXT(a,b)	((a+1)%b)

extern int errno;
extern int mmcd_initial (void);
extern int mmcd_initLog (void);
extern int mmcd_loadUserTbl (void);
extern int mmcd_savePasswdFile (void);
extern int mmcd_rebuildCmdTbl (char*);

extern int mmcd_tokeniseInputString (MMLInputCmdInfo*);
extern int mmcd_fillInputParaName (MMLInputCmdInfo*);
extern int mmcd_arrangeInputPara (MMLInputCmdInfo*);
extern int mmcd_checkMandatoryPara (MMLInputCmdInfo*);
extern int mmcd_checkValidParaVal (MMLInputCmdInfo*);
extern int mmcd_checkValidValueOnType (int, int, char*, char*);
extern int mmcd_checkUserPrivilege (MMLInputCmdInfo*);
extern int mmcd_exeClientDisconn (int);
extern int mmcd_getCmdIndex (char*);
extern int mmcd_getCliIndex (int);
extern int mmcd_getUserIndex (char*);
extern int mmcd_makeReqMsg (MMLInputCmdInfo*, GeneralQMsgType*);
extern int mmcd_saveReqData2JobTbl (MMLInputCmdInfo*, GeneralQMsgType*);
extern int mmcd_logCmdInput (MMLInputCmdInfo*);
extern int mmcd_sendInputAccepted2Client (MMLInputCmdInfo*);
extern int mmcd_sendInputError2Client (MMLInputCmdInfo*);
extern int mmcd_sendInputCancl2Client (MMLInputCmdInfo*);
extern int mmcd_sendTimeOut2Client (int);

extern int mmcd_sendResult2Client (int jobNo, GeneralQMsgType *rxGenQMsg, int clientType);
extern int mmcd_sendCancMsg2App (int);
extern int mmcd_send2COND (char *);

extern int mmcd_isBuiltInCmd (MMLInputCmdInfo*);
extern int mmcd_builtin_log_in (MMLInputCmdInfo*);
extern int mmcd_builtin_log_out (MMLInputCmdInfo*);
extern int mmcd_builtin_rebuild_mml_tbl (MMLInputCmdInfo*);
extern int mmcd_builtin_cmd_help (MMLInputCmdInfo*);
extern int mmcd_builtin_grade_help (MMLInputCmdInfo*);
extern int mmcd_builtin_dis_usr_info (MMLInputCmdInfo*);
extern int mmcd_builtin_dis_cur_usr (MMLInputCmdInfo*);
extern int mmcd_builtin_add_usr (MMLInputCmdInfo*);
extern int mmcd_builtin_del_usr (MMLInputCmdInfo*);
extern int mmcd_builtin_chg_passwd (MMLInputCmdInfo*);
extern int mmcd_builtin_dis_exe_cmd (MMLInputCmdInfo*);
extern int mmcd_builtin_canc_exe_cmd (MMLInputCmdInfo*);
extern int mmcd_builtin_canc_usr (MMLInputCmdInfo*);
extern int mmcd_builtin_send_result (char*, char, char, MMLInputCmdInfo*);
extern int mmcd_builtin_dis_cmd_his (MMLInputCmdInfo*);
extern int mmcd_test (MMLInputCmdInfo*);
extern int mmcd_builtin_stat_cmd_canc (MMLInputCmdInfo*);

extern int mmcd_builtin_heart_beat( MMLInputCmdInfo *inputCmdInfo );	//2009.07.15 by sjs
extern int mmcd_builtin_add_ipaddr( MMLInputCmdInfo *inputCmdInfo );	//
extern int mmcd_builtin_del_ipaddr( MMLInputCmdInfo *inputCmdInfo );	//
extern int mmcd_builtin_dis_ipaddr_info( MMLInputCmdInfo *inputCmdInfo );	//

extern int mmcd_loadCmdTbl (MMLCmdContext*, MMLHelpContext*, char*);
extern int mmcd_setParameter2CmdTbl (MMLCmdContext*, int, int, char*);
extern int mmcd_setSlogan2HelpTbl (MMLHelpContext*, int, char*);
extern int mmcd_setCmdClass2CmdTbl (char*);
extern int mmcd_setCmdCategory2CmdTbl (char*);
extern int mmcd_setParaOpt2CmdTbl (char*);
extern int mmcd_setParaType2CmdTbl (char*);
extern int mmcd_saveCmdInfo2DB (MMLCmdContext*, MMLHelpContext*, int);
extern int mmcd_send2Nmsib (char *,long ,char ,char ,int );

extern char *mmcd_printTimeHHMMSS (time_t);
extern char *mmcd_printTimeMMDDHHMMSS (time_t);
extern char *mmcd_printUserClass (char);
extern char *mmcd_printUserFullClass (char);
extern int mmcd_bsrchCmp (const void *a, const void *b);
extern int mmcd_qsortCmp (const void *a, const void *b);
extern void mmcd_dumpCmdTbl (MMLCmdContext*, MMLHelpContext*);
extern char *mmcd_printParaType (char);

extern int enum_cmp_sort (const void *a, const void *b);
extern void funcEnumSort(EnumPara *enumList);

extern int mmcd_checkUserConfirm ( MMLInputCmdInfo *inputCmdInfo);
extern int mmcd_sendInputConfirm2Client ( MMLInputCmdInfo *inputCmdInfo );
extern int mmcd_sendInputCancel2Client ( MMLInputCmdInfo *inputCmdInfo);
extern int mmcd_exeMmcReqMsg ( SockLibMsgType	*rxSockMsg,	int		cliFd);
extern int mmcd_exeMmcResMsg ( GeneralQMsgType *rxGenQMsg );
extern int mmcd_scanJobTbl (void);
extern int proc_overload ();
extern int proc_check_heart_beat(); 	//2009.07.15 by sjs
extern int check_ipaddr( const char* );	//2009.07.15 by sjs

extern char* GetClientIPstr( unsigned int );	//2009.07.16 by sjs
extern unsigned int GetClientIPhl( unsigned int );	//2009.07.16 by sjs

extern int mmcd_check_ipaddr( const char *clientIP ); //2009.07.17 by sjs

extern int mmcd_loadIPTbl( void ); 		//2009.07.17 by sjs
extern int mmcd_saveIPFile( void ); 	//2009.07.17 by sjs

#endif //__MMCD_PROTO_H__
