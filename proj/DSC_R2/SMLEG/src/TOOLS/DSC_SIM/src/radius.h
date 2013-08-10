#ifndef __RADIUS_H__
#define __RADIUS_H__


#define MAX_AUTH_SIZE       		16
#define MAX_IMSI_LEN            	16

#define MAX_MIN_SIZE        		17      /* Calling Station ID */
#define MAX_USERNAME_SIZE       	65

#define RADIUS_CODE_LEN             1
#define RADIUS_IDENTIFIER_LEN       1
#define RADIUS_LENGTH_LEN           2
#define RADIUS_LENGTH_POS           2

#define RADIUS_ATTR_T_LEN         	1
#define RADIUS_ATTR_L_LEN       	1
#define RADIUS_ATTR_OFFSET          2

#define DEF_VENDOR_ATTR         	26
#define RADIUS_VENDORID_LEN         4
#define RADIUS_VENDOR_T_LEN       	1
#define RADIUS_VENDOR_L_LEN     	1
#define RADIUS_VENDOR_OFFSET        8
#define RADIUS_VENDORATTR_OFFSET    2

#define DEF_RADIUS_VENDOR_ID_LGT	6964	

#define RADIUS_HEADER_LEN           20
#define MAX_RADIUS_PKTSIZE  		5000
#define MAX_RADIUS_SEND_CNT			4294967295
#define MAX_CPH_BIT_LEN				8

enum { 	ERADIUS_ACCT_START = 1,
		ERADIUS_ACCT_STOP };

typedef struct __acct_start {
	unsigned char   ucCode;                 			/* RADIUS CODE */
	unsigned char	ucID;                   			/* RADIUS IDENTIFIER */
	unsigned short	usLen;            					/* RADIUS PACKET LEN */
	unsigned char   szAuthen[MAX_AUTH_SIZE];			/* Authenticator : 16 */

	long long 		llAcctID;							/* Accounting Session ID */
	unsigned char 	szMIN[MAX_MIN_SIZE];				/* Calling-Station-ID 	: 17 */
	unsigned char	szUserName[MAX_USERNAME_SIZE];  	/* User Name 			: 65 */
	unsigned int	uiFramedIP;							/* Framed-IP-Addr 		: 6	*/
	unsigned int	dAStatType;							/* Acct_Status_Type 	: 6 */
	unsigned int	ucNASType;							/* NAS_Port_Type		: 6 */
	unsigned int	uiNASIP;							/* NAS_IP_Address		: 6 */
	unsigned int 	dEventTime;							/* Event_Timestamp		: 6 */
	unsigned char	szCBit[MAX_CPH_BIT_LEN];			/* VSA, CBIT			: 1 */
	unsigned char	szPBit[MAX_CPH_BIT_LEN];			/* VSA, PBIT			: 1 */
	unsigned char	szHBit[MAX_CPH_BIT_LEN];			/* VSA, HBIT			: 1 */

} ACCT_START, *PACCT_START;

#if 1
typedef struct __acct_stop {
	unsigned char   ucCode;                 			/* RADIUS CODE */
	unsigned char	ucID;                   			/* RADIUS IDENTIFIER */
	unsigned short	usLen;            					/* RADIUS PACKET LEN */
	unsigned char   szAuthen[MAX_AUTH_SIZE];			/* Authenticator : 16 */

	long long 		llAcctID;
	unsigned char 	szMIN[MAX_MIN_SIZE];				/* Calling-Station-ID 	: 17 */
	unsigned char	szUserName[MAX_USERNAME_SIZE];  	/* User Name 			: 65 */
	unsigned int	uiFramedIP;							/* Framed-IP-Addr 		: 6 */
	unsigned int	dAStatType;							/* Acct_Status_Type 	: 6 */
#if 0	
	unsigned int	dAcctInOctes;						/* Acct-Input-Octets	: 6 */
	unsigned int	dAcctOutOctes;						/* Acct-Output-Octets	: 6 */
	unsigned int	dAcctSessTime;						/* Acct-Session-Time	: 6 */
	unsigned int	ucNASType;							/* NAS_Port_Type		: 6 */
	unsigned int	uiNASIP;							/* NAS_IP_Address		: 6 */
	unsigned int 	dEventTime;							/* Event_Timestamp		: 6 */
#endif
} ACCT_STOP, *PACCT_STOP;
#endif

typedef struct __account_req {
	ACCT_START 	stAcctStart;
	ACCT_STOP	stAcctStop;

} ACCT_REQ, *PACCT_REQ;


#endif /* __RADIUS_H__ */

