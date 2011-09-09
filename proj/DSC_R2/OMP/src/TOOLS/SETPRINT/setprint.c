#include <time.h>
#include <comm_msgtypes.h>
#include <sysconf.h>
#include <commlib.h>

void getArgs (int, char**, TrcLibSetPrintMsgType*, char*);
int  getQid (char*);
void printUsage (void);


/*------------------------------------------------------------------------------
------------------------------------------------------------------------------*/
int main (int ac, char *av[])
{
	TrcLibSetPrintMsgType	txMsg;
	char	appName[32];
	int		qid;

	memset ((void*)&txMsg, 0, sizeof(TrcLibSetPrintMsgType));

	getArgs (ac, av, &txMsg, appName);

	txMsg.mtype = MTYPE_SETPRINT;
	strcpy (txMsg.trcDeviceName, (char*)ttyname(0));

	if ((qid = getQid(appName)) < 0)
		return -1;

	if (msgsnd(qid, &txMsg, sizeof(TrcLibSetPrintMsgType)) < 0) {
		printf(" msgsnd fail; err=%d(%s)\n", errno, strerror(errno));
		return -1;
	}

	return 0;

} /** End of main **/


/*------------------------------------------------------------------------------
------------------------------------------------------------------------------*/
void getArgs (int ac, char *av[], TrcLibSetPrintMsgType *txMsg, char *appName)
{
	int		a;

	if (ac < 3)
		printUsage();

	strcpy (appName,"");

	while ((a = getopt(ac,av,"b:t:l:h")) != EOF) {
		switch (a) {
			case 'b':
				strcpy(appName,optarg);
				break;

			case 't':
				txMsg->trcFlag.pres = 1;
				if (!strcasecmp(optarg,"off"))
					txMsg->trcFlag.octet = 0;
				else if (!strcasecmp(optarg,"on"))
					txMsg->trcFlag.octet = 1;
				else if (isdigit(optarg[0]))
					txMsg->trcFlag.octet = atoi(optarg);
				else
					printUsage();
				break;

			case 'l':
				txMsg->trcLogFlag.pres = 1;
				if (!strcasecmp(optarg,"off"))
					txMsg->trcLogFlag.octet = 0;
				else if (!strcasecmp(optarg,"on"))
					txMsg->trcLogFlag.octet = 1;
				else if (isdigit(optarg[0]))
					txMsg->trcLogFlag.octet = atoi(optarg);
				else
					printUsage();
				break;

			case 'h':
				printUsage();
				break;

			default:
				printUsage();
				break;
		}
	}

	if (optind<ac || !strcasecmp(appName,""))
		printUsage();

	return;

} /** End of getArgs **/



/*------------------------------------------------------------------------------
------------------------------------------------------------------------------*/
int getQid (char *appName)
{
	char	*env,fname[256],tmp[32];
	int		i,key,qid;

	for (i=0; i<strlen(appName); i++)
		appName[i] = toupper(appName[i]);

	if ((env = getenv(IV_HOME)) == NULL) {
		printf(" not found %s environment name\n", IV_HOME);
		return -1;
	}
	sprintf(fname,"%s/%s", env, SYSCONF_FILE);

	if (conflib_getNthTokenInFileSection(fname,"APPLICATIONS",appName,1,tmp) < 0) {
		printf(" not found msgQkey in [%s]\n", fname);
		return -1;
	}
	key = strtol(tmp,0,0);
	if ((qid = msgget(key,0)) < 0) {
		printf(" msgget fail; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
		return -1;
	}
	return qid;

} /** End of getQid **/


/*------------------------------------------------------------------------------
------------------------------------------------------------------------------*/
void printUsage (void)
{
	printf("usage : setprint [-b block] [-t on|off] [-l on|off] \n");
	printf("                 -b : application_name (block_name) \n");
	printf("                 -t : trace flag on or off \n");
	printf("                 -l : log flag on or off \n");
	printf("                 -h : display this message \n");
	exit(1);
} /** End of printUsage **/
