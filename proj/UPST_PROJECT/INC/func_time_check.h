#ifndef	__FUNC_TIME_CHECK_H__
#define __FUNC_TIME_CHECK_H__

#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

#define MAX_FUNC_TIME_CHECK_CNT		100

typedef struct _st_FuncTimeCheck {
	struct timeval		start;
	struct timeval		end;
	long long			duration;
	long long			count;
	long long			max;	
	int					funcIdx;
	char				funcName[32];
} st_FuncTimeCheck;

typedef struct _st_FuncTimeCheckList {
	st_FuncTimeCheck	func[MAX_FUNC_TIME_CHECK_CNT];
} st_FuncTimeCheckList;

#if defined(FUNC_TIME_CHECK)

#define	PRINT_FUNC		dAppLog
#define PRINT_LEVEL		1

#define START_FUNC_TIME_CHECK(p, idx)																							\
	{																															\
		if(idx < MAX_FUNC_TIME_CHECK_CNT) {																						\
			if(p->func[idx].funcName[0] == 0x00)																				\
				sprintf(p->func[idx].funcName, "%s", __FUNCTION__);																\
			gettimeofday(&p->func[idx].start, NULL);																			\
		} else {																												\
			PRINT_FUNC(PRINT_LEVEL, "OVER IDX[%d] MAX[%d]", idx, MAX_FUNC_TIME_CHECK_CNT);										\
		}																														\
	}

#define END_FUNC_TIME_CHECK(p, idx)																								\
	{																															\
		long long tmp;																											\
		if(idx < MAX_FUNC_TIME_CHECK_CNT) {																						\
			gettimeofday(&p->func[idx].end, NULL);																				\
			tmp = (p->func[idx].end.tv_sec * 1000000 + p->func[idx].end.tv_usec) 												\
					- (p->func[idx].start.tv_sec * 1000000 + p->func[idx].start.tv_usec);										\
			if(tmp > p->func[idx].max) p->func[idx].max = tmp;																	\
			p->func[idx].duration += tmp;																						\
			p->func[idx].count++;																								\
			if(p->func[idx].funcName[0] == 0x00)																				\
				sprintf(p->func[idx].funcName, "%s", __FUNCTION__);																\
		} else {																												\
			PRINT_FUNC(PRINT_LEVEL, "OVER IDX[%d] MAX[%d]", idx, MAX_FUNC_TIME_CHECK_CNT);										\
		}																														\
	}

#define PRINT_FUNC_TIME_CHECK(p)																								\
	{																															\
		int			i;																											\
		PRINT_FUNC(PRINT_LEVEL, "FUNC *** UNIT: micro sec ***");																\
		for(i = 0; i < MAX_FUNC_TIME_CHECK_CNT; i++) {																			\
			if(p->func[i].funcName[0] != 0x00) {																				\
				PRINT_FUNC(PRINT_LEVEL, 																						\
				"FUNC NAME[%s] IDX[%d] DURATION[%lld] COUNT[%lld] AVG[%0.2f] MAX[%lld]", 										\
				p->func[i].funcName, i, p->func[i].duration, p->func[i].count, 													\
				(double)p->func[i].duration/(double)p->func[i].count, p->func[i].max);											\
			}																													\
			p->func[i].start.tv_sec = 0;																						\
			p->func[i].start.tv_usec = 0;																						\
			p->func[i].end.tv_sec = 0;																							\
			p->func[i].end.tv_usec = 0;																							\
			p->func[i].duration = 0;																							\
			p->func[i].count = 0;																								\
			p->func[i].max = 0;																									\
			p->func[i].funcIdx = 0;																								\
			p->func[i].funcName[0] = 0x00;																						\
		}																														\
	}
#endif

#if !defined(FUNC_TIME_CHECK)
#define START_FUNC_TIME_CHECK(p, idx)
#define END_FUNC_TIME_CHECK(p, idx)
#define PRINT_FUNC_TIME_CHECK(p)
#endif																
							
#endif

