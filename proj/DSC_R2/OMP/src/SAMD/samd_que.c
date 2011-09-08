#include "samd.h"

extern int		trcFlag, trcLogFlag, queCNT;
extern char		trcBuf[4096], trcTmp[1024];
extern SFM_SysCommMsgType *loc_sadb;
extern  STM_LoadOMPStatMsgType    system_statistic;

/*  차순 */
int que_cmp (const void *a, const void *b)
{
//	return ( ((SFM_SysCommQueSts *)a)->qID >= ((SFM_SysCommQueSts *)b)->qID ? 1 : -1 );
//  041007.lndb.cjs 
	return (int)(((SFM_SysCommQueSts *)a)->qID - ((SFM_SysCommQueSts *)b)->qID);
}


int get_queUsage (void)
{
	int i,qId,num=0;
    static struct msqid_ds queCntrlBuf;


	for(i=0 ; i< queCNT ; i++) {
    	if ((qId = msgget (loc_sadb->loc_que_sts[i].qKEY, 0)) < 0) {
        	continue;
    	}

    	/* get MSGQ control data */
    	if (msgctl (qId, IPC_STAT, &queCntrlBuf) < 0){
    		sprintf(trcBuf,"[get_queUsage] fail msgctl ; err=%d(%s)\n",
    			errno, strerror(errno));
    		trclib_writeLogErr (FL,trcBuf);
        	continue;
    	}
		loc_sadb->loc_que_sts[i].qID = qId;	
		loc_sadb->loc_que_sts[i].qNUM = (int) queCntrlBuf.msg_qnum;	
		loc_sadb->loc_que_sts[i].cBYTES = (int) queCntrlBuf.msg_cbytes/1000;	
		loc_sadb->loc_que_sts[i].qBYTES = (int) queCntrlBuf.msg_qbytes/1000;	
		num++;
	}

	qsort ( &loc_sadb->loc_que_sts[0], queCNT, sizeof(SFM_SysCommQueSts), que_cmp );

	/* 실제 존재하는 que만 */
	loc_sadb->queCount = num;

	return 1;
}








