#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <signal.h>
#include <errno.h>


#include "radius_define.h"


#define DEF_ACCAAA_ATTR         0x1A
#define DEF_ACCAAA_VEND         0x0000159F 

#define SELECT_PERIOD       10000
#define AUTH1  ((long long)0xcef20c4920948119)
#define AUTH2  ((long long)0x602705ea971d2bcc)

/* challa --> */
//unsigned int  llRecvCnt=0;
unsigned char TimeFlg = 1;
int    tFirstTime;
int    tLastTime=0;
 /* <--- */



int sockfd;
int	cliport;
int	chk_time_sec;
int	prt_msg;
long long llRecvCnt=0;
long long llSendCnt=0;
long long llTotalRecvCnt=0;
long long llTotalSendCnt=0;

int init_conf();
void init_signal();
const char *get_time();
void siginttrt(int nSig);
int  dMakeAcctResponseADR( UCHAR,UCHAR, INT *, UCHAR * );

int dMakeRADIUSDecimal( int dVendType, UCHAR ucType, UCHAR ucLen, UCHAR ucValue, 
                        USHORT usValue, int dValue, INT64 llValue, char *szBody);

int dMakeAcctResponse( UCHAR ucID, UINT UDRSeq, char *szAuth, 
                       INT *dMsgLen, UCHAR *szAcctResMsg );


//extern char *cvt_ipaddr(UINT uiIP);


int main(int argc, char **argv)
{
    int 	clilen;
    int 	state;
	int 	dAdr=0;
	int 	sendlen;
	int 	sret;


    UINT    ulFromIP;
    int     dPort, i;

	char    szSecret[4];
    char 	buf[5000];
	UCHAR   szAuth[16];


	time_t  checktime=0;
	time_t  nowtime;
	time_t  tCurTime,tCmpTime;

    struct sockaddr_in serveraddr, ownaddr;
    struct sockaddr_in cliaddr;
    struct in_addr	tmp_addr;
	st_ACCInfo stRDPkt;

	int	nSel;
	int	dRet;
	fd_set          Rd;
	struct timeval  timeout;

    if (init_conf() < 0)
    {
        fprintf(stderr,"[%s] !!! ERROR: configuration initialization error\n",get_time());
        exit(1);
    }

	signal( SIGINT, siginttrt );

	/* MD5 CREAT */
	strncpy( szSecret, "foo", 3 );
	szSecret[3] = '\0';

    memset(buf, 0x10, 5000); 

    clilen = sizeof(serveraddr);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd < 0)
    {
        perror("socket error : ");
        exit(0);
    }
#if 0
	dRet = fcntl( sockfd, F_SETFD, O_NONBLOCK );
	if( dRet < 0 )
	{   
		fprintf(stderr, "[FAIL] fcntl Error : %s ", strerror(errno));
	    exit(0);
	 }
#endif
	 

	//if (set_sockopt(sockfd,1024,1024) < 0)
	if (set_sockopt(sockfd,256,256) < 0)
    {
        fprintf(stderr,"[%s] !!! ERROR: socket option error\n",get_time());
        return -1;
    }
    
	bzero( &ownaddr, sizeof(ownaddr) );
	ownaddr.sin_family = AF_INET;
	ownaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	ownaddr.sin_port = htons(cliport);

	fprintf(stderr,"check cliport : %d\n",cliport);
	sret = bind( sockfd, (struct sockaddr*)&ownaddr, sizeof(ownaddr) );  
	if( sret < 0 )
	{
		perror("bind");
		close(sockfd);

		exit(0);
	}

	log_debug("### START SIM ###");
	printf("START SIMULATOR >>> \n");

	while(1)
	{
		sret = 0;
		memset( &buf[0], 0x00, sizeof(buf) );
#if 0
		time(&nowtime);

		if( nowtime - checktime >= chk_time_sec )
		{
			log_debug("STAT CURRENT :  SendCnt[%llu] RecvCnt[%llu] per %d Seconds", llSendCnt, llRecvCnt, chk_time_sec );
			//fprintf(stderr,"######## nowtime[%d], checktime[%d], #######\n",nowtime,checktime);
			checktime = nowtime;
			llRecvCnt=0;
			llSendCnt=0;
		}
#endif


/* 	Perfomance Porcess..(0915) */

#if 0
		time(&nowtime);

		if( nowtime != tLastTime )
			fprintf(stderr,"##time[%d]CNT[%llu]\n", nowtime, llRecvCnt ); 

		tLastTime = nowtime;
#endif
			
    	timeout.tv_sec = 0;
    	timeout.tv_usec = SELECT_PERIOD;

		FD_ZERO( &Rd );
		FD_SET( sockfd, &Rd );
		nSel = select( sockfd+1, &Rd, NULL, NULL, &timeout );

	if( nSel > 0 ) 
    {
 		if( FD_ISSET( sockfd, &Rd ) )
			sret = recvfrom( sockfd, (void*)&buf[0], 3000, 0, (struct sockaddr *)&cliaddr, &clilen );	
		if( sret > 0 )
		{
			tmp_addr.s_addr = cliaddr.sin_addr.s_addr;
			//fprintf(stdout, "Recv Succ Len[%d] IP[%s]\n", sret, inet_ntoa(tmp_addr));
			llRecvCnt++;
			//llTotalRecvCnt++;

			/*
			if( TimeFlg == 1 )
			{
				time(&tCurTime);
				tFirstTime = tCurTime;
				TimeFlg = 0;
			}
			*/

            ulFromIP    = cliaddr.sin_addr.s_addr;
            dPort       = ntohs(cliaddr.sin_port);
			
			/*
			fprintf(stderr, "BUF HEXA : ");
			for(i=0; i<20; i++)
			{
				fprintf(stderr,"%02x ", buf[i]);
			}	
			printf("\n");
			*/


			sret =	ParsingRadius( &buf[0], sret, ulFromIP, &stRDPkt, &dAdr  );
			if( sret == 0 )
			{
				sret =  dMakeAcctResponse( stRDPkt.ucID, stRDPkt.uiUDRSeq, stRDPkt.szAuthen, 
                                           &sendlen, (UCHAR*)&buf[0] );

#if 1
				dCreateAuthMD5( &szAuth[0], szSecret, sendlen, (UCHAR*)&buf[0] );
				memcpy( (UCHAR*)&buf[4], &szAuth[0], 16 );
#endif

                sret = SendToBSD( ulFromIP, dPort, sendlen, (void*)&buf[0] );
			    if( sret < 0 )
    			{
        			fprintf(stderr,"SEND FAIL\n"); 
        			return -1;
    			}
                

				//llSendCnt++;
				//llTotalSendCnt++;

			}

#if 0
			else
			{
				printf(" CRITICAL 잘못된 데이터 INVALID MESSAGE\n" );
				if( dAdr > 0 )
				{
					sret =  dMakeAcctResponseADR( stRDPkt.ucID,  dAdr, &sendlen, (UCHAR*)&buf[0] );
					printf("INVALID MESSAGE[%d]\n", dAdr );
    				sret =	sendto(sockfd, (void *)&buf[0], sendlen, 0, (struct sockaddr *)&cliaddr, clilen);
				}
				else
				{
					printf(" CRITICAL INVALID MESSAGE\n" );

				}
			}
#endif

		}
	}
 	else if( nSel < 0 )
    {
        if (nSel == -1)
        {
            if (errno == EINTR)
                return 0;
        }
        fprintf(stderr, "SELECT FUNCTION CRITICAL PROBLEM INVOKE %s", strerror(errno));
        return -1;
    }

	}
}

const char *get_time()
{
    static char time_str[64];
    time_t  now;

    now = time(&now);
    strftime(time_str,64,"%H:%M:%S",localtime(&now));
    return time_str;
}




int init_conf()
{
    FILE    *fp;
    char    buffer[128], attribute[32], dummy[8], value[32];

    if ((fp=fopen("aaasim.conf","r")) == (FILE *) NULL)
    {
        fprintf(stderr,"[%s] !!! ERROR: file(aaasim.conf) open error(%s)\n",
            get_time(),strerror(errno));
        return -1;
    }

    while (fgets(buffer,sizeof(buffer),fp) != (char *) NULL)
    {

        if (*buffer == '#' || *buffer == '\0' || *buffer == '\n') continue;
        if (sscanf(buffer,"%s%s%s",attribute,dummy,value) != 3)
        {
            fprintf(stderr,"[%s] !!! ERROR: %s\n",get_time(),buffer);
            fclose(fp); return -1; 
        }

        else if (!strcmp(attribute,"UDP-PORT"))
            cliport = atoi(value);
        else if (!strcmp(attribute,"CHK-TIME-SEC"))
            chk_time_sec = atoi(value);
        else if (!strcmp(attribute,"PRINT-MSG"))
            prt_msg = atoi(value);
        else
        {
            fprintf(stderr,"[%s] !!! ERROR: unknown attribute(%s)\n",get_time(),attribute);
            fclose(fp); return -1; 
        }
#if 0
        fprintf(stderr,"[%s] : attribute(%s)\n",get_time(),attribute);
        fprintf(stderr,"[%s] : attribute(%s)\n",get_time(),dummy);
        fprintf(stderr,"[%s] : attribute(%s)\n",get_time(),value);
        fprintf(stderr,"[%s] : attribute(%d)\n",get_time(),cliport);
#endif

    }

    fclose(fp); return 1;
}




int set_sockopt(int sock_fd, int txbuf_size, int rxbuf_size)
{
    struct linger       linger_opt; 
    int     optv, optl, reuseaddr;

    reuseaddr = 1;
    if (setsockopt(sock_fd,SOL_SOCKET,SO_REUSEADDR,(char *)&reuseaddr,sizeof(reuseaddr)) < 0)
    {
        fprintf(stderr,"[%s] !!! ERROR: socket REUSEADDR option error(%s)\n",
            get_time(),strerror(errno));
        return -1;
    }

    linger_opt.l_onoff = 1;
    linger_opt.l_linger = 0;
    if (setsockopt(sock_fd,SOL_SOCKET,SO_LINGER,(char *)&linger_opt,sizeof(linger_opt)) < 0)
    {
        fprintf(stderr,"[%s] !!! ERROR: socket LINGER option error(%s)\n",
            get_time(),strerror(errno));
        return -1;
    }

    optl = sizeof(optv);
    optv = 1024 * rxbuf_size;
    if (setsockopt(sock_fd,SOL_SOCKET,SO_RCVBUF,(char *)&optv,optl) < 0)
    {
        fprintf(stderr,"[%s] !!! ERROR: socket RCVBUF option error(%s)\n",
            get_time(),strerror(errno));
        return -1;
    }

    optl = sizeof(optv);
    optv = 1024 * txbuf_size;
    if (setsockopt(sock_fd,SOL_SOCKET,SO_SNDBUF,(char *)&optv,optl) < 0)
    {
        fprintf(stderr,"[%s] !!! ERROR: socket SNDBUF option error(%s)\n",
            get_time(),strerror(errno));
        return -1;
    }

    return 1;
}




void siginttrt( int nSig )
{
	int  value;
	//log_debug("STAT TOTAL   :  SendCnt[%llu] RecvCnt[%llu]", llTotalSendCnt, llTotalRecvCnt );
	//log_debug("### END SIM ###\n");
	
	value = ( llRecvCnt / abs(tLastTime - tFirstTime) ); 

	//fprintf(stderr,"STAT TOTAL   :  RecvCnt[%d] TPS[%d]", llRecvCnt, value );
	printf("\n### PROGRAM END >>>\n");
	exit(0);
}



int SendToBSD( UINT uiSendIP, INT PortNo, UINT uiBodyLen, UCHAR *szBody )
{
    struct sockaddr_in  dstAddr;
    int                 ret, dLen;
    int                 dSocketFD;

    dstAddr.sin_family      = AF_INET;
    dstAddr.sin_addr.s_addr = uiSendIP;
    dstAddr.sin_port        = htons(PortNo);

    ret = sendto( sockfd, &szBody[0], uiBodyLen, 0,
                (struct sockaddr*)&dstAddr, sizeof(struct sockaddr_in) );
    if( ret < 0 )
    {
        fprintf(stderr,"[FAIL] SendToBSD AAAIP[%s] Port[%d]\n",
                inet_ntoa(uiSendIP) , PortNo); 
        return -1;
    }

    if( ret == uiBodyLen )
    {
		/*
        fprintf(stderr,"[SUCC] SendToBSD AAAIP[%s] Port[%d]\n",
                inet_ntoa(uiSendIP) , PortNo); 
		*/
        return 0;
    }

    fprintf(stderr, "SENDTO FAIL SENDTOSIZE != UDPPKTSIZE SENDRET[%d] BODYLEN[%d]",
                ret, uiBodyLen );

    return -1;
}









int dMakeRADIUSDecimal( int dVendType, UCHAR ucType, UCHAR ucLen, UCHAR ucValue, 
                        USHORT usValue, int dValue, INT64 llValue, char *szBody)
{
    USHORT      usTemp;
    int         dTemp, dOffset = 0, dOffLen, dOffFLen;
    INT64       llTemp;

    if(dVendType == DEF_ACCAAA_VEND )
    {
        szBody[dOffset++] = DEF_ACCAAA_ATTR;
        dOffFLen = dOffset++;

        dTemp = CVT_INT(dVendType);
        memcpy(&szBody[dOffset], &dTemp, 4);
        dOffset += 4;
    }

    szBody[dOffset++] = ucType;
    dOffLen = dOffset++;

    switch(ucLen)
    {
    case 1 :
        szBody[dOffset] = ucValue;
        break;
    case 2 :
        usTemp = CVT_USHORT(usValue);
        memcpy(&szBody[dOffset], &usTemp, ucLen);
        break;
    case 4 :
        dTemp = CVT_INT(dValue);
        memcpy(&szBody[dOffset], &dTemp, ucLen);
        break;
    case 8 :
        llTemp = CVT_INT64(llValue);
        memcpy(&szBody[dOffset], &llTemp, ucLen);
        break;
    default :
        return -1;
    }

    dOffset += ucLen;

    if(dVendType == DEF_ACCAAA_VEND)
    {
        szBody[dOffLen] = dOffset - dOffFLen - 1 - 4;
        szBody[dOffFLen] = dOffset;
    }
    else
        szBody[dOffLen] = dOffset;

    return dOffset;
}



int dMakeAcctResponse( UCHAR ucID, UINT UDRSeq, char *szAuth, 
                       INT *dMsgLen, UCHAR *szAcctResMsg )
{
    int     	idx = 0;
    int     	dOffset;
    short   	usLength=20;
    USHORT  	usTemp;

    szAcctResMsg[idx] = 0x05;
    idx += 1;

    szAcctResMsg[idx] = ucID ;
    idx += 1;

    dOffset = idx;
    idx += sizeof(short);

    memcpy( &szAcctResMsg[idx], &szAuth[0] , 16 );
    idx += 16;

    idx += dMakeRADIUSDecimal((int)DEF_ACCAAA_VEND, 201,
                4, 0, 0, UDRSeq,(INT64)0, &szAcctResMsg[idx]);

    usTemp = idx;
    usTemp = htons(usTemp);
    memcpy( &szAcctResMsg[dOffset], &usTemp, sizeof(short) );

    *dMsgLen = idx;

    return 0;
}



int dMakeAcctResponseADR( UCHAR ucID, UCHAR ucADR, INT *dMsgLen, UCHAR *szAcctResMsg )
{
    int idx;
    short usLength=29;
	char szBuf[17]="0000000000000000";
	unsigned int uiVendorId=5535;

    idx = 0;

    szAcctResMsg[idx] = 0x05;
    idx += 1;

    szAcctResMsg[idx] = ucID ;
    idx += 1;

    usLength = htons(usLength);

    memcpy( &szAcctResMsg[idx], &usLength, sizeof(short) );

    idx += sizeof(short);

    memcpy( &szAcctResMsg[idx], &szBuf[0] , 16 );
    idx += 16;


	szAcctResMsg[idx] = 26;
	idx += 1;

	szAcctResMsg[idx] = 12;
	idx += 1;

	uiVendorId = htonl(uiVendorId);

	memcpy( &szAcctResMsg[idx], &uiVendorId, 4 );
	idx += 4;

	szAcctResMsg[idx] = 211;
	idx += 1;

	szAcctResMsg[idx] = 3;
	idx += 1;

	szAcctResMsg[idx] = ucADR;
	idx += 1;
		

    *dMsgLen = idx ;

    return 0;
}
