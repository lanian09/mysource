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
	IxpcQMsgType	rxIxpcMsg; // MP�κ��� ������ �޽��� ������ �״�� �����Ѵ�.
	McdmResBufContext	*prev;
	McdmResBufContext	*next;
};

#define MCDM_NUM_TP_JOB_TBL	256
//jean #define MCDM_WAIT_RESPONSE_TIMER	26
#define MCDM_WAIT_RESPONSE_TIMER	35	

typedef struct {
	unsigned char	tpInd; // tuple_indicator; ����� ǥ��; empty(0), busy(1)
	unsigned char	reqSysCnt; // ��� MP�� ���´���
	unsigned char	resFlag[SYSCONF_MAX_ASSO_SYS_NUM];// ������ ������ ����� �޾Ҵ���
	time_t		deadlineTime;  // ���� ��� ����ð�
	char				dstSysName[SYSCONF_MAX_ASSO_SYS_NUM][COMM_MAX_NAME_LEN]; // request�� ���� �ý��۵��� �̸��� ����
	McdmResBufContext	*resBuf[SYSCONF_MAX_ASSO_SYS_NUM]; // ������ ���� �޽����� �׾Ƶδ� buffer
	unsigned short  mmcdJobNo; // mmcd�κ��� ������ �� ����
	char		cmdName[MML_MAX_CMD_NAME_LEN]; // mmcd�κ��� ������ �� ����
	char		srcSysName[COMM_MAX_NAME_LEN]; // mmcd ����
	char		srcAppName[COMM_MAX_NAME_LEN]; // mmcd ����
} McdmJobTblContext;


// �ڽ��� ���� ó���ؾ� �ϴ� MMC ��ɾ� ����Ʈ�� ó�� function�� ����ϴ� table
//
#define MCDM_MAX_OWN_MMC_HANDLER	16
typedef struct {
    char    cmdName[MML_MAX_CMD_NAME_LEN];
    int     (*func)(IxpcQMsgType*);
} McdmOwnMmcHdlrVector;


// �ٸ� ������ �й���� �ϴ� MMC ��ɾ� ������ ����ϴ� table
//
#define MCDM_MAX_DISTRIB_MMC	256
typedef struct {
	char    cmdName[MML_MAX_CMD_NAME_LEN];
	char	dstAppName[COMM_MAX_NAME_LEN]; // � ���μ����� ������ �ϴ���
	char	dstSysName[SYSCONF_MAX_ASSO_SYS_NUM][COMM_MAX_NAME_LEN]; // ��� ���� ������ �ϴ���
	char	sysCnt; 		// dstSysName�� ����
	char	type;			// ��ɾ� type 
#	define	DIST_BOTH			1		/* �������� ������ ������. */
#	define	DIST_SINGLE			2		/* �����������۰����� ������. */
#	define	DIST_SIN_BYPASS		3		/* �����������۰����� ���� bypass�ؼ� */
#	define	DIST_BYPASS			4		/* �������� bypass */
#	define	DIST_ACTIVE			5		/* �������� bypass */
#	define	DIST_ACTIVE_OMP		6		/* ACTIVE������ ������ OMP���� �Բ� ������.*/
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
