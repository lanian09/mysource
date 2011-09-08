#ifndef _ADNS_SERV_H_
#define _ADNS_SERV_H_

/**
 * Define constants
 */
#define DEF_CHECK_QR		0x80
#define DEF_CHECK_RCODE		0x0F

/**
 * Declare functions
 */
extern int dGetCALLProcID(unsigned int uiClientIP);
extern int dDNSProcess( Capture_Header_Msg *pCAPHEAD, INFO_ETH *pETH, TCP_INFO *pTCP, U8 *pDATA, OFFSET offset );
extern int dSend_DNSLog( DNS_SESS_KEY *pDNSSESSKEY, DNS_SESS *pDNSSESS );
extern int dUDPDNSProcess( Capture_Header_Msg *pCAPHEAD, INFO_ETH *pETH, TCP_INFO *pTCP, U8 *pDATA );
extern int dTCPDNSProcess( Capture_Header_Msg *pCAPHEAD, INFO_ETH *pETH, TCP_INFO *pTCP, U8 *pDATA, OFFSET offset );
extern void InitDNSSESS( DNS_SESS *pDNSSESS );


#endif	/* _ADNS_SERV_H_ */
