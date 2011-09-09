#ifndef __S_MNG_DFT_H__
#define __S_MNG_DEF_H__

#include "s_mng_mon.h"	/* FIVE_MIN_PER_HOUR_COUNT + */
#include "mmcdef.h"		/* st_TrendInfo */


typedef struct _st_Defect_24Hours
{           
    unsigned long long      tStart;
    char                    cReserved[4];

    unsigned int            uDefectInfo;
} st_Defect_24Hours, *pst_Defect_24Hours;

typedef struct _st_Defect_24Hours_List
{   
    int                     dCount;
    char                    cFlag;
    char                    cReserved[3];
        
    st_Defect_24Hours       st_Defect_24Hours[FIVE_MIN_PER_HOUR_COUNT];
} st_Defect_24Hours_List, *pst_Defect_24Hours_List;

typedef struct _st_Defect_24Hours_List_1Min
{
    int                     dCount;
    char                    cFlag;
    char                    cReserved[3];

    st_Defect_24Hours       st_Defect_24Hours[ONE_MIN_PER_HOUR_COUNT];
} st_Defect_24Hours_List_1Min, *pst_Defect_24Hours_List_1Min;

extern int dGetDefTrendInfo(int dIndex, st_TrendInfo *pTrend, st_Defect_24Hours_List *pData);
extern int dGetDefTrendInfo1Min(int dIndex, st_TrendInfo *pTrend, st_Defect_24Hours_List_1Min *pData);

#endif /* __S_MNG_DEF_H__ */
