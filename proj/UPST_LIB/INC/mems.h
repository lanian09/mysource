#ifndef	__mems_h__
#define	__mems_h__
/**		file  mems.h
 *      - hash header file
 *
 *     Copyright (c) 2006~ by Upresto Inc, Korea
 *     All rights reserved
 *
 *		$Id: mems.h,v 1.3 2011/08/19 06:35:02 dcham Exp $
 * 
 *     @Author      $Author: dcham $
 *     @version     $Revision: 1.3 $
 *     @date        $Date: 2011/08/19 06:35:02 $
 *     @warning     $type???로 정의된 것들만 사용가능
 *     @ref         mems.h
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

#define	MEMS_SHM_SIZE	1*1024*1024*1024


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

#define 	MEMS_MAIN_MEM		1
#define 	MEMS_SHARED_MEM		2

#define MEMS_MAX_DEBUG_STR	11
#define MEMS_ID				0x10101010

#define MEMS_FREE			0
#define MEMS_ALLOCED		1

#define MEMS_SEMA_ON		1
#define MEMS_SEMA_OFF		0
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
typedef struct _st_memsnodehdr {
	U32		uiID;			/**< MEMS_ID : mem debug를 위한 부분 : free시 확인 */ 
	U32		uiZoneID;
	STIME	TimeSec;	/**< Debug나 garbage collection을 위한 시간 저장 */
/**< <TAG_DEFINE:FREE_ALLOCED>
*	 MEMS_FREE			0
*	 MEMS_ALLOCED		1
*/
	U8 	ucIsFree;			/**< Free:0 , Alloc : 1 */
	S8		DebugStr[MEMS_MAX_DEBUG_STR];	/**< debugging을 위해서 사용된 CREATE */
	S8		DelStr[MEMS_MAX_DEBUG_STR];	/**< debugging을 위해서 사용된 DELETE */
} stMEMSNODEHDR;
#define STG_DEF_stMEMSNODEHDR		101		/* Hex( 531490 ) */
#define stMEMSNODEHDR_SIZE sizeof(stMEMSNODEHDR)



/** 
 *  @brief stMEMSINFO : 보통 memg에서 사용하는 memory을 관리하는 structure이다. 
 *
 *
 * @see memg.h 
 *
 *  @note 	사용해야 할 memory에 대한 전체적인 관리.
 */
#define MAX_MEMSZONE_CNT	10
#define	MAX_SEQ_PROC_NUM    1000

typedef struct _st_memszone {
/**< <TAG_DEFINE:SEMA>
*	 MEMS_SEMA_ON		1
*	 MEMS_SEMA_OFF		0
*/
	U32 	uiSemFlag;	/**< Semaphore 사용 여부 */
	U32		uiSemKey;			/**< Semaphore Key 값 */
	S32		iSemID;				/**< Semaphore ID 값 */ 
	U32		uiZoneID;
    U32		uiMemNodeBodySize; 	/**< Node안의 hdr뒤에 붙는 Body의 Size :\n이 Body의 Size가 실제로 원하는 Size가 될 것임 */
    U32		uiMemNodeAllocedCnt;	/**< 이미 alloc되어진 갯수 */
    U32		uiMemNodeTotCnt;		/**< 전체 노드의 갯수 */
    OFFSET  offsetNodeStart;		/**< Node들의 처음 위치 */
    OFFSET  offsetFreeList;			/**< Free된 Node의 위치 */
    OFFSET  offsetNodeEnd;			/**< Node의 마지막 Offset */
	U64		createCnt;
	U64		delCnt;
} st_MEMSZONE;
#define STG_DEF_st_MEMSZONE		102		/* Hex( 531490 ) */
#define st_MEMSZONE_SIZE sizeof(st_MEMSZONE)


typedef struct _st_memsinfo {
/**< <TAG_DEFINE:MEM>
*	 	MEMS_MAIN_MEM		1
*	 	MEMS_SHARED_MEM		2
*/
	U32 	uiType;		/**<  Main Memory = 1 , Shared Memory = 2  */
    U32 			uiShmKey;		/**<  uiType ==  Shared Memory : shared mem Key */
    OFFSET			uiTotMemSize;		/**< 전체 사용가능한 memory (alloc또는 shared mem size */
    U32				uiHeadRoomSize;		/**< 실제로 node들이 놓이기 전에 빈 공간내지는 다른 내용을 사용하기 위한 공간 */
    U32				uiMemNodeHdrSize; 	/**< sizeof(stMEMSNODEHDR) */
    OFFSET  		offsetHeadRoom;			/**< HeadRoom의 위치 */
	U32				uiZoneCnt;
	st_MEMSZONE		stMEMSZONE[MAX_MEMSZONE_CNT];
	U32				uiMatrixZoneID[MAX_SEQ_PROC_NUM];
} stMEMSINFO;
#define STG_DEF_stMEMSINFO		103		/* Hex( 531490 ) */
#define stMEMSINFO_SIZE sizeof(stMEMSINFO)



/** 
 *  @brief stMEMSCONF : 보통 memg에서 사용하는 memory을 설정하는 structure이다. 
 *
 *
 * @see memg.h 
 *
 *  @note 	사용해야 할 memory에 대한 전체적인 관리.
 */

typedef struct _st_memszone_conf {
	U32					uiZoneID;
/**< <TAG_DEFINE:SEMA>
*	 MEMS_SEMA_ON		1
*	 MEMS_SEMA_OFF		0
*/
	U32 	uiSemFlag;
	U32					uiSemKey;
	U32					uiMemNodeBodySize;
	U32					uiMemNodeTotCnt;
} st_MEMSZONECONF;
#define STG_DEF_st_MEMSZONECONF		104		/* Hex( 531490 ) */
#define st_MEMSZONECONF_SIZE sizeof(st_MEMSZONECONF)


typedef struct _st_memsconf {
/**< <TAG_DEFINE:MEM>
*	 	MEMS_MAIN_MEM		1
*	 	MEMS_SHARED_MEM		2
*/
	U32 	uiType;
	U32					uiShmKey;
	U32					uiHeadRoomSize;
	U32					uiZoneCnt;
	st_MEMSZONECONF		stMEMSZONECONF[MAX_MEMSZONE_CNT];
	U32					uiMatrixZoneID[MAX_SEQ_PROC_NUM];
} st_MEMSCONF;
#define STG_DEF_st_MEMSCONF		105		/* Hex( 531490 ) */
#define st_MEMSCONF_SIZE sizeof(st_MEMSCONF)


/**
 * memg project : External Functions.
 */
extern int mems_dump_DebugString(char *debug_str,char *s,int len);
extern stMEMSINFO *mems_init(st_MEMSCONF *pstMEMSCONF);
extern U8 *mems_alloc(stMEMSINFO *pstMEMSINFO , U32 uiSize, U32 uiZoneID, U8 *pDbgPtr);
extern S32 mems_free(stMEMSINFO *pstMEMSINFO , U8 *pFree, U8 *pDbgPtr);
extern void mems_print_info(S8 *pcPrtPrefixStr,stMEMSINFO *pstMEMSINFO);
extern void mems_print_node(S8 *pcPrtPrefixStr,stMEMSNODEHDR *pstMEMSNODEHDR);
extern void mems_print_all(S8 *pcPrtPrefixStr,stMEMSINFO *pstMEMSINFO);
extern void mems_draw_all(S8 *filename,S8 *labelname,stMEMSINFO *pstMEMSINFO);
extern void mems_garbage_collector(S8 *pcPrtPrefixStr,stMEMSINFO *pstMEMSINFO,int timegap, void (*print_func)(stMEMSINFO *pmem, stMEMSNODEHDR *pmemhdr));
extern void mems_view(S8 *pcPrtPrefixStr,stMEMSINFO *pstMEMSINFO,int timegap, void (*print_func)(stMEMSINFO *pmem, stMEMSNODEHDR *pmemhdr));
extern S32 mems_alloced_cnt(stMEMSINFO *pstMEMSINFO, U32 uiZoneID);
extern S32 mems_sem_init(key_t semkey, U32 flag);
extern S32 P(S32 semid, U32 flag);
extern S32 V(S32 semid, U32 flag);




/* Define.  DEF_NUM(type definition number)
*/
#define		st_MEMSZONECONF_DEF_NUM						104			/* Hex ( 796890 ) */
#define		st_MEMSCONF_DEF_NUM							105			/* Hex ( 797420 ) */
#define		stFlat_st_MEMSCONF_DEF_NUM					105			/* Hex ( 797430 ) */
#define		stMEMSNODEHDR_DEF_NUM						101			/* Hex ( 793b00 ) */
#define		stFlat_stMEMSINFO_DEF_NUM					103			/* Hex ( 795cd0 ) */
#define		stMEMSINFO_DEF_NUM							103			/* Hex ( 795cc0 ) */
#define		stFlat_stMEMSNODEHDR_DEF_NUM				101			/* Hex ( 793b10 ) */
#define		stFlat_st_MEMSZONE_DEF_NUM					102			/* Hex ( 794e80 ) */
#define		st_MEMSZONE_DEF_NUM							102			/* Hex ( 794e70 ) */
#define		stFlat_st_MEMSZONECONF_DEF_NUM				104			/* Hex ( 7968a0 ) */




/* Define.  MEMBER_CNT(struct안의 member들의수 : flat기준)
*/
#define		Short_stMEMSINFO_MEMBER_CNT					8
#define		Short_stMEMSNODEHDR_MEMBER_CNT				6
#define		Short_st_MEMSCONF_MEMBER_CNT				5
#define		Short_st_MEMSZONE_MEMBER_CNT				12
#define		Short_st_MEMSZONECONF_MEMBER_CNT			5
#define		stFlat_Short_stMEMSINFO_MEMBER_CNT			8
#define		stFlat_Short_stMEMSNODEHDR_MEMBER_CNT		6
#define		stFlat_Short_st_MEMSCONF_MEMBER_CNT			5
#define		stFlat_Short_st_MEMSZONE_MEMBER_CNT			12
#define		stFlat_Short_st_MEMSZONECONF_MEMBER_CNT		5
#define		stFlat_stMEMSINFO_MEMBER_CNT				8
#define		stFlat_stMEMSNODEHDR_MEMBER_CNT				6
#define		stFlat_st_MEMSCONF_MEMBER_CNT				5
#define		stFlat_st_MEMSZONE_MEMBER_CNT				12
#define		stFlat_st_MEMSZONECONF_MEMBER_CNT			5
#define		stMEMSINFO_MEMBER_CNT						8
#define		stMEMSNODEHDR_MEMBER_CNT					6
#define		st_MEMSCONF_MEMBER_CNT						5
#define		st_MEMSZONE_MEMBER_CNT						12
#define		st_MEMSZONECONF_MEMBER_CNT					5




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

#pragma pack()

/** file : mems.h
 *     $Log: mems.h,v $
 *     Revision 1.3  2011/08/19 06:35:02  dcham
 *     *** empty log message ***
 *
 *     Revision 1.1  2011/08/18 04:58:28  dcham
 *     *** empty log message ***
 *
 *     Revision 1.1  2011/07/26 06:13:04  dhkim
 *     *** empty log message ***
 *
 *     Revision 1.1.1.1  2011/01/11 01:33:02  jjinri
 *     DIFO
 *
 *     Revision 1.3  2010/12/28 07:24:14  swpark
 *     zone matrix add
 *
 *     Revision 1.2  2010/12/09 08:57:39  jjinri
 *     MEMS_SHM_SIZE
 *
 *     Revision 1.1  2010/12/02 12:50:37  upst_cvs
 *     2010.1202 commit start
 *
 *     Revision 1.2  2010/10/19 02:01:28  upst_cvs
 *     ..
 *
 *     Revision 1.1.1.1  2010/10/15 06:54:37  upst_cvs
 *     WTAS2
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
#endif	/* __mems_h__*/
