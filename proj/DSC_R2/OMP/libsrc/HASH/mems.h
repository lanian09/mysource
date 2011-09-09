#ifndef	__mems_h__
#define	__mems_h__
/**		file  mems.h
 *      - hash header file
 *
 *     Copyright (c) 2006~ by Upresto Inc, Korea
 *     All rights reserved
 *
 *		$Id: mems.h,v 1.1.1.1 2011/04/19 14:13:50 june Exp $
 * 
 *     @Author      $Author: june $
 *     @version     $Revision: 1.1.1.1 $
 *     @date        $Date: 2011/04/19 14:13:50 $
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


#ifndef __DEFINE_H__
#define __DEFINE_H__

#pragma pack(1)
/* 필요한 define 문 선언 (삭제, 추가, 변경이 가능하다) 
	여러 사용자가 사용함으로 주의 요함 - 자신이 사용하는 것만 선언해서 쓰면 됨*/

extern int debug_print(int dIndex, char *fmt, ...);



/* 기본 define 문이다. 아래 값들은 변경은 가능하다. 
	-- FRINTF나 LOG_LEVEL 값 정도  */
#define 	LOG_LEVEL           		stderr
#define 	LOG_BUG   	        		stderr
#define		FPRINTF   	 				fprintf			
#define		FILEPRINT		fprintf


#define 	HIPADDR(d) 		((d>>24)&0xff),((d>>16)&0xff),((d>>8)&0xff),(d&0xff)
#define 	NIPADDR(d) 		(d&0xff),((d>>8)&0xff),((d>>16)&0xff),((d>>24)&0xff)
#define		ASSERT(x)		if(! (x) ){ printf("EXIT\n");  exit(-1); }
#define		MALLOC			malloc
#define		FREE			free

#define		NTOH32(_DEF_TO,_DEF_FROM)   	 {												\
	U32	 _DEF_FROM_TEMP;																	\
	char *_def_to,*_def_from;											 					\
    _DEF_FROM_TEMP = (U32) _DEF_FROM;     													\
	_def_to = (char *) &_DEF_TO;															\
	_def_from = (char *) &_DEF_FROM_TEMP;													\
    _def_to[0] = _def_from[3];        														\
    _def_to[1] = _def_from[2];        														\
    _def_to[2] = _def_from[1];        														\
    _def_to[3] = _def_from[0];        														\
}															

#define		NTOH64(_DEF_TO,_DEF_FROM)   	 {												\
	U64	 _DEF_FROM_TEMP;																	\
	char *_def_to,*_def_from;											 					\
    _DEF_FROM_TEMP = (U64) _DEF_FROM;    													\
	_def_to = (char *) &_DEF_TO;															\
	_def_from = (char *) &_DEF_FROM_TEMP;													\
    _def_to[0] = _def_from[7];        														\
    _def_to[1] = _def_from[6];        														\
    _def_to[2] = _def_from[5];        														\
    _def_to[3] = _def_from[4];        														\
    _def_to[4] = _def_from[3];        														\
    _def_to[5] = _def_from[2];        														\
    _def_to[6] = _def_from[1];        														\
    _def_to[7] = _def_from[0];  															\
}															
				
#define		NTOH64V2(_DEF_TO,_DEF_FROM)   	 {												\
	U64	 _DEF_FROM_TEMP;																	\
	char *_def_to,*_def_from;											 					\
    _DEF_FROM_TEMP = (U64) _DEF_FROM;    													\
	_def_to = (char *) &_DEF_TO;															\
	_def_from = (char *) &_DEF_FROM_TEMP;													\
    _def_to[0] = _def_from[3];        														\
    _def_to[1] = _def_from[2];        														\
    _def_to[2] = _def_from[1];        														\
    _def_to[3] = _def_from[0];        														\
    _def_to[4] = _def_from[7];        														\
    _def_to[5] = _def_from[6];        														\
    _def_to[6] = _def_from[5];        														\
    _def_to[7] = _def_from[4];  															\
}															

#define STG_DiffTIME64(ENDT,ENDM,STARTT,STARTM,RESULT)	{												\
			*(RESULT) = (((S64)ENDT * 1000000 + (S64)ENDM) - ((S64)STARTT * 1000000 + (S64)STARTM));	\
			if( *(RESULT) > (S64) 3600 * 24 * 1000000 ){ *(RESULT) = 0;}								\
			else if( *(RESULT) < (S64) 0 ){ *(RESULT) = 0;}												\
}
#define STG_Diff32(FIRST,SECOND,RESULT) {													\
			*RESULT = (S32) (FIRST - SECOND);												\
}
#define STG_Equal(FROM, TO) {																\
			/* 제대로 처리 되기 위해서는 #pragma pack(1)으로 선언해야함. */					\
			memcpy(TO , & (FROM) , sizeof(FROM));											\
}
#define STG_Percent4(FIRST,SECOND,RESULT) {													\
			if(SECOND == 0){ *RESULT = 0; }													\
			else { *RESULT = (U32) ( FIRST * 10000 / SECOND ); }							\
}
#define STG_Percent3(FIRST,SECOND,RESULT) {													\
			if(SECOND == 0){ *RESULT = 0; }													\
			else { *RESULT = (U32) ( FIRST * 1000 / SECOND ); }								\
}
#define STG_Percent2(FIRST,SECOND,RESULT) {													\
			if(SECOND == 0){ *RESULT = 0; }													\
			else { *RESULT = (U32) ( FIRST * 100 / SECOND ); }								\
}
#define AVERAGE(FIRST,SECOND,RESULT) {														\
			if(SECOND == 0){ *RESULT = 0; }													\
			else { *RESULT = (FLOAT) ( FIRST / SECOND ); }									\
}

#define STG_INTEGER		1
#define STG_STRING		2
#define STG_IP			3
#define STG_DEF			4

#endif




/* code gen에서 자동으로 정의되는 type들.
*/
#ifndef __TYPEDEF_H__
#define __TYPEDEF_H__
#define		 DEF   	 unsigned int
#define		 FLOAT   	 float
#define		 IP4   	 int
#define		 MTIME   	 int
#define		 OFFSET   	 long
#define		 S16   	 short
#define		 S32   	 int
#define		 S64   	 long long
#define		 S8   	 char
#define		 STIME   	 int
#define		 STRING   	 unsigned char
#define		 U16   	 unsigned short
#define		 U32   	 unsigned int
#define		 U64   	 unsigned long long
#define		 U8   	 unsigned char
#define		 UTIME64   	 unsigned long long
#define		 X8   	 unsigned char
#endif


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
	U32	uiID;								/**< MEMS_ID : mem debug를 위한 부분 : free시 확인 */ 
	STIME	TimeSec;						/**< Debug나 garbage collection을 위한 시간 저장 */
/**< <TAG_DEFINE:FREE_ALLOCED>
*	 MEMS_FREE			0
*	 MEMS_ALLOCED		1
*/
	U8 	ucIsFree;							/**< Free:0 , Alloc : 1 */
	S8	DebugStr[MEMS_MAX_DEBUG_STR];		/**< debugging을 위해서 사용된 CREATE */
	S8	DelStr[MEMS_MAX_DEBUG_STR];			/**< debugging을 위해서 사용된 DELETE */
} stMEMSNODEHDR;
#define STG_DEF_stMEMSNODEHDR		101		/* Hex( 0x65 ) */
#define stMEMSNODEHDR_SIZE sizeof(stMEMSNODEHDR)



/** 
 *  @brief stMEMSINFO : 보통 memg에서 사용하는 memory을 관리하는 structure이다. 
 *
 *
 * @see memg.h 
 *
 *  @note 	사용해야 할 memory에 대한 전체적인 관리.
 */
typedef struct _st_memsinfo {
/**< <TAG_DEFINE:MEM>
*	 	MEMS_MAIN_MEM		1
*	 	MEMS_SHARED_MEM		2
*/
	U32 	uiType;			/**<  Main Memory = 1 , Shared Memory = 2  */
    U32		uiShmKey;		/**<  uiType ==  Shared Memory : shared mem Key */
/**< <TAG_DEFINE:SEMA>
*	 MEMS_SEMA_ON		1
*	 MEMS_SEMA_OFF		0
*/
	U32		uiSemFlag;				/**< Semaphore 사용 여부 */
	U32		uiSemKey;				/**< Semaphore Key 값 */
	S32		iSemID;					/**< Semaphore ID 값 */ 
	U32		uiReserved1;			/**< Padding for 64Bit Alignment */
    U64		uiTotMemSize;			/**< 전체 사용가능한 memory (alloc또는 shared mem size */
    U32		uiHeadRoomSize;			/**< 실제로 node들이 놓이기 전에 빈 공간내지는 다른 내용을 사용하기 위한 공간 */
    U32		uiMemNodeHdrSize;		/**< sizeof(stMEMSNODEHDR) */
    U32		uiMemNodeBodySize;		/**< Node안의 hdr뒤에 붙는 Body의 Size :\n이 Body의 Size가 실제로 원하는 Size가 될 것임 */
    U32		uiMemNodeAllocedCnt;	/**< 이미 alloc되어진 갯수 */
    U32		uiMemNodeTotCnt;		/**< 전체 노드의 갯수 */
	U32		uiReserved2;			/**< Padding for 64Bit Alignment */
    OFFSET	offsetHeadRoom;			/**< HeadRoom의 위치 */
    OFFSET	offsetNodeStart;		/**< Node들의 처음 위치 */
    OFFSET	offsetFreeList;			/**< Free된 Node의 위치 */
    OFFSET	offsetNodeEnd;			/**< Node의 마지막 Offset */
	U64		createCnt;
	U64		delCnt;
} stMEMSINFO;
#define STG_DEF_stMEMSINFO		102		/* Hex( 0x66 ) */
#define stMEMSINFO_SIZE sizeof(stMEMSINFO)



/**
 * memg project : External Functions.
 */
extern int mems_dump_DebugString(char *debug_str,char *s,int len);
extern stMEMSINFO *mems_init(U32 uiType,U32 uiShmKey, U32 uiSemFlag, U32 uiSemKey, U32 uiHeadRoomSize, U32 uiMemNodeBodySize,U32 uiMemNodeTotCnt);
extern U8 *mems_alloc(stMEMSINFO *pstMEMSINFO , U32 uiSize, U8 *pDbgPtr);
extern S32 mems_free(stMEMSINFO *pstMEMSINFO , U8 *pFree, U8 *pDbgPtr);
extern void mems_print_info(S8 *pcPrtPrefixStr,stMEMSINFO *pstMEMSINFO);
extern void mems_print_node(S8 *pcPrtPrefixStr,stMEMSNODEHDR *pstMEMSNODEHDR);
extern void mems_print_all(S8 *pcPrtPrefixStr,stMEMSINFO *pstMEMSINFO);
extern void mems_draw_all(S8 *filename,S8 *labelname,stMEMSINFO *pstMEMSINFO);
extern void mems_garbage_collector(S8 *pcPrtPrefixStr,stMEMSINFO *pstMEMSINFO,int timegap, void (*print_func)(stMEMSINFO *pmem, stMEMSNODEHDR *pmemhdr));
extern S32 mems_alloced_cnt(stMEMSINFO *pstMEMSINFO);
extern void mems_view(S8 *pcPrtPrefixStr, stMEMSINFO *pstMEMSINFO, int timegap, void (*print_func)(stMEMSINFO *pmem, stMEMSNODEHDR * pmemhdr));




/* Define.  DEF_NUM(type definition number)
*/
#define		stFlat_stMEMSNODEHDR_DEF_NUM				101			/* Hex ( 0x65 ) */
#define		stMEMSNODEHDR_DEF_NUM						101			/* Hex ( 0x65 ) */
#define		stFlat_stMEMSINFO_DEF_NUM					102			/* Hex ( 0x66 ) */
#define		stMEMSINFO_DEF_NUM							102			/* Hex ( 0x66 ) */




/* Define.  MEMBER_CNT(struct안의 member들의수 : flat기준)
*/
#define		Short_stMEMSINFO_MEMBER_CNT					17
#define		Short_stMEMSNODEHDR_MEMBER_CNT				5
#define		stFlat_Short_stMEMSINFO_MEMBER_CNT			17
#define		stFlat_Short_stMEMSNODEHDR_MEMBER_CNT		5
#define		stFlat_stMEMSINFO_MEMBER_CNT				17
#define		stFlat_stMEMSNODEHDR_MEMBER_CNT				5
#define		stMEMSINFO_MEMBER_CNT						17
#define		stMEMSNODEHDR_MEMBER_CNT					5




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



#pragma pack()
/** file : mems.h
 *     $Log: mems.h,v $
 *     Revision 1.1.1.1  2011/04/19 14:13:50  june
 *     성능 패키지
 *
 *     Revision 1.1.1.1  2011/01/20 12:18:56  june
 *     DSC CVS RECOVERY
 *
 *     Revision 1.1.1.1  2009/07/12 20:16:07  dsc
 *     LGT 상암 init
 *
 *     Revision 1.1  2009/04/25 13:42:20  june
 *     hash
 *
 *     Revision 1.1.1.1  2009/04/06 13:02:06  june
 *     LGT DSC project init
 *
 *     Revision 1.1.1.1  2009/04/06 09:10:25  june
 *     LGT DSC project start
 *
 *     Revision 1.1.1.1  2008/12/30 02:32:26  upst_cvs
 *     BSD R3.0.0
 *
 *     Revision 1.2  2008/10/15 12:04:11  jsyoon
 *     MIF 헤더파일 정리
 *
 *     Revision 1.1  2008/10/12 22:48:34  jsyoon
 *     MIF 헤더 추가
 *
 *     Revision 1.1  2008/10/07 23:29:05  jsyoon
 *     *** empty log message ***
 *
 *     Revision 1.1.1.1  2008/06/09 08:17:19  jsyoon
 *     WATAS3 PROJECT START
 *
 *     Revision 1.1  2007/08/21 12:22:38  dark264sh
 *     no message
 *
 *     */
#endif	/* __mems_h__*/
