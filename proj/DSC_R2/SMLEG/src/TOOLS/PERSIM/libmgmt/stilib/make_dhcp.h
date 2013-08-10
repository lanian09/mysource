#ifndef MAKE_DHCP_H
#define MAKE_DHCP_H

#include <time.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <sys/types.h>
#include <sys/socket.h>
#if defined(SunOS)
#include <sys/sockio.h>
#endif
#include <sys/uio.h>

#pragma pack(1)

typedef struct dhcpformat {
	unsigned char		op;
    unsigned char 		htype;
    unsigned char 		hlen;
    unsigned char 		hops;
    unsigned int 		xid;
    unsigned short	 	secs;
    unsigned short 		flags;
    uint32_t 			ciaddr;
    uint32_t 			yiaddr;
    uint32_t 			siaddr;
    uint32_t 			giaddr;
    char 				chaddr[16];
    char 				sname[64];
    char 				file[128];
	uint32_t			magic;
} dhcp_t;

typedef struct dhcpopt_type {
	unsigned char		code;
	unsigned char		length;
	unsigned char		type;
} dhcpopt_type_t;		

typedef struct dhcpopt_int {
	unsigned char 		code;
	unsigned char		length;
	uint32_t			addr;
} dhcpopt_int_t;

typedef struct dhcpopt_string {
	unsigned char		code;
	unsigned char		length;
	char				value[64];
} dhcpopt_string_t;

#define DHCP_SERVER_PORT 67
#define DHCP_CLIENT_PORT 68


extern int make_dhcp_msg(char *,unsigned int,unsigned int,char *);
extern int add_dhcp_option_type(char *,int,int,int,int);
extern int add_dhcp_option_int(char *,int,int,int,u_int);
extern int add_dhcp_option_string(char *,int,int,int,char *);
extern unsigned int make_random_number();

#pragma pack()

#endif
