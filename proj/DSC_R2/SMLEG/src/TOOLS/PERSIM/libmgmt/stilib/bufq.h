#ifndef BUFQ_H
#define BUFQ_H

#pragma pack(1)

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/time.h>
#include <netinet/in.h>

/* structure of user-message-buffer (umsg)
 * it uses message-buffering systems
 */
struct umsg 
{
	struct umsg *prev;		/* previous umsg pointor		*/
	struct umsg *next;		/* next umsg pointer			*/
	time_t	ct	;			/* messae created timestamp		*/
	time_t	rt	;			/* messae reported timestamp	*/
	int 	n;				/* message length 				*/
	char 	*p;				/* message pointer				*/
	char	*prv;			/* private information			*/
};
typedef struct umsg umsg_t;

/* structure of message queue
 * it uses message-queueing systems
 */
struct bufq {
	umsg_t      *q_head;	/* queue header pointor			*/
	umsg_t      *q_tail;	/* queue tail pointor			*/
	size_t      q_msgs;		/* queue message bytes			*/
	size_t      q_count;	/* queue message counter		*/
};
typedef struct bufq bufq_t;

#define __ensure(__exp,__sta) \
do { if (!(__exp)) { __sta; } } while(0)

#define ensure(__exp,__sta)     __ensure(__exp,__sta)
#define unless(__exp,__sta)     __ensure(!(__exp),__sta)

#define ALLOCM(n) 		alloc_umsg(n)
#define COPYM(p,n1,n2) 	copy_umsg(p,n1,n2)
#define DUPM(p) 		dupl_umsg(p)
#define FREEM(p) 		free_umsg(p)

/* allocate user-message-buffer
 * returns Upon successful completion, not null pointer is returned
 * otherwise, NULL is return
 */ 
umsg_t* alloc_umsg(int n	/* number of byte */);

/* duplicate user-message-buffer
 * returns Upon successful completion, not null pointer is returned
 * otherwise, NULL is return
 */ 
umsg_t* dupl_umsg(struct umsg *p	/* will be duplicated user buffer */);

/* copy user-message-buffer
 * returns Upon successful completion, not null pointer is returned
 * otherwise, NULL is return 
 */ 
umsg_t* copy_umsg(	char *p,	/* will be copied user buffer */
					int n1,		/* total buffer size */
					int n2		/* copy buffer size */
				);

/* free user-message-buffer
 */ 
void free_umsg(struct umsg *mp		/* pointer of buffer for free */);

/* initialize bufq structure
 */ 
void bufq_init(bufq_t *q			/* pointer of umsg for initialize */);

/* get current queued message bytes
 * returns Upon successful completion, total occuppied message bytes
 * otherwise, 0 is return 
 */ 
size_t bufq_length(bufq_t* q	/* queue */);

/* get current queued message count
 * returns Upon successful completion, total occuppied message count
 * otherwise, 0 is return 
 */ 
size_t bufq_size(bufq_t* q	/* queue */);

/* get message-buffer from pointed queue header
 * returns Upon successful completion, message-buffer's pointor return
 * otherwise, NULL is return 
 */ 
umsg_t* bufq_head(bufq_t* q	/* queue */);

/* get message-buffer from pointed queue tail
 * returns Upon successful completion, message-buffer's pointor return
 * otherwise, NULL is return 
 */ 
umsg_t* bufq_tail(bufq_t* q	/* queue */);

/* attach message-buffer to pointed queue's header
 */ 
void bufq_queue(bufq_t* q,
				umsg_t *mp
				);

/* detach message-buffer to pointed queue's header
 * returns Upon successful completion, message-buffer's pointor return
 * otherwise, NULL is return 
 */ 
umsg_t* bufq_dequeue(bufq_t* q);

void bufq_queue_head(bufq_t* q,umsg_t *mp);
void bufq_insert(bufq_t* q,umsg_t *mp,umsg_t *np);
void bufq_append(bufq_t* q,umsg_t *mp,umsg_t *np);
umsg_t* bufq_unlink(bufq_t* q,umsg_t *mp);
void bufq_freehead(bufq_t* q);

/* free all message-buffer from pointed queue
 */ 
void bufq_purge(bufq_t* q	/* queue */);

void bufq_supply(bufq_t* q,umsg_t* mp);
umsg_t* bufq_resupply(bufq_t* q,umsg_t* mp,int maxsize,int maxcount);
void freechunks(umsg_t *mp);

#ifdef __cplusplus
}
#endif

#pragma pack()

#endif
