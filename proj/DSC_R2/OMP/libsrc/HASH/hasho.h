#ifndef	__hasho_h__
#define	__hasho_h__
/**		file  hasho.h
 *      - hash header file
 *
 *     Copyright (c) 2006~ by Upresto Inc, Korea
 *     All rights reserved
 *
 *		$Id: hasho.h,v 1.1.1.1 2011/04/19 14:13:49 june Exp $
 * 
 *     @Author      $Author: june $
 *     @version     $Revision: 1.1.1.1 $
 *     @date        $Date: 2011/04/19 14:13:49 $
 *     @warning     $type???로 정의된 것들만 사용가능
 *     @ref         hasho.h
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


#pragma pack(1)

#ifndef __DEFINE_H__
#define __DEFINE_H__

/* 필요한 define 문 선언 (삭제, 추가, 변경이 가능하다) 
	여러 사용자가 사용함으로 주의 요함 - 자신이 사용하는 것만 선언해서 쓰면 됨*/

extern int debug_print(int dIndex, char *fmt, ...);



/* 기본 define 문이다. 아래 값들은 변경은 가능하다. 
	-- FRINTF나 LOG_LEVEL 값 정도  */
#define 	LOG_LEVEL           		stderr
#define 	LOG_BUG   	        		stderr
#define		FPRINTF   	 				dAppLog			
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

#define DEF_OFFSET_SIZE		sizeof(OFFSET)
#endif


/** @mainpage Offset형 Hash.
 \dot
 digraph example {
 fontname=Helvetica;
 label="Work Flow";
 rankdir=LR;
 node [shape=record, fontname=Helvetica, fontsize=10,style=rounded];
 a [ label="structg" URL="http://192.203.140.245/aaa/cjlee/structg"];
 b [ label="hashg" URL="http://192.203.140.245/aaa/cjlee/hashg"];
 c1 [ label="memg" URL="http://192.203.140.245/aaa/cjlee/memg"];
 c2 [ label="hasho" URL="http://192.203.140.245/aaa/cjlee/hasho"];
 c3 [ label="timerg" URL="http://192.203.140.245/aaa/cjlee/timerg"];
 a -> b [label="Define the TYPEDEF", arrowhead="normal", style="dashed", fontname=Helvetica, fontsize=10];
 b -> c2 [label="memory management", arrowhead="normal", style="dashed", fontname=Helvetica, fontsize=10];
 c1 -> c2 [label="apply for \n the offset definition", arrowhead="normal", style="dashed", fontname=Helvetica, fontsize=10];
 c2 -> c3 [label="Define the PRIMITIVEs", fontname=Helvetica, fontsize=10];
}
 \enddot

hash를 memory에 구현하는 것 뿐이 아닌 shared memory에 구현을 해야 할것이다.
shared memory에 구현할때는 pointer를 사용하지 말고 offset을 사용해야 한다. 
그래야 다른 것도 볼때 같이 공유해서 볼수 있기 때문이다.  

이 project가 hasho로써 offset을 사용하여 처리하는 것을 구하게 될 것이다.  

 */


#define HASHO_OFFSET(INFOPTR, ptr)  ((OFFSET) (((U8 *) (ptr)) - ((U8 *) INFOPTR)) )
#define HASHO_PTR(INFOPTR, offset)  ((U8 *) (((OFFSET) (offset)) + ((OFFSET) INFOPTR)) )


/** 
 * @brief stHASHONODE : hash안의 node들의 structure.
 *
 * 현 HASH는 doubly linked list로 구성된다. 
 * node들간의 연결에서 offset을 보고 처리하여야 한다. 
 *
 * @see hasho.h 
 *
 *  @note 		hash의 NODE (offset개념)
 *
 */
typedef struct _st_hasho_node {
    OFFSET offset_next;  		/**< next node의 offset from stHASHOINFO*/ 
    OFFSET offset_prev;  		/**< prev node의 offset from stHASHOINFO*/ 
    OFFSET offset_Key;       /**< Key  Structure Offset */
    OFFSET offset_Data;      /**< Data Structure Offset */
} stHASHONODE;
#define STG_DEF_stHASHONODE		101		/* Hex( 0x65 ) */
#define stHASHONODE_SIZE sizeof(stHASHONODE)



/** 
 *  @brief stHASHOINFO : 보통 hash에서 사용하는 node들을 관리하는 structure이다. 
 *
 * 보통 hash에서 사용하는 node들을 관리하는 structure이다. 
 * key의 size와 key안에서 sort하기 위한 부분만을 위한  size를 구별하였다.
 * sortkeylen은 key의 앞 부분만을 가리킨다. 
 *
 * @see hasho.h 
 *
 *  @note 		hash node들의 정보를 관리해준다. 
 */
typedef struct _st_hashoinfo {
	U32	version;
    OFFSET offset_psthashnode  ;  /**< HASH Node array의 pointer */
    U16 usKeyLen;           /**< Node들이 사용할 Key 길이.  Key 비교와 copy를 위해서 사용   */
    U16 usDataLen;          /**< Node에서 사용할 DataLen
                                 @exception pstData의 Structure의 type은 외부에서만 알면 된다. */
    U32 uiHashSize;         /**< Hash 크기. 임의의 설정을 위해서 set
							 *   Hash Node Array의 크기  */
	OFFSET	offset_memginfo;	/**< value : minus */
} stHASHOINFO;
#define STG_DEF_stHASHOINFO		102		/* Hex( 0x66 ) */
#define stHASHOINFO_SIZE sizeof(stHASHOINFO)

#define RESET			1
#define CONTN			0


/**
 *  hasho project : External Functions.
 *   */
extern int hasho_dump_DebugString(char *debug_str,char *s,int len);
extern U32 hasho_func_default(void *pa,U8 *pb);
extern stHASHOINFO * hasho_init(U32 uiShmKey, U16 usKeyLen, U16 usSortKeyLen, U16 usDataLen, U32 uiHashSize, U32 (*func)(void*,U8*), U8 ucResetFlag );
extern void hasho_link_node(stHASHOINFO *pstHASHOINFO , stHASHONODE *p);
extern void hasho_unlink_node(stHASHOINFO *pstHASHOINFO, stHASHONODE *p);
extern stHASHONODE * hasho_find(stHASHOINFO *pstHASHOINFO, U8 *pKey);
extern stHASHONODE * hasho_add(stHASHOINFO *pstHASHOINFO, U8 *pKey, U8 *pData);
extern void hasho_del(stHASHOINFO *pstHASHOINFO, U8 *pKey);
extern void hasho_print_info(S8 *pcPrtPrefixStr,stHASHOINFO *pstHASHOINFO);
extern void hasho_print_node(S8 *pcPrtPrefixStr,stHASHOINFO *pstHASHOINFO,stHASHONODE *pstHASHONODE);
extern void hasho_print_all(S8 *pcPrtPrefixStr,stHASHOINFO *pstHASHOINFO);
extern void hasho_draw_all(S8 *filename,S8 *labelname,stHASHOINFO *pstHASHOINFO);
extern U32 hasho_get_occupied_node_count(stHASHOINFO *pstHASHOINFO);




/* Define.  DEF_NUM(type definition number)
*/
#define		stHASHOINFO_DEF_NUM							102			/* Hex ( 0x66 ) */
#define		stFlat_stHASHOINFO_DEF_NUM					102			/* Hex ( 0x66 ) */
#define		stFlat_stHASHONODE_DEF_NUM					101			/* Hex ( 0x65 ) */
#define		stHASHONODE_DEF_NUM							101			/* Hex ( 0x65 ) */




/* Define.  MEMBER_CNT(struct안의 member들의수 : flat기준)
*/
#define		Short_stHASHOINFO_MEMBER_CNT				6
#define		Short_stHASHONODE_MEMBER_CNT				4
#define		stFlat_Short_stHASHOINFO_MEMBER_CNT			6
#define		stFlat_Short_stHASHONODE_MEMBER_CNT			4
#define		stFlat_stHASHOINFO_MEMBER_CNT				6
#define		stFlat_stHASHONODE_MEMBER_CNT				4
#define		stHASHOINFO_MEMBER_CNT						6
#define		stHASHONODE_MEMBER_CNT						4




/* Extern Function Define.
*/
extern void stHASHOINFO_CILOG(FILE *fp, stHASHOINFO *pthis);
extern void stHASHOINFO_Dec(stHASHOINFO *pstTo , stHASHOINFO *pstFrom);
extern void stHASHOINFO_Enc(stHASHOINFO *pstTo , stHASHOINFO *pstFrom);
extern void stHASHOINFO_Prt(S8 *pcPrtPrefixStr, stHASHOINFO *pthis);
extern void stHASHONODE_CILOG(FILE *fp, stHASHONODE *pthis);
extern void stHASHONODE_Dec(stHASHONODE *pstTo , stHASHONODE *pstFrom);
extern void stHASHONODE_Enc(stHASHONODE *pstTo , stHASHONODE *pstFrom);
extern void stHASHONODE_Prt(S8 *pcPrtPrefixStr, stHASHONODE *pthis);


#pragma pack()

/** file : hasho.h
 *     $Log: hasho.h,v $
 *     Revision 1.1.1.1  2011/04/19 14:13:49  june
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
 *     Revision 1.1.1.1  2008/12/30 02:32:27  upst_cvs
 *     BSD R3.0.0
 *
 *     Revision 1.2  2008/10/15 12:04:11  jsyoon
 *     MIF 헤더파일 정리
 *
 *     Revision 1.1  2008/10/12 23:21:57  jsyoon
 *     *** empty log message ***
 *
 *     Revision 1.1.1.1  2008/06/09 08:17:19  jsyoon
 *     WATAS3 PROJECT START
 *
 *     Revision 1.1  2007/08/21 12:22:38  dark264sh
 *     no message
 *
 *     */
#endif	/* __hasho_h__*/
