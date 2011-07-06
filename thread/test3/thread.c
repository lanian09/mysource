#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

/* 알맞은 구조체 선언 */ 
typedef struct {
	double volatile *p_s;      /* 스칼라 곱의 공유 변수 */
	pthread_mutex_t *p_s_lock; /* 변수 s의 lock */
	int n;                     /* 쓰레드의 수 */
	int nproc;                 /* 이용할 수 있는 프로세서의 수 */
	double *x;                 /* 첫번째 벡터의 데이터 */
	double *y;                 /* 두번째 벡터의 데이터 */
	int l;                     /* 벡터의 길이 */
} DATA;

void *SMP_scalprod(void *arg)
{
	register double localsum;
	long i;
	DATA D = *(DATA *)arg;

	localsum = 0.0;

	/* 각 쓰레드는 i = D.n에서 부터 스칼라 곱을 시작한다. 
	   D.n = 1, 2, ...
	   D.nproc 값을 갖는다. 정확히 D.nproc개의 쓰레드가 있기 
	   때문에 i의 증가 같은 D.nproc이다. */

	for(i = D.n; i < D.l; i += D.nproc)
		localsum += D.x[i]*D.y[i];

	/* s에 대한 lock을 건다 ... */
	pthread_mutex_lock(D.p_s_lock);

	/* ... s의 값을 바꾼다. ... */
	*(D.p_s) += localsum;

	/* ... 그리고 lock를 제거한다. */
	pthread_mutex_unlock(D.p_s_lock);

	return NULL;
}

#define L 9     /* 벡터의 차원 */

int main(int argc, char **argv)
{
	pthread_t *thread;
	void *retval;
	int cpu, i;
	DATA *A;
	volatile double s = 0; /* 공유 변수 */ 
	pthread_mutex_t s_lock; 
	double x[L], y[L];

	if (argc != 2) {
		printf("usage: %s <number of CPU>\n", argv[0]);
		return -1;
	}

	cpu = atoi(argv[1]);
	thread = (pthread_t *) calloc(cpu, sizeof(pthread_t));
	A = (DATA *) calloc(cpu, sizeof(DATA));


	for (i = 0; i < L; i++)
		x[i] = y[i] = i;

	/* lock 변수를 초기화한다. */
	pthread_mutex_init(&s_lock, NULL);

	for (i = 0; i < cpu; i++) {
		/* 구조체를 초기화한다. */
		A[i].n = i; /* 쓰레드의 수 */
		A[i].x = x;
		A[i].y = y;
		A[i].l = L;
		A[i].nproc = cpu; /* CPU의 수 */
		A[i].p_s = &s;
		A[i].p_s_lock = &s_lock;

		if (pthread_create(&thread[i], NULL, SMP_scalprod, 
					&A[i])) {
			fprintf(stderr, "%s: cannot make thread\n", 
					argv[0]);
			return -2;
		}
	}

	for (i = 0; i < cpu; i++) {
		if (pthread_join(thread[i], &retval)) {
			fprintf(stderr, "%s: cannot join thread\n", 
					argv[0]);
			return -3;
		}
	}

	printf("s = %f\n", s);
	return 0;
}
