#ifndef	__clisto_h__
#define	__clisto_h__
/**		file  clisto.h
 *      - hash header file
 *
 *     Copyright (c) 2006~ by Upresto Inc, Korea
 *     All rights reserved
 *
 *		$Id: clisto.h,v 1.1.1.1 2011/04/19 14:13:48 june Exp $
 * 
 *     @Author      $Author: june $
 *     @version     $Revision: 1.1.1.1 $
 *     @date        $Date: 2011/04/19 14:13:48 $
 *     @warning     $type???로 정의된 것들만 사용가능
 *     @ref         clisto.h
 *     @todo        Makefile을 만들자
 *
 *     @section     Intro(소개)
 *      - hash header file
 *
 *     @section     Requirement
 *      @li 규칙에 틀린 곳을 찾아주세요.
 *
 **/


#pragma pack(1)

/* 필요한 header file include */
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netinet/in.h>
#include	<time.h>


#ifndef __DEFINE_H__
#define __DEFINE_H__

/* 필요한 define 문 선언 (삭제, 추가, 변경이 가능하다) 
	여러 사용자가 사용함으로 주의 요함 - 자신이 사용하는 것만 선언해서 쓰면 됨*/

extern int debug_print(int dIndex, char *fmt, ...);



/* 기본 define 문이다. 아래 값들은 변경은 가능하다. 
	-- FRINTF나 LOG_LEVEL 값 정도  */

#define 	LOG_LEVEL       stderr
#define 	LOG_BUG   	    stderr
#define		FPRINTF   	 	fprintf			
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
			/* 제대로 처리 되기 위해서는 #pragma pack(1)으로 선언해야함. */					\
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




/* code gen에서 자동으로 정의되는 type들.
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

#ifndef __CLISTO_H__
#define __CLISTO_H__


#define clisto_offset(infoptr, ptr)  (OFFSET) ((U8 *) (ptr==NULL?0:(U8 *)ptr-(U8 *)infoptr))
#define clisto_ptr(infoptr, offset)  (clist_head *)(U8 *) ((OFFSET) (offset==0?NULL:(OFFSET)offset+(U8 *)infoptr))

/*
 * Simple doubly linked list implementation.
 *
 * Some of the internal functions ("__xxx") are useful when
 * manipulating whole lists rather than single entries, as
 * sometimes we already know the next/prev entries and we can
 * generate better code by using them directly rather than
 * using the generic single-entry routines.
 */

typedef struct _clist_head {
	OFFSET	offset_next;
	OFFSET	offset_prev;
} clist_head;
#define STG_DEF_clist_head		101		/* Hex( 0x65 ) */
#define clist_head_SIZE sizeof(clist_head)


#define CLIST_HEAD_INIT(infoptr, name) { clisto_offset(infoprt, &(name)), clisto_offset(infoptr, &(name)) }

#define CLIST_HEAD(infoptr, name) \
	struct clist_head name = CLIST_HEAD_INIT(infoptr, name)

#define CINIT_LIST_HEAD(infoptr, ptr) do { \
	(ptr)->offset_next = clisto_offset(infoptr, ptr); (ptr)->offset_prev = clisto_offset(infoptr, ptr); \
} while (0)

/*
 * Insert a new entry between two known consecutive entries. 
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static __inline__ void __clist_add(void *infoptr, clist_head *new, clist_head *prev, clist_head *next)
{
	next->offset_prev = clisto_offset(infoptr, new);
	new->offset_next = clisto_offset(infoptr, next);
	new->offset_prev = clisto_offset(infoptr, prev);
	prev->offset_next = clisto_offset(infoptr, new);
}

/**
 * list_add - add a new entry
 * @param infoptr: pointer of base
 * @param new: new entry to be added
 * @param head: list head to add it after
 *
 * Insert a new entry after the specified head.
 * This is good for implementing stacks.
 */
static __inline__ void clist_add_head(void *infoptr, clist_head *new, clist_head *head)
{
	__clist_add(infoptr, new, head, clisto_ptr(infoptr, head->offset_next));
	// __clist_add(infoptr, new, clisto_ptr(infoptr, head->offset_prev), head);
	// return new;
}

/**
 * list_add_tail - add a new entry
 * @param infoptr: pointer of base
 * @param new: new entry to be added
 * @param head: list head to add it before
 *
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 */
static __inline__ void clist_add_tail(void *infoptr, clist_head *new, clist_head *head)
{
	__clist_add(infoptr, new, clisto_ptr(infoptr, head->offset_prev), head);
	// return head;
}

/*
 * Delete a list entry by making the prev/next entries
 * point to each other.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static __inline__ void __clist_del(void *infoptr, clist_head * prev, clist_head * next)
{
	next->offset_prev = clisto_offset(infoptr, prev);
	prev->offset_next = clisto_offset(infoptr, next);
}

/**
 * list_del - deletes entry from list.
 * @param infoptr: pointer of base
 * @param entry: the element to delete from the list.
 * Note: list_empty on entry does not return true after this, the entry is in an undefined state.
 */
static __inline__ void clist_del(void *infoptr, clist_head *entry)
{
	__clist_del(infoptr, clisto_ptr(infoptr, entry->offset_prev), clisto_ptr(infoptr, entry->offset_next));
	entry->offset_next = entry->offset_prev = 0;
}

/**
 * list_del_init - deletes entry from list and reinitialize it.
 * @param infoptr: pointer of base
 * @param entry: the element to delete from the list.
 */
static __inline__ void clist_del_init(void *infoptr, clist_head *entry)
{
	__clist_del(infoptr, clisto_ptr(infoptr, entry->offset_prev), clisto_ptr(infoptr, entry->offset_next));
	CINIT_LIST_HEAD(infoptr, entry); 
}

/**
 * list_empty - tests whether a list is empty
 * @param infoptr: pointer of base
 * @param head: the list to test.
 */
static __inline__ int clist_empty(void *infoptr, clist_head *head)
{
	return head->offset_next == clisto_offset(infoptr, head);
}

/**
 * list_splice - join two lists
 * @param infoptr: pointer of base
 * @param list: the new list to add.
 * @param head: the place to add it in the first list.
 */
static __inline__ void clist_splice(void *infoptr, clist_head *list, clist_head *head)
{
	clist_head *last = clisto_ptr(infoptr, list->offset_prev);
	clist_head *at = clisto_ptr(infoptr, head->offset_prev);

	head->offset_prev = clisto_offset(infoptr, last);
	last->offset_next = clisto_offset(infoptr, head);

	list->offset_prev = clisto_offset(infoptr, at);
	at->offset_next = clisto_offset(infoptr, list);
}

/**
 * list_entry - get the struct for this entry
 * @param ptr:	the &struct list_head pointer.
 * @param type:	the type of the struct this is embedded in.
 * @param member:	the name of the list_struct within the struct.
 */
#define clist_entry(ptr, type, member) \
	((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))

/**
 * list_for_each	-	iterate over a list
 * @param infoptr: pointer of base
 * @param pos:	the &struct list_head to use as a loop counter.
 * @param head:	the head for your list.
 */

#define clist_for_each_start(infoptr, pos, head) \
		pos = head; do

#define clist_for_each_end(infoptr, pos, head) \
		while((pos = clisto_ptr(infoptr, pos->offset_next)) != head); 

#endif




/* Define.  DEF_NUM(type definition number)
*/
#define		clist_head_DEF_NUM							101			/* Hex ( 0x65 ) */
#define		stFlat_clist_head_DEF_NUM					101			/* Hex ( 0x65 ) */




/* Define.  MEMBER_CNT(struct안의 member들의수 : flat기준)
*/
#define		Short_clist_head_MEMBER_CNT					2
#define		clist_head_MEMBER_CNT						2
#define		stFlat_Short_clist_head_MEMBER_CNT			2
#define		stFlat_clist_head_MEMBER_CNT				2




/* Extern Function Define.
*/
extern void clist_head_CILOG(FILE *fp, clist_head *pthis);
extern void clist_head_Dec(clist_head *pstTo , clist_head *pstFrom);
extern void clist_head_Enc(clist_head *pstTo , clist_head *pstFrom);
extern void clist_head_Prt(S8 *pcPrtPrefixStr, clist_head *pthis);

#pragma pack(0)


/** file : clisto.h
 *     $Log: clisto.h,v $
 *     Revision 1.1.1.1  2011/04/19 14:13:48  june
 *     성능 패키지
 *
 *     Revision 1.1.1.1  2011/01/20 12:18:56  june
 *     DSC CVS RECOVERY
 *
 *     Revision 1.1.1.1  2009/04/06 13:02:06  june
 *     LGT DSC project init
 *
 *     Revision 1.1.1.1  2009/04/06 09:10:25  june
 *     LGT DSC project start
 *
 *     Revision 1.1.1.1  2008/12/30 02:32:30  upst_cvs
 *     BSD R3.0.0
 *
 *     Revision 1.2  2008/10/15 12:04:11  jsyoon
 *     MIF 헤더파일 정리
 *
 *     Revision 1.1  2008/10/12 22:48:34  jsyoon
 *     MIF 헤더 추가
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
#endif	/* __clisto_h__*/
