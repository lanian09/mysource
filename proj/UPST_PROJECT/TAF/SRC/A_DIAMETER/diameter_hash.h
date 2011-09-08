/**<  
 $Id: diameter_hash.h,v 1.2 2011/09/07 05:03:01 dcham Exp $
 $Author: dcham $
 $Revision: 1.2 $
 $Date: 2011/09/07 05:03:01 $
 **/

#ifndef __DIAMETER_HASH_H__
#define __DIAMETER_HASH_H__

#include <ctype.h>
#include <netinet/in.h>

#include "common_stg.h"
//#include "logutil.h"
#include "nifo.h"
#include "hasho.h"
#include "timerN.h"
#include "comm_def.h"
#include "define.h"

//#include "taf_names.h"
//#include "utillib.h"

#include "Analyze_Ext_Abs.h"
//#include "utillib.h"

#define WIDTH16   16           

//#define SUCC		1
//#define FAIL	   -1
//#define NONE		0
//#define ECHO		2

#define	REQUEST		1

#define	DIAMETER_TRANS_CNT		20011

/**< DIAMETER Msg Transaction Key **/
typedef struct _st_diameter_hashkey {                                              
	U16			usSrcPort;		/**<  TCPSCTP Src PORT **/	
	U8			szReservd[6];
	U32			uiSystemId;		/**<  SCTP uiAck **/	
	U32			uiHHID;
} HKey_Trans;                                                                   
#define DEF_HKEY_TRANS_SIZE   sizeof (HKey_Trans) 

/**< DIAMETER Msg Transaction Data **/                                             

#define	DIAMETER_USER_SAMEKEY		0
#define	DIAMETER_USER_TIMEOUT		1
#define	DIAMETER_USER_RETRANS		2
#define	DIAMETER_USER_FAILED		3
#define	DIAMETER_USER_UNKNOWN		4

typedef struct _st_diameter_hashdata {
	U64                 ulTimerNID;
	OFFSET 				dOffset;

	int					dReqFlag;
	UINT                uiCmdCode;
	UINT				uiLastEEID;

	UCHAR				szIMSI[MAX_MIN_SIZE];
	UCHAR				szTraceMIN[MAX_MIN_SIZE];

	STIME				CallTime;
	MTIME				CallMTime;
	UINT                uiLastUserErrCode;
	UINT				uiRetransReqCnt;
	UINT                uiReqDataSize;
	UINT                uiResDataSize;
} HData_Trans;                                                                  
#define DEF_HDATA_TRANS_SIZE  sizeof (HData_Trans)

typedef struct _st_timer_trans_data {
    HKey_Trans			key_trans;
} TData_Trans;
#define DEF_TDATA_TRANS_TIMER_SIZE	sizeof (TData_Trans)


/**<  diameter_init.c **/
extern S32 dInitDIAMETERProc(stMEMSINFO **pstMEMS, stHASHOINFO **pstHASHO, stTIMERNINFO **pstTIMER);
extern void SetUpSignal(void);
extern void UserControlledSignal(S32);
extern void FinishProgram(void);
extern void IgnoreSignal(S32);
extern void vDIATRANSTimerReConstruct(stHASHOINFO *pHASH, stTIMERNINFO *pTIMER);

/**<  diameter_func.c **/
int dDecode_DIAMETER_protocol (unsigned char *pData, int ilength);
int dProcDIAMETER_Trans( Capture_Header_Msg *pCAPHEAD, INFO_ETH *pINFOETH, UCHAR *pDATA, UCHAR *pETHDATA, st_DiameterInfo *pstDIAMETERINFO);
HData_Trans * pCreateTransaction(Capture_Header_Msg *pCAPHEAD, INFO_ETH *pINFOETH, HKey_Trans *pstMapKey);
int dump(char *s,int len);
int cb_timeout_transaction(HKey_Trans *pstTransKey);

#endif

/**<  
  $Log: diameter_hash.h,v $
  Revision 1.2  2011/09/07 05:03:01  dcham
  *** empty log message ***

  Revision 1.1.1.1  2011/08/29 05:56:42  dcham
  NEW OAM SYSTEM

  Revision 1.2  2011/08/09 08:17:40  uamyd
  add blocks

  Revision 1.1.1.1  2011/08/05 00:27:17  uamyd
  init DQMS2

  Revision 1.10  2011/05/09 13:32:10  dark264sh
  A_DIAMETER: A_CALL multi 처리

  Revision 1.9  2011/01/11 04:09:06  uamyd
  modified

  Revision 1.1.1.1  2010/08/23 01:12:58  uamyd
  DQMS With TOTMON, 2nd-import

  Revision 1.8  2009/10/08 07:18:14  pkg
  A_DIAMETER hasho local => shm, cont 20000 => 20011

  Revision 1.7  2009/08/19 16:46:04  pkg
  HASH를 못찾는 문제 수정 8Bytes Alignment 문제

  Revision 1.6  2009/08/19 14:04:40  pkg
  TRACE 관련 오류 수정

  Revision 1.5  2009/08/13 07:53:30  jsyoon
  HASH DATA의 offset을 int에서 OFFSET으로 변경

  Revision 1.4  2009/07/15 15:48:43  dqms
  ADD vDIATRANSTimerReConstruct()

  Revision 1.3  2009/07/08 08:35:14  dqms
  ADD TRACE INFO

  Revision 1.2  2009/06/10 21:25:17  jsyoon
  *** empty log message ***

  Revision 1.1  2009/06/02 17:35:55  jsyoon
  *** empty log message ***

  Revision 1.5  2007/07/20 12:31:53  jsyoon
  DISCARD ECHO Message

  Revision 1.4  2007/06/13 06:12:30  jsyoon
  DIAMETER 메세지 추가

  Revision 1.3  2007/05/11 06:08:05  jsyoon
  ADD AVP_PADD Values

  Revision 1.2  2007/05/09 13:02:08  jsyoon
  ADD MESSAGE LOCATION_INFO

  Revision 1.1  2007/05/09 08:17:47  jsyoon
  ADD ADD A_DIAMETER


 **/
