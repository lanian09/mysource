#ifndef MMCR_MSGTYPE_H
#define MMCR_MSGTYPE_H

#include "ipaf_svc.h"
#include "ipam_sys.h"

#pragma pack(1)
#define MID_CHG_PROC_PARAM    4

/* cmd type */
/* MMCR -> SDMD */
#define MPCMD_SDMD_SET_DUP_CONF		1 /*Set Duplication Configuration*/
#define MPCMD_SDMD_SO_COND			2 /*Set Duplication Switch-Over Conditions*/
#define MPCMD_SDMD_SWT_ACT_BSD		3 /*Switch-Over Active BSD*/
#define MPCMD_SDMD_SET_DUP_STS		4 /*Set BSD Duplication Status*/

/* MMCR -> WAP1ANA */
#define MPCMD_UAWAP_ADD_WAP_GW		11 /*Add WAP G/W Information*/
#define MPCMD_UAWAP_DEL_WAP_GW		12 /*Delete WAP G/W Information*/
#define MPCMD_UAWAP_CHG_WAP_GW		13 /*Change WAP G/W Information*/
#define MPCMD_UAWAP_SET_TXN_EXT		14 /*Set Transaction Log Extraction Information*/
#define MPCMD_UAWAP_SET_INFO		15 /*Set Transaction Log Extraction Information*/


/* MMCR -> AAAIF */
#define MPCMD_AAA_ADD_INFO			21 /*Add AAA Information*/
#define MPCMD_AAA_DEL_INFO			22 /*Delete AAA Information*/
#define MPCMD_AAA_CHG_INFO			23 /*Change AAA Information*/

/* MMCR -> UDRGEN */
#define MPCMD_UDRG_ADD_URL			31 /*Add URL Charging Information*/
#define MPCMD_UDRG_DEL_URL			32 /*Delete URL Charging Information*/
#define MPCMD_UDRG_CHG_URL			33 /*Change URL Charging Information*/
#define MPCMD_UDRG_SET_CONF			34 /*Set UDR Parameter Configuration*/
#define MPCMD_UDRG_SET_DUMP			35 /*Set UDR Dump Criteria*/
#define MPCMD_UDRG_ADD_TXC          36
#define MPCMD_UDRG_DEL_TXC          37
#define MPCMD_UDRG_CHG_TXC          38
#define MPCMD_UDRG_SET_PPS			39

/* MMCR -> PDSN */
#define MPCMD_PDSN_ADD_INFO			41 /*Add PDSN Information*/
#define MPCMD_PDSN_DEL_INFO			42 /*Delete PDSN Information*/
#define MPCMD_PDSN_CHG_INFO			43 /*Change PDSN Information*/

/* MMCR -> IP POOL */
#define MPCMD_IPPOOL_ADD			51 /*Add PDSN IP Pool Information*/
#define MPCMD_IPPOOL_DEL			52 /*Delete PDSN IP Pool Information*/
#define MPCMD_IPPOOL_CHG			53 /*Change PDSN IP Pool Information*/

/* MMCR -> SVC TYPE */
#define MPCMD_SVC_TYPE_ADD			61 /*Add Data Service Type Information*/
#define MPCMD_SVC_TYPE_DEL			62 /*Delete Data Service Type Information*/
#define MPCMD_SVC_TYPE_CHG			63 /*Change Data Service Type Information*/

/* MMCR -> CALL_TRC */
#define MPCMD_REG_CALL			71
#define MPCMD_CANC_CALL			72

/* MMCR -> UDRGEN */
#define MPCMD_SVC_OPT_ADD			81 /*Add Data Service Option Information*/
#define MPCMD_SVC_OPT_DEL			82 /*Delete Data Service Option Information*/
#define MPCMD_SVC_OPT_CHG			83 /*Change Data Service Option Information*/

typedef struct _MpConfigCmd{
    char        cmdType;
    char        cmdParaCnt;
#define MAX_PARA_CNT    20
#define PARA_STR_LEN    100         /* change the length from 20 to 100 owing to URL Address */
    char        cmdPara[MAX_PARA_CNT][PARA_STR_LEN];
}MpConfigCmd, *pMpConfigCmd;

#pragma pack()
#endif
