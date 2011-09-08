#ifndef __SIM_H__
#define __SIM_H__
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include "logutil.h"


#define MAX_IP_LEN			16
#define MAX_PATH_LEN		128
#define SIM_LOG_PATH		"../log/"
#define SIM_DATA_PATH		"../data/"
#define SIM_CONFIG_PATH		SIM_DATA_PATH"sim.conf"
#define SIM_RAIDUS_PATH		SIM_DATA_PATH"radius.dat"
#define SIM_HTTP_PATH		SIM_DATA_PATH"http.dat"

#define     MAX_HTTP_CACHE  100

#define     HIPADDR(d)      ((d>>24)&0xff),((d>>16)&0xff),((d>>8)&0xff),(d&0xff)
#define     NIPADDR(d)      (d&0xff),((d>>8)&0xff),((d>>16)&0xff),((d>>24)&0xff)

typedef struct _st_dest_info
{
	unsigned int 	uiCFD;					/* client sock */
	unsigned char 	szIP[MAX_IP_LEN];		/* UDP DEST IP Address */
	unsigned short	usPort;     			/* UDP DEST PORT */
	unsigned short  usReserved;				/* reserved feild, will use */
	unsigned int    uiTimeout;				/* time out */

} DEST_INFO, *PDEST_INFO;

typedef struct _st_send_opt
{
	unsigned int	uiMsgType;		/* 전송할 메시지 타입
									 * 1:account_start
									 * 2:account_stop
									 * 3:BOTH) */
	unsigned int	uiLoopCnt;		/* MSGType에 해당하는 메시지 반복 전송 횟수 */
	unsigned int	uiDelayCnt;		/* uiLoopCnt 만큼 전송할때 몇개 메시지를 전송하고
									 * uiDelayTime 만큼 sleep할지 결정 */
	unsigned int	uiDelayTime;	/* MSGType 메시지를 uiLoopCnt만큼 전송할때 
									 * uiDelayCnt 메시지 단위로 uiDelayTime
									 * 만큼 sleep 한다 */
} SEND_OPT, *PSEND_OPT;


typedef struct _st_radius_opt
{
	unsigned int 	uiCFD;					/* client sock */
	unsigned char	szPath[MAX_PATH_LEN]; 	/* radius data path */
	unsigned char 	szIP[MAX_IP_LEN];		/* UDP DEST IP Address */
	unsigned short	usPort;     			/* UDP DEST PORT */
	unsigned short  usReserved;				/* reserved feild, will use */
	unsigned int    uiTimeout;				/* time out */

	unsigned int	uiMsgType;		   		/* 전송할 메시지 타입
									 	 	* 1:account_start
									 	 	* 2:account_stop
									 	 	* 3:BOTH) */
	unsigned int	uiLoopCnt;		    	/* MSGType에 해당하는 메시지 반복 전송 횟수 */
	unsigned int	uiDelayCnt;				/* uiLoopCnt 만큼 전송할때 몇개 메시지를 전송하고
									 	 	* uiDelayTime 만큼 sleep할지 결정 */
	unsigned int	uiDelayTime;			/* MSGType 메시지를 uiLoopCnt만큼 전송할때 
									 	 	* uiDelayCnt 메시지 단위로 uiDelayTime
									 	 	* 만큼 sleep 한다 */
} RAD_OPT, *PRAD_OPT;


typedef struct _st_http_opt
{
	unsigned int 	uiCFD;				/* client sock */
	unsigned char	szPath[MAX_PATH_LEN]; /* http data path */
	unsigned char 	szIP[MAX_IP_LEN];	/* TDP DEST IP Address */
	unsigned short	usPort;     		/* TDP DEST PORT */
	unsigned short  usReserved;			/* reserved feild, will use */
	unsigned int    uiTimeout;			/* time out */

	unsigned int	uiMsgType;		   	/* 전송할 메시지 타입 */
	unsigned int	uiLoopCnt;		    /* MSGType에 해당하는 메시지 반복 전송 횟수 */
	unsigned int	uiDelayCnt;			/* uiLoopCnt 만큼 전송할때 몇개 메시지를 전송하고
									 	 * uiDelayTime 만큼 sleep할지 결정 */
	unsigned int	uiDelayTime;		/* MSGType 메시지를 uiLoopCnt만큼 전송할때 
									 	 * uiDelayCnt 메시지 단위로 uiDelayTime
									 	 * 만큼 sleep 한다 */
} HTTP_OPT, *PHTTP_OPT;



typedef struct _st_conf_info
{
	RAD_OPT		stRadOpt;
	HTTP_OPT	stHttpOpt;

} CONF_INFO, *PCONF_INFO;

/* function prototype */
char *CVT_INT2STR_IP(unsigned int uiIP);
int initUdpSock (void);
int sendPacketUDP(PRAD_OPT pRad, unsigned int  uiBodyLen, unsigned char *szBody, unsigned int sendCnt);
int timeSub (struct timeval *res, const struct timeval *after, const struct timeval *before);
#endif /* __SIM_H__ */

