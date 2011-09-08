#ifndef _IHTTP_INIT_H_
#define _ITHHP_INIT_H_

/**
 * Declare functions
 */
extern S32 dInitHttp(stMEMSINFO **pMEMSINFO, stHASHOINFO **pTCPHASH, stHASHOINFO **pHTTPHASH);
extern void SetUpSignal(void);
extern void UserControlledSignal(S32 isign);
extern void FinishProgram(void);
extern void IgnoreSignal(S32 isign);

#endif	/* _IHTTP_INIT_H_ */
