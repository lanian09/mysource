#ifndef __CHSMD_NTP_H__
#define __CHSMD_NTP_H__

#include "almstat.h"	/* st_NTP_STS */

extern int SetNTPSTS(int, unsigned char, unsigned char);
extern int CompareNTPSTS(st_NTP_STS, pst_NTP_STS);
extern int CheckNTPStS(void);

#endif /* __CHSMD_NTP_H__ */
