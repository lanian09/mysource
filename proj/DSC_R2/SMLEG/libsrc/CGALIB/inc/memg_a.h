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
 *     @warning     $type???로 정의된 것들만 사용가능
 *     @ref         memg.h
 *     @todo        Makefile을 만들자
 *
 *     @section     Intro(소개)
 *      - hash header file
 *
 *     @section     Requirement
 *      @li 규칙에 틀린 곳을 찾아주세요.
 *
 **/


/* 필요한 header file include */
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
 * @brief stMEMGNODE : mem안의 node들의 structure.
 *
 * 현 MEM는 doubly linked list로 구성된다. 
 * 앞에는 mem node header가 오게 되고 뒤에 mem node body가 오게 된다.  
 * mem node header + body = mem node 가 되는 것이다.  
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
 *  @note 		 Hdr뒤에 Body가 오게 될 것이며, Body부분을 실제 유저들이 사용하게 될 것이다.
 *
 */
typedef struct _st_memgnodehdr {
	U32	uiID;			/**< MEMG_ID : mem debug를 위한 부분 : free시 확인 */ 
	STIME	TimeSec;	/**< Debug나 garbage collection을 위한 시간 저장 */
	U8	ucIsFree;			/**< Free:0 , Alloc : 1 */
	S8	DebugStr[MEMG_MAX_DEBUG_STR];	/**< debugging을 위해서 사용된 */
} stMEMGNODEHDR;
#define STG_DEF_stMEMGNODEHDR		101		/* Hex( 529630 ) */
#define stMEMGNODEHDR_SIZE sizeof(stMEMGNODEHDR)



/** 
 *  @brief stMEMGINFO : 보통 memg에서 사용하는 memory을 관리하는 structure이다. 
 *
 *
 * @see memg.h 
 *
 *  @note 	사용해야 할 memory에 대한 전체적인 관리.
 */
typedef struct _st_memginfo {
    U32 uiType;		/**<  Main Memory = 1 , Shared Memory = 2  */
    U32 uiShmKey;		/**<  uiType ==  Shared Memory : shared mem Key */
    U32 uiTotMemSize;		/**< 전체 사용가능한 memory (alloc또는 shared mem size */
    U32 uiHeadRoomSize;		/**< 실제로 node들이 놓이기 전에 빈 공간내지는 다른 내용을 사용하기 위한 공간 */
    U32 uiMemNodeHdrSize; 	/**< sizeof(stMEMGNODEHDR) */
    U32 uiMemNodeBodySize; 	/**< Node안의 hdr뒤에 붙는 Body의 Size :\n이 Body의 Size가 실제로 원하는 Size가 될 것임 */
    U32 uiMemNodeAllocedCnt;	/**< 이미 alloc되어진 갯수 */
    U32 uiMemNodeTotCnt;		/**< 전체 노드의 갯수 */
    OFFSET offsetHeadRoom;			/**< HeadRoom의 위치 */
    OFFSET offsetNodeStart;		/**< Node들의 처음 위치 */
    OFFSET offsetFreeList;			/**< Free된 Node의 위치 */
    OFFSET offsetNodeEnd;			/**< Node의 마지막 Offset */
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




/* Define.  MEMBER_CNT(struct안의 member들의수 : flat기준)
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
 *     성능 패키지
 *
 *     Revision 1.1.1.1  2011/01/20 12:18:50  june
 *     DSC CVS RECOVERY
 *
 *     Revision 1.1  2009/06/23 21:16:12  dsc
 *     cgalib/inc init
 *
 *     Revision 1.2  2009/03/03 12:13:39  june
 *     *.h 파일 dos2unix로 포멧 변경
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
