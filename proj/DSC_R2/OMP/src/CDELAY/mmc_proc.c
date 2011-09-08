
#include "mmc_hld.h"


extern int mmcdQid;
extern int g_cont_flag;

char    resBuf[4096], resHead[4096], resTmp[1024];


int cdelay_mmc_start_delay_check (IxpcQMsgType *rxIxpcMsg)
{

	int		seqNo=1;

	g_cont_flag = 1;

    printf ("cdelay_mmc_start_delay_check %d\n", g_cont_flag);


	snprintf (resBuf, sizeof(resBuf), "cdelay_mmc_start_delay_check ok\n");

	comm_txMMLResult (mmcdQid, rxIxpcMsg, resBuf, 0, 0, 0, 0, seqNo++);

	return 1;
}


int cdelay_mmc_stop_delay_check (IxpcQMsgType *rxIxpcMsg)
{
    printf ("cdelay_mmc_start_delay_check \n");
	g_cont_flag = 0;
	return 1;
}
