#ifndef _EMS_H_
#define _EMS_H_

#include "common_stg.h"

extern int emsreqbody(char *sp, int slen, char *szTransactionID, unsigned short *usClientType, unsigned short *usClientPlatform, char *szClientVersion, char *szCommand, char *szSmtpServer, char *szPop3Server, unsigned short *usSmtpssl, unsigned short *usPop3ssl, unsigned int *uiPeriodTime, unsigned short *usParam, unsigned short *usImageRecv);
extern int emsrespbody(char *sp, int slen, char *szTransactionID, char *szResult);

#endif	/* _EMS_H_ */
