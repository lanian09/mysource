/*******************************************************************************
			DQMS Project

	Author   :
	Section  : SI_SVCMON
	SCCS ID  : @(#)si_svcmon_func.h	1.1
	Date     : 01/21/05
	Revision History :

	Description :

	Copyright (c) uPRESTO 2005
*******************************************************************************/
#ifndef __SI_SVCMON_FUNC_H__
#define __SI_SVCMON_FUNC_H__

#include "nsocklib.h"

extern int dCheck_File(st_SI_DB *pSIDB);
extern int dHandleFile(char *name, char *date, char *filename);
extern int dHandleSocketMsg(st_ClientInfo *stNet, int dIdx, st_FDInfo *stFD, char *szBuf);
extern int dHandleMsgQMsg(st_ClientInfo *stNet, st_FDInfo *stFD, st_SI_DB *pSIDB);

#endif	/*	__SI_SVCMON_FUNC_H__	*/

