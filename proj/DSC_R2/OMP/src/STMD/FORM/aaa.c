#include <stdio.h>

char	cronTmp[1024];
char	cronHead2[2048];

main ()
{
	memset (cronTmp, 0, 1024);
	memset (cronHead2, 0, 2048);

	printf ("[*] aaa\n");
	
	sprintf(cronTmp, "    ===============================================================================\n");
	strcat(cronHead2,cronTmp);
	sprintf(cronTmp, "            REQUEST         SUCCESS            FAIL           START    START_TIMOUT\n");
	strcat(cronHead2,cronTmp);
	sprintf(cronTmp, "      START_RETRANS         INTERIM INTERIM_TIMEOUT INTERIM_RETRANS            STOP\n");
	strcat(cronHead2,cronTmp);
	sprintf(cronTmp, "       STOP_TIMEOUT    STOP_RETRANS     NETWORK_ERR        TIME_OUT         ETC_ERR\n");
	strcat(cronHead2,cronTmp);
	sprintf(cronTmp, "       SUCC_RATE(%%)\n");
	strcat(cronHead2,cronTmp);
	sprintf(cronTmp, "    ===============================================================================\n");
	strcat(cronHead2,cronTmp);

	sprintf(cronTmp, "    %15s %15s %15s %15s %15s\n", "xxx", "xxx", "xxx", "xxx", "xxx");
	strcat(cronHead2,cronTmp);
	sprintf(cronTmp, "    %15s %15s %15s %15s %15s\n", "xxx", "xxx", "xxx", "xxx", "xxx");
	strcat(cronHead2,cronTmp);
	sprintf(cronTmp, "    %15s %15s %15s %15s %15s\n", "xxx", "xxx", "xxx", "xxx", "xxx");
	strcat(cronHead2,cronTmp);
	sprintf(cronTmp, "    %15s %15s %15s %15s %15s\n", "xxx", "xxx", "xxx", "xxx", "xxx");
	strcat(cronHead2,cronTmp);

	printf ("%s\n", cronHead2);

	sprintf(cronTmp, "    ==============================================================================================\n");
	strcat(cronHead2,cronTmp);
	sprintf(cronTmp, "    STATTIME               REQUEST         SUCCESS            FAIL           START    START_TIMOUT\n");
	strcat(cronHead2,cronTmp);
	sprintf(cronTmp, "                     START_RETRANS         INTERIM INTERIM_TIMEOUT INTERIM_RETRANS            STOP\n");
	strcat(cronHead2,cronTmp);
	sprintf(cronTmp, "                      STOP_TIMEOUT    STOP_RETRANS     NETWORK_ERR        TIME_OUT         ETC_ERR\n");
	strcat(cronHead2,cronTmp);
	sprintf(cronTmp, "                      SUCC_RATE(%%)\n");
	strcat(cronHead2,cronTmp);
	sprintf(cronTmp, "    ==============================================================================================\n");
	strcat(cronHead2,cronTmp);
	printf ("%s\n", cronHead2);
	puts ("");
}
