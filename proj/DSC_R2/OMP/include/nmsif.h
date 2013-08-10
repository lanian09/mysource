#ifndef __NMSIF_H__
#define	__NMSIF_H__


/*********************************************************************
  1. TCP/IP Connection
 *********************************************************************/
#define	NMS_SEG_MAX_BUF		1048576		// 1024*1024
#define	POLLING_FD_TMR		1000
#define	IO_RETRY_CNT		100

#define	PORT_IDX_ALM		0
#define	PORT_IDX_CONS		1
#define	PORT_IDX_CONF		2
#define	PORT_IDX_MMC		3
#define	PORT_IDX_STAT		4
#define	PORT_IDX_LAST		5

typedef struct {
	int		port[PORT_IDX_LAST];
	char	ipaddr[2][20];
} ListenInfo;


#define	MAX_NMS_CON		30

#define	FD_TYPE_LISTEN		100
#define	FD_TYPE_DATA		101

typedef struct{
   	char    mask[MAX_NMS_CON];
	char    level[MAX_NMS_CON];
	int		fd[MAX_NMS_CON];		// listen fd or accept fd
	int		port[MAX_NMS_CON];		// listen port
	int		ipaddr[MAX_NMS_CON];	// local ipaddr or nms ipaddr
	int		ptype[MAX_NMS_CON];		// fd type (FD_TYPE_LISTEN/~DATA)
	int		prev_ptype[MAX_NMS_CON];// prev fd type (FD_TYPE_LISTEN/~DATA)
	int		rxTime[MAX_NMS_CON];	// recent time to receive pkt
} AppFdTbl;

// redefine for SFDB
typedef AppFdTbl	SFM_NMSInfo;

#define SFM_NMSIF_SIZE  sizeof(SFM_NMSInfo)
#define SFM_NMSIF_MASK_CNT	6

#define	NMS_NO_EVENT		10
#define	NMS_CONN_EVENT		11
#define	NMS_DATA_EVENT		12
#define	NMS_CLOSE_EVENT		13

/*********************************************************************/

/*********************************************************************
  2. Message Structure
 *********************************************************************/
#define	MAX_NMS_BUF			4096

#define	PKT1_HEAD_SIZE		8
#define	PKT2_HEAD_SIZE		12
#define	PKT3_HEAD_SIZE		16

/*	bit endian 방식에 의해 X86 계열에서 처리되도록 수정함	*/
typedef struct {
#ifdef _SPARC_
	unsigned int	ver		:4;
	unsigned int	ctrlbit	:1;
	unsigned int	pktype	:3;
#else
	unsigned int	pktype	:3;
	unsigned int	ctrlbit	:1;
	unsigned int	ver		:4;
#endif
	unsigned int	ctrlseq	:8;
	unsigned int	src		:8;
	unsigned int	dest	:8;
} CtrlHead;


typedef struct {
	CtrlHead		chd;
	unsigned short	prmt;
	unsigned short	len;
} PktHead;


typedef struct {
	PktHead		hdr;
	char		data[MAX_NMS_BUF-8];
} CommonPkt;


typedef struct {
	PktHead		hdr;
	char		data[MAX_NMS_BUF-8];
} CtrlPkt, Packet1;


typedef struct {
	PktHead		hdr;
	int			msgtype;
	char		data[MAX_NMS_BUF-12];
} ConsolePkt, Packet2;

typedef struct {
	PktHead			hdr;
	unsigned int	msgtype;
	unsigned short	serno;
	unsigned short	attrcnt;
	char			data[MAX_NMS_BUF-16];
} InteractPkt, Packet3;

#define	NMS_PKT_BLEN		sizeof(CommonPkt)
#define	NMS_PKT_HLEN		sizeof(PktHead)


/*********************************************************************/

/*********************************************************************
  3. Message Field Information
 *********************************************************************/

/* 3.1 Connection Primitive
*/
#define	DISCONN_REQUEST		0x0085
#define	DISCONN_CONFIRM		0x0086
#define	DATA				0x0080

/* 3.2 Message Type Field (item)
*/
#define	MT_ALARM_FAULT		0x00010000
#define	MT_MONITOR_PERF		0x00020000
#define	MT_ANALYZE_PERF		0x00030000
#define	MT_MMC				0x00040000
#define	MT_PLD_CONF			0x00050000
#define	MT_SOFTWARE			0x00060000
#define	MT_HW_CONF			0x00070000
#define	MT_SW_CONF			0x00080000
#define	MT_ROUTE_CONF		0x00090000
#define	MT_PFX_CONF			0x000a0000
#define	MT_TRUNK_CONF		0x000b0000
#define	MT_NO7_CONF			0x000c0000

/* 3.3 Message Type Field (type)
*/
#define	MT_DOWNLOAD			0x00000100
#define	MT_INITIAL			0x00000200
#define	MT_CHANGE			0x00000300
#define	MT_CHANGED			0x00000400
#define	MT_ADDED			0x00000500
#define	MT_REMOVED			0x00000600
#define	MT_UPLOAD			0x00000700

#define	MT_HW_INFO			0x00001000
#define	MT_SW_INFO			0x00002000

/* 3.4 Message Type Field (flow)
*/
#define	MT_POMD_DATA		0x00000001
#define	MT_REQUEST			0x00000002
#define	MT_CONFIRM			0x00000003
#define	MT_RESPONSE			0x00000004
#define	MT_DATA_END			0x00000005
#define	MT_DATA_RCV_OK		0x00000006
#define	MT_READY			0x00000007
#define	MT_CONTINUE			0x00000008
#define	MT_NOT_CONFIRM		0x00000009


/* 3.5 Message Type Primitive
*/
/* 3.5.1 console data => one way primitive (DSCM->NMS)
*/
//  NMS                          DSC
//   |        CONSOLE_DATA        |
//   |<---------------------------|
//
#define	PRMT_CONSOLE_DATA			(MT_ALARM_FAULT | MT_POMD_DATA)

/* 3.5.2 alarm/fault => two way primitive (DSCM<->NMS)
*/
//  NMS                          DSC
//   |      INIT_HWALM_REQ        |
//   |--------------------------->|
//   |                            |
//   |     INIT_HWALM_CONFIRM     |
//   |<---------------------------|
//   |                            |
//   |    INIT_HWALM_DATA(1)      |
//   |<---------------------------|
//   |                            |
//   |    INIT_HWALM_DATA(2)      |
//   |<---------------------------|
//   |    ..................      |
//   |                            |
//   |   INIT_HWALM_DATA_END      |
//   |<---------------------------|
//   |                            |
//   |     INIT_HWALM_RCV_OK      |
//   |--------------------------->|
//
#define	PRMT_INIT_HWALM_REQ			(MT_ALARM_FAULT | MT_INITIAL | MT_HW_INFO | MT_REQUEST)
#define	PRMT_INIT_HWALM_CONFIRM		(MT_ALARM_FAULT | MT_INITIAL | MT_HW_INFO | MT_CONFIRM)
#define	PRMT_INIT_HWALM_DATA		(MT_ALARM_FAULT | MT_INITIAL | MT_HW_INFO | MT_POMD_DATA)
#define	PRMT_INIT_HWALM_DATA_END	(MT_ALARM_FAULT | MT_INITIAL | MT_HW_INFO | MT_DATA_END)
#define	PRMT_INIT_HWALM_RCV_OK		(MT_ALARM_FAULT | MT_INITIAL | MT_HW_INFO | MT_DATA_RCV_OK)

#define	PRMT_INIT_SWALM_REQ			(MT_ALARM_FAULT | MT_INITIAL | MT_SW_INFO | MT_REQUEST)
#define	PRMT_INIT_SWALM_CONFIRM		(MT_ALARM_FAULT | MT_INITIAL | MT_SW_INFO | MT_CONFIRM)
#define	PRMT_INIT_SWALM_DATA		(MT_ALARM_FAULT | MT_INITIAL | MT_SW_INFO | MT_POMD_DATA)
#define	PRMT_INIT_SWALM_DATA_END	(MT_ALARM_FAULT | MT_INITIAL | MT_SW_INFO | MT_DATA_END)
#define	PRMT_INIT_SWALM_RCV_OK		(MT_ALARM_FAULT | MT_INITIAL | MT_SW_INFO | MT_DATA_RCV_OK)

/* 3.5.3 mmc => two way primitive (DSCM<->NMS)
*/
//  NMS                          DSC
//   |        MMC_REQUEST         |
//   |--------------------------->|
//   |                            |
//   |        MMC_CONFIRM         |
//   |<---------------------------|
//   |                            |
//   |          MMC_DATA          |
//   |<---------------------------|
//
#define	PRMT_MMC_REQUEST			(MT_MMC | MT_REQUEST)
#define	PRMT_MMC_CONFIRM			(MT_MMC | MT_CONFIRM)
#define	PRMT_MMC_DATA				(MT_MMC | MT_POMD_DATA)

/* 3.5.4 config => two way primitive (DSCM<->NMS)
*/
//  NMS                          DSC
//   |        HW_EQUIP_REQ        |
//   |--------------------------->|
//   |                            |
//   |      HW_EQUIP_CONFIRM      |
//   |<---------------------------|
//   |                            |
//   |      HW_EQUIP_DATA(1)      |
//   |<---------------------------|
//   |                            |
//   |      HW_EQUIP_DATA(2)      |
//   |<---------------------------|
//   |    ..................      |
//   |                            |
//   |     HW_EQUIP_DATA_END      |
//   |<---------------------------|
//   |                            |
//   |       HW_EQUI_RCV_OK       |
//   |--------------------------->|
//
#define	PRMT_HW_EQUIP_REQ			(MT_HW_CONF | MT_INITIAL | MT_REQUEST)
#define	PRMT_HW_EQUIP_CONFIRM		(MT_HW_CONF | MT_INITIAL | MT_CONFIRM)
#define	PRMT_HW_EQUIP_DATA			(MT_HW_CONF | MT_INITIAL | MT_POMD_DATA)
#define	PRMT_HW_EQUIP_DATA_END		(MT_HW_CONF | MT_INITIAL | MT_DATA_END)
#define	PRMT_HW_EQUIP_RCV_OK		(MT_HW_CONF | MT_INITIAL | MT_DATA_RCV_OK)

#define	PRMT_SW_EQUIP_REQ			(MT_SW_CONF | MT_INITIAL | MT_REQUEST)
#define	PRMT_SW_EQUIP_CONFIRM		(MT_SW_CONF | MT_INITIAL | MT_CONFIRM)
#define	PRMT_SW_EQUIP_DATA			(MT_SW_CONF | MT_INITIAL | MT_POMD_DATA)
#define	PRMT_SW_EQUIP_DATA_END		(MT_SW_CONF | MT_INITIAL | MT_DATA_END)
#define	PRMT_SW_EQUIP_RCV_OK		(MT_SW_CONF | MT_INITIAL | MT_DATA_RCV_OK)

/*********************************************************************/

/* 3.6 Transformation Information
*/
#define	OIDCONF_FILE				"DATA/oidconfig"

#define	NMS_STAT_NUM				16	/* fault, load, rdr, rleg ... ,sms */
#define	NUM_OF_PERIOD				3	/* 5min, 1hour, 1day */

#define	RESET_RCV_FLAG				0
#define	SET_RCV_FLAG				1

typedef struct {
	char	cTableName[NUM_OF_PERIOD][40];
	int		dSidFirstNum;
	int		dObjectID;
} OidInfo;


/*********************************************************************
  4. ETC definition
 *********************************************************************/
#define	STAT_PERIOD_5MIN		1
#define	STAT_PERIOD_HOUR		2

#define	NMS_STAT_DIR			"LOG/NMS_DIR/"

#define	FILE_NUM_5MIN			576		// 5분 단위 3시간 => 48시간 : 2006.08.29
#define	FILE_NUM_HOUR			168		// 1시간 단위 48시간 => 1주일 : 2006.08.29

#define	FILE_NAME_LEN			80

#define	FLAG_NEED_RCV_OK		1
#define	FLAG_NO_NEED_RCV_OK		2

/* statistics file name buffer
 * 1. NMS 접속 시
 *    => 화일명 전송(DSC->NMS) 시 저장 (sfd, flag, time, file_name)
 *    => RCV_OK 수신(DSC<-NMS) 시 해제 & 저장 기간 만료 시 해제
 * 2. NMS 미접속 시
 *    => 통계화일 생성 시 저장 (flag, time, file_name)
 *    => 저장 기간 만료 시 해제
 */
typedef struct {
	int		sfd;
	int		flag;					// FLAG_NEED_RCV_OK, FLAG_NO_NEED_RCV_OK
	int		time;					// initial time
	char	name[FILE_NAME_LEN];	// file name
} HoldFList;

// stmd->nmsif notification
typedef struct {
	int		period;			// STAT_PERIOD_5MIN, STAT_PERIOD_HOUR
	char	stime[20];		// start time : ex) 2006-08-23 17:30
	char	etime[20];		// end time   : ex) 2006-08-23 17:30
} StatQueryInfo;

#endif
