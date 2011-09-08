#ifndef	__timerN_h__
#define	__timerN_h__
/**		file  timerN.h
 *      - hash header file
 *
 *     Copyright (c) 2006~ by Upresto Inc, Korea
 *     All rights reserved
 *
 *		$Id: timerN.h,v 1.2 2011/05/28 09:16:00 jjinri Exp $
 * 
 *     @Author      $Author: jjinri $
 *     @version     $Revision: 1.2 $
 *     @date        $Date: 2011/05/28 09:16:00 $
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

#ifndef __COMM_DEFINE_H__
#define __COMM_DEFINE_H__

/* �ʿ��� define �� ���� (����, �߰�, ������ �����ϴ�) 
	���� ����ڰ� ��������� ���� ���� - �ڽ��� ����ϴ� �͸� �����ؼ� ���� ��*/

//extern int debug_print(int dIndex, char *fmt, ...);
#if 1	// DEBUG : BY JUNE
//#include	<utillib.h>
//#define APPLOG
#endif


/* �⺻ define ���̴�. �Ʒ� ������ ������ �����ϴ�. 
	-- FRINTF�� LOG_LEVEL �� ����  */
#if defined(COMMERCIALLOG)
#define 	LOG_LEVEL           		4
#define 	LOG_BUG 	        		1
#define		FPRINTF(x,z...)   	 	{													\
				switch((int)x){																	\
					case LOG_BUG:		dAppLog(x,z);	break;							\
				}																			\
			}

#define		dAppLog(x,z...)   	 	{													\
				switch((int)x){																	\
					case LOG_BUG:		debug_print(x,z);	break;							\
				}																			\
			}
#endif

#if	!defined(COMMERCIALLOG) && !defined(APPLOG)
#define		FPRINTF   	 				fprintf			
//#define		FPRINTF   	 				dAppLog
#endif

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
			/* ����� ó�� �Ǳ� ���ؼ��� #pragma pack(1)���� �����ؾ���. */					\
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




/* code gen���� �ڵ����� ���ǵǴ� type��.
*/
#ifndef __TYPEDEF_H__
#define __TYPEDEF_H__
#define		 DEF   	 long
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
#define STG_DEF_stTIMERNINFO		101		/* Hex( 36f10 ) */
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
#define STG_DEF_stTIMERNKEY		102		/* Hex( 36f10 ) */
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
#define STG_DEF_stTIMERNDATA		103		/* Hex( 36f10 ) */
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
#define		stTIMERNDATA_DEF_NUM						103			/* Hex ( 21fd0c ) */
#define		stTIMERNKEY_DEF_NUM							102			/* Hex ( 223fbc ) */
#define		stFlat_stTIMERNDATA_DEF_NUM					103			/* Hex ( 21fd18 ) */
#define		stFlat_stTIMERNKEY_DEF_NUM					102			/* Hex ( 223fc8 ) */
#define		stTIMERNINFO_DEF_NUM						101			/* Hex ( 21bfe0 ) */
#define		stFlat_stTIMERNINFO_DEF_NUM					101			/* Hex ( 21bfec ) */




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
 *     Revision 1.2  2011/05/28 09:16:00  jjinri
 *     debug FPRINTF ����
 *
 *     Revision 1.1.1.1  2011/04/19 14:13:48  june
 *     ���� ��Ű��
 *
 *     Revision 1.1.1.1  2011/01/20 12:18:55  june
 *     DSC CVS RECOVERY
 *
 *     Revision 1.4  2009/06/28 15:47:06  june
 *     *** empty log message ***
 *
 *     Revision 1.2  2009/06/26 16:51:50  dsc
 *     cgalib header �߰�
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
