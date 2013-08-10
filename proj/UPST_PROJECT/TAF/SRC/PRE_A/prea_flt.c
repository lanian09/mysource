/**
 *	Include headers
 */
#include <stdio.h>
#include <unistd.h>

// TOP
#include "common_stg.h"
#include "commdef.h"

// LIB
#include "loglib.h"
#include "utillib.h"

// .
#include "prea_flt.h"

/**
 *	Declare var.
 */
extern int dSysTypeInfo;
extern st_SVCONOFF_DATA    stSVCONOFFINFO[MAX_SERVICE_CNT];

/**
 *	Declare func.
 */
extern char *PrintRTX(S32 rtx);

/**
 *	Implement func.
 */
unsigned int GetSubNet(unsigned int ip, int netmask)
{
	return (ip & (0xFFFFFFFFU >> (32 - netmask)) << (32 - netmask));
}

S32 FilterMOD(st_SYSCFG_INFO *pSYSCFG, INFO_ETH *pINFOETH, U8 ucRtx)
{
	S32		value;
	U32		ip;
	U8		szIP[INET_ADDRSTRLEN];

	if(ucRtx == DEF_FROM_CLIENT) {
		ip = pINFOETH->stIP.dwSrcIP;
	} else {
		ip = pINFOETH->stIP.dwDestIP;
	}
	value = ip % pSYSCFG->mod;


	if(pSYSCFG->bit[value] == 1) {
		log_print(LOGN_INFO, "FILTER_MOD SND IP=%s:%u mod=%d value=%d rtx=%d:%s", 
			util_cvtipaddr(szIP, ip), ip, pSYSCFG->mod, value, ucRtx, PrintRTX(ucRtx)); 
		return 0;
	} else {
		log_print(LOGN_INFO, "FILTER_MOD DROP IP=%s:%u mod=%d value=%d rtx=%d:%s", 
			util_cvtipaddr(szIP, ip), ip, pSYSCFG->mod, value, ucRtx, PrintRTX(ucRtx)); 
		return -1;
	}
}

S32 FilterPkt_New(stHASHOINFO *pLPREAHASH, INFO_ETH *pINFOETH, U8 *pucRtx )
{
	TAG_KEY_LPREA_CONF		LPREACODEKEY;
	LPREA_CONF				*pLPREACODE;
	TAG_KEY_LPREA_CONF		*pLPREACODEKEY = &LPREACODEKEY;

	stHASHONODE				*pHASHNODE;
	U8		szIP[INET_ADDRSTRLEN];

	
	/* 1. Search SrcIP */
	pLPREACODEKEY->SIP 		= pINFOETH->stIP.dwSrcIP;
	pLPREACODEKEY->SPort 	= pINFOETH->stUDPTCP.wSrcPort;
	pLPREACODEKEY->RpPiFlag = dSysTypeInfo;

	log_print(LOGN_DEBUG, "SIP[%s] PORT[%d] FLAG[%ld]", util_cvtipaddr(szIP, pLPREACODEKEY->SIP), pLPREACODEKEY->SPort, pLPREACODEKEY->RpPiFlag);

	if((pHASHNODE = hasho_find(pLPREAHASH, (U8 *)pLPREACODEKEY)) == NULL) {
		pLPREACODEKEY->SPort = 0;
		if((pHASHNODE = hasho_find(pLPREAHASH, (U8 *)pLPREACODEKEY)) == NULL) {

			/* 2. Search DestIP */
			pLPREACODEKEY->SIP = pINFOETH->stIP.dwDestIP;
			pLPREACODEKEY->SPort = pINFOETH->stUDPTCP.wDestPort;

			if((pHASHNODE = hasho_find(pLPREAHASH, (U8 *)pLPREACODEKEY)) == NULL) {
				pLPREACODEKEY->SPort = 0;
				if((pHASHNODE = hasho_find(pLPREAHASH, (U8 *)pLPREACODEKEY)) == NULL) {
					pINFOETH->usL4Code = 0;
					pINFOETH->usL7Code = 0;
					pINFOETH->usAppCode = 0;
					pINFOETH->usRpPiFlag = 0;
					pINFOETH->usSysType = 0;

					return -2;

				} else {
					//pLPREACODE = (LPREA_CONF *)pHASHNODE->pstData;
					*pucRtx = DEF_FROM_CLIENT;

					pLPREACODE = (LPREA_CONF *)nifo_ptr(pLPREAHASH, pHASHNODE->offset_Data);
					pINFOETH->usL4Code = pLPREACODE->L4Code;
					pINFOETH->usL7Code = pLPREACODE->L7Code;
					pINFOETH->usAppCode = pLPREACODE->AppCode;
					pINFOETH->usRpPiFlag = pLPREACODE->RpPiFlag;
					pINFOETH->usSysType = pLPREACODE->SysType;
				}
			} else {
				//pLPREACODE = (LPREA_CONF *)pHASHNODE->pstData;
				*pucRtx = DEF_FROM_CLIENT;

				pLPREACODE = (LPREA_CONF *)nifo_ptr(pLPREAHASH, pHASHNODE->offset_Data);
				pINFOETH->usL4Code = pLPREACODE->L4Code;
				pINFOETH->usL7Code = pLPREACODE->L7Code;
				pINFOETH->usAppCode = pLPREACODE->AppCode;
				pINFOETH->usRpPiFlag = pLPREACODE->RpPiFlag;
				pINFOETH->usSysType = pLPREACODE->SysType;
			}
		} else {
			//pLPREACODE = (LPREA_CONF *)pHASHNODE->pstData;
			*pucRtx = DEF_FROM_SERVER;

			pLPREACODE = (LPREA_CONF *)nifo_ptr(pLPREAHASH, pHASHNODE->offset_Data);
			pINFOETH->usL4Code = pLPREACODE->L4Code;
			pINFOETH->usL7Code = pLPREACODE->L7Code;
			pINFOETH->usAppCode = pLPREACODE->AppCode;
			pINFOETH->usRpPiFlag = pLPREACODE->RpPiFlag;
			pINFOETH->usSysType = pLPREACODE->SysType;
		}
	} else {
		//pLPREACODE = (LPREA_CONF *)pHASHNODE->pstData;
		*pucRtx = DEF_FROM_SERVER;

		pLPREACODE = (LPREA_CONF *)nifo_ptr(pLPREAHASH, pHASHNODE->offset_Data);
		pINFOETH->usL4Code = pLPREACODE->L4Code;
		pINFOETH->usL7Code = pLPREACODE->L7Code;
		pINFOETH->usAppCode = pLPREACODE->AppCode;
		pINFOETH->usRpPiFlag = pLPREACODE->RpPiFlag;
        pINFOETH->usSysType = pLPREACODE->SysType;
	}

	return 0;
}

S32	getSvcOnOffFlag(int dSvcCode)
{
	int i;

	for(i=0; i<MAX_SERVICE_CNT; i++) {
		if(stSVCONOFFINFO[i].dSvcCode == dSvcCode) {
			return stSVCONOFFINFO[i].dOnOffFlag;
		}
	}

	return 1;
}

S32 FilterPkt(stHASHOINFO *pLPREAHASH, INFO_ETH *pINFOETH, U8 ucRtx )
{
	S32						i, dCnt;	
	U8						ucRTXType[2];
	TAG_KEY_LPREA_CONF		LPREACODEKEY[2];
	LPREA_CONF				*pLPREACODE;
	TAG_KEY_LPREA_CONF		*pLPREACODEKEY;

	stHASHONODE				*pHASHNODE;


	switch( ucRtx ) {
		case DEF_FROM_CLIENT:
			pLPREACODEKEY = &LPREACODEKEY[0]; 
			ucRTXType[0] = DEF_FROM_CLIENT;
			pLPREACODEKEY->SIP = pINFOETH->stIP.dwDestIP;
        	pLPREACODEKEY->SPort = pINFOETH->stUDPTCP.wDestPort;
        	pLPREACODEKEY->RpPiFlag = dSysTypeInfo;
			dCnt = 1;
			break;
		case DEF_FROM_SERVER:
			pLPREACODEKEY = &LPREACODEKEY[0];
			ucRTXType[0] = DEF_FROM_SERVER;
			pLPREACODEKEY->SIP = pINFOETH->stIP.dwSrcIP;
        	pLPREACODEKEY->SPort = pINFOETH->stUDPTCP.wSrcPort;
        	pLPREACODEKEY->RpPiFlag = dSysTypeInfo;
			dCnt = 1;
			break;
		default:
			pLPREACODEKEY = &LPREACODEKEY[0];
			ucRTXType[0] = DEF_FROM_SERVER;
			pLPREACODEKEY->SIP = pINFOETH->stIP.dwSrcIP;
            pLPREACODEKEY->SPort = pINFOETH->stUDPTCP.wSrcPort;
            pLPREACODEKEY->RpPiFlag = dSysTypeInfo;

			pLPREACODEKEY = &LPREACODEKEY[1];
			ucRTXType[1] = DEF_FROM_CLIENT;
			pLPREACODEKEY->SIP = pINFOETH->stIP.dwDestIP;
        	pLPREACODEKEY->SPort = pINFOETH->stUDPTCP.wDestPort;
        	pLPREACODEKEY->RpPiFlag = dSysTypeInfo;
			dCnt = 2;
			break;
	}

	for(i = 0; i < dCnt; i++)
	{
		pLPREACODEKEY = &LPREACODEKEY[i];

		if((pHASHNODE = hasho_find(pLPREAHASH, (U8 *)pLPREACODEKEY)) == NULL) {
			pLPREACODEKEY->SPort = 0;
			if((pHASHNODE = hasho_find(pLPREAHASH, (U8 *)pLPREACODEKEY)) != NULL) {
				//pLPREACODE = (LPREA_CONF *)pHASHNODE->pstData;
				pLPREACODE = (LPREA_CONF *)nifo_ptr(pLPREAHASH, pHASHNODE->offset_Data);
				pINFOETH->usL4Code 		= pLPREACODE->L4Code;
				pINFOETH->usL7Code 		= pLPREACODE->L7Code;
				pINFOETH->usAppCode 	= pLPREACODE->AppCode;
				pINFOETH->usRpPiFlag 	= pLPREACODE->RpPiFlag;
				pINFOETH->usSysType 	= pLPREACODE->SysType;

				return ucRTXType[i];
			}
		} else {
			//pLPREACODE = (LPREA_CONF *)pHASHNODE->pstData;
			pLPREACODE = (LPREA_CONF *)nifo_ptr(pLPREAHASH, pHASHNODE->offset_Data);
			pINFOETH->usL4Code 		= pLPREACODE->L4Code;
			pINFOETH->usL7Code 		= pLPREACODE->L7Code;
			pINFOETH->usAppCode 	= pLPREACODE->AppCode;
			pINFOETH->usRpPiFlag 	= pLPREACODE->RpPiFlag;
			pINFOETH->usSysType 	= pLPREACODE->SysType;

			return ucRTXType[i];
		}
	}

	pINFOETH->usL4Code 		= 0;
    pINFOETH->usL7Code 		= 0;
    pINFOETH->usAppCode 	= 0;
    pINFOETH->usRpPiFlag 	= 0;
    pINFOETH->usSysType 	= 0;

	return -1;
}

S32 FilterPktSCTP(stHASHOINFO *pLPREASCTP, INFO_ETH *pINFOETH, Capture_Header_Msg *pCAPHEAD )
{                       
    TAG_KEY_LPREA_SCTP      LPREACODEKEY;
    LPREA_SCTP              *pLPREACODE1, *pLPREACODE2;
    TAG_KEY_LPREA_SCTP      *pLPREACODEKEY = &LPREACODEKEY;
                    
    stHASHONODE             *pHASHNODE1, *pHASHNODE2;


	pLPREACODEKEY->SIP = pINFOETH->stIP.dwSrcIP;
	if((pHASHNODE1 = hasho_find(pLPREASCTP, (U8 *)pLPREACODEKEY)) != NULL) {
		pLPREACODE1 = (LPREA_SCTP *)nifo_ptr(pLPREASCTP, pHASHNODE1->offset_Data);

		pLPREACODEKEY->SIP = pINFOETH->stIP.dwDestIP;
		if((pHASHNODE2 = hasho_find(pLPREASCTP, (U8 *)pLPREACODEKEY)) != NULL) {
			pLPREACODE2 = (LPREA_SCTP *)nifo_ptr(pLPREASCTP, pHASHNODE2->offset_Data);

			if( pLPREACODE1->Direction == pLPREACODE2->Direction ) {
				log_print( LOGN_CRI, "INVALID REGISTERED SCTP CONF, SAME DIRECTION SRC:%u DST:%u",
								  pINFOETH->stIP.dwSrcIP, pINFOETH->stIP.dwDestIP );
				return -1;
			}

			if( pLPREACODE1->Direction == DIR_DOWN ) {
				/* UP DIRECTION PACKET */
				pINFOETH->stUDPTCP.seq = pLPREACODE1->GroupID;
				pINFOETH->stUDPTCP.ack = pLPREACODE2->GroupID;
				pINFOETH->usSysType = pLPREACODE1->SysType;

				pCAPHEAD->bRtxType = DEF_FROM_CLIENT;
			}
			else {
				pINFOETH->stUDPTCP.seq = pLPREACODE1->GroupID;
                pINFOETH->stUDPTCP.ack = pLPREACODE2->GroupID;
                pINFOETH->usSysType = pLPREACODE1->SysType;

				pCAPHEAD->bRtxType = DEF_FROM_SERVER;
			}
		}
		else {
			log_print( LOGN_INFO, "DSTIP:%u NOT REGISTERED IN SCTP", pINFOETH->stIP.dwDestIP );
            return -1;
        }
	}
	else {
		log_print( LOGN_INFO, "SRCIP:%u NOT REGISTERED IN SCTP", pINFOETH->stIP.dwSrcIP );
		return -2;
	}

    return 0;
}
