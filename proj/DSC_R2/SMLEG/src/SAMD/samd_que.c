#include "samd.h"
#include "commlib.h"
#include "sfm_msgtypes.h"

#include <sys/msg.h>

extern char		trcBuf[4096];
extern char		trcTmp[1024];

extern int		trcFlag;
extern int		trcLogFlag;
extern int		queCNT;

extern SFM_SysCommMsgType		*loc_sadb;
extern STM_LoadMPStatMsgType	system_statistic;

int que_cmp(const void *a, const void *b);
int get_queUsage(void);


/*  차순 */
int que_cmp(const void *a, const void *b)
{
#if 0	/*	041007.lndb.cjs	*/
	return ( ((SFM_SysCommQueSts *)a)->qID >= ((SFM_SysCommQueSts *)b)->qID ? 1 : -1 );
#endif
	return (int)(((SFM_SysCommQueSts *)a)->qID - ((SFM_SysCommQueSts *)b)->qID);
}

int get_queUsage(void)
{
	int				i, qId, num;
	static struct	msqid_ds queCntrlBuf;

	for(num	= 0, i = 0; i < queCNT; i++)
	{
		if(!strcasecmp(loc_sadb->loc_que_sts[i].qNAME, "CM") || !strcasecmp(loc_sadb->loc_que_sts[i].qNAME, "SMSERVER"))
			continue;

		if( (qId = msgget(loc_sadb->loc_que_sts[i].qKEY, 0)) < 0)
		{
#if 0
			sprintf(trcBuf,"[%s] fail msgget(qname[%s], qKEY[%d]) - err[%d]:%s\n",
				__FUNCTION__, loc_sadb->loc_que_sts[i].qNAME, loc_sadb->loc_que_sts[i].qKEY, errno, strerror(errno));
			trclib_writeLogErr(FL, trcBuf);
#endif
			loc_sadb->loc_que_sts[i].qID = 0;
			continue;
		}

		/*	get MSGQ control data	*/
		if(msgctl(qId, IPC_STAT, &queCntrlBuf) < 0)
		{
			sprintf(trcBuf,"%s: fail msgctl ; err=%d(%s)\n",
				__FUNCTION__, errno, strerror(errno));
			trclib_writeLogErr(FL, trcBuf);
			continue;
		}

		loc_sadb->loc_que_sts[i].qID	= qId;
		loc_sadb->loc_que_sts[i].qNUM	= (int)queCntrlBuf.msg_qnum;
		loc_sadb->loc_que_sts[i].cBYTES	= (int)queCntrlBuf.msg_cbytes/1024;
		loc_sadb->loc_que_sts[i].qBYTES	= (int)queCntrlBuf.msg_qbytes/1024;
		num++;
	}
#if 0
	//fprintf(stderr,"\n\n");
#endif
	qsort(&loc_sadb->loc_que_sts[0], queCNT, sizeof(SFM_SysCommQueSts), que_cmp);

#if 0
	/*	실제 존재하는 que만	*/
	loc_sadb->queCount = num;
	fprintf(stderr,"2222222 queCNT: %d %d\n", queCNT, loc_sadb->queCount);
#endif

	return 1;
}
