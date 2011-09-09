#ifndef __M_SVCMON_INIT_H__
#define __M_SVCMON_INIT_H__

extern void SetUpSignal(void);
extern void UserControlledSignal(S32 isign);
extern void IgnoreSignal(S32 isign);
extern void FinishProgram(void);
extern S32 dInitMSVCMON(void);

#endif /* __M_SVCMON_INIT_H__ */
