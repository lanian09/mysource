#ifndef _NUM_PROTO
#define _NUM_PROTO

/*******************************************************************/

#define NUM_ETC         -2
#define NUM_UNKNOWN     -1

// Layer 1
#define NUM_MAC         1       /* MAC */
#define NUM_DIX         2       /* MAC (Dix Ethernet) */
#define NUM_802_3       3       /* MAC (IEEE 802.3) */

// Layer 2 - Dix
#define NUM_IP          10       /* MAC - IP */
#define NUM_ARP         11       /* MAC - ARP */
#define NUM_RARP        12       /* MAC - RARP */

// Layer 2 - 802.3
#define NUM_802_3_IP          2000     /* MAC - IP (IEEE 802.3) */
#define NUM_802_3_BPDU        2001     /* MAC - BPDU (IEEE 802.3) */
#define NUM_802_3_NETBEUI     2002     /* MAC - NETBEUI (IEEE 802.3) */

// Layer 3 - Dix
#define NUM_TCP         100     /* MAC - IP - TCP */
#define NUM_UDP         101     /* MAC - IP - UDP */
#define NUM_GRE         102     /* MAC - IP - GRE */
#define NUM_ICMP        103     /* MAC - IP - ICMP */
#define NUM_IGMP        104     /* MAC - IP - IGMP */

// Layer 3 - 802.3
#define NUM_802_3_TCP         2100      /* MAC - IP - TCP (IEEE 802.3) */
#define NUM_802_3_UDP         2101      /* MAC - IP - UDP (IEEE 802.3) */
#define NUM_802_3_GRE         2102     /* MAC - IP - GRE (IEEE 802.3) */
#define NUM_802_3_ICMP        2103     /* MAC - IP - ICMP (IEEE 802.3) */
#define NUM_802_3_IGMP        2104     /* MAC - IP - IGMP (IEEE 802.3) */


// Layer 4 - Dix
#define NUM_TCP_FTP_D   	200      /* MAC - IP - TCP - FTP(DATA) */
#define NUM_TCP_FTP_C   	201      /* MAC - IP - TCP - FTP(CONTROL) */
#define NUM_TCP_HTTP        202      /* MAC - IP - TCP - HTTP */
#define NUM_TCP_HTTP_7090   203      /* MAC - IP - TCP - HTTP(7090) */
#define NUM_TCP_HTTP_7091   204      /* MAC - IP - TCP - HTTP(7091) */
#define NUM_TCP_HTTP_8080   205      /* MAC - IP - TCP - HTTP(8080) */
#define NUM_TCP_DNS     	206      /* MAC - IP - TCP - DNS */
#define NUM_TCP_POP3        207      /* MAC - IP - TCP - POP3 */
#define NUM_TCP_SMTP        208      /* MAC - IP - TCP - SMTP */
#define NUM_TCP_TELNET      209      /* MAC - IP - TCP - TELNET */
#define NUM_TCP_NETBIOS_S 	210      /* MAC - IP - TCP - NETBIOS SESSIONSERVICE */
#define NUM_TCP_IMAP        211      /* MAC - IP - TCP - IMAP */

#define NUM_UDP_MIP         300      /* MAC - IP - UDP - MIP */
#define NUM_UDP_A11         301      /* MAC - IP - UDP - A11 */
#define NUM_UDP_RADIUS      302      /* MAC - IP - UDP - RADIUS */
#define NUM_UDP_NETBIOS_N 	303      /* MAC - IP - UDP - NETBIOS NAMESERVICE */
#define NUM_UDP_NETBIOS_D 	304      /* MAC - IP - UDP - NETBIOS DATAGRAM */
#define NUM_UDP_DNS     	305      /* MAC - IP - UDP - DNS */

#define NUM_GRE_PPP         400     /* MAC - IP - GRE - PPP */
#define NUM_GRE_CCP         401     /* MAC - IP - GRE - PPP_CCP */
#define NUM_GRE_LCP         402     /* MAC - IP - GRE - PPP_LCP */
#define NUM_GRE_IPCP        403   	/* MAC - IP - GRE - PPP_IPCP */
#define NUM_GRE_CHAP        404     /* MAC - IP - GRE - PPP_CHAP */
#define NUM_GRE_PAP         405     /* MAC - IP - GRE - PPP_PAP */
#define NUM_GRE_PPP_IP      406     /* MAC - IP - GRE - PPP - IP */
#define NUM_GRE_PPP_VJCTCP  407     /* MAC - IP - GRE - PPP - VJCTCP */
#define NUM_GRE_PPP_VJUCTCP 408     /* MAC - IP - GRE - PPP - VJUCTCP */


// Layer 4 - 802.3
#define NUM_802_3_TCP_FTP_D    		2200      /* MAC - IP - TCP - FTP(DATA) (IEEE 802.3) */
#define NUM_802_3_TCP_FTP_C   		2201      /* MAC - IP - TCP - FTP(CONTROL) (IEEE 802.3) */
#define NUM_802_3_TCP_HTTP        	2202      /* MAC - IP - TCP - HTTP (IEEE 802.3) */
#define NUM_802_3_TCP_HTTP_7090   	2203     /* MAC - IP - TCP - HTTP(7090) (IEEE 802.3) */
#define NUM_802_3_TCP_HTTP_7091   	2204     /* MAC - IP - TCP - HTTP(7091) (IEEE 802.3) */
#define NUM_802_3_TCP_HTTP_8080   	2205      /* MAC - IP - TCP - HTTP(8080) (IEEE 802.3) */
#define NUM_802_3_TCP_DNS     		2206      /* MAC - IP - TCP - DNS (IEEE 802.3) */
#define NUM_802_3_TCP_POP3        	2207      /* MAC - IP - TCP - POP3 (IEEE 802.3) */
#define NUM_802_3_TCP_SMTP        	2208      /* MAC - IP - TCP - SMTP (IEEE 802.3) */
#define NUM_802_3_TCP_TELNET      	2209      /* MAC - IP - TCP - TELNET (IEEE 802.3) */
#define NUM_802_3_TCP_NETBIOS_S 	2210      /* MAC - IP - TCP - NETBIOS SESSIONSERVICE (IEEE 802.3) */
#define NUM_802_3_TCP_IMAP        	2211      /* MAC - IP - TCP - IMAP (IEEE 802.3) */

#define NUM_802_3_UDP_MIP         	2300      /* MAC - IP - UDP - MIP (IEEE 802.3) */
#define NUM_802_3_UDP_A11         	2301      /* MAC - IP - UDP - A11 (IEEE 802.3) */
#define NUM_802_3_UDP_RADIUS      	2302      /* MAC - IP - UDP - RADIUS (IEEE 802.3) */
#define NUM_802_3_UDP_NETBIOS_N 	2303      /* MAC - IP - UDP - NETBIOS NAMESERVICE (IEEE 802.3) */
#define NUM_802_3_UDP_NETBIOS_D 	2304      /* MAC - IP - UDP - NETBIOS DATAGRAM (IEEE 802.3) */
#define NUM_802_3_UDP_DNS     		2305      /* MAC - IP - UDP - DNS (IEEE 802.3) */

#define NUM_802_3_GRE_PPP         	2400     /* MAC - IP - GRE - PPP (IEEE 802.3) */
#define NUM_802_3_GRE_CCP         	2401     /* MAC - IP - GRE - PPP_CCP (IEEE 802.3) */
#define NUM_802_3_GRE_LCP         	2402     /* MAC - IP - GRE - PPP_LCP (IEEE 802.3) */
#define NUM_802_3_GRE_IPCP        	2403     /* MAC - IP - GRE - PPP_IPCP (IEEE 802.3) */
#define NUM_802_3_GRE_CHAP        	2404     /* MAC - IP - GRE - PPP_CHAP (IEEE 802.3) */
#define NUM_802_3_GRE_PAP         	2405     /* MAC - IP - GRE - PPP_PAP (IEEE 802.3) */
#define NUM_802_3_GRE_PPP_IP      	2406     /* MAC - IP - GRE - PPP - IP (IEEE 802.3) */
#define NUM_802_3_GRE_PPP_VJCTCP  	2407     /* MAC - IP - GRE - PPP - VJCTCP (IEEE 802.3) */
#define NUM_802_3_GRE_PPP_VJUCTCP 	2408     /* MAC - IP - GRE - PPP - VJUCTCP (IEEE 802.3) */


// PPP Layer 1
#define NUM_PPP    			1000     /* PPP */
#define NUM_CCP    			1001     /* PPP_CCP */
#define NUM_LCP    			1002     /* PPP_LCP */
#define NUM_IPCP   			1003     /* PPP_IPCP */
#define NUM_CHAP   			1004     /* PPP_CHAP */
#define NUM_PAP    			1005     /* PPP_PAP */


// PPP Layer 2
#define NUM_PPP_IP     		1100     /* PPP - IP */


// PPP Layer 3
#define NUM_PPP_VJCTCP 		1200     /* PPP - VJCTCP */
#define NUM_PPP_VJUCTCP    	1201     /* PPP - VJUCTCP */
#define NUM_PPP_TCP         1202     /* PPP - IP - TCP */
#define NUM_PPP_UDP        	1203     /* PPP - IP - UDP */
#define NUM_PPP_ICMP       	1204     /* PPP - IP - ICMP */


// PPP Layer 4
#define NUM_PPP_TCP_FTP_D        	1300     /* PPP - IP - TCP - FTP(DATA) */
#define NUM_PPP_TCP_FTP_C        	1301     /* PPP - IP - TCP - FTP(CONTROL) */
#define NUM_PPP_TCP_HTTP       		1302     /* PPP - IP - TCP - HTTP */
#define NUM_PPP_TCP_HTTP_7090     	1303     /* PPP - IP - TCP - HTTP(7090) */
#define NUM_PPP_TCP_HTTP_7091     	1304     /* PPP - IP - TCP - HTTP(7091) */
#define NUM_PPP_TCP_HTTP_8080     	1305     /* PPP - IP - TCP - HTTP(8080) */
#define NUM_PPP_TCP_DNS       		1306     /* PPP - IP - TCP - DNS */
#define NUM_PPP_TCP_POP3       		1307     /* PPP - IP - TCP - POP3 */
#define NUM_PPP_TCP_SMTP       		1308     /* PPP - IP - TCP - SMTP */
#define NUM_PPP_TCP_TELNET     		1309     /* PPP - IP - TCP - TELNET */
#define NUM_PPP_TCP_NETBIOS_S 		1310     /* PPP - IP - TCP - NETBIOS SESSIONSERVICE */
#define NUM_PPP_TCP_IMAP       		1311     /* PPP - IP - TCP - IMAP */

#define NUM_PPP_ICMP_MIP   			1400     /* PPP - IP - ICMP - MIP */
#define NUM_PPP_UDP_MIP    			1401     /* PPP - IP - UDP - MIP */
#define NUM_PPP_UDP_NETBIOS_N 		1402     /* PPP - IP - UDP - NETBIOS NAMESERVICE */
#define NUM_PPP_UDP_NETBIOS_D 		1403     /* PPP - IP - UDP - NETBIOS DATAGRAM */
#define NUM_PPP_UDP_DNS     		1404     /* PPP - IP - UDP - DNS */
#define NUM_PPP_UDP_RADIUS     		1405     /* PPP - IP - UDP - RADIUS */

#define NUM_PPP_VJCTCP_FTP_D       	1500     /* PPP - VJCTCP - FTP(DATA) */
#define NUM_PPP_VJCTCP_FTP_C       	1501     /* PPP - VJCTCP - FTP(CONTROL) */
#define NUM_PPP_VJCTCP_HTTP       	1502     /* PPP - VJCTCP - HTTP */
#define NUM_PPP_VJCTCP_HTTP_7090   	1503     /* PPP - VJCTCP - HTTP(7090) */
#define NUM_PPP_VJCTCP_HTTP_7091   	1504     /* PPP - VJCTCP - HTTP(7091) */
#define NUM_PPP_VJCTCP_HTTP_8080   	1505     /* PPP - VJCTCP - HTTP(8080) */
#define NUM_PPP_VJCTCP_DNS       	1506     /* PPP - VJCTCP - DNS */
#define NUM_PPP_VJCTCP_POP3       	1507     /* PPP - VJCTCP - POP3 */
#define NUM_PPP_VJCTCP_SMTP       	1508     /* PPP - VJCTCP - SMTP */
#define NUM_PPP_VJCTCP_TELNET     	1509     /* PPP - VJCTCP - TELNET */
#define NUM_PPP_VJCTCP_NETBIOS_S 	1510     /* PPP - VJCTCP - NETBIOS SESSIONSERVICE */
#define NUM_PPP_VJCTCP_IMAP       	1511     /* PPP - VJCTCP - IMAP */

#define NUM_PPP_VJUCTCP_FTP_D      	1600     /* PPP - VJUCTCP - FTP(DATA) */
#define NUM_PPP_VJUCTCP_FTP_C      	1601     /* PPP - VJUCTCP - FTP(CONTROL) */
#define NUM_PPP_VJUCTCP_HTTP       	1602     /* PPP - VJUCTCP - HTTP */
#define NUM_PPP_VJUCTCP_HTTP_7090   1603     /* PPP - VJUCTCP - HTTP(7090) */
#define NUM_PPP_VJUCTCP_HTTP_7091   1604     /* PPP - VJUCTCP - HTTP(7091) */
#define NUM_PPP_VJUCTCP_HTTP_8080   1605     /* PPP - VJUCTCP - HTTP(8080) */
#define NUM_PPP_VJUCTCP_DNS       	1606     /* PPP - VJUCTCP - DNS */
#define NUM_PPP_VJUCTCP_POP3       	1607     /* PPP - VJUCTCP - POP3 */
#define NUM_PPP_VJUCTCP_SMTP       	1608     /* PPP - VJUCTCP - SMTP */
#define NUM_PPP_VJUCTCP_TELNET     	1609     /* PPP - VJUCTCP - TELNET */
#define NUM_PPP_VJUCTCP_NETBIOS_S 	1610     /* PPP - VJUCTCP - NETBIOS SESSIONSERVICE */
#define NUM_PPP_VJUCTCP_IMAP       	1611     /* PPP - VJUCTCP - IMAP */

/*******************************************************************/

#define STR_ETC         " ETC "
#define STR_UNKNOWN     " UNKNOWN PROTOCOL "

// Layer 1
#define STR_MAC         " MAC "
#define STR_DIX         " MAC (Dix Ethernet) "
#define STR_802_3       " MAC (IEEE 802.3) "

// Layer 2 - Dix
#define STR_IP          " MAC - IP "
#define STR_ARP         " MAC - ARP "
#define STR_RARP        " MAC - RARP "

// Layer 2 - 802.3
#define STR_802_3_BPDU        " MAC - BPDU (IEEE 802.3) "
#define STR_802_3_NETBEUI     " MAC - NETBEUI (IEEE 802.3) "
#define STR_802_3_IP          " MAC - IP (IEEE 802.3) "

// Layer 3 - Dix
#define STR_TCP         " MAC - IP - TCP "
#define STR_UDP         " MAC - IP - UDP "
#define STR_GRE         " MAC - IP - GRE "
#define STR_ICMP        " MAC - IP - ICMP "
#define STR_IGMP        " MAC - IP - IGMP "

// Layer 3 - 802.3
#define STR_802_3_TCP         " MAC - IP - TCP (IEEE 802.3) "
#define STR_802_3_UDP         " MAC - IP - UDP (IEEE 802.3) "
#define STR_802_3_GRE         " MAC - IP - GRE (IEEE 802.3) "
#define STR_802_3_ICMP        " MAC - IP - ICMP (IEEE 802.3) "
#define STR_802_3_IGMP        " MAC - IP - IGMP (IEEE 802.3) "


// Layer 4 - Dix
#define STR_TCP_FTP_D    	" MAC - IP - TCP - FTP(DATA) "
#define STR_TCP_FTP_C   	" MAC - IP - TCP - FTP(CONTROL) "
#define STR_TCP_HTTP        " MAC - IP - TCP - HTTP "
#define STR_TCP_HTTP_7090   " MAC - IP - TCP - HTTP(7090) "
#define STR_TCP_HTTP_7091   " MAC - IP - TCP - HTTP(7091) "
#define STR_TCP_HTTP_8080   " MAC - IP - TCP - HTTP(8080) "
#define STR_TCP_DNS     	" MAC - IP - TCP - DNS "
#define STR_TCP_POP3        " MAC - IP - TCP - POP3 "
#define STR_TCP_SMTP        " MAC - IP - TCP - SMTP "
#define STR_TCP_TELNET      " MAC - IP - TCP - TELNET "
#define STR_TCP_NETBIOS_S 	" MAC - IP - TCP - NETBIOS SESSIONSERVICE "
#define STR_TCP_IMAP        " MAC - IP - TCP - IMAP "

#define STR_UDP_MIP         " MAC - IP - UDP - MIP "
#define STR_UDP_A11         " MAC - IP - UDP - A11 "
#define STR_UDP_RADIUS      " MAC - IP - UDP - RADIUS "
#define STR_UDP_NETBIOS_N 	" MAC - IP - UDP - NETBIOS NAMESERVICE "
#define STR_UDP_NETBIOS_D 	" MAC - IP - UDP - NETBIOS DATAGRAM "
#define STR_UDP_DNS     	" MAC - IP - UDP - DNS "
#define STR_UDP_RADIUS     	" MAC - IP - UDP - RADIUS "

#define STR_GRE_PPP         " MAC - IP - GRE - PPP "
#define STR_GRE_CCP         " MAC - IP - GRE - PPP_CCP "
#define STR_GRE_LCP         " MAC - IP - GRE - PPP_LCP "
#define STR_GRE_IPCP        " MAC - IP - GRE - PPP_IPCP "
#define STR_GRE_CHAP        " MAC - IP - GRE - PPP_CHAP "
#define STR_GRE_PAP         " MAC - IP - GRE - PPP_PAP "
#define STR_GRE_PPP_IP      " MAC - IP - GRE - PPP - IP "
#define STR_GRE_PPP_VJCTCP  " MAC - IP - GRE - PPP - VJCTCP "
#define STR_GRE_PPP_VJUCTCP " MAC - IP - GRE - PPP - VJUCTCP "


// Layer 4 - 802.3
#define STR_802_3_TCP_FTP_D    		" MAC - IP - TCP - FTP(DATA) (IEEE 802.3) "
#define STR_802_3_TCP_FTP_C   		" MAC - IP - TCP - FTP(CONTROL) (IEEE 802.3) "
#define STR_802_3_TCP_HTTP        	" MAC - IP - TCP - HTTP (IEEE 802.3) "
#define STR_802_3_TCP_HTTP_7090   	" MAC - IP - TCP - HTTP(7090) (IEEE 802.3) "
#define STR_802_3_TCP_HTTP_7091   	" MAC - IP - TCP - HTTP(7091) (IEEE 802.3) "
#define STR_802_3_TCP_HTTP_8080   	" MAC - IP - TCP - HTTP(8080) (IEEE 802.3) "
#define STR_802_3_TCP_DNS     		" MAC - IP - TCP - DNS (IEEE 802.3) "
#define STR_802_3_TCP_POP3        	" MAC - IP - TCP - POP3 (IEEE 802.3) "
#define STR_802_3_TCP_SMTP        	" MAC - IP - TCP - SMTP (IEEE 802.3) "
#define STR_802_3_TCP_TELNET      	" MAC - IP - TCP - TELNET (IEEE 802.3) "
#define STR_802_3_TCP_NETBIOS_S 	" MAC - IP - TCP - NETBIOS SESSIONSERVICE (IEEE 802.3) "
#define STR_802_3_TCP_IMAP        	" MAC - IP - TCP - IMAP (IEEE 802.3) "

#define STR_802_3_UDP_MIP         	" MAC - IP - UDP - MIP (IEEE 802.3) "
#define STR_802_3_UDP_A11         	" MAC - IP - UDP - A11 (IEEE 802.3) "
#define STR_802_3_UDP_RADIUS      	" MAC - IP - UDP - RADIUS (IEEE 802.3) "
#define STR_802_3_UDP_NETBIOS_N 	" MAC - IP - UDP - NETBIOS NAMESERVICE (IEEE 802.3) "
#define STR_802_3_UDP_NETBIOS_D 	" MAC - IP - UDP - NETBIOS DATAGRAM (IEEE 802.3) "
#define STR_802_3_UDP_DNS     		" MAC - IP - UDP - DNS (IEEE 802.3) "
#define STR_802_3_UDP_RADIUS   		" MAC - IP - UDP - RADIUS (IEEE 802.3) "

#define STR_802_3_GRE_PPP         " MAC - IP - GRE - PPP (IEEE 802.3) "
#define STR_802_3_GRE_CCP         " MAC - IP - GRE - PPP_CCP (IEEE 802.3) "
#define STR_802_3_GRE_LCP         " MAC - IP - GRE - PPP_LCP (IEEE 802.3) "
#define STR_802_3_GRE_IPCP        " MAC - IP - GRE - PPP_IPCP (IEEE 802.3) "
#define STR_802_3_GRE_CHAP        " MAC - IP - GRE - PPP_CHAP (IEEE 802.3) "
#define STR_802_3_GRE_PAP         " MAC - IP - GRE - PPP_PAP (IEEE 802.3) "
#define STR_802_3_GRE_PPP_IP      " MAC - IP - GRE - PPP - IP (IEEE 802.3) "
#define STR_802_3_GRE_PPP_VJCTCP  " MAC - IP - GRE - PPP - VJCTCP (IEEE 802.3) "
#define STR_802_3_GRE_PPP_VJUCTCP " MAC - IP - GRE - PPP - VJUCTCP (IEEE 802.3) "


// PPP Layer 1
#define STR_PPP        			" PPP "
#define STR_CCP        			" PPP_CCP "
#define STR_LCP        			" PPP_LCP "
#define STR_IPCP       			" PPP_IPCP "
#define STR_CHAP       			" PPP_CHAP "
#define STR_PAP        			" PPP_PAP "


// PPP Layer 2
#define STR_PPP_IP     			" PPP - IP "


// PPP Layer 3
#define STR_PPP_VJCTCP 		" PPP - VJCTCP "
#define STR_PPP_VJUCTCP    	" PPP - VJUCTCP "
#define STR_PPP_TCP         " PPP - IP - TCP "
#define STR_PPP_UDP        	" PPP - IP - UDP "
#define STR_PPP_UDP_DNS    	" PPP - IP - UDP - DNS "
#define STR_PPP_ICMP       	" PPP - IP - ICMP "


// PPP Layer 4
#define STR_PPP_TCP_FTP_D        	" PPP - IP - TCP - FTP(DATA) "
#define STR_PPP_TCP_FTP_C        	" PPP - IP - TCP - FTP(CONTROL) "
#define STR_PPP_TCP_HTTP       		" PPP - IP - TCP - HTTP "
#define STR_PPP_TCP_HTTP_7090     	" PPP - IP - TCP - HTTP(7090) "
#define STR_PPP_TCP_HTTP_7091     	" PPP - IP - TCP - HTTP(7091) "
#define STR_PPP_TCP_HTTP_8080     	" PPP - IP - TCP - HTTP(8080) "
#define STR_PPP_TCP_DNS       		" PPP - IP - TCP - DNS "
#define STR_PPP_TCP_POP3       		" PPP - IP - TCP - POP3 "
#define STR_PPP_TCP_SMTP       		" PPP - IP - TCP - SMTP "
#define STR_PPP_TCP_TELNET     		" PPP - IP - TCP - TELNET "
#define STR_PPP_TCP_NETBIOS_S 		" PPP - IP - TCP - NETBIOS SESSIONSERVICE "
#define STR_PPP_TCP_IMAP       		" PPP - IP - TCP - IMAP "

#define STR_PPP_ICMP_MIP   			" PPP - IP - ICMP - MIP "
#define STR_PPP_UDP_MIP    			" PPP - IP - UDP - MIP"
#define STR_PPP_UDP_NETBIOS_N 		" PPP - IP - UDP - NETBIOS NAMESERVICE "
#define STR_PPP_UDP_NETBIOS_D 		" PPP - IP - UDP - NETBIOS DATAGRAM "
#define STR_PPP_UDP_DNS     		" PPP - IP - UDP - DNS "
#define STR_PPP_UDP_RADIUS     		" PPP - IP - UDP - RADIUS "

#define STR_PPP_VJCTCP_FTP_D        " PPP - VJCTCP - FTP(DATA) "
#define STR_PPP_VJCTCP_FTP_C        " PPP - VJCTCP - FTP(CONTROL) "
#define STR_PPP_VJCTCP_HTTP       	" PPP - VJCTCP - HTTP "
#define STR_PPP_VJCTCP_HTTP_7090    " PPP - VJCTCP - HTTP(7090) "
#define STR_PPP_VJCTCP_HTTP_7091    " PPP - VJCTCP - HTTP(7091) "
#define STR_PPP_VJCTCP_HTTP_8080    " PPP - VJCTCP - HTTP(8080) "
#define STR_PPP_VJCTCP_DNS       	" PPP - VJCTCP - DNS "
#define STR_PPP_VJCTCP_POP3       	" PPP - VJCTCP - POP3 "
#define STR_PPP_VJCTCP_SMTP       	" PPP - VJCTCP - SMTP "
#define STR_PPP_VJCTCP_TELNET     	" PPP - VJCTCP - TELNET "
#define STR_PPP_VJCTCP_NETBIOS_S 	" PPP - VJCTCP - NETBIOS SESSIONSERVICE "
#define STR_PPP_VJCTCP_IMAP       	" PPP - VJCTCP - IMAP "

#define STR_PPP_VJUCTCP_FTP_D       " PPP - VJUCTCP - FTP(DATA) "
#define STR_PPP_VJUCTCP_FTP_C       " PPP - VJUCTCP - FTP(CONTROL) "
#define STR_PPP_VJUCTCP_HTTP       	" PPP - VJUCTCP - HTTP "
#define STR_PPP_VJUCTCP_HTTP_7090   " PPP - VJUCTCP - HTTP(7090) "
#define STR_PPP_VJUCTCP_HTTP_7091   " PPP - VJUCTCP - HTTP(7091) "
#define STR_PPP_VJUCTCP_HTTP_8080   " PPP - VJUCTCP - HTTP(8080) "
#define STR_PPP_VJUCTCP_DNS       	" PPP - VJUCTCP - DNS "
#define STR_PPP_VJUCTCP_POP3       	" PPP - VJUCTCP - POP3 "
#define STR_PPP_VJUCTCP_SMTP       	" PPP - VJUCTCP - SMTP "
#define STR_PPP_VJUCTCP_TELNET     	" PPP - VJUCTCP - TELNET "
#define STR_PPP_VJUCTCP_NETBIOS_S 	" PPP - VJUCTCP - NETBIOS SESSIONSERVICE "
#define STR_PPP_VJUCTCP_IMAP       	" PPP - VJUCTCP - IMAP "

/*******************************************************************/

/* A11 Message Type */
#define A11_MSG_REGI_REQ		0x01	/* Registration - Request */
#define A11_MSG_REGI_REP		0x03	/* Registration - Reply */
#define A11_MSG_REGI_UPDATE		0x14	/* Registration - Update */
#define A11_MSG_REGI_ACK		0x15	/* Registration - Acknowledge */
#define A11_MSG_RP_KEEPALIVE	0xF0	/* RP KeepAlive */
#define A11_MSG_RP_REPAIR		0xF1	/* RP Repair */

#define STR_A11_MSG_REGI_REQ		" (A11) Registration - Request "
#define STR_A11_MSG_REGI_REP		" (A11) Registration - Reply "
#define STR_A11_MSG_REGI_UPDATE		" (A11) Registration - Update "
#define STR_A11_MSG_REGI_ACK		" (A11) Registration - Acknowledge "
#define STR_A11_MSG_RP_KEEPALIVE	" (A11) RP KeepAlive "
#define STR_A11_MSG_RP_REPAIR		" (A11) RP Repair "

/* MIP Message Type */
#define MIP_MSG_REGI_REQ		0x01	/* Registration - Request */
#define MIP_MSG_REGI_REP		0x03	/* Registration - Reply */

#define STR_MIP_MSG_REGI_REQ	" (MIP) Registration - Request "
#define STR_MIP_MSG_REGI_REP	" (MIP) Registration - Reply "

#endif

