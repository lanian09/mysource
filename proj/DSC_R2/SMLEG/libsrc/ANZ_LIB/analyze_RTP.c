#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <inttypes.h>
#include <stdint.h>

#if 0
#include "Analyze_Ext_Abs.h"
#include "VJ.h"
#include "PPP_header.h"
#include "Ethernet_header.h"
#include "IP_header.h"
#include "TCP_header.h"
#include "UDP_header.h"
#include "IEEE_header.h"

#include "utillib.h"
#include "Errcode.h"
#include "define.h"
#endif

#include "typedef.h"
#include "ana_if.h"
#include "proto_RTP.h"
#include "define.h"

// yhshin
#define TOUSHORT(x) (USHORT)(*(x)<<8|*(x+1))
#define TOULONG(x)  (ULONG)(*(x)<<24|*(x+1)<<16|*(x+2)<<8|*(x+3))
#define TOINT64(x)  (INT64)((INT64)(*(x))<<56|(INT64)(*(x+1))<<48|(INT64)(*(x+2))<<40|(INT64)(*(x+3))<<32|(INT64)(*(x+4))<<24|(INT64)(*(x+5))<<16|(INT64)(*(x+6))<<8|(INT64)(*(x+7)))
//#define TOUSHORT(x)   ntohs((USHORT*)(x))
//#define TOULONG(x)    ntohl((ULONG*)(x))
//#define TOINT64(x)  (INT64)((INT64)(*(x+7))<<56|(INT64)(*(x+6))<<48|(INT64)(*(x+5))<<40|(INT64)(*(x+4))<<32|(INT64)(*(x+3))<<24|(INT64)(*(x+2))<<16|(INT64)(*(x+1))<<8|(INT64)(*(x)))
//

#define LOUCHAR(w)  ((UCHAR)((USHORT)(w) & 0xff))
#define HIUCHAR(w)  ((UCHAR)((USHORT)(w) >> 8))



WORD AnalyzeIP_UDP_RTCP(USHORT usAppCode, ANALYZE_INFO_RTCP *info_rtcp, PUCHAR pucBuffer, WORD wSize)
{
	WORD wIdx = 0;
	PRTCP pRtcp = (PRTCP)pucBuffer;
	WORD wLen = TOUSHORT(pRtcp->Length) << 2;

	info_rtcp->ucVer = pRtcp->Ver;
	info_rtcp->ucCount = pRtcp->Count;
	info_rtcp->wLength = TOUSHORT(pRtcp->Length) << 2; // length = value * 4
	info_rtcp->bType = pRtcp->Type;
	wIdx += 4;

	if( wLen > 0 )
	{
		switch( pRtcp->Type )
		{
			case 200:   // SR
				AnalyzeIP_UDP_RTCP_SR(&info_rtcp->stRRSR, info_rtcp->ucCount, pRtcp->Data, wLen);
				break;
			case 201:   // RR
				AnalyzeIP_UDP_RTCP_RR(&info_rtcp->stRRSR, info_rtcp->ucCount, pRtcp->Data, wLen);
				break;
			case 202:   // SDES
			case 203:   // BYE
				break;
			case 204:   // APP defined
			{
				switch( usAppCode )
				{
					// LGT specific
					case LGTVODSAPPCODE:
						AnalyzeIP_UDP_RTCP_BILL_LGT(&info_rtcp->stBill_Lgt, pRtcp->Data, wLen);
						break;

					// KTF specific
					case KTFVODSAPPCODE:
						AnalyzeIP_UDP_RTCP_BILL_KTF(&info_rtcp->stBill_Ktf, pRtcp->Data, wLen);
						break;

					default:
						break;
				}
			}
			default:
				break;
		}
		wIdx += wLen;
	}

	return wIdx;
}

BOOL AnalyzeIP_UDP_RTCP_SR(ANALYZE_INFO_RTCP_RRSR *rtcp_rrsr, int nCount, PUCHAR pucBuffer, WORD wSize)
{
	PRTCP_SR pSR = (PRTCP_SR)pucBuffer;
	ANALYZE_INFO_RTCP_REPORT *rrsr_report = &rtcp_rrsr->report_block;
	PRTCP_REPORT pReport;

	rtcp_rrsr->dwSSRCsender = TOULONG(pSR->SSRC);

	// sender info
	rtcp_rrsr->llNTPTimestamp = TOINT64(pSR->NTPTimestamp);
	rtcp_rrsr->dwRTPTimestamp = TOULONG(pSR->RTPTimestamp);
	rtcp_rrsr->dwPktCnt = TOULONG(pSR->PacketCount);
	rtcp_rrsr->dwPktBytes = TOULONG(pSR->PacketBytes);

	// report block
	pReport = (PRTCP_REPORT)(pSR->Data);
	rrsr_report->dwSSRCsource = TOULONG(pReport->SSRC);
	rrsr_report->ucFracLoss = pReport->FL;
	rrsr_report->dwPackLoss = (pReport->CNPL[0] << 2) + (pReport->CNPL[1] << 1) + pReport->CNPL[2];
	rrsr_report->dwHighestSeq = TOULONG(pReport->EHSNR);
	rrsr_report->dwJitter = TOULONG(pReport->IJ);
	rrsr_report->dwLastSR = TOULONG(pReport->LSR);
	rrsr_report->dwDelayLastSR = TOULONG(pReport->DLSR);

	return TRUE;
}

BOOL AnalyzeIP_UDP_RTCP_RR(ANALYZE_INFO_RTCP_RRSR *rtcp_rrsr, int nCount, PUCHAR pucBuffer, WORD wSize)
{
	PRTCP_RR pRR = (PRTCP_RR)pucBuffer;
	ANALYZE_INFO_RTCP_REPORT *rrsr_report = &rtcp_rrsr->report_block;
	PRTCP_REPORT pReport;

	rtcp_rrsr->dwSSRCsender = TOULONG(pRR->SSRC);

	rtcp_rrsr->llNTPTimestamp = 0;
	rtcp_rrsr->dwRTPTimestamp = 0;
	rtcp_rrsr->dwPktCnt = 0;
	rtcp_rrsr->dwPktBytes = 0;

	// report block
	pReport = (PRTCP_REPORT)(pRR->Data);
	rrsr_report->dwSSRCsource = TOULONG(pReport->SSRC);
	rrsr_report->ucFracLoss = pReport->FL;
	rrsr_report->dwPackLoss = (pReport->CNPL[0] << 2) + (pReport->CNPL[1] << 1) + pReport->CNPL[2];
	rrsr_report->dwHighestSeq = TOULONG(pReport->EHSNR);
	rrsr_report->dwJitter = TOULONG(pReport->IJ);
	rrsr_report->dwLastSR = TOULONG(pReport->LSR);
	rrsr_report->dwDelayLastSR = TOULONG(pReport->DLSR);

	 return TRUE;
}

BOOL AnalyzeIP_UDP_RTCP_BILL_LGT(ANALYZE_INFO_RTCP_BILL_LGT *rtcp_bill, PUCHAR pucBuffer, WORD wSize)
{
	PRTCP_BILL_LGT pBill = (PRTCP_BILL_LGT)pucBuffer;

	rtcp_bill->dwSSRCsender = TOULONG(pBill->SSRC);

	memcpy(rtcp_bill->ucBillStr, pBill->BillStr, 4);
	rtcp_bill->ucBillStr[4] = 0;
	rtcp_bill->dwRecvDataSize = TOULONG(pBill->DataSize);

	return TRUE;
}

BOOL AnalyzeIP_UDP_RTCP_BILL_KTF(ANALYZE_INFO_RTCP_BILL_KTF *rtcp_bill, PUCHAR pucBuffer, WORD wSize)
{
	PRTCP_BILL_KTF pBill = (PRTCP_BILL_KTF)pucBuffer;

	rtcp_bill->dwSSRCsender = TOULONG(pBill->SSRC);

	memcpy(rtcp_bill->ucBillStr, pBill->BillStr, 4);
	rtcp_bill->ucBillStr[4] = 0;
	rtcp_bill->dwRecvBytes = TOULONG(pBill->BytesReceived);
	rtcp_bill->dwRecvPack = TOULONG(pBill->PackReceived);
	rtcp_bill->dwSequence = TOULONG(pBill->SN);
	rtcp_bill->dwLastSequence = TOULONG(pBill->LastSN);
	rtcp_bill->dwTimestamp = TOULONG(pBill->Timestamp);
	rtcp_bill->dwLastTimestamp = TOULONG(pBill->LastTS);

	return TRUE;
}

BOOL AnalyzeIP_UDP_RTP(ANALYZE_INFO_RTP *info_rtp, PUCHAR pucBuffer, WORD wSize)
{
	PRTP pRtp = (PRTP)pucBuffer;

	// Version. 2 bits
	info_rtp->ucVersion = pRtp->Version;

	// Extension. 1 bit
	info_rtp->ucExtension = pRtp->Extension;

	// CSRC Count. 4 bits
	info_rtp->ucCSRCCount = pRtp->CSRCCount;

	// Marker. 1 bit
	info_rtp->ucMarker = pRtp->Marker;

	// Payload Type. 7 bits
	info_rtp->ucPayloadType = pRtp->PayloadType;

	// Sequence Number. 16 bits
	info_rtp->wSequence = TOUSHORT(pRtp->Sequence);

	// Timestamp. 32 bits
	info_rtp->dwTimestamp = TOULONG(pRtp->Timestamp);

	// SSRC, Synchronization source. 32 bits
	info_rtp->dwSSRC = TOULONG(pRtp->SSRC);

	return TRUE;
}

