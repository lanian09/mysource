#include <stdio.h>

char	cronTmp[1024];
char	cronHead2[2048];

main ()
{
	memset (cronTmp, 0, 1024);
	memset (cronHead2, 0, 2048);

	printf ("[*] udr\n");
	
	sprintf(cronTmp, "    =============================================================================\n");
	strcat(cronHead2,cronTmp);
	sprintf(cronTmp, "    SVC_TYPE        TOTAL_UDR        START_UDR      INTERIM_UDR       INTERIM_URL\n");
	strcat(cronHead2,cronTmp);
	sprintf(cronTmp, "                     STOP_UDR         STOP_URL        TOTAL_URL           PPS_UDR\n");
	strcat(cronHead2,cronTmp);
	sprintf(cronTmp, "                      PPS_URL             GIFT              GET              POST\n");
	strcat(cronHead2,cronTmp);
	sprintf(cronTmp, "                          ETC     RESULT_IINFO   RESULT_SUCCESS   RESULT_REDIRECT\n");
	strcat(cronHead2,cronTmp);
	sprintf(cronTmp, "               RESULT_CLI_ERR   RESULT_SVR_ERR RESULT_TERM_ABRT   RESULT_SVR_ABRT\n");
	strcat(cronHead2,cronTmp);
	sprintf(cronTmp, "                  IP_UP_BYTES    IP_DOWN_BYTES   TCP_RE_UPBYTES  TCP_RE_DWN_BYTES\n");
	strcat(cronHead2,cronTmp);
	sprintf(cronTmp, "                COMPLETED_TXN       NORMAL_TXN     ABNORMAL_TXN\n");
	strcat(cronHead2,cronTmp);
	sprintf(cronTmp, "    =============================================================================\n");
	strcat(cronHead2,cronTmp);

	sprintf(cronTmp, "    %-8s %16s %16s %16s %16s\n", "xxx", "xxx", "xxx", "xxx", "xxx");
	strcat(cronHead2,cronTmp);
	sprintf(cronTmp, "    %8s %16s %16s %16s %16s\n", "", "xxx", "xxx", "xxx", "xxx");
	strcat(cronHead2,cronTmp);
	sprintf(cronTmp, "    %8s %16s %16s %16s %16s\n", "", "xxx", "xxx", "xxx", "xxx");
	strcat(cronHead2,cronTmp);
	sprintf(cronTmp, "    %8s %16s %16s %16s %16s\n", "", "xxx", "xxx", "xxx", "xxx");
	strcat(cronHead2,cronTmp);
	sprintf(cronTmp, "    %8s %16s %16s %16s %16s\n", "", "xxx", "xxx", "xxx", "xxx");
	strcat(cronHead2,cronTmp);
	sprintf(cronTmp, "    %8s %16s %16s %16s\n", "", "xxx", "xxx", "xxx");
	strcat(cronHead2,cronTmp);

	printf ("%s\n", cronHead2);

	memset (cronHead2, 0, 2048);
	sprintf(cronTmp, "    =============================================================================================\n");
	strcat(cronHead2,cronTmp);
	sprintf(cronTmp, "    STATTIME        SVC_TYPE        TOTAL_UDR        START_UDR      INTERIM_UDR       INTERIM_URL\n");
	strcat(cronHead2,cronTmp);
	sprintf(cronTmp, "                                     STOP_UDR         STOP_URL        TOTAL_URL           PPS_UDR\n");
	strcat(cronHead2,cronTmp);
	sprintf(cronTmp, "                                      PPS_URL             GIFT              GET              POST\n");
	strcat(cronHead2,cronTmp);
	sprintf(cronTmp, "                                          ETC     RESULT_IINFO   RESULT_SUCCESS   RESULT_REDIRECT\n");
	strcat(cronHead2,cronTmp);
	sprintf(cronTmp, "                               RESULT_CLI_ERR   RESULT_SVR_ERR RESULT_TERM_ABRT   RESULT_SVR_ABRT\n");
	strcat(cronHead2,cronTmp);
	sprintf(cronTmp, "                                  IP_UP_BYTES    IP_DOWN_BYTES   TCP_RE_UPBYTES  TCP_RE_DWN_BYTES\n");
	strcat(cronHead2,cronTmp);
	sprintf(cronTmp, "                                COMPLETED_TXN       NORMAL_TXN     ABNORMAL_TXN\n");
	strcat(cronHead2,cronTmp);
	sprintf(cronTmp, "    =============================================================================================\n");
	strcat(cronHead2,cronTmp);
	printf ("%s\n", cronHead2);

	puts ("");
}
