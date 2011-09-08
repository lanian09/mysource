#include "samd.h"

extern int		trcFlag, trcLogFlag;
extern char		trcBuf[TRCBUF_LEN], trcTmp[TRCTMP_LEN];
extern char     iv_home[64];

extern SAMD_ProcessInfo		ProcessInfo[SYSCONF_MAX_APPL_NUM];
extern SAMD_AuxilaryInfo   AuxilaryInfo[SYSCONF_MAX_AUXILARY_MSGQ_NUM];

extern int appCnt, auxilaryCnt;

extern SFM_SysCommMsgType	*loc_sadb;
extern T_keepalive			*keepalive;

void QClear(void)
{
	int				i, j, qId, clear_qcnt;
	char			msgBuff[65536], fileName[256], filepath[256], cur_time[20], mySysName[COMM_MAX_NAME_LEN];
	char			*env;
	FILE			*fp;
	time_t			msgq_now, now;
	struct tm		*pLocalTime;
	struct msqid_ds	queCntrlBuf;

	time(&now);

	if( (env = getenv(MY_SYS_NAME)) == NULL)
	{
		sprintf(trcBuf, "%s: not found %s environment name\n", __FUNCTION__, MY_SYS_NAME);
		trclib_writeLogErr(FL, trcBuf);
		return;
	}
	strcpy(mySysName, env);

	/* check all mesg Queue */
	for(i = 0; i < appCnt; i++)
	{
		for(j = 0; j < SYSCONF_MAX_APPL_MSGQ_NUM; j++)
		{
			if(!ProcessInfo[i].msgQkey)
				continue;
			/*	get one MSGQ Id	*/
			if( (qId = msgget(ProcessInfo[i].msgQkey, 0)) < 0)
			{
				if( (errno != ENOENT) && (errno != EACCES))
				{
					sprintf(trcBuf, "%s: msgget error; proc=%s, key=%x, err=%d(%s)\n",
						__FUNCTION__, ProcessInfo[i].procName, ProcessInfo[i].msgQkey, errno, strerror(errno));
					trclib_writeLogErr(FL, trcBuf);
				}
				continue;
			}

			/*	get MSGQ control data	*/
			if(msgctl(qId, IPC_STAT, &queCntrlBuf) < 0)
			{
				sprintf(trcBuf, "%s: msgctl error; proc=%s, key=%x, err=%d(%s)\n",
					__FUNCTION__, ProcessInfo[i].procName, ProcessInfo[i].msgQkey, errno, strerror(errno));
				trclib_writeLogErr(FL, trcBuf);
				continue;
			}

			if( (((now - queCntrlBuf.msg_rtime) >= MSGQ_CLEAR_TIME) && (queCntrlBuf.msg_qnum >= 3)) || (queCntrlBuf.msg_qnum >= MSGQ_CLEAR_CNT))
			{
				/*	add by helca 2007.07.05	*/
				if( ((long)ProcessInfo[i].pid < 1) || /* 죽어 있을때 */
					(keepalive->retry[i] > 10))  /* 10초이상 keepalive 동작에 이상이 있을때 */
				{
					/*	clear queue	*/
					while (msgrcv(qId, (char*)&msgBuff, sizeof(msgBuff), 0, IPC_NOWAIT) >= 0);

					sprintf(trcBuf, "%s: clear message; proc=%s, key=%x, msgCnt=%d, retry_cnt:%d\n",
						__FUNCTION__, ProcessInfo[i].procName, ProcessInfo[i].msgQkey, (int)queCntrlBuf.msg_qnum, keepalive->retry[i]);

					trclib_writeLogErr(FL, trcBuf);
				}
				else if(queCntrlBuf.msg_qnum >= MSGQ_CLEAR_CNT)
				{
					clear_qcnt = queCntrlBuf.msg_qnum;

					/* clear queue backup */
					time(&msgq_now);
					pLocalTime = (struct tm*)localtime((time_t*)&now);
					strftime(cur_time, 20, "%Y%m%d%H%M%S", pLocalTime);

					sprintf(fileName, "%s_%s_%s.MSGQ", mySysName, cur_time, ProcessInfo[i].procName);
					sprintf(filepath, "%s/LOG/MSGQ_CLEAR/%s", iv_home, fileName);

					if( ( fp = fopen(filepath, "w+")) == NULL)
					{
						sprintf(trcBuf, "%s: fopen(filename[%s]) fail \n", __FUNCTION__, filepath);
						trclib_writeLogErr(FL, trcBuf);
						return;
					}

					/* clear queue */
					while(msgrcv(qId, (char*)&msgBuff, sizeof(msgBuff), 0, IPC_NOWAIT) >= 0)
						//fprintf(fp, "%s\n", msgBuff);

					sprintf(trcBuf, "%s: msgQ count over; proc=%s, key=%x, msgCnt=%d\n",
						__FUNCTION__, ProcessInfo[i].procName, ProcessInfo[i].msgQkey, (int)queCntrlBuf.msg_qnum);
					trclib_writeLogErr(FL, trcBuf);

					fclose(fp);

					/* msgQ clear message send */
					sendQueClearMsg2COND(i, clear_qcnt);
				}
			}
		}
	}

	/*	check all mesg Queue	*/
	for(i = 0; i < auxilaryCnt; i++)
	{
		for(j = 0; j < SYSCONF_MAX_APPL_MSGQ_NUM; j++)
		{
			if(!AuxilaryInfo[i].msgQkey)
				continue;
			/*	get one MSGQ Id	*/
			if( (qId = msgget (AuxilaryInfo[i].msgQkey, 0)) < 0)
			{
				if( (errno != ENOENT) && (errno != EACCES))
				{
				#if 0
					sprintf(trcBuf, "[samd_qclear] msgget error; proc=%s, key=%x, err=%d(%s)\n",
						AuxilaryInfo[i].procName, AuxilaryInfo[i].msgQkey[j], errno, strerror(errno));
				#endif
					sprintf(trcBuf, "%s: msgget error; proc=%s, key=%x, err=%d(%s)\n",
						__FUNCTION__, AuxilaryInfo[i].procName, AuxilaryInfo[i].msgQkey, errno, strerror(errno));
					trclib_writeLogErr(FL, trcBuf);
				}
				continue;
			}

			/*	get MSGQ control data	*/
			if(msgctl(qId, IPC_STAT, &queCntrlBuf) < 0)
			{
			#if 0
				sprintf(trcBuf,"[samd_qclear] msgctl error; proc=%s, key=%x, err=%d(%s)\n",
					AuxilaryInfo[i].procName, AuxilaryInfo[i].msgQkey[j], errno, strerror(errno));
			#endif
				sprintf(trcBuf, "%s: msgctl error; proc=%s, key=%x, err=%d(%s)\n",
					__FUNCTION__, AuxilaryInfo[i].procName, AuxilaryInfo[i].msgQkey, errno, strerror(errno));
				trclib_writeLogErr(FL, trcBuf);
				continue;
			}

			if( ((now - queCntrlBuf.msg_rtime) >= MSGQ_CLEAR_TIME) && (queCntrlBuf.msg_qnum >= 3))
			{
				/*	clear queue	*/
				while(msgrcv(qId, (char*)&msgBuff, sizeof(msgBuff), 0, IPC_NOWAIT) >= 0);
			#if 0
				fprintf(stdout, "jean ++++++ qclear~! %d %x\n", qId, qId);
				sprintf(trcBuf,"[samd_qclear] clear message; proc=%s, key=%x, msgCnt=%d\n",
					AuxilaryInfo[i].procName, AuxilaryInfo[i].msgQkey[j], (int)queCntrlBuf.msg_qnum);
			#endif
				sprintf(trcBuf, "%s: clear message; proc=%s, key=%x, msgCnt=%d\n",
					__FUNCTION__, AuxilaryInfo[i].procName, AuxilaryInfo[i].msgQkey, (int)queCntrlBuf.msg_qnum);
				trclib_writeLogErr(FL, trcBuf);
			}
		}
	}
}
