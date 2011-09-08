
#include "rcapd.h"
#include "mmc_hld.h"

/* Declaration of Global Variable */
char    resBuf[4096], resHead[4096], resTmp[1024];

/* Declaration of Extern Variable */



int mmc_prc_sample1 (IxpcQMsgType *rxIxpcMsg)
{
    printf ("cdelay_mmc_start_delay_check \n");
	return 1;
}

char *str_time(time_t t)
{
	static char mtime_str[81];

	strftime(mtime_str,80,"%Y-%m-%d %T",localtime(&t));
	return (char*)mtime_str;
}


