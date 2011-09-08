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
#define MML_NUM_TP_CMD_TBL		100  // mmlCmdTbl의 tuple 갯수 yslim
#endif
#define MML_NUM_TP_CMD_TBL		500  // mmlCmdTbl의 tuple 갯수 yslim

#define MML_NUM_TP_JOB_TBL		256  // mmcdJobTbl의 tuple 갯수
#define MML_NUM_TP_USER_TBL		SOCKLIB_MAX_CLIENT_CNT // mmcdUserTbl의 tuple 갯수
#define MML_NUM_TP_CLIENT_TBL	SOCKLIB_MAX_CLIENT_CNT // mmcdClientTbl의 tuple 갯수


#define MML_NUM_IP_TBL		       	50	//mmcdIPTbl 의 tuple 갯수	// 2009.07.17 by sjs

#define MML_DEFAULT_RES_TIMER	45 // mmcd에서 application으로부터 응답을 기다리는 시간
#define MMCD_MAX_BUILTIN_CMD	32

#define MML_PTYPE_DECIMAL	1  // 파라미터 유형 - 10진수
#define MML_PTYPE_HEXA		2  // 파라미터 유형 - 16진수
#define MML_PTYPE_STRING	3  // 파라미터 유형 - string
#define MML_PTYPE_ENUM		4  // 파라미터 유형 - enumeration
#define MML_PTYPE_FIXSTR	5  // 파라미터 유형 - fixed string
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
// command file에 등록된 명령어들의 정보가 저장되는 table
//
typedef struct {
	char		enumStr[COMM_MAX_NAME_LEN]; // enum_string 
	char		enumStr2[COMM_MAX_NAME_LEN]; // enum_string2 
} EnumPara;

typedef struct {
    char		paraName[COMM_MAX_NAME_LEN];
    char		mandFlag; // mandatory(1) or optional(0) or optional2 (2)
    char		paraType; // 파라미터별 type; ex) decimal, hexa, string, enum 등
    int			minVal;   // minimum value; 입력 가능한 최소값
    int			maxVal;   // maximum value; 입력 가능한 최대값
#define FIXED_VALUE_NUM	5
    int			fixVal[FIXED_VALUE_NUM];
#define MML_MAX_ENUM_ITEM 	120
    EnumPara	enumList[MML_MAX_ENUM_ITEM];
} MMLParaContext;

typedef struct {
    char			cmdName[COMM_MAX_NAME_LEN*2]; // 반드시 첫번째 field에 그대로 둘 것. -> qsort/bsearch에 문제가 생기게된다.
    char			dstSysName[COMM_MAX_NAME_LEN]; // 명령을 처리할 application이 있는 프로세서
    char			dstAppName[COMM_MAX_NAME_LEN]; // 명령을 처리할 application
    char			privilege; // 명령을 입력할 수 있는 user 등급 -> 낮을수록 권한이 높다
    char			confirm;
    char			category[50];
    char			paraCnt;   // 파라미터 갯수; 필수/선택 포함 전체 갯수
    MMLParaContext	paraInfo[MML_MAX_PARA_CNT];
} MMLCmdContext;

typedef struct {
    char	cmdName[COMM_MAX_NAME_LEN*2]; // 반드시 첫번째 field에 그대로 둘 것. -> qsort/bsearch에 문제가 생기게된다.
    char	cmdSlogan[100];
    char	cmdHelp[10000];
} MMLHelpContext;

//
// client로부터 수신한 input string을 분석한 정보가 저장된다.
// - syntax check 과정 등 application으로 request를 보내기 위한 정보들이 임시 관리된다.
//
typedef struct {
    char	paraName[COMM_MAX_NAME_LEN];  // 입력받은 parameter name
    char	paraVal[COMM_MAX_VALUE_LEN];  // 입력받은 parameter value; "aaa=123" 형태로 입력할 수 있으므로 크게 잡아야 한다.
} MMLInputParaContext;

typedef struct 
{
    int			cliSockFd; // 명령을 보낸 client socket fd
    int			confirm;
    int			batchFlag;
    int			cliReqId;  // client에서 할당한 key값
    int			cmdIndex;  // mmlCmdTbl에서의 index
    int			cliIndex;  // mmcdCliTbl에서의 index
    int			clientType;
    int			errorCode;
    char		nmsibFlag; // nmsib에서 보낸 명령인지 표시한다.
    char		inputString[256]; // 입력받은 string 원본
    char		cmdName[COMM_MAX_NAME_LEN*2]; // 입력받은 string에서 command name 부분
    char		userName[COMM_MAX_NAME_LEN]; // 명령을 보낸 client의 user name
    char		paraCnt; // 입력받은 string에서 paramter 부분을 delimeter로 구분하여 잘라낸 갯수
    MMLInputParaContext	paraInfo[MML_MAX_PARA_CNT];
} MMLInputCmdInfo;

//
// 실행중인 명령어 정보가 저장되는 table
// - syntax check후 해당 application으로 request 메시지 처리를 보낸 후 마지막 결과
//	응답을 받을 때까지 유지된다.
// - dis-exe-cmd 명령으로 수행중 명령어 조회시 여기에 등록된 내용을 출력한다.
//
typedef struct 
{
    int			tpInd;        // tuple_indicator; 사용중 표시; empty(0), busy(1)
    int			cliSockFd;    // 명령를 보낸 client와 연결된 socket fd
    int			cliReqId;  // client에서 할당한 key값
    int 		batchFlag;
    int			cmdIndex;     // mmlCmdTbl에서의 index
    time_t		deadlineTime; // Application으로부터 respose를 받을때 까지 기다리는 시각 -> request를 보낸 시각 + 대기시간
    int			msgId4Nmsib;
    int 		clientType;
    char		nmsibFlag;    // nmsib에서 접속된 user인지 표시한다.
    char		inputString[256]; // 입력받은 명령어 string
    char		cmdName[COMM_MAX_NAME_LEN*2]; // command_name
    char		userName[COMM_MAX_NAME_LEN]; // 명령을 입력한 user_name
    char		dstSysName[COMM_MAX_NAME_LEN]; // application이 있는 프로세서
    char		dstAppName[COMM_MAX_NAME_LEN]; // application name
} MmcdJobTblContext;


//
// user 정보가 저장되는 table 정보
// - dis-usr-info 명령으로 조회시 여기에 등록된 내용을 출력한다.
//
typedef struct 
{
    char		userName[COMM_MAX_NAME_LEN];
    char 		name[COMM_MAX_NAME_LEN];
    char		passwd[COMM_MAX_NAME_LEN]; /* password -> encrypt되어 저장된다.*/
    char		privilege;  // user 권한,등급
    int			loginCnt;   // 같은 id로 여러군데에서 login할 수 있으므로 현재 login되어 있는 session 갯수
    char		nmsibFlag;  // nmsib에서 접속할때 사용되는 userName인지 표시한다.
    time_t		lastLoginTime;  // 최근 login 시각
    time_t		lastLogoutTime; // 마지막 logout 시각
#define IPLEN 16				// by june, 2010-03-09, 12자리 ip 처리 오류 원인
    char		ConnIP[IPLEN];
//    unsigned int	connIP;
} MmcdUserTblContext;

//
// 접속되어 있는 client들의 정보가 저장되는 table 정보
// - 같은 id로 여러군데에서 login할 수 있으므로 user table과 별도 관리된다.
//
typedef struct 
{
    int		cliSockFd;
    int		userIndex;  // 해당 user의 user_table에서의 index
    char	userName[COMM_MAX_NAME_LEN];
    char	privilege;  // user 권한,등급
    char	nmsibFlag;  // nmsib에서 접속된 놈인지 표시한다.
    char	hisIndex;  // history buffer의 index
#define MMCD_NUM_HISTORY_BUFF	30
    char	etcFlag;   // 기타 사용
    char	history[MMCD_NUM_HISTORY_BUFF][256];
    time_t	lastHeartBeatTime; /* 마지막으로 heartbeat을 받은 시간 */ // 2009.07.15 by sjs
    int 	useIPListIndex;
} MmcdCliTblContext;


// mmcd에서 직접 처리해야 하는 built-in 명령어 리스트와 처리 function을 등록하는 table
//
typedef struct {
	char	cmdName[COMM_MAX_NAME_LEN*2];
	int		(*func)(MMLInputCmdInfo*);
} MmcdBuiltInCmdVector;

//2009.07.17  by sjs
typedef struct
{
    char 		ipAddress[IPLEN];			// 추가된 아이피주소
    char		userName[COMM_MAX_NAME_LEN];		// 아이피를 추가한 사용자 ID
    char		listFlag;				// 리스트가 비었는지 확인
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
