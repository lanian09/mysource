#ifndef __PRE_A_FLT_H__
#define __PRE_A_FLT_H__

/**
 *	Include headers
 */
// TOP
#include "capdef.h"

// LIB
#include "typedef.h"
#include "hasho.h"
#include "Analyze_Ext_Abs.h"

// .
#include "prea_init.h"	/* st_SYSCFG_INFO */

/**
 *	Define structures
 */
typedef struct _st_SvcOnOff_Data
{
    U32     dSvcCode;
    U32     dOnOffFlag;
} st_SVCONOFF_DATA, *pst_SVC_ONOFF_DATA;
#define DEF_SVCONOFFDATA_SIZE       sizeof(stSVCONOFFDATA)

/**
 *	Declare func.
 */
extern unsigned int GetSubNet(unsigned int ip, int netmask);
extern S32 FilterMOD(st_SYSCFG_INFO *pSYSCFG, INFO_ETH *pINFOETH, U8 ucRtx);
extern S32 FilterPkt_New(stHASHOINFO *pLPREAHASH, INFO_ETH *pINFOETH, U8 *pucRtx );
extern S32	getSvcOnOffFlag(int dSvcCode);
extern S32 FilterPkt(stHASHOINFO *pLPREAHASH, INFO_ETH *pINFOETH, U8 ucRtx );
extern S32 FilterPktSCTP(stHASHOINFO *pLPREASCTP, INFO_ETH *pINFOETH, Capture_Header_Msg *pCAPHEAD );

#endif /* __PRE_A_FLT_H__ */
