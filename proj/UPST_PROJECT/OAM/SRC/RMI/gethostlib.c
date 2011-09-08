/*******************************************************************************
			DQMS Project

	Author   : 
	Section  : RMI
	SCCS ID  : @(#)gethostlib.c	1.1
	Date     : 07/21/01
	Revision History :
        '01.  7. 21     Initial
        '03.  1. 15     Initial

	Description :

	Copyright (c) uPRESTO 2005
*******************************************************************************/

/***** A.1 * File Include *******************************/

/* SYS HEADER */
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
/* LIB HEADER */
/* PRO HEADER */
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
/* LOC HEADER */

extern int errno;

int dGetHostIP(char *ipaddr)
{
	char			name[1024], **p;
	int				ret;
	size_t			namelen=1024;
	struct hostent	*host;
	struct in_addr	in;

	memset(name, 0x00, 1024);
	if( (ret = gethostname(&name[0], namelen)) < 0 )
	{
		printf("GetHostName Error[%s]\n", strerror(errno));
		return -1;
	}
	printf("HOST NAME[%s]\n", name);

	if( (host = gethostbyname(name)) == NULL)
	{
		printf("gethostbyname error\n");
		return -1;
	}

	for(p = host->h_addr_list; *p != 0; p++)
	{
		(void)memcpy(&in.s_addr, *p, sizeof(in.s_addr));
		sprintf(ipaddr, "%s", inet_ntoa(in) );
	}
	printf("IP ADDRESS[%s]\n", ipaddr);

	return 0;
}
