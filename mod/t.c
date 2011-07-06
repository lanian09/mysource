#include <stdio.h>
#include <string.h>

#define dAppLog fprintf
#define LOG_INFO  stdout
#define LOG_DEBUG stdout
#define LOG_WARN  stdout
#define LOG_CRI   stdout

#define LP "%s.%d:%s "
#define LT __FILE__,__LINE__,__FUNCTION__
#define EP ", error=%d:%s"
#define ET errno,strerror(errno)
#define LOGLN printf("\n")

typedef unsigned int  		UINT;
typedef int           		S32;
typedef unsigned short 		U16;
typedef short 				S16;
typedef unsigned char 		U8;
typedef char          		S8;
typedef unsigned long long	U64;
typedef long long 			S64;

#define MAX_RANGE_CNT 256
#define SEQ_PROC_NO 100
UINT uiSeqProcKey;
U8   procname[16];

int test(char *ab)
{
	int arg_len, pn;
	arg_len = strlen(ab);
	dAppLog(LOG_INFO,"ARG?[%s:%d]", ab, arg_len); LOGLN;
	if( !isdigit(ab[arg_len-1]) ){
		pn = 0;
	}else{
		pn = atoi(&ab[arg_len-1]);
	}

	dAppLog(LOG_INFO,"ARGS>>[%d]", pn); LOGLN;
	dAppLog(LOG_INFO,"STRING=%s", ab); LOGLN;
	dAppLog(LOG_INFO,"LEN=%d", strlen(ab)); LOGLN;
	return 0;
}

int main(int ac, char **av)
{
	int i, PROCNO;

	if( ac != 2 ){
		dAppLog(LOG_CRI,LP"need to parameter block index",LT);
		LOGLN;
		return 0;
	}

	test(av[0]);
/*
	for( i = 0; i < strlen(av[1]); i++ ){
		if( !isdigit(av[1][0]) ){
			dAppLog(LOG_WARN,LP"INVALID ARG=%s, Only Number Support",LT, av[1]);
			LOGLN;
			return 0;
		}
	}
*/

	PROCNO = atoi(av[1]);
	uiSeqProcKey = SEQ_PROC_NO + PROCNO;

	dAppLog(LOG_INFO,LP" @@@ ARG=%s",    LT, av[1]);LOGLN;
	dAppLog(LOG_INFO,LP" @@@ PROCNO=%d", LT, PROCNO);LOGLN;
	dAppLog(LOG_INFO,LP" @@@ SEQ_NO=%d", LT, uiSeqProcKey);LOGLN;

	dAppLog(LOG_INFO,LP" @@@ MOD1=%d", LT, PROCNO%MAX_RANGE_CNT);LOGLN;
	dAppLog(LOG_INFO,LP" @@@ MOD2=%d", LT, (PROCNO%MAX_RANGE_CNT)%3);LOGLN;
	
	return 0;
}
