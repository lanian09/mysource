#ifndef _DIAMETER_HASH_H_
#define _DIAMETER_HASH_H_

#include "define.h"

/**
 * Define constants
 */
//#define SUCC					1
//#define FAIL				   -1
//#define NONE					0
//#define ECHO					2

// diameter_init.c
#define	DIAMETER_TRANS_CNT		20011

// diameter_main.c
#define DEF_MSGREAD_CNT			10000

// diameter_func.c
#define WIDTH16					16
#define DIAMETER_USER_TIMEOUT	1
#define REQUEST					1
#define	DIAMETER_USER_SAMEKEYi	0
#define	DIAMETER_USER_TIMEOUT	1
#define	DIAMETER_USER_RETRANS	2
#define	DIAMETER_USER_FAILED	3
#define	DIAMETER_USER_UNKNOWN	4

/**
 * Define structures
 */
/**< DIAMETER Msg Transaction Key **/
typedef struct _st_diameter_hashkey {                                              
	U16			usSrcPort;		/**<  TCPSCTP Src PORT **/	
	U8			szReservd[6];
	U32			uiSystemId;		/**<  SCTP uiAck **/	
	U32			uiHHID;
} HKey_Trans;                                                                   
#define DEF_HKEY_TRANS_SIZE   sizeof (HKey_Trans)

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


#endif	/* _DIAMETER_HASH_H_ */
