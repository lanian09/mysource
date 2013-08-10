#include "samd.h"

extern char		trcBuf[TRCBUF_LEN];
extern char		trcTmp[TRCTMP_LEN];

extern int		statistic_cnt;
extern int		trcFlag;
extern int		trcLogFlag;
extern int		ixpcQID;

extern SFM_SysCommMsgType		*loc_sadb;
extern STM_LoadMPStatMsgType	system_statistic;

int HandleStatistics(GeneralQMsgType *rxGenQMsg);

int HandleStatistics(GeneralQMsgType *rxGenQMsg)
{
	IxpcQMsgType	*rxIxpcMsg;

	GeneralQMsgType	txGenQMsg;
	IxpcQMsgType	*txIxpcMsg;
	int				i, txLen;

	rxIxpcMsg		= (IxpcQMsgType*)rxGenQMsg->body;
	txIxpcMsg		= (IxpcQMsgType*)txGenQMsg.body;
	txGenQMsg.mtype	= MTYPE_STATISTICS_REPORT;

	strcpy(txIxpcMsg->head.srcSysName, rxIxpcMsg->head.dstSysName);
	strcpy(txIxpcMsg->head.srcAppName, rxIxpcMsg->head.dstAppName);
	strcpy(txIxpcMsg->head.dstSysName, rxIxpcMsg->head.srcSysName);
	strcpy(txIxpcMsg->head.dstAppName, rxIxpcMsg->head.srcAppName);

	txIxpcMsg->head.msgId	= MSGID_LOAD_STATISTICS_REPORT;
	txIxpcMsg->head.bodyLen	= sizeof(STM_LoadMPStatMsgType);
	txLen = sizeof(txIxpcMsg->head) + txIxpcMsg->head.bodyLen;

	////// by helca 07.31  /////////
	system_statistic.comminfo.cpu_cnt = loc_sadb->cpuCount;
	if(system_statistic.comminfo.cpu_cnt == 0)
	{
		sprintf(trcBuf, "%s: statistic_cnt is 0\n", __FUNCTION__);
		trclib_writeLog(FL, trcBuf);
		return -1;
	}

	for(i = 0; i < system_statistic.comminfo.cpu_cnt; i++)
	{
		system_statistic.comminfo.max_cpu[i] = htonl(system_statistic.comminfo.max_cpu[i]);
		if(system_statistic.comminfo.average_cpu[i] == 0)
			system_statistic.comminfo.average_cpu[i] = 0;
		else
			system_statistic.comminfo.average_cpu[i] = htonl(system_statistic.comminfo.average_cpu[i] / statistic_cnt);
	}

	system_statistic.comminfo.max_mem = htonl(system_statistic.comminfo.max_mem);
	if(system_statistic.comminfo.avg_mem == 0)
		system_statistic.comminfo.avg_mem = 0;
	else
		system_statistic.comminfo.avg_mem = htonl(system_statistic.comminfo.avg_mem / statistic_cnt);

	//	system_statistic.max_disk = htonl(system_statistic.max_disk);
	// by helca 09.11
	if(system_statistic.max_disk == 0)
		system_statistic.max_disk = 0;
	else
		system_statistic.max_disk = htonl(system_statistic.max_disk / statistic_cnt);

	if(system_statistic.avg_disk == 0)
		system_statistic.avg_disk = 0;
	else
		system_statistic.avg_disk = htonl(system_statistic.avg_disk / statistic_cnt);

	system_statistic.max_msgQ = htonl(system_statistic.max_msgQ);

	if(system_statistic.avg_msgQ == 0)
		system_statistic.avg_msgQ = 0;
	else
		system_statistic.avg_msgQ = htonl(system_statistic.avg_msgQ / statistic_cnt);

	system_statistic.max_sess = htonl(system_statistic.max_sess);
	if(system_statistic.avg_sess == 0)
		system_statistic.avg_sess = 0;
	else
		system_statistic.avg_sess = htonl(system_statistic.avg_sess / statistic_cnt);

	/////////////////////
	if(trcFlag || trcLogFlag)
	{
		sprintf(trcBuf, " avg_disk[%d], max_disk[%d], avg_msgQ[%d], max_msgQ[%d], avg_sess[%d], max_sess[%d], avr_mem[%d], max_mem[%d], cpu_cnt[%d], avr_cpu0[%d], max_cpu0[%d], avr_cpu1[%d], max_cpu1[%d]\n",
			system_statistic.avg_disk, system_statistic.max_disk,
			system_statistic.avg_msgQ, system_statistic.max_msgQ, system_statistic.avg_sess, system_statistic.max_sess,
			system_statistic.comminfo.avg_mem, system_statistic.comminfo.max_mem, system_statistic.comminfo.cpu_cnt,
			system_statistic.comminfo.average_cpu[0], system_statistic.comminfo.max_cpu[0],
			system_statistic.comminfo.average_cpu[1], system_statistic.comminfo.max_cpu[1] );
		trclib_writeLog(FL, trcBuf);
	}

	memcpy (txIxpcMsg->body, (char*)&system_statistic, sizeof(STM_LoadMPStatMsgType));
	statistic_cnt = 0;

	if(msgsnd(ixpcQID, (void*)&txGenQMsg, txLen, IPC_NOWAIT) < 0)
	{
		sprintf(trcBuf,"%s: SEND FAIL system_statistic = %s\n", __FUNCTION__, strerror(errno));
		trclib_writeLogErr (FL, trcBuf);
		return -1;
	}
	else
		memset(&system_statistic, 0x00, sizeof(STM_LoadMPStatMsgType));

	return 1;
}
