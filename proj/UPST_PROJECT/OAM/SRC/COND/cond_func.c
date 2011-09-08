/***** A.1 * File Include *******************************/

/* SYS HEADER */
/* LIB HEADER */
#include "filedb.h"
#include "loglib.h"
#include "utillib.h"
/* PRO HEADER */
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
#include "almstat.h"
/* LOC HEADER */
#include "cond_func.h"

extern pst_NTAM fidb;

char* cvt_ipaddr(unsigned int uiIP)
{
    return util_cvtipaddr(NULL, uiIP);
}

char* szCvtIPAddr(unsigned int uiIP)
{
    return util_cvtipaddr(NULL, htonl(uiIP));
}

char* cvtTime(time_t when)
{
	static char crmtime_str[81];
    strftime(crmtime_str, 80, "%Y-%m-%d %T %a", localtime((time_t *)&when));
    crmtime_str[21] = toupper(crmtime_str[21]);
    crmtime_str[22] = toupper(crmtime_str[22]);
    return crmtime_str;
}

void set_time(char loctype, char invtype, char invno, time_t tWhen)
{
	int		index;

	index	= -1;

	switch(loctype)
	{
		case LOCTYPE_PHSC:
			switch(invtype)
			{
				case INVTYPE_LINK:
					index = IDX_LINK + invno;
					break;
				case INVTYPE_POWER:
					index = IDX_PWR + invno;
					break;
				case INVTYPE_FAN:
					index = IDX_FAN + invno;
					break;
				case INVTYPE_DISKARRAY:
					index = IDX_DISKAR + invno;
					break;
			}
			break;

		case LOCTYPE_LOAD:
			switch(invtype)
			{
				case INVTYPE_CPU:
					index = IDX_CPU;
					break;
				case INVTYPE_MEMORY:
					index = IDX_MEM;
					break;
				case INVTYPE_DISK:
					index = IDX_DISK;
					break;
				case INVTYPE_QUEUE:
					index = IDX_QUE;
					break;
				case INVTYPE_DBSTATUS:
					index = IDX_DBALIVE;
					break;
				case INVTYPE_NIFO:
					index = IDX_NIFO;
					break;
			}
			break;

		case LOCTYPE_CHNL:
			switch(invtype)
			{
				/*** 이 부분에서는 SYSTYPE을 보아야 하지 않을까 함 */
				case INVTYPE_CLIENT:		/*	GTAM 에서는 INVTYPE_NTAM	*/
					index = IDX_CHNL + invno;
					break;
			}
			break;
	}

	if(index < 0)
	{
		log_print(LOGN_WARN,"UNVALID REFERENCE : LOCTYPE[%d] INVTYPE[%d] INVNO[%d] TIME[%ld]", loctype, invtype, invno, tWhen );
		return;
	}

	fidb->tEventUpTime[index]	= tWhen;
	log_print(LOGN_INFO, "STORED EVENTTIME[%ld] INDEX[%d]", tWhen, index);
}
