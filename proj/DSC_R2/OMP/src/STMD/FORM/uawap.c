#include <stdio.h>

char	cronTmp[1024];
char	cronHead2[2048];

main ()
{
	memset (cronTmp, 0, 1024);
	memset (cronHead2, 0, 2048);

	printf ("[*] uawap\n");
	
	sprintf(cronTmp, "    =============================================================================\n");
	strcat(cronHead2,cronTmp);
	sprintf(cronTmp, "         TOT_EXT_LOG TOT_EXT_DEC_ERR_LOG    NO_IPPOOL_WAP_LOG         TOT_WAP_LOG\n");
	strcat(cronHead2,cronTmp);
	sprintf(cronTmp, "         TOT_SUCCESS            TOT_FAIL              NO_CALL             TX_FAIL\n");
	strcat(cronHead2,cronTmp);
	sprintf(cronTmp, "            ETC_FAIL            COMPLETE              TIMEOUT           CANCELLED\n");
	strcat(cronHead2,cronTmp);
	sprintf(cronTmp, "         TOT_UP_BYTE         TOT_DN_BYTE      NO_CALL_UP_BYTE     NO_CALL_DN_BYTE\n");
	strcat(cronHead2,cronTmp);
	sprintf(cronTmp, "    ETC_FAIL_UP_BYTE    ETC_FAIL_DN_BYTE       GET_METHOD_CNT     POST_METHOD_CNT\n");
	strcat(cronHead2,cronTmp);
	sprintf(cronTmp, "      ETC_METHOD_CNT\n");
	strcat(cronHead2,cronTmp);
	sprintf(cronTmp, "    =============================================================================\n");
	strcat(cronHead2,cronTmp);

	sprintf(cronTmp, "    %16s %19s %20s %19s\n", "xxx", "xxx", "xxx", "xxx");
	strcat(cronHead2,cronTmp);
	sprintf(cronTmp, "    %16s %19s %20s %19s\n", "xxx", "xxx", "xxx", "xxx");
	strcat(cronHead2,cronTmp);
	sprintf(cronTmp, "    %16s %19s %20s %19s\n", "xxx", "xxx", "xxx", "xxx");
	strcat(cronHead2,cronTmp);
	sprintf(cronTmp, "    %16s %19s %20s %19s\n", "xxx", "xxx", "xxx", "xxx");
	strcat(cronHead2,cronTmp);
	sprintf(cronTmp, "    %16s %19s %20s %19s\n", "xxx", "xxx", "xxx", "xxx");
	strcat(cronHead2,cronTmp);
	sprintf(cronTmp, "    %16s\n", "xxx");
	strcat(cronHead2,cronTmp);

	printf ("%s\n", cronHead2);

	sprintf(cronTmp, "    ===========================================================================================\n");
	strcat(cronHead2,cronTmp);
	sprintf(cronTmp, "    STATTIME            TOT_EXT_LOG TOT_EXT_DEC_ERR_LOG   NO_IPPOOL_WAP_LOG           TOT_WAPLOG\n");
	strcat(cronHead2,cronTmp);
	sprintf(cronTmp, "                        TOT_SUCCESS            TOT_FAIL             NO_CALL              TX_FAIL\n");
	strcat(cronHead2,cronTmp);
	sprintf(cronTmp, "                           ETC_FAIL            COMPLETE             TIMEOUT            CANCELLED\n");
	strcat(cronHead2,cronTmp);
	sprintf(cronTmp, "                        TOT_UP_BYTE         TOT_DN_BYTE     NO_CALL_UP_BYTE      NO_CALL_DN_BYTE\n");
	strcat(cronHead2,cronTmp);
	sprintf(cronTmp, "                   ETC_FAIL_UP_BYTE    ETC_FAIL_DN_BYTE      GET_METHOD_CNT      POST_METHOD_CNT\n");
	strcat(cronHead2,cronTmp);
	sprintf(cronTmp, "                     ETC_METHOD_CNT\n");
	strcat(cronHead2,cronTmp);
	sprintf(cronTmp, "    ===========================================================================================\n");
	strcat(cronHead2,cronTmp);

	sprintf(cronTmp, "    %-14s %16s %19s %19s %20s\n", "111", "xxx", "xxx", "xxx", "xxx");
	strcat(cronHead2,cronTmp);
	sprintf(cronTmp, "    %-14s %16s %19s %19s %20s\n", "", "xxx", "xxx", "xxx", "xxx");
	strcat(cronHead2,cronTmp);
	sprintf(cronTmp, "    %-14s %16s %19s %19s %20s\n", "", "xxx", "xxx", "xxx", "xxx");
	strcat(cronHead2,cronTmp);
	sprintf(cronTmp, "    %-14s %16s %19s %19s %20s\n", "", "xxx", "xxx", "xxx", "xxx");
	strcat(cronHead2,cronTmp);
	sprintf(cronTmp, "    %-14s %16s %19s %19s %20s\n", "", "xxx", "xxx", "xxx", "xxx");
	strcat(cronHead2,cronTmp);
	sprintf(cronTmp, "    %-14s %16s\n", "", "xxx");
	strcat(cronHead2,cronTmp);

	printf ("%s\n", cronHead2);

	puts ("");
}
