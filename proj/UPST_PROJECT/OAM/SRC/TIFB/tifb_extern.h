/*******************************************************************************
			DQMS Project

	Author   : 
	Section  : TIFB
	SCCS ID  : @(#)tifb_extern.h	1.1
	Date     : 
	Revision History :

	Description :

	Copyright (c) uPRESTO 2005
*******************************************************************************/
#ifndef __TIFB_EXTERN_H__
#define __TIFB_EXTERN_H__

extern int GetYorN();
extern int dGetUserPermission();
extern void UsageAndExit();
extern int KillOnlyOneBlock();
extern int GetProcessID();
extern int is_registered_block();
extern void PrintResult();
extern void RemoveMSGQnSHM();

extern int InitOnlyOneBlock();
extern int AnalyzeLine();
extern void PrintBlocks();
extern int IsCorrectBlock();
extern int ProcessStart();
extern void PrintSuccessBlocks();

extern int dMakeNID();

#endif /* __TIFB_EXTERN_H__ */
