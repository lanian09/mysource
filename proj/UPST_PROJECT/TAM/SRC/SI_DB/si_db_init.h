#ifndef _SI_DB_INIT_H_
#define _SI_DB_INIT_H_

/**
 *	Include headers
 */
// LIB
#include "memg.h"
#include "mems.h"
#include "cifo.h"
#include "nsocklib.h"				/* pst_SubSysInfoList */

/**
 *	Declare functions
 */
int dInitProc(stMEMSINFO **pMEMSINFO, stCIFO **pCIFO);
void SetUpSignal();
void UserControlledSignal(int sign);
void FinishProgram();
void IgnoreSignal(int sign);
int dReadSubSysInfoFile(pst_SubSysInfoList pstList);
int dReadRemainFile();
int dWriteRemainFile();
int dInitFidb(void);

#endif	/* _SI_DB_INIT_H_ */
