#ifndef __CHSMD_NTPD_H__
#define __CHSMD_NTPD_H__

#include "almstat.h"	/* st_NTP_STS */

extern int CompareNTPSTS(st_NTP_STS stCurr, pst_NTP_STS pstOld);
extern int SetNTPSTS( int dType, unsigned char ucCur, unsigned char ucOld  );
extern int CheckNTPStS(void);

#endif	/* __CHSMD_NTPD_H__ */

