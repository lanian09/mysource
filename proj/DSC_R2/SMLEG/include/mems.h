#ifndef	__MEMS_H__
#define	__MEMS_H__
/**
 *		- hash header file
 *
 *		Copyright (c) 2006~ by Upresto Inc, Korea
 *		All rights reserved
 *
 *		$Id: mems.h,v 1.4 2011/05/28 09:16:00 jjinri Exp $
 * 
 *		@Author      $Author: jjinri $
 *		@version     $Revision: 1.4 $
 *		@date        $Date: 2011/05/28 09:16:00 $
 *		@ref         mems.h
 *		@todo        Makefile�� ������
 *
 *		@section     Intro(�Ұ�)
 *		- hash header file
 *
 *		@section     Requirement
 *		@li ��Ģ�� Ʋ�� ���� ã���ּ���.
 *
 **/



/* �ʿ��� header file include */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <time.h>
#include <clisto.h>


/** @mainpage Memory Management 
malloc(total size);
\dot
 digraph example {
 fontname=Helvetica;
 label="Memory Architecture";
	rankdir=LR; 	
	node [shape=record,fontname=Helvetica,width=.1,height=.1]; 	
	node0 [label = "<f0> stMEMSINFO | <f1> header\ room | <f2> memg_node_hdr | <f3> memg_node_data | ... | memg_node_hdr | memg_node_data",height = 2.5];
}
\enddot
 */

#define MEMS_OFFSET(INFO,ptr)  ((U32) (((U8 *) (ptr)) - ((U8 *) INFO)) )
#define MEMS_PTR(INFO,offset)  ((U8 *) (((S32) (offset)) + ((S32) INFO)) ) 

#define	MEMS_SHM_SIZE		524288000u		// 512M
//#define	MEMS_SHM_SIZE			200000000		// 512M
//#define	MEMS_SHM_SIZE		1610612736		

#define 	MEMS_MAIN_MEM		1
#define 	MEMS_SHARED_MEM		2

#define MEMS_MAX_DEBUG_STR	11
#define MEMS_ID				0x10101010

#define MEMS_FREE			0
#define MEMS_ALLOCED		1

#define MEMS_SEMA_ON		1
#define MEMS_SEMA_OFF		0
/** 
 * @brief stMEMGNODE : mem���� node���� structure.
 *
 * �� MEM�� doubly linked list�� �����ȴ�. 
 * �տ��� mem node header�� ���� �ǰ� �ڿ� mem node body�� ���� �ȴ�.  
 * mem node header + body = mem node �� �Ǵ� ���̴�.  
\dot
digraph G {
	fontname=Helvetica;
	label="Memory Node Architecture";
	rankdir = LR;
	node [shape=record,style=filled,color=lightgray,fontname=Helvetica];
	b [label = "<f0> Node\ Hdr2 | <f1> Node\ Body2"];
	a [label = "<f0> Node\ Hdr1 | <f1> Node\ Body1"];
}
\enddot
 *
 * @see memg.h 
 *
 *  @note 		 Hdr�ڿ� Body�� ���� �� ���̸�, Body�κ��� ���� �������� ����ϰ� �� ���̴�.
 *
 */
typedef struct _st_memsnodehdr {
	U32	uiID;			/**< MEMS_ID : mem debug�� ���� �κ� : free�� Ȯ�� */ 
	U32     uiZoneID;
	STIME	TimeSec;	/**< Debug�� garbage collection�� ���� �ð� ���� */
/**< <TAG_DEFINE:FREE_ALLOCED>
*	 MEMS_FREE			0
*	 MEMS_ALLOCED		1
*/
	U8 	ucIsFree;			/**< Free:0 , Alloc : 1 */
	S8	DebugStr[MEMS_MAX_DEBUG_STR];	/**< debugging�� ���ؼ� ���� CREATE */
	S8	DelStr[MEMS_MAX_DEBUG_STR];	/**< debugging�� ���ؼ� ���� DELETE */
} stMEMSNODEHDR;
#define STG_DEF_stMEMSNODEHDR		101		/* Hex( 0x65 ) */
#define stMEMSNODEHDR_SIZE sizeof(stMEMSNODEHDR)



/** 
 *  @brief stMEMSINFO : ���� memg���� ����ϴ� memory�� �����ϴ� structure�̴�. 
 *
 *
 * @see memg.h 
 *
 *  @note 	����ؾ� �� memory�� ���� ��ü���� ����.
 */
#define MAX_MEMSZONE_CNT    10
#define MAX_SEQ_PROC_NUM    1000

typedef struct _st_memszone {
	/**< <TAG_DEFINE:SEMA>
	 *    MEMS_SEMA_ON       1
	 *    MEMS_SEMA_OFF      0
	 */
	U32     uiSemFlag;  /**< Semaphore ��c��e ���Ϩ�I */
	U32     uiSemKey;           /**< Semaphore Key �ƨ� */
	S32     iSemID;             /**< Semaphore ID �ƨ� */
	U32     uiZoneID;
	U32     uiMemNodeBodySize;  /**< Node��EAC hdr��U���� ��U��A BodyAC Size :\nAI BodyAC Size�Ƣ� ��CA|��I ����CI��A Size�Ƣ� ��E ��IAO */
	U32     uiMemNodeAllocedCnt;    /**< AI��I alloc��C��iA�� �Ʃ���o */
	U32     uiMemNodeTotCnt;        /**< AuA�� ��e��aAC �Ʃ���o */
	OFFSET  offsetNodeStart;        /**< Node��eAC A��A�� A��A�� */
	OFFSET  offsetFreeList;         /**< Free��E NodeAC A��A�� */
	OFFSET  offsetNodeEnd;          /**< NodeAC ����Ao���� Offset */
	U64     createCnt;
	U64     delCnt;
} st_MEMSZONE;
#define STG_DEF_st_MEMSZONE     102     /* Hex( 531490 ) */
#define st_MEMSZONE_SIZE sizeof(st_MEMSZONE)

typedef struct _st_memsinfo {
	/**< <TAG_DEFINE:MEM>
	 *	 	MEMS_MAIN_MEM		1
	 *	 	MEMS_SHARED_MEM		2
	 */
	U32 	uiType;		/**<  Main Memory = 1 , Shared Memory = 2  */
	U32 	uiShmKey;		/**<  uiType ==  Shared Memory : shared mem Key */
	OFFSET	uiTotMemSize;		/**< ��ü ��밡���� memory (alloc�Ǵ� shared mem size */
	U32		uiHeadRoomSize;		/**< ������ node���� ���̱� ���� �� ���������� �ٸ� ������ ����ϱ� ���� ���� */
	U32		uiMemNodeHdrSize;	/** < sizeof(stMEMSNODEHDR) */
	OFFSET	offsetHeadRoom;		/** < HeadRoom�� ��ġ */
	U32             uiZoneCnt;
	st_MEMSZONE     stMEMSZONE[MAX_MEMSZONE_CNT];
	U32             uiMatrixZoneID[MAX_SEQ_PROC_NUM];
} stMEMSINFO;
#define STG_DEF_stMEMSINFO		103		/* Hex( 0x66 ) */
#define stMEMSINFO_SIZE sizeof(stMEMSINFO)


typedef struct _st_memszone_conf {
	U32                 uiZoneID;
	/**< <TAG_DEFINE:SEMA>
	 *    MEMS_SEMA_ON       1
	 *    MEMS_SEMA_OFF      0
	 */
	U32     uiSemFlag;
	U32                 uiSemKey;
	U32                 uiMemNodeBodySize;
	U32                 uiMemNodeTotCnt;
} st_MEMSZONECONF;
#define STG_DEF_st_MEMSZONECONF     104     /* Hex( 531490 ) */
#define st_MEMSZONECONF_SIZE sizeof(st_MEMSZONECONF)


typedef struct _st_memsconf {
	/**< <TAG_DEFINE:MEM>
	 *       MEMS_MAIN_MEM       1
	 *       MEMS_SHARED_MEM     2
	 */
	U32     uiType;
	U32                 uiShmKey;
	U32                 uiHeadRoomSize;
	U32                 uiZoneCnt;
	st_MEMSZONECONF     stMEMSZONECONF[MAX_MEMSZONE_CNT];
	U32             uiMatrixZoneID[MAX_SEQ_PROC_NUM];
} st_MEMSCONF;
#define STG_DEF_st_MEMSCONF     105     /* Hex( 531490 ) */
#define st_MEMSCONF_SIZE sizeof(st_MEMSCONF)


/**
 * memg project : External Functions.
 */
extern int mems_dump_DebugString(char *debug_str,char *s,int len);
//extern stMEMSINFO *mems_init(U32 uiType,U32 uiShmKey, U32 uiSemFlag, U32 uiSemKey, U32 uiHeadRoomSize, U32 uiMemNodeBodySize,U32 uiMemNodeTotCnt);
extern stMEMSINFO *mems_init(st_MEMSCONF *pstMEMSCONF);
//extern U8 *mems_alloc(stMEMSINFO *pstMEMSINFO , U32 uiSize, U8 *pDbgPtr);
extern U8 *mems_alloc(stMEMSINFO *pstMEMSINFO , U32 uiSize, U32 uiZoneID, U8 *pDbgPtr);
extern S32 mems_free(stMEMSINFO *pstMEMSINFO , U8 *pFree, U8 *pDbgPtr);
extern void mems_print_info(S8 *pcPrtPrefixStr,stMEMSINFO *pstMEMSINFO);
extern void mems_print_node(S8 *pcPrtPrefixStr,stMEMSNODEHDR *pstMEMSNODEHDR);
extern void mems_print_all(S8 *pcPrtPrefixStr,stMEMSINFO *pstMEMSINFO);
extern void mems_draw_all(S8 *filename,S8 *labelname,stMEMSINFO *pstMEMSINFO);
extern void mems_garbage_collector(S8 *pcPrtPrefixStr,stMEMSINFO *pstMEMSINFO,int timegap, void (*print_func)(stMEMSINFO *pmem, stMEMSNODEHDR *pmemhdr));
extern void mems_view(S8 *pcPrtPrefixStr, stMEMSINFO *pstMEMSINFO, int timegap, void (*print_func)(stMEMSINFO *pmem, stMEMSNODEHDR * pmemhdr));
//extern S32 mems_alloced_cnt(stMEMSINFO *pstMEMSINFO);
extern S32 mems_alloced_cnt(stMEMSINFO *pstMEMSINFO, U32 uiZoneID);

/* zone add func */
extern S32 mems_sem_init(key_t semkey, U32 flag);
extern S32 P(S32 semid, U32 flag);
extern S32 V(S32 semid, U32 flag);



/* Define.  DEF_NUM(type definition number)
*/
//#define		stFlat_stMEMSNODEHDR_DEF_NUM				101			/* Hex ( 0x65 ) */
//#define		stMEMSNODEHDR_DEF_NUM						101			/* Hex ( 0x65 ) */
//#define		stFlat_stMEMSINFO_DEF_NUM					102			/* Hex ( 0x66 ) */
//#define		stMEMSINFO_DEF_NUM							102			/* Hex ( 0x66 ) */

#define     st_MEMSZONECONF_DEF_NUM                     104         /* Hex ( 796890 ) */
#define     st_MEMSCONF_DEF_NUM                         105         /* Hex ( 797420 ) */
#define     stFlat_st_MEMSCONF_DEF_NUM                  105         /* Hex ( 797430 ) */
#define     stMEMSNODEHDR_DEF_NUM                       101         /* Hex ( 793b00 ) */
#define     stFlat_stMEMSINFO_DEF_NUM                   103         /* Hex ( 795cd0 ) */
#define     stMEMSINFO_DEF_NUM                          103         /* Hex ( 795cc0 ) */
#define     stFlat_stMEMSNODEHDR_DEF_NUM                101         /* Hex ( 793b10 ) */
#define     stFlat_st_MEMSZONE_DEF_NUM                  102         /* Hex ( 794e80 ) */
#define     st_MEMSZONE_DEF_NUM                         102         /* Hex ( 794e70 ) */
#define     stFlat_st_MEMSZONECONF_DEF_NUM              104         /* Hex ( 7968a0 ) */



/* Define.  MEMBER_CNT(struct���� member���Ǽ� : flat����)
*/
//#define		Short_stMEMSINFO_MEMBER_CNT					17
//#define		Short_stMEMSNODEHDR_MEMBER_CNT				5
//#define		stFlat_Short_stMEMSINFO_MEMBER_CNT			17
//#define		stFlat_Short_stMEMSNODEHDR_MEMBER_CNT		5
//#define		stFlat_stMEMSINFO_MEMBER_CNT				17
//#define		stFlat_stMEMSNODEHDR_MEMBER_CNT				5
//#define		stMEMSINFO_MEMBER_CNT						17
//#define		stMEMSNODEHDR_MEMBER_CNT					5

#define     Short_stMEMSINFO_MEMBER_CNT                 8
#define     Short_stMEMSNODEHDR_MEMBER_CNT              6
#define     Short_st_MEMSCONF_MEMBER_CNT                5
#define     Short_st_MEMSZONE_MEMBER_CNT                12
#define     Short_st_MEMSZONECONF_MEMBER_CNT            5
#define     stFlat_Short_stMEMSINFO_MEMBER_CNT          8
#define     stFlat_Short_stMEMSNODEHDR_MEMBER_CNT       6
#define     stFlat_Short_st_MEMSCONF_MEMBER_CNT         5
#define     stFlat_Short_st_MEMSZONE_MEMBER_CNT         12
#define     stFlat_Short_st_MEMSZONECONF_MEMBER_CNT     5
#define     stFlat_stMEMSINFO_MEMBER_CNT                8
#define     stFlat_stMEMSNODEHDR_MEMBER_CNT             6
#define     stFlat_st_MEMSCONF_MEMBER_CNT               5
#define     stFlat_st_MEMSZONE_MEMBER_CNT               12
#define     stFlat_st_MEMSZONECONF_MEMBER_CNT           5
#define     stMEMSINFO_MEMBER_CNT                       8
#define     stMEMSNODEHDR_MEMBER_CNT                    6
#define     st_MEMSCONF_MEMBER_CNT                      5
#define     st_MEMSZONE_MEMBER_CNT                      12
#define     st_MEMSZONECONF_MEMBER_CNT                  5


/* Extern Function Define.
*/
extern void stMEMSINFO_CILOG(FILE *fp, stMEMSINFO *pthis);
extern void stMEMSINFO_Dec(stMEMSINFO *pstTo , stMEMSINFO *pstFrom);
extern void stMEMSINFO_Enc(stMEMSINFO *pstTo , stMEMSINFO *pstFrom);
extern void stMEMSINFO_Prt(S8 *pcPrtPrefixStr, stMEMSINFO *pthis);
extern void stMEMSNODEHDR_CILOG(FILE *fp, stMEMSNODEHDR *pthis);
extern void stMEMSNODEHDR_Dec(stMEMSNODEHDR *pstTo , stMEMSNODEHDR *pstFrom);
extern void stMEMSNODEHDR_Enc(stMEMSNODEHDR *pstTo , stMEMSNODEHDR *pstFrom);
extern void stMEMSNODEHDR_Prt(S8 *pcPrtPrefixStr, stMEMSNODEHDR *pthis);
extern void st_MEMSCONF_CILOG(FILE *fp, st_MEMSCONF *pthis);
extern void st_MEMSCONF_Dec(st_MEMSCONF *pstTo , st_MEMSCONF *pstFrom);
extern void st_MEMSCONF_Enc(st_MEMSCONF *pstTo , st_MEMSCONF *pstFrom);
extern void st_MEMSCONF_Prt(S8 *pcPrtPrefixStr, st_MEMSCONF *pthis);
extern void st_MEMSZONECONF_CILOG(FILE *fp, st_MEMSZONECONF *pthis);
extern void st_MEMSZONECONF_Dec(st_MEMSZONECONF *pstTo , st_MEMSZONECONF *pstFrom);
extern void st_MEMSZONECONF_Enc(st_MEMSZONECONF *pstTo , st_MEMSZONECONF *pstFrom);
extern void st_MEMSZONECONF_Prt(S8 *pcPrtPrefixStr, st_MEMSZONECONF *pthis);
extern void st_MEMSZONE_CILOG(FILE *fp, st_MEMSZONE *pthis);
extern void st_MEMSZONE_Dec(st_MEMSZONE *pstTo , st_MEMSZONE *pstFrom);
extern void st_MEMSZONE_Enc(st_MEMSZONE *pstTo , st_MEMSZONE *pstFrom);
extern void st_MEMSZONE_Prt(S8 *pcPrtPrefixStr, st_MEMSZONE *pthis);


/**
 *	$Log: mems.h,v $
 *	Revision 1.4  2011/05/28 09:16:00  jjinri
 *	debug FPRINTF ����
 *	
 *	Revision 1.3  2011/04/29 07:11:29  jjinri
 *	tlv IPTCP, SUBS_INFO
 *	
 *	Revision 1.2  2011/04/26 09:08:14  jjinri
 *	nifo_zone
 *	
 *	Revision 1.5  2011/04/01 05:15:34  jjinri
 *	*** empty log message ***
 *	
 *	Revision 1.4  2011/03/28 06:59:01  jjinri
 *	*** empty log message ***
 *	
 *	Revision 1.3  2011/03/28 06:35:01  jjinri
 *	*** empty log message ***
 *	
 *	Revision 1.6  2011/02/22 14:06:55  swpark
 *	Hsort zone add, mem_size modify
 *	
 *	Revision 1.5  2011/02/16 11:29:15  jjinri
 *	SHM SIZE NEW
 *	
 *	Revision 1.4  2011/02/15 00:19:47  jjinri
 *	_NEW_TAF
 *	
 *	Revision 1.3  2011/02/08 00:15:15  jjinri
 *	SESS_STAT
 *	
 *	Revision 1.2  2011/01/20 02:31:16  jjinri
 *	FILTERING ��� ����. PROTO, S_MNG, RADIUS
 *	
 *	Revision 1.1.1.1  2011/01/11 03:32:00  uamyd
 *	64bit WNTAM, WGTAM added, 1st source after cvs recovery
 *	
 *	Revision 1.4  2010/12/29 06:57:43  swpark
 *	zone matrix add
 *	
 *	Revision 1.3  2010/12/23 07:59:55  swpark
 *	MEMS_SHM_SIZE: 4GB-1
 *	
 *	Revision 1.2  2010/12/23 01:31:06  swpark
 *	nifo zone add
 *	
 *	Revision 1.1.1.1  2010/11/30 05:22:39  uamyd
 *	64bit WNTAM, WGTAM add starting
 *	
 *	Revision 1.1.1.1  2009/11/24 06:44:25  pkg
 *	WNTAS 64bits
 *	
 *	Revision 1.2  2008/09/29 04:50:09  dark264sh
 *	*** empty log message ***
 *	
 *	Revision 1.1  2008/09/29 02:23:15  dark264sh
 *	multi tcp ó��
 *	
 *	Revision 1.2  2008/03/24 02:28:49  dark264sh
 *	nifo mems debug �߰�
 *	
 *	Revision 1.1  2008/01/11 12:09:01  pkg
 *	import two-step by uamyd
 *	
 **/
#endif
