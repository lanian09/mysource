/**
	@file		fstat_traffic.h
	@author		
	@version	
	@date		2011-07-27
	@brief		fstat_traffic.c 헤더파일
*/

#ifndef __FSTAT_TRAFFIC_H__
#define __FSTAT_TRAFFIC_H__

/**
	Include headers
*/
#include <time.h>

// .
#include "fstat_init.h"

/**
	Declare functions
*/
extern int dCheckTrafficList(time_t tLocalTime, st_NtafStatList *stTRAFFIC);


#endif /* __FSTAT_TRAFFIC_H__ */
