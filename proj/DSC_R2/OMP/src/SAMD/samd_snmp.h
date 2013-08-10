#ifndef SAMD_SNMP_H
#define SAMD_SNMP_H

#define MAX_OID_NUM		57

char    *oid_names[MAX_OID_NUM+1] = {
/* CPU Information */
/*	"enterprises.9.2.10.3.0",		*//* CPU Name */
#define	SNMP_OID_RANGE_CPU_START	0
#define	SNMP_OID_RANGE_CPU_END		2
	"enterprises.9.2.1.56.0",
	"enterprises.9.2.1.57.0",
	"enterprises.9.2.1.58.0",

/* Memory Information */
#define	SNMP_OID_RANGE_MEM_START	3
#define	SNMP_OID_RANGE_MEM_END		4
	"enterprises.9.9.48.1.1.1.5.1",		/* Used */
	"enterprises.9.9.48.1.1.1.6.1",		/* Free */
/*	"enterprises.9.9.48.1.1.1.7.1",		*//* Largest */

/*------------------------------------------------*\
\* Each Port Up/Down Information 0 = DOWN, 1 = UP */
#define	SNMP_OID_RANGE_IFN_START	5
#define	SNMP_OID_RANGE_IFN_END		56
/*	"enterprises.9.2.2.1.1.20.1", 		*//*vlan interface*/
/*	"enterprises.9.2.2.1.1.20.10",		*//*vlan interface*/
/*	"enterprises.9.2.2.1.1.20.21",		*//*vlan interface*/
/*	"enterprises.9.2.2.1.1.20.30",		*//*vlan interface*/
/*	"enterprises.9.2.2.1.1.20.31",		*//*vlan interface*/
/*	"enterprises.9.2.2.1.1.20.5001",		*//*EtherChannel*/
/*	"enterprises.9.2.2.1.1.20.5049",		*//*Loopback 0*/
	"enterprises.9.2.2.1.1.20.10101",	  /* RJ45 Type Interface start */
	"enterprises.9.2.2.1.1.20.10102",
	"enterprises.9.2.2.1.1.20.10103",
	"enterprises.9.2.2.1.1.20.10104",
	"enterprises.9.2.2.1.1.20.10105",
	"enterprises.9.2.2.1.1.20.10106",
	"enterprises.9.2.2.1.1.20.10107",
	"enterprises.9.2.2.1.1.20.10108",
	"enterprises.9.2.2.1.1.20.10109",
	"enterprises.9.2.2.1.1.20.10110",
	"enterprises.9.2.2.1.1.20.10111",
	"enterprises.9.2.2.1.1.20.10112",
	"enterprises.9.2.2.1.1.20.10113",
	"enterprises.9.2.2.1.1.20.10114",
	"enterprises.9.2.2.1.1.20.10115",
	"enterprises.9.2.2.1.1.20.10116",
	"enterprises.9.2.2.1.1.20.10117",
	"enterprises.9.2.2.1.1.20.10118",
	"enterprises.9.2.2.1.1.20.10119",
	"enterprises.9.2.2.1.1.20.10120",
	"enterprises.9.2.2.1.1.20.10121",
	"enterprises.9.2.2.1.1.20.10122",
	"enterprises.9.2.2.1.1.20.10123",
	"enterprises.9.2.2.1.1.20.10124",
	"enterprises.9.2.2.1.1.20.10125",
	"enterprises.9.2.2.1.1.20.10126",
	"enterprises.9.2.2.1.1.20.10127",
	"enterprises.9.2.2.1.1.20.10128",
	"enterprises.9.2.2.1.1.20.10129",
	"enterprises.9.2.2.1.1.20.10130",
	"enterprises.9.2.2.1.1.20.10131",
	"enterprises.9.2.2.1.1.20.10132",
	"enterprises.9.2.2.1.1.20.10133",
	"enterprises.9.2.2.1.1.20.10134",
	"enterprises.9.2.2.1.1.20.10135",
	"enterprises.9.2.2.1.1.20.10136",
	"enterprises.9.2.2.1.1.20.10137",
	"enterprises.9.2.2.1.1.20.10138",
	"enterprises.9.2.2.1.1.20.10139",
	"enterprises.9.2.2.1.1.20.10140",
	"enterprises.9.2.2.1.1.20.10141",
	"enterprises.9.2.2.1.1.20.10142",
	"enterprises.9.2.2.1.1.20.10143",
	"enterprises.9.2.2.1.1.20.10144",
	"enterprises.9.2.2.1.1.20.10145",
	"enterprises.9.2.2.1.1.20.10146",
	"enterprises.9.2.2.1.1.20.10147",
	"enterprises.9.2.2.1.1.20.10148",	  /* RJ45 Type interface End */
	"enterprises.9.2.2.1.1.20.10149",	  /* GBic Type Interface Start */
	"enterprises.9.2.2.1.1.20.10150",
	"enterprises.9.2.2.1.1.20.10151",
	"enterprises.9.2.2.1.1.20.10152",	  /* GBic Type Interface End */
	NULL
};

#define PD_RJ45_NUMBER			52
char *RJ45PhysOID[PD_RJ45_NUMBER] = {
	"enterprises.9.2.2.1.1.2.10101",
	"enterprises.9.2.2.1.1.2.10102",
	"enterprises.9.2.2.1.1.2.10103",
	"enterprises.9.2.2.1.1.2.10104",
	"enterprises.9.2.2.1.1.2.10105",
	"enterprises.9.2.2.1.1.2.10106",
	"enterprises.9.2.2.1.1.2.10107",
	"enterprises.9.2.2.1.1.2.10108",
	"enterprises.9.2.2.1.1.2.10109",
	"enterprises.9.2.2.1.1.2.10110",
	"enterprises.9.2.2.1.1.2.10111",
	"enterprises.9.2.2.1.1.2.10112",
	"enterprises.9.2.2.1.1.2.10113",
	"enterprises.9.2.2.1.1.2.10114",
	"enterprises.9.2.2.1.1.2.10115",
	"enterprises.9.2.2.1.1.2.10116",
	"enterprises.9.2.2.1.1.2.10117",
	"enterprises.9.2.2.1.1.2.10118",
	"enterprises.9.2.2.1.1.2.10119",
	"enterprises.9.2.2.1.1.2.10120",
	"enterprises.9.2.2.1.1.2.10121",
	"enterprises.9.2.2.1.1.2.10122",
	"enterprises.9.2.2.1.1.2.10123",
	"enterprises.9.2.2.1.1.2.10124",
	"enterprises.9.2.2.1.1.2.10125",
	"enterprises.9.2.2.1.1.2.10126",
	"enterprises.9.2.2.1.1.2.10127",
	"enterprises.9.2.2.1.1.2.10128",
	"enterprises.9.2.2.1.1.2.10129",
	"enterprises.9.2.2.1.1.2.10130",
	"enterprises.9.2.2.1.1.2.10131",
	"enterprises.9.2.2.1.1.2.10132",
	"enterprises.9.2.2.1.1.2.10133",
	"enterprises.9.2.2.1.1.2.10134",
	"enterprises.9.2.2.1.1.2.10135",
	"enterprises.9.2.2.1.1.2.10136",
	"enterprises.9.2.2.1.1.2.10137",
	"enterprises.9.2.2.1.1.2.10138",
	"enterprises.9.2.2.1.1.2.10139",
	"enterprises.9.2.2.1.1.2.10140",
	"enterprises.9.2.2.1.1.2.10141",
	"enterprises.9.2.2.1.1.2.10142",
	"enterprises.9.2.2.1.1.2.10143",
	"enterprises.9.2.2.1.1.2.10144",
	"enterprises.9.2.2.1.1.2.10145",
	"enterprises.9.2.2.1.1.2.10146",
	"enterprises.9.2.2.1.1.2.10147",
	"enterprises.9.2.2.1.1.2.10148",	  /* RJ45 Type interface End */
	"enterprises.9.2.2.1.1.2.10149",	  /* GBic Type Interface Start */
	"enterprises.9.2.2.1.1.2.10150",
	"enterprises.9.2.2.1.1.2.10151",
	"enterprises.9.2.2.1.1.2.10152",	  /* GBic Type Interface End */
};

/* 2009.04.14 by dhkim */
#define	MAX_SCE_NUMBER						32
#define RDR_PORT                            33000

char	*oid_sce_names[MAX_SCE_NUMBER+1] = {
#define SNMP_OID_RANGE_SCE_CPU_START		0
#define SNMP_OID_RANGE_SCE_CPU_END			8
#define SNMP_SCE_CPU_CNT					3
	".1.3.6.1.4.1.5655.4.1.9.1.1.35.1.1", 			// tpCpuUtilization
	".1.3.6.1.4.1.5655.4.1.9.1.1.35.1.2",
	".1.3.6.1.4.1.5655.4.1.9.1.1.35.1.3",
	".1.3.6.1.4.1.5655.4.1.9.1.1.38.1.1",			// tpFlowsCapacityUtilization
	".1.3.6.1.4.1.5655.4.1.9.1.1.38.1.2",
	".1.3.6.1.4.1.5655.4.1.9.1.1.38.1.3",
	".1.3.6.1.4.1.5655.4.1.9.1.1.41.1.1",			// tpServiceLoss
	".1.3.6.1.4.1.5655.4.1.9.1.1.41.1.2",
	".1.3.6.1.4.1.5655.4.1.9.1.1.41.1.3",
#define SNMP_OID_RANGE_SCE_MEM_START		9
#define SNMP_OID_RANGE_SCE_MEM_END			10
	".1.3.6.1.4.1.5655.4.1.5.1.0",					// 10 :  diskNumUsedBytes
	".1.3.6.1.4.1.5655.4.1.5.2.0",					// diskNumFreeBytes
#define SNMP_OID_RANGE_SCE_SYS_START		11
#define SNMP_OID_RANGE_SCE_SYS_END			17
#define SNMP_OID_SCE_SYS_STATUS				11
#define SNMP_OID_SCE_SYS_POWER_ALARM		12
#define SNMP_OID_SCE_SYS_FAN_ALARM			13
#define SNMP_OID_SCE_SYS_TEMP_ALARM         14
#define SNMP_OID_SCE_SYS_VOLT_ALARM         15
#define SNMP_OID_SCE_SYS_INTRO_USER			16
#define SNMP_OID_SCE_SYS_ACTIVE_USER		17
	".1.3.6.1.4.1.5655.4.1.1.1.0", 					// sysOperationalStatus , 1:other, 2:boot, 3:operational, 4:warning, 5: failure
	".1.3.6.1.4.1.5655.4.1.2.2.0",					// pchassisPowerSupplyAlarm 
	".1.3.6.1.4.1.5655.4.1.2.3.0",					// pchassisFansAlarm
	".1.3.6.1.4.1.5655.4.1.2.4.0",					// pchassisTempAlarm
	".1.3.6.1.4.1.5655.4.1.2.5.0",					// pchassisVoltageAlarm
	".1.3.6.1.4.1.5655.4.1.8.1.1.1.1",				// subscribersNumIntroduced
	".1.3.6.1.4.1.5655.4.1.8.1.1.9.1",				// subscribersNumActive
#define SNMP_OID_RANGE_SCE_MODULE           18
	".1.3.6.1.4.1.5655.4.1.3.1.1.16.1",				// pmoduleOperStatus
#define SNMP_OID_RANGE_SCE_LINK_START		19
#define SNMP_OID_RANGE_SCE_LINK_END			20
	".1.3.6.1.4.1.5655.4.1.4.1.1.5.1.1",			//20 : linkOperMode
	"PCUBE-SE-MIB::linkAdminModeOnFailure.1.1",		//21 : AdminModeOnFailure
#define SNMP_OID_RANGE_SCE_PORT_START       21
#define SNMP_OID_RANGE_SCE_PORT_END         26
	".1.3.6.1.4.1.5655.4.1.10.1.1.10.1.1", 			// pportOperStatus , 1: other, 2: up, 3: reflectionForcingDown, 4: redundancyForcingDown, 5: otherDown
	".1.3.6.1.4.1.5655.4.1.10.1.1.10.1.2",
	".1.3.6.1.4.1.5655.4.1.10.1.1.10.1.3",
	".1.3.6.1.4.1.5655.4.1.10.1.1.10.1.4",
	".1.3.6.1.4.1.5655.4.1.10.1.1.10.1.5",
	".1.3.6.1.4.1.5655.4.1.10.1.1.10.1.6",

#define SNMP_OID_RANGE_SCE_DESTA_RDR_START		27
#define SNMP_OID_RANGE_SCE_DESTA_RDR_END		28
	".1.3.6.1.4.1.5655.4.1.6.2.1.4",				// rdrFormatterDestStatus Dest-A
	".1.3.6.1.4.1.5655.4.1.6.2.1.5",				// rdrFormatterDestConnectionStatus Dest-A
#define SNMP_OID_RANGE_SCE_DESTB_RDR_START		29
#define SNMP_OID_RANGE_SCE_DESTB_RDR_END		30	
	".1.3.6.1.4.1.5655.4.1.6.2.1.4",				// rdrFormatterDestStatus Dest-B
	".1.3.6.1.4.1.5655.4.1.6.2.1.5",				// rdrFormatterDestConnectionStatus Dest-B
#define SNMP_OID_RANGE_SCE_VERSION          31
	".1.3.6.1.4.1.5655.4.1.1.3.0",					// sysVersion
	NULL
};
/* end by dhkim */

#define	MAX_SCE_FLOW_NUMBER					3
char	*oid_sce_flow_[MAX_SCE_FLOW_NUMBER+1] = {
#define SNMP_OID_RANGE_SCE_FLOW_START		0
#define SNMP_OID_RANGE_SCE_FLOW_END			2
	"PCUBE-SE-MIB::tpNumActiveFlows.1.1",
	"PCUBE-SE-MIB::tpNumActiveFlows.1.2",
	"PCUBE-SE-MIB::tpNumActiveFlows.1.3",
	NULL
};

#define SNMP_L2_COMMUNITY     				"LGT"
#define SNMP_L2_TIMEOUT       				100000L*5
#define SNMP_L2_RETRIES       				2
#define	MAX_L2_OID_NUMBER					29

char    *oid_L2_names[MAX_L2_OID_NUMBER+1] = {
/*--------------------------------------------------
 * Each Port Up/Down Information 2 = DOWN, 1 = UP */
#define	SNMP_L2_OID_RANGE_IFN_START			0
#define	SNMP_L2_OID_RANGE_IFN_END			23
	"ifOperStatus.10101",
	"ifOperStatus.10102",
	"ifOperStatus.10103",
	"ifOperStatus.10104",
	"ifOperStatus.10105",
	"ifOperStatus.10106",
	"ifOperStatus.10107",
	"ifOperStatus.10108",
	"ifOperStatus.10109",
	"ifOperStatus.10110",
	"ifOperStatus.10111",
	"ifOperStatus.10112",
	"ifOperStatus.10113",
	"ifOperStatus.10114",
	"ifOperStatus.10115",
	"ifOperStatus.10116",
	"ifOperStatus.10117",
	"ifOperStatus.10118",
	"ifOperStatus.10119",
	"ifOperStatus.10120",
	"ifOperStatus.10121",
	"ifOperStatus.10122",
	"ifOperStatus.10123",
	"ifOperStatus.10124",
/* CPU Information */
#define	SNMP_L2_OID_RANGE_CPU_START			24
#define	SNMP_L2_OID_RANGE_CPU_END			26
	"enterprises.9.2.1.56.0",
	"enterprises.9.2.1.57.0",
	"enterprises.9.2.1.58.0",
/* Memory Information */
#define	SNMP_L2_OID_RANGE_MEM_START			27
#define	SNMP_L2_OID_RANGE_MEM_END			28
	"CISCO-MEMORY-POOL-MIB::ciscoMemoryPoolUsed.1",		/* Used */
	"CISCO-MEMORY-POOL-MIB::ciscoMemoryPoolFree.1",		/* Free */
	"NULL"
};

	
#endif /* SAMD_SNMP_H */
