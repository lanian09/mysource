/*******************************************************************************
			DQMS Project

	Author   :
	Section  : S_MNG
	SCCS ID  : @(#)s_mng_def.h	1.1
	Date     :
	Revision History :

	Description :

	Copyright (c) uPRESTO 2005
*******************************************************************************/
#ifndef __S_MNG_DEF_H__
#define __S_MNG_DEF_H__

#include <common_stg.h>

#define INVALID_FLAG		-1

#define MAX_TAF_NUM			32
#define MAX_TAM_NUM			2
#define DEF_SYS_TAM			0
#define DEF_SYS_TAF			1

#define DEF_STAT_FAULT		0
#define DEF_STAT_LOAD		1
#define DEF_STAT_TRAFFIC	2

#define DEF_SQLSTMT_SIZE	1024
#define DEF_TABLENAME_SIZE	256

#define DEF_FLAG_DIFF		0x80
#define DEF_FLAG_START		0x01
#define DEF_FLAG_END		0x02

#define LEN_YYYYMMDDHHMM	13
#define LEN_YYYYMM			6

#define INVALID_VALUE		100

#define WATCHFILTER_FILE	"WatchFilter.dat"
#define TAM_SVCLIST_FILE	"TamSvclist.dat"


#define	DEF_IRMHASH_CNT			1009

typedef struct _st_CallKey {
    U8      szIMSI[MAX_MIN_SIZE];
}st_CallKey, *pst_CallKey;

#define RPPISESS_KEY        st_CallKey
#define PRPPISESS_KEY       pst_CallKey

#define RPPISESS_KEY_SIZE   sizeof(RPPISESS_KEY)

typedef struct _st_Model_hasdata {
    U8              szModel[MAX_MODEL_SIZE];
    U8              szMIN[MAX_MIN_SIZE];
} HData_Model;
#define HDATA_MODEL_SIZE 	sizeof(HData_Model)

typedef struct _st_Svc_Stat{
	S32		dSvcID;
	S32		dOnOff;
} st_Svc_Stat;

#define MAX_SVC_STAT_CNT	16
typedef struct _st_Taf_SvcStat{
	st_Svc_Stat stSvcStat[MAX_SVC_STAT_CNT];
} st_Taf_Svcinfo;



#endif	/*	_S_MNG_DEF_H__	*/

