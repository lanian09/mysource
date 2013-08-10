#ifndef __S_MNG_MON_H__
#define __S_MNG_MON_H__

#include "watch_mon.h"		/* st_MonInfo */
#include "mmcdef.h"			/* st_TrendInfo */

typedef struct _st_PtStatus_24Hours
{
    unsigned long long      tStartPt;
    st_MonInfo              stMonInfo;
} st_PtStatus_24Hours, *pst_PtStatus_24Hours;

#define FIVE_MIN_PER_HOUR_COUNT     (6*2)
#define ONE_MIN_PER_HOUR_COUNT      30
#define MSG_CONTINUE                0
#define MSG_END                     1

typedef struct _st_PtStatus_24Hours_List
{
    int                     dCount;
    char                    cFlag;                              /*          MSG_CONTINUE, MSG_END                       */
    char                    cReserved[3];

    st_PtStatus_24Hours     stPtStatus_24Hours[FIVE_MIN_PER_HOUR_COUNT];
} st_PtStatus_24Hours_List, *pst_PtStatus_24Hours_List;

typedef struct _st_PtStatus_24Hours_List_1Min
{
    int                     dCount;
    char                    cFlag;                              /*          MSG_CONTINUE, MSG_END                       */
    char                    cReserved[3];

    st_PtStatus_24Hours     stPtStatus_24Hours[ONE_MIN_PER_HOUR_COUNT];
} st_PtStatus_24Hours_List_1Min, *pst_PtStatus_24Hours_List_1Min;

extern int dGetTrendInfo(int dIndex, st_TrendInfo *pTrend, st_PtStatus_24Hours_List *pData);
extern int dGetTrendInfo1Min(int dIndex, st_TrendInfo *pTrend, st_PtStatus_24Hours_List_1Min *pData);

#endif /* __S_MNG_MON_H__ */
