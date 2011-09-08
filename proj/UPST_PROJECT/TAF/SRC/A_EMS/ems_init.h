#ifndef __EMS_INIT_H__
#define __EMS_INIT_H__

/**
 *	Declare func.
 */
extern S32 dInitEMS(stMEMSINFO **pMEMSINFO);
extern void SetUpSignal(void);
extern void UserControlledSignal(S32 isign);
extern void FinishProgram(void);
extern void IgnoreSignal(S32 isign);


#endif /* __EMS_INIT_H__ */
