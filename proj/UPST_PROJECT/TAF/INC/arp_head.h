#ifndef	_ARP_HEAD_H_
#define _ARP_HEAD_H_

#define DEF_A11_REG_ACCT    0x00
#define DEF_A11_REG_REQ     0x01
#define DEF_A11_REG_REP     0x03
#define DEF_A11_REG_UP      0x14
#define DEF_A11_REG_ACK     0x15
#define DEF_A11_SESS_UP		0x16
#define DEF_A11_SESS_ACK	0x17

#define DEF_A11_REQUEST     0x01
#define DEF_A11_RESPONSE    0x02
#define DEF_A11_PAIR        0x03

#define DEF_LCP_CONF_REQ    0x01
#define DEF_LCP_CONF_ACK    0x02
#define DEF_LCP_CONF_NAK    0x03
#define DEF_LCP_CONF_REJ    0x04
#define DEF_LCP_TERM_REQ    0x05
#define DEF_LCP_TERM_ACK    0x06
#define DEF_LCP_PROT_REJ    0x08
#define DEF_LCP_ECHO_REQ	0x09
#define DEF_LCP_ECHO_REP	0x0A
#define DEF_LCP_DISC_REQ    0x0B

#define DEF_CHAP_CHAL       0x01
#define DEF_CHAP_RESP       0x02
#define DEF_CHAP_SUCC       0x03
#define DEF_CHAP_FAIL       0x04

#define DEF_PAP_AUTH_REQ    0x01
#define DEF_PAP_AUTH_ACK    0x02
#define DEF_PAP_AUTH_NAK    0x03

#define DEF_IPCP_CONF_REQ   0x01
#define DEF_IPCP_CONF_ACK   0x02
#define DEF_IPCP_CONF_NAK   0x03
#define DEF_IPCP_CONF_REJ   0x04
#define DEF_IPCP_TERM_REQ   0x05
#define DEF_IPCP_TERM_ACK   0x06
#define DEF_IPCP_CODE_REJ   0x07

#define CONN_SETUP      0x01
#define ACTIVE_START    0x02
#define ACTIVE_STOP     0x04
#define DEF_SDB         0x08

#define DEF_A11_PORT	699
#define MAX_PPP_SIZE    2000

/* DEFINE PROTOCOL */
#define NUM_TCP         10
#define NUM_UDP         11
#define NUM_GRE         12
    
#define NUM_TCP_FTP_D   100
#define NUM_TCP_FTP_C   101
#define NUM_TCP_HTTP    102
    
#define NUM_UDP_A11     120         
#define NUM_UDP_RADIUS  121         
#define NUM_UDP_X2      122         
#define NUM_UDP_DNS     123
#define NUM_UDP_A14     124
#define NUM_UDP_A13     125     /* ADDED BY LDH 2005.0907 */


#define NUM_PPP         20     /* PPP */
#define NUM_CCP         21     /* PPP_CCP */
#define NUM_LCP         22     /* PPP_LCP */
#define NUM_IPCP        23     /* PPP_IPCP */
#define NUM_CHAP        24     /* PPP_CHAP */
#define NUM_PAP         25     /* PPP_PAP */
#define NUM_MIP         26     /* PPP_MIP */

#define NUM_UP_LCP      31	   /* PPP_UP_LCP */
#define NUM_DOWN_LCP    32	   /* PPP_DOWN_LCP */
#define NUM_UP_IPCP     33	   /* PPP_UP_IPCP */
#define NUM_DOWN_IPCP   34	   /* PPP_DOWN_IPCP */

#define DEF_IS95A_NUM	15
#define DEF_IS95B_NUM	25
#define DEF_1X_NUM		33
#define DEF_EVDO_NUM	59

#define DEF_CALL_NORMAL		0		/* 발신 호 */
#define DEF_CALL_RECALL		1		/* 착신 호 시작 */
#define DEF_CALL_RECALL_1	2		/* 착신 호 후 시그널 있음 */

typedef struct _st_CallHashKey_
{
	long long 		llIMSI;
} st_CallHashKey, *pst_CallHashKey;

#define DEF_CALLHASHKEY_SIZE	sizeof(st_CallHashKey)

typedef struct _st_CallHashData_
{
	UINT			uiPCFIP;
	UINT			uiReserved;
	struct timeval	stCreateTime;
} st_CallHashData, *pst_CallHashData;

#define DEF_CALLHASHDATA_SIZE	sizeof(st_CallHashData)

#define DEF_CALLHASH_COUNT		250007


#endif

