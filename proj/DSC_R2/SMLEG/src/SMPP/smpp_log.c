#include <smpp.h>

int smpp_log (int type, char *cmd_type, int smcid, int cid, SMS_HIS *sms_hist, SMPP_MSG_INFO *msg_info )
{
    FILE    *fp;
    time_t  now;
    int     len, slen;
    char    tims[100], fdir[256], temp[1024];
	char    *env;

    now = time(&now);
	if ((env = getenv(IV_HOME)) == NULL) {
        fprintf(stderr,"[smpp_log] not found %s environment name\n", IV_HOME);
        return -1;
    }

    sprintf(fdir, "%s/%s", env, SMPP_INTERFACE_LOG);
    strftime(tims, 100, "%y%m%d%H", localtime(&now));

    len = strlen(fdir);
    sprintf(&fdir[len], "%s", tims);

    /* 파일 개방*/
    fp = fopen(fdir, "a");
    if (fp == NULL) {
        printf("smpp log file open error %s", fdir);
        return -1;
    }

    strftime(tims, 100, "%T", localtime(&now));
	sprintf(temp, "[%s]", tims);
	slen = strlen(temp);

	if (type == SEND_TO_SMSC) {
		sprintf(&temp[slen], "SEND %s %d %d %d %s %s [%s]"
				, cmd_type
				, sms_hist->cid
				, smcid
				, sms_hist->cid
				, sms_hist->info.subsID
				, msg_info->callback
				, sms_hist->info.smsMsg);
	}
	else if (type == RCV_FROM_SMSC) {
		sprintf(&temp[slen], "ACK %d %d", sms_hist->cid, smcid);
	}

    fprintf(fp, "%s", temp);
    fflush(fp);
    fclose(fp);
	return 0;
}

