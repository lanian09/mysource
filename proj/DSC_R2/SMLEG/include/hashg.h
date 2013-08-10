#ifndef	__hashg_h__
#define	__hashg_h__
/**		file  hashg.h
 *      - hash header file
 *
 *     Copyright (c) 2006~ by Upresto Inc, Korea
 *     All rights reserved
 *
 *		$Id: hashg.h,v 1.2 2011/05/28 09:16:00 jjinri Exp $
 * 
 *     @Author      $Author: jjinri $
 *     @version     $Revision: 1.2 $
 *     @date        $Date: 2011/05/28 09:16:00 $
 *     @warning     $type???�� ���ǵ� �͵鸸 ��밡��
 *     @ref         hashg.h
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

//#include <utillib.h>


/* �⺻ define ���̴�. �Ʒ� ������ ������ �����ϴ�. 
	-- FRINTF�� LOG_LEVEL �� ����  */
#if defined(COMMERCIALLOG)
#define 	LOG_LEVEL           		4
#define 	LOG_BUG 	        		1
#define		FPRINTF(x,z...)   	 	{													\
				switch((int)x){																	\
					case LOG_BUG:		debug_print(x,z);	break;							\
				}																			\
			}

#define		dAppLog(x,z...)   	 	{													\
				switch((int)x){																	\
					case LOG_BUG:		debug_print(x,z);	break;							\
				}																			\
			}
#endif

#if defined(APPLOG)
#define 	LOG_LEVEL           		4
#define 	LOG_BUG 	        		1
#endif

#if	!defined(COMMERCIALLOG) && !defined(APPLOG)
//#define		FPRINTF   	 				dAppLog			
#define		FPRINTF   	 				fprintf			
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
 * @brief stHASHGNODE : hash���� node���� structure.
 *
 * �� HASH�� doubly linked list�� �����ȴ�. 
 * Ȯ���� �����ϰ� �ϱ� ���ؼ� key�� data �κ��� �и� �Ͽ���.
 *
 * @see hash.h 
 *
 *  @note 		+Note+ 
 *
 */
typedef struct _st_hashgnode {
	U8 *pstHASHGINFO;				/**< ���� ����� node�̸� hash���� �̵��� �����ϰ� �ϱ� ���ؼ�*/ 
    struct _st_hashgnode *next  ;  	/**< self-pointer */ 
    struct _st_hashgnode **prev; 	/**< self-pointer */
    U8 *pstKey;       /**< Key  Structure Pointer */
    U8 *pstData;      /**< Data Structure Pointer */
} stHASHGNODE;
#define STG_DEF_stHASHGNODE		101		/* Hex( 36f10 ) */
#define stHASHGNODE_SIZE sizeof(stHASHGNODE)



/** 
 *  @brief stHASHGINFO : ���� hash���� ����ϴ� node���� �����ϴ� structure�̴�. 
 *
 * ���� hash���� ����ϴ� node���� �����ϴ� structure�̴�. 
 * key�� size�� key�ȿ��� sort�ϱ� ���� �κи��� ����  size�� �����Ͽ���.
 * sortkeylen�� key�� �� �κи��� ����Ų��. 
 *
 * @see hash.h 
 *
 *  @note 		hash node���� ������ �������ش�. 
 */
typedef struct _st_hashginfo {
    stHASHGNODE *psthashnode  ;  /**< HASH Node array�� pointer*/ 
    U16 usKeyLen;           /**< Node���� ����� Key ����.  Key �񱳿� copy�� ���ؼ� ���  */ 
    U16 usDataLen;          /**< Node���� ����� DataLen
                                 @exception pstData�� Structure�� type�� �ܺο����� �˸� �ȴ�. */
    U32 uiHashSize;         /**< Hash ũ��. ������ ������ ���ؼ� set Hash Node Array�� ũ��  */
	U32	MaxNodeCnt;			/**< Max Node Count */
	U32 (*func)(void*,U8*);     /**< stHASHGINFO* , stKey* : hash_function pointer - init���� set*/ 
	S32 (*print_key)(S8*,S8*,S32);     /**< stHASHGINFO* , stKey* : hash_function pointer - init���� set*/ 
	U32	uiLinkedCnt;		/**< link�� �Ŵ޸� node�� ���� */ 
} stHASHGINFO;
#define STG_DEF_stHASHGINFO		102		/* Hex( 36f10 ) */
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
#define		stFlat_stHASHGINFO_DEF_NUM					102			/* Hex ( 224224 ) */
#define		stHASHGINFO_DEF_NUM							102			/* Hex ( 224218 ) */
#define		stHASHGNODE_DEF_NUM							101			/* Hex ( 215aa4 ) */
#define		stFlat_stHASHGNODE_DEF_NUM					101			/* Hex ( 215ab0 ) */




/* Define.  MEMBER_CNT(struct���� member���Ǽ� : flat����)
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
#endif	/* __hashg_h__*/
