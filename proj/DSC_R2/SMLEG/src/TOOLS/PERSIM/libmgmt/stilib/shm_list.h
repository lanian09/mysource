/*
 * Copyright 2004, albam, albamc@gmail.com
 * Licensed under the AGPL. (== DO NOT USE or MOD MY SOURCES)
 * 
 * shm_list.h: Shared memory based list management 
 * 
 * How it works:
 *  ptr is shared memory root pointer 
 *      (one large shared memory has all of list entry)
 *  shm_list_head is list pointer structure 
 *      (member (prev, next) is offset value from ptr)
 *
 * Changes:
 * 2004/10/13 - albam <albamc@gmail.com>
 *      make first version.
 *
 */

#ifndef _SHM_LIST_H
#define _SHM_LIST_H

typedef struct shm_list_head {
	int prev, next;
} shm_list_head_t;

#define SHM_LIST_HEAD_INIT(name, num) { num, num }

#define SHM_LIST_HEAD(name, num) \
	shm_list_head_t name = SHM_LIST_HEAD_INIT(name, num)

#define INIT_SHM_LIST_HEAD(ptr, num) do { \
	(ptr)->next = num; (ptr)->prev = num; \
} while(0)

#define _pt_offset(base, ptr) \
	(((void*)ptr)-((void*)base))

static __inline__ void __shm_list_add(void* ptr
	, shm_list_head_t* new
	, shm_list_head_t* prev
	, shm_list_head_t* next)
{
	next->prev = ((void*)new) - ptr;
	new->next = ((void*)next) - ptr;
	new->prev = ((void*)prev) - ptr;
	prev->next = ((void*)new) - ptr;		
}

static __inline__ void shm_list_add(void* ptr
	, shm_list_head_t* new
	, shm_list_head_t* head)
{
	__shm_list_add(ptr, new, head, (shm_list_head_t*)(ptr+head->next));	
}

static __inline__ void shm_list_add_tail(void* ptr
	, shm_list_head_t* new
	, shm_list_head_t* head)
{
	__shm_list_add(ptr, new, head, (shm_list_head_t*)(ptr+head->prev));	
}

static __inline__ void __shm_list_del(void* ptr
	, shm_list_head_t* prev 
	, shm_list_head_t* next)
{
	next->prev = ((void*)prev) - ptr;
	prev->next = ((void*)next) - ptr;
}

static __inline__ void shm_list_del(void* ptr, shm_list_head_t* ent)
{
	__shm_list_del(ptr, (shm_list_head_t*)(ptr+ent->prev)
			, (shm_list_head_t*)(ptr+ent->next));
}

static __inline__ void shm_list_del_init(void* ptr, shm_list_head_t* ent)
{
	__shm_list_del(ptr, (shm_list_head_t*)(ptr+ent->prev)
			, (shm_list_head_t*)(ptr+ent->next));
	INIT_SHM_LIST_HEAD(ent, (((void*)ent)-ptr));
}

static __inline__ void shm_list_move(void* ptr
	, shm_list_head_t* ent
	, shm_list_head_t* head)
{
	__shm_list_del(ptr, (shm_list_head_t*)(ptr+ent->prev)
			, (shm_list_head_t*)(ptr+ent->next));
	shm_list_add(ptr, ent, head);
}

static __inline__ void shm_list_move_tail(void* ptr
	, shm_list_head_t* ent
	, shm_list_head_t* head)
{
	__shm_list_del(ptr, (shm_list_head_t*)(ptr+ent->prev)
			, (shm_list_head_t*)(ptr+ent->next));
	shm_list_add_tail(ptr, ent, head);
}

static __inline__ int shm_list_empty(void* ptr, const shm_list_head_t* head)
{
	return (head->next == ((void*)head) - ptr);
}

#define shm_list_entry(ptr, type, member) \
	((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))
	
#define shm_list_for_each(ptr, pos, head) \
	for (pos = (shm_list_head_t*)(((void*)ptr)+(head)->next); pos != (head); \
		pos = (shm_list_head_t*)(((void*)ptr)+(pos)->next))

#define shm_list_for_each_reverse(ptr, pos, head) \
	for (pos = (shm_list_head_t*)(((void*)ptr)+(head)->prev); pos != (head); \
		pos = (shm_list_head_t*)(((void*)ptr)+(pos)->prev))

#define shm_list_for_each_safe(ptr, pos, n, head) \
	for (pos = (shm_list_head_t*)(((void*)ptr)+(head)->next) \
		, n = (shm_list_head_t*)(((void*)ptr)+(pos)->next); pos != (head); \
		pos = n, n = (shm_list_head_t*)(((void*)ptr)+(pos)->next))

#define shm_list_for_each_reverse_safe(ptr, pos, n, head) \
	for (pos = (shm_list_head_t*)(((void*)ptr)+(head)->prev) \
		, n = (shm_list_head_t*)(((void*)ptr)+(pos)->prev); pos != (head); \
		pos = n, n = (shm_list_head_t*)(((void*)ptr)+(pos)->prev))

#define shm_list_for_each_entry(ptr, pos, head, member) \
	for (pos = shm_list_entry(((shm_list_head_t*)(((void*)ptr)+(head)->next)) \
		, typeof(*pos), member); &pos->member != (head); \
		pos = shm_list_entry((shm_list_head_t*)(((void*)ptr)+pos->member.next) \
		, typeof(*pos), member))
		
#define shm_list_for_each_entry_reverse(ptr, pos, head, member) \
	for (pos = shm_list_entry((shm_list_head_t*)(((void*)ptr)+(head)->prev) \
		, typeof(*pos), member); &pos->member != (head); \
		pos = shm_list_entry((shm_list_head_t*)(((void*)ptr)+pos->member.prev) \
		, typeof(*pos), member))
		
#define shm_list_for_each_entry_safe(ptr, pos, n, head, member) \
	for (pos = shm_list_entry((shm_list_head_t*)(((void*)ptr)+(head)->next) \
		, typeof(*pos), member), \
		n = shm_list_entry((shm_list_head_t*)(((void*)ptr)+pos->member.next) \
			, typeof(*pos), member); \
		&pos->member != (head); \
		pos = n, n = shm_list_entry((shm_list_head_t*)(((void*)ptr)+n->member.next) \
			, typeof(*n), member))
	
#endif

