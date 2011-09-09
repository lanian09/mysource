#include <stdio.h>
#include <nmsif_proto.h>
#include <nmsif.h>

int		nmsifQid;

char	conbuf[1024] = "    BSDM-BSDM 2006-07-31 20:00:00.13\n*** A1015 SUCC RATE ALARM OCCURED\n      SYSTYPE  = BSDM\n      RSC      = WAP1\n      INFO     = UNDER THRESHOLD [80]\n      COMPLETED";

GeneralQMsgType	txmsg;
IxpcQMsgType	*alm;


main (int argc, char **argv)
{
	int		argval;
	int		len;

	if (argc != 2) {
		printf ("[usage] bsd_app 1/2\n");
		printf ("                1 => real time alarm/status\n");
		printf ("                2 => statistics\n");
		puts ("");
		exit (1);
	}

	argval = atoi (argv[1]);

	if (init () < 0)
		exit (1);

	memset (&txmsg, 0, sizeof (GeneralQMsgType));

	if (argval == 1 || argval == 2) {
		if (argval == 1) {
			alm = (IxpcQMsgType *)txmsg.body;
			txmsg.mtype = MTYPE_ALARM_REPORT;
			strcpy (alm->body, conbuf);
		}
		else if (argval == 2) {
			txmsg.mtype = MTYPE_STATISTICS_REPORT;
			sprintf (txmsg.body, "%d", STAT_PERIOD_5MIN);
			//sprintf (txmsg.body, "%d", STAT_PERIOD_HOUR);
		}
		len = strlen (txmsg.body) + 4;

		if (msgsnd (nmsifQid, &txmsg, len, IPC_NOWAIT) < 0)
			printf ("[*] msgsnd fail : %s\n", strerror (errno));
		else printf ("[*] msgsnd success (->nmsif)\n");
	}
	else {
		printf ("[usage] bsd_app 1/2\n");
		printf ("                1 => real time alarm/status\n");
		printf ("                2 => statistics\n");
	}
	puts ("");
	exit (0);
}


init ()
{
	char	*env, tmp[80], fname[200];
	int		key;

	if ((env = getenv (IV_HOME)) == NULL) {
		printf ("[*] not found %s env_variable\n", IV_HOME);
		return -1;
	}

	sprintf (fname, "%s/%s", env, SYSCONF_FILE);

	if (conflib_getNthTokenInFileSection (fname, "APPLICATIONS", "NMSIF", 1, tmp) < 0) {
		printf ("[*] fail get keyword (nmsif)\n");
		return -1;
	}
	key = strtol (tmp, 0, 0);
	if ((nmsifQid = msgget (key, IPC_CREAT|0666)) < 0) {
		printf ("[*] tx msgget fail : %s\n", strerror (errno));
		return -1;
	}

	return 1;

}
