#ifndef _SI_NMS_INIT_H_
#define _SI_NMS_INIT_H_

/**
 *	Include headers
 */
// LIB
#include "mems.h"
#include "cifo.h"

// .
#include "si_nms_comm.h"

/**
 *	Define constants
 */
#define MAX_LOCSYS_SIZE				32

/**
 *	Declare functions
 */
extern int dInitProc(stMEMSINFO **pMEMSINFO, stCIFO **pCIFO);
extern int dReadNMSPortInfoFile(st_NMSPortInfo *pstData);
extern int dReadNMSIPInfoFile(st_NMSIPList *pstData);
extern int dGetSYSCFG(void);
extern int dReadNMSOidInfoFile(void);
extern void SetUpSignal(void);
extern void UserControlledSignal(int sign);
extern void FinishProgram(void);
extern void IgnoreSignal(int sign);
extern int dInitFidb(void);

#endif	/* _SI_NMS_INIT_H_ */
