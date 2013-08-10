/*******************************************************************************
			DQMS Project

	Author   :
	Section  : ALMD
	SCCS ID  : @(#)almd_init.h	1.1
	Date     :
	Revision History :

	Description :

	Copyright (c) uPRESTO 2011
*******************************************************************************/
#ifndef __ALMD_INIT_H__
#define __ALMD_INIT_H__

#include "mmcdef.h"
#include "filedb.h"

#define SHMPERM 0666
#define MAX_STAT_LIST         12
#define MASK_VALUE           128
#define	CHECK_SEND_TIME		   1
#define CHECK_NTAF_TIME		   5
#define	CHECK_NTAF_SYNC		   4
#define CHECK_MSGLIST_GAP	   5
#define	MAX_NTAF_TIME_GAP	  30
#define MAX_FAIL_COUNT         3

extern int  dInit_LoadStat(void);
extern int  dInit_Keepalive(void);
extern int  dLoad_KeepAlive(void);
extern int  dGetSYSCFG(void);
extern int  dInit_SubFidb(void);
extern int  dInit_Fidb(void);
extern int  dInit_Keepalive(void);
extern int  dLoad_KeepAlive(void);
extern int  dInit_LoadStat(void);
extern void SetUpSignal(void);
extern void FinishProgram(void);
extern void UserControlledSignal(int sign);
extern void IgnoreSignal(int sign);

extern int MMC_Handle_Proc(mml_msg *mmsg);
extern int SendToOMP(pst_MngPkt  pstSpkt);
extern int SendToOMPMon(int length, char *data);
extern int dCheckLoadNTAM(st_NTAM *stNTAM);
extern int dSocketCheck(void);
extern int dGetMaxFds(void);
extern int dGetLoadValueStat(int SysType, int SysNo, int InvType, float fVal);
extern int dSetMinMaxVal(float *MinVal, float *MaxVal, float CurVal);
extern int dSetAvgVal(float *AvgVal, float NewVal, int dCount);


#endif /* __ALMD_INIT_H__ */
