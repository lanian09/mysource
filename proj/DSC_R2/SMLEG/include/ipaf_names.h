/**********************************************************
                 ABLEX Main-Memory DBMS

   Author   : LEE SANG HO
   Section  : IPAS Project
   SCCS ID  : %W%
   Date     : %G%
   Revision History :
        '01.  7. 29     Initial

   Description:
        MMDB and SLEE

   Copyright (c) ABLEX 2001
***********************************************************/

#ifndef __IPAM_NAMES_HEADER_FILE__
#define __IPAM_NAMES_HEADER_FILE__

#define MAX_BLOCKS				30
#define MAX_BLOCKS_ALL			30
#define IPAF_SW_COM				32	
//#define MAX_MULTI_PROC		16

#define MAX_MP_NUM1				1
#define MAX_MP_NUM2				1


#if 0   // 060714, poopee
#define SEQ_PROC_CHSMD			0
#define SEQ_PROC_ALMD			1	
#define SEQ_PROC_MMCD			2	
#define SEQ_PROC_QMONITOR		3	
#define SEQ_PROC_IPAMTIF 		4	
#define SEQ_PROC_IPAMUIF		5	
#define SEQ_PROC_CAPD			6	
#define SEQ_PROC_ANA			7	
#define SEQ_PROC_CDR			8	
#define SEQ_PROC_MDBMGR			9	
#define SEQ_PROC_ADMIN			10	

/***** MODIFIED BY LEE : 20040303 *****/
#define SEQ_PROC_SESSANA        11
#define SEQ_PROC_MESVC          12
#define SEQ_PROC_KUNSVC         13
#define SEQ_PROC_ADSSVC         14
#define SEQ_PROC_MARSSVC        15

/**** MODIFIED BY SONG JS : 20040326 ****/
#define SEQ_PROC_RDRIF          16

#define SEQ_PROC_MACSSVC        17
#define SEQ_PROC_VODMANA		19
#define SEQ_PROC_VODDANA		20

/**** MODIFIED BY SYHAN : 20060710 ****/
#define SEQ_PROC_WAP1ANA		22
#else
#define SEQ_PROC_IXPC			0
#define SEQ_PROC_SAMD			1
#define SEQ_PROC_MMCR			2
#define SEQ_PROC_STMM			3
#define SEQ_PROC_CAPD			4
#define SEQ_PROC_ANA			5
//#define SEQ_PROC_CDR			6
#define SEQ_PROC_SESSANA0		6	/* SEQ_PROC_TRCDR */
#if 0
// added  by dcham 20110530 for SM connection Ãà¼Ò(5=>1)
#define SEQ_PROC_SESSANA1		7	/* SEQ_PROC_TRCDR */
#define SEQ_PROC_SESSANA2		8	/* SEQ_PROC_TRCDR */
#define SEQ_PROC_SESSANA3		9	/* SEQ_PROC_TRCDR */
#define SEQ_PROC_SESSANA4		10	/* SEQ_PROC_TRCDR */
#endif
//#define SEQ_PROC_WAP1ANA		8
//#define SEQ_PROC_UAWAPANA		9
//#define SEQ_PROC_MESVC			10	/* SEQ_PROC_WAP2ANA */
// yhshin #define SEQ_PROC_KUNSVC			11	/* SEQ_PROC_HTTPANA */
#define SEQ_PROC_RDRCAPD		11

#define SEQ_PROC_VODSANA		12
#define SEQ_PROC_WIPINWANA		13
#define SEQ_PROC_KVMANA 		14
#define SEQ_PROC_PCDR           15
//#define SEQ_PROC_PTOPANA        15
#define SEQ_PROC_CDR2           16
#define SEQ_PROC_VTANA          17
#define SEQ_PROC_OZCDR          18
#define SEQ_PROC_WVANA          19

// 12~21: reserved
#define SEQ_PROC_UDRGEN			22
#define SEQ_PROC_AAAIF			23
#define SEQ_PROC_SDMD			24

// Added by jjj 2006.11.21
#define SEQ_PROC_LOGM			25
#define SEQ_PROC_UAWAPFETCH		26
/* ADD BY YOON 2008/09/17 */
#define SEQ_PROC_RANA			27
#define SEQ_PROC_FBANA			28

#define	SEQ_PROC_MEM 			29
#define	SEQ_PROC_REANA 			30
#define	SEQ_PROC_RDRANA 		31
#define	SEQ_PROC_SMPP			32
#endif

// Added by syhan 20060718 for Temporary * */
#define SEQ_PROC_WAP1TEST		24
/* *************************************** */

/* CHNL INDEX */
#define SEQ_CHNL_IPAMUIF        2
#define SEQ_CHNL_IPAMTIF		3

#endif

  
