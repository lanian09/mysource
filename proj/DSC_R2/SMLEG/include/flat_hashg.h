/**		file  flat_hashg.h
 *      - hash header file
 *
 *     Copyright (c) 2006~ by Upresto Inc, Korea
 *     All rights reserved
 *
 *		$Id: flat_hashg.h,v 1.1.1.1 2011/04/19 14:13:48 june Exp $
 * 
 *     @Author      $Author: june $
 *     @version     $Revision: 1.1.1.1 $
 *     @date        $Date: 2011/04/19 14:13:48 $
 *     @warning     $type???로 정의된 것들만 사용가능
 *     @ref         flat_hashg.h
 *     @todo        Makefile을 만들자
 *
 *     @section     Intro(소개)
 *      - hash header file
 *
 *     @section     Requirement
 *      @li 규칙에 틀린 곳을 찾아주세요.
 *
 **/


#include "hashg.h"
#pragma pack(1)

/** FileName : flat_hashg.h    .
 *
 * NTAF struct  name : Short__st_hashgnode 
 * NTAM struct  name : stFlat_Short__st_hashgnode 
 * NTAF typedef name : Short_stHASHGNODE
 * NTAM typedef name : stFlat_Short_stHASHGNODE
 **/
typedef struct stFlat_Short__st_hashgnode  {
} stFlat_Short_stHASHGNODE ;
#define stFlat_Short_stHASHGNODE_SIZE sizeof(stFlat_Short_stHASHGNODE)


/** FileName : flat_hashg.h    .
 *
 * NTAF struct  name : _st_hashgnode 
 * NTAM struct  name : stFlat__st_hashgnode 
 * NTAF typedef name : stHASHGNODE
 * NTAM typedef name : stFlat_stHASHGNODE
 **/
typedef struct stFlat__st_hashgnode  {
	U8 *pstHASHGINFO; 	/**< U8 *pstHASHGINFO */
	struct stFlat__st_hashgnode *next  ; 	/**< struct stFlat__st_hashgnode *next   */
	struct stFlat__st_hashgnode **prev; 	/**< struct stFlat__st_hashgnode **prev */
	U8 *pstKey; 	/**< U8 *pstKey */
	U8 *pstData; 	/**< U8 *pstData */
} stFlat_stHASHGNODE ;
#define stFlat_stHASHGNODE_SIZE sizeof(stFlat_stHASHGNODE)


/** FileName : flat_hashg.h    .
 *
 * NTAF struct  name : Short__st_hashginfo 
 * NTAM struct  name : stFlat_Short__st_hashginfo 
 * NTAF typedef name : Short_stHASHGINFO
 * NTAM typedef name : stFlat_Short_stHASHGINFO
 **/
typedef struct stFlat_Short__st_hashginfo  {
	U16	usKeyLen; 	/**< U16	usKeyLen */
	U16	usDataLen; 	/**< U16	usDataLen */
	U32	uiHashSize; 	/**< U32	uiHashSize */
	U32	MaxNodeCnt; 	/**< U32	MaxNodeCnt */
	U32	uiLinkedCnt; 	/**< U32	uiLinkedCnt */
} stFlat_Short_stHASHGINFO ;
#define stFlat_Short_stHASHGINFO_SIZE sizeof(stFlat_Short_stHASHGINFO)


/** FileName : flat_hashg.h    .
 *
 * NTAF struct  name : _st_hashginfo 
 * NTAM struct  name : stFlat__st_hashginfo 
 * NTAF typedef name : stHASHGINFO
 * NTAM typedef name : stFlat_stHASHGINFO
 **/
typedef struct stFlat__st_hashginfo  {
	stFlat_stHASHGNODE 	*psthashnode  ;	/**< stHASHGNODE *psthashnode   */
	U16 usKeyLen; 	/**< U16 usKeyLen */
	U16 usDataLen; 	/**< U16 usDataLen */
	U32 uiHashSize; 	/**< U32 uiHashSize */
	U32	MaxNodeCnt; 	/**< U32	MaxNodeCnt */
	U32 (*func)(void*,U8*); 	/**< U32 (*func)(void*,U8*) */
	S32 (*print_key)(S8*,S8*,S32); 	/**< S32 (*print_key)(S8*,S8*,S32) */
	U32	uiLinkedCnt; 	/**< U32	uiLinkedCnt */
} stFlat_stHASHGINFO ;
#define stFlat_stHASHGINFO_SIZE sizeof(stFlat_stHASHGINFO)

#pragma pack(0)

extern int DBInsert_stFlat_Short_stHASHGINFO(int dCount, stFlat_Short_stHASHGINFO *pstData, char *pszName, int *pdErrRow);
extern int DBInsert_stFlat_Short_stHASHGNODE(int dCount, stFlat_Short_stHASHGNODE *pstData, char *pszName, int *pdErrRow);
extern int DBInsert_stFlat_stHASHGINFO(int dCount, stFlat_stHASHGINFO *pstData, char *pszName, int *pdErrRow);
extern int DBInsert_stFlat_stHASHGNODE(int dCount, stFlat_stHASHGNODE *pstData, char *pszName, int *pdErrRow);

#pragma pack()

/** file : flat_hashg.h
 *     $Log: flat_hashg.h,v $
 *     Revision 1.1.1.1  2011/04/19 14:13:48  june
 *     성능 패키지
 *
 *     Revision 1.1.1.1  2011/01/20 12:18:55  june
 *     DSC CVS RECOVERY
 *
 *     Revision 1.2  2009/06/28 15:30:01  dsc
 *     cgalib hashg_init에 초기화 추가
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
