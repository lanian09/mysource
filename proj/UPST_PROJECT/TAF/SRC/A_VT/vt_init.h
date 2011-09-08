/*
 * vt_init.h
 *
 *  Created on: 2011. 8. 10.
 *      Author: hohyun
 */

#ifndef _VT_INIT_H_
#define _VT_INIT_H_

/**
 *	Define constants
 */
#define MEMG_FREE				0
#define MEMG_ALLOCED			1
#define MEMG_MAX_DEBUG_STR		11
#define MEMG_ID					0x10101010

#define DEF_SR                  200
#define DEF_RR                  201
#define DEF_BILL                204

#define STG_DEF_TIMER_INFO      173     /* Hex( 52b5d0 ) */
#define TIMER_INFO_SIZE 		sizeof(TIMER_INFO)

/**
 *	Define structures
 */
typedef struct _st_memgnodehdr {
	U32	uiID;			/**< MEMG_ID : mem debug를 위한 부분 : free시 확인 */
	STIME	TimeSec;	/**< Debug나 garbage collection을 위한 시간 저장 */
	U8	ucIsFree;			/**< Free:0 , Alloc : 1 */
	S8	DebugStr[MEMG_MAX_DEBUG_STR];	/**< debugging을 위해서 사용된 */
} stMEMGNODEHDR;
#define STG_DEF_stMEMGNODEHDR		101		/* Hex( 1813f1e0 ) */
#define stMEMGNODEHDR_SIZE sizeof(stMEMGNODEHDR)

typedef struct _st_call_timer_arg {
	IP4				ClientIP;
} st_CALLTimer;

typedef struct _st_SIPSessKey {
    UINT    uiClientIP;
    UINT    uiReserved;
} st_SIPSessKey, *pst_SIPSessKey;

typedef struct _st_RTCP_COMM_
{
    UCHAR   ucVerCnt;
    UCHAR   ucType;
    USHORT  usLength;
    UINT    uiSSRC;
} st_RTCP_COMM, *pst_RTCP_COMM;
#define DEF_RTCP_COMM_SIZE      sizeof( st_RTCP_COMM )
#define DEF_RTCP_SRHDR_SIZE		20

typedef struct _st_RTCP_SR_
{
    INT64   llNTPTime;  
    UINT    uiRTPTime;  
    UINT    uiSendPktCnt;
    UINT    uiSendOctCnt;
} st_RTCP_SR, *pst_RTCP_SR;
    
typedef struct _st_RTCP_RR_
{   
    UINT    uiSSRC;     
    UINT    uiLostInfo;
    UINT    uiSeqNum;
    UINT    uiJitter;
    UINT    uiLSR;
    UINT    uiDLSR;
} st_RTCP_RR, *pst_RTCP_RR;

/**
 *	Declare functions
 */
S32 dInitVT(stMEMSINFO **pMEMSINFO, stHASHOINFO **pHASHOINFO, stTIMERNINFO **pTIMERNINFO);
void SetUpSignal(void);
void UserControlledSignal(S32 isign);
void FinishProgram(void);
void IgnoreSignal(S32 isign);
void vVTTimerReConstruct(stHASHOINFO *pHASH, stTIMERNINFO *pTIMER);


#endif /* _VT_MAIN_H_ */
