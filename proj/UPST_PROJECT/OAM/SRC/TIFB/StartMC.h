/*******************************************************************************
			DQMS Project

	Author   : 
	Section  : TIFB
	SCCS ID  : @(#)StartMC.h	1.1
	Date     : 
	Revision History :

	Description :

	Copyright (c) uPRESTO 2005
*******************************************************************************/
#ifndef __STARTMC_H__
#define __STARTMC_H__

#include "tifb_util.h"

#define _NO		             -1
#define _YES		          0 
#define _FALSE               -1
#define _TRUE                 0
#define MAX_ERRBUF_LEN     1024

int InitOnlyOneBlock(char *szBlockName);
int AnalyzeLine(char *szBuf);
void PrintBlocks(void);
int IsCorrectBlock(int dIdx);
int ProcessStart(int dIdx);
void PrintSuccessBlocks(void);

#endif /* __STARTMC_H__ */
