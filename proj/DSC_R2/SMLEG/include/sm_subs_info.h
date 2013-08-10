#ifndef _SM_SUBS_INFO_H_
#define _SM_SUBS_INFO_H_

#include "SmApiBlocking_c.h"
#include "SmApiNonBlocking_c.h"

#define MAX_CALLING_STATION_ID_SIZE			16
#define MAX_FRAMEDIP_SIZE		 			16
#define MAX_DOMAIN_SIZE		 				16
typedef struct __subscribers_info {

#define		MAX_PROPORTIES_STR_SIZE			16
#define		MAX_PROPERY_CNT					2

#define		PROPERTY_PACKAGE_ID				"packageId"
#define		PROPERTY_MONITOR				"monitor"

#define		PROPERTY_MONITOR_MODE_OFF		0
#define		PROPERTY_MONITOR_MODE_ON		1

	short 			sPkgNo;
	unsigned int	uiCBit;
	unsigned int	uiPBit;
	unsigned int	uiHBit;
	unsigned char 	szMIN[MAX_CALLING_STATION_ID_SIZE];
	unsigned char 	szFramedIP[MAX_FRAMEDIP_SIZE];
	unsigned char 	szDomain[MAX_DOMAIN_SIZE];
	MappingType 	type;
	//char 			*prop_key[MAX_PROPERY_CNT];
	//char 			*prop_val[MAX_PROPERY_CNT];
	int				prop_size;
	int 			dlogoutTmrMode;
	int				dTrcFlag;
	unsigned short	usSvcID;				
} SUBS_INFO, *PSUBS_INFO;

typedef struct _st_sm_conn_ {
	int 			isPreConnect;
	int 			isConnect;
	SMNB_HANDLE     hdl_nbSceApi;
	SMB_HANDLE      hdl_bSceApi;
} SM_CONN;

#endif /* _SM_SUBS_INFO_H_ */
