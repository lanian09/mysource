#ifndef _VOD_INIT_H_
#define _VOD_INIT_H_

/**
 *	Include headers
 */
// LIB
#include "mems.h"
#include "typedef.h"

/**
 *	Define cons.
 */
#define VOD_MENU_TITLE_HASH_SIZE        50000

/**
 *	Declare func.
 */
extern S32 dInitVOD( stMEMSINFO **pMEMSINFO, stHASHGINFO **pLVODHASH, stHASHGINFO **pMENUTITLE, 
			  stTIMERNINFO **pTIMERNINFO, stHASHOINFO **pstVODSESSHASH, stHASHOINFO **pstRTCPSESSHASH );
extern void ReadData_VOD(stHASHGINFO *pLVODHASH);
extern void SetUpSignal(void);
extern void UserControlledSignal(S32 isign);
extern void FinishProgram(void);
extern void IgnoreSignal(S32 isign);

#endif /* _VOD_INIT_H_ */
