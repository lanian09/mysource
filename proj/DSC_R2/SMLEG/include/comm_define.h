//------------------------------------------------------------------------------
// file name : comm_define.h
// project name : LGT_SCP 
// 모든 시스템에서 공통으로 사용되는 메시지에 대한 정보를 선언한다.
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

//MyDta struct 에서 pwdFlag 값
#define S_MTYPE_STATISTICS 		5906	// 통계 요청시
#define S_MTYPE_PERIODIC_STATUS	5908	// MP가 FIMD로 fidb를 전송할때 
#define S_MTYPE_INTERFACE		5909	// MP가 FIMD로 interface 전송할때
#define S_MTYPE_QUERY			5913	//connect check msg
#define S_MTYPE_SIGTRAN			5914 	//MP가 FIMD로 sigtran 전송할때
#define S_MTYPE_STAT_ONDEMAND	5915	//MP가 STMD로 ondemand 통계를 전송할때 

#define TIME_CHECK_SND		10
#define TIME_CHECK_RCV		60
//2006/10/10 choi hye kyung ADD
#define S_MTYPE_ALM_REPORT      5911 // MP가 실시간으로 fault 정보를 전송할때

// 2006/09/04 choi hye kyung ADD
#define S_MTYPE_SVCSUBS  5910		// SMP로 총 가입자 요쳥시
////////// 2006/09/04 choi hye kyung END

#define S_MTYPE_ALARM			1000 // MP가 COND로 Alarm을 전송할때 
#define S_PROC_CIAM			8303 // MP의 MMCD로 전송할때:
#define S_MTYPE_STATUS      4       // MP의 COND로 status 전송할때
#define S_MTYPE_STATIC      1001    // MP가 COND로 STAT 전송할때 

#define SYS_STATE_ACTIVE					1
#define SYS_STATE_STANDBY					2
#define SYS_STATE_FAULTED					3

#endif


