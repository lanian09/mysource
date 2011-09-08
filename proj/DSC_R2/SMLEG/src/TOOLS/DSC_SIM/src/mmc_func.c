
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/time.h>
#include <stdarg.h>

#include "mmc.h"
#include "logutil.h"
#include "sim.h"
#include "radius.h"
#include "session.h"

extern 	CONF_INFO       g_conf_info;
//extern	DEST_INFO       g_dest_info;
//extern 	SEND_OPT        g_send_opt;
 
extern 	ACCT_REQ		g_acct_req;
extern 	ACCT_START      g_acct_start;
extern 	ACCT_STOP       g_acct_stop;
extern 	char        	g_http_header [MAX_HTTP_CACHE][2048];
 
extern	unsigned char	g_radius_buf[];

void
set_http (char *fmt,...)
{
	return;
}

void set_radius_SubsID (char *fmt,...)
{
	va_list ap;
    char    *arg1, *arg2, *arg3, *arg4, *arg5;
	
	PACCT_START	pAccStart = &g_acct_req.stAcctStart;

    va_start (ap, fmt); 
    arg1 = va_arg(ap, char*);
    arg2 = va_arg(ap, char*);
    arg3 = va_arg(ap, char*);
    arg4 = va_arg(ap, char*);
    arg5 = va_arg(ap, char*);
    va_end(ap);

	mprintf (" @ [set_radius_SubsID] :");
    mprintf (" %s. %s. %s.%s.%s\n", arg1, arg2, arg3, arg4, arg5);

	strcpy(pAccStart->szMIN, arg5);
	dAppLog (LOG_CRI, " @ set_radius_start_subsID: %s", pAccStart->szMIN);

}

void set_radius_SubsIP (char *fmt,...)
{
	va_list ap;
    char    *arg1, *arg2, *arg3, *arg4, *arg5;
	
	PACCT_START	pAccStart = &g_acct_req.stAcctStart;

    va_start (ap, fmt); 
    arg1 = va_arg(ap, char*);
    arg2 = va_arg(ap, char*);
    arg3 = va_arg(ap, char*);
    arg4 = va_arg(ap, char*);
    arg5 = va_arg(ap, char*);
    va_end(ap);

	mprintf (" @ [set_radius_SubsIP] :");
    mprintf (" %s. %s. %s.%s.%s\n", arg1, arg2, arg3, arg4, arg5);

	pAccStart->uiFramedIP = htonl((unsigned int)inet_addr(arg5));
	dAppLog (LOG_CRI, " @ set_radius_subsip: %d.%d.%d.%d", HIPADDR(pAccStart->uiFramedIP));
}

void set_radius_CBIT (char *fmt,...)
{
	va_list ap;
    char    *arg1, *arg2, *arg3, *arg4, *arg5;
	char	tmp[8];

	PACCT_START	pAccStart = &g_acct_req.stAcctStart;

    va_start (ap, fmt); 
    arg1 = va_arg(ap, char*);
    arg2 = va_arg(ap, char*);
    arg3 = va_arg(ap, char*);
    arg4 = va_arg(ap, char*);
    arg5 = va_arg(ap, char*);
    va_end(ap);

	mprintf (" @ [set_radius_CBIT] :");
    mprintf (" %s. %s. %s.%s.%s\n", arg1, arg2, arg3, arg4, arg5);

	sprintf(pAccStart->szCBit, "cbit=%s", arg5);
	dAppLog (LOG_CRI, " @ set_radius_start_cbit: %s", pAccStart->szCBit);
}

void set_radius_PBIT (char *fmt,...)
{
	va_list ap;
    char    *arg1, *arg2, *arg3, *arg4, *arg5;
	
	PACCT_START	pAccStart = &g_acct_req.stAcctStart;

    va_start (ap, fmt); 
    arg1 = va_arg(ap, char*);
    arg2 = va_arg(ap, char*);
    arg3 = va_arg(ap, char*);
    arg4 = va_arg(ap, char*);
    arg5 = va_arg(ap, char*);
    va_end(ap);

	mprintf (" @ [set_radius_PBIT] :");
    mprintf (" %s. %s. %s.%s.%s\n", arg1, arg2, arg3, arg4, arg5);

	sprintf(pAccStart->szPBit, "pbit=%s", arg5);
	dAppLog (LOG_CRI, " @ set_radius_start_pbit: %s", pAccStart->szPBit);
}

void set_radius_HBIT (char *fmt,...)
{
	va_list ap;
    char    *arg1, *arg2, *arg3, *arg4, *arg5;
	
	PACCT_START	pAccStart = &g_acct_req.stAcctStart;

    va_start (ap, fmt); 
    arg1 = va_arg(ap, char*);
    arg2 = va_arg(ap, char*);
    arg3 = va_arg(ap, char*);
    arg4 = va_arg(ap, char*);
    arg5 = va_arg(ap, char*);
    va_end(ap);

	mprintf (" @ [set_radius_HBIT] :");
    mprintf (" %s. %s. %s.%s.%s\n", arg1, arg2, arg3, arg4, arg5);

	sprintf(pAccStart->szHBit, "hbit=%s", arg5);
	dAppLog (LOG_CRI, " @ set_radius_start_hbit: %s", pAccStart->szHBit);
}

void
set_loglv_num (char *fmt, ...)
{
    va_list ap;
    char    *arg1, *arg2, *arg3;

    va_start (ap, fmt); 
    arg1 = va_arg(ap, char*);
    arg2 = va_arg(ap, char*);
    arg3 = va_arg(ap, char*);
    va_end(ap);

    vdLogLevel = atoi(arg3);

    mprintf ("-- %s. %s. %s.\n", arg1, arg2, arg3);
    mprintf ("-- Set Log Level %d", vdLogLevel);

}

void
reload_conf (char *fmt, ...)
{
    va_list ap;
    char    *arg1, *arg2, *arg3;
    int     client_cnt, i;

    va_start (ap, fmt); 
    arg1 = va_arg(ap, char*);
    arg2 = va_arg(ap, char*);
    arg3 = va_arg(ap, char*);
    va_end(ap);

    mprintf ("-- %s. %s. %s.\n", arg1, arg2, arg3);

	//load_cfg_http(arg3);
}

void
set_send_msg_type (char *fmt, ...)
{
	va_list ap;
    char    *arg1, *arg2, *arg3, *arg4;
    int     client_cnt, i;

    va_start (ap, fmt); 
    arg1 = va_arg(ap, char*);
    arg2 = va_arg(ap, char*);
    arg3 = va_arg(ap, char*);
    arg4 = va_arg(ap, char*);
    va_end(ap);

	mprintf ("set_sned_msg_type:");
    mprintf ("-- %s. %s. %s.%s\n", arg1, arg2, arg3, arg4);

}

void dump_RAD_OPT (PRAD_OPT pRad)
{
	mprintf("\n");
	mprintf(" @ DEST_INFO, uiCFD               :%d\n",pRad->uiCFD);
	mprintf(" @ DEST_INFO, szIP                :%s\n",pRad->szIP);
	mprintf(" @ DEST_INFO, usPort              :%d\n",pRad->usPort);
	mprintf(" @ DEST_INFO, uiTimeout           :%d\n",pRad->uiTimeout);
	mprintf(" @ SEND_OPT, uiMsgType            :%d\n",pRad->uiMsgType);
	mprintf(" @ SEND_OPT, uiLoopCnt            :%d\n",pRad->uiLoopCnt);
	mprintf(" @ SEND_OPT, uiDelayCnt           :%d\n",pRad->uiDelayCnt);
	mprintf(" @ SEND_OPT, uiDelayTime          :%d\n",pRad->uiDelayTime);
}

void dump_SEND_OPT (PSEND_OPT pso)
{
	mprintf("\n");
	mprintf(" @ SEND_OPT, uiMsgType            :%d\n",pso->uiMsgType);
	mprintf(" @ SEND_OPT, uiLoopCnt            :%d\n",pso->uiLoopCnt);
	mprintf(" @ SEND_OPT, uiDelayCnt           :%d\n",pso->uiDelayCnt);
	mprintf(" @ SEND_OPT, uiDelayTime          :%d\n",pso->uiDelayTime);
}

void dump_ACCT_START (PACCT_START past)
{
	mprintf("\n");
	mprintf(" @ ACCT_START, ucCode             :%d\n",past->ucCode);
	mprintf(" @ ACCT_START, ucID               :%d\n",past->ucID);
	mprintf(" @ ACCT_START, usLen              :%d\n",past->usLen);
	mprintf(" @ ACCT_START, llAcctID           :%llx\n",past->llAcctID);
	mprintf(" @ ACCT_START, szMIN              :%s\n",past->szMIN);
	mprintf(" @ ACCT_START, szUserName         :%s\n",past->szUserName);
	mprintf(" @ ACCT_START, uiFramedIP         :%s\n",CVT_INT2STR_IP(ntohl(past->uiFramedIP)));
	mprintf(" @ ACCT_START, dAStatType         :%d\n",past->dAStatType);
	mprintf(" @ ACCT_START, ucNASType          :%d\n",past->ucNASType);
	mprintf(" @ ACCT_START, uiNASIP            :%s\n",CVT_INT2STR_IP(ntohl(past->uiNASIP)));
	mprintf(" @ ACCT_START, dEventTime         :%d\n",past->dEventTime);
	mprintf(" @ ACCT_START, szCBIT             :%s\n",past->szCBit);
	mprintf(" @ ACCT_START, szPBIT             :%s\n",past->szPBit);
	mprintf(" @ ACCT_START, szHBIT             :%s\n",past->szHBit);
}

int send_account_start (PRAD_OPT pRad, PACCT_START pAccStart, unsigned int sendCnt)
{
	unsigned int	msgLen;	
	unsigned char 	*msg = g_radius_buf;
	int				rtn=0;

	mprintf ("send_account_start:\n");
#ifdef DUMP
	dump_RAD_OPT (pRad);
	dump_ACCT_START (pAccStart);
#endif
	msgLen = makeRadius_startMsg (pAccStart, msg, ERADIUS_ACCT_START);
	rtn = sendPacketUDP (pRad, msgLen, msg, sendCnt);
	dAppLog (LOG_CRI, " @ [send_account_start] send fail count: %d", rtn);

	return 0;
}


int send_account_stop (PRAD_OPT pRad, PACCT_STOP pAccStop, unsigned int sendCnt)
{
	unsigned int	msgLen;	
	unsigned char 	*msg = g_radius_buf;
	int				rtn=0;

	mprintf ("send_account_start:\n");
#ifdef DUMP
	dump_RAD_OPT (pRad);
	//dump_ACCT_STOP (pAccStop);
#endif
	msgLen = makeRadius_stopMsg  (pAccStop, msg, ERADIUS_ACCT_STOP);
	rtn = sendPacketUDP (pRad, msgLen, msg, sendCnt);
	dAppLog (LOG_CRI, " @ [send_account_stop] send fail count: %d", rtn);


	return 0;
}


#define MAX_RADIUS_SEND_CNT	4294967295
void
proc_radius (char *operId, char *sendCnt)
{
	PRAD_OPT 	pRad  = &g_conf_info.stRadOpt;
	PACCT_START	pAccStart = (PACCT_START)&g_acct_req.stAcctStart;
	PACCT_STOP	pAccStop  = (PACCT_STOP)&g_acct_req.stAcctStop;

	int temp = -1;
	int sendNum = atoi(sendCnt);
	if (sendNum > MAX_RADIUS_SEND_CNT){
		mprintf ("send count input wrong[1 - %d\n", MAX_RADIUS_SEND_CNT);
		dAppLog (LOG_CRI, "send count input wrong[1 - %d\n", MAX_RADIUS_SEND_CNT);
	}
	// int initUdpSock (DEST_INFO *di)
	temp = initUdpSock ();
	if (temp >=0)
		pRad->uiCFD = temp;

	mprintf ("proc_radius: OPERATION ID: %s, SEND COUNT: %s\n", operId, sendCnt);
	if(!strncmp(operId, "start", 5)) {
		mprintf ("send message type: ACCOUNT REQ START\n");
		send_account_start (pRad, pAccStart, sendNum);
	}
	else if(!strncmp(operId, "stop", 4)) {
		mprintf ("send message type: ACCOUNT REQ STOP\n");
		send_account_stop (pRad, pAccStop, sendNum);
	}
	else {
		mprintf ("send message type: invaild radius-msg type, retry...\n");
		dAppLog (LOG_CRI, "invaild radius-msg type, retry...");
	}

#if 0
	switch(*operId)
	{
	case 49: /* character 1 */
		mprintf ("send message type: ACCOUNT REQ START\n");
		send_account_start (pRad, pAccStart, sendCnt);
		break;
	case 50: /* character 2 */
		mprintf ("send message type: ACCOUNT REQ STOP\n");
		send_account_stop (pRad, pAccStop, sendCnt);
		break;
	case 51: /* character 3 */
		mprintf ("send message type: ACCOUNT REQ START / STOP\n");
		break;
	default:
		mprintf ("send message type: invaild radius-msg type, retry...\n");
		dAppLog (LOG_CRI, "invaild radius-msg type, retry...");
	}
#endif
}

void
send_radius (char *fmt, ...)
{
	va_list ap;
    char    *arg1, *arg2, *arg3, *arg4;

    va_start (ap, fmt); 
    arg1 = va_arg(ap, char*);
    arg2 = va_arg(ap, char*);
    arg3 = va_arg(ap, char*);
    arg4 = va_arg(ap, char*);
    va_end(ap);

	mprintf ("send_radius:");
    mprintf ("-- %s. %s. %s.%s\n", arg1, arg2, arg3, arg4);
	dAppLog (LOG_CRI, " @ send_radius : arg3:%s, arg4:%s", arg3, arg4);

	proc_radius (arg3, arg4);
}



void
set_client_cnt (char *fmt, ...)
{
#if 0
    va_list ap;
    char    *arg1, *arg2, *arg3;
    int     client_cnt, i;
	_hkey_fd 		hfd_key;
	_hbody_fd		*hfd_body, stData;

    va_start (ap, fmt); 
    arg1 = va_arg(ap, char*);
    arg2 = va_arg(ap, char*);
    arg3 = va_arg(ap, char*);
    va_end(ap);

    client_cnt = atoi(arg3);

    mprintf ("-- %s. %s. \n", arg1, arg2);
    mprintf ("-- Set Client Cnt! %d\n", client_cnt);


	/**< Create TCP Client 
	  1. TCP Client 생성. 
	  2. Session 에 등록. 
	 **/



for (i=0; i < client_cnt; i++) {

	hfd_key.sockfd = create_tcp_sock ();
	if (hfd_key.sockfd < 0) {
		fprintf (stderr, "add_fd_session fail !!\n");
		exit(0);
	}

	hfd_body = find_fd_session (&hfd_key);
	if (hfd_body == NULL) {
		hfd_body = add_fd_session (&hfd_key, &stData);
		if (hfd_body == NULL) {
			fprintf (stderr, "add_fd_session fail !!\n");
			exit(0);
		}

		hfd_body->tcpsock.sockfd = hfd_key.sockfd;
		hfd_body->tcpsock.f_flags = F_CONNECTING;		// F_SID_INIT
		
		hfd_body->dest_ipaddr 	= g_proxy_ip;
		hfd_body->dest_port 	= htons(g_proxy_port);
		if (start_connect (&hfd_key, hfd_body) < 0) {
			fprintf (stderr, "connection fail !! fd: %d.\n", hfd_body->tcpsock.sockfd);
			exit(0);
		}

//		fprintf (stderr, "connection success !! fd: %d.\n", hfd_body->tcpsock.sockfd);

	} else {
		; // 대기
	}
}
#endif

}

int get_idx_http_cmd (int http_cmd) {
#if 0
	switch (http_cmd) {
	case 200:
		return RESP_CODE_200;
		break;
	case 304:
		return RESP_CODE_304;
		break;
	default:
		printf ("Not Define. Default: 200\n");
		break;
	}

	return RESP_CODE_200;
#endif
	return 0;
}

void
send_http_num_only (int sockfd) 
{
#if 0
    _hbody_fd       *phfd_body;
    _hkey_fd        hfd_key;
    int     i, len, wlen;
	char 	sendbuf[4096];

	len = snprintf (sendbuf, sizeof(sendbuf), "%s", g_http_header[30]);

	wlen = write (sockfd, g_http_header[30], len);
	hfd_key.sockfd = sockfd;
	if ((phfd_body = find_fd_session ((_hkey_fd *)&hfd_key)) == NULL ) {
		dAppLog (LOG_CRI, "find_fd_session fail %d!", hfd_key.sockfd);
		close (sockfd);
//		g_sock [i--] = g_sock[g_sock_cnt];
//		g_sock_cnt--;
		return;
	}

	phfd_body->send_timer = timerN_add (pReTimerInfo, invoke_send_wait_timeout, (U8 *)&sockfd, sizeof(sockfd), time(0) + g_wait_timeout);
	if (phfd_body->send_timer == 0) {
		dAppLog (LOG_CRI, "timer add fail !, %d", sockfd);
//		del_fd_session (g_sock[i--]);
//		g_sock [i--] = g_sock[g_sock_cnt];
//		g_sock_cnt--;
		return;
	}

	dAppLog (LOG_WARN, "fd.. %d. write:%d-%d", sockfd, len, wlen);

#endif
}

void
send_http_num (char *fmt, ...)
{
    va_list ap;
    char    *arg1, *arg2, *arg3;
	int msg_idx=0;

    va_start (ap, fmt); 
    arg1 = va_arg(ap, char*);
    arg2 = va_arg(ap, char*);
    arg3 = va_arg(ap, char*);
    va_end(ap);

    msg_idx= atoi(arg3);

    //mprintf ("-- %s. %s. %s\n", arg1, arg2, arg3);
	dAppLog (LOG_CRI, " @ send_http_num: %d", msg_idx);

	proc_http (msg_idx);
}

int proc_http (int index)
{
    int     i, len, wlen;
	char 	sendbuf[4096];
	hs_key		hsKey;
	hs_body     *phsBody, hsBody;

	int cfd=-1;
	PHTTP_OPT 	pHttp  = &g_conf_info.stHttpOpt;
	cfd = pHttp->uiCFD; 

	printf ("%s.\n", g_http_header[index]);
	len = snprintf (sendbuf, sizeof(sendbuf), "%s", g_http_header[index]);
	printf ("%s.\n", sendbuf);

	for (i=0; i< pHttp->uiLoopCnt; i++)
	{
		/* socket create */
		hsKey.sockfd = create_tcp_sock ();
		if (hsKey.sockfd < 0) {
			dAppLog (LOG_CRI, "add_fd_session fail");
			return -1;
		}

		/* session add */
		phsBody = find_fd_session (&hsKey);
		if (phsBody == NULL) {
			phsBody = add_fd_session (&hsKey, &hsBody);
			if (phsBody == NULL) {
				dAppLog (LOG_CRI, "add_fd_session fail");
				return -1;
			}

			phsBody->cfd = hsKey.sockfd;
			phsBody->dest_ipaddr   = (unsigned int)inet_addr(pHttp->szIP);
			phsBody->dest_port     = htons(pHttp->usPort);

		/* connection */
			if (start_connect (&hsKey, phsBody) < 0) {
				dAppLog (LOG_CRI, "sconnection fail !! fd: %d\n", phsBody->cfd);
				return -1;
			}
		}else {
			dAppLog (LOG_CRI, "send fail, session already exist");
			return -1;
		}

		/* write packet */
		wlen = write (cfd, g_http_header[index], len);
        dAppLog(LOG_DEBUG, "SendToTCP FD:%d [%s][%d] : send size[%d/%d]"
				            , hsKey.sockfd, pHttp->szIP, pHttp->usPort, wlen, len);
		/* set timer */
		phsBody->tmr_id = timerN_add (pTmrNInfo
									, (void *)invoke_send_wait_timeout
									, (U8 *)&hsKey
									, HKEY_FD_SIZE
									, time(NULL)+pHttp->uiDelayTime);
		if (phsBody->tmr_id == 0) {
			dAppLog (LOG_CRI, "timer add fail !, %d", cfd);
			sessionRelease (-1, cfd, phsBody->tmr_id);
			return -1;
		}
	}
	return 0;
}


void
set_loop_num (char *fmt, ...)
{
#if 0
    va_list ap;
    char    *arg1, *arg2, *arg3, *arg4;
    int     loop_cnt;
	TIMERNID            loop_timer;

    va_start (ap, fmt); 
    arg1 = va_arg(ap, char*);
    arg2 = va_arg(ap, char*);
    arg3 = va_arg(ap, char*);
    arg4 = va_arg(ap, char*);
    va_end(ap);

    loop_cnt = atoi(arg3);

    mprintf ("-- %s. %s. %s. %s\n", arg1, arg2, arg3, arg4);
    mprintf ("-- Set loop_cnt! %d\n", loop_cnt);

	g_auto_client_cnt = atoi(arg3);
	g_auto_timer_cnt = atoi(arg4);

	printf ("set auto configuration!\n");
	printf ("send client %ld /sec \n", g_auto_client_cnt);
	printf ("timer cnt %ld cnt\n", g_auto_timer_cnt);
//	set_client_cnt ("%s %s %s", "set", "client", arg3 );

	loop_timer = timerN_add (pTimerInfo, invoke_loop_timeout, (U8 *)&g_auto_timer_cnt, sizeof(g_auto_timer_cnt), time(0) + 2);
	if (loop_timer == 0) {
		dAppLog (LOG_CRI, "loop timer add fail (time 2 sec)!");
	} else 
		g_auto_send = 1;
#endif
}
