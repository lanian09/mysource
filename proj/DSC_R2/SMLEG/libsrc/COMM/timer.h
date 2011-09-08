#ifndef TIMER_H
#define TIMER_H

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TIMER_LOCK          pthread_mutex_lock(&timer_cond_mutex)
#define TIMER_UNLOCK        pthread_mutex_unlock(&timer_cond_mutex)
extern pthread_mutex_t timer_cond_mutex;

extern pthread_t t_timer;
extern int 	set_timeout(unsigned short port,char* key,int len,int msec);
extern void del_timeout(char* key,int len);
extern int 	set_cb_timeout(void (*func)(char*),void *data,uint32_t msec);
extern void del_cb_timeout(int tid);

#ifdef __cplusplus
}
#endif

#endif
