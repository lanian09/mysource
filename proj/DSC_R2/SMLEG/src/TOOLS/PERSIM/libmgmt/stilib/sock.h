#ifndef SOCK_H
#define SOCK_H

#ifdef __cplusplus
extern "C" {
#endif

/* 
 * get icmp socket descriptor.
 */
int icmpsock(void);

/* 
 * tcp socket listener.
 */
int tcp_listen(unsigned short port);

/* 
 * tcp socket connector
 */
int tcp_connect(unsigned int addr,unsigned short port);

/* 
 * get udp socket descriptor.
 */
int udpsock(unsigned short port);
extern int pingtest(char*,int,int,int);

/* 
 * get udp socket descriptor with specific ip address.
 */
int udpaddrsock(unsigned char* ipaddr,unsigned short port);

/* 
 * get ip address by hostname
 */
int getsaddrbyhost(char* h,int port,struct sockaddr_in* snd);

/* 
 * ip checksum routine
 * returns sucess return 0
 */
int in_chksum(unsigned short* s,int n);

/* 
 * udp send & wait function with timer
 */
int sendnwait(char *s,int len,char *addr,unsigned short port,unsigned int wait);

#ifdef __cplusplus
}
#endif

#endif
