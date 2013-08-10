/**********************************************************
                 LGT DSC Project

   Author   : JUNE.
   Section  : UPRESTO
   SCCS ID  : %W%
   Date     : %G%
   Revision History :
	'09.  11.14	Revised by JUNE.
	'11.  4. 20	Revised by JUNE.


   Description:

   Copyright (c) UPRESTO 2009
***********************************************************/

/**A.1*  File Inclusion ***********************************/
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <time.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <unistd.h>
#include <pcap.h>

//#include "VJ.h"
#include <sys/ethernet.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/timeb.h>
#include <ctype.h>

#include "comm_define.h"
#include <eth_capd.h>
#include <shmutil.h>
#include <utillib.h>
#include "Errcode.h"
#include <capd_global.h>
#include <init_shm.h>
#include "mems.h"
#include "nifo.h"
#include "clisto.h"
#include "capd_test.h"
//#include "capd_bfr.h"

#if 0
#include <capd_def.h>
#endif

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
        u_char  ip_vhl;							/* version << 4 | header length >> 2 */
        u_char  ip_tos;							/* type of service */
        u_short ip_len;							/* total length */
        u_short ip_id;							/* identification */
        u_short ip_off;							/* fragment offset field */
        #define IP_RF 0x8000					/* reserved fragment flag */
        #define IP_DF 0x4000					/* dont fragment flag */
        #define IP_MF 0x2000					/* more fragments flag */
        #define IP_OFFMASK 0x1fff				/* mask for fragmenting bits */
        u_char  ip_ttl;							/* time to live */
        u_char  ip_p;							/* protocol */
        u_short ip_sum;							/* checksum */
        struct  in_addr ip_src,ip_dst;			/* source and dest address */
};

#define IP_HL(ip)               (((ip)->ip_vhl) & 0x0f)
#define IP_V(ip)                (((ip)->ip_vhl) >> 4)


/* TCP header */
typedef u_int tcp_seq;

struct sniff_tcp {
        u_short th_sport;						/* source port */
        u_short th_dport;						/* destination port */
        tcp_seq th_seq;							/* sequence number */
        tcp_seq th_ack;							/* acknowledgement number */
        u_char  th_offx2;						/* data offset, rsvd */
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
        u_short th_win;							/* window */
        u_short th_sum;							/* checksum */
        u_short th_urp;							/* urgent pointer */
};


extern int      check_my_run_status (char *procname);
extern int 		dSend_CAPD_Data(stMEMSINFO *pstMEMSINFO, S32 dSndMsgQ, U8 *pNode, U32 sec);
extern void 	test_func(char *ip, unsigned short usPort);
extern int 		INIT_CAPD_IPCS(void);
extern void 	SetupSignal(void);
extern int 		open_device(char *dev);
extern int		keepalivelib_init(char *processName);
extern int 		open_device_dlpi (char *dev_path);

extern int 		dCurDev;

stMEMSINFO 		*pstMEMSINFO;
UCHAR  			*pstBuffer;
UCHAR 			*pstNode;
UCHAR 			*pstTLVNode;

int 	dANAQid;
int 	cnt=0, tot=0;
int		semid_capindex = -1;
struct 	timeb time_start, time_stop;
#ifdef BUFFERING
#if 0
time_t  			nowTime = 0;
time_t  			oldTime = 0;
UINT                Diff_Node_Cnt = 0;
#endif
#endif /* BUFFERING */

static char vERSION[7] = "R2.0.0";	// R1.0.0 -> R2.0.0

SFM_SysCommMsgType   *loc_sadb;

int main (int argc, char *argv[])
{
	int 	ret;
	char 	logbuffer[4096];

	Init_logdebug( getpid(), "CAPD", "/DSC/APPLOG" );

	ret = INIT_CAPD_IPCS();
	if(ret < 0) {
		sprintf(logbuffer, "FAIL[init_ipc] [%s] %d.", strerror(errno), ret);
		dAppWrite(LOG_CRI, logbuffer);
		exit(0);
	}
	if( gen_info->DebugLevel == 0 )
		gen_info->DebugLevel = LOG_INFO; /* temporary code (without CHSMD) */

	if((ret=set_version(SEQ_PROC_CAPD, vERSION)) < 0 ) {
		dAppLog( LOG_CRI, "SET_VERSION ERROR(RET=%d,IDX=%d,VER=%s)\n",
				ret,SEQ_PROC_CAPD,vERSION);
	}
	
	if( check_my_run_status("CAPD") < 0)
		exit(0);

	if( keepalivelib_init("CAPD") < 0 )
		exit(1);

#if 0
	if( argc==3 ) {
		test_func(argv[1], atoi(argv[2]));
	}
#endif
	if( argc == 3 ) {
		test_func2(argv[1], atoi(argv[2]));
		exit(0);
	}
	else if( argc!=1 ) {
		exit(0);
	}

	SetupSignal();

	dAppLog(LOG_CRI, "CAPD %s %d] [PROCESS INIT SUCCESS", vERSION, getpid());
	dAppLog(LOG_CRI, "CAPD %s %d] [PROCESS STARTED", vERSION, getpid());

	open_device_dlpi(DEV_PATH);

	return 0;
} /***** end of main *****/


void do_action(int port, int len, char *data, struct timeval *tmv)
{
	int				ret;
	T_CAPHDR		*pstCAPHead;

	if(len > 1518)	{	/* ethernet packet size */
		dAppLog(LOG_CRI, "Ethernet Packet Size Error, input length =%d", len);
	}

	/* ADD BY YOON 2008.10.14 */
	if( (pstNode = nifo_node_alloc(pstMEMSINFO)) == NULL) {
		dAppLog(LOG_CRI, "[%s.%d] nifo_node_alloc NULL", __FUNCTION__, __LINE__);
		exit(0);
	}
	if( (pstCAPHead = (T_CAPHDR *)nifo_tlv_alloc(pstMEMSINFO, pstNode, CAP_HEADER_NUM, CAP_HDR_SIZE, DEF_MEMSET_OFF)) == NULL ) {
		dAppLog(LOG_CRI, "[%s.%d] nifo_tlv_alloc [%d] NULL", __FUNCTION__, __LINE__, CAP_HEADER_NUM);
		exit(0);
	}
	if( (pstBuffer = nifo_tlv_alloc(pstMEMSINFO, pstNode, ETH_DATA_NUM, len, DEF_MEMSET_OFF)) == NULL ) {
		dAppLog(LOG_CRI, "[%s.%d] nifo_tlv_alloc [%d] NULL", __FUNCTION__, __LINE__, ETH_DATA_NUM);
		exit(0);
	}

	pstCAPHead->bRtxType = port;
	pstCAPHead->curtime = tmv->tv_sec;
	pstCAPHead->ucurtime = tmv->tv_usec;
	pstCAPHead->datalen = len;

	memcpy(pstBuffer, data, pstCAPHead->datalen);
	if((ret = dSend_CAPD_Data(pstMEMSINFO, dANAQid, pstNode, pstCAPHead->curtime)) < 0) {
		dAppLog(LOG_CRI, "[%s.%d] dSend_CAPD_Data [%d][%s]", __FUNCTION__, __LINE__, ret, strerror(-ret));
		exit(111);
	}

} /**** end of do_action ******/


unsigned short handle_ethernet
        (u_char *args,const struct pcap_pkthdr* pkthdr,const u_char* packet)
{
    struct ether_header *eptr;  /* net/ethernet.h */

    /* lets start with the ether header... */
    eptr = (struct ether_header *) packet;

    fprintf(stdout,"ethernet header source: %s"
            ,ether_ntoa((const struct ether_addr *)&eptr->ether_shost));
    fprintf(stdout," destination: %s "
            ,ether_ntoa((const struct ether_addr *)&eptr->ether_dhost));

    /* check to see if we have an ip packet */
    if (ntohs (eptr->ether_type) == ETHERTYPE_IP)
    {
        fprintf(stdout,"(IP)");
    }else  if (ntohs (eptr->ether_type) == ETHERTYPE_ARP)
    {
        fprintf(stdout,"(ARP)");
    }else  if (ntohs (eptr->ether_type) == ETHERTYPE_REVARP)
    {
        fprintf(stdout,"(RARP)");
    }else {
        fprintf(stdout,"(?)");
        exit(1);
    }
    fprintf(stdout,"\n");

    return eptr->ether_type;
}


/*
 * print data in rows of 16 bytes: offset   hex   ascii
 *
 * 00000   47 45 54 20 2f 20 48 54  54 50 2f 31 2e 31 0d 0a   GET / HTTP/1.1..
 */
void print_hex_ascii_line(const u_char *payload, int len, int offset)
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


void print_payload(const u_char *payload, int len)
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


int send_packet(UCHAR *pNode, T_CAPHDR *pHead, UCHAR *pBuffer)
{
	int	ret;
	struct timeval  cap_t;
        
	cap_t.tv_sec = 0; cap_t.tv_usec = 0;
   	
	gettimeofday (&cap_t, NULL);
	pHead->bRtxType = 1; //port;                                                                                                         
	pHead->curtime 	= cap_t.tv_sec;
	pHead->ucurtime = cap_t.tv_usec;

	if(pHead->datalen > 1518) {
		/* ethernet packet size */
        dAppLog(LOG_CRI, "eth packet size ERR(len=%d)", pHead->datalen);
	}
	
	/* ADD : BY JUNE, 2011-05-09
	 * DESC: STANDBY 상태에서는 DLPI BUFFER 에서 Packet을 빼내기 위해 읽은 후 버리고
	 *		 ACTIVE 상태에서는 DLPI BUFFER 에서 Packet을 읽은 후 Next Block으로 전송.
	 */
	if (loc_sadb->loc_system_dup.myLocalDupStatus != SYS_STATE_ACTIVE /** NOT ACTIVE **/) {
		dAppLog(LOG_DEBUG, "SYSTEM MODE : %d", loc_sadb->loc_system_dup.myLocalDupStatus);
		nifo_node_delete(pstMEMSINFO, pNode);
		return 0;
	}

/* ADD  : BY JUNE, 2010-05-10
 * DESC : BUFFERING 시에 (Send_Node_Cnt <= Collection_Cnt) 의 경우 처리.
 */
#ifdef BUFFERING
#if 0
	nowTime = time(0); 
	if (oldTime + COLLECTION_TIME < nowTime) {
		if (Send_Node_Cnt && Send_Node_Cnt == Diff_Node_Cnt) {
			// Send Buffring Packet 
    		if((ret = dSend_CAPD_Data(pstMEMSINFO, dANAQid, NULL, 0)) < 0) {
				dAppLog(LOG_CRI, "dSend_CAPD_Data[%d] ERR[%s", ret, strerror(-ret));
			}
			Collection_Cnt = COLLECTION_MIN;
		}   
		Diff_Node_Cnt = Send_Node_Cnt;
		oldTime = nowTime;
	}
#endif
#endif /* BUFFERING */

    if((ret = dSend_CAPD_Data(pstMEMSINFO, dANAQid, pNode, pHead->curtime)) < 0) {
        dAppLog(LOG_CRI, "dSend_CAPD_Data[%d] ERR[%s]", ret, strerror(-ret));
		nifo_node_delete(pstMEMSINFO, pNode);
   		return -1; 
	}
	return 0;
}

