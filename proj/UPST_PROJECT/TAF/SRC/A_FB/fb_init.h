#ifndef _FB_INIT_H_ 
#define _FB_INIT_H_ 

extern S32 dInitFB(stMEMSINFO **pMEMSINFO);
extern void SetUpSignal(void);
extern void UserControlledSignal(S32 isign);
extern void FinishProgram(void);
extern void IgnoreSignal(S32 isign);

#endif /* _FB_INIT_H_ */
