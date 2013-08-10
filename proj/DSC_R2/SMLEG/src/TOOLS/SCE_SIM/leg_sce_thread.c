#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/timeb.h>
#include "utillib.h"
#include "leg.h"



/********************************** 
 *  SET PROPERTY OPTION
**********************************/
//#define 	_MACRO_MAPPING_NULL_
//#define 	_MACRO_PROPERTY_NULL_
#define 	_MACRO_PROPERTY_ONE_
//#define 	_MACRO_PROPERTY_TWO_
/**********************************/


#define NUM_PROPERTY	2
#define NUM_HANDLE		0xFFFF

typedef struct _hLOGIN {
	char *key[NUM_PROPERTY];
	char *val[NUM_PROPERTY];
} hLOGIN;

#define GETHANDLE(d)	((Uint32)(d%NUM_HANDLE))
hLOGIN	hlogin[NUM_HANDLE];


/* definition global variable */
int numSubscribersToLogin;
int count = 0;
int cnt=0, tot=0;

SMNB_HANDLE 		gSCE_nbapi;
SMNB_HANDLE 		gSCE_nbapi2;
SMNB_HANDLE 		gSCE_nbapi3;
SMNB_HANDLE 		gSCE_nbapi4;
SMNB_HANDLE 		gSCE_nbapi5;
SMNB_HANDLE 		gSCE_nbapi6;
SMNB_HANDLE 		gSCE_nbapi7;
SMNB_HANDLE 		gSCE_nbapi8;
SMNB_HANDLE 		gSCE_nbapi9;
SMNB_HANDLE 		gSCE_nbapi10;

#define PERIOD 100
struct timeb time_start, time_stop;

void freeHandle(Uint32 argHandle) 
{
	Uint32 handle = GETHANDLE(argHandle);

	if(hlogin[handle].key[0]) { free(hlogin[handle].key[0]); hlogin[handle].key[0] = NULL; }
	if(hlogin[handle].key[1]) { free(hlogin[handle].key[1]); hlogin[handle].key[1] = NULL; }
	if(hlogin[handle].val[0]) { free(hlogin[handle].val[0]); hlogin[handle].val[0] = NULL; }
	if(hlogin[handle].val[1]) { free(hlogin[handle].val[1]); hlogin[handle].val[1] = NULL; }
}

void connectionIsDown (void)
{
	printf ( "disconnect listener callback:: connection is down\n");
}

//prints every error that occurs
void handleError (Uint32 argHandle, ReturnCode* argReturnCode)
{
	++count;
	printf ( "[handleError] error:%d", count);
	freeHandle(argHandle);
	printReturnCode(argReturnCode);
	freeReturnCode(argReturnCode);
}

//prints a success result every 100 results
void handleSuccess (Uint32 argHandle, ReturnCode* argReturnCode)
{
	double sec;
//	printf ( "%d, [handleSuccess] result:%d\n", time(NULL), count);

	freeHandle(argHandle);
	cnt++;
	tot++;
	if (++count%500 == 0)
	{
//		printf("\t %d result %d:\n", time(NULL), count);
		printReturnCode(argReturnCode);

		ftime (&time_stop);
		sec = (double)((time_stop.time*1000 + time_stop.millitm)         
				-(time_start.time*1000 + time_start.millitm))/1000.0;         
		printf("#%6d Elapsed time: %.3f sec (%.1f cps)\n" , tot, sec ,(double)(cnt)/sec);

		time_start = time_stop; 

		cnt = 0;
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


void *thread_login_one (void *arg)
{
	SMNB_HANDLE 	*pgSCE_nbapi = &gSCE_nbapi;
	SMNB_HANDLE 	*pgSCE_nbapi2 = &gSCE_nbapi2;
	int				i;

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
	printf("Call SMNB_setReplyFailCallBack()\n");fflush(stdout);
	SMNB_setReplySuccessCallBack (*pgSCE_nbapi, handleSuccess);
	printf("AFTER SMNB_setReplySuccessCallBack()\n");fflush(stdout);


	// instantiation
	*pgSCE_nbapi2 = SMNB_init(0,2000000,10,30);
	if (*pgSCE_nbapi2 == NULL) {
		printf("[LEG_MAIN] [SMNB INITIAL FAIL]");
		exit(0);
	}

	/* SM CONNECTION */
	SMNB_setDisconnectListener (*pgSCE_nbapi2, connectionIsDown);
	printf("Call SMNB_connect() IP[%s] PORT[%d]\n", LEG_SCM_ADDR, LEG_SCM_PORT); fflush(stdout);
	SMNB_connect (*pgSCE_nbapi2, LEG_SCM_ADDR, LEG_SCM_PORT);
	printf("Call SMNB_setReplyFailCallBack()\n");fflush(stdout);
	SMNB_setReplyFailCallBack (*pgSCE_nbapi2, handleError);
	printf("Call SMNB_setReplyFailCallBack()\n");fflush(stdout);
	SMNB_setReplySuccessCallBack (*pgSCE_nbapi2, handleSuccess);
	printf("AFTER SMNB_setReplySuccessCallBack()\n");fflush(stdout);



	// login
	char name[10];
	char ipString[15];
	char* ip = &(ipString[0]);
	char *prop_key[2] = { NULL,  NULL };
	char *prop_val[2] = { NULL,  NULL };

	Uint32 argHandle=0;
	MappingType type = IP_RANGE;
	Uint32 ipVal = 0x1a000000;

	printf("%d, [LOGIN] Start \n", time(NULL));
	for (i=0; i<numSubscribersToLogin; i++)
	{
//		sprintf((char*)name,"s%d", i);
		//sprintf((char*)name,"011100%05d", i);
		sprintf((char*)name,"4500611100%05d", i);
		sprintf((char*)ip,"%d.%d.%d.%d",
				(int)((ipVal & 0xFF000000) >> 24),
				(int)((ipVal & 0x00FF0000) >> 16),
				(int)((ipVal & 0x0000FF00) >> 8),
				(int)(ipVal & 0x000000FF));

#ifdef _MACRO_PROPERTY_ONE_
		if((prop_key[0]=(char*)malloc(10)))	sprintf(prop_key[0], "%s", PROPERTY_PACKAGE_ID);
		if((prop_val[0]=(char*)malloc(10)))	sprintf(prop_val[0], "%d", PROPERTY_PACKAGE_ID_VAL);
		//fprintf(stderr, "_MACRO_PROPERTY_ONE_\n");
#endif
#ifdef _MACRO_PROPERTY_TWO_
		if((prop_key[0]=(char*)malloc(10)))	sprintf(prop_key[0], "%s", PROPERTY_MONITOR);
		if((prop_key[1]=(char*)malloc(10)))	sprintf(prop_key[1], "%s", PROPERTY_PACKAGE_ID);
		if((prop_val[0]=(char*)malloc(10)))	sprintf(prop_val[0], "%d", PROPERTY_MONITOR_MODE_OFF);
		if((prop_val[1]=(char*)malloc(10)))	sprintf(prop_val[1], "%d", PROPERTY_PACKAGE_ID_VAL);
		//fprintf(stderr, "_MACRO_PROPERTY_TWO_\n");
#endif
//		printf("%d, [LOGIN] OF NAME[%s]ip[%s]\n", time(NULL), name, ip);

		if (pgSCE_nbapi != NULL) {
			ipVal++;
			argHandle = SMNB_login(*pgSCE_nbapi,
									name, 			//subscriber name
#ifdef _MACRO_MAPPING_NULL_
									NULL,
#else
									&ip, 			//a single ip mapping
#endif
									&type,
									1,
#ifdef _MACRO_PROPERTY_ONE_
									prop_key, 			//no properties
									prop_val,
									1,
#endif
#ifdef _MACRO_PROPERTY_TWO_
									prop_key, 			//no properties
									prop_val,
									2,
#endif
#ifdef _MACRO_PROPERTY_NULL_
									NULL,
									NULL,
									0,
#endif
									"subscribers", 	//domain
									0, 				//mappings are not additive
									-1); 			//disable auto-logout
			{
				Uint32 handle = GETHANDLE(argHandle);

				hlogin[handle].key[0] = prop_key[0];
				hlogin[handle].key[1] = prop_key[1];
				hlogin[handle].val[0] = prop_val[0];
				hlogin[handle].val[1] = prop_val[1];
			}
		}
	}
	printf("%d, [LOGIN] end1 \n", time(NULL));

#if 0
	while (1) {
		usleep (1);
	}
#endif
	sleep(2);

	printf("%d, [LOGOUT] Start \n", time(NULL));

	ipVal = 0x1a000000;
	for (i=0; i<numSubscribersToLogin; i++)
	{
		sprintf((char*)name,"4500611100%05d", i);
		sprintf((char*)ip,"%d.%d.%d.%d",
				(int)((ipVal & 0xFF000000) >> 24),
				(int)((ipVal & 0x00FF0000) >> 16),
				(int)((ipVal & 0x0000FF00) >> 8),
				(int)(ipVal & 0x000000FF));

		ipVal++;

		//printf("[LOGOUT] OF NAME[%s]ip[%s]\n", name, ip);

#if 0
		/* CASE1. one connection */
		SMNB_logoutByMapping(*pgSCE_nbapi, ip, type, LEG_SCM_ADDR);
#else
		/* CASE1. two connection(logon & logout) */
		SMNB_logoutByMapping(*pgSCE_nbapi2, ip, type, LEG_SCM_ADDR);
#endif
	}
	printf("%d, [LOGOUT] end \n", time(NULL));

	pthread_exit (NULL);
}

void *thread_login_two (void *arg)
{
	SMNB_HANDLE 	*pgSCE_nbapi = &gSCE_nbapi2;
	int				i;

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
	printf("Call SMNB_setReplyFailCallBack()\n");fflush(stdout);
	SMNB_setReplySuccessCallBack (*pgSCE_nbapi, handleSuccess);
	printf("AFTER SMNB_setReplySuccessCallBack()\n");fflush(stdout);

	// login
	char name[10];
	char ipString[15];
	char* ip = &(ipString[0]);
	char *prop_key[2] = { NULL,  NULL };
	char *prop_val[2] = { NULL,  NULL };

	Uint32 argHandle=0;
	MappingType type = IP_RANGE;
	Uint32 ipVal = 0x1b000000;

	printf("%d, [LOGIN] Start \n", time(NULL));
	for (i=0; i<numSubscribersToLogin; i++)
	{
		sprintf((char*)name,"011101%05d", i);
		sprintf((char*)ip,"%d.%d.%d.%d",
				(int)((ipVal & 0xFF000000) >> 24),
				(int)((ipVal & 0x00FF0000) >> 16),
				(int)((ipVal & 0x0000FF00) >> 8),
				(int)(ipVal & 0x000000FF));
		
#ifdef _MACRO_PROPERTY_ONE_
		if((prop_key[0]=(char*)malloc(10)))	sprintf(prop_key[0], "%s", PROPERTY_PACKAGE_ID);
		if((prop_val[0]=(char*)malloc(10)))	sprintf(prop_val[0], "%d", PROPERTY_PACKAGE_ID_VAL);
		//fprintf(stderr, "_MACRO_PROPERTY_ONE_\n");
#endif
#ifdef _MACRO_PROPERTY_TWO_
		if((prop_key[0]=(char*)malloc(10)))	sprintf(prop_key[0], "%s", PROPERTY_MONITOR);
		if((prop_key[1]=(char*)malloc(10)))	sprintf(prop_key[1], "%s", PROPERTY_PACKAGE_ID);
		if((prop_val[0]=(char*)malloc(10)))	sprintf(prop_val[0], "%d", PROPERTY_MONITOR_MODE_OFF);
		if((prop_val[1]=(char*)malloc(10)))	sprintf(prop_val[1], "%d", PROPERTY_PACKAGE_ID_VAL);
		//fprintf(stderr, "_MACRO_PROPERTY_TWO_\n");
#endif
//		printf("%d, [LOGIN] OF NAME[%s]ip[%s]\n", time(NULL), name, ip);

		if (pgSCE_nbapi != NULL) {
			ipVal++;
			argHandle = SMNB_login(*pgSCE_nbapi,
									name, //subscriber name
#ifdef _MACRO_MAPPING_NULL_
									NULL,
#else
									&ip,            //a single ip mapping
#endif
									&type,
									1,
#ifdef _MACRO_PROPERTY_ONE_
									prop_key, //no properties
									prop_val,
									1,
#endif
#ifdef _MACRO_PROPERTY_TWO_
									prop_key, //no properties
									prop_val,
									2,
#endif
#ifdef _MACRO_PROPERTY_NULL_
									NULL,
									NULL,
									0,
#endif
									"subscribers", //domain
									0, //mappings are not additive
									-1); //disable auto-logout
			{
				Uint32 handle = GETHANDLE(argHandle);

				hlogin[handle].key[0] = prop_key[0];
				hlogin[handle].key[1] = prop_key[1];
				hlogin[handle].val[0] = prop_val[0];
				hlogin[handle].val[1] = prop_val[1];
			}
		}
	}
	printf("%d, [LOGIN] end1 \n", time(NULL));

	while (1) {
		usleep (1);
	}

	pthread_exit (NULL);
}


void *thread_login_three (void *arg)
{
	SMNB_HANDLE 	*pgSCE_nbapi = &gSCE_nbapi3;
	int				i;

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
	printf("Call SMNB_setReplyFailCallBack()\n");fflush(stdout);
	SMNB_setReplySuccessCallBack (*pgSCE_nbapi, handleSuccess);
	printf("AFTER SMNB_setReplySuccessCallBack()\n");fflush(stdout);

	// login
	char name[10];
	char ipString[15];
	char* ip = &(ipString[0]);
	char *prop_key[2] = { NULL,  NULL };
	char *prop_val[2] = { NULL,  NULL };

	Uint32 argHandle=0;
	MappingType type = IP_RANGE;
	Uint32 ipVal = 0x1c000000;

	printf("%d, [LOGIN] Start \n", time(NULL));
	for (i=0; i<numSubscribersToLogin; i++)
	{
		sprintf((char*)name,"011103%05d", i);
		sprintf((char*)ip,"%d.%d.%d.%d",
				(int)((ipVal & 0xFF000000) >> 24),
				(int)((ipVal & 0x00FF0000) >> 16),
				(int)((ipVal & 0x0000FF00) >> 8),
				(int)(ipVal & 0x000000FF));
#ifdef _MACRO_PROPERTY_ONE_
		if((prop_key[0]=(char*)malloc(10)))	sprintf(prop_key[0], "%s", PROPERTY_PACKAGE_ID);
		if((prop_val[0]=(char*)malloc(10)))	sprintf(prop_val[0], "%d", PROPERTY_PACKAGE_ID_VAL);
		//fprintf(stderr, "_MACRO_PROPERTY_ONE_\n");
#endif
#ifdef _MACRO_PROPERTY_TWO_
		if((prop_key[0]=(char*)malloc(10)))	sprintf(prop_key[0], "%s", PROPERTY_MONITOR);
		if((prop_key[1]=(char*)malloc(10)))	sprintf(prop_key[1], "%s", PROPERTY_PACKAGE_ID);
		if((prop_val[0]=(char*)malloc(10)))	sprintf(prop_val[0], "%d", PROPERTY_MONITOR_MODE_OFF);
		if((prop_val[1]=(char*)malloc(10)))	sprintf(prop_val[1], "%d", PROPERTY_PACKAGE_ID_VAL);
		//fprintf(stderr, "_MACRO_PROPERTY_TWO_\n");
#endif
//		printf("%d, [LOGIN] OF NAME[%s]ip[%s]\n", time(NULL), name, ip);

		if (pgSCE_nbapi != NULL) {
			ipVal++;
			argHandle = SMNB_login(*pgSCE_nbapi,
									name, //subscriber name
#ifdef _MACRO_MAPPING_NULL_
									NULL,
#else
									&ip,            //a single ip mapping
#endif
									&type,
									1,
#ifdef _MACRO_PROPERTY_ONE_
									prop_key, //no properties
									prop_val,
									1,
#endif
#ifdef _MACRO_PROPERTY_TWO_
									prop_key, //no properties
									prop_val,
									2,
#endif
#ifdef _MACRO_PROPERTY_NULL_
									NULL,
									NULL,
									0,
#endif
									"subscribers", //domain
									0, //mappings are not additive
									-1); //disable auto-logout
			{
				Uint32 handle = GETHANDLE(argHandle);

				hlogin[handle].key[0] = prop_key[0];
				hlogin[handle].key[1] = prop_key[1];
				hlogin[handle].val[0] = prop_val[0];
				hlogin[handle].val[1] = prop_val[1];
			}
		}
	}
	printf("%d, [LOGIN] end1 \n", time(NULL));

	while (1) {
		usleep (1);
	}

	pthread_exit (NULL);
}


void *thread_login_four (void *arg)
{
	SMNB_HANDLE 	*pgSCE_nbapi = &gSCE_nbapi4;
	int				i;

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
	printf("Call SMNB_setReplyFailCallBack()\n");fflush(stdout);
	SMNB_setReplySuccessCallBack (*pgSCE_nbapi, handleSuccess);
	printf("AFTER SMNB_setReplySuccessCallBack()\n");fflush(stdout);

	// login
	char name[10];
	char ipString[15];
	char* ip = &(ipString[0]);
	char *prop_key[2] = { NULL,  NULL };
	char *prop_val[2] = { NULL,  NULL };

	Uint32 argHandle=0;
	MappingType type = IP_RANGE;
	Uint32 ipVal = 0x1d000000;

	printf("%d, [LOGIN] Start \n", time(NULL));
	for (i=0; i<numSubscribersToLogin; i++)
	{
		sprintf((char*)name,"011104%05d", i);
		sprintf((char*)ip,"%d.%d.%d.%d",
				(int)((ipVal & 0xFF000000) >> 24),
				(int)((ipVal & 0x00FF0000) >> 16),
				(int)((ipVal & 0x0000FF00) >> 8),
				(int)(ipVal & 0x000000FF));
#ifdef _MACRO_PROPERTY_ONE_
		if((prop_key[0]=(char*)malloc(10)))	sprintf(prop_key[0], "%s", PROPERTY_PACKAGE_ID);
		if((prop_val[0]=(char*)malloc(10)))	sprintf(prop_val[0], "%d", PROPERTY_PACKAGE_ID_VAL);
		//fprintf(stderr, "_MACRO_PROPERTY_ONE_\n");
#endif
#ifdef _MACRO_PROPERTY_TWO_
		if((prop_key[0]=(char*)malloc(10)))	sprintf(prop_key[0], "%s", PROPERTY_MONITOR);
		if((prop_key[1]=(char*)malloc(10)))	sprintf(prop_key[1], "%s", PROPERTY_PACKAGE_ID);
		if((prop_val[0]=(char*)malloc(10)))	sprintf(prop_val[0], "%d", PROPERTY_MONITOR_MODE_OFF);
		if((prop_val[1]=(char*)malloc(10)))	sprintf(prop_val[1], "%d", PROPERTY_PACKAGE_ID_VAL);
		//fprintf(stderr, "_MACRO_PROPERTY_TWO_\n");
#endif
//		printf("%d, [LOGIN] OF NAME[%s]ip[%s]\n", time(NULL), name, ip);

		if (pgSCE_nbapi != NULL) {
			ipVal++;
			argHandle = SMNB_login(*pgSCE_nbapi,
									name, //subscriber name
#ifdef _MACRO_MAPPING_NULL_
									NULL,
#else
									&ip,            //a single ip mapping
#endif
									&type,
									1,
#ifdef _MACRO_PROPERTY_ONE_
									prop_key, //no properties
									prop_val,
									1,
#endif
#ifdef _MACRO_PROPERTY_TWO_
									prop_key, //no properties
									prop_val,
									2,
#endif
#ifdef _MACRO_PROPERTY_NULL_
									NULL,
									NULL,
									0,
#endif
									"subscribers", //domain
									0, //mappings are not additive
									-1); //disable auto-logout
			{
				Uint32 handle = GETHANDLE(argHandle);

				hlogin[handle].key[0] = prop_key[0];
				hlogin[handle].key[1] = prop_key[1];
				hlogin[handle].val[0] = prop_val[0];
				hlogin[handle].val[1] = prop_val[1];
			}
		}
	}
	printf("%d, [LOGIN] end1 \n", time(NULL));

	while (1) {
		usleep (1);
	}

	pthread_exit (NULL);
}


void *thread_login_five (void *arg)
{
	SMNB_HANDLE 	*pgSCE_nbapi = &gSCE_nbapi5;
	int				i;

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
	printf("Call SMNB_setReplyFailCallBack()\n");fflush(stdout);
	SMNB_setReplySuccessCallBack (*pgSCE_nbapi, handleSuccess);
	printf("AFTER SMNB_setReplySuccessCallBack()\n");fflush(stdout);

	// login
	char name[10];
	char ipString[15];
	char* ip = &(ipString[0]);
	char *prop_key[2] = { NULL,  NULL };
	char *prop_val[2] = { NULL,  NULL };

	Uint32 argHandle=0;
	MappingType type = IP_RANGE;
	Uint32 ipVal = 0x1e000000;

	printf("%d, [LOGIN] Start \n", time(NULL));
	for (i=0; i<numSubscribersToLogin; i++)
	{
		sprintf((char*)name,"011105%05d", i);
		sprintf((char*)ip,"%d.%d.%d.%d",
				(int)((ipVal & 0xFF000000) >> 24),
				(int)((ipVal & 0x00FF0000) >> 16),
				(int)((ipVal & 0x0000FF00) >> 8),
				(int)(ipVal & 0x000000FF));
#ifdef _MACRO_PROPERTY_ONE_
		if((prop_key[0]=(char*)malloc(10)))	sprintf(prop_key[0], "%s", PROPERTY_PACKAGE_ID);
		if((prop_val[0]=(char*)malloc(10)))	sprintf(prop_val[0], "%d", PROPERTY_PACKAGE_ID_VAL);
		//fprintf(stderr, "_MACRO_PROPERTY_ONE_\n");
#endif
#ifdef _MACRO_PROPERTY_TWO_
		if((prop_key[0]=(char*)malloc(10)))	sprintf(prop_key[0], "%s", PROPERTY_MONITOR);
		if((prop_key[1]=(char*)malloc(10)))	sprintf(prop_key[1], "%s", PROPERTY_PACKAGE_ID);
		if((prop_val[0]=(char*)malloc(10)))	sprintf(prop_val[0], "%d", PROPERTY_MONITOR_MODE_OFF);
		if((prop_val[1]=(char*)malloc(10)))	sprintf(prop_val[1], "%d", PROPERTY_PACKAGE_ID_VAL);
		//fprintf(stderr, "_MACRO_PROPERTY_TWO_\n");
#endif
//		printf("%d, [LOGIN] OF NAME[%s]ip[%s]\n", time(NULL), name, ip);

		if (pgSCE_nbapi != NULL) {
			ipVal++;
			argHandle = SMNB_login(*pgSCE_nbapi,
									name, //subscriber name
#ifdef _MACRO_MAPPING_NULL_
									NULL,
#else
									&ip,            //a single ip mapping
#endif
									&type,
									1,
#ifdef _MACRO_PROPERTY_ONE_
									prop_key, //no properties
									prop_val,
									1,
#endif
#ifdef _MACRO_PROPERTY_TWO_
									prop_key, //no properties
									prop_val,
									2,
#endif
#ifdef _MACRO_PROPERTY_NULL_
									NULL,
									NULL,
									0,
#endif
									"subscribers", //domain
									0, //mappings are not additive
									-1); //disable auto-logout
			{
				Uint32 handle = GETHANDLE(argHandle);

				hlogin[handle].key[0] = prop_key[0];
				hlogin[handle].key[1] = prop_key[1];
				hlogin[handle].val[0] = prop_val[0];
				hlogin[handle].val[1] = prop_val[1];
			}
		}
	}
	printf("%d, [LOGIN] end1 \n", time(NULL));

	while (1) {
		usleep (1);
	}

	pthread_exit (NULL);
}


void *thread_login_six (void *arg)
{
	SMNB_HANDLE 	*pgSCE_nbapi = &gSCE_nbapi6;
	int				i;

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
	printf("Call SMNB_setReplyFailCallBack()\n");fflush(stdout);
	SMNB_setReplySuccessCallBack (*pgSCE_nbapi, handleSuccess);
	printf("AFTER SMNB_setReplySuccessCallBack()\n");fflush(stdout);

	// login
	char name[10];
	char ipString[15];
	char* ip = &(ipString[0]);
	char *prop_key[2] = { NULL,  NULL };
	char *prop_val[2] = { NULL,  NULL };

	Uint32 argHandle=0;
	MappingType type = IP_RANGE;
	Uint32 ipVal = 0x1f000000;

	printf("%d, [LOGIN] Start \n", time(NULL));
	for (i=0; i<numSubscribersToLogin; i++)
	{
		sprintf((char*)name,"011106%05d", i);
		sprintf((char*)ip,"%d.%d.%d.%d",
				(int)((ipVal & 0xFF000000) >> 24),
				(int)((ipVal & 0x00FF0000) >> 16),
				(int)((ipVal & 0x0000FF00) >> 8),
				(int)(ipVal & 0x000000FF));
#ifdef _MACRO_PROPERTY_ONE_
		if((prop_key[0]=(char*)malloc(10)))	sprintf(prop_key[0], "%s", PROPERTY_PACKAGE_ID);
		if((prop_val[0]=(char*)malloc(10)))	sprintf(prop_val[0], "%d", PROPERTY_PACKAGE_ID_VAL);
		//fprintf(stderr, "_MACRO_PROPERTY_ONE_\n");
#endif
#ifdef _MACRO_PROPERTY_TWO_
		if((prop_key[0]=(char*)malloc(10)))	sprintf(prop_key[0], "%s", PROPERTY_MONITOR);
		if((prop_key[1]=(char*)malloc(10)))	sprintf(prop_key[1], "%s", PROPERTY_PACKAGE_ID);
		if((prop_val[0]=(char*)malloc(10)))	sprintf(prop_val[0], "%d", PROPERTY_MONITOR_MODE_OFF);
		if((prop_val[1]=(char*)malloc(10)))	sprintf(prop_val[1], "%d", PROPERTY_PACKAGE_ID_VAL);
		//fprintf(stderr, "_MACRO_PROPERTY_TWO_\n");
#endif
//		printf("%d, [LOGIN] OF NAME[%s]ip[%s]\n", time(NULL), name, ip);

		if (pgSCE_nbapi != NULL) {
			ipVal++;
			argHandle = SMNB_login(*pgSCE_nbapi,
									name, //subscriber name
#ifdef _MACRO_MAPPING_NULL_
									NULL,
#else
									&ip,            //a single ip mapping
#endif
									&type,
									1,
#ifdef _MACRO_PROPERTY_ONE_
									prop_key, //no properties
									prop_val,
									1,
#endif
#ifdef _MACRO_PROPERTY_TWO_
									prop_key, //no properties
									prop_val,
									2,
#endif
#ifdef _MACRO_PROPERTY_NULL_
									NULL,
									NULL,
									0,
#endif
									"subscribers", //domain
									0, //mappings are not additive
									-1); //disable auto-logout
			{
				Uint32 handle = GETHANDLE(argHandle);

				hlogin[handle].key[0] = prop_key[0];
				hlogin[handle].key[1] = prop_key[1];
				hlogin[handle].val[0] = prop_val[0];
				hlogin[handle].val[1] = prop_val[1];
			}
		}
	}
	printf("%d, [LOGIN] end1 \n", time(NULL));

	while (1) {
		usleep (1);
	}

	pthread_exit (NULL);
}

void *thread_login_7 (void *arg)
{
	SMNB_HANDLE 	*pgSCE_nbapi = &gSCE_nbapi7;
	int				i;

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
	printf("Call SMNB_setReplyFailCallBack()\n");fflush(stdout);
	SMNB_setReplySuccessCallBack (*pgSCE_nbapi, handleSuccess);
	printf("AFTER SMNB_setReplySuccessCallBack()\n");fflush(stdout);

	// login
	char name[10];
	char ipString[15];
	char* ip = &(ipString[0]);
	char *prop_key[2] = { NULL,  NULL };
	char *prop_val[2] = { NULL,  NULL };

	Uint32 argHandle=0;
	MappingType type = IP_RANGE;
	Uint32 ipVal = 0x20000000;

	printf("%d, [LOGIN] Start \n", time(NULL));
	for (i=0; i<numSubscribersToLogin; i++)
	{
		sprintf((char*)name,"011107%05d", i);
		sprintf((char*)ip,"%d.%d.%d.%d",
				(int)((ipVal & 0xFF000000) >> 24),
				(int)((ipVal & 0x00FF0000) >> 16),
				(int)((ipVal & 0x0000FF00) >> 8),
				(int)(ipVal & 0x000000FF));
#ifdef _MACRO_PROPERTY_ONE_
		if((prop_key[0]=(char*)malloc(10)))	sprintf(prop_key[0], "%s", PROPERTY_PACKAGE_ID);
		if((prop_val[0]=(char*)malloc(10)))	sprintf(prop_val[0], "%d", PROPERTY_PACKAGE_ID_VAL);
		//fprintf(stderr, "_MACRO_PROPERTY_ONE_\n");
#endif
#ifdef _MACRO_PROPERTY_TWO_
		if((prop_key[0]=(char*)malloc(10)))	sprintf(prop_key[0], "%s", PROPERTY_MONITOR);
		if((prop_key[1]=(char*)malloc(10)))	sprintf(prop_key[1], "%s", PROPERTY_PACKAGE_ID);
		if((prop_val[0]=(char*)malloc(10)))	sprintf(prop_val[0], "%d", PROPERTY_MONITOR_MODE_OFF);
		if((prop_val[1]=(char*)malloc(10)))	sprintf(prop_val[1], "%d", PROPERTY_PACKAGE_ID_VAL);
		//fprintf(stderr, "_MACRO_PROPERTY_TWO_\n");
#endif
//		printf("%d, [LOGIN] OF NAME[%s]ip[%s]\n", time(NULL), name, ip);

		if (pgSCE_nbapi != NULL) {
			ipVal++;
			argHandle = SMNB_login(*pgSCE_nbapi,
									name, //subscriber name
#ifdef _MACRO_MAPPING_NULL_
									NULL,
#else
									&ip,            //a single ip mapping
#endif
									&type,
									1,
#ifdef _MACRO_PROPERTY_ONE_
									prop_key, //no properties
									prop_val,
									1,
#endif
#ifdef _MACRO_PROPERTY_TWO_
									prop_key, //no properties
									prop_val,
									2,
#endif
#ifdef _MACRO_PROPERTY_NULL_
									NULL,
									NULL,
									0,
#endif
									"subscribers", //domain
									0, //mappings are not additive
									-1); //disable auto-logout
			{
				Uint32 handle = GETHANDLE(argHandle);

				hlogin[handle].key[0] = prop_key[0];
				hlogin[handle].key[1] = prop_key[1];
				hlogin[handle].val[0] = prop_val[0];
				hlogin[handle].val[1] = prop_val[1];
			}
		}
	}
	printf("%d, [LOGIN] end1 \n", time(NULL));

	while (1) {
		usleep (1);
	}

	pthread_exit (NULL);
}


void *thread_login_8 (void *arg)
{
	SMNB_HANDLE 	*pgSCE_nbapi = &gSCE_nbapi8;
	int				i;

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
	printf("Call SMNB_setReplyFailCallBack()\n");fflush(stdout);
	SMNB_setReplySuccessCallBack (*pgSCE_nbapi, handleSuccess);
	printf("AFTER SMNB_setReplySuccessCallBack()\n");fflush(stdout);

	// login
	char name[10];
	char ipString[15];
	char* ip = &(ipString[0]);
	char *prop_key[2] = { NULL,  NULL };
	char *prop_val[2] = { NULL,  NULL };

	Uint32 argHandle=0;
	MappingType type = IP_RANGE;
	Uint32 ipVal = 0x21000000;

	printf("%d, [LOGIN] Start \n", time(NULL));
	for (i=0; i<numSubscribersToLogin; i++)
	{
		sprintf((char*)name,"011108%05d", i);
		sprintf((char*)ip,"%d.%d.%d.%d",
				(int)((ipVal & 0xFF000000) >> 24),
				(int)((ipVal & 0x00FF0000) >> 16),
				(int)((ipVal & 0x0000FF00) >> 8),
				(int)(ipVal & 0x000000FF));
#ifdef _MACRO_PROPERTY_ONE_
		if((prop_key[0]=(char*)malloc(10)))	sprintf(prop_key[0], "%s", PROPERTY_PACKAGE_ID);
		if((prop_val[0]=(char*)malloc(10)))	sprintf(prop_val[0], "%d", PROPERTY_PACKAGE_ID_VAL);
		//fprintf(stderr, "_MACRO_PROPERTY_ONE_\n");
#endif
#ifdef _MACRO_PROPERTY_TWO_
		if((prop_key[0]=(char*)malloc(10)))	sprintf(prop_key[0], "%s", PROPERTY_MONITOR);
		if((prop_key[1]=(char*)malloc(10)))	sprintf(prop_key[1], "%s", PROPERTY_PACKAGE_ID);
		if((prop_val[0]=(char*)malloc(10)))	sprintf(prop_val[0], "%d", PROPERTY_MONITOR_MODE_OFF);
		if((prop_val[1]=(char*)malloc(10)))	sprintf(prop_val[1], "%d", PROPERTY_PACKAGE_ID_VAL);
		//fprintf(stderr, "_MACRO_PROPERTY_TWO_\n");
#endif
//		printf("%d, [LOGIN] OF NAME[%s]ip[%s]\n", time(NULL), name, ip);

		if (pgSCE_nbapi != NULL) {
			ipVal++;
			argHandle = SMNB_login(*pgSCE_nbapi,
									name, //subscriber name
#ifdef _MACRO_MAPPING_NULL_
									NULL,
#else
									&ip,            //a single ip mapping
#endif
									&type,
									1,
#ifdef _MACRO_PROPERTY_ONE_
									prop_key, //no properties
									prop_val,
									1,
#endif
#ifdef _MACRO_PROPERTY_TWO_
									prop_key, //no properties
									prop_val,
									2,
#endif
#ifdef _MACRO_PROPERTY_NULL_
									NULL,
									NULL,
									0,
#endif
									"subscribers", //domain
									0, //mappings are not additive
									-1); //disable auto-logout
			{
				Uint32 handle = GETHANDLE(argHandle);

				hlogin[handle].key[0] = prop_key[0];
				hlogin[handle].key[1] = prop_key[1];
				hlogin[handle].val[0] = prop_val[0];
				hlogin[handle].val[1] = prop_val[1];
			}
		}
	}
	printf("%d, [LOGIN] end1 \n", time(NULL));

	while (1) {
		usleep (1);
	}

	pthread_exit (NULL);
}


void *thread_login_9 (void *arg)
{
	SMNB_HANDLE 	*pgSCE_nbapi = &gSCE_nbapi9;
	int				i;

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
	printf("Call SMNB_setReplyFailCallBack()\n");fflush(stdout);
	SMNB_setReplySuccessCallBack (*pgSCE_nbapi, handleSuccess);
	printf("AFTER SMNB_setReplySuccessCallBack()\n");fflush(stdout);

	// login
	char name[10];
	char ipString[15];
	char* ip = &(ipString[0]);
	char *prop_key[2] = { NULL,  NULL };
	char *prop_val[2] = { NULL,  NULL };

	Uint32 argHandle=0;
	MappingType type = IP_RANGE;
	Uint32 ipVal = 0x22000000;

	printf("%d, [LOGIN] Start \n", time(NULL));
	for (i=0; i<numSubscribersToLogin; i++)
	{
		sprintf((char*)name,"011109%05d", i);
		sprintf((char*)ip,"%d.%d.%d.%d",
				(int)((ipVal & 0xFF000000) >> 24),
				(int)((ipVal & 0x00FF0000) >> 16),
				(int)((ipVal & 0x0000FF00) >> 8),
				(int)(ipVal & 0x000000FF));
#ifdef _MACRO_PROPERTY_ONE_
		if((prop_key[0]=(char*)malloc(10)))	sprintf(prop_key[0], "%s", PROPERTY_PACKAGE_ID);
		if((prop_val[0]=(char*)malloc(10)))	sprintf(prop_val[0], "%d", PROPERTY_PACKAGE_ID_VAL);
		//fprintf(stderr, "_MACRO_PROPERTY_ONE_\n");
#endif
#ifdef _MACRO_PROPERTY_TWO_
		if((prop_key[0]=(char*)malloc(10)))	sprintf(prop_key[0], "%s", PROPERTY_MONITOR);
		if((prop_key[1]=(char*)malloc(10)))	sprintf(prop_key[1], "%s", PROPERTY_PACKAGE_ID);
		if((prop_val[0]=(char*)malloc(10)))	sprintf(prop_val[0], "%d", PROPERTY_MONITOR_MODE_OFF);
		if((prop_val[1]=(char*)malloc(10)))	sprintf(prop_val[1], "%d", PROPERTY_PACKAGE_ID_VAL);
		//fprintf(stderr, "_MACRO_PROPERTY_TWO_\n");
#endif
//		printf("%d, [LOGIN] OF NAME[%s]ip[%s]\n", time(NULL), name, ip);

		if (pgSCE_nbapi != NULL) {
			ipVal++;
			argHandle = SMNB_login(*pgSCE_nbapi,
									name, //subscriber name
#ifdef _MACRO_MAPPING_NULL_
									NULL,
#else
									&ip,            //a single ip mapping
#endif
									&type,
									1,
#ifdef _MACRO_PROPERTY_ONE_
									prop_key, //no properties
									prop_val,
									1,
#endif
#ifdef _MACRO_PROPERTY_TWO_
									prop_key, //no properties
									prop_val,
									2,
#endif
#ifdef _MACRO_PROPERTY_NULL_
									NULL,
									NULL,
									0,
#endif
									"subscribers", //domain
									0, //mappings are not additive
									-1); //disable auto-logout
			{
				Uint32 handle = GETHANDLE(argHandle);

				hlogin[handle].key[0] = prop_key[0];
				hlogin[handle].key[1] = prop_key[1];
				hlogin[handle].val[0] = prop_val[0];
				hlogin[handle].val[1] = prop_val[1];
			}
		}
	}
	printf("%d, [LOGIN] end1 \n", time(NULL));

	while (1) {
		usleep (1);
	}

	pthread_exit (NULL);
}


void *thread_login_10 (void *arg)
{
	SMNB_HANDLE 	*pgSCE_nbapi = &gSCE_nbapi10;
	int				i;

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
	printf("Call SMNB_setReplyFailCallBack()\n");fflush(stdout);
	SMNB_setReplySuccessCallBack (*pgSCE_nbapi, handleSuccess);
	printf("AFTER SMNB_setReplySuccessCallBack()\n");fflush(stdout);

	// login
	char name[10];
	char ipString[15];
	char* ip = &(ipString[0]);
	char *prop_key[2] = { NULL,  NULL };
	char *prop_val[2] = { NULL,  NULL };

	Uint32 argHandle=0;
	MappingType type = IP_RANGE;
	Uint32 ipVal = 0x23000000;

	printf("%d, [LOGIN] Start \n", time(NULL));
	for (i=0; i<numSubscribersToLogin; i++)
	{
		sprintf((char*)name,"011110%05d", i);
		sprintf((char*)ip,"%d.%d.%d.%d",
				(int)((ipVal & 0xFF000000) >> 24),
				(int)((ipVal & 0x00FF0000) >> 16),
				(int)((ipVal & 0x0000FF00) >> 8),
				(int)(ipVal & 0x000000FF));
#ifdef _MACRO_PROPERTY_ONE_
		if((prop_key[0]=(char*)malloc(10)))	sprintf(prop_key[0], "%s", PROPERTY_PACKAGE_ID);
		if((prop_val[0]=(char*)malloc(10)))	sprintf(prop_val[0], "%d", PROPERTY_PACKAGE_ID_VAL);
		//fprintf(stderr, "_MACRO_PROPERTY_ONE_\n");
#endif
#ifdef _MACRO_PROPERTY_TWO_
		if((prop_key[0]=(char*)malloc(10)))	sprintf(prop_key[0], "%s", PROPERTY_MONITOR);
		if((prop_key[1]=(char*)malloc(10)))	sprintf(prop_key[1], "%s", PROPERTY_PACKAGE_ID);
		if((prop_val[0]=(char*)malloc(10)))	sprintf(prop_val[0], "%d", PROPERTY_MONITOR_MODE_OFF);
		if((prop_val[1]=(char*)malloc(10)))	sprintf(prop_val[1], "%d", PROPERTY_PACKAGE_ID_VAL);
		//fprintf(stderr, "_MACRO_PROPERTY_TWO_\n");
#endif
//		printf("%d, [LOGIN] OF NAME[%s]ip[%s]\n", time(NULL), name, ip);

		if (pgSCE_nbapi != NULL) {
			ipVal++;
			argHandle = SMNB_login(*pgSCE_nbapi,
									name, //subscriber name
#ifdef _MACRO_MAPPING_NULL_
									NULL,
#else
									&ip,            //a single ip mapping
#endif
									&type,
									1,
#ifdef _MACRO_PROPERTY_ONE_
									prop_key, //no properties
									prop_val,
									1,
#endif
#ifdef _MACRO_PROPERTY_TWO_
									prop_key, //no properties
									prop_val,
									2,
#endif
#ifdef _MACRO_PROPERTY_NULL_
									NULL,
									NULL,
									0,
#endif
									"subscribers", //domain
									0, //mappings are not additive
									-1); //disable auto-logout
			{
				Uint32 handle = GETHANDLE(argHandle);

				hlogin[handle].key[0] = prop_key[0];
				hlogin[handle].key[1] = prop_key[1];
				hlogin[handle].val[0] = prop_val[0];
				hlogin[handle].val[1] = prop_val[1];
			}
		}
	}
	printf("%d, [LOGIN] end1 \n", time(NULL));

	while (1) {
		usleep (1);
	}

	pthread_exit (NULL);
}



void thread_ping_test (void) {

	pthread_attr_t  thrAttr, thrAttr2, thrAttr3, thrAttr4, thrAttr5, thrAttr6;
	pthread_attr_t  thrAttr7, thrAttr8, thrAttr9, thrAttr10;
	pthread_t   thrId, thrId2, thrId3, thrId4, thrId5, thrId6;
	pthread_t   thrId7, thrId8, thrId9, thrId10;
	int ret;

	pthread_attr_init ( &thrAttr);
	pthread_attr_setscope ( &thrAttr, PTHREAD_SCOPE_SYSTEM);
	pthread_attr_setdetachstate ( &thrAttr, PTHREAD_CREATE_DETACHED);
	if ((ret = pthread_create (&thrId, &thrAttr, thread_login_one, NULL)) != 0) {
		fprintf(stderr,"[thread_ping_test] pthread_create fail\n" );
//		trclib_writeLogErr (FL,trcBuf);
	}

#if 0
	pthread_attr_init ( &thrAttr2);
	pthread_attr_setscope ( &thrAttr2, PTHREAD_SCOPE_SYSTEM);
	pthread_attr_setdetachstate ( &thrAttr2, PTHREAD_CREATE_DETACHED);
	if ((ret = pthread_create (&thrId2, &thrAttr2, thread_login_two, NULL)) != 0) {
		fprintf(stderr,"[thread_ping_test] pthread_create fail\n" );
//		trclib_writeLogErr (FL,trcBuf);
	}

	pthread_attr_init ( &thrAttr3);
	pthread_attr_setscope ( &thrAttr3, PTHREAD_SCOPE_SYSTEM);
	pthread_attr_setdetachstate ( &thrAttr3, PTHREAD_CREATE_DETACHED);
	if ((ret = pthread_create (&thrId3, &thrAttr3, thread_login_three, NULL)) != 0) {
		fprintf(stderr,"[thread_ping_test] pthread_create fail\n" );
//		trclib_writeLogErr (FL,trcBuf);
	}

	pthread_attr_init ( &thrAttr4);
	pthread_attr_setscope ( &thrAttr4, PTHREAD_SCOPE_SYSTEM);
	pthread_attr_setdetachstate ( &thrAttr4, PTHREAD_CREATE_DETACHED);
	if ((ret = pthread_create (&thrId4, &thrAttr4, thread_login_four, NULL)) != 0) {
		fprintf(stderr,"[thread_ping_test] pthread_create fail\n" );
//		trclib_writeLogErr (FL,trcBuf);
	}

	pthread_attr_init ( &thrAttr5);
	pthread_attr_setscope ( &thrAttr5, PTHREAD_SCOPE_SYSTEM);
	pthread_attr_setdetachstate ( &thrAttr5, PTHREAD_CREATE_DETACHED);
	if ((ret = pthread_create (&thrId5, &thrAttr5, thread_login_five, NULL)) != 0) {
		fprintf(stderr,"[thread_ping_test] pthread_create fail\n" );
//		trclib_writeLogErr (FL,trcBuf);
	}

	pthread_attr_init ( &thrAttr6);
	pthread_attr_setscope ( &thrAttr6, PTHREAD_SCOPE_SYSTEM);
	pthread_attr_setdetachstate ( &thrAttr6, PTHREAD_CREATE_DETACHED);
	if ((ret = pthread_create (&thrId6, &thrAttr6, thread_login_six, NULL)) != 0) {
		fprintf(stderr,"[thread_ping_test] pthread_create fail\n" );
	}

	pthread_attr_init ( &thrAttr7);
	pthread_attr_setscope ( &thrAttr7, PTHREAD_SCOPE_SYSTEM);
	pthread_attr_setdetachstate ( &thrAttr7, PTHREAD_CREATE_DETACHED);
	if ((ret = pthread_create (&thrId7, &thrAttr7, thread_login_7, NULL)) != 0) {
		fprintf(stderr,"[thread_ping_test] pthread_create fail\n" );
	}

	pthread_attr_init ( &thrAttr8);
	pthread_attr_setscope ( &thrAttr8, PTHREAD_SCOPE_SYSTEM);
	pthread_attr_setdetachstate ( &thrAttr8, PTHREAD_CREATE_DETACHED);
	if ((ret = pthread_create (&thrId8, &thrAttr8, thread_login_8, NULL)) != 0) {
		fprintf(stderr,"[thread_ping_test] pthread_create fail\n" );
	}

	pthread_attr_init ( &thrAttr9);
	pthread_attr_setscope ( &thrAttr9, PTHREAD_SCOPE_SYSTEM);
	pthread_attr_setdetachstate ( &thrAttr9, PTHREAD_CREATE_DETACHED);
	if ((ret = pthread_create (&thrId9, &thrAttr9, thread_login_9, NULL)) != 0) {
		fprintf(stderr,"[thread_ping_test] pthread_create fail\n" );
	}

	pthread_attr_init ( &thrAttr10);
	pthread_attr_setscope ( &thrAttr10, PTHREAD_SCOPE_SYSTEM);
	pthread_attr_setdetachstate ( &thrAttr10, PTHREAD_CREATE_DETACHED);
	if ((ret = pthread_create (&thrId10, &thrAttr10, thread_login_10, NULL)) != 0) {
		fprintf(stderr,"[thread_ping_test] pthread_create fail\n" );
	}
#endif
}





int main (int argc, char* argv[])
{
	int i;
	SMNB_HANDLE 	*pgSCE_nbapi = &gSCE_nbapi;

	if (argc != 2) {
		printf ( "usage: %s <테스트로그인 개수>\n", argv[0]);
		exit(1);
	}

	numSubscribersToLogin = atoi(argv[1]);

	thread_ping_test ();






#if 0

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
	printf("Call SMNB_setReplyFailCallBack()\n");fflush(stdout);
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
#		sprintf((char*)name,"s%d", i);
		sprintf((char*)name,"010123%05d", i);
		sprintf((char*)ip,"%d.%d.%d.%d",
				(int)((ipVal & 0xFF000000) >> 24),
				(int)((ipVal & 0x00FF0000) >> 16),
				(int)((ipVal & 0x0000FF00) >> 8),
				(int)(ipVal & 0x000000FF));

//		printf("%d, [LOGIN] OF NAME[%s]ip[%s]\n", time(NULL), name, ip);

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
#endif

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
//	SMNB_disconnect(*pgSCE_nbapi);
//	SMNB_release(*pgSCE_nbapi);

	return 0;
}
