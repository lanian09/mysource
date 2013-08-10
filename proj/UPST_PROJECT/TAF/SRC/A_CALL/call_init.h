#ifndef __CALL_INIT_H__
#define __CALL_INIT_H__

extern S32 dInitCALL(stMEMSINFO **pMEMSINFO, stHASHOINFO **pHASHOINFO, stTIMERNINFO **pTIMERNINFO);
extern void SetUpSignal(void);
extern void UserControlledSignal(S32 isign);
extern void FinishProgram(void);
extern void IgnoreSignal(S32 isign);
extern void vCALLTimerReConstruct(stMEMSINFO *pMEMSINFO, stHASHOINFO *pHASH, stTIMERNINFO *pTIMER);

#endif	/* __CALL_INIT_H__ */
