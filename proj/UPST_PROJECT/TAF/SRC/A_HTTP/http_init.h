#ifndef _HTTP_INIT_H_
#define _HTTP_INIT_H_

extern S32 dInitHttp(stMEMSINFO **pMEMSINFO, stHASHOINFO **pTCPHASH, stHASHOINFO **pHTTPHASH);
extern void SetUpSignal(void);
extern void UserControlledSignal(S32 isign);
extern void FinishProgram(void);
extern void IgnoreSignal(S32 isign);

#endif /* _HTTP_INIT_H_ */
