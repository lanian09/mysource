#ifndef _A2G_INIT_H_
#define _A2G_INIT_H_

extern S32 dInit2G(stMEMSINFO **pMEMSINFO);
extern void SetUpSignal(void);
extern void UserControlledSignal(S32 isign);
extern void FinishProgram(void);
extern void IgnoreSignal(S32 isign);

#endif /* _A2G_INIT_H_ */
