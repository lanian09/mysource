/* gcc -I../../INC -I. o_svcmon_size.c
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "o_svcmon_api.h"
#include "o_svcmon_size.h"

//#include <utillib.h>

S32						giFinishSignal;					/**< Finish Signal */
S32						giStopFlag;						/**< main loop Flag 0: Stop, 1: Loop */

extern st_MonTotal		*gMonTotal;
extern st_MonTotal_1Min	*gMonTotal1Min;
extern st_WatchFilter	*gWatchFilter;

S32						dMyQID;
S32						dMONDQID;
S32						dALMDQID;
S32						dMSVCMONQID;

stHASHOINFO				*pAMonHash;
stHASHOINFO				*pSMonHash;

stHASHOINFO				*pMonHash;
stHASHOINFO				*pNextMonHash;

stHASHOINFO				*pDefHash;
stHASHOINFO				*pThresHash;
stHASHOINFO				*pNasIPHash;

st_MonList				stBaseMonList;
st_MonList_1Min			stBaseMonList1Min;

/** main function.
 *
 *  man Function
 *
 *  @param	argc	:	파라미터 개수
 *  @param	*argv[]	:	파라미터
 *
 *  @return			S32
 *  @see			o_svcmon_main.c o_svcmon_init.c o_svcmon_func.c
 *
 *  @exception		.
 *  @note			.
 **/
S32 main(S32 argc, S8 *argv[])
{
	st_WatchMsg		*pWATCH;
	st_MonList		*pMonList = NULL;
	st_MonList_1Min	*pMonList1Min = NULL;
	st_SvcMonMsg	*pSvcMonMsg;

	printf("st_WatchMsg [%d]\n", sizeof(st_WatchMsg));
	printf("st_MonList [%ld]\n", sizeof(st_MonList));
	printf("st_MonList_1Min [%d]\n", sizeof(st_MonList_1Min));
	printf("st_SvcMonMsg [%d]\n", sizeof(st_SvcMonMsg));

	return 0;
}


