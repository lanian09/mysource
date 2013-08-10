/*******************************************************************************
			DQMS Project

	Author   :
	Section  : ALMD
	SCCS ID  : @(#)fstat_mmc.h	1.1
	Date     :
	Revision History :

	Description :

	Copyright (c) uPRESTO 2011
*******************************************************************************/
#ifndef __FSTAT_MMC_H__
#define __FSTAT_MMC_H__

typedef struct _st_MASKNTAF {
	unsigned char    link[MAX_NTAF_LINK];       /*  4 */
	unsigned char    hwntp[2];
	unsigned char    hwpwr[2];
	unsigned char    hwdisk[2];
	unsigned char    hwfan[6];
	unsigned char    mpsw[MAX_NTAF_SW_BLOCK];   /* 80 */
} st_MASKNTAF, *pst_MASKNTAF;

typedef struct _st_TOT_MASKNTAF {
	st_MASKNTAF  ntaf[32];
} st_TOT_MASKNTAF, *pst_TOT_MASKNTAF;

extern int mask_ntp_alm( mml_msg *mmsg , dbm_msg_t *smsg );

#endif /* __FSTAT_MMC_H__ */
