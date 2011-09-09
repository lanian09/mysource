#ifndef NTP_H
#define NTP_H

#include "__time.h"

typedef struct ntph {
    struct ntph *next;
    struct ntph **prev;
    unsigned int addr;
    int             poll;
    unsigned int    sec;
    unsigned int    fix;
} ntph_t;

void prnntp(ntp_t *np,struct sockaddr_in sin,int dir);
int sndntp(int fd,char *host);
void *ntpstask(void *arg);

#endif
