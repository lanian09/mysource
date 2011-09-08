/*
 * sniffex.c
 *
 * Sniffer example of TCP/IP packet capture using libpcap.
 * 
 * Version 0.1.1 (2005-07-05)
 * Copyright (c) 2005 The Tcpdump Group
 *
 * This software is intended to be used as a practical example and 
 * demonstration of the libpcap library; available at:
 * http://www.tcpdump.org/
  Below are a few simple examples:
 *
 * Expression			Description
 * ----------			-----------
 * ip					Capture all IP packets.
 * tcp					Capture only TCP packets.
 * tcp port 80			Capture only TCP packets with a port equal to 80.
 * ip host 10.1.2.3		Capture all IP packets to or from host 10.1.2.3.
 *
 ****************************************************************************
 *
 */

#define APP_NAME		"sniffex"
#define APP_DESC		"Sniffer example using libpcap"
#define APP_COPYRIGHT	"Copyright (c) 2005 The Tcpdump Group"
#define APP_DISCLAIMER	"THERE IS ABSOLUTELY NO WARRANTY FOR THIS PROGRAM."

#include <pcap.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "hasho.h"
#include "mems.h"

#include "mmc_hld.h"
#include "sysconf.h"
#include "proc_version.h"
#include "omp_filepath.h"
#include "keepalivelib.h"
#include <mysql_db_tables.h>
#include <mysql.h>
#include <commlib.h>

/* default snap length (maximum bytes per packet to capture) */
#define SNAP_LEN 1518

/* ethernet headers are always exactly 14 bytes [1] */
#define SIZE_ETHERNET 14

/* Ethernet addresses are 6 bytes */
#define ETHER_ADDR_LEN	6

/* Ethernet header */
struct sniff_ethernet {
        u_char  ether_dhost[ETHER_ADDR_LEN];    /* destination host address */
        u_char  ether_shost[ETHER_ADDR_LEN];    /* source host address */
        u_short ether_type;                     /* IP? ARP? RARP? etc */
};

/* IP header */
struct sniff_ip {
        u_char  ip_vhl;                 /* version << 4 | header length >> 2 */
        u_char  ip_tos;                 /* type of service */
        u_short ip_len;                 /* total length */
        u_short ip_id;                  /* identification */
        u_short ip_off;                 /* fragment offset field */
        #define IP_RF 0x8000            /* reserved fragment flag */
        #define IP_DF 0x4000            /* dont fragment flag */
        #define IP_MF 0x2000            /* more fragments flag */
        #define IP_OFFMASK 0x1fff       /* mask for fragmenting bits */
        u_char  ip_ttl;                 /* time to live */
        u_char  ip_p;                   /* protocol */
        u_short ip_sum;                 /* checksum */
        struct  in_addr ip_src,ip_dst;  /* source and dest address */
};
#define IP_HL(ip)               (((ip)->ip_vhl) & 0x0f)
#define IP_V(ip)                (((ip)->ip_vhl) >> 4)

/* TCP header */
typedef u_int tcp_seq;

struct sniff_tcp {
        u_short th_sport;               /* source port */
        u_short th_dport;               /* destination port */
        tcp_seq th_seq;                 /* sequence number */
        tcp_seq th_ack;                 /* acknowledgement number */
        u_char  th_offx2;               /* data offset, rsvd */
#define TH_OFF(th)      (((th)->th_offx2 & 0xf0) >> 4)
        u_char  th_flags;
        #define TH_FIN  0x01
        #define TH_SYN  0x02
        #define TH_RST  0x04
        #define TH_PUSH 0x08
        #define TH_ACK  0x10
        #define TH_URG  0x20
        #define TH_ECE  0x40
        #define TH_CWR  0x80
        #define TH_FLAGS        (TH_FIN|TH_SYN|TH_RST|TH_ACK|TH_URG|TH_ECE|TH_CWR)
        u_short th_win;                 /* window */
        u_short th_sum;                 /* checksum */
        u_short th_urp;                 /* urgent pointer */
};


stHASHOINFO *pstHASHOINFO;
long		g_match_cnt = 0;				/** Match count **/
int			g_cont_flag = 1;
int			MyQid, mmcdQid;
struct timeval 	avg_delta;

long		g_read_cnt = 0;
long		g_avg_delta = 0;
long		g_min_delta = 0;
long		g_max_delta = 0;

#define SMS_DB      "DSCM"             // Mysql Database ¸i 
#define SMS_TBL     "pkc_delay"       // Mysql Table ¸i 
#define HOST_IP     "localhost"    // Mysql A￠¼O IP
#define USER_NAME   "root"              // Mysql A￠¼O °eA¤ ¸i 
#define PASSWD      "mysql"             // Mysql A￠¼O ÆÐ½º¿oμa 

time_t		g_startTime;


char    mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN];
extern  char    trcBuf[4096], trcTmp[1024];
extern  int     trcLogId, trcErrLogId, trcLogFlag;

char    ver[8] = "R1.0.0";
unsigned char	env_cap_dev [16] = "e1000g0";
int				env_match_cnt = 5;
int				env_start_time = 3600;

MYSQL       sql, *conn;

typedef struct _aaa {

	unsigned short ip_id;                  /* identification */
	unsigned short ip_sum;                 /* checksum */
	struct  in_addr ip_src,ip_dst;  /* source and dest address */
} stKEY;

typedef struct _bbb {
	struct timeval ts;
} stDATA;



void
got_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet);

void
print_payload(const u_char *payload, int len);

void
print_hex_ascii_line(const u_char *payload, int len, int offset);

void
print_app_banner(void);

void
print_app_usage(void);

int 
cdelay_mysql_init(void);

/*
 * app name/banner
 */
void
print_app_banner(void)
{

	printf("%s - %s\n", APP_NAME, APP_DESC);
	printf("%s\n", APP_COPYRIGHT);
	printf("%s\n", APP_DISCLAIMER);
	printf("\n");

return;
}

/*
 * print help text
 */
void
print_app_usage(void)
{

	printf("Usage: %s [interface]\n", APP_NAME);
	printf("\n");
	printf("Options:\n");
	printf("    interface    Listen on <interface> for packets.\n");
	printf("\n");

return;
}

/*
 * print data in rows of 16 bytes: offset   hex   ascii
 *
 * 00000   47 45 54 20 2f 20 48 54  54 50 2f 31 2e 31 0d 0a   GET / HTTP/1.1..
 */
void
print_hex_ascii_line(const u_char *payload, int len, int offset)
{

	int i;
	int gap;
	const u_char *ch;

	/* offset */
	printf("%05d   ", offset);
	
	/* hex */
	ch = payload;
	for(i = 0; i < len; i++) {
		printf("%02x ", *ch);
		ch++;
		/* print extra space after 8th byte for visual aid */
		if (i == 7)
			printf(" ");
	}
	/* print space to handle line less than 8 bytes */
	if (len < 8)
		printf(" ");
	
	/* fill hex gap with spaces if not full line */
	if (len < 16) {
		gap = 16 - len;
		for (i = 0; i < gap; i++) {
			printf("   ");
		}
	}
	printf("   ");
	
	/* ascii (if printable) */
	ch = payload;
	for(i = 0; i < len; i++) {
		if (isprint(*ch))
			printf("%c", *ch);
		else
			printf(".");
		ch++;
	}

	printf("\n");

return;
}

/*
 * print packet payload data (avoid printing binary data)
 */
void
print_payload(const u_char *payload, int len)
{

	int len_rem = len;
	int line_width = 16;			/* number of bytes per line */
	int line_len;
	int offset = 0;					/* zero-based offset counter */
	const u_char *ch = payload;

	if (len <= 0)
		return;

	/* data fits on one line */
	if (len <= line_width) {
		print_hex_ascii_line(ch, len, offset);
		return;
	}

	/* data spans multiple lines */
	for ( ;; ) {
		/* compute current line length */
		line_len = line_width % len_rem;
		/* print line */
		print_hex_ascii_line(ch, line_len, offset);
		/* compute total remaining */
		len_rem = len_rem - line_len;
		/* shift pointer to remaining bytes to print */
		ch = ch + line_len;
		/* add offset */
		offset = offset + line_width;
		/* check if we have line width chars or less */
		if (len_rem <= line_width) {
			/* print last line and get out */
			print_hex_ascii_line(ch, len_rem, offset);
			break;
		}
	}

return;
}


/* *************************************** */
/*
 * The time difference in microseconds
 */
long delta_time (struct timeval * now,
                 struct timeval * before) {
  time_t delta_seconds;
  time_t delta_microseconds;

  /*
   * compute delta in second, 1/10's and 1/1000's second units
   */
  delta_seconds      = now -> tv_sec  - before -> tv_sec;
  delta_microseconds = now -> tv_usec - before -> tv_usec;

  if(delta_microseconds < 0) {
    /* manually carry a one from the seconds field */
    delta_microseconds += (1000000);  /* 1e6 */
    -- delta_seconds;
  }
  return((delta_seconds * (1000000)) + delta_microseconds);
}




int timeSub (struct timeval *res,
        const struct timeval *after, const struct timeval *before)
{
    long sec = after->tv_sec - before->tv_sec;
    long usec = after->tv_usec - before->tv_usec;

    if (usec < 0)
        usec += 1000000, --sec;

    res->tv_sec = sec;
    res->tv_usec = usec;

    return (sec < 0) ? (-1) : ((sec == 0 && usec == 0) ? 0 : 1);
}


int mysql_insert_cdelay (time_t locTime, long g_match_cnt, long g_min_delta, long g_max_delta, long g_avg_delta)
{
	char query[1024] = {0,};
	unsigned int  min_sec=0.0, max_sec=0.0, avg_sec=0.0;
	unsigned int  min_usec=0, max_usec=0, avg_usec=0;

	min_usec  = (int)(g_min_delta % 1000000);
	min_sec = (int)(g_min_delta / 1000000);
	max_usec  = (int)(g_max_delta % 1000000);
	max_sec = (int)(g_max_delta / 1000000);
	avg_usec  = (int)(g_avg_delta % 1000000);
	avg_sec = (int)(g_avg_delta / 1000000);

	snprintf(query, sizeof(query),
			"Insert Into pkt_delay_hour_statistics Values ("                                 
			"'%s', from_unixtime(%d), from_unixtime(%d), %d, %d.%06d, %d.%06d, %d.%06d);",
			"SCE", (int)g_startTime, (int)locTime, (int)g_match_cnt,
			min_sec, min_usec,
			max_sec, max_usec,
			avg_sec, avg_usec);
#if 0
			(float)g_min_delta == 0 ? 0: (int)(g_min_delta%1000000), (float)g_min_delta == 0 ? 0: (((float)g_min_delta)/((float)1000000)),
			(float)g_max_delta == 0 ? 0: (int)(g_max_delta%1000000), (float)g_max_delta == 0 ? 0: (float)g_max_delta/(float)1000000,
			(float)g_avg_delta == 0 ? 0: (int)(g_avg_delta%1000000), (float)g_avg_delta == 0 ? 0: (float)g_avg_delta/(float)1000000);
#endif


	if ( mysql_query(conn, query) != 0 )                                      
	{                                                                         
		logPrint(trcLogId, FL, "Query Fail: query=%s, err=%d:%s\n", query, mysql_errno(conn), mysql_error(conn));   
		return -1;                                                            
	}                                                                         
	logPrint(trcLogId, FL, "Query Success : %s\n", query);
	return 0; 
}

/*
 * dissect/print packet
 */
void
got_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet)
{

	static int count = 1;                   /* packet counter */
	
	/* declare pointers to packet headers */
	const struct sniff_ethernet *ethernet;  /* The ethernet header [1] */
	const struct sniff_ip *ip;              /* The IP header */
	const struct sniff_tcp *tcp;            /* The TCP header */
	const char *payload;                    /* Packet payload */

	int size_ip;
	int size_tcp;
	int size_payload;

	stKEY  			stCKey;
	stHASHONODE 	*pnode = NULL;
	stDATA 			stData, *pstData;
//	struct timeval 	res;
	

	if (g_cont_flag != 1) 
		return;

//	printf("\nPacket number %d:\n", count);
	count++;
	
	/* define ethernet header */
	ethernet = (struct sniff_ethernet*)(packet);
	
	/* define/compute ip header offset */
	ip = (struct sniff_ip*)(packet + SIZE_ETHERNET);
	size_ip = IP_HL(ip)*4;
	if (size_ip < 20) {
		printf("   * Invalid IP header length: %u bytes\n", size_ip);
		return;
	}

	/* print source and destination IP addresses */
//	printf("       From: %s\n", inet_ntoa(ip->ip_src));
//	printf("         To: %s\n", inet_ntoa(ip->ip_dst));
	
	/* determine protocol */	
	switch(ip->ip_p) {
		case IPPROTO_IP:
		case IPPROTO_UDP:
		case IPPROTO_ICMP:
			return;
		case IPPROTO_TCP:
			break;
		default:
			return;
	}
	
	/*
	 *  OK, this packet is TCP.
	 */
	
#if 1
	/* define/compute tcp header offset */
	tcp = (struct sniff_tcp*)(packet + SIZE_ETHERNET + size_ip);
	size_tcp = TH_OFF(tcp)*4;
	if (size_tcp < 20) {
		printf("   * Invalid TCP header length: %u bytes\n", size_tcp);
		return;
	}
#endif
	
//	fprintf(stderr, "   Src port: %d\n", ntohs(tcp->th_sport));
//	fprintf(stderr, "   Dst port: %d\n", ntohs(tcp->th_dport));


	g_read_cnt++;
#ifndef TEST
	stCKey.ip_id = ip->ip_id;
#endif
	stCKey.ip_sum = ip->ip_sum;
//	stCKey.ip_sum = 0;

	memcpy ((char *)&stCKey.ip_src, (char *)&ip->ip_src, sizeof(struct  in_addr));
	memcpy ((char *)&stCKey.ip_dst, (char *)&ip->ip_dst, sizeof(struct  in_addr));


	pnode = hasho_find (pstHASHOINFO, (U8 *)&stCKey);
	if (pnode == NULL) {	// node 가 없다. hash 에 add

		stData.ts.tv_sec = header->ts.tv_sec;
		stData.ts.tv_usec = header->ts.tv_usec;

		pnode = hasho_add (pstHASHOINFO, (U8 *)&stCKey, (U8 *)&stData);
		if (pnode == NULL)
		{
			// hasho node가 더 이상 없다. 작업 종료.

    		pstHASHOINFO = hasho_init(0 /**< maim memory */, sizeof(stKEY), sizeof(stKEY), sizeof(stDATA), 400 /**< hash size */, NULL, 0);
    		if (pstHASHOINFO == NULL) {
        		exit(0);
    		}
			// 기동 중지.
			g_cont_flag = 0;
			return;
		}

	} else {				// 이미 node가 있다. Match !! 

		long  delta_usec;
		pstData = (stDATA *)HASHO_PTR (pstHASHOINFO, pnode->offset_Data);

		g_match_cnt++;

		delta_usec = delta_time ((struct timeval *)&header->ts,(struct timeval *)&pstData->ts)/(long)2;
//		printf ("delta_usec : %d\n", delta_usec);
 
		if (delta_usec > 100000) {
			delta_usec = 0;
		}

		if (g_match_cnt == 1) {
			g_avg_delta = delta_usec;
			g_min_delta = g_max_delta = g_avg_delta;
		} else {
			if (g_min_delta > (delta_usec)) {
				g_min_delta = (delta_usec);
			} 

			if (g_max_delta < (delta_usec)) {
				g_max_delta = (delta_usec);
			}

			g_avg_delta = (g_avg_delta + delta_usec)/2;
		}

		if (g_match_cnt == env_match_cnt) {

			time_t		locTime;
			int			dret;

    		pstHASHOINFO = hasho_init(0 /**< maim memory */, sizeof(stKEY), sizeof(stKEY), sizeof(stDATA), 400 /**< hash size */, NULL, 0);
    		if (pstHASHOINFO == NULL) {
        		exit(0);
    		}

			printf ("g_read_cnt: %ld\n", g_read_cnt);
			printf ("g_match_cnt: %ld\n", g_match_cnt);

			printf ("g_avg_delta: %ld\n", g_avg_delta);
			printf ("g_min_delta: %ld\n", g_min_delta);
			printf ("g_max_delta: %ld\n", g_max_delta);


			locTime = time(NULL);
//			printf ("%d. %d. %d.\n", g_min_delta, g_max_delta, g_avg_delta);
			if( !cdelay_mysql_init() ){
				mysql_insert_cdelay (locTime, g_match_cnt, g_min_delta, g_max_delta, g_avg_delta);
				printf ("insert dret: %d\n", dret);
			} else {
				logPrint(trcLogId, FL, "Connection Fail: err=%d:%s\n", mysql_errno(conn), mysql_error(conn));
			}

			sleep(1);

			g_read_cnt = 0;
			g_match_cnt = 0;
			g_cont_flag = 0;
			g_avg_delta = 0;
			g_min_delta = 0;
			g_max_delta = 0;
		}
//	    header->ts.tv_sec;
//	   	header->ts.tv_usec;
	}




	return;




	/* define/compute tcp payload (segment) offset */
	payload = (u_char *)(packet + SIZE_ETHERNET + size_ip + size_tcp);
	
	/* compute tcp payload (segment) size */
	size_payload = ntohs(ip->ip_len) - (size_ip + size_tcp);
	
	/*
	 * Print payload data; it might be binary, so don't just
	 * treat it as a string.
	 */
	if (size_payload > 0) {
		printf("   Payload (%d bytes):\n", size_payload);
		print_payload(payload, size_payload);
	}

return;
}



int cdelay_initLog (void)
{
    char    *env, fname[256];

    if ((env = getenv(IV_HOME)) == NULL) {
        fprintf(stderr,"[stmd_initLog] not found %s environment name\n", IV_HOME);
        return -1;
    }

    sprintf(fname,"%s/%s.%s", env, CDELAY_TRCLOG_FILE, mySysName);
    if ((trcLogId = loglib_openLog (fname,
            LOGLIB_MODE_LIMIT_SIZE | LOGLIB_FLUSH_IMMEDIATE |
            LOGLIB_TIME_STAMP | LOGLIB_FNAME_LNUM)) < 0) {
        fprintf(stderr,"[stmd_initLog] openLog fail[%s]\n", fname);
        return -1;
    }   

    sprintf(fname,"%s/%s.%s", env, CDELAY_ERRLOG_FILE, mySysName);
    if ((trcErrLogId = loglib_openLog (fname,
            LOGLIB_MODE_LIMIT_SIZE | LOGLIB_FLUSH_IMMEDIATE |
            LOGLIB_TIME_STAMP | LOGLIB_FNAME_LNUM)) < 0) {
        fprintf(stderr,"[stmd_initLog] openLog fail[%s]\n", fname);
        return -1;
    }

    return 1;
}

int cdelay_mysql_init()
{
	mysql_init (&sql);
	if ( (conn = mysql_real_connect (&sql, HOST_IP, USER_NAME, PASSWD, SMS_DB, 0, 0, 0)) == NULL){
		sprintf(trcBuf, "cdelay init, mysql connection Fail = %d:%s\n", mysql_errno(&sql), mysql_error(&sql));
		trclib_writeErr(FL, trcBuf);
		return -1;
	}
	return 0;
}

int cdelay_init ()
{
	char	*env, fname[256], tmp[64];
    int     key; 
//	int		shmId;
//	int     i;


	avg_delta.tv_sec = 0;
	avg_delta.tv_usec = 0;

    if ((env = getenv(MY_SYS_NAME)) == NULL) {
        fprintf(stderr,"[stmd_init] not found %s environment name\n", MY_SYS_NAME);
        return -1;
    }


	pstHASHOINFO = hasho_init(0 /**< maim memory */, sizeof(stKEY), sizeof(stKEY), sizeof(stDATA), 400 /**< hash size */, NULL, 0);
	if (pstHASHOINFO == NULL) {
		exit(0);
	}


    strcpy (mySysName, env);
    strcpy (myAppName, "CDELAY");
    commlib_setupSignals(NULL);

    if(set_proc_version(OMP_VER_INX_CDELAY, ver) < 0){
        fprintf(stderr, "[InitSys] setting process version failed\n");
        return -1;
    }

	if (cdelay_initLog() < 0)
		return -1;

    // Keepalive를 공유메모리를 할당한다.
    if (keepalivelib_init (myAppName) < 0)
        return -1;

    if ((env = getenv(IV_HOME)) == NULL) {
        fprintf(stderr,"[InitSys] not found %s environment name\n", IV_HOME);
        return -1;
    }
    sprintf (fname, "%s/%s", env, SYSCONF_FILE);


    // sysconfig에서 자신의 message queue key를 구한다.
    if (conflib_getNthTokenInFileSection (fname, "APPLICATIONS", myAppName, 1, tmp) < 0) {
        sprintf(trcBuf, "My System message queue Get Fail = %s[%d]\n", strerror(errno), errno);
        trclib_writeErr(FL, trcBuf);
        return -1;
    }   
    key = strtol(tmp,0,0);
    if ((MyQid = msgget (key, IPC_CREAT|0666)) < 0) {
        sprintf(trcBuf,"[cdealy_init] msgget fail; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
        trclib_writeErr(FL, trcBuf);
        return -1;
    }   
    fprintf(stdout, "cdelayQid = %x, key = %x\n", MyQid, key);


    // sysconfig에서 COND의 message queue key를 구한다.
    if (conflib_getNthTokenInFileSection (fname, "APPLICATIONS", "MMCD", 1, tmp) < 0) {
        sprintf(trcBuf, "MMCD message queue Get Fail = %s[%d]\n", strerror(errno), errno);
        trclib_writeErr(FL, trcBuf);
        return -1;
    }   
    key = strtol(tmp,0,0);
    if ((mmcdQid = msgget (key, IPC_CREAT|0666)) < 0) {
		fprintf (stderr, "dddddcode\n");
        sprintf(trcBuf,"[stmd_init] msgget fail; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
        trclib_writeErr(FL, trcBuf);
        return -1;
    }


    // sysconfig에서 Device # 를 구한다.
    if (conflib_getNthTokenInFileSection (fname, "CDELAY_PART", "CAPD_DEV", 1, env_cap_dev) < 0) {
        sprintf(trcBuf, "CAPD_DEV Value Get Fail = %s[%d]\n", strerror(errno), errno);
        trclib_writeErr(FL, trcBuf);
        return -1;
    }   

	// sysconfig에서 Device # 를 구한다.
    if (conflib_getNthTokenInFileSection (fname, "CDELAY_PART", "MATCH_CNT", 1, tmp) < 0) {
        sprintf(trcBuf, "MATCH_CTN Value Get Fail = %s[%d]\n", strerror(errno), errno);
        trclib_writeErr(FL, trcBuf);
        return -1;
    }   
	env_match_cnt = atoi(tmp);

	// sysconfig에서 Device # 를 구한다.
    if (conflib_getNthTokenInFileSection (fname, "CDELAY_PART", "LOOP_TIME", 1, tmp) < 0) {
        sprintf(trcBuf, "MATCH_CTN Value Get Fail = %s[%d]\n", strerror(errno), errno);
        trclib_writeErr(FL, trcBuf);
        return -1;
    }   
	env_start_time = atoi(tmp);


	if( cdelay_mysql_init() < 0 ){
		sprintf(trcBuf, "cdelay terminate.\n"); trclib_writeErr(FL, trcBuf);
		return -1;
		
	}

	return 1;
}

#define STAT_UNIT        300
int isTimeToWork ()
{
    time_t      cur_time;
    struct tm*  cur_tMS;

    cur_time  = time ( (time_t *)0);

    //localtime_r ( &cur_time, &cur_tMS );
    cur_tMS = (struct tm*)localtime((time_t*)&cur_time);

    // 현재의 분이 5로 나누어 떨어지면 할 일을 한다.
//    if ( (cur_tMS->tm_min%STAT_UNIT) == 0 && (cur_tMS->tm_sec > 5) && (cur_tMS->tm_sec < 45) && g_cont_flag == 0 )

	if (((cur_time%env_start_time) == 0) && (g_cont_flag == 0)) 	
    {
        g_cont_flag = 1;
		g_startTime = (time(0)/env_start_time)*env_start_time;
#if 0
            fprintf(stderr, "start time: %04d-%02d-%02d %02d:%02d:%02d\n",
                    cur_tMS->tm_year + 1900,
                    cur_tMS->tm_mon +1,
                    cur_tMS->tm_mday,
                    cur_tMS->tm_hour,
                    cur_tMS->tm_min,
                    cur_tMS->tm_sec );
#endif
            logPrint(trcLogId, FL, "start time: %04d-%02d-%02d %02d:%02d:%02d\n",
                    cur_tMS->tm_year + 1900,
                    cur_tMS->tm_mon +1,
                    cur_tMS->tm_mday,
                    cur_tMS->tm_hour,
                    cur_tMS->tm_min,
                    cur_tMS->tm_sec );
            return 1;
    }


    return -1;

}



void
main_loop (pcap_t *handle) 
{
    struct pcap_pkthdr  *pkt_header;
    const u_char        *pkt_data;
	int					dRet;
	fd_set          	fds,rfds;
	int             	fd;
	struct timeval      tm;
//	char        		errbuf[PCAP_ERRBUF_SIZE];
	GeneralQMsgType rxGenQMsg;

	FD_ZERO(&fds);

	fd = pcap_get_selectable_fd (handle);
	FD_SET (fd, &fds);

	g_startTime	=  time(0);
	while (1) {


		isTimeToWork ();

        keepalivelib_increase();
        
		while ((dRet = msgrcv(MyQid, &rxGenQMsg, sizeof(GeneralQMsgType), 0, IPC_NOWAIT)) > 0) {
            mmcd_exeRxQMsg (&rxGenQMsg);
            memset(&rxGenQMsg, 0, sizeof(GeneralQMsgType));
        }

#if 1
		//fprintf (stderr, "g_cont_flag: %d\n", g_cont_flag);

		tm.tv_sec = 0;
		tm.tv_usec = 1;
		rfds = fds;

		if (select(FD_SETSIZE, &rfds, NULL, NULL, &tm) <= 0) {
			continue;
		}

		if (FD_ISSET (fd, &rfds)) {
			dRet = pcap_next_ex(handle, &pkt_header, &pkt_data);

			if (pkt_header->caplen) {
				got_packet(NULL, pkt_header, pkt_data);
				pkt_header->caplen = 0;
			}
		}
#endif
		//usleep(1);
		commlib_microSleep(10);
	}
}


int main(int argc, char **argv)
{

	char errbuf[PCAP_ERRBUF_SIZE];		/* error buffer */
	pcap_t *handle;				/* packet capture handle */

	char filter_exp[] = "ip";		/* filter expression [3] */
	struct bpf_program fp;			/* compiled filter program (expression) */
	bpf_u_int32 mask;			/* subnet mask */
	bpf_u_int32 net;			/* ip */
	int num_packets = 20;			/* number of packets to capture */
	int check_Index;


//    stKEY           stCKey;
//    stHASHONODE     *pnode = NULL;


#if 0
	/* check for capture device name on command-line */
	if (argc == 2) {
		dev = argv[1];
	}
	else if (argc > 2) {
		fprintf(stderr, "error: unrecognized command-line options\n\n");
		print_app_usage();
		exit(EXIT_FAILURE);
	} else {
		print_app_usage();
		exit(EXIT_FAILURE);
	}
	
#endif

    if((check_Index = check_my_run_status("CDELAY")) < 0)
        exit(0);

	if (cdelay_init () < 0)
		exit(-2);



	/* get network number and mask associated with capture device */
	if (pcap_lookupnet(env_cap_dev, &net, &mask, errbuf) == -1) {
		fprintf(stderr, "Couldn't get netmask for device %s: %s\n",
		    env_cap_dev, errbuf);
		net = 0;
		mask = 0;
	}

	/* print capture info */
	printf("Device: %s\n", env_cap_dev);
	printf("Number of packets: %d\n", num_packets);
	printf("Filter expression: %s\n", filter_exp);

	/* open capture device */
	handle = pcap_open_live(env_cap_dev, SNAP_LEN, 1, 1000, errbuf);
	if (handle == NULL) {
		fprintf(stderr, "Couldn't open device %s: %s\n", env_cap_dev, errbuf);
		exit(EXIT_FAILURE);
	}

	/* make sure we're capturing on an Ethernet device [2] */
	if (pcap_datalink(handle) != DLT_EN10MB) {
		fprintf(stderr, "%s is not an Ethernet\n", env_cap_dev);
		exit(EXIT_FAILURE);
	}

	/* compile the filter expression */
	if (pcap_compile(handle, &fp, filter_exp, 0, net) == -1) {
		fprintf(stderr, "Couldn't parse filter %s: %s\n",
		    filter_exp, pcap_geterr(handle));
		exit(EXIT_FAILURE);
	}

	/* apply the compiled filter */
	if (pcap_setfilter(handle, &fp) == -1) {
		fprintf(stderr, "Couldn't install filter %s: %s\n",
		    filter_exp, pcap_geterr(handle));
		exit(EXIT_FAILURE);
	}


	main_loop (handle);

//	/* now we can set our callback function */
//	pcap_loop(handle, -1, got_packet, NULL);

	/* cleanup */
	pcap_freecode(&fp);
	pcap_close(handle);

	printf("\nCapture complete.\n");

return 0;
}

