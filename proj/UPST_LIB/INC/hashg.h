#ifndef	__hashg_h__
#define	__hashg_h__
/**		file  hashg.h
 *      - hash header file
 *
 *     Copyright (c) 2006~ by Upresto Inc, Korea
 *     All rights reserved
 *
 *		$Id: hashg.h,v 1.3 2011/08/19 06:35:02 dcham Exp $
 * 
 *     @Author      $Author: dcham $
 *     @version     $Revision: 1.3 $
 *     @date        $Date: 2011/08/19 06:35:02 $
 *     @warning     $type???로 정의된 것들만 사용가능
 *     @ref         hashg.h
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


/** @mainpage Work_Flow & Hash_Table
 \dot
 digraph example {
 fontname=Helvetica;
 label="Work Flow";
 rankdir=LR;
 node [shape=record, fontname=Helvetica, fontsize=10,style=rounded];
 a [ label="structg" URL="http://192.203.140.245/aaa/cjlee/structg"];
 b [ label="hashg" URL="http://192.203.140.245/aaa/cjlee/hashg"];
 d [ label="hasho" URL="http://192.203.140.245/aaa/cjlee/hasho"];
 c [ label="timerg" URL="http://192.203.140.245/aaa/cjlee/timerg"];
 a -> b [label="Define the TYPEDEF", arrowhead="normal", style="dashed", fontname=Helvetica, fontsize=10];
 b -> d [label="apply for \n the offset definition", arrowhead="normal", style="dashed", fontname=Helvetica, fontsize=10];
 d -> c [label="Define the PRIMITIVEs", fontname=Helvetica, fontsize=10];
}
 \enddot

 */


/** 
 * @brief stHASHGNODE : hash안의 node들의 structure.
 *
 * 현 HASH는 doubly linked list로 구성된다. 
 * 확장을 가능하게 하기 위해서 key와 data 부분을 분리 하였다.
 *
 * @see hash.h 
 *
 *  @note 		+Note+ 
 *
 */
typedef struct _st_hashgnode {
	U8 *pstHASHGINFO;				/**< 같은 모양의 node이면 hash간의 이동이 가능하게 하기 위해서*/ 
    struct _st_hashgnode *next  ;  	/**< self-pointer */ 
    struct _st_hashgnode **prev; 	/**< self-pointer */
    U8 *pstKey;       /**< Key  Structure Pointer */
    U8 *pstData;      /**< Data Structure Pointer */
} stHASHGNODE;
#define STG_DEF_stHASHGNODE		101		/* Hex( 5c861e0 ) */
#define stHASHGNODE_SIZE sizeof(stHASHGNODE)



/** 
 *  @brief stHASHGINFO : 보통 hash에서 사용하는 node들을 관리하는 structure이다. 
 *
 * 보통 hash에서 사용하는 node들을 관리하는 structure이다. 
 * key의 size와 key안에서 sort하기 위한 부분만을 위한  size를 구별하였다.
 * sortkeylen은 key의 앞 부분만을 가리킨다. 
 *
 * @see hash.h 
 *
 *  @note 		hash node들의 정보를 관리해준다. 
 */
typedef struct _st_hashginfo {
    stHASHGNODE *psthashnode  ;  /**< HASH Node array의 pointer*/ 
    U16 usKeyLen;           /**< Node들이 사용할 Key 길이.  Key 비교와 copy를 위해서 사용  */ 
    U16 usDataLen;          /**< Node에서 사용할 DataLen
                                 @exception pstData의 Structure의 type은 외부에서만 알면 된다. */
    U32 uiHashSize;         /**< Hash 크기. 임의의 설정을 위해서 set Hash Node Array의 크기  */
	U32	MaxNodeCnt;			/**< Max Node Count */
	U32 (*func)(void*,U8*);     /**< stHASHGINFO* , stKey* : hash_function pointer - init에서 set*/ 
	S32 (*print_key)(S8*,S8*,S32);     /**< stHASHGINFO* , stKey* : hash_function pointer - init에서 set*/ 
	U32	uiLinkedCnt;		/**< link로 매달린 node의 갯수 */ 
} stHASHGINFO;
#define STG_DEF_stHASHGINFO		102		/* Hex( 5c861e0 ) */
#define stHASHGINFO_SIZE sizeof(stHASHGINFO)



/**
 * hashg project : External Functions.
 */
extern void hashg_print_info(S8 *pcPrtPrefixStr,stHASHGINFO *pstHASHGINFO);
extern void hashg_print_node(S8 *pcPrtPrefixStr,stHASHGINFO *pstHASHGINFO,stHASHGNODE *pstHASHGNODE);
extern void hashg_print_all(S8 *pcPrtPrefixStr,stHASHGINFO *pstHASHGINFO);
extern void hashg_draw_all(S8 *filename,S8 *labelname,stHASHGINFO *pstHASHGINFO);
extern S32 hashg_dump_DebugString(S8 *debug_str,S8 *s,S32 len);
extern U32 hashg_func_default(void *pa,U8 *pb);
extern stHASHGINFO * hashg_init(U32 (*hashg_func)(void*,U8*),U16 usKeyLen, S32 (*print_key)(S8*,S8*,S32), U16 usDataLen, U32 uiHashSize);
extern void hashg_link_node(stHASHGINFO *pstHASHGINFO , stHASHGNODE *p);
extern void hashg_unlink_node(stHASHGNODE *p);
extern stHASHGNODE * hashg_find(stHASHGINFO *pstHASHGINFO, U8 *pKey);
extern stHASHGNODE * hashg_add(stHASHGINFO *pstHASHGINFO, U8 *pKey, U8 *pData);
extern void hashg_del_from_key(stHASHGINFO *pstHASHGINFO,U8 *pKey);
extern void hashg_del(stHASHGNODE *pstHASHGNODE);
extern U32 hashg_get_occupied_node_count(stHASHGINFO *pstHASHGINFO);
extern void hashg_set_MaxNodeCnt(stHASHGINFO *pstHASHGINFO,U32 MaxNodeCnt);




/* Define.  DEF_NUM(type definition number)
*/
#define		stFlat_stHASHGINFO_DEF_NUM					102			/* Hex ( 5e32fe0 ) */
#define		stHASHGINFO_DEF_NUM							102			/* Hex ( 5e32fd0 ) */
#define		stHASHGNODE_DEF_NUM							101			/* Hex ( 5eeaa40 ) */
#define		stFlat_stHASHGNODE_DEF_NUM					101			/* Hex ( 5eeaa50 ) */




/* Define.  MEMBER_CNT(struct안의 member들의수 : flat기준)
*/
#define		Short_stHASHGINFO_MEMBER_CNT				5
#define		Short_stHASHGNODE_MEMBER_CNT				0
#define		stFlat_Short_stHASHGINFO_MEMBER_CNT			5
#define		stFlat_Short_stHASHGNODE_MEMBER_CNT			0
#define		stFlat_stHASHGINFO_MEMBER_CNT				8
#define		stFlat_stHASHGNODE_MEMBER_CNT				5
#define		stHASHGINFO_MEMBER_CNT						8
#define		stHASHGNODE_MEMBER_CNT						5




/* Extern Function Define.
*/
extern void stHASHGINFO_CILOG(FILE *fp, stHASHGINFO *pthis);
extern void stHASHGINFO_Dec(stHASHGINFO *pstTo , stHASHGINFO *pstFrom);
extern void stHASHGINFO_Enc(stHASHGINFO *pstTo , stHASHGINFO *pstFrom);
extern void stHASHGINFO_Prt(S8 *pcPrtPrefixStr, stHASHGINFO *pthis);
extern void stHASHGNODE_CILOG(FILE *fp, stHASHGNODE *pthis);
extern void stHASHGNODE_Dec(stHASHGNODE *pstTo , stHASHGNODE *pstFrom);
extern void stHASHGNODE_Enc(stHASHGNODE *pstTo , stHASHGNODE *pstFrom);
extern void stHASHGNODE_Prt(S8 *pcPrtPrefixStr, stHASHGNODE *pthis);

#pragma pack()

/** file : hashg.h
 *     $Log: hashg.h,v $
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
#endif	/* __hashg_h__*/
