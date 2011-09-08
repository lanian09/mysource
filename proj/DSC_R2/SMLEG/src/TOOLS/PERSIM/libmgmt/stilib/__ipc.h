#ifndef IPC_H
#define IPC_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void  initq(const key_t key);
extern int   recvq(const key_t key, char *s);
extern int   sendq(const key_t key, char *s, int len);
extern char* memget(key_t key,int size);
extern int   memfree(key_t key);

#ifdef __cplusplus
}
#endif

#endif
