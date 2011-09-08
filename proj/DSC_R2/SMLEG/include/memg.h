#ifndef	__memg_h__
#define	__memg_h__
/**		file  memg.h
 *      - hash header file
 *
 *     Copyright (c) 2006~ by Upresto Inc, Korea
 *     All rights reserved
 *
 *		$Id: memg.h,v 1.3 2011/05/28 09:16:00 jjinri Exp $
 * 
 *     @Author      $Author: jjinri $
 *     @version     $Revision: 1.3 $
 *     @date        $Date: 2011/05/28 09:16:00 $
 *     @warning     $type???�� ���ǵ� �͵鸸 ��밡��
 *     @ref         memg.h
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


#ifndef __DEFINE_H__
#define __DEFINE_H__

#pragma pack(1)
/* �ʿ��� define �� ���� (����, �߰�, ������ �����ϴ�) 
	���� ����ڰ� ��������� ���� ���� - �ڽ��� ����ϴ� �͸� �����ؼ� ���� ��*/

//extern int debug_print(int dIndex, char *fmt, ...);



/* �⺻ define ���̴�. �Ʒ� ������ ������ �����ϴ�. 
	-- FRINTF�� LOG_LEVEL �� ����  */
//#include <utillib.h>
#define 	LOG_LEVEL           		stderr
#define 	LOG_BUG   	        		stderr
//#define		FPRINTF   	 				dAppLog			
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
	node0 [label = "<f0> stMEMGINFO | <f1> header\ room | <f2> memg_node_hdr | <f3> memg_node_data | ... | memg_node_hdr | memg_node_data",height = 2.5];
}
\enddot
 */

#define MEMG_OFFSET(INFO,ptr)  ((OFFSET) (((U8 *) (ptr)) - ((U8 *) INFO)) )
#define MEMG_PTR(INFO,offset)  ((U8 *) (((OFFSET) (offset)) + ((OFFSET) INFO)) ) 

#define 	MEMG_MAIN_MEM		1
#define 	MEMG_SHARED_MEM		2

#define MEMG_MAX_DEBUG_STR	11
#define MEMG_ID				0x10101010
#define MEMG_FREE			0
#define MEMG_ALLOCED		1
/** 
 * @brief stMEMGNODE : mem���� node���� structure.
 *
 * �� MEM�� doubly linked list�� �����ȴ�. 
 * �տ��� mem node header�� ���� �ǰ� �ڿ� mem node body�� ���� �ȴ�.  
 * mem node header + body = mem node �� �Ǵ� ���̴�.  
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
 *  @note 		 Hdr�ڿ� Body�� ���� �� ���̸�, Body�κ��� ���� �������� ����ϰ� �� ���̴�.
 *
 */
typedef struct _st_memgnodehdr {
	U32	uiID;			/**< MEMG_ID : mem debug�� ���� �κ� : free�� Ȯ�� */ 
	STIME	TimeSec;	/**< Debug�� garbage collection�� ���� �ð� ���� */
	U8	ucIsFree;			/**< Free:0 , Alloc : 1 */
	S8	DebugStr[MEMG_MAX_DEBUG_STR];	/**< debugging�� ���ؼ� ���� */
} stMEMGNODEHDR;
#define STG_DEF_stMEMGNODEHDR		101		/* Hex( 0x65 ) */
#define stMEMGNODEHDR_SIZE sizeof(stMEMGNODEHDR)



/** 
 *  @brief stMEMGINFO : ���� memg���� ����ϴ� memory�� �����ϴ� structure�̴�. 
 *
 *
 * @see memg.h 
 *
 *  @note 	����ؾ� �� memory�� ���� ��ü���� ����.
 */
typedef struct _st_memginfo {
    U32 uiType;		/**<  Main Memory = 1 , Shared Memory = 2  */
    U32 uiShmKey;		/**<  uiType ==  Shared Memory : shared mem Key */
    U32 uiTotMemSize;		/**< ��ü ��밡���� memory (alloc�Ǵ� shared mem size */
    U32 uiHeadRoomSize;		/**< ������ node���� ���̱� ���� �� ���������� �ٸ� ������ ����ϱ� ���� ���� */
    U32 uiMemNodeHdrSize; 	/**< sizeof(stMEMGNODEHDR) */
    U32 uiMemNodeBodySize; 	/**< Node���� hdr�ڿ� �ٴ� Body�� Size :\n�� Body�� Size�� ������ ���ϴ� Size�� �� ���� */
    U32 uiMemNodeAllocedCnt;	/**< �̹� alloc�Ǿ��� ���� */
    U32 uiMemNodeTotCnt;		/**< ��ü ����� ���� */
    OFFSET offsetHeadRoom;			/**< HeadRoom�� ��ġ */
    OFFSET offsetNodeStart;		/**< Node���� ó�� ��ġ */
    OFFSET offsetFreeList;			/**< Free�� Node�� ��ġ */
    OFFSET offsetNodeEnd;			/**< Node�� ������ Offset */
} stMEMGINFO;
#define STG_DEF_stMEMGINFO		102		/* Hex( 0x66 ) */
#define stMEMGINFO_SIZE sizeof(stMEMGINFO)



/**
 * memg project : External Functions.
 */
extern int memg_dump_DebugString(char *debug_str,char *s,int len);
extern stMEMGINFO * memg_init(U32 uiType,U32 uiShmKey,U32 uiHeadRoomSize, U32 uiMemNodeBodySize,U32 uiMemNodeTotCnt, U8 ucResetFlag);
extern void memg_reset(stMEMGINFO *pstMEMGINFO, size_t base);
extern U8 * memg_alloc(stMEMGINFO *pstMEMGINFO , U32 uiSize, S8 *pDbgPtr);
extern S32 memg_free(stMEMGINFO *pstMEMGINFO , U8 *pFree);
extern void memg_print_info(S8 *pcPrtPrefixStr,stMEMGINFO *pstMEMGINFO);
extern void memg_print_node(S8 *pcPrtPrefixStr,stMEMGNODEHDR *pstMEMGNODEHDR);
extern void memg_print_all(S8 *pcPrtPrefixStr,stMEMGINFO *pstMEMGINFO);
extern void memg_draw_all(S8 *filename,S8 *labelname,stMEMGINFO *pstMEMGINFO);




/* Define.  DEF_NUM(type definition number)
*/
#define		stFlat_stMEMGNODEHDR_DEF_NUM				101			/* Hex ( 0x65 ) */
#define		stMEMGNODEHDR_DEF_NUM						101			/* Hex ( 0x65 ) */
#define		stMEMGINFO_DEF_NUM							102			/* Hex ( 0x66 ) */
#define		stFlat_stMEMGINFO_DEF_NUM					102			/* Hex ( 0x66 ) */




/* Define.  MEMBER_CNT(struct���� member���Ǽ� : flat����)
*/
#define		Short_stMEMGINFO_MEMBER_CNT					12
#define		Short_stMEMGNODEHDR_MEMBER_CNT				4
#define		stFlat_Short_stMEMGINFO_MEMBER_CNT			12
#define		stFlat_Short_stMEMGNODEHDR_MEMBER_CNT		4
#define		stFlat_stMEMGINFO_MEMBER_CNT				12
#define		stFlat_stMEMGNODEHDR_MEMBER_CNT				4
#define		stMEMGINFO_MEMBER_CNT						12
#define		stMEMGNODEHDR_MEMBER_CNT					4




/* Extern Function Define.
*/
extern void stMEMGINFO_CILOG(FILE *fp, stMEMGINFO *pthis);
extern void stMEMGINFO_Dec(stMEMGINFO *pstTo , stMEMGINFO *pstFrom);
extern void stMEMGINFO_Enc(stMEMGINFO *pstTo , stMEMGINFO *pstFrom);
extern void stMEMGINFO_Prt(S8 *pcPrtPrefixStr, stMEMGINFO *pthis);
extern void stMEMGNODEHDR_CILOG(FILE *fp, stMEMGNODEHDR *pthis);
extern void stMEMGNODEHDR_Dec(stMEMGNODEHDR *pstTo , stMEMGNODEHDR *pstFrom);
extern void stMEMGNODEHDR_Enc(stMEMGNODEHDR *pstTo , stMEMGNODEHDR *pstFrom);
extern void stMEMGNODEHDR_Prt(S8 *pcPrtPrefixStr, stMEMGNODEHDR *pthis);


#pragma pack()

/** file : memg.h
 *     $Log: memg.h,v $
 *     Revision 1.3  2011/05/28 09:16:00  jjinri
 *     debug FPRINTF ����
 *
 *     Revision 1.2  2011/04/27 14:30:40  jjinri
 *     *** empty log message ***
 *
 *     Revision 1.1.1.1  2011/04/19 14:13:48  june
 *     ���� ��Ű��
 *
 *     Revision 1.1.1.1  2011/01/20 12:18:56  june
 *     DSC CVS RECOVERY
 *
 *     Revision 1.2  2009/06/26 16:51:50  dsc
 *     cgalib header �߰�
 *
 *     Revision 1.1.1.1  2009/04/06 13:02:06  june
 *     LGT DSC project init
 *
 *     Revision 1.1.1.1  2009/04/06 09:10:25  june
 *     LGT DSC project start
 *
 *     Revision 1.1.1.1  2008/12/30 02:32:33  upst_cvs
 *     BSD R3.0.0
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
#endif	/* __memg_h__*/
