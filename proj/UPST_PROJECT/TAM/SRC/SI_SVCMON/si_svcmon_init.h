/*******************************************************************************
			DQMS Project

	Author   :
	Section  : SI_SVCMON
	SCCS ID  : @(#)si_svcmon_init.h	1.1
	Date     : 01/21/05
	Revision History :

	Description :

	Copyright (c) uPRESTO 2005
*******************************************************************************/
#ifndef __SI_SVCMON_INIT_H__
#define __SI_SVCMON_INIT_H__

extern void SetUpSignal(void);
extern int dReadSubSysInfoFile(pst_SubSysInfoList pstList);
extern int dReadRemainFile(void);
extern void UserControlledSignal(int sign);
extern void IgnoreSignal(int sign);
extern int dWriteRemainFile(void);
extern int Init_Fidb(void);
extern int dInitProc(void);
extern void FinishProgram(void);

#endif	/*	__SI_SVCMON_INIT_H__	*/

