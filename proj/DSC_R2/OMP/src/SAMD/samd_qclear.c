#include "samd.h"

extern int		trcFlag, trcLogFlag;
extern char		trcBuf[4096], trcTmp[1024];
extern SAMD_ProcessInfo		ProcessInfo[SYSCONF_MAX_APPL_NUM];
extern SFM_SysCommMsgType	*loc_sadb;
extern T_keepalive			*keepalive;



void QClear ()
{
	int		i, qId, now;
	struct msqid_ds queCntrlBuf;
	char	msgBuff[65536];

	/* check all mesg Queue */
	for (i=0; i < loc_sadb->processCount; i++)
	{
		if (!ProcessInfo[i].msgQkey) continue;

		/* get one MSGQ Id */
		if ((qId = msgget (ProcessInfo[i].msgQkey, 0)) < 0) {
			if (errno != ENOENT && errno != EACCES) {
				sprintf(trcBuf,"[samd_qclear] msgget error; proc=%s, key=%x, err=%d(%s)\n",
						ProcessInfo[i].procName, ProcessInfo[i].msgQkey, errno, strerror(errno));
				trclib_writeLogErr (FL,trcBuf);
			}
			continue;
		}
		/* get MSGQ control data */
		if (msgctl (qId, IPC_STAT, &queCntrlBuf) < 0) {
			sprintf(trcBuf,"[samd_qclear] msgctl error; proc=%s, key=%x, err=%d(%s)\n",
					ProcessInfo[i].procName, ProcessInfo[i].msgQkey, errno, strerror(errno));
			trclib_writeLogErr (FL,trcBuf);
			continue;
		}

		now = time(0);

		if (((now - queCntrlBuf.msg_rtime) >= MSGQ_CLEAR_TIME) &&
			(queCntrlBuf.msg_qnum >= 3))
		{
			// queue ���¸� Ȯ���ϴ� �ֱ����� �ð��� ���� ���μ����� queue���� �޽�����
			//  �о�� �ð����� ����ȭ �۾��� �ϱ� �����Ƿ�,
			// - ������ �׿��ִٰ� ������ �о�� ������ �ʵȴ�.
			// - �޽����� �׿��ְ�, �ش� ���μ����� �׾��ְų�, keepalive ���ۿ��� �̻���
			//  �ִٰ� �Ǵܵɶ��� ������ �о��� �Ѵ�.
			//
			if ((ProcessInfo[i].pid < 1) || // �׾� ������
				(keepalive->retry[i] > 5))  // 5���̻� keepalive ���ۿ� �̻��� ������
			{
				/* clear queue */
				while (msgrcv (qId, (char *)&msgBuff, sizeof(msgBuff), 0, IPC_NOWAIT) >= 0);

				sprintf(trcBuf,"[samd_qclear] clear message; proc=%s, key=%x, msgCnt=%d, diff_time=%d, retry=%d\n",
						ProcessInfo[i].procName, ProcessInfo[i].msgQkey, 
						         (int)queCntrlBuf.msg_qnum, (int)now-(int)queCntrlBuf.msg_rtime, keepalive->retry[i]);
				trclib_writeLogErr (FL,trcBuf);
			}
		}
	}
}
