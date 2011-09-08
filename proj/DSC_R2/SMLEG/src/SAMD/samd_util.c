#include "samd.h"

extern VersionIndexTable vit[];

#if 0
char *get_ver_str(char *procname);
#endif
int valid_ipaddr(char *s1);
int compare_ipaddr(char *s1, char *s2);
void strtoupper(char *buff);
void sync_files_to_remote(void);

#if 0
char *get_ver_str(char *procname)
{
	int			i, ret, rst;
	char		prcName[COMM_MAX_NAME_LEN];
	char		ver[10];

	memset(ver, 0x00, sizeof(ver));
	memset(prcName, 0x00, sizeof(char) * COMM_MAX_NAME_LEN);

	ret	= strlen(procname);
	rst	= isdigit(procname[ret-1]);

	if(rst)
		strncpy(prcName, procname, ret-1);
	else
		strcpy(prcName, procname);

	for(i = 0; vit[i].name != NULL; i++)
	{
		fprintf(stderr, "TEST procname:%s, vit[%d].name:%s\n", procname, i, vit[i].name);

		if(!strcasecmp(prcName, vit[i].name))
		{
			strcpy(ver, get_proc_version(prcName));
		//	get_version(vit[i].index, ver);
		//	fprintf(stderr, "TEST procname:%s, vit[%d].index:%d, ver:%s\n", prcName,i,vit[i].index, ver);

			if(strlen(ver) < 1)
				strcpy(ver, "UNKNOWN");

			return ver;
		}
	}

	return NULL;
}
#endif
void strtoupper(char *buff)
{
	int		i;
	size_t	len_buff;

	len_buff = strlen(buff);
	for(i = 0; i < len_buff; i++)
		buff[i] = toupper(buff[i]);
}

int valid_ipaddr(char *s1)
{
	in_addr_t addr1 = inet_addr(s1);

	if(addr1 == -1)
	{
		if(!strcmp(s1, "255.255.255.255"))
			return 0;
		else
			return -1;
	}
	else
		return 0;
}

int compare_ipaddr(char *s1, char *s2)
{
    in_addr_t   addr1, addr2;

    addr1 = inet_addr(s1);
    addr2 = inet_addr(s2);

    if(addr1 == addr2)
        return 0;
    else
        return 1;
}

void sync_files_to_remote(void)
{
    char    cmd[128], *myname;
    char    hostname[20];

    myname = getenv(MY_SYS_NAME);
#if 0
    if(!strcasecmp(myname, "bsda")){
        strcpy(hostname, "bsdb");
    }else if(!strcasecmp(myname, "bsdb")){
        strcpy(hostname, "bsda");
    }

    sprintf(cmd, "%s %s/NEW/BIN/* %s::BSD/BIN", RSYNC, getenv(IV_HOME), hostname);
    system(cmd);

    sprintf(cmd, "%s %s/NEW/DATA/* %s::BSD/DATA", RSYNC, getenv(IV_HOME), hostname);
    system(cmd);
#endif
    if(!strcasecmp(myname, "dsca")){
        strcpy(hostname, "dscb");
    }else if(!strcasecmp(myname, "dscb")){
        strcpy(hostname, "dsca");
    }

    sprintf(cmd, "%s %s/NEW/BIN/* %s::DSC/BIN", RSYNC, getenv(IV_HOME), hostname);
    my_system(cmd);

    sprintf(cmd, "%s %s/NEW/DATA/* %s::DSC/DATA", RSYNC, getenv(IV_HOME), hostname);
    my_system(cmd);
}
