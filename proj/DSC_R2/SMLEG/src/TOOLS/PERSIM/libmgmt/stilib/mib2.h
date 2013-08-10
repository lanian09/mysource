#ifndef STI_MIB2_H
#define STI_MIB2_H

#ifdef SunOS

#include <inet/mib2.h>

typedef unsigned long TimeTicks;

typedef struct mib2_ifEntry {
    int     	ifIndex;        	/* ifEntry 1 */
    DeviceName  ifDescr;        	/* ifEntry 2 */
    int     	ifType;         	/* ifEntry 3 */
    int     	ifMtu;          	/* ifEntry 4 */
    Gauge   	ifSpeed;        	/* ifEntry 5 */
    PhysAddress ifPhysAddress;      /* ifEntry 6 */
    int     	ifAdminStatus;      /* ifEntry 7 */
    int     	ifOperStatus;       /* ifEntry 8 */
    TimeTicks   ifLastChange;       /* ifEntry 9 */
    Counter 	ifInOctets;     	/* ifEntry 10 */
    Counter 	ifInUcastPkts;      /* ifEntry 11 */
    Counter 	ifInNUcastPkts;     /* ifEntry 12 */
    Counter 	ifInDiscards;       /* ifEntry 13 */
    Counter 	ifInErrors;     	/* ifEntry 14 */
    Counter 	ifInUnknownProtos;  /* ifEntry 15 */
    Counter 	ifOutOctets;        /* ifEntry 16 */
    Counter 	ifOutUcastPkts;     /* ifEntry 17 */
    Counter 	ifOutNUcastPkts;    /* ifEntry 18 */
    Counter 	ifOutDiscards;      /* ifEntry 19 */
    Counter 	ifOutErrors;        /* ifEntry 20 */
    Gauge   	ifOutQLen;      	/* ifEntry 21 */
    int     	ifSpecific;     	/* ifEntry 22 */
} mib2_ifEntry_t;

#endif

#ifdef Linux

#include <stdint.h>

/* SunOS.... inet/mib2.h	*/
#define OCTET_LENGTH    32  /* Must be at least LIFNAMSIZ */
typedef struct Octet_s {
    int o_length;
    char    o_bytes[OCTET_LENGTH];
} Octet_t;

typedef uint32_t 	Counter;
typedef uint32_t    Counter32;
typedef uint32_t    Gauge;
typedef uint32_t    IpAddress;
typedef struct in6_addr Ip6Address;
typedef Octet_t     DeviceName;
typedef Octet_t     PhysAddress;
typedef uint32_t    DeviceIndex;    /* Interface index */

typedef unsigned long TimeTicks;

typedef struct mib2_ifEntry {
    int     	ifIndex;        	/* ifEntry 1 */
    DeviceName  ifDescr;        	/* ifEntry 2 */
    int     	ifType;         	/* ifEntry 3 */
    int     	ifMtu;          	/* ifEntry 4 */
    Gauge   	ifSpeed;        	/* ifEntry 5 */
    PhysAddress ifPhysAddress;      /* ifEntry 6 */
    int     	ifAdminStatus;      /* ifEntry 7 */
    int     	ifOperStatus;       /* ifEntry 8 */
    TimeTicks   ifLastChange;       /* ifEntry 9 */
    Counter 	ifInOctets;     	/* ifEntry 10 */
    Counter 	ifInUcastPkts;      /* ifEntry 11 */
    Counter 	ifInNUcastPkts;     /* ifEntry 12 */
    Counter 	ifInDiscards;       /* ifEntry 13 */
    Counter 	ifInErrors;     	/* ifEntry 14 */
    Counter 	ifInUnknownProtos;  /* ifEntry 15 */
    Counter 	ifOutOctets;        /* ifEntry 16 */
    Counter 	ifOutUcastPkts;     /* ifEntry 17 */
    Counter 	ifOutNUcastPkts;    /* ifEntry 18 */
    Counter 	ifOutDiscards;      /* ifEntry 19 */
    Counter 	ifOutErrors;        /* ifEntry 20 */
    Gauge   	ifOutQLen;      	/* ifEntry 21 */
    int     	ifSpecific;     	/* ifEntry 22 */
} mib2_ifEntry_t;

#endif

#endif



