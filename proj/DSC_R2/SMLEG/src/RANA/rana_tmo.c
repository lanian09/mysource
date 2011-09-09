#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/timeb.h>
#include <pthread.h>

pthread_t t_timer;
int callbacked=0;

typedef struct hash {
    struct hash	**prev;
    struct hash	*next;

	struct timeb t;		// t
	int  k;			// cb key
	void *d;		// cb parameter
	void (*fn)(int, void*);	// cb function; if timer expired then func(d) will be called
} hash_t;

hash_t *hash[1] = { NULL, };


void _hash(hash_t *p)                                                         
{                                                                                
    hash_t **pp;                                                                 
	double t1, t2;
                                                                                 
    // sort asc w/timestamp
    for(pp=&hash[0];*pp;pp=&((*pp)->next)) {                                   
		//printf("%d %d %d [cmp]\n",p->k, (*pp)->t.time, (*pp)->t.millitm);
		
		t1 = (double)((*pp)->t.time) * 1000 + (*pp)->t.millitm;
		t2 = (double)(p->t.time) * 1000 + p->t.millitm;
        if(t1 >= t2) break;
	}
    if((p->next = *pp))                                                          
        p->next->prev = &p->next;                                                
    p->prev = pp;                                                                
    *pp = p;                                                                     
}                                                                                

void _unhash(hash_t *p)
{
	//printf("%d %d %d [del]\n",p->k, p->t.time, p->t.millitm);
    if(p->prev) {
        if((*(p->prev) = p->next))
            p->next->prev = p->prev;
        p->next = NULL;
        p->prev = NULL;
    }
}

void _add(void (*fn)(int, void*), int k, void* d, unsigned int msec)
{
	struct timeb t;
	hash_t *p;

	ftime(&t);
	if(!(p=calloc(1,sizeof(hash_t)))) return;

	p->fn 	= fn;
	p->k	= k;
	p->t	= t;
	p->t.time   += msec/1000;
	p->t.millitm+= msec%1000;
	p->d = d;

	//printf("%d %d %d [add]\n",p->k, p->t.time, p->t.millitm);
	_hash(p);
}

static void*
timer_check(void* args) 
{
	struct timeb now;
	struct timeval tv;
	hash_t *p, *tp;
	double t1, t2;

	while (1) {
		tv.tv_sec 	= 0;
		tv.tv_usec 	= 100;

		select(0, 0, 0, 0, &tv);
		ftime(&now);

    	for(p=hash[0];p;p=p->next) {                                   
			tp = *(p->prev);

			t1 = (double)(p->t.time) * 1000 + p->t.millitm;
			t2 = (double)(now.time) * 1000 + now.millitm;
        	if(t1 > t2) break;
			if(p->fn) p->fn(p->k, p->d);
			_unhash(p);
			free(p); 
			p=tp;
		}
	}
}

void
set_cb_timeout(void (*func)(int, void*), int key, void* data, unsigned int msec)
{
    if (!callbacked) {
        pthread_create(&t_timer, NULL, timer_check, NULL);
        callbacked = 1;
    }
    _add(func, key, data, msec);
}

#if 0
// Example CODE

int cnt=0;
void cbfn (int k, void *d)
{
	printf("[%d] CALLBACK FUNCTION %d CALLED \n", ++cnt, k);
}

#define LIMIT 100
#define LOOP 1

int main()
{
	int i,t;
	srand(getpid()|time(0));

	for(i=1;i<=LIMIT;i++) {
		t=rand()%10000+5000; // MIN 5 sec later
		printf("[%d] SET CALLBACK REGISTER (%d msec)\n", i, t); set_cb_timeout(cbfn, i, NULL, t);
	}

	while(LOOP) {
		sleep(1);
	}
}
#endif

