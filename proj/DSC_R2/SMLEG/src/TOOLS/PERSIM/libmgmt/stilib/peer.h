#ifndef PEER_H
#define PEER_H

#include <netinet/in.h>

typedef struct peer {
    struct peer 	**prev;
    struct peer 	*next;
    unsigned int    addr;
    time_t      	t;
    int         	flags;
    int         	ref;
    int         	stat;
} peer_t;

#define peer_hash_size  1

extern peer_t *prhash[peer_hash_size];

extern void (*peerupcbfn)(unsigned int);
extern void (*peerdncbfn)(unsigned int);


#endif
