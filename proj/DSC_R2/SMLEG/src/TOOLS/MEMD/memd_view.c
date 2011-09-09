
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <errno.h>
#include <unistd.h>
#include <getopt.h>

#include "ipaf_shm.h"
#include "shmutil.h"
#include "ipaf_sem.h"
#include "ipaf_define.h"
#include "ipaf_names.h"

#include "mems.h"
#include "nifo.h"

/* Function Time Check */
#include "func_time_check.h"

#ifdef PRINT_FUNC
#undef PRINT_FUNC
#define PRINT_FUNC 	fprintf
#endif 


#ifdef PRINT_LEVEL
#undef PRINT_LEVEL
#define PRINT_LEVEL 	stderr
#endif 

stMEMSINFO 		*pstMEMSINFO;

/* Function Time Check */
st_FuncTimeCheckList    stFuncTimeCheckList;
st_FuncTimeCheckList    *pFUNC = &stFuncTimeCheckList;

int 			semid_mif = -1;
int 			key = 0x282C;
int				g_StatIndex = 0;
int 			gaptime = 60;
int 			sleep_cnt = 1;

time_t 			now;

void help()
{
	printf("Usage: ./MEM [OPTION] []\n"
			"  -h		사용법\n"
			"  -s 		sleep count\n"
			"  -d		gaptime이 지난 노드 삭제\n"
			"  -v		모니터링\n"
			"Sample: ./MEMD -s 1 -d 600\n" );
	exit(0);
}


/** main function.
 *
 *  @note       pstMEMSINFO의 현재 정보를 프린트 
 **/
int main(int argc, char **argv)
{
	int 	opt;
	int 	opt_ok = 0;
	int	 	delete_flag = 0;
	int		view_flag = 0;
	int 	nodecnt=0;
	
	while( (opt = getopt(argc, argv, "hs:d:v")) != -1 ) {
		switch(opt) {
			case 'h':
				help();
				break;
			case 's':
				sleep_cnt = atoi(optarg);
				FPRINTF(LOG_LEVEL, "monitoring time=%d sec\n", sleep_cnt);
				opt_ok = 1;
				break;
			case 'd':
				delete_flag = 1;
				gaptime = atoi(optarg);
				FPRINTF(LOG_LEVEL, "delete node %d sec after adding it\n", gaptime);
				opt_ok = 1;
				break;
			case 'v':
				view_flag = 1;
				break;
		}
	}
	if (!opt_ok) {
		help();
		exit(0);
	}

	semid_mif = Init_sem( S_SEMA_MIF );
	pstMEMSINFO = nifo_init(key, semid_mif, "MEMD", SEQ_PROC_MEM);

	if(delete_flag) {
		mems_garbage_collector("MIF NODE DELETE" , pstMEMSINFO, gaptime, NULL);
	}

	if(view_flag) {
		while (1) {
			now = time(NULL);

			if( ((now/60)%60) != g_StatIndex ) {
				g_StatIndex = (now/60)%60;
				mems_view("MEMS VIEW", pstMEMSINFO, gaptime, NULL);

				FPRINTF(LOG_LEVEL,"INIT=%s SKEY=0x%x SID=%d TOT=%u ALLOCED=%u START=%ld FREE=%ld END=%ld CREATE=%llu DELCNT=%llu\n",
						procName, 
						pstMEMSINFO->uiShmKey, pstMEMSINFO->iSemID,
						pstMEMSINFO->uiMemNodeTotCnt, mems_alloced_cnt(pstMEMSINFO), 
						pstMEMSINFO->offsetNodeStart, pstMEMSINFO->offsetFreeList, pstMEMSINFO->offsetNodeEnd,
						pstMEMSINFO->createCnt, pstMEMSINFO->delCnt);
			}
			usleep(0);
		}
	} 
	else {
		while(1) {

START_FUNC_TIME_CHECK(pFUNC, 0);
			nodecnt = mems_alloced_cnt(pstMEMSINFO);
END_FUNC_TIME_CHECK(pFUNC, 0);

			FPRINTF(LOG_LEVEL,"INIT=%s SKEY=0x%x SID=%d TOT=%u ALLOCED=%u START=%ld FREE=%ld END=%ld CREATE=%llu DELCNT=%llu\n",
					procName, 
					pstMEMSINFO->uiShmKey, pstMEMSINFO->iSemID,
					pstMEMSINFO->uiMemNodeTotCnt, nodecnt, 
					pstMEMSINFO->offsetNodeStart, pstMEMSINFO->offsetFreeList, pstMEMSINFO->offsetNodeEnd,
					pstMEMSINFO->createCnt, pstMEMSINFO->delCnt);
			if(!sleep_cnt) exit(0);
			sleep(sleep_cnt);

#ifdef FUNC_TIME_CHECK
			now = time(NULL);

			if( ((now/60)%60) != g_StatIndex ) {
				g_StatIndex = (now/60)%60;
PRINT_FUNC_TIME_CHECK(pFUNC);
			}
#endif
		}
	}


	return 0;
}
