#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <getopt.h>
#include <sys/time.h>
#include <sys/timeb.h>

#include "sim.h"
#include "conf.h"
#include "misc.h"

#define MYCONF	"./sim.conf"

uint32_t MNIP=0, PDSN=0, PCF=0, AAA=0;
uint32_t SUBS=0, MIN=0, SOPT=0;
uint32_t CBIT=0, PBIT=0, HBIT=0;
uint64_t IMSI=0;
uint8_t  ID=0;
uint8_t REALM[BUFSIZ]="";
uint8_t KEY[BUFSIZ]="";

int FD;
int verbose = 0;
int hexdump = 0;
int prnstat = 0;
int keycalc = 0;
int summary = 0;
int loop	= 0;
int check_cnt	= 0;

uint32_t interval=100000, duration=5, run_time=30;
int32_t incl = 0;
uint32_t remain=0, login=0, logout=0;
uint32_t g_tot_login=0, g_tot_logout=0;
struct timeb current, time_start, time_stop;

uint64_t atoi64(uint8_t *in)
{
    uint64_t ret = 0;
    uint8_t c;
    while((c=*in++)) { ret = (c-'0') + ret * 10; }
    return (ret);
}

#define HASH_SIZE	0xFF
#define subsfunc(d)      (hton64(d)%HASH_SIZE)

static struct list_head subslist[HASH_SIZE];

typedef struct subs {
	pthread_mutex_t         lock;
	struct list_head        list;
	uint64_t 	imsi;
	char	 	nai[256];
	uint32_t	ip;
	uint8_t		id;
} subs_t;

static inline subs_t*
_subs_find (uint64_t imsi)
{       
	struct list_head* head = &subslist[subsfunc(imsi)];
	struct list_head *l, *tl;
    subs_t *h;

	if(list_empty (head)) return (NULL);
	list_for_each_safe (l, tl, head) {
        h = list_entry(l, subs_t, list);
        if(h->imsi != imsi) continue;
        return (h);
    }
    return (NULL);
} 

#define LOCKINIT        pthread_mutex_init (&h->lock,0)
#define LOCK            pthread_mutex_lock (&h->lock)
#define UNLOCK          pthread_mutex_unlock (&h->lock)

static inline subs_t* _subs_add (uint64_t imsi, uint32_t ip, char *nai)
{
	subs_t *h;
	if((h=_subs_find(imsi))) return (h);
	if(!(h=calloc(1,sizeof(subs_t)))) return (NULL);

	LOCKINIT;
	h->imsi = imsi;
	h->ip   = ip;
	strcpy(h->nai, nai);
	INIT_LIST_HEAD(&h->list); list_add(&h->list, &subslist[subsfunc(imsi)]);
	return (h);
}

static inline void _subs_del (uint64_t imsi)
{
    subs_t *h;
    if(!(h=_subs_find(imsi))) return;
	LOCK; list_del_init (&h->list); UNLOCK;
    free(h); h=NULL;
}

static inline void _subs_free (subs_t* h)
{
    if(!h) return;
    LOCK; list_del_init (&h->list); UNLOCK;
    free(h); h=NULL;
}

static inline void _subs_flush (void)
{
    struct list_head *head;
    struct list_head *l, *tl;
    subs_t *h;
    int i;

    for(i=0;i<HASH_SIZE;i++) {
        head = &subslist[i];
        list_for_each_safe(l, tl, head) {
            h = list_entry(l, subs_t, list);
            _subs_free (h);
        }
    }
}

static inline void _subs_list(void)
{
    struct list_head *head;
    struct list_head *l, *tl;
    subs_t *h;
    int i;

    printf("%-20s%-30s%-20s\n","IMSI", "NAI", "IP address");
    for(i=0;i<HASH_SIZE;i++) {
        head = &subslist[i];
        list_for_each_safe(l, tl, head) {
            h = list_entry(l, subs_t, list);
			printf("%-20lld%-30s %d.%d.%d.%d\n", h->imsi, h->nai, IPADDR(h->ip));
        }
    }
}

void initconf()
{
	char buf[BUFSIZ];
	sprintf(buf, "Find Configuration File ('%s')", MYCONF);
	STATUS(buf);
	if(access(MYCONF, R_OK)!=0) { PRN_FAIL; exit (0); } else PRN_OK;

	STATUS("Initialize Emulator Environment w/Configuration");

	if(!SUBS)	if(getconf(MYCONF,"SUBSCRIBER",buf))	SUBS =atoi(buf);
	if(!MIN)	if(getconf(MYCONF,"MIN",buf))			MIN  =atoi(buf);
	if(getconf(MYCONF,"REALM",buf))			strcpy(REALM, buf);
	if(!SOPT)	if(getconf(MYCONF,"SOPT",buf))			SOPT =atoi(buf);
	if(!MNIP)	if(getconf(MYCONF,"IP",buf))			MNIP =inet_addr(buf);
	if(!PDSN)	if(getconf(MYCONF,"PDSN",buf))			PDSN =inet_addr(buf);
	if(!PCF)	if(getconf(MYCONF,"PCF",buf))			PCF  =inet_addr(buf);
	if(!AAA)	if(getconf(MYCONF,"AAA",buf))			AAA  =inet_addr(buf);
	getconf(MYCONF,"KEY",KEY);
	if(!CBIT)	if(getconf(MYCONF,"CBIT",buf))			CBIT =atoi(buf);
	if(!PBIT)	if(getconf(MYCONF,"PBIT",buf))			PBIT =atoi(buf);
	if(!HBIT)	if(getconf(MYCONF,"HBIT",buf))			HBIT =atoi(buf);
	PRN_OK;

	printf(" - %-20s %d\n", "SUBSCRIBERS",SUBS);
	sprintf(buf, "45006%010d", MIN); IMSI=atoi64(buf);
	printf(" - %-20s %lld\n", "IMSI",IMSI);
	printf(" - %-20s %s\n", "REALM",REALM);
	printf(" - %-20s %d\n", "Service Option",SOPT);
	printf(" - %-20s %d.%d.%d.%d \n", "MN IP Address",IPADDR(MNIP));
	printf(" - %-20s %d.%d.%d.%d \n", "PDSN IP Address",IPADDR(PDSN));
	printf(" - %-20s %d.%d.%d.%d \n", "PCF IP Address",IPADDR(PCF));
	printf(" - %-20s %d.%d.%d.%d \n", "AAA IP Address",IPADDR(AAA));
	printf(" - %-20s %s \n", "KEY",KEY);
	printf(" - %-20s %d\n", "CBIT",CBIT);
	printf(" - %-20s %d\n", "PBIT",PBIT);
	printf(" - %-20s %d\n", "HBIT",HBIT);
	printf(" - %-20s %d\n", "Inclease (usec)",incl);
	printf(" - %-20s %d\n", "Interval (usec)",interval);
	printf(" - %-20s %d\n", "Duration (sec)",duration);
	printf(" - %-20s %d\n", "Run Time (sec)",run_time);
	printf("\n");
}

void initsubs()
{
	char NAI[BUFSIZ]="";
	int i;

	STATUS("Initialize Subscriber Information");

    for (i=0; i<HASH_SIZE; i++) INIT_LIST_HEAD(&subslist[i]);

	for(i=0;i<SUBS;i++) {
		// DISPLAY SUBSCRIBER INFORMATION
		sprintf(NAI, "%lld@%s", IMSI+i, REALM);
		_subs_add (IMSI+i, htonl(htonl(MNIP)+i), NAI);
		//printf("%d %lld %s %d.%d.%d.%d %d\n", i, IMSI+i, NAI, IPADDR(htonl(htonl(MNIP)+i)), ID);
	}
#if 0
	_subs_list();
#endif
	PRN_OK;
}

typedef struct RADIUS {
    uint8_t         code;
    uint8_t         id;
    uint16_t         length;
    uint8_t         authenticator[16];
} RADIUS_T ;    /* 20 Byte */

#define RADIUS_PORT				1813
#define RADIUS_ACCOUNTING_REQUEST	4

#define USER_NAME					1
#define NAS_IP_ADDRESS				4
#define FRAMED_IP_ADDRESS			8
#define VENDOR_SPEC					26
#define CALLING_STATION_ID			31
#define ACCT_STATUS_TYPE			40
#	define START				1
#	define STOP					2
#define ACCT_SESSION_ID				44
#define EVENT_TIMESTAMP				55

#define VENDOR_3GPP2	htonl(5535)
#define 	PCF_IP_ADDRESS	9
#define 	SERVICE_OPTION	16
#define 	IP_TECHNOLOGY		22
#define 	COMP_TUNNEL_IND	23
#define 	CORRELATION_ID	44
#define 	ESN				52
#define 	SERVICE_REF_ID	94
#define 	SUBNET			108

#define VENDOR_ELUON	htonl(6964)
#define 	ACCT_BIT				1

int AVP (uint8_t *BUF, uint8_t T, uint8_t L, uint8_t *V) 
{
	*BUF++ = T;
	*BUF++ = L+2;
	memcpy(BUF, V, L); BUF += L;

	return (L+2);
}


int VSA (uint8_t *BUF, uint32_t VENDOR, uint8_t T, uint8_t L, uint8_t *V) 
{
	*BUF++ = VENDOR_SPEC;
	*BUF++ = L+2+6;
	*(uint32_t*)BUF = VENDOR; BUF+=4;

	*BUF++ = T;
	*BUF++ = L+2;
	memcpy(BUF, V, L); BUF += L;

	return (L+2+6);
}

void SEND(uint8_t *BUF, uint32_t LEN)
{
	struct sockaddr_in sin;

    bzero((char *)&sin, sizeof(sin));
    sin.sin_family      = AF_INET;
    sin.sin_addr.s_addr = AAA;
    sin.sin_port        = htons(RADIUS_PORT);
	
	sendto(FD,BUF,LEN, 0,(struct sockaddr *)&sin, sizeof(sin));
}

void start(subs_t *h)
{
	uint8_t BUF[BUFSIZ], *BP, V[BUFSIZ];
	RADIUS_T *rh = (RADIUS_T*)BUF;
	uint8_t DIGEST[16];
	MD5_CTX context;

	ftime (&current);
	fmttm("%H:%M:%S",V);
	rh->code 	= RADIUS_ACCOUNTING_REQUEST;
	rh->id 		= h->id = ++ID;

	if(verbose) printf("%s.%03d " CLR_BOLD CLR_AQUA "Acc-Start %3d" CLR_NRML " (%lld, %d.%d.%d.%d)\n",
		V, current.millitm, h->id, h->imsi, IPADDR(h->ip));

	BP = (uint8_t*)(rh+1);

	*(uint32_t*)V = htonl(START);
	BP += AVP (BP, ACCT_STATUS_TYPE, 4, V);

	sprintf(V,"%lld", h->imsi);
	BP += AVP (BP, CALLING_STATION_ID, 15, V);

	BP += VSA (BP, VENDOR_3GPP2, PCF_IP_ADDRESS, 4, (uint8_t*)&PCF);

	*(uint32_t*)V = htonl(SOPT);
	BP += VSA (BP, VENDOR_3GPP2, SERVICE_OPTION, 4, V);

	*(uint32_t*)V = htonl(1);
	BP += VSA (BP, VENDOR_3GPP2, IP_TECHNOLOGY, 4, V);

	*(uint32_t*)V = htonl(h->id);
	BP += VSA (BP, VENDOR_3GPP2, CORRELATION_ID, 4, V);

	*(uint32_t*)V = htonl(time(0));
	BP += AVP (BP, EVENT_TIMESTAMP, 4, V);

	*(uint64_t*)V = hton64(h->id);
	BP += AVP (BP, ACCT_SESSION_ID, 8, V);

	BP += AVP (BP, FRAMED_IP_ADDRESS, 4, (uint8_t*)&h->ip);
	BP += AVP (BP, USER_NAME, strlen(h->nai), h->nai);

	BP += AVP (BP, NAS_IP_ADDRESS, 4, (uint8_t*)&PDSN);

	sprintf(V, "cbit=%d", CBIT);
	BP += VSA (BP, VENDOR_ELUON, ACCT_BIT, strlen(V), V);

	sprintf(V, "pbit=%d", PBIT);
	BP += VSA (BP, VENDOR_ELUON, ACCT_BIT, strlen(V), V);

	sprintf(V, "hbit=%d", HBIT);
	BP += VSA (BP, VENDOR_ELUON, ACCT_BIT, strlen(V), V);

	memcpy(BP, KEY, strlen(KEY));

	rh->length = htons(BP-BUF);

	if(keycalc) {
	MD5Init(&context);
	MD5Update(&context, BUF, BP-BUF+strlen(KEY));
	MD5Final(DIGEST, &context);
	memcpy(rh->authenticator, DIGEST, 16);
	}

	if(hexdump) dump(BUF, BP-BUF);
	SEND(BUF, BP-BUF); 
	login++;
}

void stop(subs_t *h) 
{
	uint8_t BUF[BUFSIZ], *BP, V[BUFSIZ];
	RADIUS_T *rh = (RADIUS_T*)BUF;
	uint8_t DIGEST[16];
	MD5_CTX context;

	ftime (&current);
	fmttm("%H:%M:%S",V);
	rh->code 	= RADIUS_ACCOUNTING_REQUEST;
	rh->id 		= h->id = ++ID;

	if(verbose) printf("%s.%03d " CLR_BOLD CLR_YLLW "Acc-Stop  %3d" CLR_NRML " (%lld, %d.%d.%d.%d)\n",
		V, current.millitm, h->id, h->imsi, IPADDR(h->ip));

	BP = (uint8_t*)(rh+1);

	*(uint32_t*)V = htonl(STOP);
	BP += AVP (BP, ACCT_STATUS_TYPE, 4, V);

	sprintf(V,"%lld", h->imsi);
	BP += AVP (BP, CALLING_STATION_ID, 15, V);

	BP += VSA (BP, VENDOR_3GPP2, PCF_IP_ADDRESS, 4, (uint8_t*)&PCF);

	*(uint32_t*)V = htonl(SOPT);
	BP += VSA (BP, VENDOR_3GPP2, SERVICE_OPTION, 4, V);

	*(uint32_t*)V = htonl(1);
	BP += VSA (BP, VENDOR_3GPP2, IP_TECHNOLOGY, 4, V);

	*(uint32_t*)V = htonl(h->id);
	BP += VSA (BP, VENDOR_3GPP2, CORRELATION_ID, 4, V);

	*(uint32_t*)V = htonl(time(0));
	BP += AVP (BP, EVENT_TIMESTAMP, 4, V);

	*(uint64_t*)V = hton64(h->id);
	BP += AVP (BP, ACCT_SESSION_ID, 8, V);

	BP += AVP (BP, FRAMED_IP_ADDRESS, 4, (uint8_t*)&h->ip);
	BP += AVP (BP, USER_NAME, strlen(h->nai), h->nai);

	BP += AVP (BP, NAS_IP_ADDRESS, 4, (uint8_t*)&PDSN);

	sprintf(V, "cbit=%d", CBIT);
	BP += VSA (BP, VENDOR_ELUON, ACCT_BIT, strlen(V), V);

	sprintf(V, "pbit=%d", PBIT);
	BP += VSA (BP, VENDOR_ELUON, ACCT_BIT, strlen(V), V);

	sprintf(V, "hbit=%d", HBIT);
	BP += VSA (BP, VENDOR_ELUON, ACCT_BIT, strlen(V), V);

	memcpy(BP, KEY, strlen(KEY));

	rh->length = htons(BP-BUF);

	if(keycalc) {
	MD5Init(&context);
	MD5Update(&context, BUF, BP-BUF+strlen(KEY));
	MD5Final(DIGEST, &context);
	memcpy(rh->authenticator, DIGEST, 16);
	}

	if(hexdump) dump(BUF, BP-BUF);
	SEND(BUF, BP-BUF); logout++;
}

void subs_expire (char *arg)
{
    subs_t *h = (subs_t*)arg;
    if(!h) return;

	// BUILD RADIUS-ACCOUNT-STOP
	
	stop (h);	
}

void check()
{
	double sec;

	if(!prnstat && !summary) return;

	ftime(&time_stop);
   	sec = (double)((time_stop.time*1000 + time_stop.millitm)
 		-(time_start.time*1000 + time_start.millitm))/1000.0;

	if(prnstat && sec>=1.0) {
   		printf("#%6d Elapsed time: %.3f sec (%3.1f cps)\n" , loop, sec ,(double)(loop)/sec);

		time_start = time_stop; loop = 0;
	}
	else if(summary &&  sec>=1.0) {
   		printf("Remain:%5d[%d-%d], Login:%4d (%6.1f cps), Logout:%4d (%6.1f cps), %3.1f sec\n" , 
		remain+=(login-logout), g_tot_login, g_tot_logout, login, (double)(login)/sec, logout, (double)(logout)/sec, sec);

		g_tot_login += login;
		g_tot_logout += logout;

		time_start = time_stop; login = logout = 0;
		check_cnt++;
	}
}

void usage(char *progname)
{
	printf("Usage: %s [-vhxkp] [--interval=] [--incl=] [--duration=] [--run_time]\n", progname);
	printf("    -h: help         \n");
	printf("    -v: verbose      \n");
	printf("    -x: hexa-dump    \n");
	printf("    -p: print status \n");
	printf("    -k: generate md5 \n");
	printf("    -s: summary      \n");
	printf("    --incl=args (usec; default=0)\n");
	printf("    --interval=args (usec; default=100000=0.1s=10 cps)\n");
	printf("    --duration=args (sec; session keep-alive duration)\n");
	printf("    --run_time=args (sec; sending time )\n");

	exit(0);
}

int main(int argc, char *argv[])
{
	int c;

	while (1) {
		int option_index = 0;
		static struct option lopt[] = {
			{"duration", 1, 0, 0},
			{"interval", 1, 0, 0},
			{"incl", 1, 0, 0},
			{"run_time", 1, 0, 0},
			{"help", 0, 0, 0},
			{0, 0, 0, 0}
		};  
		c = getopt_long (argc, argv, "vhxkps",lopt, &option_index);
		if (c == -1) break;
		switch (c) { 
		case 0:
			if(!strcmp(lopt[option_index].name,"duration") && optarg) duration = atoi(optarg);
			if(!strcmp(lopt[option_index].name,"interval") && optarg) interval = atoi(optarg);
			if(!strcmp(lopt[option_index].name,"incl") && optarg) incl = atoi(optarg);
			if(!strcmp(lopt[option_index].name,"run_time") && optarg) run_time = atoi(optarg);
			if(!strcmp(lopt[option_index].name,"help")) usage(argv[0]);
			break;
		case 'v': verbose =1; break;
		case 'x': hexdump =1; break;
		case 'p': prnstat =1; break;
		case 'k': keycalc =1; break;
		case 's': summary =1; break;
		case 'h':
		case '?':
		default:
			usage(argv[0]);
		}
	}

	initconf();
	initsubs();
	printf("\n");
	
	FD = udpsock(0);

	ftime (&time_start);
	while(1) {
		struct list_head *head;
		struct list_head *l, *tl;
		subs_t *h;
		int i;

		for(i=0;i<HASH_SIZE;i++) {
			head = &subslist[i];
			list_for_each_safe(l, tl, head) {
				h = list_entry(l, subs_t, list);

				// BUILD RADIUS-ACCOUNT-START
				if (check_cnt < run_time) {
					start (h); loop++;
					// TIMER (duration)
					set_cb_timeout (subs_expire, h, duration * 1000);
				}

				// CHECK
				check();

				// INTERVAL (SLEEP HERE)
				usleep(interval);

			}
		}
		interval += incl;
	}
}

