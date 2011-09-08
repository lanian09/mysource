#ifndef	__memg_a_h__
#define	__memg_a_h__
/**		file  memg.h
 *      - hash header file
 *
 *     Copyright (c) 2006~ by Upresto Inc, Korea
 *     All rights reserved
 *
 *		$Id: memg_a.h,v 1.1.1.1 2011/04/19 14:13:41 june Exp $
 * 
 *     @Author      $Author: june $
 *     @version     $Revision: 1.1.1.1 $
 *     @date        $Date: 2011/04/19 14:13:41 $
 *     @warning     $type???�� ���ǵ� �͵鸸 ��밡��
 *     @ref         memg.h
 *     @todo        Makefile�� ������
 *
 *     @section     Intro(�Ұ�)
 *      - hash header file
 *
 *     @section     Requirement
 *      @li ��Ģ�� Ʋ�� ���� ã���ּ���.
 *
 **/


/* �ʿ��� header file include */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <time.h>
#include <typeaqua.h>
#include <stg_def.h>


/** @mainpage Memory Management 
malloc(total size);
\dot
 digraph example {
 fontname=Helvetica;
 label="Memory Architecture";
	rankdir=LR; 	
	node [shape=record,fontname=Helvetica,width=.1,height=.1]; 	
	node0 [label = "<f0> stMEMGINFO | <f1> header\ room | <f2> memg_node_hdr | <f3> memg_node_data | ... | memg_node_hdr | memg_node_data",height = 2.5];
}
\enddot
 */

#define MEMG_OFFSET(INFO,ptr)  ((OFFSET) (((U8 *) (ptr)) - ((U8 *) INFO)) )
#define MEMG_PTR(INFO,offset)  ((U8 *) (((OFFSET) (offset)) + ((OFFSET) INFO)) ) 

#define 	MEMG_MAIN_MEM		1
#define 	MEMG_SHARED_MEM		2

#define MEMG_MAX_DEBUG_STR	11
#define MEMG_ID				0x10101010
#define MEMG_FREE			0
#define MEMG_ALLOCED		1
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
typedef struct _st_memgnodehdr {
	U32	uiID;			/**< MEMG_ID : mem debug�� ���� �κ� : free�� Ȯ�� */ 
	STIME	TimeSec;	/**< Debug�� garbage collection�� ���� �ð� ���� */
	U8	ucIsFree;			/**< Free:0 , Alloc : 1 */
	S8	DebugStr[MEMG_MAX_DEBUG_STR];	/**< debugging�� ���ؼ� ���� */
} stMEMGNODEHDR;
#define STG_DEF_stMEMGNODEHDR		101		/* Hex( 529630 ) */
#define stMEMGNODEHDR_SIZE sizeof(stMEMGNODEHDR)



/** 
 *  @brief stMEMGINFO : ���� memg���� ����ϴ� memory�� �����ϴ� structure�̴�. 
 *
 *
 * @see memg.h 
 *
 *  @note 	����ؾ� �� memory�� ���� ��ü���� ����.
 */
typedef struct _st_memginfo {
    U32 uiType;		/**<  Main Memory = 1 , Shared Memory = 2  */
    U32 uiShmKey;		/**<  uiType ==  Shared Memory : shared mem Key */
    U32 uiTotMemSize;		/**< ��ü ��밡���� memory (alloc�Ǵ� shared mem size */
    U32 uiHeadRoomSize;		/**< ������ node���� ���̱� ���� �� ���������� �ٸ� ������ ����ϱ� ���� ���� */
    U32 uiMemNodeHdrSize; 	/**< sizeof(stMEMGNODEHDR) */
    U32 uiMemNodeBodySize; 	/**< Node���� hdr�ڿ� �ٴ� Body�� Size :\n�� Body�� Size�� ������ ���ϴ� Size�� �� ���� */
    U32 uiMemNodeAllocedCnt;	/**< �̹� alloc�Ǿ��� ���� */
    U32 uiMemNodeTotCnt;		/**< ��ü ����� ���� */
    OFFSET offsetHeadRoom;			/**< HeadRoom�� ��ġ */
    OFFSET offsetNodeStart;		/**< Node���� ó�� ��ġ */
    OFFSET offsetFreeList;			/**< Free�� Node�� ��ġ */
    OFFSET offsetNodeEnd;			/**< Node�� ������ Offset */
} stMEMGINFO;
#define STG_DEF_stMEMGINFO		102		/* Hex( 529630 ) */
#define stMEMGINFO_SIZE sizeof(stMEMGINFO)



/**
 * memg project : External Functions.
 */
extern int memg_dump_DebugString(char *debug_str,char *s,int len);
extern stMEMGINFO * memg_init(U32 uiType,U32 uiShmKey,U32 uiHeadRoomSize, U32 uiMemNodeBodySize,U32 uiMemNodeTotCnt);
extern U8 * memg_alloc(stMEMGINFO *pstMEMGINFO , U32 uiSize, S8 *pDbgPtr);
extern S32 memg_free(stMEMGINFO *pstMEMGINFO , U8 *pFree);
extern void memg_print_info(S8 *pcPrtPrefixStr,stMEMGINFO *pstMEMGINFO);
extern void memg_print_node(S8 *pcPrtPrefixStr,stMEMGNODEHDR *pstMEMGNODEHDR);
extern void memg_print_all(S8 *pcPrtPrefixStr,stMEMGINFO *pstMEMGINFO);
extern void memg_draw_all(S8 *filename,S8 *labelname,stMEMGINFO *pstMEMGINFO);




/* Define.  DEF_NUM(type definition number)
*/
#define		stFlat_stMEMGNODEHDR_DEF_NUM				101			/* Hex ( 78ffd0 ) */
#define		stMEMGNODEHDR_DEF_NUM						101			/* Hex ( 78ffc0 ) */
#define		stMEMGINFO_DEF_NUM							102			/* Hex ( 85da80 ) */
#define		stFlat_stMEMGINFO_DEF_NUM					102			/* Hex ( 85da90 ) */




/* Define.  MEMBER_CNT(struct���� member���Ǽ� : flat����)
*/
#define		Short_stMEMGINFO_MEMBER_CNT					12
#define		Short_stMEMGNODEHDR_MEMBER_CNT				4
#define		stFlat_Short_stMEMGINFO_MEMBER_CNT			12
#define		stFlat_Short_stMEMGNODEHDR_MEMBER_CNT		4
#define		stFlat_stMEMGINFO_MEMBER_CNT				12
#define		stFlat_stMEMGNODEHDR_MEMBER_CNT				4
#define		stMEMGINFO_MEMBER_CNT						12
#define		stMEMGNODEHDR_MEMBER_CNT					4




/* Extern Function Define.
*/
extern void stMEMGINFO_CILOG(FILE *fp, stMEMGINFO *pthis);
extern void stMEMGINFO_Dec(stMEMGINFO *pstTo , stMEMGINFO *pstFrom);
extern void stMEMGINFO_Enc(stMEMGINFO *pstTo , stMEMGINFO *pstFrom);
extern void stMEMGINFO_Prt(S8 *pcPrtPrefixStr, stMEMGINFO *pthis);
extern void stMEMGNODEHDR_CILOG(FILE *fp, stMEMGNODEHDR *pthis);
extern void stMEMGNODEHDR_Dec(stMEMGNODEHDR *pstTo , stMEMGNODEHDR *pstFrom);
extern void stMEMGNODEHDR_Enc(stMEMGNODEHDR *pstTo , stMEMGNODEHDR *pstFrom);
extern void stMEMGNODEHDR_Prt(S8 *pcPrtPrefixStr, stMEMGNODEHDR *pthis);

#pragma pack()

/** file : memg.h
 *     $Log: memg_a.h,v $
 *     Revision 1.1.1.1  2011/04/19 14:13:41  june
 *     ���� ��Ű��
 *
 *     Revision 1.1.1.1  2011/01/20 12:18:50  june
 *     DSC CVS RECOVERY
 *
 *     Revision 1.1  2009/06/23 21:16:12  dsc
 *     cgalib/inc init
 *
 *     Revision 1.2  2009/03/03 12:13:39  june
 *     *.h ���� dos2unix�� ���� ����
 *
 *     Revision 1.1.1.1  2009/02/17 13:35:34  june
 *     client_server simulator start
 *
 *     Revision 1.1  2008/12/12 00:06:28  yhshin
 *     cga64
 *
 *     Revision 1.1.1.1  2008/06/09 08:17:19  jsyoon
 *     WATAS3 PROJECT START
 *
 *     Revision 1.1  2007/08/21 12:22:38  dark264sh
 *     no message
 *
 *     */
#endif	/* __memg_h__*/
