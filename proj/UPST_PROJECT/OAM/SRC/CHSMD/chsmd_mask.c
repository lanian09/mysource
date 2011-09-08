/**A.1 * File Include *******************************/

/* SYS HEADER */
#include <errno.h>		/* errno */
#include <string.h>		/* strerror */
/* LIB HEADER */
#include "filedb.h"		/* pst_NTAM, pst_DIRECT_MNG, pst_SWITCH_MNG */
#include "loglib.h"
#include "filelib.h"	/* write_file(), read_file() */
/* PRO HEADER */
#include "path.h"
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
#include "almstat.h"	/* MASK */
/* LOC HEADER */
#include "chsmd_mask.h"

/**B.1*  Definition of New Constants *********************/
/**B.2*  Definition of New Type  **************************/
/**C.1*  Declaration of Variables  ( Local ) **************/
/**C.2*  Declaration of Variables  ************************/
pst_NTAM       fidb;
pst_DIRECT_MNG director;
pst_SWITCH_MNG swch;

st_ChnlMask		maskInfo;

/**D.1*  Definition of Functions  ( Local ) ***************/
/**D.2*  Definition of Functions  *************************/

void subChannelMaskInfoSave()
{
	int i;
	for( i = 0; i < MAX_CH_COUNT; i++ ){
		if( fidb->NTAFChnl[i] & MASK ){
			maskInfo.ucSubChannel[i] |= MASK;
		}
	}
}

void subChannelMaskInfoRestore()
{
	int i;
	for( i = 0; i < MAX_CH_COUNT; i++ ){
		if( maskInfo.ucSubChannel[i] & MASK ){
			fidb->NTAFChnl[i] |= MASK;
		}
	}
}

void interlockMaskInfoSave()
{
	int i;
	for( i = 0; i < MAX_ICH_COUNT; i++ ){
		if( fidb->cInterlock[i] & MASK ){
			maskInfo.ucInterlock[i] |= MASK;
		}
	}
}

void interlockMaskInfoRestore()
{
	int i;
	for( i = 0; i < MAX_ICH_COUNT; i++ ){
		if( maskInfo.ucInterlock[i] & MASK ){
			fidb->cInterlock[i] |= MASK;
		}
	}
}

void directorMaskInfoSave()
{
	int i;
	for( i = 0; i < MAX_DIRECT_COUNT; i++ ){
		if( director->cDirectorMask[i] & MASK ){
			maskInfo.ucDirector[i] |= MASK;
		}
	}
}

void directorMaskInfoRestore()
{
	int i;
	for( i = 0; i < MAX_DIRECT_COUNT; i++ ){
		if( maskInfo.ucDirector[i] & MASK ){
			director->cDirectorMask[i] |= MASK;
		}
	}
}

void switchMaskInfoSave()
{
	int i;
	for( i = 0; i < MAX_SWITCH_COUNT; i++ ){
		if( swch->cSwitchMask[i] & MASK ){
			maskInfo.ucSwitch[i] |= MASK;
		}
	}
}

void switchMaskInfoRestore()
{
	int i;
	for( i = 0; i < MAX_SWITCH_COUNT; i++ ){
		if( maskInfo.ucSwitch[i] & MASK ){
			swch->cSwitchMask[i] |= MASK;
		}
	}
}

void directorAllMaskInfoSave(pst_DirectMask pDirect)
{
	int i,j;
	for( i = 0; i < MAX_DIRECT_COUNT; i++ ){
		if( director->cDirectorMask[i] & MASK ){
			maskInfo.ucDirector[i] |= MASK;
		}

		for( j = 0; j < MAX_MONITOR_PORT_COUNT; j++ ){
			if( director->stDIRECT[i].cMonitorPort[j] & MASK ){
				pDirect->ucMonitorPort[j] |= MASK;
			}
		}

		for( j = 0; j < MAX_MIRROR_PORT_COUNT; j++ ){
			if( director->stDIRECT[i].cMirrorPort[j] & MASK ){
				pDirect->ucMirrorPort[j] |= MASK;
			}
		}

		for( j = 0; j < MAX_DIRECT_POWER_COUNT; j++ ){
			if( director->stDIRECT[i].cPower[j] & MASK ){
				pDirect->ucPower[j] |= MASK;
			}
		}
	}
}

void directorAllMaskInfoRestore(pst_DirectMask pDirect)
{
	int i,j;
	for( i = 0; i < MAX_DIRECT_COUNT; i++ ){
		if( maskInfo.ucDirector[i] & MASK ){
			director->cDirectorMask[i] |= MASK;
		}

		for( j = 0; j < MAX_MONITOR_PORT_COUNT; j++ ){
			if( pDirect->ucMonitorPort[j] & MASK ){
				director->stDIRECT[i].cMonitorPort[j] |= MASK;
			}
		}

		for( j = 0; j < MAX_MIRROR_PORT_COUNT; j++ ){
			if( pDirect->ucMirrorPort[j] & MASK ){
				director->stDIRECT[i].cMirrorPort[j] |= MASK;
			}
		}

		for( j = 0; j < MAX_DIRECT_POWER_COUNT; j++ ){
			if( pDirect->ucPower[j] & MASK ){
				director->stDIRECT[i].cPower[j] |= MASK;
			}
		}
	}
}

void switchAllMaskInfoSave(pst_SwitchMask pSwitch)
{
	int i,j;
	for( i = 0; i < MAX_SWITCH_COUNT; i++ ){
		if( swch->cSwitchMask[i] & MASK ){
			maskInfo.ucSwitch[i] |= MASK;
		}

		for( j = 0; j < MAX_SWITCH_PORT_COUNT; j++ ){
			if( swch->stSwitch[i].cSwitchPort[j] & MASK ){
				pSwitch->ucPort[j] |= MASK;
			}
		}

		for( j = 0; j < MAX_SWITCH_CPU_COUNT; j++ ){
			if( swch->stSwitch[i].cSwitchCPUStatus[j] & MASK ){
				pSwitch->ucCPU[j] |= MASK;
			}
		}

		if( swch->stSwitch[i].cSwitchMEMStatus & MASK ){
			pSwitch->ucMEM |= MASK;
		}
	}
}

void switchAllMaskInfoRestore(pst_SwitchMask pSwitch)
{
	int i,j;
	for( i = 0; i < MAX_SWITCH_COUNT; i++ ){
		if( maskInfo.ucSwitch[i] & MASK ){
			swch->cSwitchMask[i] |= MASK;
		}

		for( j = 0; j < MAX_SWITCH_PORT_COUNT; j++ ){
			if( pSwitch->ucPort[j] & MASK ){
				swch->stSwitch[i].cSwitchPort[j] |= MASK;
			}
		}

		for( j = 0; j < MAX_SWITCH_CPU_COUNT; j++ ){
			if( pSwitch->ucCPU[j] & MASK ){
				swch->stSwitch[i].cSwitchCPUStatus[j] |= MASK;
			}
		}

		if( pSwitch->ucMEM & MASK ){
			swch->stSwitch[i].cSwitchMEMStatus |= MASK;
		}
	}
}


int dWriteMaskInfo(int rcv_type)
{
	pst_DirectMask	pDirect;
	pst_SwitchMask	pSwitch;
	int dRet, type = rcv_type;

	pDirect = &maskInfo.stDirector[0];
	pSwitch = &maskInfo.stSwitch[0];

	switch(type){
		case EN_SUBCH: 			subChannelMaskInfoSave(); break;
		case EN_ICH: 			interlockMaskInfoSave(); break;
		case EN_DIRECTOR: 		directorMaskInfoSave(); break;
		case EN_SWITCH: 		switchMaskInfoSave(); break;
		case EN_DIRECTOR_ALL: 	directorAllMaskInfoSave(pDirect); break;
		case EN_SWITCH_ALL: 	switchAllMaskInfoSave(pSwitch); break;
		default: /* EN_CH_ALL */
			subChannelMaskInfoSave();
			interlockMaskInfoSave();
			directorAllMaskInfoSave(pDirect);
			switchAllMaskInfoSave(pSwitch);
	}

	if( (dRet = write_file(FILE_MASK, (char*)&maskInfo, DEF_CHNL_MASK_SIZE,0)) < 0 ){
		log_print(LOGN_CRI,LH"FAILED IN write_file(MASK=%s), dRet=%d"EH,LT,FILE_MASK,dRet,ET);
		return -1;
	}
	log_print(LOGN_CRI,LH"write_mask_info(MASK=%s)",LT,FILE_MASK);
	return 0;
}

int dReadMaskInfo(int rcv_type)
{
	pst_DirectMask pDirect;
	pst_SwitchMask pSwitch;
	int dRet, type = rcv_type;

	pDirect = &maskInfo.stDirector[0];
	pSwitch = &maskInfo.stSwitch[0];

	if( (dRet = read_file(FILE_MASK, (char*)&maskInfo, DEF_CHNL_MASK_SIZE,0)) < 0 ){
		log_print(LOGN_CRI,LH"FAILED IN read_file(MASK=%s), dRet=%d"EH,LT,FILE_MASK,dRet,ET);
		return -1;
	}

	switch(type){
		case EN_SUBCH: 			subChannelMaskInfoRestore(); break;
		case EN_ICH: 			interlockMaskInfoRestore(); break;
		case EN_DIRECTOR: 		directorMaskInfoRestore(); break;
		case EN_SWITCH: 		switchMaskInfoRestore(); break;
		case EN_DIRECTOR_ALL: 	directorAllMaskInfoRestore(pDirect); break;
		case EN_SWITCH_ALL: 	switchAllMaskInfoRestore(pSwitch); break;
		default: /* EN_CH_ALL */
			subChannelMaskInfoRestore();
			interlockMaskInfoRestore();
			directorAllMaskInfoRestore(pDirect);
			switchAllMaskInfoRestore(pSwitch);
	}

	log_print(LOGN_CRI,LH"read_mask_info(MASK=%s)",LT,FILE_MASK);
	return 0;
}
