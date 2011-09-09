#ifndef	__clisto_h__
#define	__clisto_h__
/**		file  clisto.h
 *      - hash header file
 *
 *     Copyright (c) 2006~ by Upresto Inc, Korea
 *     All rights reserved
 *
 *		$Id: clisto.h,v 1.3 2011/08/19 06:34:49 dcham Exp $
 * 
 *     @Author      $Author: dcham $
 *     @version     $Revision: 1.3 $
 *     @date        $Date: 2011/08/19 06:34:49 $
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


/* 필요한 header file include */
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netinet/in.h>
#include	<time.h>


#include <commdef.h>
#include <typedef.h>


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
#define STG_DEF_clist_head		101		/* Hex( 531470 ) */
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
#define		clist_head_DEF_NUM							101			/* Hex ( 756850 ) */
#define		stFlat_clist_head_DEF_NUM					101			/* Hex ( 756820 ) */




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

#pragma pack()

/** file : clisto.h
 *     $Log: clisto.h,v $
 *     Revision 1.3  2011/08/19 06:34:49  dcham
 *     *** empty log message ***
 *
 *     Revision 1.1  2011/08/18 04:58:27  dcham
 *     *** empty log message ***
 *
 *     Revision 1.1  2011/07/26 06:13:03  dhkim
 *     *** empty log message ***
 *
 *     Revision 1.1.1.1  2011/01/11 01:33:02  jjinri
 *     DIFO
 *
 *     Revision 1.1  2010/12/02 12:50:37  upst_cvs
 *     2010.1202 commit start
 *
 *     Revision 1.2  2010/10/19 02:01:28  upst_cvs
 *     ..
 *
 *     Revision 1.1.1.1  2010/10/15 06:54:37  upst_cvs
 *     WTAS2
 *
 *     Revision 1.1  2010/07/21 04:39:06  upst_cvs
 *     CGALIB
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
#endif	/* __clisto_h__*/
