#ifndef __FILEDB_H__
#define __FILEDB_H__

#include <time.h>

#include "config.h"	 /* MAX_SW_COUNT */
#include "commdef.h" /* DATA_PATH */

/* DIRECTOR 관련 정보 */

typedef struct _st_DIRECT
{
#define MAX_MONITOR_PORT_COUNT      10
#define MAX_MIRROR_PORT_COUNT       24
#define MAX_DIRECT_POWER_COUNT      2
    unsigned long   tEachUpTime;                            /*  Each Director is updated at this time_t                                 */
    char            cReserved[4];                           /*  This variable(cReserved) is use for alignment(2bytes, 4bytes, 8bytes)   */
    unsigned char   cMonitorPort[MAX_MONITOR_PORT_COUNT];   /*  Monitor port count: MAX_MONITOR_PORT_COUNT[10]                          */
    unsigned char   cMirrorPort[MAX_MIRROR_PORT_COUNT];     /*  Mirror(Inline) port count: MAX_MIRROR_PORT_COUNT[24]                    */
    unsigned char   cPower[MAX_DIRECT_POWER_COUNT];         /*  Direct power count: MAX_DIRECT_POWER_COUNT[2]                           */
} st_DIRECT, *pst_DIRECT;

typedef struct _st_DIRECT_MNG
{
#define MAX_DIRECT_COUNT 5
    unsigned long   tUpTime;                        /*  If only one director is updated, then this value should be updated.     */

    unsigned char   cDirectorMask[MAX_DIRECT_COUNT];      /*  Direct Masking Flag: This value is 0x02, then director is masked        */
    unsigned char   cReserved[3];                   /*  This variable(cReserved) is use for alignment(2bytes, 4bytes, 8bytes)   */
    st_DIRECT       stDIRECT[MAX_DIRECT_COUNT];
} st_DIRECT_MNG, *pst_DIRECT_MNG;

#define DEF_DIRECT_MNG_SIZE sizeof(st_DIRECT_MNG)

typedef struct _st_SWITCH
{
#define MAX_SWITCH_CPU_COUNT        3
#define MAX_SWITCH_PORT_COUNT       24
#define MAX_SWITCH_MEM_COUNT        2
    unsigned long   tEachUpTime;                            /*  Each Switch is updated at this time_t                                   */

    unsigned char   cSwitchCPUStatus[MAX_SWITCH_CPU_COUNT]; /*  NORMAL: 0x03, CRITICAL: 0x06                                            */
    unsigned char   cSwitchMEMStatus;                       /*  NORMAL: 0x03, CRITICAL: 0x06                                            */

    unsigned char   cSwitchPort[MAX_SWITCH_PORT_COUNT];     /*  Switch PORT count: MAX_SWITCH_PORT_COUNT[24]                            */
    unsigned int    uSwitchCPU[MAX_SWITCH_CPU_COUNT];       /*  Switch CPU count: MAX_SWITCH_CPU_COUNT[3]                               */
    unsigned int    uSwitchMEM[MAX_SWITCH_MEM_COUNT];       /*  Switch MEM count: MAX_SWITCH_MEM_COUNT[2]                               */
} st_SWITCH, *pst_SWITCH;

typedef struct _st_SWITCH_MNG
{
#define MAX_SWITCH_COUNT 2
    unsigned long   tUpTime;                        /*  If only one switch is updated, then this value should be updated.       */

    unsigned char   cSwitchMask[MAX_SWITCH_COUNT];        /*  Masking Flag: This value is 0x02, then director is masked       */
    unsigned char   cReserved[2];                   /*  This variable(cReserved) is use for alignment(2bytes, 4bytes, 8bytes)   */
    st_SWITCH       stSwitch[MAX_SWITCH_COUNT];
} st_SWITCH_MNG, *pst_SWITCH_MNG;

#define DEF_SWITCH_MNG_SIZE sizeof(st_SWITCH_MNG)


/*
	FIDB 관련된 각종 define 들.
*/

typedef struct _st_CurrVal
{
    long long   llCur;
    long long   lMax;
} st_CurrVal, *pst_CurrVal;

typedef struct _st_ProcInfo
{
    long long   pid;
    long long   when;
} st_ProcInfo, *pst_ProcInfo;

// 2011년 07월 이전 TAF,TAM fidb 구조체
#define MAX_DISK_COUNT 4
#define MAX_PWR_COUNT  4
#define MAX_FAN_COUNT  8
#define MAX_CH_COUNT   8
#define MAX_LINK_COUNT 8
#define MAX_NTP_COUNT  2
#define MAX_ICH_COUNT  4
#define MAX_PORT_COUNT 8

#define MAX_DISK_ARRAY_COUNT 128

#define SI_DB_INTERLOCK		0	/* TAM_DB */
#define SI_NMS_INTERLOCK	1	/* NMS */
#define SI_DNMS_INTERLOCK	2	/* DNMS */

/*  TAM_APP FIDB structures     */
typedef struct _st_NTAM
{
    long long       tUpTime;
    int             active;
    int             nifoCnt;
    int             chanCnt;
    unsigned short  usSysChg[8];                    /*  0-NAS, 1-AAA, 2-DSCP, 3-IPAF, 4-IDR */

    unsigned char   cpu;                            /*** STANDBY INFORMATION ***/
    unsigned char   mem;
    unsigned char   queue;
    unsigned char   nifo;
    unsigned char   cDBAlive;                       /*  0x03: NORMAL, 0x06: CRITICAL    */
    unsigned char   disk[MAX_DISK_COUNT];            /*  4   */

    unsigned char   link[MAX_LINK_COUNT];            /*  8   */
    unsigned char   sess[MAX_SW_COUNT];             /*  32, each process session memory load alarm status */
    unsigned char   mpsw[MAX_SW_COUNT];             /*  32  */

    unsigned char   IpcChUnAvail;                   /*  no available IPC channel */
    unsigned char   ucReserved2[6];

    unsigned char   NTAFChnl[MAX_CH_COUNT];        /*  8 - NTAFChannel [0x03: NORMAL, 0x06: CRITICAL]  */
    unsigned char   cInterlock[MAX_ICH_COUNT]; /*  4 - SI_DB: SI_DB_INTERLOCK, (SI_SVCMON: SI_SVCMON_INTERLOCK), SI_NMS: SI_NMS_INTERLOCK [0x03: NORMAL, 0x06: CRITICAL]   */
                                                    /*  SI_SVCMON: SI_SVCMON_INTERLOCK variable don't used, moved to total-mon system, by uamyd 20100927 */

    unsigned char   dReserved3[2];
    unsigned char   hwPWR[MAX_PWR_COUNT];              /*  4, STANDBY INFORMATION ***/
    unsigned char   hwNTP[MAX_NTP_COUNT];              /*  2 */

    unsigned char   hwFan[MAX_FAN_COUNT];              /*** 8, STANDBY INFORMATION ***/
    unsigned char   hwDiskArray[MAX_DISK_ARRAY_COUNT];    /*  128 */

    unsigned char   hwNtpCnt;
    unsigned char   hwFanCnt;                       /*** STANDBY INFORMATION : MORE INFO ***/
    unsigned char   hwPwrCnt;                       /*** STANDBY INFORMATION : MORE INFO ***/
    unsigned char   hwDiskArrayCnt;
    unsigned char   NTAFChnlCnt;
    unsigned char   dReserved4[3];

    st_CurrVal      cpusts;                         /*** STANDBY INFORMATION ***/
    st_CurrVal      memsts;                         /*** STANDBY INFORMATION ***/
    st_CurrVal      queuests;
	st_CurrVal      nifosts;
    st_CurrVal      disksts[MAX_DISK_COUNT];         /*  4   */
    st_ProcInfo     mpswinfo[MAX_SW_COUNT];         /*  32  */

    long long       tEventUpTime[256];
                                                /*
                                                // 0 : cpu
                                                // 1 : mem
                                                // 2 : queue
                                                // 3 : nifo
                                                // 4 : DBAlive
                                                // 5 - 8    : disk - 4
                                                // 9 - 16   : link - 8
                                                // 17 - 48  : sess - 32
                                                // 49       : no available IPC ch
                                                // 50 - 56 : Reserved - 7
                                                // 57 - 64: NTAF channel - 8
                                                // 65 - 68: Interlock channel - 4
                                                // 69 - 72: Reserved - 4
                                                // 73 - 76: HW power - 4
                                                // 77 - 78: HW NTP - 2
                                                // 79 - 86: HW fan - 8
                                                // 87 - 214: HW disk array - 128
                                                */
} st_NTAM, *pst_NTAM;

typedef struct _st_WNTAM
{
    st_NTAM         stNTAM;
    st_DIRECT_MNG   stDirectTOT;
    st_SWITCH_MNG   stSwitchTOT;
} st_WNTAM, *pst_WNTAM;

#define DEF_NTAM_SIZE		sizeof(st_NTAM)
#define DEF_DIRECT_MNG_SIZE	sizeof(st_DIRECT_MNG)
#define DEF_SWITCH_MNG_SIZE	sizeof(st_SWITCH_MNG)

#define DEF_WNTAM_SIZE	(DEF_NTAM_SIZE+DEF_DIRECT_MNG_SIZE+DEF_SWITCH_MNG_SIZE)

/*** MASK INFO */
typedef struct _st_direct_mask {
	unsigned char ucMonitorPort[MAX_MONITOR_PORT_COUNT];
	unsigned char ucMirrorPort[MAX_MIRROR_PORT_COUNT];
	unsigned char ucPower[MAX_DIRECT_POWER_COUNT];
} st_DirectMask, *pst_DirectMask;

#define DEF_DIRECT_MASK_SIZE sizeof(st_DirectMask)

typedef struct _st_switch_mask {
	unsigned char ucPort[MAX_SWITCH_PORT_COUNT];
	unsigned char ucCPU[MAX_SWITCH_CPU_COUNT];
	unsigned char ucMEM;
} st_SwitchMask, *pst_SwitchMask;

#define DEF_SWITCH_MASK_SIZE sizeof(st_SwitchMask)

typedef struct _st_chnl_mask {
	unsigned char ucSubChannel[MAX_CH_COUNT];
	unsigned char ucInterlock[MAX_ICH_COUNT];
	unsigned char ucDirector[MAX_DIRECT_COUNT];
	unsigned char ucSwitch[MAX_SWITCH_COUNT];
	st_DirectMask stDirector[MAX_DIRECT_COUNT];
	st_SwitchMask stSwitch[MAX_SWITCH_COUNT];
} st_ChnlMask, *pst_ChnlMask;

#define DEF_CHNL_MASK_SIZE sizeof(st_ChnlMask)

/********** 언젠가는 fidb library 로 이 부분을 추출해야 할 것이다.... */


/*	TAF_RPPI definition		*/
#define MAX_NTAF_DISK_COUNT		4
#define MAX_NTAF_SW_BLOCK		80 /* 64 -> 80 defined by uamyd 20110420 */
#define MAX_NTAF_LINK			4
#define	MAX_NTAF_RSRC_NUM		16
#define MAX_ENDACE_COUNT		2
#define MAX_NTAF_NUM 8

/*	TAF_RPPI FIDB structures	*/
typedef struct _st_NTAF
{
	long long		tUpTime;

	unsigned char	cpu;
	unsigned char	mem;
	unsigned char	queue;
	unsigned char	nifo;
	unsigned char	disk[MAX_NTAF_DISK_COUNT];		/*	4	*/
	unsigned char	sess;
	unsigned char	ucStatFrames;					/*	0x02: MASK, 0x03: NORMAL, 0x06: CRITICAL		*/
	unsigned char	ucStatBytes;					/*	0x02: MASK, 0x03: NORMAL, 0x06: CRITICAL		*/
	unsigned char	ucReserved[7];
	unsigned char	mirrorsts[MAX_ENDACE_COUNT];	/*	2 - endace card mirrorring status [0x03:NORMAL,0x06:CRITICAL]	*/
	unsigned char	mirrorActsts[MAX_ENDACE_COUNT];	/*	2 - endace card active status[0x01: ACTIVE, 0x00, DEACTIVE]		*/

	unsigned char	link[MAX_NTAF_LINK];			/*	4	*/
	unsigned char	mpsw[MAX_NTAF_SW_BLOCK];				/*	80	*/

	unsigned char	hwfan[6];
	unsigned char	hwntp[2];
	unsigned char	hwpwr[2];
	unsigned char	hwdisk[2];
	unsigned char	hwport[8];

	unsigned char	hwpwrcnt;
	unsigned char	hwdiskcnt;
	unsigned char	hwfancnt;
	unsigned char	hwntpcnt;
	unsigned char	hwportcnt;
	unsigned char	reserved3[5];

	st_CurrVal		cpusts;
	st_CurrVal		memsts;
	st_CurrVal		quests;
	st_CurrVal		disksts[MAX_NTAF_DISK_COUNT];	/*	4	*/
	st_CurrVal		sesssts;
	st_CurrVal		nifosts;
	st_CurrVal		framests;
	st_CurrVal		bytests;
	st_ProcInfo		mpswinfo[MAX_NTAF_SW_BLOCK];			/*	80	*/

	int         	rsrcload[MAX_NTAF_RSRC_NUM];	/*	16	*/
	            									/*	0:MMDB_SESS, 1:MMDB_OBJ, 2:MMDB_CDR...	*/

	long long		tEventUpTime[64];	/*
								//  0: cpu load
                                //  1: mem load
                                //  2: queue load
                                //  3: nifo load
                                //  4 - 7: disk load
                                //  8: sess load
                                //  9 - 10: mirror(NORMAL, CRITICAL) - 2
                                //  11 - 12: mirror(ACTIVE, DEACTIVE) - 2
                                //  13 - 15: reserved - 3
                                //  16 - 19: link - 4
                                //  20 - 25: fan - 6
                                //  26 - 27: ntp - 2
                                //  28 - 29: pwr - 2
                                //  30 - 31: disk - 2
                                //  32 - 39: DAG PORT - 8
                                //  40: Bytes load
                                //	41: Frames load
                                */
} st_NTAF, *pst_NTAF;
#define DEF_TAF_SIZE		sizeof(st_NTAF)

#define MAX_NTAF_COUNT 8
typedef struct _st_NTAF_List_SHM
{
	    st_NTAF stNTAF[MAX_NTAF_COUNT];
} st_NTAF_List_SHM, *pst_NTAF_List_SHM;
#define DEF_TAF_LIST_SIZE sizeof(st_NTAF_List_SHM)

// 신규 FIDB 구조체
#if defined(_NEW_FIDB_)

typedef struct _st_fidb
{
#define MAX_DISK_COUNT 8
#define MAX_PWR_COUNT  4
#define MAX_FAN_COUNT  8
#define MAX_CH_COUNT   8
#define MAX_LINK_COUNT 8
#define MAX_NTP_COUNT  2
#define MAX_ICH_COUNT  4
#define MAX_PORT_COUNT 8
    long long       tUpTime;						/* update time */
    int             active;

    unsigned char   pwrsts[MAX_PWR_COUNT];          /* POWER STATUS */
	unsigned char   fansts[MAX_FAN_COUNT];			/* FAN STATUS */
	unsigned char   chsts[MAX_CH_COUNT];			/* CHANNEL STATUS */
	unsigned char   linksts[MAX_LINK_COUNT];		/* LINK STATUS */
	unsigned char   ntpsts[MAX_NTP_COUNT];		    /* NTP STATUS */
	unsigned char   portsts[MAX_PORT_COUNT];		/* PORT STATUS */
	unsigned char   dbsts;							/* DBMS STATUS - ALIVE/DEAD STATUS */
	/* interlock channel 은 꼭 별도로 분기할 것 !!!!! */
	/* interlock channel 은 꼭 별도로 분기할 것 !!!!! */
	/* interlock channel 은 꼭 별도로 분기할 것 !!!!! */
	/* count 변수를 두고, general 하게 작성하는 게 좋을까??? 고민됨 */
	/* count 변수를 두고, general 하게 작성하는 게 좋을까??? 고민됨 */
	/* count 변수를 두고, general 하게 작성하는 게 좋을까??? 고민됨 */
	unsigned char   cInterlock[MAX_ICH_COUNT];		/* INTERLOCK CHANNEL STATUS */
	unsigned char   mirrorsts[MAX_PORT_COUNT];
	unsigned char   mirroractsts[MAX_PORT_COUNT];
	unsigned char   swsts[MAX_SW_COUNT];			/* SOFTWARE STATUS */

    unsigned char   pwrcnt;                         /* POWER COUNT */
    unsigned char   fancnt;                         /* FAN COUNT */
    unsigned char   ntpcnt;                         /* NTP COUNT */
	unsigned char   portcnt;						/* PORT COUNT */
    unsigned char   diskcnt;                        /* DISK COUNT */

	st_CurrVal      cpusts;							/* CPU LOAD */
	st_CurrVal      memsts;							/* MEM LOAD */
	st_CurrVal		quests;							/* QUEUE LOAD */
	st_CurrVal		nifosts;						/* NIFO LOAD */
    st_CurrVal      disksts[MAX_DISK_COUNT];        /* DISK LOAD */
	st_CurrVal		bytests;						/* TRAFFIC */
	st_ProcInfo     swinfo[MAX_SW_COUNT];           /* SOFTWARE INFORMATION  */

#if 0
    unsigned short  usSysChg[8];                    /*  0-NAS, 1-AAA, 2-DSCP, 3-IPAF, 4-IDR */

    unsigned char   cpu;                            /*** STANDBY INFORMATION ***/
    unsigned char   mem;
    unsigned char   queue;
    unsigned char   nifo;
		unsigned char   cDBAlive;                       /*  0x03: NORMAL, 0x06: CRITICAL    */
    unsigned char   disk[MAX_NTAM_DISK];            /*  4   */

		unsigned char   link[MAX_NTAM_LINK];            /*  8   */
    unsigned char   sess[MAX_SW_COUNT];             /*  32, each process session memory load alarm status */
		unsigned char   mpsw[MAX_SW_COUNT];             /*  32  */

    unsigned char   IpcChUnAvail;                   /*  no available IPC channel */
    unsigned char   ucReserved2[6];

		unsigned char   NTAFChnl[MAX_NTAF_CHNL];        /*  8 - NTAFChannel [0x03: NORMAL, 0x06: CRITICAL]  */
		unsigned char   cInterlock[MAX_INTERLOCK_CHNL]; /*  4 - SI_DB: SI_DB_INTERLOCK, SI_SVCMON: SI_SVCMON_INTERLOCK, SI_NMS: SI_NMS_INTERLOCK [0x03: NORMAL, 0x06: CRITICAL] */

    unsigned char   dReserved3[2];
		unsigned char   hwPWR[MAX_HW_PWR];              /*  4, STANDBY INFORMATION ***/
		unsigned char   hwNTP[MAX_HW_NTP];              /*  2 */

		unsigned char   hwFan[MAX_HW_FAN];              /*** 8, STANDBY INFORMATION ***/
    unsigned char   hwDiskArray[MAX_DISK_ARRAY];    /*  128 */

		unsigned char   hwNtpCnt;
		unsigned char   hwFanCnt;                       /*** STANDBY INFORMATION : MORE INFO ***/
		unsigned char   hwPwrCnt;                       /*** STANDBY INFORMATION : MORE INFO ***/
		unsigned char   hwDiskArrayCnt;
		unsigned char   NTAFChnlCnt;
    unsigned char   dReserved4[3];

		st_CurrVal      cpusts;                         /*** STANDBY INFORMATION ***/
		st_CurrVal      memsts;                         /*** STANDBY INFORMATION ***/
		st_CurrVal      queuests;
		st_CurrVal      nifosts;
		st_CurrVal      disksts[MAX_NTAM_DISK];         /*  4   */
		st_ProcInfo     mpswinfo[MAX_SW_COUNT];         /*  32  */

#endif
    long long       tEventUpTime[256];
                                                /*
                                                // 0 : cpu
                                                // 1 : mem
                                                // 2 : queue
                                                // 3 : nifo
                                                // 4 : DBAlive
                                                // 5 - 8    : disk - 4
                                                // 9 - 16   : link - 8
                                                // 17 - 48  : sess - 32
                                                // 49       : no available IPC ch
                                                // 50 - 56 : Reserved - 7
                                                // 57 - 64: NTAF channel - 8
                                                // 65 - 68: Interlock channel - 4
                                                // 69 - 72: Reserved - 4
                                                // 73 - 76: HW power - 4
                                                // 77 - 78: HW NTP - 2
                                                // 79 - 86: HW fan - 8
                                                // 87 - 214: HW disk array - 128
                                                */
} st_fidb, *pst_fidb;

#define DEF_FIDB_SIZE	sizeof(st_fidb)
#endif /* defined(_NEW_FIEB_) */

/*
*	KEEPALIVE
*/
typedef struct _st_load_val
{
    unsigned short  usMinor;
    unsigned short  usMajor;
    unsigned short  usCritical;
    unsigned short  usReserved;
} st_load_val, *pst_load_val;

typedef struct _st_load_hw
{
	st_load_val		cpu;
	st_load_val		mem;
	st_load_val		disk;
	st_load_val		que;
	st_load_val		nifo;
} st_load_hw, *pst_load_hw;

typedef struct _st_keepalive
{
#define MAX_SUB_COUNT 8
    long        oldcnt[MAX_SW_COUNT];
    long        cnt[MAX_SW_COUNT];
    int         slee_state;
    int         peer_slee_state;
    int         slee_load_state;
    time_t      update_time;
    int         reserved2;
    int         reserved3;
	st_load_hw	stTAFLoad[MAX_SUB_COUNT];
	st_load_hw  stTAMLoad;
	st_load_hw	stSWCHLoad;
	
} st_keepalive, *pst_keepalive;

#define DEF_KEEPALIVE_SIZE	sizeof(st_keepalive)

typedef struct {
    long   minor;
    long   major;
    long   critical;
    long   reserved;
} st_load_val_l;

typedef struct {
    //long    oldcnt[MAX_SW_COUNT];
    //long    cnt[MAX_SW_COUNT];
    long    oldcnt[MAX_NTAF_SW_BLOCK];
    long    cnt[MAX_NTAF_SW_BLOCK];
    short   aply_status;
    short   dtaf_status;
    short   Reserved1;
    short   Reserved2;
    time_t  inittime;
    time_t  resvtime;
    st_load_val_l     cpu;
    st_load_val_l     mem;
    st_load_val_l     queue;
    st_load_val_l     nifo;
    st_load_val_l     disk;
    st_load_val_l     stBytes;
    st_load_val_l     stFrames;
    char              procnorm[MAX_NTAF_SW_BLOCK];
} st_keepalive_taf, *pst_keepalive_taf;

#define DEF_KEEPALIVE_TAF_SIZE sizeof(st_keepalive_taf)

#endif /* __FILEDB_H__ */
