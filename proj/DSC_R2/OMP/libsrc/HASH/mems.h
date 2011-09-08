#ifndef	__mems_h__
#define	__mems_h__
/**		file  mems.h
 *      - hash header file
 *
 *     Copyright (c) 2006~ by Upresto Inc, Korea
 *     All rights reserved
 *
 *		$Id: mems.h,v 1.1.1.1 2011/04/19 14:13:50 june Exp $
 * 
 *     @Author      $Author: june $
 *     @version     $Revision: 1.1.1.1 $
 *     @date        $Date: 2011/04/19 14:13:50 $
 *     @warning     $type???�� ���ǵ� �͵鸸 ��밡��
 *     @ref         mems.h
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

extern int debug_print(int dIndex, char *fmt, ...);



/* �⺻ define ���̴�. �Ʒ� ������ ������ �����ϴ�. 
	-- FRINTF�� LOG_LEVEL �� ����  */
#define 	LOG_LEVEL           		stderr
#define 	LOG_BUG   	        		stderr
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
	node0 [label = "<f0> stMEMSINFO | <f1> header\ room | <f2> memg_node_hdr | <f3> memg_node_data | ... | memg_node_hdr | memg_node_data",height = 2.5];
}
\enddot
 */

#define MEMS_OFFSET(INFO,ptr)  ((U32) (((U8 *) (ptr)) - ((U8 *) INFO)) )
#define MEMS_PTR(INFO,offset)  ((U8 *) (((S32) (offset)) + ((S32) INFO)) ) 

#define 	MEMS_MAIN_MEM		1
#define 	MEMS_SHARED_MEM		2

#define MEMS_MAX_DEBUG_STR	11
#define MEMS_ID				0x10101010

#define MEMS_FREE			0
#define MEMS_ALLOCED		1

#define MEMS_SEMA_ON		1
#define MEMS_SEMA_OFF		0
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
typedef struct _st_memsnodehdr {
	U32	uiID;								/**< MEMS_ID : mem debug�� ���� �κ� : free�� Ȯ�� */ 
	STIME	TimeSec;						/**< Debug�� garbage collection�� ���� �ð� ���� */
/**< <TAG_DEFINE:FREE_ALLOCED>
*	 MEMS_FREE			0
*	 MEMS_ALLOCED		1
*/
	U8 	ucIsFree;							/**< Free:0 , Alloc : 1 */
	S8	DebugStr[MEMS_MAX_DEBUG_STR];		/**< debugging�� ���ؼ� ���� CREATE */
	S8	DelStr[MEMS_MAX_DEBUG_STR];			/**< debugging�� ���ؼ� ���� DELETE */
} stMEMSNODEHDR;
#define STG_DEF_stMEMSNODEHDR		101		/* Hex( 0x65 ) */
#define stMEMSNODEHDR_SIZE sizeof(stMEMSNODEHDR)



/** 
 *  @brief stMEMSINFO : ���� memg���� ����ϴ� memory�� �����ϴ� structure�̴�. 
 *
 *
 * @see memg.h 
 *
 *  @note 	����ؾ� �� memory�� ���� ��ü���� ����.
 */
typedef struct _st_memsinfo {
/**< <TAG_DEFINE:MEM>
*	 	MEMS_MAIN_MEM		1
*	 	MEMS_SHARED_MEM		2
*/
	U32 	uiType;			/**<  Main Memory = 1 , Shared Memory = 2  */
    U32		uiShmKey;		/**<  uiType ==  Shared Memory : shared mem Key */
/**< <TAG_DEFINE:SEMA>
*	 MEMS_SEMA_ON		1
*	 MEMS_SEMA_OFF		0
*/
	U32		uiSemFlag;				/**< Semaphore ��� ���� */
	U32		uiSemKey;				/**< Semaphore Key �� */
	S32		iSemID;					/**< Semaphore ID �� */ 
	U32		uiReserved1;			/**< Padding for 64Bit Alignment */
    U64		uiTotMemSize;			/**< ��ü ��밡���� memory (alloc�Ǵ� shared mem size */
    U32		uiHeadRoomSize;			/**< ������ node���� ���̱� ���� �� ���������� �ٸ� ������ ����ϱ� ���� ���� */
    U32		uiMemNodeHdrSize;		/**< sizeof(stMEMSNODEHDR) */
    U32		uiMemNodeBodySize;		/**< Node���� hdr�ڿ� �ٴ� Body�� Size :\n�� Body�� Size�� ������ ���ϴ� Size�� �� ���� */
    U32		uiMemNodeAllocedCnt;	/**< �̹� alloc�Ǿ��� ���� */
    U32		uiMemNodeTotCnt;		/**< ��ü ����� ���� */
	U32		uiReserved2;			/**< Padding for 64Bit Alignment */
    OFFSET	offsetHeadRoom;			/**< HeadRoom�� ��ġ */
    OFFSET	offsetNodeStart;		/**< Node���� ó�� ��ġ */
    OFFSET	offsetFreeList;			/**< Free�� Node�� ��ġ */
    OFFSET	offsetNodeEnd;			/**< Node�� ������ Offset */
	U64		createCnt;
	U64		delCnt;
} stMEMSINFO;
#define STG_DEF_stMEMSINFO		102		/* Hex( 0x66 ) */
#define stMEMSINFO_SIZE sizeof(stMEMSINFO)



/**
 * memg project : External Functions.
 */
extern int mems_dump_DebugString(char *debug_str,char *s,int len);
extern stMEMSINFO *mems_init(U32 uiType,U32 uiShmKey, U32 uiSemFlag, U32 uiSemKey, U32 uiHeadRoomSize, U32 uiMemNodeBodySize,U32 uiMemNodeTotCnt);
extern U8 *mems_alloc(stMEMSINFO *pstMEMSINFO , U32 uiSize, U8 *pDbgPtr);
extern S32 mems_free(stMEMSINFO *pstMEMSINFO , U8 *pFree, U8 *pDbgPtr);
extern void mems_print_info(S8 *pcPrtPrefixStr,stMEMSINFO *pstMEMSINFO);
extern void mems_print_node(S8 *pcPrtPrefixStr,stMEMSNODEHDR *pstMEMSNODEHDR);
extern void mems_print_all(S8 *pcPrtPrefixStr,stMEMSINFO *pstMEMSINFO);
extern void mems_draw_all(S8 *filename,S8 *labelname,stMEMSINFO *pstMEMSINFO);
extern void mems_garbage_collector(S8 *pcPrtPrefixStr,stMEMSINFO *pstMEMSINFO,int timegap, void (*print_func)(stMEMSINFO *pmem, stMEMSNODEHDR *pmemhdr));
extern S32 mems_alloced_cnt(stMEMSINFO *pstMEMSINFO);
extern void mems_view(S8 *pcPrtPrefixStr, stMEMSINFO *pstMEMSINFO, int timegap, void (*print_func)(stMEMSINFO *pmem, stMEMSNODEHDR * pmemhdr));




/* Define.  DEF_NUM(type definition number)
*/
#define		stFlat_stMEMSNODEHDR_DEF_NUM				101			/* Hex ( 0x65 ) */
#define		stMEMSNODEHDR_DEF_NUM						101			/* Hex ( 0x65 ) */
#define		stFlat_stMEMSINFO_DEF_NUM					102			/* Hex ( 0x66 ) */
#define		stMEMSINFO_DEF_NUM							102			/* Hex ( 0x66 ) */




/* Define.  MEMBER_CNT(struct���� member���Ǽ� : flat����)
*/
#define		Short_stMEMSINFO_MEMBER_CNT					17
#define		Short_stMEMSNODEHDR_MEMBER_CNT				5
#define		stFlat_Short_stMEMSINFO_MEMBER_CNT			17
#define		stFlat_Short_stMEMSNODEHDR_MEMBER_CNT		5
#define		stFlat_stMEMSINFO_MEMBER_CNT				17
#define		stFlat_stMEMSNODEHDR_MEMBER_CNT				5
#define		stMEMSINFO_MEMBER_CNT						17
#define		stMEMSNODEHDR_MEMBER_CNT					5




/* Extern Function Define.
*/
extern void stMEMSINFO_CILOG(FILE *fp, stMEMSINFO *pthis);
extern void stMEMSINFO_Dec(stMEMSINFO *pstTo , stMEMSINFO *pstFrom);
extern void stMEMSINFO_Enc(stMEMSINFO *pstTo , stMEMSINFO *pstFrom);
extern void stMEMSINFO_Prt(S8 *pcPrtPrefixStr, stMEMSINFO *pthis);
extern void stMEMSNODEHDR_CILOG(FILE *fp, stMEMSNODEHDR *pthis);
extern void stMEMSNODEHDR_Dec(stMEMSNODEHDR *pstTo , stMEMSNODEHDR *pstFrom);
extern void stMEMSNODEHDR_Enc(stMEMSNODEHDR *pstTo , stMEMSNODEHDR *pstFrom);
extern void stMEMSNODEHDR_Prt(S8 *pcPrtPrefixStr, stMEMSNODEHDR *pthis);



#pragma pack()
/** file : mems.h
 *     $Log: mems.h,v $
 *     Revision 1.1.1.1  2011/04/19 14:13:50  june
 *     ���� ��Ű��
 *
 *     Revision 1.1.1.1  2011/01/20 12:18:56  june
 *     DSC CVS RECOVERY
 *
 *     Revision 1.1.1.1  2009/07/12 20:16:07  dsc
 *     LGT ��� init
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
 *     Revision 1.1.1.1  2008/12/30 02:32:26  upst_cvs
 *     BSD R3.0.0
 *
 *     Revision 1.2  2008/10/15 12:04:11  jsyoon
 *     MIF ������� ����
 *
 *     Revision 1.1  2008/10/12 22:48:34  jsyoon
 *     MIF ��� �߰�
 *
 *     Revision 1.1  2008/10/07 23:29:05  jsyoon
 *     *** empty log message ***
 *
 *     Revision 1.1.1.1  2008/06/09 08:17:19  jsyoon
 *     WATAS3 PROJECT START
 *
 *     Revision 1.1  2007/08/21 12:22:38  dark264sh
 *     no message
 *
 *     */
#endif	/* __mems_h__*/
