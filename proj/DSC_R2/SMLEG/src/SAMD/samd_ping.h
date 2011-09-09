#ifndef SAMD_PING_H
#define SAMD_PING_H


#define ICMP_ECHO 8 
#define ICMP_ECHOREPLY 0 

#define THR_KEEP_ALIVE      400000000
#define DEF_PACKET_SIZE		64
#define MAXWAIT             10
#define	MAX_PACKET			4096

#define CONFFILE_RELOAD     10

typedef struct stPingCheck{
    char        ipAlias[SFM_MAX_LAN_NAME_LEN];
    char        ipAddrS[SFM_MAX_TARGET_IP_LEN];
    in_addr_t   hostIP;
    char        check;
}PingCheck;

unsigned int    ntransmitted = 1;
struct timezone tz;	/* leftover */
 
void *thr_ping_test(void *args);
int ping_send_recv(int pingsock, PingCheck *pingchk, int destnum, SFM_SysCommLanSts *lansts);
int local_ping_info(PingCheck *pingchk);
int remote_ping_info(PingCheck *pingchk);
int	init_ping_socket();
int send_ping_to_host(int pingsock, in_addr_t hostIP);
int recv_ping_from_host(int pingsock, in_addr_t *hostIP);
unsigned short check_sum(unsigned short *buff, int rsize);
void decode_resp(char *buf, int bytes, struct sockaddr_in *from);
void fill_icmp_data(char *data);
char * pr_type( register int t );
void finish_ping(int pingsock);

 
#endif /* SAMD_PING_H */
