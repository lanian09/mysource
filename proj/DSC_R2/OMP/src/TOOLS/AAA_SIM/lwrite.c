
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>


extern int errno;

FILE  *user_def_ptr ;

int log_debug( char *fmt, ... )
{

    char        mesg_path[80];
    time_t  nowtime;
    struct tm chktm;

    FILE *fptr;

    va_list  ap;


    if( user_def_ptr == NULL )
    {
        sprintf( mesg_path, "aaa_sim.stat");

        if( (fptr = fopen(mesg_path, "a+")) == NULL )
        {
            fprintf(stderr, "LOG FILE OPEN ERROR %s\n", strerror(errno)); 
			return -1;
        }


        user_def_ptr = fptr;

    }


    time(&nowtime);
    localtime_r( &nowtime, &chktm );

    fprintf(user_def_ptr, "[%02d/%02d %02d:%02d:%02d] ",
        chktm.tm_mon+1, chktm.tm_mday, chktm.tm_hour,
        chktm.tm_min, chktm.tm_sec );

    va_start( ap, fmt );

    vfprintf( user_def_ptr, fmt, ap );

    fprintf( user_def_ptr, "\n");
    fflush(user_def_ptr);

    va_end(ap);

    return 1;
}
