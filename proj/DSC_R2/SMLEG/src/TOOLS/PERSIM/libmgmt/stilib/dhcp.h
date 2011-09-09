#ifndef DHCP_H
#define DHCP_H

#define MAGIC   0x63825363

#define DHCP_MESSAGE_TYPE   0x35
#define DHCP_SERVER_ID      0x36
#define DHCP_CLIENT_ID      0x3d

/* op */
#define BOOTREQUEST     1
#define BOOTREPLY       2

/* htype & hlen */
#define ETH_10MB        1
#define ETH_10MB_LEN    6

/* Message Type */
#define DHCPDISCOVER    1
#define DHCPOFFER       2
#define DHCPREQUEST     3
#define DHCPDECLINE     4
#define DHCPACK         5
#define DHCPNAK         6
#define DHCPRELEASE     7
#define DHCPINFORM      8

#define DHCPOPTLEN		308

typedef struct dhcp {
    unsigned char 		op;
    unsigned char 		htype;
    unsigned char 		hlen;
    unsigned char 		hops;
    unsigned int 		xid;
    unsigned short 		secs;
    unsigned short 		flags;
    unsigned int 		ciaddr;
    unsigned int 		yiaddr;
    unsigned int 		siaddr;
    unsigned int 		giaddr;
    unsigned char 		chaddr[16];
    unsigned char 		sname[64];
    unsigned char 		file[128];
    unsigned int 		cookie;
    unsigned char 		options[DHCPOPTLEN];
} dhcp_t;

#define DHCPHDRSZ   (sizeof(struct dhcp)-DHCPOPTLEN)

/* DHCP option and value (cf. RFC1533) */
enum
{
        padOption               =   0,
        subnetMask              =   1,
        timerOffset             =   2,
        routersOnSubnet         =   3,
        timeServer              =   4,
        nameServer              =   5,
        dnsServer               =   6,
        logServer               =   7,
        cookieServer            =   8,
        lprServer               =   9,
        impressServer           =   10,
        resourceLocationServer  =   11,
        hostName                =   12,
        bootFileSize            =   13,
        meritDumpFile           =   14,
        domainName              =   15,
        swapServer              =   16,
        rootPath                =   17,
        extentionsPath          =   18,
        IPforwarding            =   19,
        nonLocalSourceRouting   =   20,
        policyFilter            =   21,
        maxDgramReasmSize       =   22,
        defaultIPTTL            =   23,
        pathMTUagingTimeout     =   24,
        pathMTUplateauTable     =   25,
        ifMTU                   =   26,
        allSubnetsLocal         =   27,
        broadcastAddr           =   28,
        performMaskDiscovery    =   29,
        maskSupplier            =   30,
        performRouterDiscovery  =   31,
        routerSolicitationAddr  =   32,
        staticRoute             =   33,
        trailerEncapsulation    =   34,
        arpCacheTimeout         =   35,
        ethernetEncapsulation   =   36,
        tcpDefaultTTL           =   37,
        tcpKeepaliveInterval    =   38,
        tcpKeepaliveGarbage     =   39,
        nisDomainName           =   40,
        nisServers              =   41,
        ntpServers              =   42,
        vendorSpecificInfo      =   43,
        netBIOSnameServer       =   44,
        netBIOSdgramDistServer  =   45,
        netBIOSnodeType         =   46,
        netBIOSscope            =   47,
        xFontServer             =   48,
        xDisplayManager         =   49,
        dhcpRequestedIPaddr     =   50,
        dhcpIPaddrLeaseTime     =   51,
        dhcpOptionOverload      =   52,
        dhcpMessageType         =   53,
        dhcpServerIdentifier    =   54,
        dhcpParamRequest        =   55,
        dhcpMsg                 =   56,
        dhcpMaxMsgSize          =   57,
        dhcpT1value             =   58,
        dhcpT2value             =   59,
        dhcpClassIdentifier     =   60,
        dhcpClientIdentifier    =   61,
        endOption               =  255
};

#define DHCP_SPORT	67
#define DHCP_CPORT	68

#define DHCP_INIT			1
#define DHCP_SELECTING		2
#define DHCP_REQUESTING		3
#define DHCP_BOUND			4
#define DHCP_RENEWING		5
#define DHCP_REBINDING		6
#define DHCP_REBOOTING		7
#define DHCP_INIT_REBOOT	8

typedef struct dhcp_fsm {
	unsigned int prev;
	unsigned int event;
	unsigned int act;
	unsigned int next;
} dhcp_fsm_t;

#define DHCP_NOP			0
#define DHCP_UP				1
#define DHCP_DOWN			2
#define DHCP_LEASE			3
#define SND_DHCPDISCOVER	4
#define SND_DHCPREQUEST		5
#define RCV_DHCPOFFER		6
#define RCV_DHCPACK			7
#define RCV_DHCPNAK			8
#define DHCP_T1_TMO			9
#define DHCP_T2_TMO			10
#define DHCP_TMO			11

extern dhcp_fsm_t dhcpfsm[];
#endif
