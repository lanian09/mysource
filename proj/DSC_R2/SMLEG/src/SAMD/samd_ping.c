#include "samd.h"
#include "samd_ping.h"

#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

#include <arpa/inet.h>

/* global member */
#define  MAX_LINK_CNT 12
int g_linkSts[MAX_LINK_CNT];

/*	Declare extern variables	*/
extern char	trcBuf[TRCBUF_LEN];
extern char	trcTmp[TRCTMP_LEN];

extern int	trcFlag;
extern int	trcLogFlag;

extern SFM_SysCommMsgType	*loc_sadb;


/*	Declare extern functions	*/
extern int valid_ipaddr(char *s1);
extern int compare_ipaddr(char *s1, char *s2);

int hwLinkCheck();    /* HW LinkCheck */

/*	Declare funcstions	*/
int local_ping_info(PingCheck *pingchk);
int remote_ping_info(PingCheck *pingchk);

void *thr_ping_test(void *args);

extern int lan_startIndex, lan_endIndex;

extern char lanConfigName[20][20];

int kFlg[12];

int dHWLinkTest (void);
int dCheckLink_IFCONFIG (void);
int dCheckLink_DLADM (void);

void thread_ping_test (void)
{
    pthread_attr_t  thrAttr;
    pthread_t   thrId;
    int ret;

    pthread_attr_init ( &thrAttr);
    pthread_attr_setscope ( &thrAttr, PTHREAD_SCOPE_SYSTEM);
    pthread_attr_setdetachstate ( &thrAttr, PTHREAD_CREATE_DETACHED);

    if ((ret = pthread_create (&thrId, &thrAttr, thr_ping_test, NULL)) != 0) {
        sprintf(trcBuf,"[thread_ping_test] pthread_create fail\n" );
        trclib_writeLogErr (FL,trcBuf);
    }

}

#define     NIPADDR(d)      (d&0xff),((d>>8)&0xff),((d>>16)&0xff),((d>>24)&0xff)
#define 	HIPADDR(d) 		((d>>24)&0xff),((d>>16)&0xff),((d>>8)&0xff),(d&0xff)
void loc_lan_pingtest()
{
    int i, j, rv;
	//int k;
    char    ipaddr[16];
    char    result[16];

	rv = 0;

	// local_lan ping 상태 체크를 한다. 
    for (i=0; loc_sadb->loc_lan_sts[i].target_IPaddress != 0; i++)
	{

        sprintf (ipaddr, "%d.%d.%d.%d", HIPADDR(loc_sadb->loc_lan_sts[i].target_IPaddress));
        sprintf(result, "%s\n",(pingtest(ipaddr, 3, 1,500000)>0)? "alive":"die");

        if (!memcmp (result, "alive", 5)) {
            loc_sadb->loc_lan_sts[i].status = SFM_LAN_CONNECTED;
        } else {

			rv = SFM_LAN_DISCONNECTED;

    		j = 0;
            do {
                //commlib_microSleep(100000);
                commlib_microSleep(50000);
                //sprintf(result, "%s\n",(pingtest(ipaddr, 0, 1,500000)>0)? "alive":"die");
                sprintf(result, "%s\n",(pingtest(ipaddr, 3, 1,500000)>0)? "alive":"die");
                if (!memcmp (result, "alive", 5)) {
                    rv = SFM_LAN_CONNECTED;
                    break;
                }
                j++;
            } while(j <= 2);
            loc_sadb->loc_lan_sts[i].status = rv;
        }
    }

#if 0	// BY JUNE, 2010-12-21
	// H/W link status initialization 20100912 by dcham
    for(k = 0; k < MAX_LINK_CNT; k++) {
       g_linkSts[k] = SFM_LAN_DISCONNECTED; 
    }
	// 1. H/W Link (dladm) 상태 체크를 한다. 
	// 0.0.0.0 (dladm check)
	hwLinkCheck();
        
    sprintf (trcBuf, "[lan_startIndex] : %d, [lan_endIndex] : %d\n",lan_startIndex,lan_endIndex); 
    trclib_writeLogErr (FL,trcBuf);


	// 2. HW LINK PING Check (ifconfig down check)
	rv = 0;
	//for (i=0; i < SFM_MAX_HPUX_HW_COM; i++) 
	for (i=0; i < MAX_LINK_CNT; i++)  //sysconfig 순서가 HARDWARE0 부터 link일때만...
	{

		// 2-1. e1000g로 시작하며, 1.1.1.1 은 ping check 대상에서 제외 
		// 1.1.1.1 : dladm check 대상.
		/* hjjung_20100607 */
//		if (!strncasecmp (loc_sadb->sysHW[i].StsName, "e1000g", 6))
		if (lan_startIndex <= i && lan_endIndex >= i)
		{
			// 2-1. Not equip으로 설정 되어 있으면 status에 SFM_LAN_NOT_EQUIP (4)설징
			// 0.0.0.0 : not equip
			if(!strcmp(loc_sadb->sysHW[i].StsInfo, "0.0.0.0")){
				g_linkSts[i] = SFM_LAN_NOT_EQUIP;
				//fprintf(stderr,"IP : %s , index : %d\n",loc_sadb->sysHW[i].StsInfo,i);
				continue;
			}

			// replication은 h/w link out시  ping은 되므로. dladm은 down(1)
			// ex) 10.1.1.2 , 10.2.1.2
			if( g_linkSts[i] == SFM_LAN_DISCONNECTED || 
					!strcmp(loc_sadb->sysHW[i].StsInfo, "1.1.1.1")){ 
				// dladm check down or "1.1.1.1' (skip)
				continue;
			} 

			sprintf(result, "%s\n",(pingtest(loc_sadb->sysHW[i].StsInfo, 0, 1,500000)>0)? "alive":"die");
			if (!strncasecmp(result, "alive", 5)) {
				//loc_sadb->sysHW[i].status = SFM_LAN_CONNECTED;
				g_linkSts[i] = SFM_LAN_CONNECTED;
			} else {

				rv = SFM_LAN_DISCONNECTED;
				j=0;
				do {
					// 재시도..
					commlib_microSleep(100000);
					sprintf(result, "%s\n",(pingtest(loc_sadb->sysHW[i].StsInfo, 0, 1,500000)>0)? "alive":"die");
					//fprintf(stderr,"DEBUG] re-try ping...%s\n",loc_sadb->sysHW[i].StsInfo);
					if (!memcmp (result, "alive", 5)) {
						rv = SFM_LAN_CONNECTED;
						break;
					}
					j++;
				} while(j <= 2);

				//loc_sadb->sysHW[i].status = rv;
				g_linkSts[i] = rv;
			}
			//fprintf(stderr, "DEBUG] name: %s, ip : %s, sts : %d\n", 
			//		loc_sadb->sysHW[i].StsName, 
			//		loc_sadb->sysHW[i].StsInfo, rv);
		} 
	} /* for */

	// 최종 값 설정..
	for(i=0; i<MAX_LINK_CNT; i++){
		loc_sadb->sysHW[i].status = g_linkSts[i];
#if 1
		if(g_linkSts[i]!=0){
//			sprintf (logbuf, "[%s] link down....(i:%d, value:%d)\n",
//					__FUNCTION__,i,loc_sadb->sysHW[i].status);
			sprintf (trcBuf, "[%s] link ....(i:%d, value:%d)\n",
					__FUNCTION__,i,loc_sadb->sysHW[i].status);
			trclib_writeLogErr (FL,trcBuf);
		}
#endif
	}
#endif	// END BY JUNE, 2010-120-21.
}
/* End of loc_lan_pingtest()*/


/*
	by sjjeon
 HW Link Check --> "dladm show-dev"

 1. Ping Check를 한 Link를 다시 dladm command를 이용해 
    H/W 링크 상태를 체크한다. 
 -> 1.dladm check -> 2. ping check  로 수정.
 2. Ping Check시 Not Equip 상태더라도 Link가 살아 있으면 up 상태로 보여준다. 

 */
int hwLinkCheck()
{
	char out_file[64];
	char TraceBuf[1024];
	char buf[512], cmd[128];
	int retry_cnt=0, rv=0, i=0, k=0;
	int linkNum;
	FILE *fp = NULL;

	const int MAX_LINK = 15;
	char linkName[MAX_LINK], linkSts[MAX_LINK];

	bzero(linkName,sizeof(linkName));
	bzero(linkSts,sizeof(linkSts));
	bzero(out_file,sizeof(out_file));
	bzero(cmd,sizeof(cmd));
	for(i=0; i<MAX_LINK_CNT; i++)
		g_linkSts[i] = SFM_LAN_DISCONNECTED; // down init : 1
	

	/*
	cmd ===>> /usr/sbin/dladm show-dev | awk '{ printf "%s %s\n",$1, $3 }'

	// result
	e1000g0 up
	e1000g1 up
	e1000g10 unknown
	e1000g11 unknown
	e1000g8 unknown
	e1000g9 unknown
	e1000g2 up
	e1000g3 up
	e1000g4 up
	e1000g5 up
	e1000g6 up
	e1000g7 up	
	 */
	sprintf(out_file, "/tmp/hwlinkchk.txt");
	sprintf(cmd,"/usr/sbin/dladm show-dev | awk '\{printf \"%%s %%s \\n\", $1, $3}' > %s", 
			(char*)out_file);

RETRY_CHECK:

	if(retry_cnt>3){
		sprintf(TraceBuf, "[%s] retry fail.\n",__FUNCTION__);
		trclib_writeLogErr(FL, TraceBuf);
		goto GO_OUT;
	}
	// 쉘을 수행한다.
	my_system(cmd);

	// 결과 파일을 읽는다.                                                                
	fp = fopen(out_file,"r");                                                             
	if(fp == NULL)                                                                        
	{                                                                                     
		sprintf(TraceBuf, "[%s] fopen fail.\n",__FUNCTION__ );                            
		trclib_writeLogErr(FL, TraceBuf);                                                 
		retry_cnt++;                                                                      
		rv=-1;                                                                            
		if(fp)fclose(fp);                                                                      
		goto RETRY_CHECK;                                                                 
	}   

	i=0; // 초기화 한다.                                                                  
	while(fgets(buf, 512, fp) != NULL)                                                  
	{                                                                                     
		/* e1000g0 ~ e1000g11 */                                                          
		/* hjjung_20100607 */
//		if(strstr(buf,"e1000g")!=NULL)
		
		if (lan_startIndex <= i && lan_endIndex >= i)
		{           
			/* 해당 링크 이름과 상태를 설정.*/                                            
            sscanf(buf,"%s%s", linkName, linkSts);

    		// Matching interface name value setting 20100912 by dcham
            for(k = 0; k < MAX_LINK_CNT; k++){
				if( strcasecmp(lanConfigName[k],linkName)==NULL){
				 g_linkSts[k] = SFM_LAN_CONNECTED;
				}
			}

			//sscanf(buf,"%s%s", linkName, linkSts);
			/* 
			   sysconfig 설정 0.0.0.0  으로 되어 있으면 status에 SFM_LAN_NOT_EQUIP 으로 설정된다.
			   status에 SFM_LAN_NOT_EQUIP으로 설정된 것중에 H.B, MIRROR Link의 
			   dladm link 정보를 체크한다. 
			   (단. H.B: e1000g2, e1000g6, MIRROR: e1000g3, e1000g5로 설정한다.)
			*/
               
			// link number를 얻는다.
			linkNum = get_hwinfo_index(linkName);
			if (linkNum < 0) 
				continue;

				if(!strcasecmp(linkSts, "up")!=NULL) 
				{
					g_linkSts[linkNum] = SFM_LAN_CONNECTED;  // up : 0
				}else{
					g_linkSts[linkNum] = SFM_LAN_DISCONNECTED;  // down : 1
				}
			i++;
		} /*End of if()*/
	}

	// link 12개 
	if(i!=12){
		retry_cnt++;
		if(fp)fclose(fp);                                                                      
		goto RETRY_CHECK;
	}

GO_OUT:
	//fprintf(stderr,"=======================================\n");
	if(fp)fclose(fp);                                                                      
	return rv;
}
/* End of hwLinkCheck*/

void rmt_lan_pingtest()
{
    int i=0,j=0, rv =0;
    char    ipaddr[16];
    char    result[16];

    for (i=0; loc_sadb->rmt_lan_sts[i].target_IPaddress != 0; i++) {

        sprintf (ipaddr, "%d.%d.%d.%d", HIPADDR(loc_sadb->rmt_lan_sts[i].target_IPaddress));
        //sprintf(result, "%s\n",(pingtest(ipaddr, 0, 1,500000)>0)? "alive":"die");
        sprintf(result, "%s\n",(pingtest(ipaddr, 3, 1,500000)>0)? "alive":"die");
//      fprintf(stderr, "%s. %s\n", ipaddr, result);

        if (!memcmp (result, "alive", 5)) {
            loc_sadb->rmt_lan_sts[i].status = SFM_LAN_CONNECTED;
        } else {

            rv = SFM_LAN_DISCONNECTED;

            do {
                commlib_microSleep(100000);
                //sprintf(result, "%s\n",(pingtest(ipaddr, 0, 1,500000)>0)? "alive":"die");
                sprintf(result, "%s\n",(pingtest(ipaddr, 3, 1,500000)>0)? "alive":"die");
                if (!memcmp (result, "alive", 5)) {
                    rv = SFM_LAN_CONNECTED;
                    break;
                }
                j++;
            } while(j <= 2);
            loc_sadb->rmt_lan_sts[i].status = rv;
        }
    }
}
/* End of rmt_lan_pingtest */

void *thr_ping_test(void *args)
{
	int			lclPingCnt, rmtPingCnt;
	PingCheck	lclpingchk[SFM_MAX_LAN_CNT], rmtpingchk[SFM_MAX_RMT_LAN_CNT];


	memset(&lclpingchk, 0x00, sizeof(lclpingchk));
	memset(&rmtpingchk, 0x00, sizeof(rmtpingchk));

	lclPingCnt	= local_ping_info(lclpingchk);
	rmtPingCnt	= remote_ping_info(rmtpingchk);

	// 2초 주기로 핑테스트 한다.
	while (1) {
		//fprintf(stderr,"ping test .......\n");
		loc_lan_pingtest();
		rmt_lan_pingtest();
		dHWLinkTest ();
		sleep (2);
	}

	pthread_exit(NULL);
}


int local_ping_info(PingCheck *pingchk)
{
	int				i, count;
	struct in_addr	ipAddr;

	count	= 0;

	for(i = 0; i < SFM_MAX_LAN_CNT; i++)
	{
		if(loc_sadb->loc_lan_sts[i].target_IPaddress != 0)
		{
			pingchk[count].hostIP	= loc_sadb->loc_lan_sts[i].target_IPaddress;
			ipAddr.s_addr			= pingchk[count].hostIP;
			strcpy(pingchk[count].ipAddrS, inet_ntoa(ipAddr));
			strcpy(pingchk[count].ipAlias, loc_sadb->loc_lan_sts[i].target_SYSName);
			pingchk[count].check	= 0;
			count++;
		}
	}

	return count;
}

int remote_ping_info(PingCheck *pingchk)
{
	//int		index=0, mIndex=0, i=0, alreadyHasIt, rv=0;
	int		index=0, i=0 ;
    char    *env=NULL, confFile[128]={0x00,}, rbuff[128]={0x00,};
    FILE    *fp=NULL;
    //char    token[20][20], tempAlias[SFM_MAX_LAN_NAME_LEN];
    char    token[20][20];
    char    logbuf[256]={0x00,};

    env = getenv(IV_HOME);
	if(!env){
         sprintf (logbuf, "[remote_ping_test] getenv fail\n");
         trclib_writeLogErr (FL,logbuf);
		 return -1;
	}

	index = 0;

	/*sjjeon*/
    sprintf(confFile, "%s/%s", env, RMT_LAN_CONF_FILE);
	fp = fopen(confFile, "r");  

	if(!fp){                                                                                     
		sprintf (logbuf, "[remote_ping_test] fopen fail[%s]; err=%d(%s)\n", 
							confFile, errno, strerror(errno));
		trclib_writeLogErr (FL,logbuf);                                                         
		return 0;                                                                               
	}  

	while(fgets(rbuff, sizeof(rbuff), fp))
	{
		memset(token, 0x00, sizeof(token));
		sscanf(rbuff, "%s%s", token[0], token[1]);
		if(token[0][0] == '#' || token[0][0] == '@') continue;
		
		if(valid_ipaddr(token[1]) == 0){
			strcpy(pingchk[index].ipAlias, token[0]);	/* System Name*/
			strcpy(pingchk[index].ipAddrS, token[1]);	/* IP Address */
			pingchk[index].check=0;						/* Status */
			index++;
		}
	}

	fclose(fp);

    loc_sadb->rmtlanCount = index; // setting the number of remote IP Address to ping to.
    for(i = 0; i < index; i++){
		loc_sadb->rmt_lan_sts[i].target_IPaddress = inet_addr(pingchk[i].ipAddrS);
		strcpy(loc_sadb->rmt_lan_sts[i].target_SYSName, pingchk[i].ipAlias);
		loc_sadb->rmt_lan_sts[i].status = 0;
    }

	return index;
}

int dHWLinkTest (void)
{
	int 	i;

	// INIT
	for(i=0; i<MAX_LINK_CNT; i++)
		g_linkSts[i] = SFM_LAN_DISCONNECTED; // down init : 1

	// LINK CHECK to DLADM
	dCheckLink_DLADM ();
	// LINK CHECK to IFCONFIG
	dCheckLink_IFCONFIG ();

	// SET LINK STATE
	for(i=0; i<MAX_LINK_CNT; i++)
	{
		loc_sadb->sysHW[i].status = g_linkSts[i];
		loc_sadb->sysSts.linkSts[i].status = g_linkSts[i];

		if((g_linkSts[i]!=0) && (loc_sadb->sysHW[i].status != 4)) {
			sprintf (trcBuf, "[%s] link ....(i:%d, value:%d)\n",
					__FUNCTION__, i, loc_sadb->sysHW[i].status);
			trclib_writeLogErr (FL,trcBuf);
		}
	}
	return 0;
}

int dCheckLink_DLADM (void)
{
	char out_file[64];
	char TraceBuf[1024];
	char buf[512], cmd[128];
	int retry_cnt=0, rv=0, i=0;
	int linkNum;
	FILE *fp = NULL;

	const int MAX_LINK = 15;
	char linkName[MAX_LINK], linkSts[MAX_LINK];

	bzero(linkName,sizeof(linkName));
	bzero(linkSts,sizeof(linkSts));
	bzero(out_file,sizeof(out_file));
	bzero(cmd,sizeof(cmd));

	/*
	cmd ===>> /usr/sbin/dladm show-dev | awk '{ printf "%s %s\n",$1, $3 }'

	// result
	e1000g0 up
	e1000g1 up
	e1000g10 unknown
	e1000g11 unknown
	e1000g8 unknown
	e1000g9 unknown
	e1000g2 up
	e1000g3 up
	e1000g4 up
	e1000g5 up
	e1000g6 up
	e1000g7 up	
	 */
	sprintf(out_file, "/tmp/hwlinkchk.txt");
	sprintf(cmd,"/usr/sbin/dladm show-dev | awk '\{printf \"%%s %%s \\n\", $1, $3}' > %s", 
			(char*)out_file);

RETRY_CHECK:

	if(retry_cnt>3){
		sprintf(TraceBuf, "[%s] retry fail.\n",__FUNCTION__);
		trclib_writeLogErr(FL, TraceBuf);
		goto GO_OUT;
	}
	// 쉘을 수행한다.
	my_system(cmd);

	// 결과 파일을 읽는다.                                                                
	fp = fopen(out_file,"r");                                                             
	if(fp == NULL)                                                                        
	{                                                                                     
		sprintf(TraceBuf, "[%s] fopen fail.\n",__FUNCTION__ );                            
		trclib_writeLogErr(FL, TraceBuf);                                                 
		retry_cnt++;                                                                      
		rv=-1;                                                                            
		if(fp)fclose(fp);                                                                      
		goto RETRY_CHECK;                                                                 
	}   

	i=0;
	while(fgets(buf, 512, fp) != NULL)                                                  
	{                                                                                     
		/* e1000g0 ~ e1000g11 */                                                          
		/* 해당 링크 이름과 상태를 설정.*/                                            
		sscanf(buf,"%s%s", linkName, linkSts);

		/* 
		   sysconfig 설정 0.0.0.0  으로 되어 있으면 status에 SFM_LAN_NOT_EQUIP 으로 설정된다.
		   status에 SFM_LAN_NOT_EQUIP으로 설정된 것중에 H.B, MIRROR Link의 
		   dladm link 정보를 체크한다. 
		   (단. H.B: e1000g2, e1000g6, MIRROR: e1000g3, e1000g5로 설정한다.)
		*/
		   
		// link number를 얻는다.
		linkNum = get_hwinfo_index(linkName);
		if (linkNum < 0) 
			continue;

			if(!strcasecmp(linkSts, "up")!=NULL) 
			{
				g_linkSts[linkNum] = SFM_LAN_CONNECTED;  // up : 0
			}else{
				g_linkSts[linkNum] = SFM_LAN_DISCONNECTED;  // down : 1
			}
		i++;
	}

	// link 12개 
	if(i!=12){
		retry_cnt++;
		if(fp)fclose(fp);                                                                      
		goto RETRY_CHECK;
	}

GO_OUT:
	if(fp)fclose(fp);                                                                      
	return rv;
}

int dCheckLink_IFCONFIG (void)
{
	int 	i, j;
	int		rv=0;
    char    result[16];

	memset(result, 0x00, sizeof(char)*16);

	// 2. HW LINK PING Check (ifconfig down check)
	rv = 0;
	//for (i=0; i < SFM_MAX_HPUX_HW_COM; i++) 
	for (i=0; i < MAX_LINK_CNT; i++)  //sysconfig 순서가 HARDWARE0 부터 link일때만...
	{

		// 2-1. Not equip으로 설정 되어 있으면 status에 SFM_LAN_NOT_EQUIP (4)설징
		// 0.0.0.0 : not equip
		if(!strcmp(loc_sadb->sysHW[i].StsInfo, "0.0.0.0")){
			g_linkSts[i] = SFM_LAN_NOT_EQUIP;
			//fprintf(stderr,"IP : %s , index : %d\n",loc_sadb->sysHW[i].StsInfo,i);
			continue;
		}

		// 2-2. 1.1.1.1 은 ping check 대상에서 제외, 1.1.1.1 : dladm check 대상.
		// replication은 h/w link out시  ping은 되므로. dladm은 down(1)
		// ex) 10.1.1.2 , 10.2.1.2
		// dladm check down or "1.1.1.1' (skip)
		if(!strcmp(loc_sadb->sysHW[i].StsInfo, "1.1.1.1")
				|| (g_linkSts[i] == SFM_LAN_DISCONNECTED)){ 
			continue;
		} 

		sprintf(result, "%s\n",(pingtest(loc_sadb->sysHW[i].StsInfo, 0, 1,500000)>0)? "alive":"die");
		if (!strncasecmp(result, "alive", 5)) {
			g_linkSts[i] = SFM_LAN_CONNECTED;
		} else {

			rv = SFM_LAN_DISCONNECTED;
			j=0;
			do {
				// 재시도..
				commlib_microSleep(100000);
				sprintf(result, "%s\n",(pingtest(loc_sadb->sysHW[i].StsInfo, 0, 1,500000)>0)? "alive":"die");
				if (!memcmp (result, "alive", 5)) {
					rv = SFM_LAN_CONNECTED;
					break;
				}
				j++;
			} while(j <= 2);

			g_linkSts[i] = rv;
		}
	} /* for */
	return 0;
}

