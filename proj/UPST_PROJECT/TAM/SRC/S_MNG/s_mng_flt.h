#ifndef __S_MNG_FLT_H__
#define __S_MNG_FLT_H__

#include "filter.h"		/* st_AlmLevel_List */
#include "filedb.h"		/* MAX_NTAF_NUM */
#include "loglib.h"		/* st_LogLevel_List */
#include "filedb.h"		/* st_keepalive */

typedef struct _st_Sub_GI_NTAF_List {
    unsigned short      usSysType[MAX_NTAF_NUM];
    unsigned short      usSysNo[MAX_NTAF_NUM];

    unsigned int        uiIP[MAX_NTAF_NUM];
} st_Sub_GI_NTAF_List, *pst_Sub_GI_NTAF_List;

typedef struct _st_NtafIP_db
{
    SHORT   sNo[MAX_NTAF_NUM];
    UINT    uiIP[MAX_NTAF_NUM];
} st_NtafIP_db, *pst_NtafIP_db;
    
/**
typedef struct _st_NtafIP_List
{   
    int             dCount;
    int             reserved;
    st_NtafIP_db    stNtafIPdb;
} st_NtafIP_List, *pst_NtafIP_List;
***/


extern int dRead_FLT_AlmLvl_NTAF(st_AlmLevel_List *stAlm, int dNo);
extern int dInitLogLvl(void);
extern int dInit_Ntaf_Alarm(void);
extern int dRead_FLT_Tmf(st_NtafIP_db *stNtafIP);
extern int dRead_FLT_GINTAF_Tmf(st_Sub_GI_NTAF_List *stGINtafIP);
extern int dInitSysConfig(void);
extern int dLogWrite(st_LogLevel_List	*pstLogLevelList);
extern int dAlmWrite_NTAM(st_keepalive *keepalive);
extern int dAlmWrite_NTAF(void);

#endif /* __S_MNG_FLT_H__ */
