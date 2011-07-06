#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define MAX_THREAD_CNT 					6
#define MAX_REQUEST_CNT 				MAX_THREAD_CNT
#define MAX_LOOP_COUNT					100000

///////THREAD 관리하기 위한 
typedef struct _st_pthread{
	pthread_t 		pthread_id;
	pthread_mutex_t pthread_mutex;
	char			stop;
	int			    arg;

	time_t			tStart;
	time_t			tLast;
} st_Pthread, *pst_Pthread;

typedef int S32 ;

// LOG 용
#define dAppLog   fprintf
#define LOG_DEBUG stdout
#define LOG_WARN  stdout
#define LOG_INFO  stdout
#define LOG_CRI   stdout

#define LP "%s.%d:%s "
#define LT __FILE__,__LINE__,__FUNCTION__
#define EP ", error=%d:%s"
#define ET errno,strerror(errno)
#define LOGLN dAppLog(LOG_INFO,"\n")

//global
st_Pthread stPthread[MAX_THREAD_CNT];

void sigHandler(int sign)
{
	int       i;
	pthread_t ptid;

	ptid = pthread_self();

	for( i = 0; i< MAX_THREAD_CNT; i++ ){
		if( stPthread[i].pthread_id == ptid ){
			dAppLog(LOG_DEBUG, LP"  RCVED SIGNAL(=%d) INTO THREAD[%d]", LT, sign, i);
			LOGLN;
			break;
		}
	}
	
	if( i >= MAX_THREAD_CNT ){
		dAppLog(LOG_CRI, LP"  RCVED SIGNAL(=%d) INTO THREAD OVER(MAX=%d)", LT, sign, MAX_THREAD_CNT);
		LOGLN;
		exit(-1);
	}

	stPthread[i].arg = -5;

	//pthread_mutex_unlock(&stPthread[i].pthread_mutex);
	pthread_exit(&stPthread[i].arg);
}

void sigWaitHandler(int sign)
{
	int       i;
	pthread_t ptid;

	ptid = pthread_self();

	for( i = 0; i< MAX_THREAD_CNT; i++ ){
		if( stPthread[i].pthread_id == ptid ){
			dAppLog(LOG_DEBUG, LP"  RCVED SIGNAL(=%d) INTO THREAD(MAX=%d)", LT, sign, MAX_THREAD_CNT);
			LOGLN;
			break;
		}
	}
	
	if( i >= MAX_THREAD_CNT ){
		dAppLog(LOG_CRI, LP"  RCVED SIGNAL(=%d) INTO THREAD OVER(MAX=%d)", LT, sign, MAX_THREAD_CNT);
		LOGLN;
		exit(-1);
	}

//	pthread_mutex_unlock(&stPthread[i].pthread_mutex);
	sleep(2);
}

int thread_test(int tid)
{
	struct timeval stv;
	struct tm stm;
	gettimeofday(&stv, NULL);
	localtime_r(&stv.tv_sec, &stm);
	dAppLog(LOG_INFO,"thread alive=%d, %02d:%02d:%02d.%ld", tid, stm.tm_hour, stm.tm_min, stm.tm_sec, stv.tv_usec);
	LOGLN;
	sleep(1);
}

void *packet_merge_init(void *arg)
{
	struct sigaction sapthrd, sapthrd_wait;
	S32	tid, *pdRet, dRet;
	//U32	loop = 0;

	pdRet = (int*)arg;
	tid  = *pdRet;

	*pdRet             = 0;
	sapthrd.sa_handler = sigHandler;

	if( sigemptyset(&sapthrd.sa_mask) == -1 ){
		*pdRet = -1;
		pthread_exit(arg);
	}

	sapthrd.sa_flags = 0;
	if( sigaction(SIGUSR1, &sapthrd, NULL) == -1 ){
		*pdRet = -2;
		pthread_exit(arg);
	}

	sapthrd_wait.sa_handler = sigWaitHandler;
	if( sigemptyset(&sapthrd_wait.sa_mask) == -1 ){
		*pdRet = -3;
		pthread_exit(arg);
	}

	sapthrd.sa_flags = 0;
	if( sigaction(SIGUSR2, &sapthrd_wait, NULL) == -1 ){
		*pdRet = -4;
		pthread_exit(arg);
	}

	while (!stPthread[tid].stop) {
		thread_test(tid);
	}

	return arg;
}

int each_thread_init(time_t now, int tid)
{
	int dRet;

	pthread_mutex_init(&stPthread[tid].pthread_mutex, NULL);
	stPthread[tid].stop = 0;
	stPthread[tid].arg  = tid;

	dRet = pthread_create(&stPthread[tid].pthread_id, NULL, packet_merge_init, (void*)&stPthread[tid].arg);
	if( dRet != 0 ){
		dAppLog(LOG_CRI, LP"FAILED IN pthread_create(=%d)"EP, LT, tid, ET);
		LOGLN;
		return -1;
	}
	stPthread[tid].tLast = now;
	dAppLog(LOG_CRI, LP"  SUCCESS IN pthread_create(=%d:%ld).", LT, tid, stPthread[tid].tLast);
	LOGLN;

	return 0;
}

int dInitThread()
{
	int    i, dRet;
	time_t now;

	now = time(NULL);
	for( i = 0; i < MAX_THREAD_CNT; i++ ){
		dRet = each_thread_init(now,i);
		if( dRet < 0 ){
			return dRet;
		}
	}
	
	
	return 0;
}

int dCheckThread()
{
	int i, dRet;
	void *status;
	time_t now;

	now = time(NULL);
	for( i = 0; i < MAX_THREAD_CNT; i++ ){
		//dAppLog(LOG_INFO,"MAIN THREAD, THREAD_ID[%d].status(arg)=%d",i, stPthread[i].arg);
		//LOGLN;

		if( stPthread[i].arg < 0 ){
			dAppLog(LOG_WARN,"THREAD[%d].STATUS IS DEADED. STATUS=%d", i, stPthread[i].arg);
			LOGLN;

			dRet = pthread_join(stPthread[i].pthread_id, &(status));
			if( dRet != 0 ){
				dAppLog(LOG_CRI, LP"FAILED IN pthread_join(=%d)"EP,LT,i,ET);
				LOGLN;
				continue;
			}

			dRet = each_thread_init(now, i);
			if( dRet < 0 ){
				dAppLog(LOG_CRI,LP"FAILED IN each_thread_init(time=%ld, id=%d)",LT,now,i);
				LOGLN;
				continue;
			}
			dAppLog(LOG_INFO,"THREAD[%d].STATUS IS ALIVE. STATUS=%d", i, stPthread[i].arg);
			LOGLN;
		}
	}
}

int main()
{
	char ch;
	int  i, id, dRet;
	memset( stPthread, 0x00, sizeof(stPthread) );
	dRet = dInitThread();
	if( dRet < 0 ){
		dAppLog(LOG_CRI,"FAILED IN dInitThread. dRet=%d", dRet);
		exit(-1);
	}

	
	i = 0;
	id = 0;
	id =  id % MAX_THREAD_CNT;
	dAppLog(LOG_INFO,"What do you want to killed thread?(0~%d)",MAX_THREAD_CNT);
	ch = getchar();
	id = (int)(ch - 48);
	dAppLog(LOG_INFO,"kill thread no = %d",id);
	LOGLN;

	dRet = pthread_kill(stPthread[id].pthread_id, SIGUSR1);
	if( dRet != 0 ){
		dAppLog(LOG_CRI,"FAILED IN pthread_kill[%d], dRet=%d",id, dRet);
		LOGLN;
		return -1;
	}else{
		sleep(1);
	}
	while(1){
	/*
	*/
		

		dCheckThread();
		i++;
		sleep(1);
	}

	return 0;
}
