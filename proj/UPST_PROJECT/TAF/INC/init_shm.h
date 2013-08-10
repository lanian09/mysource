/*******************************************************************************
			DQMS Project

	Author   : joonhk
	Section  : DQMS Project
	SCCS ID  : @(#)init_shm.h	1.1
	Date     : 01/07/03
	Revision History :
		'03.  1. 7  Initial
		'04.  4. 6  Update By LSH For SHM ERROR Definition

	Description :

	Copyright (c) uPRESTO 2005
*******************************************************************************/
#ifndef __INIT_SHM_H__
#define __INIT_SHM_H__

/**B.1*  Definition of New Constants **********************/

#define SHM_ERROR1          -1      /* Shared Memory shmget() Error & EXIST shmget() Error -6 */
#define SHM_ERROR2          -2      /* Shared Memory shmat() Error & EXIST shmat() Error -7 */
#define SHM_ERROR3          -3      /* Shared Memory shmdt() Error used REMOVE SHM Funcation */
#define SHM_ERROR4          -4      /* Shared Memory shmctl() Error used REMOVE SHM Funcation */
#define SHM_ERROR_ETC       -5      /* Shared Memory ETC Error used REMOVE SHM Funcation */

#define ERR_SHM_FIDB        -100    /* FIDB SHM ERROR */
#define ERR_SHM_KEEP        -200    /* KEEPALIVE SHM ERROR */
#define ERR_SHM_CAPBUF      -50     /* CAPBUF SHM ERROR with CAP ERROR */
#define ERR_SHM_CAP         -300    /* CAP SHM ERROR */
#define ERR_SHM_GEN         -400    /* GENINFO SHM ERROR */
#define ERR_SHM_OBJ         -500    /* MMDB OBJ SHM ERROR */
#define ERR_SHM_SESS        -600    /* MMDB SESS SHM ERROR */
#define ERR_SHM_CDR         -700    /* MMDB CDR SHM ERROR */
#define ERR_SHM_DESTIP      -800    /* MMDB DESTIP SHM ERROR */
#define ERR_SHM_DESTPORT    -900    /* MMDB DESTPORT SHM ERROR */

#define ERR_SHM_MESTAT		-1000	/* ME STAT SHM */
#define ERR_SHM_KUNSTAT		-1100	/* KUN STAT SHM */
#define ERR_SHM_ADSSTAT		-1200	/* ADS STAT SHM */
#define ERR_SHM_MARSSTAT	-1300	/* MARS STAT SHM */
#define ERR_SHM_SESSSTAT	-1400	/* SESS STAT SHM */

#define ERR_SHM_VERSION		-1500	/* VERSION */

#define ERR_SHM_MESVC		-1600	/* MESVC SHM */		/*** added by SJS ***/
#define ERR_SHM_KUNSVC		-1700	/* KUNSVC SHM */
#define ERR_SHM_ADSSVC		-1800	/* ADSSVC SHM */
#define ERR_SHM_MARSSVC		-1900	/* MARSSVC SHM */
#define ERR_SHM_SESSANA		-2000	/* SESSANA SHM */

#define ERR_SHM_RDRSEQ		-2100	/* RDR SEQ SHM */

#define ERR_SHM_MACSSTAT	-2200	/* MACSSTAT SHM */
#define ERR_SHM_MACSSVC		-2300	/* NACSSVC	SHM */
#define ERR_SHM_VODANA		-2400	/* VODANA	SHM */
#define ERR_SHM_VODMANA		-2500	/* VODMANA 	SHM */
#define ERR_SHM_VODDANA		-2600	/* VODDANA 	SHM */
#define ERR_SHM_WICGSSTAT	-2700	/* WICGSSTAT	SHM */
#define ERR_SHM_WICGSSVC	-2800	/* WICGSSVC	SHM */
#define ERR_SHM_VODUDP		-2900	/* VODUDP	SHM */

#define ERR_SHM_VODSSTAT	-3000
#define ERR_SHM_VODMSTAT	-3100
#define ERR_SHM_VODDSTAT	-3200

#endif	/*	__INIT_SHM_H__	*/
