#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

/* �˸��� ����ü ���� */ 
typedef struct {
	double volatile *p_s;      /* ��Į�� ���� ���� ���� */
	pthread_mutex_t *p_s_lock; /* ���� s�� lock */
	int n;                     /* �������� �� */
	int nproc;                 /* �̿��� �� �ִ� ���μ����� �� */
	double *x;                 /* ù��° ������ ������ */
	double *y;                 /* �ι�° ������ ������ */
	int l;                     /* ������ ���� */
} DATA;

void *SMP_scalprod(void *arg)
{
	register double localsum;
	long i;
	DATA D = *(DATA *)arg;

	localsum = 0.0;

	/* �� ������� i = D.n���� ���� ��Į�� ���� �����Ѵ�. 
	   D.n = 1, 2, ...
	   D.nproc ���� ���´�. ��Ȯ�� D.nproc���� �����尡 �ֱ� 
	   ������ i�� ���� ���� D.nproc�̴�. */

	for(i = D.n; i < D.l; i += D.nproc)
		localsum += D.x[i]*D.y[i];

	/* s�� ���� lock�� �Ǵ� ... */
	pthread_mutex_lock(D.p_s_lock);

	/* ... s�� ���� �ٲ۴�. ... */
	*(D.p_s) += localsum;

	/* ... �׸��� lock�� �����Ѵ�. */
	pthread_mutex_unlock(D.p_s_lock);

	return NULL;
}

#define L 9     /* ������ ���� */

int main(int argc, char **argv)
{
	pthread_t *thread;
	void *retval;
	int cpu, i;
	DATA *A;
	volatile double s = 0; /* ���� ���� */ 
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

	/* lock ������ �ʱ�ȭ�Ѵ�. */
	pthread_mutex_init(&s_lock, NULL);

	for (i = 0; i < cpu; i++) {
		/* ����ü�� �ʱ�ȭ�Ѵ�. */
		A[i].n = i; /* �������� �� */
		A[i].x = x;
		A[i].y = y;
		A[i].l = L;
		A[i].nproc = cpu; /* CPU�� �� */
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
