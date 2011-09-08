#ifndef	__timerN_h__
#define	__timerN_h__
/**		file  timerN.h
 *      - hash header file
 *
 *     Copyright (c) 2006~ by Upresto Inc, Korea
 *     All rights reserved
 *
 *		$Id: timerN.h,v 1.3 2011/08/19 06:35:02 dcham Exp $
 * 
 *     @Author      $Author: dcham $
 *     @version     $Revision: 1.3 $
 *     @date        $Date: 2011/08/19 06:35:02 $
 *     @warning     $type???�� ���ǵ� �͵鸸 ��밡��
 *     @ref         timerN.h
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
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netinet/in.h>
#include	<time.h>

#include <commdef.h>
#include <typedef.h>


/** @page TimergWorkFlow
\dot
 digraph example {
 fontname=Helvetica;
 label="Work Flow";
 rankdir=LR;
 node [shape=record, fontname=Helvetica, fontsize=10,style=rounded];
 sturctg [ label="structg" URL="http://192.203.140.245/aaa/cjlee/structg/STRUCTG/html"];
 hashg [ label="hashg" URL="http://192.203.140.245/aaa/cjlee/hashg/HASHG/html"];
 memg [ label="memg" URL="http://192.203.140.245/aaa/cjlee/memg/MEMG/html"];
 hasho [ label="hasho" URL="http://192.203.140.245/aaa/cjlee/hasho/HASHO/html"];
 timerg [ label="timerg" URL="http://192.203.140.245/aaa/cjlee/timerg/TIMTERG/html"];
 timerN [ label="timerN" URL="http://192.203.140.245/aaa/cjlee/timerN/TIMTERN/html"];

 structg -> hashg [label="Define the TYPEDEF of hashg", arrowhead="normal", style="dashed", fontname=Helvetica, fontsize=10];
 structg -> memg [label="Define the TYPEDEF of memg", arrowhead="normal", style="dashed", fontname=Helvetica, fontsize=10];
 structg -> hasho [label="Define the TYPEDEF of hasho", arrowhead="normal", style="dashed", fontname=Helvetica, fontsize=10];
 structg -> timerg [label="Define the TYPEDEF of timerg", arrowhead="normal", style="dashed", fontname=Helvetica, fontsize=10];
 structg -> timerN [label="Define the TYPEDEF of timerN", arrowhead="normal", style="dashed", fontname=Helvetica, fontsize=10];
 memg -> hasho [label="apply for \n the offset definition\nmemory management", arrowhead="normal", style="dashed", fontname=Helvetica, fontsize=10];
 hashg -> timerN [label="Time Table Management", arrowhead="normal", style="dashed", fontname=Helvetica, fontsize=10];
 hasho -> timerg [label="Time Table Management", arrowhead="normal", style="dashed", fontname=Helvetica, fontsize=10];
}
\enddot
*/


#define TIMERNID		U64

/** 
 * @brief stTIMERNINFO : timer ���� 
 *
 *
 * @see timerN.h 
 *
 *  @note 		timer node���� ������ �������ش�. 
 */
typedef struct _st_timerNinfo {
    void  *pstHASHGINFO  ;  	/**< TIMER Node array�� pointer (CurrentTime���κ��� 1�� �̳��� timeout���� �־�д�.) */ 
    U32 uiMaxNodeCnt;           /**< �޸��� �ִ� NODE�� MAX*/ 
    U32 uiNodeCnt;           	/**< Node���� �� */ 
	U32 uiArgMaxSize;			/**< invoke function�� argument�� size�߿��� �ִ밪 (�̸�ŭ alloc�� ���ѵӴϴ�.)*/ 
	U32	uiTimerNIdIndex;		/**< 1�� �����ϸ鼭 index�� ���� */ 
	STIME	uiCurrentTime;			/**< ������ �ð� */ 
} stTIMERNINFO;
#define STG_DEF_stTIMERNINFO		101		/* Hex( a8801e0 ) */
#define stTIMERNINFO_SIZE sizeof(stTIMERNINFO)


/** 
 * @brief stTIMERNKEY :  timer hash key structure
 *
 * Hash���� ����� timer�� key�̴�. (�ð� : �ʰ� �� ���̴�. ) 
 *
 * @see 	timerN.h 
 *
 * @note 	���� key�� �ø��� �ְ� �� ���̴�. 
 *
 */
typedef struct _st_timerNkey {
	U32	uiTimerNIdIndex;		/**< 1�� �����ϸ鼭 index�� ���� */ 
	STIME 	sTimeKey;		/**< tTimeKey : �ʴ����� hash�� ���� ���̴�.  (�Ϸ� �з�) */ 
} stTIMERNKEY;
#define STG_DEF_stTIMERNKEY		102		/* Hex( a8801e0 ) */
#define stTIMERNKEY_SIZE sizeof(stTIMERNKEY)



#define TIMERN_ARG_MAX_SIZE		100
/** 
 * @brief stTIMERNDATA :  timer hash DATA structure
 *
 * Hash���� ����� timer�� DATA �̴�. 
 *
 * @see 	timerN.h 
 *
 * @note 	���� key�� �ø��� �ְ� �� ���̴�. 
 *
 */
typedef struct _st_timerNdata {
	stTIMERNINFO 	*pstTIMERNINFO;	/**< Timer ���� ����*/ 
	void (*invoke_func)(void*);     /**< stTIMERNINFO* , stKey* : timer_function pointer - init���� set*/ 
	S32  arg; 				/**< usage : pstDATA = (stDATA *) &(...->arg)*/ 
} stTIMERNDATA;
#define STG_DEF_stTIMERNDATA		103		/* Hex( a8801e0 ) */
#define stTIMERNDATA_SIZE sizeof(stTIMERNDATA)




S32 timerN_print_key(S8 *pcPrtPrefixStr,S8 *s,S32 len);
int timerN_dump_DebugString(char *debug_str,char *s,int len);
U32 timerN_timeout_func(void *pa,U8 *pb);
stTIMERNINFO * timerN_init(U32 uiMaxNodeCnt,U32 uiArgMaxSize);
TIMERNID timerN_add(stTIMERNINFO *pstTIMERNINFO,void (*invoke_func)(void*),U8 *pArg,U32 uiArgSize,time_t timeout);
void timerN_del(stTIMERNINFO *pstTIMERNINFO,TIMERNID timerN_id);
TIMERNID timerN_update(stTIMERNINFO *pstTIMERNINFO,U64 timerNid,time_t timeout);
void timerN_print_info(S8 *pcPrtPrefixStr,stTIMERNINFO *pstTIMERNINFO);
void timerN_print_nodekey(S8 *pcPrtPrefixStr,stTIMERNINFO *pstTIMERNINFO,stTIMERNKEY *pstTIMERNNODEKEY);
void timerN_print_all(S8 *pcPrtPrefixStr,stTIMERNINFO *pstTIMERNINFO);
void timerN_draw_all(S8 *filename,S8 *labelname,stTIMERNINFO *pstTIMERNINFO);
void timerN_invoke(stTIMERNINFO *pstTIMERNINFO);




/* Define.  DEF_NUM(type definition number)
*/
#define		stTIMERNDATA_DEF_NUM						103			/* Hex ( aa2d790 ) */
#define		stTIMERNKEY_DEF_NUM							102			/* Hex ( aa2cf00 ) */
#define		stFlat_stTIMERNDATA_DEF_NUM					103			/* Hex ( aa2d7a0 ) */
#define		stFlat_stTIMERNKEY_DEF_NUM					102			/* Hex ( aa2cf10 ) */
#define		stTIMERNINFO_DEF_NUM						101			/* Hex ( aae4d20 ) */
#define		stFlat_stTIMERNINFO_DEF_NUM					101			/* Hex ( aae4d30 ) */




/* Define.  MEMBER_CNT(struct���� member���Ǽ� : flat����)
*/
#define		Short_stTIMERNDATA_MEMBER_CNT				1
#define		Short_stTIMERNINFO_MEMBER_CNT				5
#define		Short_stTIMERNKEY_MEMBER_CNT				2
#define		stFlat_Short_stTIMERNDATA_MEMBER_CNT		1
#define		stFlat_Short_stTIMERNINFO_MEMBER_CNT		5
#define		stFlat_Short_stTIMERNKEY_MEMBER_CNT			2
#define		stFlat_stTIMERNDATA_MEMBER_CNT				3
#define		stFlat_stTIMERNINFO_MEMBER_CNT				6
#define		stFlat_stTIMERNKEY_MEMBER_CNT				2
#define		stTIMERNDATA_MEMBER_CNT						3
#define		stTIMERNINFO_MEMBER_CNT						6
#define		stTIMERNKEY_MEMBER_CNT						2




/* Extern Function Define.
*/
extern void stTIMERNDATA_CILOG(FILE *fp, stTIMERNDATA *pthis);
extern void stTIMERNDATA_Dec(stTIMERNDATA *pstTo , stTIMERNDATA *pstFrom);
extern void stTIMERNDATA_Enc(stTIMERNDATA *pstTo , stTIMERNDATA *pstFrom);
extern void stTIMERNDATA_Prt(S8 *pcPrtPrefixStr, stTIMERNDATA *pthis);
extern void stTIMERNINFO_CILOG(FILE *fp, stTIMERNINFO *pthis);
extern void stTIMERNINFO_Dec(stTIMERNINFO *pstTo , stTIMERNINFO *pstFrom);
extern void stTIMERNINFO_Enc(stTIMERNINFO *pstTo , stTIMERNINFO *pstFrom);
extern void stTIMERNINFO_Prt(S8 *pcPrtPrefixStr, stTIMERNINFO *pthis);
extern void stTIMERNKEY_CILOG(FILE *fp, stTIMERNKEY *pthis);
extern void stTIMERNKEY_Dec(stTIMERNKEY *pstTo , stTIMERNKEY *pstFrom);
extern void stTIMERNKEY_Enc(stTIMERNKEY *pstTo , stTIMERNKEY *pstFrom);
extern void stTIMERNKEY_Prt(S8 *pcPrtPrefixStr, stTIMERNKEY *pthis);

#pragma pack()

/** file : timerN.h
 *     $Log: timerN.h,v $
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
 *     64bits �۾�
 *
 *     Revision 1.1.1.1  2008/06/09 08:17:19  jsyoon
 *     WATAS3 PROJECT START
 *
 *     Revision 1.1  2007/08/21 12:22:38  dark264sh
 *     no message
 *
 *     */
#endif	/* __timerN_h__*/
