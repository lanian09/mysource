#ifndef __MCDM_PROTO_H__
#define __MCDM_PROTO_H__

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <time.h>
#include <pthread.h>

#include <commlib.h>
#include <sysconf.h>
#include <comm_msgtypes.h>
#include <sfm_msgtypes.h>
#include <omp_filepath.h>
#include <sfmconf.h>
#include <proc_version.h>

#define MP_NAME_STR		"MPNAME"

typedef struct McdmResBufContext_s McdmResBufContext;
struct McdmResBufContext_s {
	IxpcQMsgType	rxIxpcMsg; // MP로부터 수신한 메시지 원본을 그대로 저장한다.
	McdmResBufContext	*prev;
	McdmResBufContext	*next;
};

#define MCDM_NUM_TP_JOB_TBL	256
//jean #define MCDM_WAIT_RESPONSE_TIMER	26
#define MCDM_WAIT_RESPONSE_TIMER	35	

typedef struct {
	unsigned char	tpInd; // tuple_indicator; 사용중 표시; empty(0), busy(1)
	unsigned char	reqSysCnt; // 몇개의 MP로 보냈는지
	unsigned char	resFlag[SYSCONF_MAX_ASSO_SYS_NUM];// 마지막 응답을 어디어디서 받았는지
	time_t		deadlineTime;  // 응답 대기 만료시각
	char				dstSysName[SYSCONF_MAX_ASSO_SYS_NUM][COMM_MAX_NAME_LEN]; // request를 보낸 시스템들의 이름을 저장
	McdmResBufContext	*resBuf[SYSCONF_MAX_ASSO_SYS_NUM]; // 수신한 응답 메시지를 쌓아두는 buffer
	unsigned short  mmcdJobNo; // mmcd로부터 수신한 값 저장
	char		cmdName[MML_MAX_CMD_NAME_LEN]; // mmcd로부터 수신한 값 저장
	char		srcSysName[COMM_MAX_NAME_LEN]; // mmcd 정보
	char		srcAppName[COMM_MAX_NAME_LEN]; // mmcd 정보
} McdmJobTblContext;


// 자신이 직접 처리해야 하는 MMC 명령어 리스트와 처리 function을 등록하는 table
//
#define MCDM_MAX_OWN_MMC_HANDLER	16
typedef struct {
    char    cmdName[MML_MAX_CMD_NAME_LEN];
    int     (*func)(IxpcQMsgType*);
} McdmOwnMmcHdlrVector;


// 다른 놈으로 분배햐야 하는 MMC 명령어 정보를 등록하는 table
//
#define MCDM_MAX_DISTRIB_MMC	256
typedef struct {
	char    cmdName[MML_MAX_CMD_NAME_LEN];
	char	dstAppName[COMM_MAX_NAME_LEN]; // 어떤 프로세스로 보내야 하는지
	char	dstSysName[SYSCONF_MAX_ASSO_SYS_NUM][COMM_MAX_NAME_LEN]; // 어디 어디로 보내야 하는지
	char	sysCnt; 		// dstSysName의 갯수
	char	type;			// 명령어 type 
#	define	DIST_BOTH			1		/* 양쪽으로 무조건 보낸다. */
#	define	DIST_SINGLE			2		/* 한쪽으로전송가능한 보낸다. */
#	define	DIST_SIN_BYPASS		3		/* 한쪽으로전송가능한 보낸 bypass해서 */
#	define	DIST_BYPASS			4		/* 양쪽으로 bypass */
#	define	DIST_ACTIVE			5		/* 양쪽으로 bypass */
#	define	DIST_ACTIVE_OMP		6		/* ACTIVE쪽으로 던지되 OMP에도 함께 던진다.*/
} McdmDistribMmcTblContext;

#	define	STR_BOTH			"BOTH"
#	define	STR_SINGLE			"SINGLE"
#	define	STR_ACTIVE			"ACTIVE"
#	define	STR_SIN_BYPASS		"S-BYPASS"
#	define	STR_BYPASS			"BYPASS"
#	define	STR_ACTIVE_OMP		"ACTIVE_OMP"

typedef struct {
	char	str[16];
	short	type;
} McdmMmcType;

#ifdef __MCDM_TYPE__
const	McdmMmcType	mcdmTYPE[] =
	{
		{STR_BOTH,			DIST_BOTH		},
		{STR_SINGLE,		DIST_SINGLE		},
		{STR_SIN_BYPASS,	DIST_SIN_BYPASS		},
		{STR_BYPASS,		DIST_BYPASS		},
		{STR_ACTIVE,		DIST_ACTIVE		},
		{STR_ACTIVE_OMP,	DIST_ACTIVE_OMP		}
	};
#endif


extern int errno;

extern int mcdm_exeRxQMsg (GeneralQMsgType*);
extern int mcdm_rxMMCReqMsg (GeneralQMsgType*);

extern int mcdm_initial (void);
extern int mcdm_initLog (void);

extern int mcdm_rxDistribMmcReq (GeneralQMsgType*);
extern int mcdm_saveDistrMmcInfo2JobTbl (int, IxpcQMsgType*, McdmDistribMmcTblContext*, int );

extern int mcdm_rxDistribMmcRes (GeneralQMsgType*);
extern int mcdm_sendDistribMmcRes2MMCD (int);

extern int mcdm_allocJobTbl (void);
extern void mcdm_deallocJobTbl (int);
extern void mcdm_checkJobTbl (void);

extern int mcdm_ownMmcHdlrVector_qsortCmp (const void *, const void *);
extern int mcdm_ownMmcHdlrVector_bsrchCmp (const void *, const void *);
extern int mcdm_distrMmcTbl_qsortCmp (const void *, const void *);
extern int mcdm_distrMmcTbl_bsrchCmp (const void *, const void *);
extern int mcdm_sendDistribMmcRes2MMCD2 (int, char, char, unsigned short, char *);


#endif //__MCDM_PROTO_H__
