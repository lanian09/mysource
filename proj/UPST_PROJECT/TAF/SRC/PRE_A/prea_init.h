#ifndef __PRE_A_INIT_H__
#define __PRE_A_INIT_H__

/**
 *	Define cons.
 */
#define DEF_ROAMHASH_CNT			503
#define MAX_SERVICE_CNT             20

/**
 *	Define structures
 */
#define DEF_MAX_BIT         1024
typedef struct _st_SYSCFG_INFO
{
    S32         range;
    S32         mod;
    U8          bit[DEF_MAX_BIT];
} st_SYSCFG_INFO, *pst_SYSCFG_INFO;

typedef struct _st_ROAMHash_Key
{
    U32         ip;
    U32         reserved;
} st_ROAMHash_Key, *pst_ROAMHash_Key;
#define DEF_ROAMHASHKEY_SIZE        sizeof(st_ROAMHash_Key)
    
typedef struct _st_ROAMHash_Data
{   
    U32         systype; 
    U32         reserved;
} st_ROAMHash_Data, *pst_ROAMHash_Data;
#define DEF_ROAMHASHDATA_SIZE       sizeof(st_ROAMHash_Data)

/**
 *	Declare func.
 */
extern int dInitPREAProc(stMEMSINFO **pMEMSINFO, stHASHOINFO **pIPFRAGHASH, stTIMERNINFO **pIPFRAGTIMER, stHASHOINFO **pLPREAHASH, stHASHOINFO **pLPREASCTP, stHASHOINFO **pROAMHASH, st_SYSCFG_INFO *pSYSCFG);
extern S32 dGetSYSTYPE();
extern S32 dGetSYSCFG(st_SYSCFG_INFO *pSYSCFG);
extern void Read_MNData(stHASHOINFO *pROAMHASH);
extern void Read_SVRData( stHASHOINFO *pLPREAHASH );
extern void Read_SCTPData( stHASHOINFO *pLPREASCTP );
extern void Read_SVC_ONOFF();
extern void UserControlledSignal(int sign);
extern void FinishProgram();
extern void IgnoreSignal(int sign);
extern void SetUpSignal();
extern void vIPFRAGTimerReConstruct(stHASHOINFO *pHASH, stTIMERNINFO *pTIMER);

#endif /* __PRE_A_INIT_H__ */
