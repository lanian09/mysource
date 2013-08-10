#include "cscm.h"

#define SA  struct sockaddr

extern int  	errno, trcFlag, trcLogFlag;
extern SFM_SysCommMsgType	*loc_sadb;
extern char	trcBuf[4096], trcTmp[1024];
extern char	mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN], sysLabel[COMM_MAX_NAME_LEN];

void ping_test (void);

/*****************************************************************************
    FUNCTION NAME   : Test_Lan ()
    PARAMETER       : 검사하고자 하는 인터페이스의 ip 주소
    RETURN VALUE    : 성공시 0, 실패시 1 
    DESCRIPTION     : 랜의 상태를 검사하는 루틴 
******************************************************************************/
#define 	NIPADDR(d) 		(d&0xff),((d>>8)&0xff),((d>>16)&0xff),((d>>24)&0xff)

int ping_test_with_ip (char *ipaddr)
{
	int sts,j;
	char 	result[16];

	j = 0;

	// try one
	sprintf(result, "%s\n",(pingtest(ipaddr, 0, 1,500000)>0)? "alive":"die");

	if (!memcmp (result, "alive", 5)) {
		sts = SFM_LAN_CONNECTED;
	} else {
		
		sts = SFM_LAN_DISCONNECTED;
		do {
			//commlib_microSleep(100000);
			sleep(1);
			sprintf(result, "%s\n",(pingtest(ipaddr, 0, 1,500000)>0)? "alive":"die");
			if (!memcmp (result, "alive", 5)) {
				sts = SFM_LAN_CONNECTED;
				break;
			}
			j++;
		} while(sts == SFM_LAN_DISCONNECTED && j != 2);
	}

	return sts;
}
/*
ping test : local lan check
			실패시 2.5초 대기..
 */
void ping_test (void)
{
	int i,j, rv;
	char 	ipaddr[16];
	char 	result[16];

	for (i=0; loc_sadb->loc_lan_sts[i].target_IPaddress != 0; i++) 
	{
		j = 0;
		sprintf (ipaddr, "%d.%d.%d.%d", NIPADDR(loc_sadb->loc_lan_sts[i].target_IPaddress));
		sprintf(result, "%s\n",(pingtest(ipaddr, 0, 1,500000)>0)? "alive":"die");
		//fprintf(stderr, "%s. %s\n", ipaddr, result);

		if (!memcmp (result, "alive", 5)) {
			loc_sadb->loc_lan_sts[i].status = SFM_LAN_CONNECTED;
		} else {

			sprintf(trcBuf,"[%s] pingtest fail.ip:%s (re-try:1)\n",__FUNCTION__,ipaddr );
			trclib_writeLogErr (FL,trcBuf);

			rv = SFM_LAN_DISCONNECTED;

			do {
				sleep(1);
				sprintf(result, "%s\n",(pingtest(ipaddr, 0, 1,500000)>0)? "alive":"die");
				if (!memcmp (result, "alive", 5)) {
					rv = SFM_LAN_CONNECTED;
					break;
				}else{
					sprintf(trcBuf,"[%s] pingtest fail.ip:%s (re-try:%d)\n",__FUNCTION__,ipaddr,j+2 );
					trclib_writeLogErr (FL,trcBuf);
				}
				j++;
			} while(j < 2);

			loc_sadb->loc_lan_sts[i].status = rv;
		}
		// 만약 핑이 안될경우 ping test 를 두번한다 
	}
}

