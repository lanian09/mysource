#ifndef __PCIV_INIT_H__
#define __PCIV_INIT_H__

extern S32 dInitProc(stMEMSINFO** ppMEMSINFO, stHASHOINFO** ppHASH);
extern void SetUpSignal(void);
extern void UserControlledSignal(S32 isign);
extern void FinishProgram(void);
extern void IgnoreSignal(S32 isign);

#endif /* __PCIV_INIT_H__ */
