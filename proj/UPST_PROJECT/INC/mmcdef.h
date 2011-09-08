#ifndef __MMCDEF_H__
#define __MMCDEF_H__

#include <sys/types.h> /* u_xxx type */

/*
	명령어 관련하여 MMCD와 handler 간의
	메세지및 필요한 정보를 기록해 놓음
*/

#define MAX_TMR_REC				128

/*
	명령어 처리 결과 각종 에러 코드를 정의해 놓음 
*/

#define CONT					20

/*
	명령어에서 사용하는 MI_ 로 시작하는 MID
*/

#define MI_DIS_USER_LOGIN		220
#define MI_CHG_USER_PASS		221

#define MI_ACT_PRC				300
#define MI_DACT_PRC				301
#define MI_DIS_PRC				302

#define MI_ACT_SUB_PRC			303
#define MI_DACT_SUB_PRC			304
#define MI_DIS_SUB_PRC			305
#define MI_DIS_TIMER			307

#define MI_DIS_SUB_SESS			308
#define MI_SUB_TIMER			309		/* 중복 */
#define MI_SET_TIMER			309

#define MI_DIS_MASK_ALM			310
#define MI_MASK_ALM				311

#define MI_DIS_SVC_SW			313
#define MI_SET_SVC_SW			314

#define MI_DIS_MASK_SVC			320
#define MI_MASK_SVC				321

#define MI_DIS_ALM_LEVL			330
#define MI_SET_ALM_LEVL			331

#define MI_DIS_SVCALM_LEVL		350
#define MI_SET_SVCALM_LEVL		351


#define MI_DIS_LOG_LEVL			400
#define MI_SET_LOG_LEVL			401

#define MI_DIS_HIS_CMD		 	800
#define MI_DIS_CMD_EXE		 	802
#define MI_DEL_CMD_EXE		 	803

#define MI_DIS_SVC				1000
#define MI_ADD_SVC				1001
#define MI_DEL_SVC				1002
#define MI_CHK_SVC				1003
#define MI_CPY_SVC				1004

#define MI_DIS_EQUIP_INFO		1100
#define MI_ADD_EQUIP_INFO		1101
#define MI_DEL_EQUIP_INFO		1102
#define MI_CHK_EQUIP_INFO		1103
#define MI_CPY_EQUIP_INFO		1104

#define MI_DIS_MNIP				1105
#define MI_ADD_MNIP				1106
#define MI_DEL_MNIP				1107
#define MI_CHK_MNIP				1108
#define MI_CPY_MNIP				1109


#define MI_DIS_ROAM_INFO		1110
#define MI_ADD_ROAM_INFO		1111
#define MI_DEL_ROAM_INFO		1112
#define MI_CHK_ROAM_INFO		1113
#define MI_SET_ROAM_INFO		1114

#define MI_DIS_MSMODEL			1200
#define MI_ADD_MSMODEL			1201
#define MI_DEL_MSMODEL			1202
#define MI_CHK_MSMODEL			1203
#define MI_CPY_MSMODEL			1204

#define MI_DIS_EQUIP_SG			1210
#define MI_ADD_EQUIP_SG			1211
#define MI_DEL_EQUIP_SG			1212

#define MI_DIS_SCENARIO			1300
#define MI_ADD_SCENARIO			1301
#define	MI_DEL_SCENARIO			1302
#define MI_CHK_SCENARIO			1304
#define MI_CPY_SCENARIO			1305


#define MI_DIS_MSC_INFO			1400
#define MI_ADD_MSC_INFO			1401
#define MI_DEL_MSC_INFO			1402
#define MI_CHK_MSC_INFO			1403
#define MI_CPY_MSC_INFO			1404

#define MI_DIS_THRES_INFO		1500
#define MI_CHG_THRES_INFO		1501

#define MI_RELOAD_INFO			1502
#define MI_SND_DNMS_CONF		1503

#define MI_DIS_HTTP_METHOD		1600
#define MI_DIS_CONTENT_TYPE		1601
#define MI_DIS_LMS_APP			1602
#define MI_DIS_LMS_ADDR			1603
#define MI_DIS_BREW_CMD			1604
#define MI_DIS_A11_APP			1605
#define MI_DIS_SVC_OPT			1606
#define MI_DIS_NET_INFO			1607
#define MI_DIS_LOC_INFO			1608
#define MI_DIS_SVC_TYPE			1609

#define MI_DIS_SYS_INFO			1700
#define MI_DIS_SUB_SYS_INFO		1701



#define MI_MASK_EDF				3105
#define MI_UNMASK_EDF			3106

#define MI_MASK_NTAF_CHN		3107
#define MI_UMASK_NTAF_CHN		3108
#define MI_MASK_INTERLOCK_CHN	3109
#define MI_UMASK_INTERLOCK_CHN	3110

#define MI_MASK_DIRECTOR_PORT	3111
#define MI_UMASK_DIRECTOR_PORT	3112

#define MI_MASK_SWITCH_PORT		3113
#define MI_UMASK_SWITCH_PORT	3114

#define MI_DIS_NTAF				4000
#define MI_CHG_NTAF				4001
#define MI_DIS_ALM_NTAF			4002
#define MI_CHG_ALM_NTAF			4003

#define MI_DIS_NTAM				4100
#define MI_CHG_NTAM				4101
#define MI_DIS_ALM_NTAM			4102
#define MI_CHG_ALM_NTAM			4103
#define MI_DIS_NTAM_LINK		4104	/* 중복 */
#define MI_DIS_NTAF_LINK		4104

#define MI_DIS_GTAM				4200
#define MI_CHG_GTAM				4201
#define MI_DIS_ALM_GTAM			4202
#define MI_CHG_ALM_GTAM			4203
#define MI_DIS_ALM_GTAM			4202
#define MI_CHG_ALM_GTAM			4203

#define MI_GET_NTAF_INFO		4300
#define MI_GET_BSM_INFO			4301
#define MI_GET_EDF_INFO			4302
#define MI_GET_NTAM_INFO		4400

#define MI_DIS_TRC_INFO			4500
#define MI_ADD_TRC_INFO			4501
#define MI_DEL_TRC_INFO			4502

#define MI_DIS_SVR_TRC			4503
#define MI_ADD_SVR_TRC			4504
#define MI_DEL_SVR_TRC			4505


/* added by uamyd about SG - phase */
#define MI_DIS_SGIP_FLT			4600
#define MI_ADD_SGIP_FLT			4601
#define MI_DEL_SGIP_FLT			4602

#define MI_DIS_GT_INFO			4700
#define MI_ADD_GT_INFO			4701
#define MI_DEL_GT_INFO			4702

#define MI_DIS_USER_INFO		5000
#define MI_ADD_USER				5001
#define MI_DEL_USER				5002
#define MI_CHG_USER_INFO		5003
#define MI_USER_LOGIN			5004
#define MI_USER_LOGOUT		 	5005
#define MI_KILL_USER			5006

#define MI_MASK_NTP_ALM			6109

#define MI_MASK_PRC				7001
#define MI_UNMASK_PRC			7002

#define MI_HIS_NTAM_LOAD_STAT	7003
#define MI_HIS_NTAM_FLT_STAT	7004
#define MI_HIS_NTAM_NTAF_STAT	7005

#define MI_DIS_NTAM_LOAD_STAT	7006
#define MI_DIS_NTAM_FLT_STAT	7007
#define MI_DIS_NTAM_NTAF_STAT	7008

#define MI_DIS_NET_ALM			7009
#define MI_SET_NET_ALM			7010

#define MI_DIS_FAULT_STAT		7011
#define MI_DIS_LOAD_STAT		7012

#define MI_DIS_SVC_ALM			7013
#define MI_SET_SVC_ALM			7014

#define MI_GET_COM				9900

#define MI_DIS_CLST_INFO		9910

#define MI_DIS_DNS_INFO			9911
#define MI_ADD_DNS_INFO			9912
#define MI_DEL_DNS_INFO			9913

#define MI_DIS_GTAM_FLT_STAT	9920
#define MI_HIS_GTAM_FLT_STAT	9921
#define MI_DIS_GTAM_LOAD_STAT	9922
#define MI_HIS_GTAM_LOAD_STAT	9923

#define MI_MAKE_HASH			9925

#define MI_DIS_NET_STAT			9930

#define MI_DIS_SVCURL_INFO		9934
#define MI_ADD_SVCURL_INFO		9935
#define MI_DEL_SVCURL_INFO		9936
#define MI_CHK_SVCURL_INFO		9937

#define MI_NTAM_PKG_DOWN		9940
#define MI_NTAM_PKG_PATCH		9941
#define MI_NTAM_PKG_ROLLBACK	9942
#define MI_NTAF_PKG_DOWN		9943
#define MI_NTAF_PKG_PATCH		9944
#define MI_NTAF_PKG_ROLLBACK	9945
#define MI_GTAM_PKG_PATCH		9946
#define MI_GTAM_PKG_ROLLBACK	9947

#define MI_DIS_FLT_HIS			9948

#define MI_DIS_FTP_TRC			9950
#define MI_ADD_FTP_TRC			9951
#define MI_DEL_FTP_TRC			9952

#define MI_DIS_TRAFFIC_STAT		9953

#define MI_DIS_SCTP				9954
#define MI_ADD_SCTP				9955
#define MI_DEL_SCTP				9956

#define MI_DIS_MONTHRES			9957
#define MI_CHG_MONTHRES			9958

#define MI_DIS_TREND_INFO		9959
#define MI_DIS_DEFECT_INFO		9960

#define MI_ADD_MONTHRES			9961


/* S_MNG 에서 사용하는 SID, MID */


/* Variable Type : 변경시 주의바람
 cmd_load.c에 상응하는 string있음 */

#define VT_DECSTR				0
#define VT_DECIMAL				1
#define VT_STRING				2
#define VT_ENUM					3
#define VT_HEXA					4
#define VT_HEXSTR				5
#define VT_HTIME				6
#define VT_YTIME				7
#define VT_NONABS				8
#define VT_EPOS					9
#define VT_LONGDECIMAL			10
#define VT_MTIME				11
#define VT_NUM_TYPE				12

#define MAX_USER_NAME			25
#define MAX_USER_NAME_LEN		24
#define MAX_USER				500
#define MAX_SENDBUF_LEN			204800

#define MSG_DATA_LEN			12000
#define MAX_MMCD_MSG_SIZE		(MSG_DATA_LEN+1000)
#define MAX_MIN_SIZE			16
#define MAX_MIN_LEN				(MAX_MIN_SIZE - 1)
#define MAX_EXTRA_SIZE			32
#define MAX_FDSET2				20
#define MAX_MNG_PKT_BUFSIZE		1000000



#if 0 /* 일단 사용하는 것만 꺼내서 사용하자. */
#define COMMAND_FILE_PATH	START_PATH"/DATA/DQMS_COM"
#define ENUM_LIST_PATH		START_PATH"/DATA/DQMS_ENUM"
#define SO_LIB_PATH		 START_PATH"/BIN/libsmsprn.so"	/* lib_load.h 에서만 사용*/
#define DEFAULT_MMC_FUNC	"P_default_function"	/* lib_load.h 에서만 사용 */

#define MAX_PARA_SYMBOL	 2048

#define MAX_COM_SYMBOL		512

#define MAX_STRING_BUFFER	262144	/* 256K User String Area */

#define MAX_COMSTR_LEN		32

typedef int (* COMPFUNC)(const	void	*,	const	void *);

#endif

typedef struct {
#define MMLCONTENTS 64
	u_short		para_id;
	u_short		para_type;
	char		para_cont[MMLCONTENTS];
} mml_body, *pmml_body;

typedef struct {
#define MMLNUMOFPARA 16
	u_short		src_proc;
	u_short		src_func;
	u_short		dst_proc;
	u_short		dst_func;
	u_short		msg_id;
	u_short		cmd_id;
	u_short		msg_len;
	u_short		dummy;
	char		adminid[24];
	u_short		num_of_para;
	u_short		eom_mark;
	mml_body	msg_body[MMLNUMOFPARA];
} mml_msg, *pmml_msg;

#define DEF_MMLHEADER_SIZE	sizeof(mml_msg) - sizeof(mml_body)*MMLNUMOFPARA
#define DEF_MMLBODY_SIZE	sizeof(mml_body)

#define PRINT_UNIT 20

/*
*	결과를 받기 위한 구조체 정보
*/

typedef struct _IXPC_HEAD
{
	short	src_proc;
	short	src_func;
	short	dst_proc;
	short	dst_func;
	short	msg_id;
	short	cmd_id;
	short	msg_len;
	short	dummy;
} IXPC_HEAD;

typedef struct _MML_COMMON
{
	short	mml_err;		/*	명령어 실행시 error code를 나타내는 필드		*/
	short	cont_flag;		/*	1: CONTINUE, 0: END 여부						*/
	short	curr_cnt;		/*	현재 return되는 통계수집 횟수: use in nms		*/
							/*	현재 RECORD의 개수							*/
	short	total_cnt;		/*	통계수집 총 횟수: use in nms					*/
							/*	전체 RECORD의 개수							*/
	short	time_out;
	short	TotPage;		/*	PAGE단위의 전체 PAGE: 하나의 통계 DATA에 대한	*/
	short	CurPage;		/*	PAGE단위의 현재 PAGE: 하나의 통계 DATA에 대한	*/
	short	StatFlag;
} MML_COMMON;

typedef struct _MML_RESULT
{
	IXPC_HEAD	head;
	MML_COMMON	common;	 /*	추가된 필드								 */
	char		data[MSG_DATA_LEN];
} MML_RESULT;

typedef struct _dbm_msg_t
{
#define MSG_DATA_LEN 12000
	IXPC_HEAD	head;
	MML_COMMON	common;
	char		data[MSG_DATA_LEN];
} dbm_msg_t;

#define DEF_DBMMSG_SIZE	sizeof(dbm_msg_t)

/* HHBAEK - mmcd_comm.h 로 이동
typedef struct _In_Arg
{
	char		name[32];
	char		value[64];
} In_Arg;
*/


/** HHBAEK 추가 **/
typedef struct _st_MngHead
{
	long long		llMagicNumber;	/* Magic Number 0x3812121281282828L*/
	long long		llIndex;		/* Request for DB Index Used NTAFUIF */

	short			usResult;		/* Result,	PERIODIC TIME COMMAND FROM OMP = 1 */
	unsigned short	usReserved1;
	unsigned int	usTotLen;		/* Packet Header + Packet Length */
	unsigned int	usBodyLen;		/* Packet Body Length + Extended Len */
	unsigned short	usSrcProc;	 
	unsigned short	usReserved2;

	unsigned short	usTotPage;		/* TOTAL COUNT				: NEW ADDED */
	unsigned short	usCurPage;		/* CURRENT COUNT 			: NEW ADDED */
	unsigned short	usStatFlag;		/* 0:NORMAL, 1:STAT COMMAND : NEW ADDED */
	unsigned short	usReserved;

	unsigned char	ucNTAMID;
	unsigned char	ucNTAFID;
	unsigned char	ucSysNo;		/* System Num	: NTAM:1~2 NTAF:1~32 BDF:1~16 SUCC_R : 1~7 */
	unsigned char	ucSvcID;
	unsigned char	ucMsgID;
	unsigned char	ucmmlid[2];
	unsigned char	ucBinFlag;

	char			TimeStamp[40];
	char			userName[MAX_USER_NAME_LEN]; /* 24 */
}st_MngHead, *pst_MngHead;

#define MNG_PKT_HEAD_SIZE 		sizeof(st_MngHead)
#define MAX_MNGPKT_BODY_SIZE 	MAX_MMCD_MSG_SIZE

typedef struct _St_MngPkt
{
	st_MngHead	head;
	char		data[MAX_MNGPKT_BODY_SIZE];
} st_MngPkt, *pst_MngPkt;


/* S_MNG, MMCD 에서 사용 */
typedef struct _st_UserAdd
{
#define USERINFO					21
#define USERLN						21
	char			szUserName[USERINFO];
	char			szPassword[USERINFO];
	short			sSLevel;
	unsigned short	usLogin;
	unsigned int	uLastLoginTime;
	unsigned int	uConnectIP;
	char			szLocalName[USERLN];
	char			szContact[USERLN];
	unsigned int	uCreateTime;
} st_UserAdd, *pst_UserAdd;


typedef struct _st_ConTbl
{
	char			cSockBlock;
	char			cRetryCnt;
	char			Reserv[6];
	int				dSfd;
	unsigned int	uiCliIP;
	int				dLastSendTime;
	int				dBufLen;
	char			adminid[16];
	unsigned char	szSendBuffer[MAX_SENDBUF_LEN];
} st_ConTbl, *pstConTbl;

typedef struct _st_TrendInfo
{
	unsigned char	ucOfficeID;
	unsigned char	ucSysType;
	unsigned char	ucSubType;
	unsigned char	ucBSCID;
	unsigned int	uiIP;
	unsigned short	usL4Code;
	unsigned short	usBTSID;
	unsigned char	ucFAID;
	unsigned char	ucSECID;
	unsigned char	ucSYSID;
	unsigned char	ucDefectCode;
} st_TrendInfo, *pst_TrendInfo;

typedef struct _st_MSGBUF
{
	int	 dWidx;
	int	 dRidx;
	int	 dFullTag;
	int	 dReserv;
	char	szBuf[MAX_MNG_PKT_BUFSIZE];
} st_MSG_BUF, *pst_MSG_BUF;

typedef struct _st_ConnInfo
{
#define MAX_IPADDR_SIZE				16
#define MAX_NAME_SIZE				64
#define MAX_PASS_SIZE				21
#define MAX_DBNAME_SIZE				21
	char	szIP[MAX_IPADDR_SIZE];		/*< DB Server IP */
	char	szName[MAX_NAME_SIZE];		/*< User Name */
	char	szPass[MAX_PASS_SIZE];		/*< Password */
	char	szDBName[MAX_DBNAME_SIZE];	/*< DB Name */
} st_ConnInfo, *pst_ConnInfo;



/*
	명령어 처리 결과 각종 에러 코드를 정의해 놓음 
	MMCD/ERRPRNLIB/ERR_PRT.c 에서 사용할 때 반드시 아래 기술해 놓은 값을
	적은 값부터 순서대로 사용해야 함. 
*/

#define DBM_SUCCESS					0
#define DBM_END						0
#define DBM_CONTINUE				1 // redefined 19 line
#define DBM_NOT_LOGON				2
#define DBM_FAILURE					-1

#define eMMCDTimeOut				-23

#define eMANDATORY_MISSED			-509
#define eENUM_LIST_FORMAT			-510
#define eENUM_LIST_ELEMENT			-511
#define eLOAD_CMD_FILE				-520
#define eINVALID_CMD				-521
#define eINVALID_PAPA_IN_CMD		-522
#define eINVALID_SUBSYSTEM			-523
#define eINVALID_PAPA_CONTENT		-524
#define eNOT_FOUND_PRN_FUNC			-530

#define eINVALIDE_SEARCH_TIME		-565

#define eAdminInfoNotRegistered 	-575

#define eDBQUERYERROR				-610
#define eOVERMAXROW			 		-614


#define eINVALID_IP			 		-700

#define eBadParameter				-1001
#define eDataReadError				-1003

#define eBlockNotRegistered			-1020
#define eProcAliveError 			-1021
#define eCHSMDNotDEAD				-1022
#define eProcDeadError				-1024
#define eNeedProcName				-1027
#define eGeneralError				-1102

#define eNotFoundData				-1051

#define eAlreadyRegisteredData		-2023

#define eAlreadyMaskChn				-2030
#define eAlreadyUMaskChn			-2031
#define eNotFindSysNo			 	-2032
#define eDuplicateEntry		 		-2033

#define eINVALID_PARAM_RANGE		-3000
#define eTABLE_NOT_EXIST			-3002
#define eNotCompleteProcess		 	-3003

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/* 필요한 에러 코드가 있다면...명명법을 잘 정의해서...뽑아다 쓰시던가 하시면...*/
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
#if 0
#define eBadParameterBDFNTP	 	-506
#define eBadParameterNTAFNTPC	-507
#define eBadParameterNTAMTSYNC	-508
#define eMANDATORY_MISSED		-509

#define eENUM_LIST_FORMAT		-510
#define eENUM_LIST_ELEMENT		-511

#define eLOAD_CMD_FILE			-520
#define eINVALID_CMD			-521
#define eINVALID_PAPA_IN_CMD	-522
#define eINVALID_SUBSYSTEM		-523
#define eINVALID_PAPA_CONTENT	-524
#define eINVALID_PAPA_ID		-525

#define eNOT_FOUND_PRN_FUNC		-530

#define eDUPLICATE_DSCP_IP		-534
#define eNO_SESSION			 	-535
#define eNO_SUCH_SOCKET_FD		-536
#define eINVALID_DSCP_ID		-537
#define eRELOAD_FAIL_DSCP_INFO	-538
#define eCONNECT_REFUSE_BY_DSCP -539

#define eINACT_DSCP_ID			-540
#define eMAX_SESSION_CNT_OVER	-541
#define eALREADY_INACT_DSCPID	-542
#define eALREADY_ACT_DSCPID	 	-543
#define eREQUIRED_CHANGE_INFO	-544
#define eDUPLICATE_DSCP_ID		-545
#define eMAX_DSCP_LIST_OVER	 	-546
#define eREQUIRED_MORE_PARA	 	-547
#define eNOTREG_WIN_SERVICE	 	-548
#define eALREADY_EXIST_ROUTE	-549

#define ePFX_OUT_OF_BOUND		-550
#define eNO_SUCH_DSCPID		 	-551
#define eNO_SUCH_DSCP_ROUTE	 	-552
#define eINVALID_DSCP_TIMER	 	-553
#define eFAIL_CREATE_ROUTE_LIST -554
#define eOVERFLOW_ROUTE_LIST	-555

/* MASK CHANNEL : 2003. 5. 7 *************/
#define eALREADY_MASK_CHNL		-556
#define eALREADY_UMASK_CHNL	 	-557
#define eNO_SUCH_IPADDRESS		-558
#define eINVALIDE_BDF_RANGE	 	-559

#define eSETNTAFCONFFILE		-560
#define eSETNTAFCONFPARAUNDEFINE -561
#define eSERCATFILENOTFOUND		-562
#define eIPPOOLLISTFILENOTFOUND	-563
#define eINVALIDE_NTAM_RANGE	-564
#define eINVALIDE_SEARCH_TIME	-565
#define eINVALIDE_NTAM_LOAD_RANGE -566

/* 2003. 7. 22 */
#define eMAX_SVC_LIST_OVER		-570
#define eDUPLICATE_SVC_LIST	 	-571
#define eNO_SUCH_WIN_SERVICE	-572
#define eEXAMINE_ROUTE_LIST	 	-573
#define eEQUIPNotRegistered		-574
#define eAdminInfoNotRegistered -575

#define eDBQUERYERROR			-610
#define ePRNTNOTFOUND			-611
#define eCHDRCDFOUND			-612
#define eOVERMAXSVC				-613
#define eOVERMAXROW				-614

#define eINVALID_IP			 	-700
#define eINVALID_IP_RANGE		-701

#define eNEED_CHG_DATA			-705

#define eBadParameter			-1001
#define eFileNotFound			-1002	/* Conf File Not Found */
#define eDataReadError			-1003	/* Data Read Error */
#define eAlreadyMASK			-1004	/* Already MASK ALM */
#define eAlreadyUMASK			-1005	/* Already MASK ALM */
#define eNotSupportIdx			-1006	/* UnInitial Component Idx */
#define eBiggerWARNToCRI		-1007	/* Bigger WARN Than CRI */

#define eMissingMandatory		-1008	/* MISSING MANDATORY */

#define eBlockNotRegistered		-1020
#define eProcAliveError 		-1021
#define eCHSMDNotDEAD			-1022
#define eCHSMDNotALIVE			-1023
#define eProcDeadError			-1024
#define eProcMaskError			-1025
#define eProcUMaskError			-1026
#define eNeedProcName			-1027

#define eCannotBlockMask		-1030

#define eInvalidSysNo			-1040

#define eInvalidNasInfo			-1050
#define eNotFoundData			-1051

#define eGeneralError			-1102

/* 2003/4/14 hwh add */
#define eINVALID_IPAFNO			-2001
#define eINVALID_IPADDRESS		-2002
#define eINVALID_NASTYPE		-2003
#define eALREADY_IP				-2004
#define eNOTREG_IP				-2005
#define ePREFIX_ERR				-2006
#define eNOTREG_PREFIX			-2007
#define eALREADY_PREFIX			-2008
#define eNOTREG_SERCAT			-2009
#define ePKG_VER_ERR			-2010
#define eIPAF_NOT_LOGIN			-2011
#define eALREADY_VER			-2012
#define eSERCAT_MAX_OVER		-2013
#define eCATEGORY_MAX_OVER		-2014
#define eALREADY_SERVICE		-2015
#define eGROUPPORT				-2016
#define ePERIOD_OVER			-2017
#define eNETMASK_ERROR			-2018
#define eGRUPPORTERR			-2019
#define eNASTYPEERROR			-2020

#define eAlreadyRegisteredData	-2023

#define eAlreadyMaskChn			-2030
#define eAlreadyUMaskChn		-2031
#define eNotFindSysNo			-2032
#define eDuplicateEntry		 	-2033

#define eINVALID_PARAM_RANGE	-3000
#define eUNKNOWN_PRCNAME		-3001	/* added by LYH 20050812 */

#define eTABLE_NOT_EXIST		-3002
#define eNotCompleteProcess		-3003


/* 2003/4/14 hwh add */
#define eINVALID_NTAFNO			-2001
#define eINVALID_IPADDRESS		-2002
#define eINVALID_NASTYPE		-2003
#define eALREADY_IP				-2004
#define eNOTREG_IP				-2005
#define ePREFIX_ERR				-2006
#define eNOTREG_PREFIX			-2007
#define eALREADY_PREFIX			-2008
#define eNOTREG_SERCAT			-2009
#define ePKG_VER_ERR			-2010
#define eNTAF_NOT_LOGIN			-2011
#define eALREADY_VER			-2012
#define eSERCAT_MAX_OVER		-2013
#define eCATEGORY_MAX_OVER		-2014
#define eALREADY_SERVICE		-2015
#define eGROUPPORT				-2016
#define ePERIOD_OVER			-2017
#define eNETMASK_ERROR			-2018
#define eGRUPPORTERR			-2019
#define eNASTYPEERROR			-2020
#define eNOTIPPOOL				-2021
#define eMAXPREFIXOVER			-2022
#define eMAXAAAOVER				-2023

// TTTT
#define ePackagePatchError		-2021	/* 중복 */
#define ePackageRollbackError	-2022	/* 중복 */
#define eAlreadyRegisteredData	-2023	/* 중복 */

/* 20040520, sunny */
#define eALREADY_IP_NETMASK	 	-2024
#define eINVALID_BLOCK			-2025
#define eINVALID_MODE			-2026
#define eALREADY_IPC			-2027
#define eMAXIPPOOLOVER			-2030
#define eMAXNASTYPEOVER			-2031

#define eAlreadyMaskNtafChn		-2030	/* 중복 */
#define eAlreadyUMaskNtafChn	-2031	/* 중복 */
// TTTT
#define eNotFindSysNo			-2032
#define eDuplicateEntry			-2033
#define eAlreadyMaskInterlockChn	-2034
#define eAlreadyUMaskInterlockChn	-2035

/* 20040419,poopee */
#define ERR_ALREADY_EXIST		-3000
#define ERR_DATABASE_FULL		-3001
#define ERR_DATA_NOT_FOUND		-3002
#define ERR_IPC_NOT_FOUND		-3003
#define ERR_REGISTERED_IPC		-3004
#define ERR_SAME_ROUTE			-3005
#define ERR_NOINPUT_MASTER		-3006

// TTTT
#define eINVALID_PARAM_RANGE	-3000	/* 중복 */
#define eTABLE_NOT_EXIST		-3002	/* 중복 */
#define eNotCompleteProcess		-3003	/* 중복 */

#define eBadNetmaskRoam             -3004

#endif //////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

#endif /* __MMCDEF_H__ */
