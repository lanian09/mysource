#ifndef __IPAF_SHM_HEADER_FILE__
#define __IPAF_SHM_HEADER_FILE__

#include <time.h>
//#include "ipaf_define.h"	// 071203, poopee
#include "ipaf_names.h"
#include "ipaf_svc.h"

#pragma pack(1)
#define NO_MSG				0
#define NOMSG           	0

/* write shm */
#define Err_BUF_FULL		-200
#define Err_BUF_FULL2		-201

/* read shm */
#define Err_BUF_EMPTY		-202
#define Err_CRITICAL		-203
#define Err_POINTER			-204

typedef struct {
	double idle;
	double wait;
	double user;
	double syst;
} T_CPU_LOAD;

typedef struct {
	double phiscal;
	double free;
} T_MEM_LOAD;

/***
#define ALIVE		0x03
#define DEAD		0x02
#define ABSENT		0x00
#define MASKED		0x01
#define UNMASKED	0x00
#define NORMAL		0x00
#define MINOR		0x01
#define MAJOR		0x02
#define CRITICAL	0x03
#define INITIAL		0x01
#define LISTEN		0x01

#define STOP		0x01
#define NONSTOP		0x00
***/

/* STAT */
#define DEF_TCP_TIMEOUT	300
#define DEF_PPP_TIMEOUT	300

#define MAX_LOAD_SIZE	6
#define MAX_SW_COUNT	60
#define MAX_LINK_COUNT	2
#define MAX_DISK_COUNT	2


/* KEEPALIVE DEFINE */
#define     READY       0
#define     START       1
#define     FINISH      2
#define     PROCESSING  3
#define     BROKEN  	-1

/* 
	LOAD 상태 관리를 위한 구조체 
		alarm : NORMAL, MINOR, MAJOR, CRITICAL 
		mask  : MASK, UNMASK 
	 	usage :  DISK 사용율(%)[xx.xx] 
*/

typedef struct {
	char  		alarm;
	char		mask;
	char		load[MAX_LOAD_SIZE];	
	long long   used;
	long long   total;
}T_MP_LOAD;


/* 
	ETHERNET PORT 상태 관리를 위한 구조체 
		status :  ALIVE, DEAD, ABSENT 
		alarm  :  NORMAL, CRITICAL 
		mask   :  MASK, UNMASK 
*/

typedef struct {
	char	status;  	
	char	alarm;		
	char	mask;		
	char	reserved;
} T_MP_LINK;



/*
	Process 상태를 관리하기 위한 구조체 
		status 	:  	ALIVE, DEAD, ABSENT 
		alarm  	:  	NORMAL, MINOR, MAJOR, CRITICAL 
		mask	:  	MASK, UNMASK 
		stop	:	STOP, NONSTOP
		when	: 	프로세스 상태 변경 시간 
		pid		:  	pid
*/

typedef struct {
	char	status;  	
	char	alarm;		
	char	mask;		
	char	stop;		
	time_t	when;		
	int		pid;
} T_MP_SW;



/*
	Process 상태를 전송하기 위한 구조체 
		status 	:  	ALIVE, DEAD, ABSENT 
		alarm  	:  	NORMAL, MINOR, MAJOR, CRITICAL 
		mask	:  	MASK, UNMASK 
		stop	:	STOP, NONSTOP
		when	: 	프로세스 상태 변경 시간 
*/

typedef struct {
	char	status;  	
	char	alarm;		
	char	mask;		
	char	stop;		
	time_t	when;		
} T_MP_SW_ALMD;


/* 
	DTAF 시스템 전체를 관리하기 위한 정보 테이블 
		alarmlevel 	:  	시스템 전체 장애 등급 	
		mpmqful		:	IPC MESSAGE QUEUE FULL 상태 관리
		mpcpuload   :	CPU 부하율 
		mpmemload	:	MEM 부하율
		mpdisk		:	DISK 부하율 정보
		mplink		:	ETHERNET 상태 정보
		mpsw		: 	PROCESS 상태 정보
*/

typedef struct {
	char  		alarm;
	char		mask;
	char		mpmqful;
	char 		reserved;
	long		pid;
}T_MP_MQ;

typedef struct {
    short  sDtafNo;
	short  sLineNo;
	short  sSvcNo;
	short  sLineInfo;
} T_MP_DTAF;

typedef struct {
	char 		alarmlevel;				
	char		stop;
	char		mask;
	char		status;
	char		reserved[4];
	T_MP_DTAF	mpdtaf;
	T_MP_LOAD	mpcpu;
	T_MP_LOAD	mpmem;	
	T_MP_LOAD	mpdisk;		
	T_MP_LINK 	mpethlink[MAX_LINK_COUNT];		
	T_MP_SW		mpsw[MAX_SW_COUNT];		
} T_TMF_STATUS;

typedef struct {
	char 			alarmlevel;				
	char			stop;
	char			mask;
	char			status;
	char			reserved[4];
	T_MP_DTAF		mpdtaf;
	T_MP_LOAD		mpcpu;
	T_MP_LOAD		mpmem;	
	T_MP_LOAD		mpdisk;
	T_MP_LINK 		mpethlink[MAX_LINK_COUNT];		
	T_MP_SW_ALMD	mpsw[MAX_SW_COUNT];			
} T_TMF_STATUS_ALMD;


/*
	Process 수행 여부를 진단하기 위한 Heart Beat 처리용 구조체  
*/

typedef struct {
    long   minor;
    long   major;
    long   critical;
    long   reserved;
} T_AlarmLoad;

typedef struct {
    long    oldcnt[MAX_SW_COUNT];
    long    cnt[MAX_SW_COUNT];
    short   aply_status;
    short   dtaf_status;
    short   Reserved1;
    short   Reserved2;
	time_t  inittime;
	time_t  resvtime;
    T_AlarmLoad  cpu;
    T_AlarmLoad  mem;
    T_AlarmLoad  disk;
	char	procnorm[MAX_SW_COUNT];
} T_Keepalive;

/* MRG_SHM : OLD
#define	MRG_READER_COUNT	1
#define MRG_MEM_SIZE   		32000000
#define MRG_MEM_COUNT  		3 
*/

/***** MODIFIED BY LEE : 20040303 *****/
#define MRG_READER_COUNT   	1 

/* UPRESTO TEST MEM SIZE */
#define MRG_MEM_SIZE        480000000
//#define MRG_MEM_SIZE		100000000

#define MRG_READER_ANA      0
#define MRG_READER_SVCANA   1


// Shared memory structure - MRG
typedef struct {
	unsigned char	Mem[MRG_MEM_SIZE];
} T_CAPBUF, *PT_CAPBUF;

#define DEF_TCAPBUF_SIZE    sizeof(T_CAPBUF)

typedef struct {
    long    WritePos;
    long    WriteEndPos;
    long    ReadPos[MRG_READER_COUNT];
} T_CAP, *PT_CAP;

#define DEF_TCAP_SIZE       sizeof(T_CAP)

#define	SHM_EMPTY		0x00
#define	SHM_WRITE		0x01
#define	SHM_FINISH		0x02

#define MAX_STAT_SIZE	12
typedef struct {
	time_t			KeyTime;
	int				Reserved;

	unsigned int	uiUpFrames;
	unsigned int	uiDownFrames;
	//unsigned int	uiUpBytes;
	//unsigned int	uiDownBytes;
	INT64			llUpBytes;
	INT64			llDownBytes;
} st_UpDownStat;

typedef struct _st_TotalReqResStat
{
    UINT    uiReqCnt;
    UINT    uiReqRealCnt;
    //UINT    uiReqByte;
	INT64	llReqByte;

    UINT    uiResCnt;
    UINT    uiResRealCnt;
    //UINT    uiResByte;
	INT64	llResByte;
}st_TotalReqResStat, *pst_TotalReqResStat;

typedef struct {
	char	upinf;
	char	downinf;
	char	DebugLevel;
	char	CDRLevel;
	char	IPAFID;
	char	reserved[3];

	int		IdleTime;
	int		RetryTime;

	/* Not Used */
	st_UpDownStat	ThruStat[MAX_STAT_SIZE];

	/* Not Used */
	st_UpDownStat	TotStat[MAX_STAT_SIZE];

	/* TotStat (<-IPStat) */
	st_UpDownStat	IPStat[MAX_STAT_SIZE];
	
	/* UDPStat */
	st_UpDownStat	UDPStat[MAX_STAT_SIZE];	

	/* TCPStat */
	st_UpDownStat	TCPStat[MAX_STAT_SIZE];

	/* IPError */
	st_UpDownStat	IPError[MAX_STAT_SIZE];

	/* UTCPError */
	st_UpDownStat	UTCPError[MAX_STAT_SIZE];

	/* EtcStat (<-TCPReTrans) */
	st_UpDownStat	TCPReTrans[MAX_STAT_SIZE];

	/* UserStat (<-OutOfIP) */
	st_UpDownStat	OutOfIP[MAX_STAT_SIZE];

	/* Not Used */ 
	st_UpDownStat	DropData[MAX_STAT_SIZE];

	/* EtcError (<-FailData) */
	st_UpDownStat	FailData[MAX_STAT_SIZE];

	/* FILTER OUT */
	st_UpDownStat	FilterOut[MAX_STAT_SIZE];

} T_GENINFO;

typedef struct _st_IPAF_CONF {
	int		dIdleTime;
	int	  	dRetryTime;
	char	IPAFID;
	char	UPINF;
	char	CDRLevel;
	char  	LogLevel;
	char	Reserv[4];
	char 	szIPAMIP_A[16];
	char 	szIPAMIP_S[16];
	char 	szIPAFIP[16];
} st_IPAF_CONF, *pst_IPAF_CONF;

/* 040114,poopee */
#define DEF_VERSION_SIZE        7       /* Inserted By LSH in 2004.04.05 */

typedef struct _st_VERSION
{
	char	ver[IPAF_SW_COM][DEF_VERSION_SIZE];
} st_VERSION;

/* 2007.11.27 BY LDH FOR DUPLICATE PACKET */
typedef struct _st_DupNode_
{
    USHORT      usNext;
    USHORT      usPrev;
    USHORT      usIdentification;
    USHORT      usFragmentOffset;
} st_DupNode, *pst_DupNode;

/* 2007.11.27 BY LDH FOR DUPLICATE PACKET */
#define	MAX_DUP_NODE		20
typedef struct _st_DupList_
{
    USHORT      usFirstIndex;
    USHORT      usLastIndex;
    USHORT      usCurrentCount;
    USHORT      usReserved;

	st_DupNode  stNode[MAX_DUP_NODE];
} st_DupList, *pst_DupList;

#pragma pack()
#endif
