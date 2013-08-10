/**
	@file		fstat_fault.h	
	@author		
	@version	
	@date		2011-07-26
	@brief		fstat_fault.c 헤더파일
*/

#ifndef __FSTAT_FAULT_H__
#define __FSTAT_FAULT_H__

/**
	Include headers
*/
#include <time.h>

#include "almstat.h"


/**
	Declare functions
*/
extern int dCheckFaultList(time_t tLocalTime, st_almsts * stAlmStatus, short usNTAFID);
extern void Stat_FAULTFunc(void* p);


#endif	/* __FSTAT_FAULT_H__ */
