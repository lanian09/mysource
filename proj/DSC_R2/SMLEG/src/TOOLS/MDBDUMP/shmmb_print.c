#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "ipaf_svc.h"
#include "mmdb_destip.h"
#include "mmdb_destport.h"
#include "mmdb_svcopt.h"


#define IPTYPE_IPPOOL   0

#define PRINT_IP(b)		\
{ \
	struct in_addr addr; \
 \
	addr.s_addr = b; \
	printf( "%-15s", inet_ntoa(addr)); \
}

#define PRINT_PORT(a)	printf( "%5d", a);
#define PRINT_SVCOPT(a)	printf( "%5d", a);
#define PRINT_SVCTYPE(a)	printf( "%5d", a);
#define PRINT_CATEGORY(a)	printf( "%8d", a);
#define PRINT_SVCBLOCK(a) \
{ \
    char *cp = NULL; \
    switch(a){ \
        case 1: \
            cp = "CDR"; \
            break; \
        case 2: \
            cp = "WAP1ANA"; \
            break; \
        case 3: \
            cp = "UAWAPANA"; \
            break; \
        case 4: \
            cp = "WAP2ANA"; \
            break; \
        case 5: \
            cp = "HTTPANA"; \
            break; \
        case 6: \
            cp = "VODSANA"; \
            break; \
        case 7: \
            cp = "WIPINWANA"; \
            break; \
        case 8: \
            cp = "JAVANWANA"; \
            break; \
        case 9: \
            cp = "LOGM"; \
            break; \
        case 10: \
            cp = "VTANA"; \
            break; \
    } \
 \
    printf( "%-8s", cp); \
}

#define PRINT_ONOFF(a) printf("%d(%-3s)", a, (a == 0)?"OFF":"ON");

#define PRINT_FLAG(a) printf( "%c(%-11s)", a, (a == 'S')?"SOURCE":"DESTINATION");
#define PRINT_IPTYPE(a) printf( "%d(%-7s)", a, (a == IPTYPE_IPPOOL)?"IP POOL":"PDSN");
#define PRINT_PROTOCOL(a) \
{ \
	if(a == 6) \
		printf( "%d(%-7s)", a, "TCP"); \
	else if(a == 17) \
		printf( "%d(%-7s)", a, "UDP"); \
	else	\
		printf( "%d(%-7s)", a, "UNKNOWN"); \
}

#define PRINT_LAYER(a)	\
{ \
	if(a == 1) \
		printf( "%d(%-7s)", a, "IP"); \
	else if(a == 2) \
		printf( "%d(%-7s)", a, "TCP"); \
	else if(a == 3) \
		printf( "%d(%-7s)", a, "UDP"); \
	else if(a == 21) \
		printf( "%d(%-7s)", a, "DEFAULT"); \
	else	\
		printf( "%d(%-7s)", a, "UNKNOWN"); \
}

#define PRINT_DESTIP(a, b) \
{ \
	printf( "%3d", b); \
    printf(" "); \
	PRINT_IP(a##.key.uiIP) \
    printf(" "); \
	PRINT_FLAG(a##.key.ucFlag) \
    printf(" "); \
	PRINT_IP(a##.uiNetmask) \
    printf("  "); \
	PRINT_IPTYPE(a##.ucIPType) \
	printf("\n"); \
}

#define PRINT_DESTIP1(a, b) \
{ \
	printf( "%3d", b); \
    printf(" "); \
	PRINT_IP(a##.key.uiIP) \
    printf(" "); \
	PRINT_FLAG(a##.key.ucFlag) \
    printf(" "); \
	PRINT_IP(a##.uiNetmask) \
    printf("  "); \
    PRINT_CATEGORY(a##.usCatID); \
    printf("    "); \
    PRINT_LAYER(a##.ucLayer); \
    printf(" "); \
    PRINT_ONOFF(a##.ucFilterOut); \
	printf("\n"); \
}

#define PRINT_DESTPORT(a, b) \
{ \
	printf( "%3d", b); \
    printf(" "); \
	PRINT_PROTOCOL(a##.key.ucProtocol) \
    printf(" "); \
	PRINT_IP(a##.key.uiDestIP) \
    printf(" "); \
    PRINT_PORT(a##.key.usDestPort); \
    printf("  "); \
	PRINT_IP(a##.uiNetmask) \
    printf(" "); \
    PRINT_CATEGORY(a##.usCatID); \
    printf("    "); \
    PRINT_LAYER(a##.ucLayer); \
    printf(" "); \
    PRINT_SVCBLOCK(a##.ucSvcBlk); \
    printf("  "); \
    PRINT_ONOFF(a##.ucFilterOut); \
	printf( "\n"); \
}

#if 0
#define PRINT_SVCOPTION(a, b) \
{ \
    printf( "%3d", b); \
    printf(" "); \
    PRINT_SVCOPT(a##.key.svcOpt) \
    printf("     "); \
    PRINT_SVCTYPE(a##.svcType) \
    printf("      "); \
    PRINT_LAYER(a##.layer) \
    printf(" "); \
    PRINT_ONOFF(a##.fout) \
    printf("  "); \
    PRINT_SVCBLOCK(a##.block) \
    printf("   "); \
    PRINT_ONOFF(a##.urlCha) \
	printf( "\n"); \
}
#endif


int             semid_destip=-1;                /* semaphore id for MMDB_DESTIP */
int             semid_destport =-1;
int             semid_svcopt =-1;

DESTPORT_TABLE  *destport_tbl;
DESTIP_TABLE    *destip_tbl;
SVCOPT_TABLE    *svcopt_tbl;




int main()
{

    int				key, shmId, count;

	DESTIP_DATA     DestIP, *pDestIP;
	DESTIP_KEY      DestIPKey1, DestIPKey2;

	DESTPORT_DATA   DestPort, *pDestPort;
	DESTPORT_KEY    DestPortKey1, DestPortKey2;
#if 0
    SVCOPT_DATA     SvcOpt, *pSvcOpt;
    SVCOPT_KEY      SvcOptKey1, SvcOptKey2;
#endif

    key = strtol("0x2815",0,0);

    if ((shmId = (int)shmget (key, sizeof(DESTPORT_TABLE), 0644)) < 0) {
        if (errno != ENOENT) {
            fprintf (stderr,"[mmcr_init] shmget fail; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
            return -1;
        }
    }

    if ((destport_tbl = (DESTPORT_TABLE*) shmat (shmId,0,0)) == (DESTPORT_TABLE*)-1) {
        fprintf (stderr,"[mmdr_init] shmat fail; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
        return -1;
    }

    key = strtol("0x2814",0,0);

    if ((shmId = (int)shmget (key, sizeof(DESTIP_TABLE), 0644)) < 0) {
        if (errno != ENOENT) {
            fprintf (stderr,"[mmcr_init] shmget fail; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
            return -1;
        }
    }

    if ((destip_tbl = (DESTIP_TABLE*) shmat (shmId,0,0)) == (DESTIP_TABLE*)-1) {
        fprintf (stderr,"[mmdr_init] shmat fail; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
        return -1;
    }

#if 0
    key = strtol("0x5121",0,0);

    if ((shmId = (int)shmget (key, sizeof(SVCOPT_TABLE), 0644)) < 0) {
        if (errno != ENOENT) {
            fprintf (stderr,"[mmcr_init] shmget fail; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
            return ;
        }
    }

    if ((svcopt_tbl = (SVCOPT_TABLE*) shmat (shmId,0,0)) == (SVCOPT_TABLE*)-1) {
        fprintf (stderr,"[mmdr_init] shmat fail; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
        return ;
    }
#endif

	fprintf(stderr, "================================= DEST IP START ===================================\n");
    printf("NO. IP-Address      FLAG           SUBNET-MASK      IP-TYPE \n"); \
	memset( &DestIPKey1, 0x00, sizeof(DESTIP_KEY) );
	DestIPKey1.ucFlag = 'S';

	memset( &DestIPKey2, 0xff, sizeof(DESTIP_KEY) );
	DestIPKey2.ucFlag = 'S';

	count = 0;
	while(1){
		pDestIP = Select_DESTIP( &DestIPKey1, &DestIPKey2 );
		if( pDestIP == 0 || pDestIP->key.ucFlag != 'S')
			break;
        count++;
		memcpy( &DestIPKey1, &pDestIP->key, sizeof(DESTIP_KEY) );
		memcpy( &DestIP, pDestIP, sizeof(DESTIP_DATA) );
		PRINT_DESTIP(DestIP, count);
	}

	fprintf(stderr, "================================== DEST IP END ====================================\n");

	fprintf(stderr, "================================= DEST PORT(IP) START ===================================\n");
    printf("NO. IP-Address      FLAG           SUBNET-MASK      CATEGORY-ID LAYER       FLT-OUT\n"); \
	memset( &DestIPKey1, 0x00, sizeof(DESTIP_KEY) );
	DestIPKey1.ucFlag = 'D';

	memset( &DestIPKey2, 0xff, sizeof(DESTIP_KEY) );
	DestIPKey2.ucFlag = 'D';

	count = 0;
	while(1){
		pDestIP = Select_DESTIP( &DestIPKey1, &DestIPKey2 );
		if( pDestIP == 0 || pDestIP->key.ucFlag != 'D')
			break;
        count++;
		memcpy( &DestIPKey1, &pDestIP->key, sizeof(DESTIP_KEY) );
		memcpy( &DestIP, pDestIP, sizeof(DESTIP_DATA) );
		PRINT_DESTIP1(DestIP, count);
	}

	fprintf(stderr, "=============================== DEST PORT(IP) END =================================\n");

	fprintf(stderr, "============================== DEST PORT(TCP/UDP) START ================================\n");
    printf("NO. PROTOCOL   IP-Address       PORT  SUBNET-MASK     CATEGORY-ID LAYER      SVC-BLOCK FLT-OUT\n"); \
	memset( &DestPortKey1, 0x00, sizeof(DESTPORT_KEY) );
	memset( &DestPortKey2, 0xff, sizeof(DESTPORT_KEY) );

	count = 0;
	while(1){
		pDestPort = Select_DESTPORT( &DestPortKey1, &DestPortKey2 );
        if(pDestPort == 0)
            break;
        count++;
		memcpy( &DestPortKey1, &pDestPort->key, sizeof(DESTPORT_KEY) );
		memcpy( &DestPort, pDestPort, sizeof(DESTPORT_DATA) );

		PRINT_DESTPORT(DestPort, count)
	}

	fprintf(stderr, "=============================== DEST PORT(TCP/UDP) END =================================\n");

#if 0
	fprintf(stderr, "============================== SERVICE OPTION START ================================\n"); 
    printf("NO. SVC-OPT   SVC-TYPE   LAYER      F-OUT   SVC-BLOCK   URL-CHA   \n");
	memset( &SvcOptKey1, 0x00, sizeof(SVCOPT_KEY) );
	memset( &SvcOptKey2, 0xff, sizeof(SVCOPT_KEY) );

	count = 0;
	while(1){
		pSvcOpt = Select_SVCOPT( &SvcOptKey1, &SvcOptKey2 );
        if(pSvcOpt == 0)
            break;
        count++;
		memcpy( &SvcOptKey1, &pSvcOpt->key, sizeof(SVCOPT_KEY) );
		memcpy( &SvcOpt, pSvcOpt, sizeof(SVCOPT_DATA) );
		PRINT_SVCOPTION(SvcOpt, count)
    }
	fprintf(stderr, "=============================== SERVICE OPTION END =================================\n");
#endif

    return 0;
 
}
