#define		NMS_MSG_LEN		4096
#define		NMS_HEAD_LEN	16
#define		NMS_BODY_LEN	4080

#define		LEN_FIELD_IDX	12

#include <sys/types.h>

typedef struct {
	int		packType;
	int		mesgID;
	short	Flag;
	short	seqNum;
	int		length;
	char	parameter[NMS_BODY_LEN];
} NmsMesg;

/*
** mesgID가 int인데 음수는 사용하지 않는다고 생각하고
** 왜냐하면 MP쪽은 건드리지 않기 위해서
*/
#define     SUBS_MMLIN_IND      0x80000000
#define     MASK_SUBS_MMLIN     0x0fffffff

/*----------------------------------------*/
/*  NMS/MPC I/F 메시지의 각 필드 값 정의  */
/*----------------------------------------*/
/* packType 필드 값 (요청)
**/
#define		TR_AppStatusCheck		1
#define		TR_ClosePort			3
#define		TR_InputCommand			5
#define		TR_PortTypeConfirm		8
#define		TR_StartMsgTrans		10
#define		TR_StopMsgTrans			12


/* packType 필드 값 (응답)
**/
#define		TA_AppStatusCheck		2
#define		TA_ClosePort			4
#define		TA_InputCommand			6
#define		TA_OutPutMsg			7
#define		TA_PortTypeConfirm		9
#define		TA_StartMsgTrans		11


/* Sub Message의 Flag 필드 값
**/
#define		DIVIDE_FLAG			0
#define		NON_DIVIDE_FLAG		1

/* parameter는 ASCII형태의 string으로 구성되며
** 처음 1Byte는 Syntax Check Flag,
** 다음 1Byte는 NE 구분자로 이용된다.
*/
#define		SYNTAX_RELGAL		1
#define		SYNTAX_IRREGAL		2

#define		NMC_IPADDR		"DATA/nmsib_port_info"
#define		NMS_LOG			"LOG/NMS_LOG"
#define		ERR_LOG			"LOG/NMS_LOG/ERR_LOG"
#define		ALM_LOG			"LOG/NMS_LOG/ALARM"
#define		STS_LOG			"LOG/NMS_LOG/STATUS"
#define		PSTAT_LOG		"LOG/NMS_LOG/PSTAT"
#define		MMLIN_LOG		"LOG/NMS_LOG/MMLIN"
#define		SSTAT_LOG		"LOG/NMS_LOG/SSTAT"
#define		LOCAL_LOG		"LOG/NMS_LOG/LOCAL"
#define     SUBS_MMLOUT_LOG "LOG/NMS_LOG/SUBS_MMLOUT"


/* PortTypeConfirm 메시지의 파라미터 내용 정의 */
#define		PT_ALARM			1		/* 00000001 */
#define		PT_SSTAT			2		/* 00000010 */
#define		PT_PSTAT			3		/* 00000011 */
#define		PT_STATUS			4		/* 00000100 */
#define		PT_MMLIN			5		/* 00000101 */
#define		PT_LOCAL			6		/* 00000110 */
#define		PT_SUBS_MMLOUT		7		/* 00000111 */
#define		PT_SUBS_MMLIN		8		/* 00001000 */

/* StartMsgTransmission/StopMsgTransmission
** 메시지의 파라미터 내용 정의
*/
#define		MT_ALARM			1		/* 00000001 */
#define		MT_SSTAT			2		/* 00000010 */
#define		MT_PSTAT			3		/* 00000011 */
#define		MT_STATUS			4		/* 00000100 */
#define		MT_MMLIN			5		/* 00000101 */
#define		MT_SUBS_MMLOUT		6		/* 00000110 */


#define		NMS_PORT_NO			3
#define		CLIENT_PORT_NUM		2

#define		LOCAL_RES			87
#define		NMS_RES				88

#define		START_MODE			101
#define		NOSTART_MODE		100

typedef struct {
	short	last_seq;
	short	new_seq;
	short	start_mon;
	short	start_day;
	short	start_hour;
} Rsend;


#define		PORT_CONFIRM		101
#define		START_MSG			102
#define		STOP_MSG			103


typedef struct {
	short	sockfd[10];
	short	portType[10];
	short	startFlag[10];
	short	status[10];
	Rsend	rsend[10];
} NmsPortTable;

#define  MAX_NUM_PORT   20

/* NMS에는 server로 동작한다. */
typedef struct {
	int		fd;			/* connect가 되었을 경우의 FD */
	unsigned int	ip;			/* connect가 되었을 경우의 ip */
	int		flag;		
#	define		SERVER_PORT			1
#	define		CLIENT_PORT			2
	int		type;	
#	define  	PT_MMCD      		(PT_SUBS_MMLIN+1)	
#	define  	PT_COND      		(PT_MMCD+1)
	int		port;		/* port 번호 */
} NmsInfo;

/* LOCAL MMCD, COND에는 client로 동작한다. */
/* SERVER로 접속된 client(NMS)정보도 가지고 있다. */

#define			MAX_NUM_LOCAL		20
typedef struct {
	int			fd;
	int			ip;
	int			size;
	int			leftLen;
	int			port;
	int			flag;		
#	define		CLIENT_CONNECT		3
	int			type;
#	define		MAX_FD_BUF_LEN		8196

	int			noResCnt;
	int			timerStatus;
	int			portType;
	int			startFlag;
	int			status;
	char		cliIpAddr[32];
	Rsend		rsend;
	short		reserved;
	char		rcvBuf[MAX_FD_BUF_LEN];
} LocalInfo;

#define	MAX_NUM_IP		20
typedef struct {
	int			num;
	unsigned int		ip[MAX_NUM_IP];
} LocalIp;

//#define			MAX_NUM_BUFFER		64
#define			MAX_NUM_BUFFER		2048
typedef struct {
	int			len;		/* 현재 buffer의 길이 */
	int 		cmd_id;		/* cmd_id ID :MMCD의 unique ID */
	char		data[819200];
} MsgBuffer;
