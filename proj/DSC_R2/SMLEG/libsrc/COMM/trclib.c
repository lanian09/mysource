#include "trclib.h"

int		trcFlag=0, trcLogFlag=0, trcOmdFlag=0, trcLogId, trcErrLogId;
time_t	trcStartTime=0, trcLogStartTime=0;
FILE	*trcFp=NULL,*trcOmdFp=NULL;



/*------------------------------------------------------------------------------
* trcFlag�� �ɷ� ������ ��ϵ� tty�� ����Ѵ�.
* - trcFlag�� �ɷ�������, fp�� NULL�̸� ������ trcFlag�� �����Ѵ�.
* - trcFlag�� ����� tty�� close�Ǹ� write fail�� �߻��Ѵ�.
* - write fail�� ��� trcFlag ������ fp�� close�Ѵ�.
* - trcFlag�� �ɸ� �� �����ð��� �������� �ڵ� �����ȴ�.
------------------------------------------------------------------------------*/
int trclib_printOut (char *fName, int lNum, char *msg)
{
	int 	len;
	char	tmp[32];
	struct timeval	curr;

	if (!trcFlag)
		return 0;

	gettimeofday (&curr, NULL);

	if (trcFp == NULL) {
		trcFlag = 0;
		return -1;
	}

	strftime(tmp, 32, "%m-%d %T", localtime((time_t*)&curr.tv_sec));

	if ((len = fprintf (trcFp, "[%s.%03ld] %s", tmp, (curr.tv_usec/1000), msg)) < 0) {
		trcFlag = 0;
		trcFp = NULL;
		logPrint (trcErrLogId,fName,lNum,"  TRACE OFF AUTOMATICALLY (write fail)\n\n");
		return -1;
	}

#ifdef TRC_TIMER
	if ((curr.tv_sec - trcStartTime) >= TRCLIB_MAX_TRC_TIMER) {
		logPrint (trcErrLogId,fName,lNum,"  TRACE OFF AUTOMATICALLY (due to time limit)\n\n");
		if (trcFp != NULL) {
			fprintf(trcFp,"  TRACE OFF AUTOMATICALLY (due to time limit)\n\n");
		}
		trcFlag = 0;
		trcFp = NULL;
	}
#endif /*TRC_TIMER*/

	return len;

} /** End of trclib_printOut **/



/*------------------------------------------------------------------------------
* - trcLogFlag�� setting�Ǿ� ������ log file�� wrtie�ϰ�,
* - trcFlag�� �ɷ� ������ ��ϵ� tty�� ����Ѵ�.
* - trcLogFlag�� �ɸ� �� �����ð��� �������� �ڵ� �����ȴ�.
------------------------------------------------------------------------------*/
int trclib_writeLog (char *fName, int lNum, char *msg)
{
	//time_t	now;

	if (trcLogFlag) {
		logPrint (trcLogId,fName,lNum,"%s", msg);
	}
	if (trcFlag) {
		trclib_printOut(fName,lNum,msg);
	}

#ifdef TRC_TIMER
	if (trcLogFlag) {
		now = time(0);
		if ((now - trcLogStartTime) >= TRCLIB_MAX_TRC_TIMER) {
			trcLogFlag = 0;
			logPrint (trcLogId,fName,lNum,FL,"  MSG_LOG OFF AUTOMATICALLY (due to time limit)\n\n");
			if (trcFp != NULL) {
				fprintf(trcFp,"  MSG_LOG OFF AUTOMATICALLY (due to time limit)\n\n");
			}
		}
	}
#endif /*TRC_TIMER*/

	return 1;

} /** End of trclib_writeLog **/



/*------------------------------------------------------------------------------
* - error file�� ������ write�ϰ�,
* - trcFlag�� �ɷ� ������ ��ϵ� tty�� ����Ѵ�.
------------------------------------------------------------------------------*/
int trclib_writeErr (char *fName, int lNum, char *msg)
{
	logPrint (trcErrLogId,fName,lNum,"%s", msg);
	if (trcFlag) {
		trclib_printOut(fName,lNum,msg);
	}
	return 1;
} /** End of trclib_writeErr **/



/*------------------------------------------------------------------------------
* - error file�� ������ write�Ѵ�.
* - trcLogFlag�� setting�Ǿ� ������ log file���� �Բ� wrtie�ϰ�,
* - trace�� �ɷ� ������ ��ϵ� tty�� ����Ѵ�.
------------------------------------------------------------------------------*/
int trclib_writeLogErr (char *fName, int lNum, char *msg)
{
    logPrint (trcErrLogId,fName,lNum,"%s", (msg));
	if (trcLogFlag) {
    	trclib_writeLog (fName,lNum,msg);
	}
	return 1;
} /** End of trclib_writeLogErr **/



/*------------------------------------------------------------------------------
------------------------------------------------------------------------------*/
int trclib_exeSetPrintMsg (TrcLibSetPrintMsgType *msg)
{
	char	trcBuf[256];

	/* setprint -t on/off
	* -> trace message ��� ��� on/off
	*/
	if (msg->trcFlag.pres) {
		if (msg->trcFlag.octet) {
			/* trace on
			* - trace�� ����� tty device name ���޵ǰ� �̸� open�Ͽ� �ش� device��
			*	write�����ν� �޽����� ����Ѵ�.
			* - trcFlag�� setting�ϰ�, �����ð� ��� �� �ڵ� off �ɼ� �ֵ��� ����ð���
			*	trcStartTime���� �����Ѵ�.
			*/
			if ((trcFp = fopen(msg->trcDeviceName,"w")) == NULL) {
				sprintf(trcBuf,"  TRACE DEVICE OPEN FAIL; dev=%s\n\n", msg->trcDeviceName);
			} else {
				sprintf(trcBuf,"  TRACE ON\n\n");
				trcStartTime = time(0);
				trcFlag = (int)msg->trcFlag.octet;
			}
			trclib_writeLog(FL,trcBuf);
		} else {
			/* trace off
			* - open�� device�� fp�� close�ϰ� trcFlag�� clear�Ѵ�.
			*/
			sprintf(trcBuf,"  TRACE OFF\n\n");
			trclib_writeLog(FL,trcBuf);
			if (trcFp != NULL) {
				fclose(trcFp);
			}
			trcFlag = 0;
		}
	}

	/* setprint -l on/off
	* -> trace log message�� log file�� ����� ��� on/off
	*/
	if (msg->trcLogFlag.pres) {
		if (msg->trcLogFlag.octet) {
			/* log on
			* - trcLogFlag�� setting�ϰ�, �����ð� ��� �� �ڵ� off �ɼ� �ֵ��� ����ð���
			*	trcLogStartTime���� �����Ѵ�.
			*/
			trcLogFlag = (int)msg->trcLogFlag.octet;
			trcLogStartTime = time(0);
			sprintf(trcBuf,"  MSG_LOG ON\n\n");
			trclib_writeLog(FL,trcBuf);
		} else {
			sprintf(trcBuf,"  MSG_LOG OFF\n\n");
			trclib_writeLog(FL,trcBuf);
			trcLogFlag = 0;
		}
	}

	return 1;

} /** End of trclib_exeSetPrintMsg **/

/*------------------------------------------------------------------------------
------------------------------------------------------------------------------
int trclib_exeOmdSysMsg (TrcLibSetPrintMsgType *msg)
{
	char	trcBuf[256];

	// setprint -t on/off
	// -> trace message ��� ��� on/off
	if (msg->trcFlag.pres) {
		if (msg->trcFlag.octet) {
			// omdsys on
			// - trace�� ����� tty device name ���޵ǰ� �̸� open�Ͽ� �ش� device��
			//	write�����ν� �޽����� ����Ѵ�.
			// - trcFlag�� setting�ϰ�, �����ð� ��� �� �ڵ� off �ɼ� �ֵ��� ����ð���
			//	trcStartTime���� �����Ѵ�.
			if ((trcOmdFp = fopen(msg->trcDeviceName,"w")) == NULL) {
				sprintf(trcBuf,"  OMDSYS DEVICE OPEN FAIL; dev=%s\n\n", msg->trcDeviceName);
			} else {
				sprintf(trcBuf,"  OMDSYS ON\n\n");
				trcStartTime = time(0);
				trcOmdFlag = (int)msg->trcFlag.octet;
			}
			trclib_writeLog(FL,trcBuf);
		} else {
			// omdsys off
			// - open�� device�� fp�� close�ϰ� trcOmdFlag�� clear�Ѵ�.
			sprintf(trcBuf,"  OMDSYS OFF\n\n");
			trclib_writeLog(FL,trcBuf);
			if (trcOmdFp != NULL) {
				fclose(trcOmdFp);
			}
			trcOmdFlag = 0;
		}
	}

	return 1;

} // End of trclib_exeOmdSysMsg 
*/

/*------------------------------------------------------------------------------
* - trcOmdFlag�� �ɷ� ������ ��ϵ� tty�� ����Ѵ�.
------------------------------------------------------------------------------
int trclib_writeOmdSys (char *fName, int lNum, char *msg)
{
    time_t  now;
    int     len;
    char    tmp[32];
    struct timeval  curr;

    if (!trcOmdFlag)
        return 0;

    gettimeofday (&curr, NULL);

    if (trcOmdFp == NULL) {
        trcOmdFlag = 0;
        return -1;
    }

    strftime(tmp, 32, "%m-%d %T", localtime((time_t*)&curr.tv_sec));

    if ((len = fprintf (trcOmdFp, "[%s.%03d] %s", tmp, (curr.tv_usec/1000), msg)) < 0) {
        trcOmdFlag = 0;
        trcOmdFp = NULL;
        logPrint (trcErrLogId,fName,lNum,"  OMDSYS OFF AUTOMATICALLY (write fail)\n\n");
        return -1;
    }

#ifdef TRC_TIMER
    if ((curr.tv_sec - trcStartTime) >= TRCLIB_MAX_TRC_TIMER) {
        logPrint (trcErrLogId,fName,lNum,"  OMDSYS OFF AUTOMATICALLY (due to time limit)\n\n");
        if (trcOmdFp != NULL) {
            fprintf(trcFp,"  OMDSYS OFF AUTOMATICALLY (due to time limit)\n\n");
        }
        trcOmdFlag = 0;
        trcOmdFp = NULL;
    }
#endif 

    return 1;

} // End of trclib_writeOmdSys 
*/

