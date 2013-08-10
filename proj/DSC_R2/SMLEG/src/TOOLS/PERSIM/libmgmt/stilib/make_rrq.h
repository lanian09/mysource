#ifndef MAKE_RRQ_H
#define MAKE_RRQ_H

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
#include <pthread.h>

#pragma pack (1)

#define MIP_RRQ_PORT 434

typedef struct req_format{
	unsigned char 	type;
	unsigned char 	bits;
	unsigned short	lifetime;
	uint32_t		home_address;
	uint32_t		home_agent;
	uint32_t		coa;
	unsigned int	id_high;
	unsigned int	id_low;
} request_t;

typedef struct rep_format{
	unsigned char type;
	unsigned char code;
	unsigned short	lifetime;
	uint32_t home_address;
	uint32_t home_agent;
	unsigned int id_high;
	unsigned int id_low;
} reply_t;

typedef struct extension{
	unsigned char	type;
	uint8_t			length;
	int				spi;
	unsigned char	authenticator[16];
} extension_t;

typedef struct nai{
	unsigned char	type;
	uint8_t			length;
	char			mn_nai[64];
} nai_t;

typedef struct challenge_extension{
	unsigned char	type;
	uint8_t			length;
	char			challenge[64];
} challenge_t;

typedef struct general{
	unsigned char	type;
	unsigned char	subtype;
	short			length;
	int				spi;
	char			auth[16];
} general_t;

typedef struct find{
	unsigned char	type;
	uint8_t			length;
	char			value[64];
} find_t;

typedef struct udp_request{
	uint8_t type;
	uint8_t length;
	uint8_t sub_type;
	uint8_t reserved1;
#if 0
	uint8_t f:1,
			r:1,
			reserved2:6;
#endif
	uint8_t flag;
	uint8_t encapsulation;
	uint16_t reserved3;
}udp_request_t;

typedef struct udp_reply{
	uint8_t type;
	uint8_t length;
	uint8_t sub_type;
	uint8_t reply_code;
#if 0
	uint16_t f:1,
			reserved:15;
#endif
	uint16_t flag;
	uint16_t keepalive;
}udp_reply_t;

typedef struct my_ext{
	uint8_t type;
	uint8_t length;
	char buf[16];
}my_ext_t;

typedef struct myint_ext{
	uint8_t type;
	uint8_t length;
	uint32_t value;
}myint_ext_t;

#define IP_ADDR_FORMAT(addr) addr>>24,((addr>>16)&0xFF),((addr>>8)&0xFF),(addr&0xFF)

extern int rrq_msg_generator(char *, char,int,uint32_t,uint32_t,uint32_t,unsigned int,unsigned int);
extern int add_rrq_extension(char *,int,char,int,char *);
extern int add_nai_extension(char *,int,int,char *);
extern int add_udp_tunnel_request(char *,int,int,int,int);
extern int add_general_mip_ext(char *,int,int,int,char *);
extern int add_my_ext(char *,int,int,int,char *);
extern int add_myint_ext(char *,int,int,int,unsigned int);
extern int make_presuf_md5(char *,int,char *,char *);

#pragma pack()

#endif
