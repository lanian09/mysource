#ifndef __RPPI_GLOBAL_H__
#define __RPPI_GLOBAL_H__

#include "timerN.h"
#include "hasho.h"
#include "nifo.h"

int             giFinishSignal;     
int             giStopFlag;         


stMEMSINFO   *pMEMSINFO;
stHASHOINFO  *pHASHOINFO; 
stHASHOINFO  *pPCFINFO;
stHASHOINFO  *pDEFECTINFO;
stHASHOINFO  *pMODELINFO;
stHASHOINFO  *pMODELINFO1;
stHASHOINFO  *pMODELINFO2;
stHASHOINFO	*pPDSNIF_HASHO;
stTIMERNINFO *pTIMERNINFO;

#endif
