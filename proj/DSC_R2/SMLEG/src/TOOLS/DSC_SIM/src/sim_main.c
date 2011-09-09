#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#ifdef MEM_CHECK
#include <mcheck.h>
#endif

#include "define.h"
#include "logutil.h"
#include "mmc.h"
#include "tcp_task.h"
#include "radius.h"
#include "sim.h"
#include "session.h"

#define	SEQ_PROC_ACCELS		101

int             g_flags;             /* argv flags */
int 			g_sid;
unsigned char   g_procname [80];
CONF_INFO		g_conf_info;
DEST_INFO		g_dest_info;
SEND_OPT		g_send_opt;

ACCT_REQ		g_acct_req;
ACCT_START 		g_acct_start;
ACCT_STOP 		g_acct_stop;

unsigned char*	g_radius_buf[MAX_RADIUS_PKTSIZE];
/* argument */
#define B_FLAG          0x00000001
#define D_FLAG          0x00000010
#define H_FLAG          0x00000100
#define M_FLAG          0x00001000
#define X_FLAG          0x00010000
#define P_FLAG          0x00100000
#define A_FLAG          0x01000000

extern mmc_t    _mmc[];


void
abort_program (int signo)
{
	signal (SIGTERM, abort_program);
	signal (SIGCLD, SIG_IGN);

#ifdef MEM_CHECK
	muntrace();
#endif
	dAppLog (LOG_CRI, "STOP DSC-SIM!");
	signo = 0;	// no warnning
    exit(0);
}

void
cmd_quit(char* arg,...)
{
	arg = NULL;
	abort_program (SIGTERM);
}


void make_pid_file (pid_t pid)
{
	char path[64];
	FILE *fp;

	memset(path, 0x00, sizeof(path));
	sprintf(path, "%s%s.pid", DATA_PATH, g_procname);

    if ((fp = fopen(path, "w")) == NULL) {
		dAppLog(LOG_CRI, "cannot open fd for pid file (%s)\n", path);
        exit(1);
    }

	fprintf(fp, "%d", pid);
	fclose(fp);
}

void checkParam(int argc, char **argv)
{
	if (argc != 1) {
		fprintf (stderr, "Usage: %s <conffile>\n", argv[0]);
		exit(0);
	}
}




int initProc (PCONF_INFO pci)
{
	int tmp;
	
	/* init log */
	InitAppLog (getpid(), SEQ_PROC_ACCELS, SIM_LOG_PATH, "dsc-sim");

	/* load conifg */
	getConfig (SIM_CONFIG_PATH, &g_conf_info);

	/* init udp socket */
	tmp  = initUdpSock ();
	if (tmp < 0) {
		return -1;
	}
	pci->stRadOpt.uiCFD = tmp;

#if 0
	/* init tcp socket */
	tmp = create_tcp_sock ();
	if (tmp < 0) {
		return -2;
	}
	pci->stHttpOpt.uiCFD = tmp;
#endif

	/* session init */
	pHashInfo = hasho_init (0, sizeof(hs_key), sizeof(hs_key), sizeof(hs_body), MAX_FD_HASHO_SIZE, 0);
	if (pHashInfo == NULL) {
		fprintf(stderr, " !hasho_init error(%s)", strerror(errno));
		return -3;
	}
	/* timer init */
	pTmrNInfo = timerN_init (MAX_FD_HASHO_SIZE, sizeof(hs_key));
	if (pTmrNInfo == NULL) {
		fprintf(stderr, " !timerN_init ERROR(%s)\n", strerror(errno));
		return -4;
	}
	return 0;
}

int main (int argc, char *argv[])
{
	char	buf [BUFSIZ], path[BUFSIZ], *p;
	int		line, cont;
	int		c, rtn=-1;


	checkParam (argc, &argv[0]);

#if 1
	while ((c = getopt (argc,argv,"hbd")) != -1) {
		switch(c) {
			case 'd'    : g_flags |= D_FLAG; break;     /* Is Dump print ? */
			case 'p'    : g_flags |= P_FLAG; break;     /* Is Performance print? */
		}
	}
#endif

	signal (SIGTERM, abort_program);

	fflush(stdout);
	strcpy (g_procname, argv [0]);

	/* init process */
	if((rtn = initProc (&g_conf_info)) < 0){
		fprintf(stderr, " !!! simulator init failed (%d)!!!", rtn);
		return -1;
	}

	/* load packet data */
	getRadiusData (SIM_RAIDUS_PATH, &g_acct_req);

	/* load http data */
	load_cfg_http (SIM_HTTP_PATH);

#ifdef MEM_CHECK
	mtrace ();
#endif

	dAppLog (LOG_CRI, "START DSC-SIM");

////////////////////////////////////////////////////////////////////////////////
//  fallowing MMC
////////////////////////////////////////////////////////////////////////////////
	regmmc(_mmc);

	fflush(stdout);
#if 1
	bzero (buf, BUFSIZ);
	printf ("\rPress the <tab> key at any time for completions.\n\n");
    for(line=0,cont=1;;cont=1)
	{
        if (argc != 1) {
            sleep (10);
        } else {
            printf("WCLI:%d # %s",line,buf);
            mmcgets(buf);
            if((p=strchr(buf,'\n'))!=NULL) { *p=0; cont=0;  }
            if((p=strchr(buf,'\t'))!=NULL) { *p=' ';        }
            if(strlen(buf)<=0) continue;

            line=pparse(buf,cont);
            if(!cont) bzero(buf,BUFSIZ);
        }
    }
#endif
    return 1;
}







