#ifndef	__hasho_h__
#define	__hasho_h__
/**		file  hasho.h
 *      - hash header file
 *
 *     Copyright (c) 2006~ by Upresto Inc, Korea
 *     All rights reserved
 *
 *		$Id: hasho.h,v 1.3 2011/08/19 06:35:02 dcham Exp $
 * 
 *     @Author      $Author: dcham $
 *     @version     $Revision: 1.3 $
 *     @date        $Date: 2011/08/19 06:35:02 $
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

#include <commdef.h>
#include <typedef.h>

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
#define STG_DEF_stHASHONODE		101		/* Hex( 43271e0 ) */
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
#define STG_DEF_stHASHOINFO		102		/* Hex( 43271e0 ) */
#define stHASHOINFO_SIZE sizeof(stHASHOINFO)


/**
 *  hasho project : External Functions.
 *   */
extern int hasho_dump_DebugString(char *debug_str,char *s,int len);
extern OFFSET hasho_func_default(void *pa,U8 *pb);
extern stHASHOINFO * hasho_init(U32 uiShmKey, U16 usKeyLen, U16 usSortKeyLen, U16 usDataLen, U32 uiHashSize, OFFSET (*func)(void*,U8*));
extern void hasho_reset(stHASHOINFO *pstHASHOINFO);
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
#define		stHASHOINFO_DEF_NUM							102			/* Hex ( 44d4190 ) */
#define		stFlat_stHASHOINFO_DEF_NUM					102			/* Hex ( 44d41a0 ) */
#define		stFlat_stHASHONODE_DEF_NUM					101			/* Hex ( 458bba0 ) */
#define		stHASHONODE_DEF_NUM							101			/* Hex ( 458bb90 ) */




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
 *     Revision 1.3  2011/08/19 06:35:02  dcham
 *     *** empty log message ***
 *
 *     Revision 1.1  2011/08/18 04:58:27  dcham
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
#endif	/* __hasho_h__*/
