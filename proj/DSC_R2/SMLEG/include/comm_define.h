//------------------------------------------------------------------------------
// file name : comm_define.h
// project name : LGT_SCP 
// ��� �ý��ۿ��� �������� ���Ǵ� �޽����� ���� ������ �����Ѵ�.
//------------------------------------------------------------------------------

#ifndef __COMM_DEFINE_H
#define __COMM_DEFINE_H

// system name 
#define SYS_NAME_SCP1		1
#define SYS_NAME_SCP2		2
#define SYS_NAME_SCP3		3
#define SYS_NAME_SCP4		4
#define SYS_NAME_SCP5		5
#define SYS_NAME_SCP6		6
//2007/05/09 choi wonsoon ADD
#define SYS_NAME_SCP7		7

#define SYS_NAME_SMP		50
#define SYS_NAME_OMP		60

//MyDta struct ���� pwdFlag ��
#define S_MTYPE_STATISTICS 		5906	// ��� ��û��
#define S_MTYPE_PERIODIC_STATUS	5908	// MP�� FIMD�� fidb�� �����Ҷ� 
#define S_MTYPE_INTERFACE		5909	// MP�� FIMD�� interface �����Ҷ�
#define S_MTYPE_QUERY			5913	//connect check msg
#define S_MTYPE_SIGTRAN			5914 	//MP�� FIMD�� sigtran �����Ҷ�
#define S_MTYPE_STAT_ONDEMAND	5915	//MP�� STMD�� ondemand ��踦 �����Ҷ� 

#define TIME_CHECK_SND		10
#define TIME_CHECK_RCV		60
//2006/10/10 choi hye kyung ADD
#define S_MTYPE_ALM_REPORT      5911 // MP�� �ǽð����� fault ������ �����Ҷ�

// 2006/09/04 choi hye kyung ADD
#define S_MTYPE_SVCSUBS  5910		// SMP�� �� ������ �䫊��
////////// 2006/09/04 choi hye kyung END

#define S_MTYPE_ALARM			1000 // MP�� COND�� Alarm�� �����Ҷ� 
#define S_PROC_CIAM			8303 // MP�� MMCD�� �����Ҷ�:
#define S_MTYPE_STATUS      4       // MP�� COND�� status �����Ҷ�
#define S_MTYPE_STATIC      1001    // MP�� COND�� STAT �����Ҷ� 

#define SYS_STATE_ACTIVE					1
#define SYS_STATE_STANDBY					2
#define SYS_STATE_FAULTED					3

#endif


