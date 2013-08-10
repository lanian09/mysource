#ifndef	__memg_h__
#define	__memg_h__
/**		file  memg.h
 *      - hash header file
 *
 *     Copyright (c) 2006~ by Upresto Inc, Korea
 *     All rights reserved
 *
 *		$Id: memg.h,v 1.3 2011/08/19 06:35:02 dcham Exp $
 * 
 *     @Author      $Author: dcham $
 *     @version     $Revision: 1.3 $
 *     @date        $Date: 2011/08/19 06:35:02 $
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
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netinet/in.h>
#include	<time.h>

#include <commdef.h>
#include <typedef.h>

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
#define STG_DEF_stMEMGNODEHDR		101		/* Hex( 1813f1e0 ) */
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
#define STG_DEF_stMEMGINFO		102		/* Hex( 1813f1e0 ) */
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
extern void memg_reset(stMEMGINFO *pstMEMGINFO, size_t base);




/* Define.  DEF_NUM(type definition number)
*/
#define		stFlat_stMEMGNODEHDR_DEF_NUM				101			/* Hex ( 183a3c60 ) */
#define		stMEMGNODEHDR_DEF_NUM						101			/* Hex ( 183a3c50 ) */
#define		stMEMGINFO_DEF_NUM							102			/* Hex ( 18477520 ) */
#define		stFlat_stMEMGINFO_DEF_NUM					102			/* Hex ( 18477530 ) */




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
 *     $Log: memg.h,v $
 *     Revision 1.3  2011/08/19 06:35:02  dcham
 *     *** empty log message ***
 *
 *     Revision 1.1  2011/08/18 04:58:28  dcham
 *     *** empty log message ***
 *
 *     Revision 1.1  2011/08/03 06:02:38  uamyd
 *     CGA, HASHO, TIMERN library added
 *
 *     Revision 1.2  2011/01/11 04:09:03  uamyd
 *     modified
 *
 *     Revision 1.1.1.1  2010/08/23 01:13:06  uamyd
 *     DQMS With TOTMON, 2nd-import
 *
 *     Revision 1.1  2009/06/10 16:45:50  dqms
 *     *** empty log message ***
 *
 *     Revision 1.1.1.1  2009/05/26 02:13:28  dqms
 *     Init TAF_RPPI
 *
 *     Revision 1.2  2008/11/17 09:00:04  dark264sh
 *     64bits 작업
 *
 *     Revision 1.1.1.1  2008/06/09 08:17:19  jsyoon
 *     WATAS3 PROJECT START
 *
 *     Revision 1.1  2007/08/21 12:22:38  dark264sh
 *     no message
 *
 *     */
#endif	/* __memg_h__*/
