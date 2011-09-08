/******************************************************************************* 
        @file   sctpstack.h
 *      - A_SCTP 프로세스를 초기화 하는 함수들
 *
 *      Copyright (c) 2006~ by Upresto Inc. Korea
 *      All rights reserved
 *
 *      $Id: sctpstack.h,v 1.2 2011/09/06 02:07:45 dcham Exp $
 *
 *      @Author     $Author: dcham $
 *      @version    $Revision: 1.2 $
 *      @date       $Date: 2011/09/06 02:07:45 $
 *      @ref        sctpstack.h 
 *
 *      @section    Intro(소개)
 *      - SCTP를 위한 MMDB HEADER 정보
 *
 *      @section    Requirement
 *
*******************************************************************************/
#ifndef __SCTPSTACK_H__
#define __SCTPSTACK_H__

/**A.1*  File Inclusion *******************************************************/
/* SYS HEADER */
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
/* LIB HEADER */
#include "typedef.h"
/* PRO HEADER */
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
/* LOC HEADER */

/**B.1*  Definition of New Constants ******************************************/

/**** LINKED LIST TCP STACK DEFINITION ****************************************/

/**** LINKED LIST TCP SESSION DEFINITION **************************************/
#define MAX_TCP_LIST    		10002 				/* MAX TCP SESSION COUNT */
#define MAX_TCPREAL_LIST    	MAX_TCP_LIST-2 		/* MAX REAL TCP SESSION COUNT */

/********* MMDB SESSION DEFINITION ********************************************/
#define MAX_ASSO_RECORD			10003				/* MAX MMDB ASSOCIATION RECORD COUNT */
#define MAX_ASSOREAL_RECORD		MAX_ASSO_RECORD-1	/* MAX REAL ASSOCIATION COUNT */

/**B.2*  Definition of New Type  **********************************************/
#define DEF_STACK_REQ           1   
#define DEF_STACK_RES           2

/** MMDB ASSOCIATION DEFINITION **/
/** MMDB ASSOCIATION KEY **/
typedef struct _st_AssoKey {
	USHORT		usSrcPort;
	USHORT		usDestPort;
	UINT		uiSysInfo;
} st_AssoKey, *pst_AssoKey;

#define ASSO_KEY             st_AssoKey
#define PASSO_KEY            pst_AssoKey

/** TABLE DISPATCHER'S RECORD DETAILS **/
typedef struct _st_Association
{
    /* MANAGEMENT PARAMETERS */
	ASSO_KEY		stKey;			/* MY KEY */
	struct timeval	stSessTime;		/* START SESSION TIME */
	UINT			uiReqCount;		/* TCP SESSION LIST COUNT */
	UINT			uiReqFirst;		/* TCP SESSION FIRST NODE INDEX */
	UINT			uiReqLast;		/* TCP SESSION LAST NODE INDEX */
	UINT			uiResCount;
	UINT			uiResFirst;
	UINT			uiResLast;
	
	/* USER INFORMATION */
	struct timeval	stUpdateTime;
	UINT			uiReqVerifTag;
	UINT			uiResVerifTag;
	UINT			uiReqInitTSN;
	UINT			uiResInitTSN;

	UINT            uiSrcIPFst;
    UINT            uiSrcIPSnd;
    UINT            uiDestIPFst;
    UINT            uiDestIPSnd;

	UCHAR			ucSetupStatus;	/* INIT, INIT-ACK, COOKIE-ECHO, COOKIE-ACK */
	UCHAR			ucCloseStatus;	/* SHUTDOWN, SHUTDOWN-ACK, SHUTDOWN-COMPLETE */
	UCHAR			szReserved[2];
	USHORT			usInboundStrm;
	USHORT			usOutBoundStrm;

} st_Association, *pst_Association;

#define ASSO_DATA            st_Association
#define PASSO_DATA           pst_Association

/** 최대 record의 길이, 8byte 기준 **/
#define MAX_ASSO_KEY_LEN     ( sizeof( ASSO_KEY ) / 8 )
#define MAX_ASSO_BODY_LEN    ( ( sizeof( ASSO_DATA ) / 8 ) - MAX_ASSO_KEY_LEN )

typedef struct _st_ASSOTYPE {
    ASSO_KEY	key;    		/* PRIMARY KEY */
    INT64   	body[MAX_ASSO_BODY_LEN];
    INT         left;   		/* INDEX LEFT */
    INT         right;  		/* INDEX RIGHT */
    SHORT       bf;     		/* BALANCE FACTOR*/
    SHORT       reserved[3];
} ASSO_TYPE, *PASSO_TYPE;

/** TYPICAL FORMAT OF DATABASE **/
typedef struct _st_MMDB_ASSO_TABLE {
    ASSO_TYPE	tuple[MAX_ASSO_RECORD];
    INT			free;   		/* INDEX FREE */
    INT			root;   		/* INDEX ROOT */
	UINT		uiAssoCount;	/* CURRENT USED ASSOCIATION COUNT */
	UCHAR		szReserved[128];
} ASSO_TABLE, *pASSO_TABLE;

#define ASSO_TABLE_SIZE		sizeof(ASSO_TABLE)
#define ASSO_TYPE_SIZE		sizeof(ASSO_TYPE)
#define ASSO_KEY_SIZE		sizeof(ASSO_KEY)


/**** LINKED LIST TCP STACK SESSION DEFINITION ****/
typedef struct _st_STACK_KEY
{
	UINT		uiTSN;			/* Sequnce Number */
	UINT		uiReserved;
} STACK_KEY, *pSTACK_KEY;

#define STACK_KEY_SIZE	sizeof(STACK_KEY)



#define MAX_TCPDATA_SIZE		2048
typedef struct _st_STACK_LIST
{
    /* Management Parameters */
	ASSO_KEY		stTKey;			/* TCP Parents Key */	
	STACK_KEY		stSKey;			/* Stack Key : My Key */
    struct timeval	stStackTime;	/* Add Time */
    UINT			next;			/* Next Node Index */
    UINT			prev;			/* Previous Node Index */
	UINT			uiStackNext;	/* Only Stack Previous Node Index */
	UINT			uiStackPrev;	/* Only Stack Next Node Index */
	UINT			uiIndex;		/* My Index */
	UCHAR			ucRetranInfo;	/* RETRNSMISSION INFO */
	UCHAR			ucFragStatus;	/* */

	/* User Information */
	struct timeval	stDataTime;		/* DATA RECEIVED TIME */
	struct timeval	stSackTime;		/* SACK RECEIVED TIME */
	UINT			uiChunkOffset;	/* NIFO OFFSET FOR CHUNK DATA */
	USHORT			usStreamID;
	USHORT			usStreamSeq;

	int				dSendQID;
	UINT			uiReserved;

} STACK_LIST, *pSTACK_LIST;
#define MAX_STACK_LIST          100002              /* MAX TCP STACK COUNT */
#define MAX_STACKREAL_LIST      MAX_STACK_LIST-2    /* MAX REAL TCP STACK COUNT */
typedef struct _st_STACK_TABLE {
	UINT            uiFreeList;     /* Free Node List */
	UINT            uiUsedFirst;    /* Used Fist Node Index */
	UINT            uiUsedLast;     /* Used Last Node Index */
	UINT            uiCurrCount;    /* Current Used Node Count */
	UCHAR           szReserved[128];
	STACK_LIST      stNode[MAX_STACK_LIST];
} STACK_TBL, *pSTACK_TBL;

#define STACK_LIST_SIZE		sizeof(STACK_LIST)
#define STACK_TABLE_SIZE	sizeof(STACK_TBL)

typedef struct _st_ASSOSTACK_TABLE {
	STACK_TBL	stStackTbl;
	ASSO_TABLE	stAssoTbl;
} ASSOSTACK_TABLE, *pASSOSTACK_TABLE;

#define ASSOSTACK_TABLE_SIZE		sizeof(ASSOSTACK_TABLE)


/**C.1*  DECLARATION OF VARIABLES  ********************************************/
extern int             		gdASSOSHMID;    	/* SHM ID */
extern pASSOSTACK_TABLE 	pstASSOSTACKTbl; 	/* MMDB&LIST TABLE POINTER */
extern pASSO_TABLE     		pstAssoTbl;    		/* ASSOCIATION MMDB POINTER */
extern pSTACK_TBL    		pstStackTbl;		/* STACK LIST TABLE POINTER */


/**D.1*  DEFINITION OF FUNCTIONS  *********************************************/
/** LINKED LIST STACK INTERNAL FUNCTION DEFINITION **/
pSTACK_LIST 	pGetStackFirstNode();
void 			InitStackList();
void 			FreeStackNode(pSTACK_LIST node);
pSTACK_LIST 	pGetStackNode(UINT *puiIndex);
pSTACK_LIST 	pSetStackNode(UINT uiIndex);

/** MMDB CALL SESSION INTERNAL FUNCATION DEFINITION **/
int 			avl_insert_sess( PASSO_KEY key, long long *body, int *root);
int 			left_rotation_sess( int index, int *pindex );
ASSO_TYPE 		*avl_search_sess( int root, PASSO_KEY key );
int 			avl_delete_sess( int *root, PASSO_KEY key );
ASSO_TYPE 		*avl_select_sess( int root, PASSO_KEY first_key, PASSO_KEY last_key );
int 			avl_update_sess( ASSO_TYPE *tree, long long *body );
ASSO_TYPE 		*get_sess( int index);

void 			Init_SessDB();
int 			sess_alloc();
void 			sess_dealloc( int index );
int 			Insert_SESS(PASSO_DATA pstSrc);			
PASSO_DATA 		Search_SESS(PASSO_KEY stKey);
int 			Delete_SESS(PASSO_KEY pstKey);
int 			Update_SESS(PASSO_DATA disp, PASSO_DATA input);
PASSO_DATA 		pstSelectMMDB(PASSO_KEY pstFKey, PASSO_KEY pstLKey);

/** INITIALIZE INTERNAL MMDB & LINKED LIST **/
int 			dMmapGet(char *path);
int 			dShmGet(int dSHMKey);

/** USER FUNCTION DEFINITION **/
/** INITIALIZE FUNCTION **/
int 			dInitMMAPMMDB(char *szMMAPFile); 		/* INITIALIZING FUNCTION USED FOR MMAP */
int 			dInitSHMMMAB(int dMMDBSHMKey);			/* INITIALIZING FUNCTION USED FOR SHM */

/** TCP STACK LINKED LIST USER FUNCTION **/
int 			dGetStackOnly(pSTACK_LIST *pstStack);
int 			dGetStack(PASSO_DATA pstMMDB, UCHAR ucType, pSTACK_KEY pstSKey, pSTACK_LIST *pstStack);
void 			FreeStackOnly(pSTACK_LIST pstStack);
int 			dSetStackNext(UINT uiIndex, pSTACK_LIST *pstNextStack); 
int 			dSetStackPrev(UINT uiIndex, pSTACK_LIST *pstPrevStack); 
//int 			dSetStackForward(pTCP_LIST pstTCP, UCHAR ucType,  pSTACK_KEY pstSKey, pSTACK_LIST *pstStack);
//int 			dSetStackBackword(pTCP_LIST pstTCP, UCHAR ucType,  pSTACK_KEY pstSKey, pSTACK_LIST *pstStack);
//int 			dSetStackIndex(pTCP_LIST pstTCP, UINT uiIndex, pSTACK_LIST *pstStack);
int 			dDelStack(PASSO_DATA pstMMDB, UCHAR ucType, pSTACK_LIST pstStack);
int				dSetStackIndex(UINT uiIndex, pSTACK_LIST *pstStack);
int 			dAddStackNext(PASSO_DATA pstMMDB, UCHAR ucType, pSTACK_LIST pstStack, pSTACK_LIST pstAddStack);
int 			dAddStackPrev(PASSO_DATA pstMMDB, UCHAR ucType, pSTACK_LIST pstStack, pSTACK_LIST pstAddStack);

/** TCP SESSION LINKED LIST USER FUNCTION **/
//int 			dGetTCP(PCALL_DATA pstMMDB, pTCP_KEY pstTCPKey, pTCP_LIST *pstOutput);
//int 			dSetTCPForward(PCALL_DATA pstMMDB, pTCP_KEY pstTCPKey, pTCP_LIST *pstTCP);
//int 			dSetTCPBackword(PCALL_DATA pstMMDB, pTCP_KEY pstTCPKey, pTCP_LIST *pstTCP);
//int 			dSetTCPIndex(PCALL_DATA pstMMDB, UINT uiIndex, pTCP_LIST *pstTCP);
//int 			dDelTCP(PCALL_DATA pstMMDB, pTCP_LIST pstNode);

/** MMDB USER FUNCTION **/
int 			dInsertMMDB(PASSO_DATA pstSrc, PASSO_DATA *ppstDst);
int 			dSetMMDB(PASSO_KEY pstKey, PASSO_DATA *pstMMDB);
int 			dFreeMMDB(PASSO_DATA pstMMDB);

/** Timer USER FUNCTION **/
int 			dTimerTCPLastProc(time_t stCurrTime, UINT uiTimeOut, UINT uiCheck);
int 			dTimerTCPFullProc(time_t stCurrTime, UINT uiTimeOut, UINT uiCheck);
int 			dTimerASSOProc(time_t stTime, PASSO_KEY pstFKey, PASSO_KEY pstLKey, UINT uiCheck, UINT uiTimeOut);

//pTCP_LIST 		pGetNode(UINT *puiIndex);

/*
* $Log: sctpstack.h,v $
* Revision 1.2  2011/09/06 02:07:45  dcham
* *** empty log message ***
*
* Revision 1.1.1.1  2011/08/29 05:56:42  dcham
* NEW OAM SYSTEM
*
* Revision 1.1  2011/08/05 02:38:56  uamyd
* A_SCTP modified
*
* Revision 1.1.1.1  2011/08/05 00:27:18  uamyd
* init DQMS2
*
* Revision 1.3  2011/01/11 04:09:04  uamyd
* modified
*
* Revision 1.1.1.1  2010/08/23 01:12:57  uamyd
* DQMS With TOTMON, 2nd-import
*
* Revision 1.2  2009/05/27 17:26:22  dqms
* *** empty log message ***
*
* Revision 1.1  2009/05/27 07:38:44  dqms
* *** empty log message ***
*
* Revision 1.1  2009/05/13 11:37:48  upst_cvs
* NEW
*
* Revision 1.1  2008/01/11 12:09:01  pkg
* import two-step by uamyd
*
* Revision 1.4  2007/05/10 06:33:46  doit1972
* ADD QID INFO
*
* Revision 1.3  2007/05/10 02:27:48  doit1972
* MODIFY SCTP HEADER INFO
*
* Revision 1.2  2007/05/04 00:45:10  doit1972
* ADD LOG INFO
*
*/

#endif
