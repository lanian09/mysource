#ifndef __COND_MSG_H__
#define __COND_MSG_H__

#include "msgdef.h"	 /* st_MsgQ */
#include "almstat.h" /* st_almsts */

#define TYPE_M_PRIMARY    0
#define TYPE_F_PRIMARY    4
#define TYPE_F_SECONDARY  8

typedef struct _st_CondCount
{   
    unsigned short usTotPage;
    unsigned short usCurPage; 
    unsigned short usSerial;
    unsigned short usReserved; 
} st_CondCount, *pst_CondCount;

extern int MakeHWAlmInfo(st_almsts alm, char *AlmHeader, char *AlmInfo);
extern int MakeLOADAlmInfo(st_almsts alm, char *AlmHeader, char *AlmInfo, long long llLoadVal);
extern int MakeSWAlmInfo(st_almsts alm, char *AlmHeader, char *AlmInfo, int ucTAMID, int ucTAFID);
extern int MakeCHNLAlmInfo(st_almsts alm, char *AlmHeader, char *AlmInfo);
extern int MakeNTPAlmInfo(st_almsts alm, char *AlmHeader, char *AlmInfo);
extern int MakeServiceAlmInfo(st_almsts alm, char *AlmHeader, char *AlmInfo);

extern int Set_Cond_Msg(st_MsgQ *pstMsg, char *szTotMsg, st_CondCount *pstCnt, int ucTAMID, int ucTAFID);

#endif /* __COND_MSG_H__ */
