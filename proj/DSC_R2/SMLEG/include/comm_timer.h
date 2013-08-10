#ifndef COMM_TIMER_H
#define COMM_TIMER_H

#pragma pack(1)

/* General Macro */
#define TIMER_KIND_NUM          2   /* the number of kinds of Timer   */
#define TIMER_KIND_GUBUN_NUM    9   /* the number of kinds of Timer   */


#define TIMER_KIND_SESSION      0   /* Message - kind of Timer        */
#define TIMER_KIND_MESSAGE      1   /* Message - kind of Timer        */
#define TIMER_KIND_CALL         TIMER_KIND_SESSION   /* call - kind of Timer        */

#define TIMER_KIND_SESS_CALL    0
#define TIMER_KIND_SESS_LCALL   1
#define TIMER_KIND_SESS_TCPIP   2
#define TIMER_KIND_SESS_WAP1    3
#define TIMER_KIND_SESS_WAP2    4
#define TIMER_KIND_SESS_HTTP    5
#define TIMER_KIND_SESS_VODS    6
#define TIMER_KIND_SESS_VTSVC   7
#define TIMER_KIND_SESS_FBCDR   8
#define TIMER_KIND_MSG_AAA      0

typedef struct _stTimer{
	unsigned int timer[TIMER_KIND_NUM][TIMER_KIND_GUBUN_NUM];
}stTimer;


typedef struct _st_MPTimer{
	unsigned int sess_timeout;
	unsigned int sm_sess_timeout;
	unsigned int sms_timeout;
} MPTimer;

#pragma pack(0)
#endif
