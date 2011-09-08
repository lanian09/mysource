/**		file  flat_timerN.h
 *      - hash header file
 *
 *     Copyright (c) 2006~ by Upresto Inc, Korea
 *     All rights reserved
 *
 *		$Id: flat_timerN.h,v 1.1.1.1 2011/04/19 14:13:48 june Exp $
 * 
 *     @Author      $Author: june $
 *     @version     $Revision: 1.1.1.1 $
 *     @date        $Date: 2011/04/19 14:13:48 $
 *     @warning     $type???로 정의된 것들만 사용가능
 *     @ref         flat_timerN.h
 *     @todo        Makefile을 만들자
 *
 *     @section     Intro(소개)
 *      - hash header file
 *
 *     @section     Requirement
 *      @li 규칙에 틀린 곳을 찾아주세요.
 *
 **/


#include "timerN.h"
#pragma pack(1)

/** FileName : flat_timerN.h    .
 *
 * NTAF struct  name : Short__st_timerNinfo 
 * NTAM struct  name : stFlat_Short__st_timerNinfo 
 * NTAF typedef name : Short_stTIMERNINFO
 * NTAM typedef name : stFlat_Short_stTIMERNINFO
 **/
typedef struct stFlat_Short__st_timerNinfo  {
	U32	uiMaxNodeCnt; 	/**< U32	uiMaxNodeCnt */
	U32	uiNodeCnt; 	/**< U32	uiNodeCnt */
	U32	uiArgMaxSize; 	/**< U32	uiArgMaxSize */
	U32	uiTimerNIdIndex; 	/**< U32	uiTimerNIdIndex */
	STIME	uiCurrentTime; 	/**< STIME	uiCurrentTime */
} stFlat_Short_stTIMERNINFO ;
#define stFlat_Short_stTIMERNINFO_SIZE sizeof(stFlat_Short_stTIMERNINFO)


/** FileName : flat_timerN.h    .
 *
 * NTAF struct  name : _st_timerNinfo 
 * NTAM struct  name : stFlat__st_timerNinfo 
 * NTAF typedef name : stTIMERNINFO
 * NTAM typedef name : stFlat_stTIMERNINFO
 **/
typedef struct stFlat__st_timerNinfo  {
	void  *pstHASHGINFO  ; 	/**< void  *pstHASHGINFO   */
	U32 uiMaxNodeCnt; 	/**< U32 uiMaxNodeCnt */
	U32 uiNodeCnt; 	/**< U32 uiNodeCnt */
	U32 uiArgMaxSize; 	/**< U32 uiArgMaxSize */
	U32	uiTimerNIdIndex; 	/**< U32	uiTimerNIdIndex */
	STIME	uiCurrentTime; 	/**< STIME	uiCurrentTime */
} stFlat_stTIMERNINFO ;
#define stFlat_stTIMERNINFO_SIZE sizeof(stFlat_stTIMERNINFO)


/** FileName : flat_timerN.h    .
 *
 * NTAF struct  name : Short__st_timerNkey 
 * NTAM struct  name : stFlat_Short__st_timerNkey 
 * NTAF typedef name : Short_stTIMERNKEY
 * NTAM typedef name : stFlat_Short_stTIMERNKEY
 **/
typedef struct stFlat_Short__st_timerNkey  {
	U32	uiTimerNIdIndex; 	/**< U32	uiTimerNIdIndex */
	STIME	sTimeKey; 	/**< STIME	sTimeKey */
} stFlat_Short_stTIMERNKEY ;
#define stFlat_Short_stTIMERNKEY_SIZE sizeof(stFlat_Short_stTIMERNKEY)


/** FileName : flat_timerN.h    .
 *
 * NTAF struct  name : _st_timerNkey 
 * NTAM struct  name : stFlat__st_timerNkey 
 * NTAF typedef name : stTIMERNKEY
 * NTAM typedef name : stFlat_stTIMERNKEY
 **/
typedef struct stFlat__st_timerNkey  {
	U32	uiTimerNIdIndex; 	/**< U32	uiTimerNIdIndex */
	STIME 	sTimeKey; 	/**< STIME 	sTimeKey */
} stFlat_stTIMERNKEY ;
#define stFlat_stTIMERNKEY_SIZE sizeof(stFlat_stTIMERNKEY)


/** FileName : flat_timerN.h    .
 *
 * NTAF struct  name : Short__st_timerNdata 
 * NTAM struct  name : stFlat_Short__st_timerNdata 
 * NTAF typedef name : Short_stTIMERNDATA
 * NTAM typedef name : stFlat_Short_stTIMERNDATA
 **/
typedef struct stFlat_Short__st_timerNdata  {
	S32	arg; 	/**< S32	arg */
} stFlat_Short_stTIMERNDATA ;
#define stFlat_Short_stTIMERNDATA_SIZE sizeof(stFlat_Short_stTIMERNDATA)


/** FileName : flat_timerN.h    .
 *
 * NTAF struct  name : _st_timerNdata 
 * NTAM struct  name : stFlat__st_timerNdata 
 * NTAF typedef name : stTIMERNDATA
 * NTAM typedef name : stFlat_stTIMERNDATA
 **/
typedef struct stFlat__st_timerNdata  {
	stFlat_stTIMERNINFO 	*pstTIMERNINFO;	/**< stTIMERNINFO 	*pstTIMERNINFO */
	void (*invoke_func)(void*); 	/**< void (*invoke_func)(void*) */
	S32  arg; 	/**< S32  arg */
} stFlat_stTIMERNDATA ;
#define stFlat_stTIMERNDATA_SIZE sizeof(stFlat_stTIMERNDATA)

#pragma pack(0)

extern int DBInsert_stFlat_Short_stTIMERNDATA(int dCount, stFlat_Short_stTIMERNDATA *pstData, char *pszName, int *pdErrRow);
extern int DBInsert_stFlat_Short_stTIMERNINFO(int dCount, stFlat_Short_stTIMERNINFO *pstData, char *pszName, int *pdErrRow);
extern int DBInsert_stFlat_Short_stTIMERNKEY(int dCount, stFlat_Short_stTIMERNKEY *pstData, char *pszName, int *pdErrRow);
extern int DBInsert_stFlat_stTIMERNDATA(int dCount, stFlat_stTIMERNDATA *pstData, char *pszName, int *pdErrRow);
extern int DBInsert_stFlat_stTIMERNINFO(int dCount, stFlat_stTIMERNINFO *pstData, char *pszName, int *pdErrRow);
extern int DBInsert_stFlat_stTIMERNKEY(int dCount, stFlat_stTIMERNKEY *pstData, char *pszName, int *pdErrRow);

#pragma pack()

/** file : flat_timerN.h
 *     $Log: flat_timerN.h,v $
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
