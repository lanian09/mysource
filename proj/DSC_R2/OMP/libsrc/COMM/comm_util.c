#include "comm_util.h"


/*------------------------------------------------------------------------------
------------------------------------------------------------------------------*/
void commlib_convByteOrd (char *p, int siz)
{
	char	buff[80];
	int		i;

	for (i=0; i<siz; i++)
		buff[siz-i-1] = p[i];
	memcpy(p,buff,siz);
} /** End of commlib_convByteOrd **/



/*------------------------------------------------------------------------------
------------------------------------------------------------------------------*/
void commlib_microSleep (int usec)
{
	struct timeval	sleepTmr;
	if(usec <= 0) return;

//	if(usec > 1000000) sleepTmr.tv_sec  = usec / 1000000;
	if(usec >= 1000000) sleepTmr.tv_sec  = usec / 1000000;  // sjjeon
	else sleepTmr.tv_sec  = 0;

	if(usec > 0) sleepTmr.tv_usec  = usec % 1000000;
	else sleepTmr.tv_usec  = 0;

	//sleepTmr.tv_sec  = usec / 1000000;
	//sleepTmr.tv_usec = usec % 1000000;
	select(0,0,0,0,&sleepTmr);
	return;
} /** End of commlib_microSleep **/



/*------------------------------------------------------------------------------
------------------------------------------------------------------------------*/
void commlib_setupSignals (int *notMaskSig)
{
	int		i, flag, *ptr;

	signal(SIGINT,  commlib_quitSignal);
	signal(SIGTERM, commlib_quitSignal);

	signal(SIGHUP,   commlib_ignoreSignal);
	signal(SIGPIPE,  commlib_ignoreSignal);
	signal(SIGALRM,  commlib_ignoreSignal);

#ifdef TRU64
	for (i=16; i<=SIGMAX; i++)
#else
	for (i=16; i<=MAXSIG; i++)
#endif
	{
		// catch하지 말아야 할 놈으로 지정되었는지 확인한다.
		for (ptr = notMaskSig, flag=0; (ptr != NULL && *ptr != 0); ptr++) {
			if (*ptr == i) {
				flag = 1;
			}
		}
		if (flag) continue;

		signal(i, commlib_ignoreSignal);
	}

	return;

} /** End of commlib_setupSignals **/



/*------------------------------------------------------------------------------
------------------------------------------------------------------------------*/
void commlib_quitSignal (int signo)
{
	char	buff[256];
	trclib_writeLogErr (FL,">>> terminated by user request \n");
	exit(1);
} /** End of commlib_quitSignal **/



/*------------------------------------------------------------------------------
------------------------------------------------------------------------------*/
void commlib_ignoreSignal (int signo)
{
	signal (signo, commlib_ignoreSignal);
	return;
} /** End of commlib_quitSignal **/




static char commlib_timeStamp[32];
/*------------------------------------------------------------------------------
------------------------------------------------------------------------------*/
char *commlib_printTime (void)
{
	time_t	now;
	struct tm	*pLocalTime;

	now = time(0);
	if ((pLocalTime = (struct tm*)localtime((time_t*)&now)) == NULL) {
		strcpy(commlib_timeStamp,"");
	} else {
		strftime(commlib_timeStamp, 32, "%T", pLocalTime);
	}
	return ((char*)commlib_timeStamp);
} /** End of commlib_printTime **/



/*------------------------------------------------------------------------------
------------------------------------------------------------------------------*/
char *commlib_printTime_usec (void)
{
	struct timeval  now;
	struct tm	*pLocalTime;
	char	tmp[32];

	gettimeofday (&now, NULL);
	if ((pLocalTime = (struct tm*)localtime((time_t*)&now.tv_sec)) == NULL) {
		strcpy(commlib_timeStamp,"");
	} else {
		strftime(tmp, 32, "%T", pLocalTime);
		sprintf (commlib_timeStamp, "%s.%03d", tmp, (now.tv_usec/1000));
	}
	return ((char*)commlib_timeStamp);
} /** End of commlib_printTime_usec **/



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
char *commlib_printTStamp (void)
{
#if 0 /* jhnoh : 030812 */
	time_t		now;
	struct tm	*pLocalTime;

	now = time(0);

	if ((pLocalTime = (struct tm*)localtime((time_t*)&now)) == NULL) {
		strcpy (commlib_timeStamp,"");
	} else {
		strftime (commlib_timeStamp, 32, "%Y-%m-%d %T %a", pLocalTime);
		commlib_timeStamp[21] = toupper(commlib_timeStamp[21]);
		commlib_timeStamp[22] = toupper(commlib_timeStamp[22]);
	}

	return ((char*)commlib_timeStamp);
#else 
	struct timeval now;
	struct tm   *pLocalTime;

	char tmp[32];

	gettimeofday (&now, NULL);

	if ((pLocalTime = (struct tm*)localtime((time_t*)&now.tv_sec)) == NULL) {
		strcpy (commlib_timeStamp,"");
	} else {
		strftime (tmp, 32, "%Y-%m-%d %T", pLocalTime);
		sprintf(commlib_timeStamp, "%s.%02d", tmp, (now.tv_usec/10000));
	}

    return ((char*)commlib_timeStamp);

#endif
} //----- End of commlib_printTStamp -----//



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
char *commlib_printDateTime (time_t when)
{
	time_t		now;
	struct tm	*pLocalTime;

	now = time(0);

	if ((pLocalTime = (struct tm*)localtime((time_t*)&when)) == NULL) {
		strcpy (commlib_timeStamp,"");
	} else {
		strftime (commlib_timeStamp, 32, "%Y-%m-%d %T", pLocalTime);
	}

	return ((char*)commlib_timeStamp);

} //----- End of commlib_printDateTime -----//
