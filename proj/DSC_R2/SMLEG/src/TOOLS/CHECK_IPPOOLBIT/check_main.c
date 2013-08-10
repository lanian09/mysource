#include <stdio.h>
#include <stdlib.h>
#include <ipaf_svc.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ippool_bitarray.h>

extern pst_IPPOOLLIST     pstIPPOOLBIT;

extern int	dInit_IPPOOLBIT( int dKey );
UINT dCheck_IPPOOLList( UINT uiIPAddress, UINT uiNetMask, pst_IPPOOLLIST pstIPPOOL );

int main()
{
	FILE	*fp;
	int		dRet;
	UINT	i =0, uiSum = 0;
	unsigned char	szBuffer[128];
	unsigned char	szIP1[16], szIP2[16];
	unsigned int	uiIP1, uiIP2, uiIP3, uiNetMask;
	unsigned short	usNetMask;

	dRet = dInit_IPPOOLBIT( 10264 );
	if( dRet < 0 ) {
		printf("ERROR dInit_IPPOOLBIT dRet:%d\n", dRet );
		return -1;
	}
	else
		//memset( pstIPPOOLBIT, 0x00, sizeof( st_IPPOOLLIST) );

	
	if((fp = fopen( "/DSC/NEW/DATA/IP_POOL.conf", "r" )) == NULL ) {
		printf("ERROR FILE OPEN!!\n");
		return -1;
	}

	while( fgets(szBuffer, 128, fp) != NULL ) {
		if( szBuffer[0] == '@' ) {
			if( szBuffer[1] == 'E' )
				break;
			else
				continue;
		}

		dRet = sscanf( szBuffer, "%s %s %d", szIP1, szIP2, &uiNetMask );
		if( dRet != 3 ) {
			printf("ERROR FIELD COUNT dRet:%d\n", dRet );
			break;
		}

		uiIP1 = inet_addr(szIP1);
		uiIP2 = ntohl(uiIP1);
		uiIP3 = htonl(uiIP2);

		uiIP1 = inet_network(szIP1);
		uiIP2 = inet_network(szIP2);
		usNetMask = (unsigned short)uiNetMask;

		uiNetMask = 0xffffffff << (32 - uiNetMask);

		dRet = dCheck_IPPOOLList( uiIP2, uiNetMask, pstIPPOOLBIT );
		if( dRet < 0 ) {
			printf("FAIL : IP:%16s %10u IP:%16s %10u NETMASK:%10u\n", szIP1, uiIP1, szIP2, uiIP2, uiNetMask);

			dRet = dSetIPPOOLList( uiIP2, uiNetMask, 1, pstIPPOOLBIT );
			if( dRet > 0 ) {
				dRet = dCheck_IPPOOLList( uiIP2, uiNetMask, pstIPPOOLBIT );
				if( dRet >= 0 ) {
					uiSum += dRet;
					printf("SUCCESS:%u %u IP:%s %u IP:%s %u NETMASK:%u\n", dRet, uiSum, szIP1, uiIP1, szIP2, uiIP2, uiNetMask );
				}
			}
		}
		else {
			uiSum += dRet;
			printf("SUCCESS:%u %u IP:%s %u IP:%s %u NETMASK:%u\n", dRet, uiSum, szIP1, uiIP1, szIP2, uiIP2, uiNetMask );
		}
	}

	fclose(fp);

	uiSum = 0;

	for( i=0; i<4200000000; i++ ) {
		if( _IPPOOL_ISSET(i, pstIPPOOLBIT) )
			uiSum++;
		else if( i%100000000 == 0 )
			printf("%10u %10u\n", i, uiSum );
	}

	printf("%10u\n", uiSum );

	return 1;
}

UINT dCheck_IPPOOLList( UINT uiIPAddress, UINT uiNetMask, pst_IPPOOLLIST pstIPPOOL )
{
	UINT    i;
    UINT    uiTempIP, uiTempIP1, uiTempIP2, uiTempIP3;
	UINT	uiNet1, uiNet2;
	UINT	uiSuccess = 0, uiFail = 0;

    uiTempIP1 = 0;
    uiTempIP2 = 0xFFFFFFFF;
    uiTempIP3 = 0;

	uiNet1 = uiNetMask;

    uiTempIP = uiIPAddress;
	uiTempIP1 = (uiTempIP & uiNet1);

	uiNet2 = ~(uiNet1);
	uiTempIP2 = (uiTempIP | uiNet2);

    for( i=uiTempIP1; i<=uiTempIP2; i++ ) {
        if( _IPPOOL_ISSET( i, pstIPPOOL ) )
			uiSuccess++;
		else
			uiFail++;
	}

	if( uiFail != 0 )
		return -1;

	return uiSuccess;
}
