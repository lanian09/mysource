#ifndef __CAPD_GLOBAL_H__
#define __CAPD_GLOBAL_H__

#include <stdio.h>
#include <sfm_msgtypes.h>

#define SMS_MSG_FILE   	      "NEW/DATA/SMS_MSG.conf"
#define SMPP_CONF_FILE 	      "NEW/DATA/smpp.conf"
#define MAX_RULE_SET_LIST   5000 
int g_TotalRuleSetCnt;
typedef struct __RuleSetList__ {
	unsigned short  pBit;
	unsigned short  hBit;
	unsigned short  pkgNo;     
} RuleSetList;

#define MAX_RULE_ENTRY_LIST	500
#define STR_ENTRY_NAME_LEN	128
typedef struct __RuleEntryList__ {
	unsigned short  entId;
	char			entName[STR_ENTRY_NAME_LEN];
} RuleEntryList;


typedef struct _st_sms_msg_ {
#define MAX_SMS_TEXT_SIZE		90
	unsigned short hBit;
	unsigned char  text[MAX_SMS_TEXT_SIZE];
} SMS_MSG, *PSMS_MSG;

#define MAX_SCE_NUM		2
typedef struct _SCE_LIST_ {
#define STR_SCE_NAME_LEN	32
#define STR_SCE_IP_LEN		16
	char  sce_name[STR_SCE_NAME_LEN];
	char  sce_ip[STR_SCE_IP_LEN];
} SCE_LIST, *PSCE_LIST;


#endif /* __CAPD_GLOBAL_H__ */

