#ifndef RADIUS_H
#define RADIUS_H

typedef struct radius {
	unsigned char	code;
	unsigned char	id;
	unsigned short	len;
	unsigned char	auth[16];
} radius_t;

typedef struct attr {
	unsigned char	type;
	unsigned char	len;
} attr_t;

typedef struct vend_attr {
	unsigned int 	id;
	unsigned char	type;
	unsigned char	len;
} vend_attr_t;

typedef struct rattr {
	unsigned char	type;
	unsigned char	len;
	char *val;
} rattr_t;

#define set_radius_init(r,d)		bzero(r,sizeof(rattr_t)*d)
#define set_radius_attr(r,d,x,y,z)	{ (r)[d].type=x; (r)[d].len=y; (r)[d].val=(char*)z; }

/*************************************************************
    RADIUS (RFC-2139 & P.S0001-B)
*************************************************************/

#define RAD_SEND_BUFFER_SIZE    16536

#define VID_3GPP2               5535

/* RADIUS codes */
#define PW_ACCESS_REQUEST       0x01
#define PW_ACCESS_ACCEPT        0x02
#define PW_ACCESS_REJECT        0x03
#define PW_ACCOUNTING_REQUEST   0x04
#define PW_ACCOUNTING_RESPONSE  0x05
#define PW_ACCESS_CHALLENGE     0x0b

/* service options */
#define SO_IS_95C               33

/* ip technology */
#define TECH_MOBILE_IP          2

/* RADIUS standard attribute-value pairs */
#define AT_USER_NAME            1   /* string   (64) */
#define AT_USER_PASSWD          2   /* string   (64) */
#define AT_CHAP_PASSWD          3   /* string   (64) */
#define AT_NAS_IP_ADDRESS       4   /* ipaddr   (4) */
#define AT_FRAMED_IP_ADDRESS    8   /* ipaddr   (4) */
#define AT_LOGIN_IP_HOST        14  /* ipaddr   (4) */
#define AT_3GPP2                26  /* vendor   (64) */
#define AT_CALLING_STATION_ID   31  /* string   (15) */
#define AT_CALLED_STATION_ID    30  /* string   (15) */
#define AT_ACCT_STATUS_TYPE     40  /* integer  (64) */
#define AT_ACCT_INPUT_OCTETS    42  /* integer  (64) */
#define AT_ACCT_OUTPUT_OCTETS   43  /* integer  (64) */
#define AT_ACCT_SESSION_ID      44  /* string   (8) */
#define AT_ACCT_SESSION_TIME    46  /* integer  (8) */
#define AT_ACCT_INPUT_PACKETS   47  /* integer  (64) */
#define AT_ACCT_OUTPUT_PACKETS  48  /* integer  (64) */

#define AT_ACCT_TERM_CAUSE      49  /* integer  (8) */
#define AT_CHAP_CHALLENGE       60  /* string   (16) */

/* RADIUS vendor specific attribute-value pairs */
#define AT_3GPP2_HA_IP_ADDR     7   /* D1: ipadd    (4) */
#define AT_3GPP2_SO             16  /* F5: integer  (4) */
#define AT_3GPP2_IP_TECH        22  /* F11: integer (4) */

/* Acct-Terminate-Cause */
#define PW_USER_REQUEST         1
#define PW_LOST_CARRIER         2
#define PW_LOST_SERVICE         3
#define PW_IDLE_TIMEOUT         4
#define PW_SESS_TIMEOUT         5
#define PW_ADMIN_RESET          6
#define PW_ADMIN_REBOOT         7
#define PW_PORT_ERROR           8
#define PW_NAS_ERROR            9
#define PW_NAS_REQUEST          10
#define PW_NAS_REBOOT           11
#define PW_PORT_UNN             12
#define PW_PORT_PREE            13
#define PW_PORT_SUSP            14
#define PW_SERVICE_UNA          15
#define PW_CALLBACK             16
#define PW_USER_ERROR           17
#define PW_HOST_REQUEST         18

/* Accounting Status types */
#define PW_STATUS_INIT          0
#define PW_STATUS_START         1
#define PW_STATUS_STOP          2
#define PW_STATUS_INTERIM       3
#define PW_STATUS_ACCOUNTING_ON 7
#define PW_STATUS_ACCOUNTING_OFF 8

/* Attribute types */
#define PW_TYPE_STRING          0
#define PW_TYPE_INTEGER         1
#define PW_TYPE_IPADDR          2
#define PW_TYPE_DATE            3

/* Attribute size */
#define SZ_INTEGER              4
#define SZ_IPADDR               4
#define SZ_DATE                 4
#define SZ_64                   64
#define SZ_15                   15
#define SZ_12                   12
#define SZ_8                    8

#endif
