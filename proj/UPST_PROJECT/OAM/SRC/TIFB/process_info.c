#include <stdio.h>
#include <string.h>

#include "procid.h"

typedef struct {
	int   id;
	char *name;
} process_inf;

/* procid.h 에 있는 것과 동일 해야 함 */
process_inf process[] = {
	
	{ SEQ_PROC_A_SIP,		"A_SIPD" },
	{ SEQ_PROC_A_SIPT,		"A_SIPT" },
	{ SEQ_PROC_A_SIPM,		"A_SIPM" },
	{ SEQ_PROC_A_JNC,		"A_JNC" },
	{ SEQ_PROC_A_IV,		"A_IV" },
	{ SEQ_PROC_A_ONLINE,	"A_ONLINE" },
	{ SEQ_PROC_A_DNS,		"A_DNS" },
	{ SEQ_PROC_A_FTP,		"A_FTP" },
	{ SEQ_PROC_A_WAP20,		"A_WAP20" },

	{ SEQ_PROC_A_WIPI,		"A_WIPI" },
	{ SEQ_PROC_A_2G,		"A_2G" },
	{ SEQ_PROC_A_JAVA,		"A_JAVA" },
	{ SEQ_PROC_A_VOD,		"A_VOD" },
	{ SEQ_PROC_A_MMS,		"A_MMS" },
	{ SEQ_PROC_A_FV,		"A_FV" },
	{ SEQ_PROC_A_EMS,		"A_EMS" },
	{ SEQ_PROC_A_FB,		"A_FB" },
	{ SEQ_PROC_A_XCAP,		"A_XCAP" },
	{ SEQ_PROC_A_WIDGET,	"A_WIDGET" },

	{ SEQ_PROC_A_MSRPM,		"A_MSRPM" },
	{ SEQ_PROC_A_MSRPT,		"A_MSRPT" },
	{ SEQ_PROC_A_VT,		"A_VT" },
	{ SEQ_PROC_A_SCTP,		"A_SCTP" },
	{ SEQ_PROC_A_DIAMETER,	"A_DIAMETER" },
	{ SEQ_PROC_A_RADIUS,	"A_RADIUS" },
	{ SEQ_PROC_A_IM,		"A_IM" },
	{ SEQ_PROC_A_UDP,		"A_UDP" },
	{ SEQ_PROC_A_L2TP,		"A_L2TP" },

	{ SEQ_PROC_A_HTTP,		"A_HTTP" },
	{ SEQ_PROC_A_HTTP0,		"A_HTTP0" },
	{ SEQ_PROC_A_HTTP1,		"A_HTTP1" },
	{ SEQ_PROC_A_HTTP2,		"A_HTTP2" },
	{ SEQ_PROC_A_HTTP3,		"A_HTTP3" },
	{ SEQ_PROC_A_HTTP4,		"A_HTTP4" },

	{ SEQ_PROC_A_CALL,		"A_CALL" },
	{ SEQ_PROC_A_CALL0,		"A_CALL0" },
	{ SEQ_PROC_A_CALL1,		"A_CALL1" },
	{ SEQ_PROC_A_CALL2,		"A_CALL2" },
	{ SEQ_PROC_A_CALL3, 	"A_CALL3" },
	{ SEQ_PROC_A_CALL4, 	"A_CALL4" },

	{ SEQ_PROC_A_TCP,		"A_TCP" },
	{ SEQ_PROC_A_TCP0,		"A_TCP0" },
	{ SEQ_PROC_A_TCP1, 		"A_TCP1" },
	{ SEQ_PROC_A_TCP2, 		"A_TCP2" },
	{ SEQ_PROC_A_TCP3, 		"A_TCP3" },
	{ SEQ_PROC_A_TCP4, 		"A_TCP4" },

	{ SEQ_PROC_A_ITCP,		"A_ITCP" },
	{ SEQ_PROC_A_ITCP0,		"A_ITCP0" },
	{ SEQ_PROC_A_ITCP1,		"A_ITCP1" },
	{ SEQ_PROC_A_ITCP2,		"A_ITCP2" },
	{ SEQ_PROC_A_ITCP3,		"A_ITCP3" },
	{ SEQ_PROC_A_ITCP4,		"A_ITCP4" },
	
	{ SEQ_PROC_A_INET,		"A_INET" },
	{ SEQ_PROC_A_INET0,		"A_INET0" },
	{ SEQ_PROC_A_INET1,		"A_INET1" },
	{ SEQ_PROC_A_INET2,		"A_INET2" },
	{ SEQ_PROC_A_INET3,		"A_INET3" },
	{ SEQ_PROC_A_INET4,		"A_INET4" },


	{ SEQ_PROC_A_IHTTP,		"A_IHTTP" },
	{ SEQ_PROC_A_IHTTP0,	"A_IHTTP0" },
	{ SEQ_PROC_A_IHTTP1,	"A_IHTTP1" },
	{ SEQ_PROC_A_IHTTP2,	"A_IHTTP2" },
	{ SEQ_PROC_A_IHTTP3,	"A_IHTTP3" },
	{ SEQ_PROC_A_IHTTP4,	"A_IHTTP4" },

	{ SEQ_PROC_A_RP,		"A_RP" },
	{ SEQ_PROC_A_RP0,		"A_RP0" },
	{ SEQ_PROC_A_RP1,		"A_RP1" },
	{ SEQ_PROC_A_RP2,		"A_RP2" },
	{ SEQ_PROC_A_RP3,		"A_RP3" },
	{ SEQ_PROC_A_RP4,		"A_RP4" },

	{ SEQ_PROC_A_GRE,		"A_GRE" },
	{ SEQ_PROC_A_GRE0,		"A_GRE0" },
	{ SEQ_PROC_A_GRE1,		"A_GRE1" },
	{ SEQ_PROC_A_GRE2,		"A_GRE2" },
	{ SEQ_PROC_A_GRE3,		"A_GRE3" },
	{ SEQ_PROC_A_GRE4,		"A_GRE4" },

	{ SEQ_PROC_PRE_A,		"PRE_A" },
	{ SEQ_PROC_CAPD,		"CAPD" },

	{ SEQ_PROC_A_RPPI,		"A_RPPI" },
	{ SEQ_PROC_A_RPPI0,		"A_RPPI0" },
	{ SEQ_PROC_A_RPPI1,		"A_RPPI1" },
	{ SEQ_PROC_A_RPPI2,		"A_RPPI2" },
	{ SEQ_PROC_A_RPPI3,		"A_RPPI3" },
	{ SEQ_PROC_A_RPPI4,		"A_RPPI4" },

	{ SEQ_PROC_M_LOG,		"M_LOG" },
	{ SEQ_PROC_O_SVCMON,	"O_SVCMON" },
	{ SEQ_PROC_M_SVCMON,	"M_SVCMON" },
	{ SEQ_PROC_SI_SVCMON, 	"SI_SVCMON" },

	{ SEQ_PROC_A_ROAM,		"A_ROAM" },
	{ SEQ_PROC_M_TRACE,		"M_TRACE" },
	{ SEQ_PROC_SI_DB,		"SI_DB" },
	{ SEQ_PROC_SI_LOG,		"SI_LOG" },
	{ SEQ_PROC_CI_LOG,		"CI_LOG" },
	{ SEQ_PROC_MOND,		"MOND" },
	{ SEQ_PROC_CHGSVCM,		"CHGSVCM" },

	{ SEQ_PROC_CHSMD,		"CHSMD" },
	{ SEQ_PROC_COND,		"COND" },
	{ SEQ_PROC_ALMD,		"ALMD" },
	{ SEQ_PROC_MMCD,		"MMCD" },
	{ SEQ_PROC_SI_SVC,		"SI_SVC" },
	{ SEQ_PROC_CI_SVC,		"CI_SVC" },

	{ SEQ_PROC_FSTAT,		"FSTAT" },
	{ SEQ_PROC_S_MNG,		"S_MNG" },
	{ SEQ_PROC_QMON,		"QMON" },
	{ SEQ_PROC_SI_NMS,		"SI_NMS" },
	{ SEQ_PROC_SNMPIF,		"SNMPIF" },
};

int SeqProcID(char *pname)
{
	int i, size;
	
	size = sizeof(process)/sizeof(process_inf);
	for( i = 0; i < size; i++ ){
		
		if( !strcmp( pname, process[i].name ) ){
			return process[i].id;
		}
	}
	return -1;
}
