#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <sys/poll.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>

#include "sock.h"
#include "__time.h"
#include "timer.h"

#define ICMP_HDRLEN     8
#define TIMEOF_DAYLEN   8
#define NUM_OF_PACKET   56

int buildicmp(char *s,short id,short seq,int n)
{
    struct icmp *icp;
	char *p;
	int i,len;

    icp = (struct icmp*)s;
    icp->icmp_type  = ICMP_ECHO;
    icp->icmp_code  = 0;
    icp->icmp_cksum = 0;
    icp->icmp_id    = htons(id);
    icp->icmp_seq   = htons(seq);

	p = (char*)(icp+1);
    gettimeofday((struct timeval*)p,(struct timezone*)0);

	p += TIMEOF_DAYLEN;
    for(i=TIMEOF_DAYLEN;i<n;i++) *p++ = '0'+i;
    len = ICMP_HDRLEN+n;
    icp->icmp_cksum = in_chksum((u_short*)icp,len);

	return len;
}

#define Line printf("%s:%d \n",__FILE__,__LINE__)

int wait_icmp_reply(int fd,short id,struct sockaddr_in *sin, int tsec,int usec)
{
	char s[BUFSIZ];
	struct pollfd pfd[1];
   	struct ip* ip;
   	struct icmp *icp;
	struct sockaddr_in rsin;
	struct timeb t1,t2;
	int msec = usec/1000;
	int len,remain,nbyte;

	ftime(&t2);
	remain = tsec*1000 + msec;
	t2.time 	+= tsec;
	t2.millitm 	+= msec;

	while(1) {
		pfd[0].fd       = fd;
		pfd[0].events   = POLLIN | POLLPRI;
		pfd[0].revents  = 0;

		switch (poll(pfd,1,remain)) {
		case -1 :
            break;
		case 0  :
			return 0;
		case 1	:
			if (pfd[0].revents & (POLLIN | POLLPRI | POLLERR | POLLHUP)) {
        		len = sizeof(rsin);
        		if((nbyte=recvfrom(fd,s,BUFSIZ,0,(struct sockaddr*)&rsin,(int*)&len))<=0) break;
                if(sin->sin_addr.s_addr!=rsin.sin_addr.s_addr) break;

        		ip  = (struct ip*)s;
        		icp = (struct icmp*)(ip+1);

        		if(icp->icmp_type==ICMP_ECHOREPLY && icp->icmp_id==htons(id)) {
					return 1;
				}
			}
		}
		ftime(&t1);
		remain = (t2.millitm-t1.millitm>0)? 
			(t2.time-t1.time)*1000+(t2.millitm-t1.millitm)
			:(t2.time-t1.time-1)*1000+(t2.millitm-t1.millitm+1000);
        if(remain<=0) return 0; 
	}
	return 0;
}

int
pingtest(char* addr,int npacket,int tsec,int usec)
{
    struct sockaddr_in sin;
    //struct ip* ip;
    //struct icmp *icp;
    //struct timeval tm;
    char s[BUFSIZ];
	unsigned short id;
    int fd,i;
	int ret=0, nbyte=0, len=0;

    if((fd=icmpsock()) < 0) return 0;
    getsaddrbyhost(addr,0,&sin);
	id = getpid() & 0xffff;

    for(i=0,nbyte=0;i<=npacket;i++) {
		len = buildicmp(s,id,i,NUM_OF_PACKET);
        nbyte += sendto(fd,s,len,0,(struct sockaddr*)&sin,sizeof(sin));
    }
	if(nbyte > 0) {
		ret = wait_icmp_reply(fd,id,&sin,tsec,usec);
	}
	close(fd);

	return ret;
}


/* sdfasd   */
int continuous;

char*
geticmptype(unsigned char type)
{
    switch(type) {
#ifdef Linux
    case ICMP_ECHOREPLY     : return (char*)"echo reply";
    case ICMP_DEST_UNREACH  : return (char*)"destination unreachable";
    case ICMP_SOURCE_QUENCH : return (char*)"source quench";
    case ICMP_REDIRECT      : return (char*)"redirect (change route)";
    case ICMP_ECHO          : return (char*)"echo request";
    case ICMP_TIME_EXCEEDED : return (char*)"time exceeded";
    case ICMP_PARAMETERPROB : return (char*)"parameter problem";
    case ICMP_TIMESTAMP     : return (char*)"timestamp request";
    case ICMP_TIMESTAMPREPLY: return (char*)"timestamp reply";
    case ICMP_INFO_REQUEST  : return (char*)"information request";
    case ICMP_INFO_REPLY    : return (char*)"information reply";
    case ICMP_ADDRESS       : return (char*)"address mask request";
    case ICMP_ADDRESSREPLY  : return (char*)"address mask reply";
#endif

#ifdef SunOS
    case ICMP_ECHOREPLY     : return (char*)"echo reply";
    case ICMP_UNREACH  		: return (char*)"unreachable";
    case ICMP_SOURCEQUENCH  : return (char*)"source quench";
    case ICMP_REDIRECT      : return (char*)"redirect (change route)";
    case ICMP_ECHO          : return (char*)"echo request";
    case ICMP_TIMXCEED 		: return (char*)"time exceeded";
    case ICMP_PARAMPROB 	: return (char*)"parameter problem";
    case ICMP_TSTAMP     	: return (char*)"timestamp request";
    case ICMP_TSTAMPREPLY	: return (char*)"timestamp reply";
    case ICMP_IREQ  		: return (char*)"information request";
    case ICMP_IREQREPLY    	: return (char*)"information reply";
    case ICMP_MASKREQ       : return (char*)"address mask request";
    case ICMP_MASKREPLY  	: return (char*)"address mask reply";
#endif
    }
    return (char*)"unknown";
}

int
pingcheck(char* addr)
{
    struct sockaddr_in sin;
    struct ip *ip=NULL;
    struct icmp *icp=NULL;
    //struct timeval tm;
    char s[BUFSIZ];
    int fd,i,nbyte=0,len,ret=0;
	unsigned short id;

    fd = icmpsock();
	if(fd<0) return 0;
    getsaddrbyhost(addr,0,&sin);
    id = getpid() & 0xffff;

    for(i=0;continuous;i++) {
		len = buildicmp(s,id,i,NUM_OF_PACKET);
        nbyte = sendto(fd,s,len,0,(struct sockaddr*)&sin,sizeof(sin));
	}
	if(nbyte > 0) {
		ret = wait_icmp_reply(fd,id,&sin, 1,0);
	}
	close(fd);

	if(!ret) {
		printf("No response from %s \n",addr);
	}
    else {
		printf("%d bytes from %s: icmp_seq=%d ttl=%d\n"
			,nbyte-20
			,addr
       		,htons(icp->icmp_seq)
			,ip->ip_ttl
		);
	}
    return ret;
}


