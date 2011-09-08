#include "udrdb.h"

extern	int					errno;
static 	pthread_t			thr_id;
static	pthread_attr_t		thr_attr;

/**
 *	
 **/
print_dot()
{

	int dot_cnt;

	dot_cnt = 0;

	while(1)
	{

        fprintf(stdout, ".");
		fflush(stdout);
		dot_cnt++;

		if (dot_cnt%60 == 0)
			fprintf(stdout, "%d seconds\n", (30*dot_cnt)/60);

		usleep(500000);
        
    }
}


/**
 *
 **/
int 
start_wait()
{
	
	/** thread attribute initialize **/
	if (pthread_attr_init(&thr_attr) < 0)
		fprintf(stdout, "THREAD ATTRIBUTE INIT ERROR [ERRNO=%s]", strerror(errno));

	/** start wait print thread **/
	if (pthread_create(&thr_id, &thr_attr, (void *(*)(void *))print_dot, NULL) < 0)
	{
		fprintf(stdout, "WAIT PRINT JOB THREAD CREATE ERROR [ERRNO=%s]", strerror(errno));
		return (-1);
	}
	return (0);
}


/**
 *		
 **/
int
stop_wait( )
{
	
	if (pthread_cancel(thr_id) != 0)
	{
		fprintf(stderr, "PTHREAD CANCEL ERROR [ERRNO=%s]", strerror(errno));
		return (-1);
	}
	return (0);
}


/**
 *
 **/
int
conv_time_to_string(time_t conv_time, char *time_str)
{

	struct tm	*time_info;
	char		temp_time[32];

	memset(temp_time, 0x00, sizeof(temp_time));

	time_info = localtime(&conv_time);

	sprintf(temp_time, "%d-%02d-%02d %02d:%02d:%02d",
						time_info->tm_year+1900,
						time_info->tm_mon+1,
						time_info->tm_mday,
						time_info->tm_hour,
						time_info->tm_min,
						time_info->tm_sec);

	memcpy(time_str, temp_time, strlen(temp_time));

	return (0);

}
