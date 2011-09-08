#ifndef __SMS_MSGTYPES_H__
#define __SMS_MSGTYPES_H__

/* 
 * KTF 지능망 SMS 전용의 알람 정보
 * alarm을 report하기 위한 mtype은 MTYPE_SMS_ALARM_REPORT 사용
 * 1. 알람 발생/해제가 필요한 경우 : almTime에 SMS_MANUAL_CLEAR_ALARM(0) 설정
 * 2. 알람 발생 후 자동 해제되는 경우 : almTime에 가청 경보가 유지될 시간(초) 지정
 * almInfo에 각 alarm을 구분할 수 있는 임의의 Text를 MP에서 정해서 설정해줌
 * ==> OMP는 해제시 current alarm list에서 sysname, appname, alminfo를 사용하여 삭제함
 */
#define SMS_MANUAL_CLEAR_ALARM  	0

/* event성 알람에 대한 기간 */
#define SMS_DEFAULT_ALARM_PERIOD  	5		/* 미지정시 default 알람 기간 */
#define SMS_MIN_ALARM_PERIOD  		0		/* event 알람의 최소 경보 기간 (0은 알람을 안울림) */
#define SMS_MAX_ALARM_PERIOD  		3600	/* event 알람의 최대 경보 기간 */

typedef struct {
    int  almTime;           /* 알람이 울리는 시간 , 0 이면 반드시 해제하고, > 0 이면 해당 기간(초) 후에 자동 해제 */
    int  almLevel;          /* 알람 등급, normal(0), minor(1), major(2), critical(3) */
    char almSysName[32];    /* 알람 발생 시스템 */
    char almProcName[32];   /* 알람 발생 프로세스 */
    char almInfo[32];       /* 알람 발생 구분 정보(text) */
    char almMsg[512];       /* 알람 상세 정보, console로 뿌려지는 메시지  */
} SMS_AlmMsgType;

#endif /*__SMS_MSGTYPES_H__*/

