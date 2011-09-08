/**A.1*  File Inclusion ***********************************/
#include <stdlib.h>

#include "mmcdef.h"

/**B.1*  Definition of New Constants *********************/

/**B.2*  Definition of New Type  **************************/
typedef struct {
	int	 id;
	char *mess;
} JT_ErrMess;

/**C.1*  Declaration of Variables  ************************/

JT_ErrMess JaErrMess[] = {
	{eMMCDTimeOut, 				"COMMUNICATION TIMEOUT"},				/* -23 */
	{eMANDATORY_MISSED ,		"MISSING MANDATORY PARA"},				/* -509	*/
	{eENUM_LIST_FORMAT ,		"ERROR IN ENUMLIST"},					/* -510	*/
	{eENUM_LIST_ELEMENT ,		"INVALID CONTENT IN ENUMLIST"},			/* -511	*/
	{eLOAD_CMD_FILE ,			"ERROR IN LOADING CMD FILE"},			/* -520	*/
	{eINVALID_CMD ,				"INVALID COMMANDS"},					/* -521	*/
	{eINVALID_PAPA_IN_CMD ,		"INVALID_PAPA_IN_CMD"},					/* -522	*/
	{eINVALID_SUBSYSTEM ,   	"INVALID SUBSYSTEM"},					/* -523	*/
	{eINVALID_PAPA_CONTENT,		"INVALID_PAPA_CONTENT"},				/* -524	*/
	{eNOT_FOUND_PRN_FUNC  ,		"UNDEFINED PRINT-OUT LIB"},				/* -530	*/
	{eINVALIDE_SEARCH_TIME,     "INVALID SEARCH TIME "},				/* -565	*/
	{eAdminInfoNotRegistered,	"NOT EXIST ADMINISTRATOR"},				/* -575	*/
    {eDBQUERYERROR,				"DB QUERY ERROR"},						/* -610	*/
    {eOVERMAXROW,				"OVER MAX ROW"},						/* -614	*/
	{eINVALID_IP, 				"INVALID IP FORMAT"},					/* -700 */
	{eBadParameter, 			"BAD PARAMETER"},						/* -1001 */
	{eDataReadError, 			"DATA READ ERROR - INVALID DATA"},		/* -1003 */
	{eBlockNotRegistered, 		"BLOCK IS NOT REGISTERED"}, 			/* -1020 */
	{eProcAliveError, 			"ALREADY ALIVED"},						/* -1021 */
	{eCHSMDNotDEAD, 			"CHSMD CANNOT BE KILLED"}, 				/* -1022 */
	{eProcDeadError, 			"ALREADY DEAD"},						/* -1024 */
	{eNeedProcName, 			"MISSED PROCESS NAME"},					/* -1027 */
	{eGeneralError, 			"GENERAL FAILURE"}, 					/* -1102 */
	{eNotFoundData, 			"CANNOT FOUND MATCHED DATA"},			/* -1051 */
	{eAlreadyRegisteredData,	"ALREADY REGISTERED DATA"},				/* -2023 */
	{eAlreadyMaskChn,			"ALREADY MASKED CHANNEL"},				/* -2030 */
	{eAlreadyUMaskChn,			"ALREADY UNMASKED CHANNEL"},			/* -2031 */
	{eNotFindSysNo,				"CAN NOT FIND SYSTEM NUMBER"},			/* -2032 */
	{eDuplicateEntry,			"DUPLICATED DATA"},						/* -2033 */
	{eINVALID_PARAM_RANGE,		"INVALID PARAMETER RANGE"},				/* -3000 */
	{eTABLE_NOT_EXIST,			"TABLE NOT EXIST"},						/* -3002 */
	{eNotCompleteProcess,		"NOT COMPLETE"},						/* -3003 */
#if 0


	{eINVALID_PAPA_ID,		"eINVALID_PAPA_ID"},			/*  -525	*/

	{eDUPLICATE_DSCP_IP,		"DUPLICATE DSCP IP"},			/*	-534	*/
	{eNO_SESSION,				"NO SESSION"},					/*	-535	*/
	{eNO_SUCH_SOCKET_FD,		"NO SUCH SOCKET FD"},			/*	-536	*/
	{eINVALID_DSCP_ID,			"INVALID DSCP ID"},				/*	-537	*/
	{eRELOAD_FAIL_DSCP_INFO,	"RELOAD FAIL DSCP INFO"},		/*	-538	*/
	{eCONNECT_REFUSE_BY_DSCP,	"CONNECT REFUSE BY DSCP"},		/*	-539	*/
	{eINACT_DSCP_ID,			"INACTIVATE DSCP ID"},			/*	-540	*/
	{eMAX_SESSION_CNT_OVER,		"MAX SESSION COUNT OVER"},		/*	-541	*/
	{eALREADY_INACT_DSCPID,		"ALREADY INACT DSCP ID"},		/*	-542	*/
	{eALREADY_ACT_DSCPID,		"ALREADY ACT DSCP ID"},			/*	-543	*/
	{eREQUIRED_CHANGE_INFO,		"REQUIRED CHANGE INFOR"},		/*	-544	*/
	{eDUPLICATE_DSCP_ID,		"DUPLICATE DSCP ID"},			/*	-545	*/
	{eMAX_DSCP_LIST_OVER,		"MAX DSCP LIST OVER"},			/*	-546	*/
	{eREQUIRED_MORE_PARA,		"REQUIRED MORE PARAMETER"},		/*	-547	*/
	{eNOTREG_WIN_SERVICE,		"NOT REGISTERED SERVICE"},		/*	-548	*/
	{eALREADY_EXIST_ROUTE,		"ALREADY EXIST ROUTE"},			/*	-549	*/
	{ePFX_OUT_OF_BOUND,			"PREFIX OUT OF BOUND"},			/*	-550	*/
	{eNO_SUCH_DSCPID,			"NO SUCH DSCP ID"},				/*	-551	*/
	{eNO_SUCH_DSCP_ROUTE,		"NO SUCH DSCP ROUTE"},			/*	-552	*/
	{eINVALID_DSCP_TIMER,		"INVALID DSCP TIMER"},			/*	-553	*/
	{eFAIL_CREATE_ROUTE_LIST,	"FAIL CREATE ROUTE LIST"},		/*	-554	*/
	{eOVERFLOW_ROUTE_LIST,		"OVERFLOW ROUTE LIST"},			/*	-555	*/
	{eALREADY_MASK_CHNL,		"ALREADY MASK CHANNEL"},		/*	-556	*/
	{eALREADY_UMASK_CHNL,		"ALREADY UMASK CHANNEL"},		/*	-557	*/
	{eINVALIDE_BDF_RANGE,       "INVALID BDF RANGE [1-16]"},	/*	-559	*/

    {eSETIPAFCONFFILE,          "FAIL SETTING INIT_IPAF.dat FILE"},			/*	-560	*/
    {eSETIPAFCONFPARAUNDEFINE,  "UNDEFINE PARAMTER INIT_IPAF CONFIGURE"},	/*	-561	*/
    {eSERCATFILENOTFOUND,       "NOT FOUND SERVICE_CATEGORY.dat FILE"},		/*	-562	*/
    {eIPPOOLLISTFILENOTFOUND,   "NOT FOUND IPAF: IP_POOL.dat FILE"},		/*	-563	*/
	{eINVALIDE_IPAM_RANGE,      "INVALID IPAM RANGE [1-2]"},				/*	-564	*/
	{eINVALIDE_IPAM_LOAD_RANGE, "INVALID IPAM LOAD RANGE[0-1] "},			/*	-566	*/

	/* 2003. 7. 22 */
	{eMAX_SVC_LIST_OVER,        "MAX SVC LIST OVER "}, 			/*	-570	*/
	{eDUPLICATE_SVC_LIST,       "DUPLICATE SVC LIST"}, 			/*	-571	*/
	{eNO_SUCH_WIN_SERVICE,      "NO SUCH WIN SERVICE"},			/*	-572	*/
	{eBDFMaskSetFail,			"EDF MASK SETTING FAIL"},		/*	-573	*/
	{eEQUIPNotRegistered, "EQUIP NOT REGISTERED"},						/*	-574	*/

    {ePRNTNOTFOUND,	"Integrity constraint (Parent key not found)"},	/*	-611	*/
    {eCHDRCDFOUND,	"Integrity constraint (Child record found)"},	/*	-612	*/
	{eOVERMAXSVC,	"OVER MAX SERVICE COUNT"},						/*	-613	*/

	{eINVALID_IP_RANGE, "INVALID IP RANGE"},                /* -701 */
	{eNEED_CHG_DATA, "NEED TO CHANGE DATA"},				/* -705 */
	{eAlreadyMASK, "ALREADY MASK ALM "},					/* -1004 */
	{eAlreadyUMASK, "ALREADY UMASK ALM "},					/* -1005 */
	{eNotSupportIdx, "UNINITIAL COMPONENT IDX"},			/* -1006 */
	{eBiggerWARNToCRI, "BIGGER WARN THAN CRI"}, 			/* -1007 */

	{eMissingMandatory	, "MISSING MANDATORY"},		/* -1008 MISSING MANDATORY */

	{eCHSMDNotALIVE, "CHSMD ALREADY KILLED"},				/* -1023 */
	{eProcMaskError, "MASKED STATUS"},						/* -1025 */
	{eProcUMaskError, "CANNOT UMASK"},						/* -1026 */
	{eCannotBlockMask, "CANNOT MASK"},						/* -1030 */
	{eInvalidSysNo, "INAVLID SYSNO"},						/* -1040 */

	{eInvalidNasInfo, "INVALID NAS INFORMATION"},           /* -1050 */


	{eINVALID_IPAFNO	, "INVALID IPAF NO"},				/* -2001 */
	{eINVALID_IPADDRESS , "INVALID IP ADDRESS"}, 			/* -2002 */
	{eINVALID_NASTYPE	, "INVALID NAS TYPE"}, 				/* -2003 */
	{eALREADY_IP		, "ALREADY REGISTERED IP ADDRESS"}, /* -2004 */
	{eNOTREG_IP			, "NOT REGISTERED IP ADDRESS"},		/* -2005 */
	{ePREFIX_ERR		, "INVALID PREFIX"},				/* -2006 */
	{eNOTREG_PREFIX		, "NOT REGISTERED PREFIX"},			/* -2007 */
	{eALREADY_PREFIX	, "ALREADY REGISTERED PREFIX"},		/* -2008 */
	{eNOTREG_SERCAT		, "NOT REGISTERED SERVICE CATEGORY"},	/* -2009 */
	{ePKG_VER_ERR		, "PKG VERSION NO ERROR"},				/* -2010 */
	{eIPAF_NOT_LOGIN	, "IPAF CHANNEL DISCONNECTED"},					/* -2011 */
	{eALREADY_VER		, "ALREADY REGISTERED VERSION"},		/* -2012 */
	{eSERCAT_MAX_OVER	, "MAX SERVICE CATEGORY COUNT"},		/* -2013 */
	{eCATEGORY_MAX_OVER	, "MAX CATEGORY COUNT"},				/* -2014 */
	{eALREADY_SERVICE	, "ALREADY REGISTERED SERVICE CATEGORY"},	/* -2015 */
	{eGROUPPORT			, "GROUP ID, PORT NOT MATCH"},				/*-2016*/
	{ePERIOD_OVER		, "SEARCH PERIOD OVER [MAXIMUM SEARCH PERIOD 1-DAY]"},/*-2017*/
	{eNETMASK_ERROR		, "NETMASK BIT ERROR"},							/*	-2018	*/
	{eGRUPPORTERR		, "CAN NOT CHANGE PORT FOR GROUP ID NUMBER 1"},	/*	-2019	*/
	{eNASTYPEERROR		, "NAS TYPE ERROR"},							/*	-2020	*/

	{eAlreadyMaskInterlockChn,	"ALREADY MASKED INTERLOCK CHANNEL"},	/*	-2034	*/
	{eAlreadyUMaskInterlockChn,	"ALREADY UNMASKED INTERLOCK CHANNEL"},	/*	-2035	*/

	{eBadNetmaskRoam,			"NETMASK SHOULD BE IN ROAM EQUIP"}		/*	-3004	*/
#endif
/* TO BE DETERMINED	*/
};


/**D.1*  Definition of Functions  *************************/
/**D.2*  Definition of Functions  *************************/

int Errcmp (const void *a, const void *b)
{
	return ((JT_ErrMess *)b)->id - ((JT_ErrMess *)a)->id;
}

char *MH_ErrMess(short code)
{
	JT_ErrMess	*p, dummy;

	dummy.id = code;
	if( (p = (JT_ErrMess*)bsearch(&dummy, JaErrMess, sizeof(JaErrMess)/sizeof(JT_ErrMess), sizeof(JT_ErrMess), Errcmp)))
		return p->mess;

	return "UNKNOWN ERROR";
}
