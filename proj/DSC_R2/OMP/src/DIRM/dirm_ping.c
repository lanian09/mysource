#include "dirm.h"

#define SA  struct sockaddr

extern int  	errno, trcFlag, trcLogFlag;
extern char	trcBuf[4096], trcTmp[1024];
extern char	mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN], sysLabel[COMM_MAX_NAME_LEN];

/*****************************************************************************
    FUNCTION NAME   : Test_Lan ()
    PARAMETER       : �˻��ϰ��� �ϴ� �������̽��� ip �ּ�
    RETURN VALUE    : ������ 0, ���н� 1 
    DESCRIPTION     : ���� ���¸� �˻��ϴ� ��ƾ 
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
