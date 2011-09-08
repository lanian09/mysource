#ifndef _ADNS_INIT_H_
#define _ADNS_INIT_H_


/**
 * Declare functions
 */
extern int Init_DNSIPCS();
extern void UserControlledSignal(int sign);
extern void IgnoreSignal(int sign);
extern void SetUpSignal();
extern int dInitDNSProc();
extern void FinishProgram();
extern void vDNSSESSTimerReConstruct(stHASHOINFO *pHASH, stTIMERNINFO *pTIMER);
extern void invoke_del_DNS( void *p);

#endif

