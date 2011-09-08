#ifndef MAKE_RADIUS_H
#define MAKE_RADIUS_H

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#if defined(SunOS)
#include <sys/sockio.h>
#endif
#include <sys/uio.h>

#pragma pack(1)

#define ACCESS_PORT 1812
#define ACCOUNT_PORT 1813

typedef struct rad{
    u_char          code;
    u_char          id;
    u_short         length;
    u_char          authenticator[16];
} rad_t;    /* 20 Byte */

typedef struct att{
    u_char          type;
    u_char          att_length;
    char            temp[64];
} att_t;

typedef struct att_addr{
    u_char          type;
    u_char          att_length;
    uint32_t        address;
} att_addr_t;

typedef struct eap{
	u_char		code;
	u_char		id;
	u_short		length;
	u_char		type;
	char		value[64];
} eap_t;

typedef struct find_challenge{
	u_char 		code;
	u_char 		id;
	u_short 	length;
	u_char		type;
	u_char 		value_size;
	char 		value[16];
	char 		name[32];
} find_challenge_t;

typedef struct make_challenge{
	u_char		id;
	char		data[63];
} make_challenge_t;

typedef struct vendor_int{
	u_char		type;
	u_char		att_length;
	uint32_t	vendor_id;
	u_char		vendor_type;
	u_char		vendor_length;
	uint32_t	vendor_value;
}vendor_int_t;

typedef struct vendor_string{
	u_char		type;
	u_char		att_length;
	uint32_t	vendor_id;
	u_char		vendor_type;
	u_char		vendor_length;
	char		vendor_value[64];
}vendor_string_t;

extern int make_md5(char *, int, char *);
extern int make_access_msg(char *, int, int, char *);
extern int make_account_msg(char *, int, int);
extern int finish_access_msg(char *, int );
extern int finish_account_msg(char *, int, char *);
extern int get_challenge(char *, char *);
extern int get_state(char *, char *);
extern int make_req_challenge(char *, int, char *, char *, char *);
extern int get_authenticator(char *);
extern int add_radius_att_string(char *, int, int, int, char *);
extern int add_radius_att_int(char *, int, int, uint32_t);
extern int add_radius_att_vendor_string(char *, int, uint32_t, int, int, char *);
extern int add_radius_att_vendor_int(char *, int, uint32_t, int, uint32_t);
extern int make_eap_msg(char *, int, int, int, int, char *);
extern int make_req_eap_msg(char *, int, int, int, int, char *, char *);
extern int finish_access_eap(char *, int, char *, char *);

#pragma pack()

#endif
