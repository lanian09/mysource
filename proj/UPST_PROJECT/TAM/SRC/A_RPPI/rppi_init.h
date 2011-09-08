#ifndef __RPPI_INIT_H__
#define __RPPI_INIT_H__

#include "common_stg.h"
#include "hasho.h"	/* stHASHOINFO */ 

extern S32 dInitRPPI();;
extern void UserControlledSignal(int sign);
extern void IgnoreSignal(int sign); 
extern void SetUpSignal();
extern void FinishProgram();
extern void vRPPITimerReConstruct(stHASHOINFO *pRPPIHASH, stTIMERNINFO *pTIMER);
extern void ReadTimerFile();
extern int	dGetMapping(void);
extern void invoke_del_call(void *p);

#endif /* __RPPI_INIT_H__ */
