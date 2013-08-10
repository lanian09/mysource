/**
	@file		fstat_load.h
	@author		
	@version	
	@date		2011-07-26
	@brief		fstat_load.c 헤더파일
*/

#ifndef __FSTAT_LOAD_H__
#define __FSTAT_LOAD_H__

/**
	Include headers
*/
#include <time.h>

// OAM
#include "filedb.h"

/**
	Declare functions
*/
extern int dCheckNTAMLoadList(time_t tLocalTime, st_NTAM * stNTAM);
extern int dCheckNTAFLoadList(time_t tLocalTime, st_NTAF *stNTAF, short usNTAFID);
extern void InitSHMLOAD(short TimeFlag, unsigned int uiStatTime);
extern void Stat_LOADFunc(void *p);


#endif	/* __FSTAT_LOAD_H__ */
