#ifndef __CHSMD_DISK_H__
#define __CHSMD_DISK_H__

typedef struct
{
    char		szSpecial[50];
    char    	szMountp[20];
    int     	dPercent;
    long long   llUsed;
    long long   llTotal;
}st_Soldf;

typedef struct
{
    int         dCount;
    st_Soldf    stSoldf[20];
}st_SoldfList;

#define MAX_DF_DATA     40

typedef struct _st_df_data
{
    char    szName[50];
    int     dSize;
    int     dUsed;
    int     dAvail;
    double  dUsedPercent;
} st_df_data;

typedef struct _st_df_total
{
    int     dIndex;
    int     dTotSize;
    int     dTotUsed;
    int     dTotAvail;
    st_df_data stData[MAX_DF_DATA];
} st_df_total;

extern int dInitializeData( st_df_total* );
extern int df_compute(st_df_total *stTotal, st_SoldfList *pstSolDfList);


#endif /* __CHSMD_DISK_H__ */
