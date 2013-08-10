#include <stdio.h>
//#include <mysql.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <sys/signal.h>
#include <sys/msg.h>
#include <sys/wait.h>

char	trcBuf[1024];

#define SAMD_CONF_FILE "/DSC/NEW/DATA/samd.conf"

#define MAX_FILE_NAME_LEN    128 
#define MAX_STR_LEN     32
#define MAX_TBL_CNT     4 // MP mysql DB backup table °³¼ö 
#define MAX_SYS_CNT     3
typedef struct _DB_INFO_ {
	char    sysName[MAX_STR_LEN];
	char    dbIp[MAX_STR_LEN];
	char    dbName[MAX_STR_LEN];
	char    dbId[MAX_STR_LEN];
	char    dbPass[MAX_STR_LEN];
	int     tblCnt;                                                                        
	char    tblName[MAX_TBL_CNT][MAX_STR_LEN];
	char	backFile[MAX_FILE_NAME_LEN];
} DB_INFO_t;

DB_INFO_t	g_stDBInfo;

int init_db_info(void)
{
	char    fname[256] = {0,};
	FILE    *fp = NULL;
	char    buf[512] = {0,};
	int     cnt = 0, tCnt = 0;
	char    sSys[32] = {0,}, sDb[32] = {0,}, sTbl1[32] = {0,}, sTbl2[32] = {0,}, sBack[128] = {0,};
	char    sIp[20] = {0,}, sId[8] = {0,}, sPass[8] = {0,};

	const char	*path = "/DSC/OLD";

	sprintf(fname, "%s", SAMD_CONF_FILE);

	memset(&g_stDBInfo, 0x00, sizeof(DB_INFO_t));

	if( (fp = fopen(fname, "r")) != NULL )
	{
		while( fgets(buf, sizeof(buf), fp) )
		{
			if( buf[0] == '#' )
				continue;
			else if(!strncmp(buf, "SCMB",4))
			{
				sscanf(buf, "%s %s %s %s %s %s %s %s", sSys, sIp, sId, sPass, sDb, sBack, sTbl1, sTbl2);
				strcpy(g_stDBInfo.sysName, sSys);
				strcpy(g_stDBInfo.dbIp, sIp);
				strcpy(g_stDBInfo.dbId, sId);
				strcpy(g_stDBInfo.dbPass, sPass);
				strcpy(g_stDBInfo.dbName, sDb);
				strcpy(g_stDBInfo.backFile, sBack);
				if( sTbl1[0] != NULL )
				{
					strcpy(g_stDBInfo.tblName[tCnt], sTbl1);
					tCnt++;
				}
				if( sTbl2[0] != NULL )
				{
					strcpy(g_stDBInfo.tblName[tCnt], sTbl2);
					tCnt++;
				}

				g_stDBInfo.tblCnt = tCnt;
				cnt++;
			}
			tCnt = 0;
		}
	}
	else
	{
		sprintf(trcBuf, "[init_db_info()] file open fail=%s\n", fname);
		printf("%s",trcBuf);
		return -1;
	}                                                                                      

	fclose(fp);

	return 0;
}

void print_db_info(void)
{
	int i = 0, j = 0;
	for(i = 0; i < MAX_SYS_CNT; i++)
	{
		printf("SYS:%s\n",g_stDBInfo.sysName);
		printf("IP:%s\n",g_stDBInfo.dbIp);
		printf("ID:%s\n",g_stDBInfo.dbId);
		printf("PASS:%s\n",g_stDBInfo.dbPass);
		printf("DB:%s\n",g_stDBInfo.dbName);
		printf("table count : %d\n", g_stDBInfo.tblCnt);
		for(j = 0; j < g_stDBInfo.tblCnt; j++ )
		{
			printf("TBL:%s\n", g_stDBInfo.tblName[j]);
		}

	}
}

int db_backup(char *backupdir)
{
	int     ret = 0, i = 0, j = 0;
	char    cmd[1024] = {0,};
	char    backFile[256] = {0,};

	for( i = 0; i < MAX_SYS_CNT; i++ )
	{
		memset(cmd, 0x00, sizeof(cmd));
		memset(backFile, 0x00, sizeof(backFile));
		sprintf(backFile, "%s/%s", backupdir, g_stDBInfo.backFile);
		strcpy(g_stDBInfo.backFile, backFile);


		if( !strcmp(g_stDBInfo.sysName, "DSCM" ))
		{                                                                              
			sprintf(cmd, "mysqldump -u%s -p%s -h%s %s > %s", g_stDBInfo.dbId, 
					g_stDBInfo.dbPass, 
					g_stDBInfo.dbIp, 
					g_stDBInfo.dbName, 
					g_stDBInfo.backFile);
			system(cmd);
			break;

		}
		else if( !strcmp(g_stDBInfo.sysName, "SCMA" ))
		{
			sprintf(cmd, "mysqldump -u%s -p%s -h%s %s %s %s > %s", g_stDBInfo.dbId,
					g_stDBInfo.dbPass,
					g_stDBInfo.dbIp,
					g_stDBInfo.dbName,
					g_stDBInfo.tblName[0], 
					g_stDBInfo.tblName[1], 
					g_stDBInfo.backFile);
			system(cmd);
			break;
		}
		else if( !strcmp(g_stDBInfo.sysName, "SCMB" ))
		{
			sprintf(cmd, "mysqldump -u%s -p%s -h%s %s %s %s > %s", g_stDBInfo.dbId,
					g_stDBInfo.dbPass,
					g_stDBInfo.dbIp,
					g_stDBInfo.dbName,
					g_stDBInfo.tblName[0], 
					g_stDBInfo.tblName[1], 
					g_stDBInfo.backFile);  
	
			printf("cmd");

			system(cmd);                                                               
			break;
		}                                                                              
	}                                               
	return 0;
}

void handleChildProcess (void)
{       
	int status;
	while (wait3 (&status, WNOHANG, (struct rusage *)0) > 0);
	signal(SIGCHLD, handleChildProcess);
}   


int main(void)
{
	int ret;
	char	*path = "/DSC/OLD";
	char	*path2 = "/DSC";
	int	pid = -1;
	int	cnt = 0;

	ret = init_db_info();

	signal(SIGCHLD, handleChildProcess);
	if( ret == 0 )
	{
		print_db_info();

		if ((pid = fork()) < 0){
			printf ("fork() fail....\n");
			return -1;
		}

		if ( pid == 0) 
		{
			ret = db_backup(path);
			if( ret == 0 )
			{
				printf("success db backup...\n");
			}

			exit(0);
		}
	}

	if ((pid = fork()) < 0){
		printf ("fork() fail....\n");
		return -1;
	}

	if ( pid == 0) 
	{
		ret = db_backup(path2);
		if( ret == 0 )
		{
			printf("success db backup...\n");
		}

		exit(0);
	}



	while(1)
	{
		if( (cnt % 500000) == 0 )
			printf("mother is alive.\n");

		if( cnt > 1000000 )
		{
			break;
		}
		cnt++;
	}

	return 0;
}

