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
** mesgID�� int�ε� ������ ������� �ʴ´ٰ� �����ϰ�
** �ֳ��ϸ� MP���� �ǵ帮�� �ʱ� ���ؼ�
*/
#define     SUBS_MMLIN_IND      0x80000000
#define     MASK_SUBS_MMLIN     0x0fffffff

/*----------------------------------------*/
/*  NMS/MPC I/F �޽����� �� �ʵ� �� ����  */
/*----------------------------------------*/
/* packType �ʵ� �� (��û)
**/
#define		TR_AppStatusCheck		1
#define		TR_ClosePort			3
#define		TR_InputCommand			5
#define		TR_PortTypeConfirm		8
#define		TR_StartMsgTrans		10
#define		TR_StopMsgTrans			12


/* packType �ʵ� �� (����)
**/
#define		TA_AppStatusCheck		2
#define		TA_ClosePort			4
#define		TA_InputCommand			6
#define		TA_OutPutMsg			7
#define		TA_PortTypeConfirm		9
#define		TA_StartMsgTrans		11


/* Sub Message�� Flag �ʵ� ��
**/
#define		DIVIDE_FLAG			0
#define		NON_DIVIDE_FLAG		1

/* parameter�� ASCII������ string���� �����Ǹ�
** ó�� 1Byte�� Syntax Check Flag,
** ���� 1Byte�� NE �����ڷ� �̿�ȴ�.
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


/* PortTypeConfirm �޽����� �Ķ���� ���� ���� */
#define		PT_ALARM			1		/* 00000001 */
#define		PT_SSTAT			2		/* 00000010 */
#define		PT_PSTAT			3		/* 00000011 */
#define		PT_STATUS			4		/* 00000100 */
#define		PT_MMLIN			5		/* 00000101 */
#define		PT_LOCAL			6		/* 00000110 */
#define		PT_SUBS_MMLOUT		7		/* 00000111 */
#define		PT_SUBS_MMLIN		8		/* 00001000 */

/* StartMsgTransmission/StopMsgTransmission
** �޽����� �Ķ���� ���� ����
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

/* NMS���� server�� �����Ѵ�. */
typedef struct {
	int		fd;			/* connect�� �Ǿ��� ����� FD */
	unsigned int	ip;			/* connect�� �Ǿ��� ����� ip */
	int		flag;		
#	define		SERVER_PORT			1
#	define		CLIENT_PORT			2
	int		type;	
#	define  	PT_MMCD      		(PT_SUBS_MMLIN+1)	
#	define  	PT_COND      		(PT_MMCD+1)
	int		port;		/* port ��ȣ */
} NmsInfo;

/* LOCAL MMCD, COND���� client�� �����Ѵ�. */
/* SERVER�� ���ӵ� client(NMS)������ ������ �ִ�. */

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
	int			len;		/* ���� buffer�� ���� */
	int 		cmd_id;		/* cmd_id ID :MMCD�� unique ID */
	char		data[819200];
} MsgBuffer;
