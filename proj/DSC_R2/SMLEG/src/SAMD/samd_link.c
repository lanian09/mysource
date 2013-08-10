#include "samd.h"


extern int  errno, trcFlag, trcLogFlag;
extern SFM_SysCommMsgType	*loc_sadb;
extern char	trcBuf[TRCBUF_LEN], trcTmp[TRCTMP_LEN];
extern char	mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN];
extern int findIndex(char *syntax, char *idex);

OpticalLan	optLan[SFM_MAX_DEV_CNT];

#ifdef __LINUX__
int link_test(void)
{
	int		rtn, cnt;
	char	*env, command[256], fname[256], lineBuf[1024], token[12][32];
	FILE	*fp;

	cnt = 0;

	if( (env = getenv(IV_HOME)) == NULL)
		return 0;

	sprintf(fname, "%s/TMP/linktest_%s", env, mySysName);
	sprintf(command, "dagfour -s > %s", fname );
	system(command);

	if( (fp = fopen (fname, "r")) == NULL)
	{
		sprintf (trcBuf, "[link_test] fopen fail[%s]; err=%d(%s)\n", fname, errno, strerror(errno));
		trclib_writeLogErr(FL, trcBuf);
		return 0;
	}

	while(fgets(lineBuf, sizeof(lineBuf), fp) != NULL)
	{
		rtn = sscanf (lineBuf, "%s%s%s%s%s%s%s%s%s", token[0], token[1], token[2], token[3], token[4], token[5], token[6], token[7], token[8]);
		//printf("0 = %s\n", token[0]);
		if( !strcasecmp(token[0], "Port") ) continue;

		//#ORDER    0    1    2    3            4    5    6    7
		// Port A: Sync Link Auto RFlt Port B: Sync Link Auto RFlt
		//          1    1    1    0            1    1    1    0

		set_optical_lan_status('A', token[1]);
		set_optical_lan_status('B', token[5]);
		cnt++;
	}
	fclose(fp);

	if(cnt == 0)
	{
		loc_sadb->loc_link_sts[0].status = SFM_LAN_DISCONNECTED;
		loc_sadb->loc_link_sts[1].status = SFM_LAN_DISCONNECTED;
	}

	return 1;

}

void set_optical_lan_status(char side, char *value)
{
	int		i, j;

	for(i = 0; i < SFM_MAX_DEV_CNT; i++)
	{
		if(optLan[i].side == side)
		{
			for(j = 0; j < SFM_MAX_DEV_CNT; j++)
			{
				if(!strcmp(optLan[i].name, loc_sadb->loc_link_sts[j].linkName))
					loc_sadb->loc_link_sts[j].status = !atoi(value);
			}
		}
	}
}
#endif
