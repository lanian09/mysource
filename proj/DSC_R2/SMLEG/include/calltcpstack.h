#ifndef __MMDBLIST2_TABLE_DEFINE___
#define __MMDBLIST2_TABLE_DEFINE___

#pragma pack(1)

/**A.1*  File Inclusion ***********************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/types.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <ipaf_define.h>
#include <ipaf_svc.h>
#include <ipaf_shm.h>

/**B.1*  Definition of New Constants *********************/
#define DEF_STACK_REQ			2
#define DEF_STACK_RES			1

/**** LINKED LIST TCP STACK DEFINITION ****/
#ifdef SMALL_SESSION
#define MAX_STACK_LIST            20002
#else
#define MAX_STACK_LIST    		200002 				/* MAX TCP STACK COUNT */
#endif
#define MAX_STACKREAL_LIST    	MAX_STACK_LIST-2 	/* MAX REAL TCP STACK COUNT */

/**** LINKED LIST TCP SESSION DEFINITION ****/
#ifdef SMALL_SESSION
#define MAX_TCP_LIST          20002
#else
#define MAX_TCP_LIST    		200002 				/* MAX TCP SESSION COUNT */
#endif
#define MAX_TCPREAL_LIST    	MAX_TCP_LIST-2 		/* MAX REAL TCP SESSION COUNT */

/********* MMDB SESSION DEFINITION *********/
#ifdef SMALL_SESSION
#define MAX_CALL_RECORD           15003
#else
#define MAX_CALL_RECORD			150003				/* MAX MMDB CALL SESSION RECORD COUNT */
#endif
#define MAX_CALLREAL_RECORD		MAX_CALL_RECORD-1	/* MAX REAL CALL MMDB SESSION COUNT */

#define SND_CDR			1
#define SND_SVC			0
/**B.2*  Definition of New Type  **************************/

/********* MMDB SESSION DEFINITION *********/
/* MMDB CALL SESSION KEY */
typedef struct _st_CallKey {				
	UINT		uiSrcIP;			/* Call Session User IP Addr */
	UINT		uiReserved;			/* Reserved */
} st_CallKey, *pst_CallKey;

#define CALL_KEY             st_CallKey
#define PCALL_KEY            pst_CallKey

/* TABLE Dispatcher's Record Details ***/
typedef struct _st_CallSess 
{
    /* Management Parameters */
	CALL_KEY		stKey;			/* My Key */
	st_TimeVal		stSessTime;		/* Start Session Time */
	UINT			uiTCPCount;		/* TCP Session List Count */
	UINT			uiTCPFirst;		/* TCP Session First Node Index */
	UINT			uiTCPLast;		/* TCP Session Last Node Index */
	
	/* User Information */
	time_t			tLastUpdateTime;		/* 초기값 생성시간, 이후 패킷받은 시간 */			
	INT64			llAcctSessID;			/* Account Session ID */
	INT64   		llCorrelID;
	char        	szMin[MAX_MIN_SIZE];	/* MIN */
	UCHAR           ucPayType;          	/* User type 1: POST_PAID, 2: PRE_PAID 4:NO_REMAIN*/
	UCHAR			ucUDPFlag;
	char			ucResRcv;
	UCHAR			ucID;
	char			szReserved[3];
	UINT			uiLastRDR[4];			/* 0 :  RDR_SEQ, 1 : UDP_QID */

	/* LDH 2006.08.25 : SERVICE OPTION */
    int             dSvcOpt;
//    int             dReserved;
	UINT			uiC23BIT;

	UINT			uiNASIP;
	UINT			uiReserved;

	char			szMDN[MAX_MIN_SIZE-1];	/* 2006-07-15 ADD : MDN*/

	st_TCP_DATA		stTCPDATA; /* SESSION PACKET BYTE */

	/* NEW ADDED 2006.0424 */
//	st_AccReq		stAccReq;
} st_CallSess, *pst_CallSess;

#define CALL_DATA            st_CallSess
#define PCALL_DATA           pst_CallSess

// 최대 record의 길이, 8byte 기준
#define MAX_CALL_KEY_LEN     ( sizeof( CALL_KEY ) / 8 )
#define MAX_CALL_BODY_LEN    ( ( sizeof( CALL_DATA ) / 8 ) - MAX_CALL_KEY_LEN )

typedef struct _st_CALLTYPE {
    CALL_KEY	key;    		/* Primary Key */
    INT64   	body[MAX_CALL_BODY_LEN];
    INT         left;   		/* Index left */
    INT         right;  		/* Index right */
    SHORT       bf;     		/* balance factor*/
    SHORT       reserved[3];
} CALL_TYPE, *PCALL_TYPE;

// Typical format of Database
typedef struct _st_MMDB_CALL_TABLE {
    CALL_TYPE	tuple[MAX_CALL_RECORD];
    INT			free;   		/* index free */
    INT			root;   		/* index root */
	UINT		uiCallCount;	/* Current Used Call Count */
	CHAR		szReserved[128];
} CALL_TABLE, *pCALL_TABLE;

#define CALL_TABLE_SIZE		sizeof(CALL_TABLE)
#define CALL_TYPE_SIZE		sizeof(CALL_TYPE)
#define CALL_KEY_SIZE		sizeof(CALL_KEY)

/**** LINKED LIST TCP SESSION DEFINITION ****/
typedef struct _st_TCP_KEY
{
	UINT		uiSIP;				/* Source IP */
	UINT		uiDIP;				/* Destination IP */
	UCHAR		ucProType;			/* Protocal Type : 6->TCP 17->UDP */
	UCHAR		szReserved[3];		/* Reserved */
	USHORT		usSPort;			/* Source Port */
	USHORT		usDPort;			/* Destination Port */
} TCP_KEY, *pTCP_KEY;

#define TCP_KEY_SIZE	sizeof(TCP_KEY)


typedef struct _st_TCP_LIST
{
    /* Management Parameters */
    CALL_KEY	stPKey;    		/* Parents Session Primary Key */
	TCP_KEY		stTKey;			/* My Key */	
    st_TimeVal  stTCPTime;		/* Add Time */
    UINT		next;			/* LINKED_LIST Next Node Index */
    UINT		prev;			/* LINKED_LIST Previous Node Index */
	UINT		uiTCPNext;		/* Only TCP Previous Node Index */
	UINT		uiTCPPrev;		/* Only TCP Next Node Index */
	UINT		uiIndex;		/* My Index */
	UCHAR		ucStatus;		/* STATUS : ADD BY HWH : 0->INIT, 1->FIN START, 2->FIN  */ 
	UCHAR		ucSvcCatInfo;	/* 0 : SEND IPAM, 1 : LOG SAVE */
	CHAR		szReserved[2];	/* Reserved */

	UINT		uiReqCount;		/* Request Stack Count */
	UINT		uiReqFirst;		/* Request Stack First Node */
	UINT		uiReqLast;		/* Request Statck Last Node */

	UINT		uiResCount;		/* Response Stack Count */
	UINT		uiResFirst;		/* Response Stack First Node */
	UINT		uiResLast;		/* Response Statck Last Node */

	/* User Information */
	UINT		uiSndMsgQ;		/* 보내야 할 서비스블럭의 메시지큐아이디 */
	UINT		uiFINSetTime;
	UINT		uiFINChkAck;	/* FIN PACKET에 대한 ACK PACKET 번호 */
	UINT		ucRDRSndType;	

	UINT		uiLastReqSeq;	/* 마지막에 처리한 SEQ 번호 : REQ */
	UINT		uiLastResSeq;	/* 마지막에 처리한 SEQ 번호 : RES */

	UINT		uiLastReqLen;	/* 마지막에 처리한 데이타 길이: REQ */
	UINT		uiLastResLen;	/* 마지막에 처리한 데이타 길이: RES */
/*
	UINT		uiUpLastID[2];
	UINT		uiDownLastID[2];

	st_DupList	stUpIDList;
	st_DupList	stDownIDList;
*/

} TCP_LIST, *pTCP_LIST;

typedef struct _st_TCP_TABLE {
    UINT			uiFreeList;		/* Free Node List */
    UINT			uiUsedFirst;	/* Used Fist Node Index */
    UINT			uiUsedLast;		/* Used Last Node Index */
    UINT    		uiCurrCount;	/* Current Used Node Count */
	CHAR			szReserved[128];
    TCP_LIST    	stNode[MAX_TCP_LIST];
} TCP_TBL, *pTCP_TBL;

#define TCP_LIST_SIZE	sizeof(TCP_LIST)
#define TCP_TABLE_SIZE	sizeof(TCP_TBL)

/**** LINKED LIST TCP STACK SESSION DEFINITION ****/
typedef struct _st_STACK_KEY
{
	UINT		uiSeqNum;			/* Sequnce Number */
	UINT		uiAckNum;			/* Ack Number */
} STACK_KEY, *pSTACK_KEY;

#define STACK_KEY_SIZE	sizeof(STACK_KEY)

#define MAX_TCPDATA_SIZE		2048
typedef struct _st_STACK_LIST
{
    /* Management Parameters */
	TCP_KEY		stTKey;			/* TCP Parents Key */	
	STACK_KEY	stSKey;			/* Stack Key : My Key */
    st_TimeVal  stStackTime;	/* Add Time */
    UINT		next;			/* Next Node Index */
    UINT		prev;			/* Previous Node Index */
	UINT		uiStackNext;	/* Only Stack Previous Node Index */
	UINT		uiStackPrev;	/* Only Stack Next Node Index */
	UINT		uiIndex;		/* My Index */
	CHAR		szReserved[2];	/* Reserved */

	/* User Information */
	UCHAR		ucRetransFlag;	/* 0 : normal, 1 : retrans */
	UCHAR		ucFlag;			/* 0 : wait, 1 : check-succ(보내야함) */
	UINT		dTCPDataLen;
	UINT		dDataLen;
	UINT 		uiOffset;		/* Start Offset of MIF Node Data */
	USHORT 		usRetransCnt;
	USHORT 		usRetransLen;
	UCHAR		szData[MAX_TCPDATA_SIZE]; /* MIF NODE OFFSET */

} STACK_LIST, *pSTACK_LIST;

typedef struct _st_STACK_TABLE {
    UINT			uiFreeList;		/* Free Node List */
    UINT			uiUsedFirst;	/* Used Fist Node Index */
    UINT			uiUsedLast;		/* Used Last Node Index */
    UINT    		uiCurrCount;	/* Current Used Node Count */
	CHAR			szReserved[128];
    STACK_LIST    	stNode[MAX_STACK_LIST];
} STACK_TBL, *pSTACK_TBL;

#define STACK_LIST_SIZE		sizeof(STACK_LIST)
#define STACK_TABLE_SIZE	sizeof(STACK_TBL)

typedef struct _st_CALLTCPSTACK_TABLE {
	STACK_TBL	stStackTbl;
	TCP_TBL		stTCPTbl;
	CALL_TABLE	stCallTbl;
} CALLTCPSTACK_TABLE, *pCALLTCPSTACK_TABLE;

#define CALLTCPSTACK_TABLE_SIZE		sizeof(CALLTCPSTACK_TABLE)

/**C.1*  Declaration of Variables  ************************/
extern int             		gdCALLSHMID;    /* SHM ID */
extern pCALLTCPSTACK_TABLE 	pstCALLTCPSTACKTbl; /* MMDB&LIST TABLE POINTER */
extern pCALL_TABLE     		pstCallTbl;    	/* CALL SESSION MMDB POINTER */
extern pTCP_TBL    			pstTCPTbl;		/* TCP SESSION LIST TABLE POINTER */
extern pSTACK_TBL    		pstStackTbl;	/* STACK LIST TABLE POINTER */

/**D.1*  Definition of Functions  *************************/

/**** LINKED LIST STACK INTERNAL FUNCTION DEFINITION ****/
pSTACK_LIST pGetStackFirstNode();
void InitStackList();
void FreeStackNode(pSTACK_LIST node);
pSTACK_LIST pGetStackNode(UINT *puiIndex);
pSTACK_LIST pSetStackNode(UINT uiIndex);

/**** LINKED LIST TCP SESSION INTERNAL FUNCTION DEFINITION ****/
pTCP_LIST pGetTCPFirstNode();
void InitTCPList();
void FreeTCPNode(pTCP_LIST node);
pTCP_LIST pGetTCPNode(UINT *puiIndex);
pTCP_LIST pSetTCPNode(UINT uiIndex);
void FreeNode(pTCP_LIST node);

/********* MMDB CALL SESSION INTERNAL FUNCATION DEFINITION *********/
int avl_insert_sess( PCALL_KEY key, long long *body, int *root);
int left_rotation_sess( int index, int *pindex );
CALL_TYPE *avl_search_sess( int root, PCALL_KEY key );
int avl_delete_sess( int *root, PCALL_KEY key );
CALL_TYPE *avl_select_sess( int root, PCALL_KEY first_key, PCALL_KEY last_key );
int avl_update_sess( CALL_TYPE *tree, long long *body );
CALL_TYPE *get_sess( int index);

void Init_SessDB();
int sess_alloc();
void sess_dealloc( int index );
int Insert_SESS(PCALL_DATA pstSrc);			
PCALL_DATA Search_SESS(PCALL_KEY stKey);
int Delete_SESS(PCALL_KEY pstKey);
int Update_SESS(PCALL_DATA disp, PCALL_DATA input);
PCALL_DATA pstSelectMMDB(PCALL_KEY pstFKey, PCALL_KEY pstLKey);

/********* Initialize INTERNAL MMDB & LINKED LIST *********/
int dMmapGet(char *path);
int dShmGet(int dSHMKey);

/********* USER FUNCTION DEFINITION *********/
/* Initialize Function */
int dInitMMAPMMDB(char *szMMAPFile); 		/* Initializing Function Used For MMAP */
int dInitSHMMMAB(int dMMDBSHMKey);			/* Initializing Function Used For SHM */

/* TCP Stack LINKED LIST User Function */
int dGetStackOnly(pSTACK_LIST *pstStack);
void FreeStackOnly(pSTACK_LIST pstStack);
int dGetStack(pTCP_LIST pstTCP, UCHAR ucType, pSTACK_KEY pstSKey, pSTACK_LIST *pstStack);
int dAddStack(pTCP_LIST pstTCP, UCHAR ucType, pSTACK_LIST pstAddStack);
int dAddStackNext(pTCP_LIST pstTCP, UCHAR ucType, pSTACK_LIST pstStack, pSTACK_LIST pstAddStack);
int dAddStackPrev(pTCP_LIST pstTCP, UCHAR ucType, pSTACK_LIST pstStack, pSTACK_LIST pstAddStack);
int dSetStackNext(UINT uiIndex, pSTACK_LIST *pstNextStack); 
int dSetStackPrev(UINT uiIndex, pSTACK_LIST *pstPrevStack); 
int dSetStackForward(pTCP_LIST pstTCP, UCHAR ucType,  pSTACK_KEY pstSKey, pSTACK_LIST *pstStack);
int dSetStackBackword(pTCP_LIST pstTCP, UCHAR ucType,  pSTACK_KEY pstSKey, pSTACK_LIST *pstStack);
int dSetStackIndex(pTCP_LIST pstTCP, UINT uiIndex, pSTACK_LIST *pstStack);
int dDelStack(pTCP_LIST pstTCP, UCHAR ucType, pSTACK_LIST pstStack);

/* TCP Session LINKED LIST User Function */
int dGetTCP(PCALL_DATA pstMMDB, pTCP_KEY pstTCPKey, pTCP_LIST *pstOutput);
int dSetTCPForward(PCALL_DATA pstMMDB, pTCP_KEY pstTCPKey, pTCP_LIST *pstTCP);
int dSetTCPBackword(PCALL_DATA pstMMDB, pTCP_KEY pstTCPKey, pTCP_LIST *pstTCP);
int dSetTCPIndex(PCALL_DATA pstMMDB, UINT uiIndex, pTCP_LIST *pstTCP);
int dDelTCP(PCALL_DATA pstMMDB, pTCP_LIST pstNode);

/* MMDB User Function */
int dInsertMMDB(PCALL_DATA pstSrc, PCALL_DATA *ppstDst);
int dSetMMDB(PCALL_KEY pstKey, PCALL_DATA *pstMMDB);
int dFreeMMDB(PCALL_DATA pstMMDB);

/* Timer User Function */
int dTimerTCPLastProc(time_t stCurrTime, UINT uiTimeOut, UINT uiCheck);
int dTimerTCPFullProc(time_t stCurrTime, UINT uiTimeOut, UINT uiCheck);
int dTimerCALLProc(
		time_t stTime, PCALL_KEY pstFKey, PCALL_KEY pstLKey, UINT uiCheck, UINT uiTimeOut);

#pragma pack(0)
#endif
