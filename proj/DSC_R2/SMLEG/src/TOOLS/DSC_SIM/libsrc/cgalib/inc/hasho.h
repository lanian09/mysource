#ifndef	__hasho_h__
#define	__hasho_h__
/**		file  hasho.h
 *      - hash header file
 *
 *     Copyright (c) 2006~ by Upresto Inc, Korea
 *     All rights reserved
 *
 *		$Id: hasho.h,v 1.1.1.1 2011/04/19 14:13:43 june Exp $
 * 
 *     @Author      $Author: june $
 *     @version     $Revision: 1.1.1.1 $
 *     @date        $Date: 2011/04/19 14:13:43 $
 *     @warning     $type???�� ���ǵ� �͵鸸 ��밡��
 *     @ref         hasho.h
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

/** @mainpage Offset�� Hash.
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

hash�� memory�� �����ϴ� �� ���� �ƴ� shared memory�� ������ �ؾ� �Ұ��̴�.
shared memory�� �����Ҷ��� pointer�� ������� ���� offset�� ����ؾ� �Ѵ�. 
�׷��� �ٸ� �͵� ���� ���� �����ؼ� ���� �ֱ� �����̴�.  

�� project�� hasho�ν� offset�� ����Ͽ� ó���ϴ� ���� ���ϰ� �� ���̴�.  

 */


#define HASHO_OFFSET(INFOPTR, ptr)  ((OFFSET) (((U8 *) (ptr)) - ((U8 *) INFOPTR)) )
#define HASHO_PTR(INFOPTR, offset)  ((U8 *) (((OFFSET) (offset)) + ((OFFSET) INFOPTR)) )


/** 
 * @brief stHASHONODE : hash���� node���� structure.
 *
 * �� HASH�� doubly linked list�� �����ȴ�. 
 * node�鰣�� ���ῡ�� offset�� ���� ó���Ͽ��� �Ѵ�. 
 *
 * @see hasho.h 
 *
 *  @note 		hash�� NODE (offset����)
 *
 */
typedef struct _st_hasho_node {
    OFFSET offset_next;  		/**< next node�� offset from stHASHOINFO*/ 
    OFFSET offset_prev;  		/**< prev node�� offset from stHASHOINFO*/ 
    OFFSET offset_Key;       /**< Key  Structure Offset */
    OFFSET offset_Data;      /**< Data Structure Offset */
} stHASHONODE;
#define STG_DEF_stHASHONODE		101		/* Hex( 529630 ) */
#define stHASHONODE_SIZE sizeof(stHASHONODE)



/** 
 *  @brief stHASHOINFO : ���� hash���� ����ϴ� node���� �����ϴ� structure�̴�. 
 *
 * ���� hash���� ����ϴ� node���� �����ϴ� structure�̴�. 
 * key�� size�� key�ȿ��� sort�ϱ� ���� �κи��� ����  size�� �����Ͽ���.
 * sortkeylen�� key�� �� �κи��� ����Ų��. 
 *
 * @see hasho.h 
 *
 *  @note 		hash node���� ������ �������ش�. 
 */
typedef struct _st_hashoinfo {
	U32	version;
    OFFSET offset_psthashnode  ;  /**< HASH Node array�� pointer */
    U16 usKeyLen;           /**< Node���� ����� Key ����.  Key �񱳿� copy�� ���ؼ� ���   */
    U16 usDataLen;          /**< Node���� ����� DataLen
                                 @exception pstData�� Structure�� type�� �ܺο����� �˸� �ȴ�. */
    U32 uiHashSize;         /**< Hash ũ��. ������ ������ ���ؼ� set
							 *   Hash Node Array�� ũ��  */
	OFFSET	offset_memginfo;	/**< value : minus */
} stHASHOINFO;
#define STG_DEF_stHASHOINFO		102		/* Hex( 529630 ) */
#define stHASHOINFO_SIZE sizeof(stHASHOINFO)


/**
 *  hasho project : External Functions.
 *   */
extern int hasho_dump_DebugString(char *debug_str,char *s,int len);
extern OFFSET hasho_func_default(void *pa,U8 *pb);
extern stHASHOINFO * hasho_init(U32 uiShmKey, U16 usKeyLen, U16 usSortKeyLen, U16 usDataLen, U32 uiHashSize, OFFSET (*func)(void*,U8*));
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
#define		stHASHOINFO_DEF_NUM							102			/* Hex ( 6d8830 ) */
#define		stFlat_stHASHOINFO_DEF_NUM					102			/* Hex ( 6d8840 ) */
#define		stFlat_stHASHONODE_DEF_NUM					101			/* Hex ( 5487e0 ) */
#define		stHASHONODE_DEF_NUM							101			/* Hex ( 548810 ) */




/* Define.  MEMBER_CNT(struct���� member���Ǽ� : flat����)
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
 *     Revision 1.1.1.1  2011/04/19 14:13:43  june
 *     ���� ��Ű��
 *
 *     Revision 1.1.1.1  2011/01/20 12:18:51  june
 *     DSC CVS RECOVERY
 *
 *     Revision 1.1  2009/05/09 09:35:59  dsc
 *     init
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
#endif	/* __hasho_h__*/