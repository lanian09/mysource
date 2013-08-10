/**
	@file		fstat_traffic.h
	@author		
	@version	
	@date		2011-07-27
	@brief		fstat_traffic.c 헤더파일
*/

/**
 *	Include headers
 */

/* SYS HEADER */
#include <time.h>
/* LIB HEADER */
#include "loglib.h"
/* PRO HEADER */
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
#include "almstat.h"
/* LOC HEADER */
#include "fstat_init.h"
#include "fstat_traffic.h"

/**
 *	Declare functions
 */
extern void Insert_STAT_TRAFFICData(st_TRAFFIC *pstData, int dTotCount);

/**
 *	Implement func.
 */
int dCheckTrafficList(time_t tLocalTime, st_NtafStatList *stTRAFFIC)
{
	st_TRAFFIC	pDate;

	log_print(LOGN_DEBUG, "%9s: [%10u] [%10lu]", "ThruStat",  stTRAFFIC->ThruStat.uiFrames, stTRAFFIC->ThruStat.ulBytes);

	log_print(LOGN_DEBUG, "%9s: [%10u] [%10lu]", "TotStat",   stTRAFFIC->TotStat.uiFrames, stTRAFFIC->TotStat.ulBytes);
	log_print(LOGN_DEBUG, "%9s: [%10u] [%10lu]", "IPStat",    stTRAFFIC->IPStat.uiFrames, stTRAFFIC->IPStat.ulBytes);
	log_print(LOGN_DEBUG, "%9s: [%10u] [%10lu]", "UDPStat",   stTRAFFIC->UDPStat.uiFrames, stTRAFFIC->UDPStat.ulBytes);
	log_print(LOGN_DEBUG, "%9s: [%10u] [%10lu]", "TCPStat",   stTRAFFIC->TCPStat.uiFrames, stTRAFFIC->TCPStat.ulBytes);
	log_print(LOGN_DEBUG, "%9s: [%10u] [%10lu]", "SCTPStat",  stTRAFFIC->SCTPStat.uiFrames, stTRAFFIC->SCTPStat.ulBytes);
	log_print(LOGN_DEBUG, "%9s: [%10u] [%10lu]", "ETCStat",   stTRAFFIC->ETCStat.uiFrames, stTRAFFIC->ETCStat.ulBytes);

	log_print(LOGN_DEBUG, "%9s: [%10u] [%10lu]", "IPError",   stTRAFFIC->IPError.uiFrames, stTRAFFIC->IPError.ulBytes);
	log_print(LOGN_DEBUG, "%9s: [%10u] [%10lu]", "UTCPError", stTRAFFIC->UTCPError.uiFrames, stTRAFFIC->UTCPError.ulBytes);
	log_print(LOGN_DEBUG, "%9s: [%10u] [%10lu]", "FailData",  stTRAFFIC->FailData.uiFrames, stTRAFFIC->FailData.ulBytes);
	log_print(LOGN_DEBUG, "%9s: [%10u] [%10lu]", "FilterOut", stTRAFFIC->FilterOut.uiFrames, stTRAFFIC->FilterOut.ulBytes);

	pDate.uiStatTime			= stTRAFFIC->KeyTime;
//	pDate.usTAFID				= stTRAFFIC->ucNTAFID;
	pDate.usTAFID				= stTRAFFIC->ucTAFID;

	pDate.ThruStat.uiFrames		= stTRAFFIC->ThruStat.uiFrames;
	pDate.ThruStat.ulBytes		= stTRAFFIC->ThruStat.ulBytes;

	pDate.TotStat.uiFrames		= stTRAFFIC->TotStat.uiFrames;
	pDate.TotStat.ulBytes		= stTRAFFIC->TotStat.ulBytes;

	pDate.IPStat.uiFrames		= stTRAFFIC->IPStat.uiFrames;
	pDate.IPStat.ulBytes		= stTRAFFIC->IPStat.ulBytes;

	pDate.UDPStat.uiFrames		= stTRAFFIC->UDPStat.uiFrames;
	pDate.UDPStat.ulBytes		= stTRAFFIC->UDPStat.ulBytes;

	pDate.TCPStat.uiFrames		= stTRAFFIC->TCPStat.uiFrames;
	pDate.TCPStat.ulBytes		= stTRAFFIC->TCPStat.ulBytes;

	pDate.SCTPStat.uiFrames		= stTRAFFIC->SCTPStat.uiFrames;
	pDate.SCTPStat.ulBytes		= stTRAFFIC->SCTPStat.ulBytes;

	pDate.ETCStat.uiFrames		= stTRAFFIC->ETCStat.uiFrames;
	pDate.ETCStat.ulBytes		= stTRAFFIC->ETCStat.ulBytes;

	pDate.IPError.uiFrames		= stTRAFFIC->IPError.uiFrames;
	pDate.IPError.ulBytes		= stTRAFFIC->IPError.ulBytes;

	pDate.UTCPError.uiFrames	= stTRAFFIC->UTCPError.uiFrames;
	pDate.UTCPError.ulBytes		= stTRAFFIC->UTCPError.ulBytes;

	pDate.FailData.uiFrames		= stTRAFFIC->FailData.uiFrames;
	pDate.FailData.ulBytes		= stTRAFFIC->FailData.ulBytes;

	pDate.FilterOut.uiFrames	= stTRAFFIC->FilterOut.uiFrames;
	pDate.FilterOut.ulBytes		= stTRAFFIC->FilterOut.ulBytes;

	Insert_STAT_TRAFFICData(&pDate, 1);

	return 0;
}
