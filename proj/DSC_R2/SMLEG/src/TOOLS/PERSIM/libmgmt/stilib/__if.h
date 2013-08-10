#ifndef __IF_H
#define __IF_H

#define IFSZ	20

#define IF_NAME_FLD		0
#define IF_MAC_FLD		1
#define IF_ADDR_FLD		2
#define IF_BCAST_FLD	3
#define IF_MASK_FLD		4

#define IF_MAX_FLD		IF_MASK_FLD+1

typedef struct __if {
	char	fld[IF_MAX_FLD][IFSZ];
	unsigned char 	mac[6];		/* hardware address		*/
	unsigned int 	addr;		/* IPv4 address			*/
	unsigned int 	bcast;		/* broadcast address	*/
	unsigned int 	mask;		/* netmask				*/
	int		mbl;				/* maskbit length		*/
	int		mtu;				/* interface mtu 		*/
	char	u:1,
			b:1,
			r:1,
			m:1,
			l:1,
			spare:3;		/* interface flags		*/
} if_t;

#define MAXIF	16

extern if_t    ifs[MAXIF];
extern int getifs();


/* ethtool 	*/
#ifndef _LINUX_ETHTOOL_H
#define _LINUX_ETHTOOL_H

#ifndef U32
#define U32
typedef unsigned int u32;
#endif
#ifndef U16
#define U16
typedef unsigned short u16;
#endif
#ifndef U8
#define U8
typedef unsigned char u8;
#endif

/* This should work for both 32 and 64 bit userland. */
struct ethtool_cmd {
    u32 cmd;
    u32 supported;  /* Features this interface supports */
 	u32 advertising;    /* Features this interface advertises */
	u16 speed;      /* The forced speed, 10Mb, 100Mb, gigabit */
	u8  duplex;     /* Duplex, half or full */
	u8  port;       /* Which connector port */
	u8  phy_address;
	u8  transceiver;    /* Which tranceiver to use */
	u8  autoneg;    /* Enable or disable autonegotiation */
	u32 maxtxpkt;   /* Tx pkts before generating tx int */
	u32 maxrxpkt;   /* Rx pkts before generating rx int */
	u32 reserved[4];
};

#define ETHTOOL_BUSINFO_LEN 32
/* these strings are set to whatever the driver author decides... */
struct ethtool_drvinfo {
	u32 cmd;
	char    driver[32]; /* driver short name, "tulip", "eepro100" */
    char    version[32];    /* driver version string */
    char    fw_version[32]; /* firmware version string, if applicable */
    char    bus_info[ETHTOOL_BUSINFO_LEN];  /* Bus info for this IF. */
			                /* For PCI devices, use pci_dev->slot_name. */
    char    reserved1[32];
    char    reserved2[16];
    u32 n_stats;    /* number of u64's from ETHTOOL_GSTATS */
    u32 testinfo_len;
    u32 eedump_len; /* Size of data from ETHTOOL_GEEPROM (bytes) */
    u32 regdump_len;    /* Size of data from ETHTOOL_GREGS (bytes) */
};

#define SOPASS_MAX  6
/* wake-on-lan settings */
struct ethtool_wolinfo {
    u32 cmd;
    u32 supported;
    u32 wolopts;
    u8  sopass[SOPASS_MAX]; /* SecureOn(tm) password */
};

/* for passing single values */
struct ethtool_value {
    u32 cmd;
    u32 data;
};

/* CMDs currently supported */
#define ETHTOOL_GSET        0x00000001 /* Get settings. */
#define ETHTOOL_SSET        0x00000002 /* Set settings, privileged. */
#define ETHTOOL_GDRVINFO    0x00000003 /* Get driver info. */
#define ETHTOOL_GREGS       0x00000004 /* Get NIC registers, privileged. */
#define ETHTOOL_GWOL        0x00000005 /* Get wake-on-lan options. */
#define ETHTOOL_SWOL        0x00000006 /* Set wake-on-lan options, priv. */
#define ETHTOOL_GMSGLVL     0x00000007 /* Get driver message level */
#define ETHTOOL_SMSGLVL     0x00000008 /* Set driver msg level, priv. */
#define ETHTOOL_NWAY_RST    0x00000009 /* Restart autonegotiation, priv. */
#define ETHTOOL_GLINK       0x0000000a /* Get link status (ethtool_value) */
#define ETHTOOL_GEEPROM     0x0000000b /* Get EEPROM data */
#define ETHTOOL_SEEPROM     0x0000000c /* Set EEPROM data, priv. */
#define ETHTOOL_GCOALESCE   0x0000000e /* Get coalesce config */
#define ETHTOOL_SCOALESCE   0x0000000f /* Set coalesce config, priv. */
#define ETHTOOL_GRINGPARAM  0x00000010 /* Get ring parameters */
#define ETHTOOL_SRINGPARAM  0x00000011 /* Set ring parameters, priv. */
#define ETHTOOL_GPAUSEPARAM 0x00000012 /* Get pause parameters */
#define ETHTOOL_SPAUSEPARAM 0x00000013 /* Set pause parameters, priv. */
#define ETHTOOL_GRXCSUM     0x00000014 /* Get RX hw csum enable (ethtool_value) */
#define ETHTOOL_SRXCSUM     0x00000015 /* Set RX hw csum enable (ethtool_value) */
#define ETHTOOL_GTXCSUM     0x00000016 /* Get TX hw csum enable (ethtool_value) */
#define ETHTOOL_STXCSUM     0x00000017 /* Set TX hw csum enable (ethtool_value) */
#define ETHTOOL_GSG     	0x00000018 /* Get scatter-gather enable
									                          * (ethtool_value) */
#define ETHTOOL_SSG     	0x00000019 /* Set scatter-gather enable
									                          * (ethtool_value), priv. */
#define ETHTOOL_TEST        0x0000001a /* execute NIC self-test, priv. */
#define ETHTOOL_GSTRINGS    0x0000001b /* get specified string set */
#define ETHTOOL_PHYS_ID     0x0000001c /* identify the NIC */
#define ETHTOOL_GSTATS      0x0000001d /* get NIC-specific statistics */
#define ETHTOOL_GTSO        0x0000001e /* Get TSO enable (ethtool_value) */
#define ETHTOOL_STSO        0x0000001f /* Set TSO enable (ethtool_value) */

/* The forced speed, 10Mb, 100Mb, gigabit, 10GbE. */
#define SPEED_10        10
#define SPEED_100       100
#define SPEED_1000      1000
#define SPEED_10000     10000

/* Duplex, half or full. */
#define DUPLEX_HALF     0x00
#define DUPLEX_FULL     0x01

#endif	/* _LINUX_ETHTOOL_H	*/

#endif	/* __IF_H			*/
