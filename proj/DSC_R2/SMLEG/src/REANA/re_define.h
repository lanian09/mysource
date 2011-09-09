#ifndef __RE_DEFINE_H
#define __RE_DEFINE_H

/**A.1*	FILE INCLUSION ********************************************************/
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>

#include <eth_capd.h>
#include <ipaf_define.h>
#include <ipaf_names.h>
#include <ipaf_shm.h>
#include <init_shm.h>
#include <shmutil.h>
#include <ipaf_sem.h>

#include <Num_Proto.h>
#include <Analyze_Ext_Abs.h>
#include <Ethernet_header.h>
#include <define.h>

#include <utillib.h>
#include <Errcode.h>

#include "ipaf_svc.h"
#include <ipam_ipaf.h>
#include <packet_def.h>
#include <pps_conf.h>

#include "mems.h"
#include "nifo.h"

/* Function Time Check */
#include "func_time_check.h"

/**B.1*	DEFINITION OF NEW CONSTANTS *******************************************/
#define	UP			2
#define DOWN		1

#define TCP_SYN		0x02
#define TCP_SYNACK 	0x12
#define TCP_ACK		0x10
#define TCP_RST		0x04
#define TCP_FIN		0x01

#define	SYN			128
#define SYNACK		64
#define ACK			32
#define	RST			16
#define FIN			8

#define	UDP_HEADER_LEN	8

/* STAT FLAG */
#define	STAT_TOTSTAT	1
#define	STAT_UDPSTAT	2
#define	STAT_TCPSTAT	3
#define	STAT_ETCSTAT	4
#define STAT_USERSTAT	5
#define	STAT_IPERROR	6
#define	STAT_UTCPERROR	7
#define	STAT_ETCERROR	8
#define STAT_FILTEROUT	9

#define DUAL_MOBILE		2  /* 자사간 서비스일 경우 */  
#define	UNI_MOBILE		1 /*타사와의 서비스일경우 */

/* ADD BY YOON 2008/09/17 */
#define		QID_RANA		1
#define		QID_REANA		2
#define		QID_RLEG		3
#define		QID_RADIUS		4

/**B.2*	DEFINITION OF NEW TYPE ************************************************/
#define TOUSHORT(x) (USHORT)(*(x)<<8|*(x+1))
#define TOULONG(x)  (ULONG)(*(x)<<24|*(x+1)<<16|*(x+2)<<8|*(x+3))


/**D.1*  DEFINITION OF FUNCTIONS  *********************************************/
extern int 	Init_msgq( key_t q_key );

int			dMakeNIDTID(UCHAR ucSysType, UCHAR ucSvcID, UCHAR ucMsgID, INT64 *pllNID, INT64 *pllTID);
int			dMakeNID(UCHAR ucSysType, INT64 *pllNID);
int			dAppLog(int dIndex, char *fmt, ...);

void		FinishProgram();
void		SetUpSignal();

void		Init_Stat();
void		Check_StatTimer(time_t now);
void		Set_Stat( int iFlag, unsigned int uiFrames, unsigned int uiBytes );

/* ADD BY LDH 2004/10/19 */
int 		dSend_Data( int dSvcCatInfo, UCHAR ucSvcBlk, USHORT usCatID, UCHAR ucURLChar, UCHAR ucUDRFlag );

/* LG BMT 2006.04.11 */
UINT		CVT_UINT( UINT value );
INT64		CVT_INT64( INT64 value );
INT			CVT_INT( INT value );
inline int 	dAnalyze_RADIUS( PUCHAR pBuf, pst_RADInfo pstAccReq, pst_IPTCPHeader pstIPTCPHeader );
inline int	dAnalyze_RADIUS_ATTRIB( PUCHAR pBuf, USHORT usDataLen, pst_RADInfo pstAccInfo );
int			dSend_AccReq( st_IPTCPHeader *pstIPTCPHeader, pst_RADInfo pstAccReq );
void		print_ana_stat(int level, int index);
int			dReadIPPOOL();
int			keepalivelib_init(char *processName);
void		keepalivelib_increase();
int			conflib_getNthTokenInFileSection ( char *fname, char *section, char *keyword, int n, char *string );
void		Set_Stat( int iFlag, unsigned int uiFrames, unsigned int uiBytes );
void		Set_StatRad(unsigned int uiPDSNAddr, int index, int type, int flag);

char		*CVT_ipaddr(UINT uiIP);

/* ADD BY YOON 2008.10.16 */
OFFSET mif_msg_read(stMEMSINFO *pstMEMSINFO, st_MsgQ *pstMsgQ);

/**D.2*  DEFINITION OF FUNCTIONS  *********************************************/

extern int      check_my_run_status (char *procname);
extern int		dLoad_PDSNIP();

int dProcessRedirect(UCHAR *pNode, UCHAR *pNodeData, st_IPTCPHeader *pstIPTCPHeader, pst_MsgQ pstMsgQ);

extern int dIsRcvedMessage(pst_MsgQ pstMsgQ);
extern void dSendToMsg(int qid, pst_MsgQ pstMsgQ, st_IPTCPHeader *pstIPTCPHeader);
extern void dSendToMsg2 (int qid, pst_MsgQ pstMsgQ, st_IPTCPHeader *pstIPTCPHeader);
extern int parsedata(char *sp, int slen);


#endif /* __RE_DEFINE_H__ */

