
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include "utillib.h"
#include "leg.h"

/* definition global variable */
int numSubscribersToLogin;
int count = 0;

SMNB_HANDLE 		gSCE_nbapi;


void connectionIsDown (void)
{
	printf ( "disconnect listener callback:: connection is down\n");
}

//prints every error that occurs
void handleError (Uint32 argHandle, ReturnCode* argReturnCode)
{
	++count;
	printf ( "[handleError] error:%d", count);
	printReturnCode(argReturnCode);
	freeReturnCode(argReturnCode);
}

//prints a success result every 100 results
void handleSuccess (Uint32 argHandle, ReturnCode* argReturnCode)
{
//	printf ( "%d, [handleSuccess] result:%d\n", time(NULL), count);

	if (++count%500 == 0)
	{
		printf("\t %d result %d:\n", time(NULL), count);
		printReturnCode(argReturnCode);
	}
	freeReturnCode(argReturnCode);
}

//waits for result number 'last result' to arrive
void waitForLastResult (int lastResult)
{
	while (count<lastResult)
	{
		sleep(1);
	}
}


void IgnoreSignal(int sign)
{
//    if (sign != SIGALRM)
 //       dAppLog( LOG_CRI, "UNWANTED SIGNAL IS RECEIVED, signal = %d", sign);
    signal(sign, IgnoreSignal);
}

/*******************************************************************************
    
*******************************************************************************/
void SetUpSignal()
{

    /* UNWANTED SIGNALS */
    signal(SIGHUP,  IgnoreSignal);
    signal(SIGALRM, IgnoreSignal);
    signal(SIGPIPE, IgnoreSignal);
    signal(SIGPOLL, IgnoreSignal);
    signal(SIGPROF, IgnoreSignal);
    signal(SIGUSR1, IgnoreSignal);
    signal(SIGUSR2, IgnoreSignal);
    signal(SIGVTALRM, IgnoreSignal);
    signal(SIGCLD, SIG_IGN);
}


int main (int argc, char* argv[])
{
	int i;
	int numSubscribersToLogin;
	SMNB_HANDLE 	*pgSCE_nbapi = &gSCE_nbapi;

	if (argc != 2) {
		printf ( "usage: %s <테스트로그인 개수>\n", argv[0]);
		exit(1);
	}

	SetUpSignal();

	numSubscribersToLogin = atoi(argv[1]);

	printf("START SIMSCE numSubscribersToLogin[%d]\n", numSubscribersToLogin); fflush(stdout);

	// instantiation
	*pgSCE_nbapi = SMNB_init(0,2000000,10,30);
	if (*pgSCE_nbapi == NULL) {
		printf("[LEG_MAIN] [SMNB INITIAL FAIL]");
		exit(0);
	}

	/* SM CONNECTION */
	SMNB_setDisconnectListener (*pgSCE_nbapi, connectionIsDown);
	printf("Call SMNB_connect() IP[%s] PORT[%d]\n", LEG_SCM_ADDR, LEG_SCM_PORT); fflush(stdout);
	SMNB_connect (*pgSCE_nbapi, LEG_SCM_ADDR, LEG_SCM_PORT);
	printf("Call SMNB_setReplyFailCallBack()\n");fflush(stdout);
	SMNB_setReplyFailCallBack (*pgSCE_nbapi, handleError);
	printf("Call SMNB_setReplySuccessCallBack()\n");fflush(stdout);
	SMNB_setReplySuccessCallBack (*pgSCE_nbapi, handleSuccess);
	printf("AFTER SMNB_setReplySuccessCallBack()\n");fflush(stdout);


	// login
	char name[10];
	char ipString[15];
	char* ip = &(ipString[0]);

	MappingType type = IP_RANGE;
	Uint32 ipVal = 0x0a000000;

	printf("%d, [LOGIN] Start \n", time(NULL));
	for (i=0; i<numSubscribersToLogin; i++)
	{
//		sprintf((char*)name,"s%d", i);
		sprintf((char*)name,"010123%05d", i);
		sprintf((char*)ip,"%d.%d.%d.%d",
				(int)((ipVal & 0xFF000000) >> 24),
				(int)((ipVal & 0x00FF0000) >> 16),
				(int)((ipVal & 0x0000FF00) >> 8),
				(int)(ipVal & 0x000000FF));

		if ((i % 500) == 0) 
			printf("%d, [LOGIN] OF NAME[%s]ip[%s]\n", time(NULL), name, ip);

		ipVal++;
		SMNB_login(*pgSCE_nbapi,
					name, //subscriber name
					&ip, //a single ip mapping
					&type,
					1,
					NULL, //no properties
					NULL,
					0,
					"subscribers", //domain
					0, //mappings are not additive
					-1); //disable auto-logout
	}
	printf("%d, [LOGIN] end1 \n", time(NULL));


	while (1) 
		sleep (1);
//	waitForLastResult(numSubscribersToLogin);
	printf("%d, [LOGIN] end2 \n", time(NULL));

#ifdef	_LOGOUT_
	ipVal = 0x0a000000;
	for (i=0; i<numSubscribersToLogin; i++)
	{
		sprintf((char*)ip,"%d.%d.%d.%d",
				(int)((ipVal & 0xFF000000) >> 24),
				(int)((ipVal & 0x00FF0000) >> 16),
				(int)((ipVal & 0x0000FF00) >> 8),
				(int)(ipVal & 0x000000FF));
		ipVal++;

		printf("[LOGOUT] OF NAME[%s]ip[%s]\n", name, ip);

		SMNB_logoutByMapping(*pgSCE_nbapi, ip, type, LEG_SCM_ADDR);
	}
	waitForLastResult(numSubscribersToLogin*2);
#endif

	// release
	SMNB_disconnect(*pgSCE_nbapi);
	SMNB_release(*pgSCE_nbapi);

	return 0;
}
