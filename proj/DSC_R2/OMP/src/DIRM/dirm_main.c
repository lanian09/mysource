#include "dirm.h"

#include <sys/signal.h>
#include <sys/msg.h>
#include <sys/wait.h>

int		dirmQID, ixpcQID, mcdmQID;
char	*iv_home, l_sysconf[256];
char    trcBuf[4096], trcTmp[1024];
char	mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN], sysLabel[COMM_MAX_NAME_LEN];
char	l3pdFileName[256];

SFM_SysCommMsgType              *loc_sadb;

SFM_L3PD        *l3pd;
extern int	trcFlag, trcLogFlag;
extern T_keepalive		*keepalive;
extern int set_TAP_Info_init(void);
extern int get_TAP_Info_main(void);
extern void SetUpSignal(void);
extern void UserControlledSignal(int sign);
extern void IgnoreSignal(int sign);

/*******************************************************************************
    
*******************************************************************************/
int main()
{
	setbuf(stdout,NULL);

	if (InitSys() < 0 )
		exit(1);
	
	SetUpSignal();

	{ // clear previous messages
		char	tmpBuff[65536];
		while (msgrcv (dirmQID, tmpBuff, sizeof(tmpBuff), 0, IPC_NOWAIT) > 0);
	}

	if(set_TAP_Info_init() < 0)
		exit(0);

	while (1)
	{
		get_TAP_Info_main();
		keepalivelib_increase();
		sleep(2);
	}
}

