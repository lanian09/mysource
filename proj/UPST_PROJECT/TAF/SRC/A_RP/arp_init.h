#ifndef _ARP_INIT_H_
#define _ARP_INIT_H_

/**
 *	Include headers
 */


/**
 * Define constants
 */
#define S_SEMA_CALL		11001


/**
 * Declare functions
 */
int dInit_Proc();
void UserControlledSignal(int sign);
void FinishProgram();
void IgnoreSignal(int sign);
void SetUpSignal();
int Init_A11_PSESS();
int Init_TraceShm();
int Init_GREEntry_Shm(void);

extern int	Init_TraceShm();
extern int Init_A11_PSESS();
extern int Init_GREEntry_Shm(void);


#endif	/* _ARP_INIT_H_ */
