/*******************************************************************************
			DQMS Project

	Author   : 
	Section  : TIFB
	SCCS ID  : @(#)KillMC.h	1.0
	Date     : 
	Revision History :

	Description :

	Copyright (c) uPRESTO 2011
*******************************************************************************/
#ifndef __KILLMC_H__
#define __KILLMC_H__

#include "tifb_util.h"

#define _NO		             -1
#define _YES		          0 
#define   MAX_MSGBUF_LEN	1024

void RemoveSHMnSEMA(void);
void UsageAndExit();
int KillOnlyOneBlock();
void PrintResult(int dKilled, int dTried, int dFailed);

#endif /* __KILLMC_H__ */
