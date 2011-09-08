#ifndef __CHSMD_DISK_H__
#define __CHSMD_DISK_H__


typedef struct
{
    char        szSpecial[50];
    char        szMountp[20];
    int         dPercent;
    long long   llUsed;
    long long   llTotal;
} st_Soldf;

typedef struct
{
    int         dCount;
    st_Soldf    stSoldf[20];
} st_SoldfList;

extern int dGetDF(st_SoldfList *stSoldfList);

#endif /* __CHSMD_DISK_H__ */
