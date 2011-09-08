#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>

#define ICMP_ECHO   8
#define ICMP_ECHOREPLY  0

#define ICMP_MIN    8


#define MAX_PING_TRANS_SEQ  65630
#define DEF_PACKET_SIZE		64
#define MAXWAIT             10
#define	MAX_PACKET			4096
#define PKTSIZ				(sizeof(struct ICMPHdr) + MAX_PACKET)

int		        pingsock;
unsigned int    ntransmitted = 1;
struct timezone tz;	/* leftover */

void decode_resp(char *buf, int bytes, struct sockaddr_in *from);
unsigned short check_sum(unsigned short *buff, int rsize);
void fill_icmp_data(char *data);


int	init_ping_socket()
{
	int					timeout = 1000;

	pingsock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if(pingsock < 0){
		printf("[ping_to_host] socket creation error - %s\n", strerror(errno));
		return -1;
	}

/*
	if(setsockopt(pingsock, SOL_SOCKET, SO_RCVTIMEO,
		         (char *)&timeout, sizeof(timeout))){
		printf("[ping_to_host] setsockopt SO_RCVTIMEO error - %s\n", strerror(errno));
		return -1;
	}

	timeout = 1000;
	if(setsockopt(pingsock, SOL_SOCKET, SO_SNDTIMEO,
		         (char *)&timeout, sizeof(timeout))){
		printf("[ping_to_host] setsockopt SO_SNDTIMEO error - %s\n", strerror(errno));
		return -1;
	}
*/

	return 0;
}

int send_ping_to_host(in_addr_t hostIP)
{
	int					i, sents, datalen = DEF_PACKET_SIZE;
	char				data[MAX_PACKET];
	struct icmp		    *picmp = (struct icmp *)data;
    struct     timeval *tp = (struct timeval *)&data[8];
    unsigned char      *ppayload = &data[8+sizeof(struct timeval)];
	struct sockaddr_in	host_addr;


    bzero((char *)&host_addr, sizeof(struct sockaddr));
	host_addr.sin_family = AF_INET;
	host_addr.sin_addr.s_addr = hostIP;

	memset(data, 0x00, sizeof(data));
	
    fill_icmp_data(data);

    gettimeofday(tp, &tz);

    for(i = 8; i < datalen-8; i++)
        *ppayload++ = i;

	picmp->icmp_cksum = check_sum((unsigned short *)picmp, datalen);
	
	sents = sendto(pingsock, data, datalen, 0,
						(struct sockaddr *)&host_addr, sizeof(host_addr));

	if(sents < 0 || sents != datalen){
		if(errno == ETIMEDOUT){
			printf("[ping_to_host] ping send(%s) error - TIMEOUT \n", hostIP);
			return 1;
		}
		printf("[ping_to_host] ping send(%s) error - %s\n", hostIP, strerror(errno));
		
	}

	return 0;

}

int recv_ping_from_host(in_addr_t *hostIP)
{
	int					recvds, len, fromlen;
	char				data[MAX_PACKET];
	struct imcp		    *picmp;
	struct sockaddr_in	host_addr;


    len = sizeof(data);
    fromlen = sizeof(host_addr);

	recvds = recvfrom(pingsock, data, len, 0,
						(struct sockaddr *)&host_addr, &fromlen);

	if(recvds < 0){
		if(errno == ETIMEDOUT){
			printf("[ping_to_host] ping recv(%s) error - TIMEOUT \n", hostIP);
			return 1;
		}
		printf("[ping_to_host] ping recv(%s) error - %s\n", hostIP, strerror(errno));
		
	}

	decode_resp(data, recvds, &host_addr);

	*hostIP = htonl(host_addr.sin_addr.s_addr);

	return 0;
}

unsigned short check_sum(unsigned short *buff, int rsize)
{
	int chksum = 0;
    unsigned short odd_byte = 0;

	while(rsize > 1)
	{
		chksum += *buff++;
		rsize -= sizeof(unsigned short);
	}

	if(rsize){
        *(unsigned char *)(&odd_byte) = *(unsigned char *) buff;
        chksum += odd_byte;
    }

    chksum = (chksum >> 16)+(chksum & 0xffff);
	chksum += (chksum >> 16);
	return (unsigned short)(~chksum);

}

void fill_icmp_data(char *data)
{
	struct icmp 	*picmp;

	picmp = (struct icmp *)data;
	picmp->icmp_type = ICMP_ECHO;
	picmp->icmp_code = 0;
	picmp->icmp_id   = getpid() & 0xFFFF;
	picmp->icmp_cksum = 0;
    
	picmp->icmp_seq = (ntransmitted++);
    if(ntransmitted > MAX_PING_TRANS_SEQ)
        ntransmitted = 1;
}

char *
pr_type( t )
register int t;
{
    static char *ttab[] = {
        "Echo Reply",
        "ICMP 1",
        "ICMP 2",
        "Dest Unreachable",
        "Source Quence",
        "Redirect",
        "ICMP 6",
        "ICMP 7",
        "Echo",
        "ICMP 9",
        "ICMP 10",
        "Time Exceeded",
        "Parameter Problem",
        "Timestamp",
        "Timestamp Reply",
        "Info Request",
        "Info Reply"
    };

    if( t < 0 || t > 16 )
        return("OUT-OF-RANGE");

    return(ttab[t]);
}

tvsub( out, in )
register struct timeval *out, *in;
{
        if( (out->tv_usec -= in->tv_usec) < 0 )   {
                out->tv_sec--;
                out->tv_usec += 1000000;
        }
        out->tv_sec -= in->tv_sec;
}

void decode_resp(char *buf, int recvds, struct sockaddr_in *from)
{
	struct ip	    *pip;
    long            *lp = (long *)buf;
	struct icmp	*picmp;
    struct timeval  tv, *tp;
	int				i, hlen, triptime;
    struct in_addr   from_addr;

    from_addr = from->sin_addr;
    from->sin_addr.s_addr = ntohl( from->sin_addr.s_addr );

    gettimeofday(&tv, &tz);

    pip = (struct ip *)buf;
    hlen = pip->ip_hl << 2;

	if(recvds < (hlen + ICMP_MINLEN)){
		printf("Too few bytes from %s\n", inet_ntoa(from_addr));
		return;
	}

    recvds -= hlen;
	picmp = (struct icmp *)(buf+hlen);


    if(picmp->icmp_type != ICMP_ECHOREPLY){
#if 0
        printf("%d bytes from %s: ", recvds,
            inet_ntoa(from_addr));
        printf("icmp_type=%d (%s)\n",
            picmp->icmp_type, pr_type(picmp->icmp_type) );
        for( i=0; i<12; i++)
            printf("x%2.2x: x%8.8x\n", i*sizeof(long), *lp++ );
        printf("icmp_code=%d\n", picmp->icmp_code );
#endif

        return;
    }

	if(picmp->icmp_id != (unsigned short)getpid()){
//		fprintf(stderr, "someone else's packet!\n");
		return;
    }


	tp = (struct timeval *)&picmp->icmp_data[0];
	printf("%d bytes from %s: ", recvds,
		inet_ntoa(from_addr));
	printf("icmp_seq=%d. ", picmp->icmp_seq );
	tvsub( &tv, tp );
	triptime = tv.tv_sec*1000+(tv.tv_usec/1000);
	printf("time=%d. ms\n", triptime );
}

char    *ipAddr[10] = {
    "10.160.59.201",
    "10.160.59.202",
    "10.160.59.203",
    "10.160.59.204",
    "10.160.59.205",
    "10.160.59.206",
    "10.160.59.207",
    "10.160.59.208",
    "10.160.59.209",
    "10.160.59.210",
};

typedef struct stPingCheck{
    in_addr_t   hostIP;
    char        check;
}PingCheck;


void main(int argc, char *argv[])
{
	int			hostNo, i, waitcount;
	PingCheck   pingchk[10];
    in_addr_t   recvIP;

	init_ping_socket();

    for(i = 0; i < 10; i++){
        pingchk[i].hostIP = inet_addr(ipAddr[i]);
        pingchk[i].check = 0;
    }

	for(i = 1; i < 10; i++){
		send_ping_to_host(pingchk[i].hostIP);
	}
printf("start -> %u %u\n", clock(), time(NULL));
    waitcount = 0;
	while(waitcount < 100){
		recv_ping_from_host(&recvIP);
        for(i = 0; i < 10; i++)
            if(recvIP == pingchk[i].hostIP){
                pingchk[i].check = 1;
                break;
            }
        waitcount++;
    }
printf("end -> %u %u\n", clock(), time(NULL));

    for(i = 0; i < 10; i++){
        if(pingchk[i].check)
            printf("Ping Success -%s\n", ipAddr[i]);
        else
            printf("Ping Fail -%s\n", ipAddr[i]);

    }
}
