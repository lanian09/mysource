#include "samd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

void filterCheck (int DrtNum);
void linkCheck (int DrtNum);
void linkCheck2 (int sysNum);
void *get_TAP_Info_main(void*);
void getL3PD_Info(void);
int findIndex(char *syntax, char *idex);
void ManagePortChk(int Dnum,char *IPAddrs);
int getShell_TAPLinkInfo_snmp(int sysType);
int isFileCheck(char *filepath);

extern SFM_L3PD     *l3pd;
extern pthread_mutex_t  mutex;
extern char g_szL2_IPaddr[MAX_PROBE_DEV_NUM][20];
char	l3pd_IPaddr[MAX_PROBE_DEV_NUM][20];

/* Static Variables */
typedef struct _DIRECTOR_INFO {
    unsigned char inLineStatus[12];
    unsigned char monitorStatus[10];
} DIRECTOR_INFO;


DIRECTOR_INFO directInfo[MAX_PROBE_DEV_NUM];

#define INLINE_MAX		12		/* IN-LINE PORT 갯수 */
#define MONITOR_MAX		10		/* MONITOR PORT 갯수 */
#define TAP_LINK_MAX	INLINE_MAX+MONITOR_MAX


/*--------------------------------------------------------------
   L3PD (Dirctor) 정보를 획득을 수행하는 쓰레드
--------------------------------------------------------------*/
//void getL3PD_Info(void)
void thread_get_TAP_Info(void)
{
    pthread_attr_t  thrAttr;
    pthread_t       thrId;
	//char trcBuf[1024] = {0x00,};
	int status;

    pthread_attr_init(&thrAttr);                                                   
    pthread_attr_setscope(&thrAttr, PTHREAD_SCOPE_SYSTEM);                         
    pthread_attr_setdetachstate(&thrAttr, PTHREAD_CREATE_DETACHED);                
    if(pthread_create(&thrId, &thrAttr, get_TAP_Info_main, NULL)) {                    
        sprintf (trcBuf, "[%s] pthread_create fail\n",__FUNCTION__);                     
        trclib_writeLogErr (FL,trcBuf);                                            
    }
	pthread_join(thrId, (void *) &status);
    pthread_attr_destroy(&thrAttr);
	return;

}

/*
   by sjjeon

 */
//#define _GET_SHELL_INFO
#define CHECK_COUNTER 100
void *get_TAP_Info_main(void *arg)
{
	int i, counter=0;
	//const int SLEEP_TIME=3;
	char * iv_h, l_sysc[256];
	//char trcBuf[1024] = {0x00,};
	char trapLogPath[] = "/tmp/trapevent.log";

	if ((iv_h = getenv(IV_HOME)) == NULL) {
		sprintf(trcBuf, "[samd_init] not found %s environment name\n", IV_HOME);
		trclib_writeLogErr (FL,trcBuf);                                          
		exit(0);
	}

	sprintf(l_sysc, "%s/%s", iv_h, SYSCONF_FILE);

	if(samd_get_TAP_ipaddress(l_sysc) < 0){
		sprintf(trcBuf, "[samd_init] samd_get_TAP_ipaddress fails\n");
		trclib_writeLogErr (FL,trcBuf);                                          
		exit(0);
	}

#ifndef _GET_SHELL_INFO
	// TAP SNMP GET INFO (LINK, MGMT), 최초 체크 시작..
	for(i=0;i<2;i++){
		getShell_TAPLinkInfo_snmp(i);
	}
#endif

	while(1)
	{
		counter++;

#ifdef _GET_SNMP_TRAP_
		/* 
		   snmptrap 메시지가 있는지 확인한다.
		   이 파일이 존재 하면, snmp trap 이벤트가 
		   발생하여 TAP 상태 체크를 실시한다. 
		*/
		if((isFileCheck(trapLogPath)<0) && (counter<=CHECK_COUNTER))
		{
	//		fprintf(stderr,"don't generated trap event !!\n");
			//management port는 주기적으로 체크한다. (snmptrap이 안온다)
			for(i=0;i<2;i++){
				ManagePortChk(i, (char*)l3pd_IPaddr[i]);
			}

			report_l3pd2FIMD ();
			continue;
		}

		if(counter<CHECK_COUNTER){
			sprintf(trcBuf ,"[%s] trap event.\n", __FUNCTION__);
			trclib_writeLogErr (FL,trcBuf);
		}
		counter = 0; // 초기화
#endif  /*End of _GET_SNMP_TRAP_*/

		for(i=0;i<2;i++)
		{
#ifndef _GET_SHELL_INFO
			// TAP SNMP GET INFO (LINK, MGMT)
			getShell_TAPLinkInfo_snmp(i);
#else
			// SCRIPT GET INFO (LINK. CPU, MEM)
			linkCheck2(i);
			//linkCheck(i); 
			//filterCheck (i);  // 잘못된 정보로 체크하지 않음. sjjeon 
#endif
			report_l3pd2FIMD ();
			ManagePortChk(i, (char*)l3pd_IPaddr[i]);
			sleep(3);
		}

#ifdef _GET_SNMP_TRAP_
		/*
		   trap event 로그를 삭제하여 준다.
		   : 상태 체크를 한 후, 다시 체크하면 안된다.
		 */
		if(isFileCheck(trapLogPath)>0){
			char cmd[256];
			sprintf(cmd,"/usr/bin/rm %s",trapLogPath);
			system(cmd);

			sprintf(trcBuf ,"[%s] delete trap log file : %s \n", __FUNCTION__, cmd);
			trclib_writeLogErr (FL,trcBuf);
		}
#endif	/*End of _GET_SNMP_TRAP_*/

	}
}
/*End of get_TAP_Info_main()*/

/*--------------------------------------------------------------
	TAP의 SNMP 정보를 수집한다. (by sjjeon)
--------------------------------------------------------------*/
int getShell_TAPLinkInfo_snmp(int sysType)
{
	const int _SIZE = 512;
	//char buf[_SIZE],trcBuf[_SIZE], out_file[64], cmd[100];
	char buf[_SIZE],out_file[64], cmd[100];
    FILE *fp=NULL;
    unsigned int    sts_arr[TAP_LINK_MAX];
    int i=0, retry_cnt=0, rv;
	SFM_PDGigaLanInfo *pDirect=NULL;
	char tmp[10], szValue[10];


RETRY_TAP_CHECK:

	if(retry_cnt > 3){
		sprintf(trcBuf, "[%s] %s SNMP infomation get failed.. re-try(%d)\n",
			__FUNCTION__,sysType==0?"TAP-A":"TAP-B", retry_cnt);
		trclib_writeLogErr (FL,trcBuf);                                          
		rv = -1;
		goto GO_OUT;	
	}

	bzero(szValue,sizeof(szValue));
	bzero(out_file, sizeof(out_file));
	bzero(cmd, sizeof(cmd));

	for(i=0; i<TAP_LINK_MAX; i++)
	{
		sts_arr[i] = 1; //초기화는 FAULT로 : 1
	}
    /* 20110228 by dcham, popen => system */
	if(sysType==0){
		sprintf(out_file,"/tmp/tapa_sts.txt");
		sprintf(cmd, "/DSC/SCRIPT/snmp-tap.sh tapa > %s", out_file);
		//pp = popen("/DSC/SCRIPT/snmp-tap.sh tapa", "r");
		pDirect = l3pd->l3ProbeDev[0].gigaLanInfo; /* 1st Director */ 
	}
	else{
		sprintf(out_file,"/tmp/tapb_sts.txt");
		sprintf(cmd, "/DSC/SCRIPT/snmp-tap.sh tapb > %s", out_file);
		//pp = popen("/DSC/SCRIPT/snmp-tap.sh tapb", "r");
		pDirect = l3pd->l3ProbeDev[1].gigaLanInfo; /* 2nd Director */ 
	}

	system(cmd);

	fp = fopen(out_file, "r");
	if(fp==NULL)
	{
		sprintf(trcBuf, "[%s] fopen error..\n",__FUNCTION__);
		trclib_writeLogErr(FL, trcBuf);
		retry_cnt++;
		sleep(3);
		goto RETRY_TAP_CHECK;
	}

#if 0
	if(pp==NULL)
	{
		retry_cnt++;
		sprintf(trcBuf, "[%s] popen error.. retry-cnt: %d\n", __FUNCTION__, retry_cnt);
        trclib_writeLogErr(FL, trcBuf);
		commlib_microSleep(1000000);
        goto RETRY_TAP_CHECK;
    }
#endif

	
	/*
	   Shell 수행 결과
	   /DSC/SCRIPT/snmp-tap.sh tapa | tapb

		INTEGER: 1
		INTEGER: 1
		INTEGER: 0
		....
	 */
	i=0; // init
    bzero(buf, _SIZE);
    while(fgets(buf,_SIZE,fp)!=NULL)
    {
        if(i>=TAP_LINK_MAX) continue;

        // port 정보.
        if(i<TAP_LINK_MAX)
		{
			//fprintf(stderr,"%s\n",buf);
            if(strstr(buf,"INTEGER:")!=NULL)
			{
				sscanf(buf,"%s %s",tmp, szValue);
				if(szValue[0]  == '1'){
                	sts_arr[i] = 0;  // up
				}
				else{
                	sts_arr[i] = 1;  // down
				}
        		i++;
				//fprintf(stderr,"p1 : [%s], p1 : [%s]\n", tmp , szValue);
            } 
        }
    }

	if(i!= TAP_LINK_MAX){
    	if(fp) fclose(fp); fp=NULL;
		retry_cnt++;
        sprintf(trcBuf, "[%s] index %d (success:%d).. retry-cnt: %d\n", __FUNCTION__, i, TAP_LINK_MAX, retry_cnt);
        trclib_writeLogErr(FL, trcBuf);
		//commlib_microSleep(1000000);
		sleep(1);
		goto RETRY_TAP_CHECK;
	}

GO_OUT:
	// 상태값을 반영한다. 
	for(i=0; i<TAP_LINK_MAX; i++)
	{
		pDirect[i].status = sts_arr[i];  
		//fprintf(stderr,"Systype[%d], TAP[%d],  status: %d\n", sysType, i, pDirect[i].status);
	}
	rv =0;	

    if(fp) {
		while(fgets(buf,_SIZE,fp)!=NULL){;}; //  fp의 buffer를 비워준다. (broken pipe  방지)
		fclose(fp);
	}
    return rv;

}

/*--------------------------------------------------------------
	Director 정보를 획득한다. (MEM/CPU정보)
--------------------------------------------------------------*/
#if 1
void filterCheck (int DrtNum)
{
	const int _BUFSIZ = 512;
	char *token;
	//char buf[_BUFSIZ],trcBuf[_BUFSIZ];
	char buf[_BUFSIZ];
	FILE *ptr=NULL;
	int  i=0;
	char index[]="IPv4";
	int	Dnum=0;
	char cmd[128]; 
	char szRetMsg[50][1024];
	int usage[2], retry_cnt=0;

	/*
		tap mem/cpu info shell 
		/DSC/SCRIPT/l3pdFilterInfo.sh | grep "IPv4 filter" | awk '{print $5, $6}' | sed 's/%//g'
		result ->>  15 3
	 */

	if(DrtNum == 0 ){ 
//		strcpy(cmd, "/DSC/SCRIPT/l3pdFilterInfo.sh"); /* 1st Director */
//		strcpy(cmd, "/DSC/SCRIPT/l3pdFilterInfo.sh | grep \"IPv4 filter resource utilization\""); /* 1st Director */
		strcpy(cmd, "/DSC/SCRIPT/l3pdFilterInfo.sh | grep \"IPv4 filter\" | sed 's/%//g'");
		Dnum = 0;
	}
	else{
//		strcpy(cmd, "/DSC/SCRIPT/l3pdFilterInfo2.sh"); /* 2nd Director */
//		strcpy(cmd, "/DSC/SCRIPT/l3pdFilterInfo2.sh | grep \"IPv4 filter resource utilization\""); /* 1st Director */
		strcpy(cmd, "/DSC/SCRIPT/l3pdFilterInfo2.sh | grep \"IPv4 filter\" | sed 's/%//g'");
		Dnum = 1;
	}

	memset(buf,0,sizeof(buf));
	memset(szRetMsg, 0x00, sizeof(szRetMsg) );

RETRY:

	if ((ptr = popen(cmd, "r")) == NULL)
	{
		sprintf(trcBuf, "[%s] popen directer error!\n",__FUNCTION__);
		trclib_writeLogErr (FL,trcBuf);                                          
		return ;           
	}

	usage[0] = 0;
	usage[1] = 0;

	i=0;
	while (fgets(buf,_BUFSIZ, ptr) != NULL) 
	{
		// result : "IPv4 filter resource utilization: 15  3"
		if((token = strstr(buf,index))!=NULL){ 
			sscanf(buf, "%s %s %s %s %s %s", szRetMsg[0], szRetMsg[1], szRetMsg[2], szRetMsg[3], szRetMsg[4], szRetMsg[5]);
			usage[0] = atoi(szRetMsg[4]); //mem info
			usage[1] = atoi(szRetMsg[5]); //cpu info
			//fprintf(stderr,"result : %s %s %s %s %s %s", szRetMsg[0], szRetMsg[1], szRetMsg[2], szRetMsg[3], szRetMsg[4], szRetMsg[5]);
			//fprintf(stderr,"%s", buf);
			i++;
		}

	}
	if(ptr)pclose(ptr);
	ptr=NULL;

	if(retry_cnt>=3) {
		// 실패시 0 값을 넣어준다.
		l3pd->l3ProbeDev[Dnum].memInfo.usage = 0;
		l3pd->l3ProbeDev[Dnum].cpuInfo.usage = 0;
		sprintf(trcBuf, "[%s] get TAP info fail.. retry(%d)\n",__FUNCTION__, retry_cnt);
		trclib_writeLogErr (FL,trcBuf);                                          
		return;
	}

	// cpu, mem 값을 정상적으로 얻어오면 OK. 아니면 retry
	if(i==1){
		l3pd->l3ProbeDev[Dnum].memInfo.usage = usage[0];
		l3pd->l3ProbeDev[Dnum].cpuInfo.usage = usage[1];
	}else{
		retry_cnt++;
		goto RETRY;
	}
}
#endif

/*--------------------------------------------------------------
   	디렉터 장비의 정보 (link 정보)  
--------------------------------------------------------------*/
#if 1
void linkCheck (int DrtNum)
{
	const int BUFSIZ_ = 512;
	char seps[] = " ,\t\n";
	char *token;
	//char buf[BUFSIZ_],trcBuf[256];
	char buf[BUFSIZ_];
	FILE *ptr;
	int  i=0, j=0, k=0, cnt=0,line=0;
#ifdef TAP_TEST
	char indexN1[]="n1.";
	char indexN2[]="n2.";
#else
	char index1[]="n2.";
#endif
	char index2[]="m.";
	int Dnum;
	char cmd[128];
	char szRetMsg[150][256];
	memset(cmd,0,128);


	SFM_PDGigaLanInfo *pDirect;

	if(DrtNum == 0){
		pDirect = l3pd->l3ProbeDev[0].gigaLanInfo; /* 1st Director */
		strcpy(cmd, "/DSC/SCRIPT/l3pdLinkInfo.sh | grep [n2,m].[0,1,2] | grep -v Copy");
		Dnum = 0;
	}
	else{
		pDirect = l3pd->l3ProbeDev[1].gigaLanInfo; /* 2nd Director */
#ifdef TAP_TEST
		strcpy(cmd, "/DSC/SCRIPT/l3pdLinkInfo2.sh | grep [n1,m].[0,1,2] | grep -v Copy");
#else
		strcpy(cmd, "/DSC/SCRIPT/l3pdLinkInfo2.sh | grep [n2,m].[0,1,2] | grep -v Copy");
#endif
		Dnum = 1;
	}

	memset(buf,0,BUFSIZ_);
	memset(szRetMsg,0,sizeof(szRetMsg));

	if ((ptr = popen(cmd, "r")) == NULL)
	{
		sprintf(trcBuf, "[%s] popen directer error!\n",__FUNCTION__);
		trclib_writeLogErr (FL,trcBuf);                                          
		return ;           
	}

    cnt=0;
#if 0
	while(1){
		fgets(buf,BUFSIZ_, ptr);
		sprintf(szRetMsg[cnt], "%s",buf);
		cnt++;
		if(!strcasecmp(szRetMsg[cnt],"finish")){
			printf("finish OK...\n");
			break;
		}
	}
#else
	while (fgets(buf,BUFSIZ_, ptr) != NULL)
	{
		if(cnt>=150) break;
		sprintf(szRetMsg[cnt], "%s",buf);
		cnt++;
	}
#endif
	if(ptr)pclose(ptr);
	ptr=NULL;

	for(line=0;line<cnt;line++)
	{
//		printf("%s\n",szRetMsg[line]);

		// IN-LINE PORT (순서 1~12)
#ifdef TAP_TEST
		//if((findIndex(szRetMsg[line], indexN1)) || (findIndex(szRetMsg[line], indexN2)))
		if((strstr(szRetMsg[line], indexN1)) || (strstr(szRetMsg[line], indexN2)))
#else
		if(findIndex(szRetMsg[line], index1))
#endif
		{
			//fprintf(stderr, "read line :%s\n", buf);
			token = strtok (szRetMsg[line], seps);
			while (token != NULL)
			{
			//	printf ("j:%d, i:%d--.%s.\n", j, i, token);
				token = strtok (NULL, seps);

				if (i == 1) 
				{
					i = 0;
					if (!strncmp (token, "dwn", 3)){
						//pthread_mutex_lock(&mutex);
					//	printf ("j: %d, link down\n", j);
						pDirect[j].status  = 1;
						//pthread_mutex_unlock(&mutex);
					}
					else{ 
						//pthread_mutex_lock(&mutex);
					//	printf ("j: %d, link up\n", j);
						pDirect[j].status	= 0;
						//pthread_mutex_unlock(&mutex);
					}
#if 0
					printf ("No.%d IN-LINE j[%d] sts:%d, link\n",
							DrtNum, j, 
							l3pd->l3ProbeDev[Dnum].gigaLanInfo[j].status);
#endif
					break;
				}
				i++;
			}
			j++;
		}
		// MONITOR PORT (순서 13 ~ 22)
		else if(findIndex(szRetMsg[line], index2)){
			i = 0; 
	//		printf("%s", szRetMsg[line]);
			token = strtok (szRetMsg[line], seps);
			while (token != NULL)
			{
				//printf ("k:%d, i:%d--.%s.\n", k, i, token);
				token = strtok (NULL, seps);
				if (i == 1) {
					if (!memcmp (token, "dwn", 3)){
						//pthread_mutex_lock(&mutex);
					//	printf ("%d, link down\n", k);
						pDirect[INLINE_MAX+k].status	= 1;
						//pthread_mutex_unlock(&mutex);
					}
					else {
						//pthread_mutex_lock(&mutex);
					//	printf ("%d, link up\n", k);
						pDirect[INLINE_MAX+k].status	= 0;
						//pthread_mutex_unlock(&mutex);
					}
#if 0
					printf ("NO.%d MONITOR l[%d] sts:%d, link\n",
							DrtNum, k+INLINE_MAX, 
							l3pd->l3ProbeDev[Dnum].gigaLanInfo[INLINE_MAX+k].status);
#endif
					break;
				}
				i++;
			}
			k++;
		}
	} /* End of while */
	//printf("=========================================\n");
	
	// yhshin
	// line 수가  20line 미만이면 정보를 정상적으로 가져오지 못했다.
	if (line < 20) {
		for (j=0; j < (MONITOR_MAX + INLINE_MAX); j++) {
			pDirect[j].status  = 1;
		}
	}
}
#endif

/*--------------------------------------------------------------
   	디렉터 장비의 정보 (link 정보)  
	linkCheck() 함수와 같은 기능..
--------------------------------------------------------------*/
void linkCheck2 (int sysNum)
{
	const int BUFSIZ_ = 512;
	const int MAX_LINK = 32;
	//char cmd[256], trcBuf[256], buf[BUFSIZ_], tmp[10], out_file[200]; // 20110228 by dcham, out_file declare
	char cmd[256], buf[BUFSIZ_], tmp[10], out_file[200]; // 20110228 by dcham, out_file declare
	char IN_linkSts[MAX_LINK/2][10], IN_linkName[MAX_LINK/2][10]; // IN-LINE PORT : 12 
	char MO_linkSts[MAX_LINK/2][10], MO_linkName[MAX_LINK/2][10]; // Monitor PORT : 10 
	FILE *fp = NULL;
	int tot_cnt=0, in_cnt=0, mo_cnt=0; // in_cnt : in-line port count, mo_cnt : monitor port count
	int retry_cnt=0, i,j;
		
	SFM_PDGigaLanInfo *pTAP;


RETRY_CHECK:

	if(retry_cnt > 3)
	{
		sprintf(trcBuf, "[%s] TAP port check fail. retry(%d)\n",__FUNCTION__, retry_cnt);
		trclib_writeLogErr (FL,trcBuf);
		goto GO_OUT;

	}

	bzero(out_file,sizeof(out_file)); // 20110228 by dcham, out_file initialize
	bzero(cmd,sizeof(cmd));
	bzero(buf,sizeof(buf));
	bzero(IN_linkSts,sizeof(IN_linkSts));
	bzero(MO_linkSts,sizeof(MO_linkSts));
	bzero(IN_linkName,sizeof(IN_linkName));
	bzero(MO_linkName,sizeof(MO_linkName));

	if(sysNum == 0){
		pTAP = l3pd->l3ProbeDev[0].gigaLanInfo; /* 1st TAP */
		sprintf(cmd, "/DSC/SCRIPT/l3pdLinkInfo.sh | grep [n1,n2,m].[0,1,2] | grep -v Copy");
	}
	else{
		pTAP = l3pd->l3ProbeDev[1].gigaLanInfo; /* 2nd TAP */
		sprintf(cmd, "/DSC/SCRIPT/l3pdLinkInfo2.sh | grep [n1,n2,m].[0,1,2] | grep -v Copy");
	}

	system(cmd);

	fp  =  fopen(out_file, "r");
	if(fp==NULL)
	{
		sprintf(trcBuf, "[%s] fopen error..\n",__FUNCTION__);
		trclib_writeLogErr(FL, trcBuf);
		retry_cnt++;
		goto RETRY_CHECK;
	}

#if 0
	if ((fp = popen(cmd, "r")) == NULL)	
	{
		sprintf(trcBuf, "[%s] popen error. (%s)\n",__FUNCTION__, cmd);
		trclib_writeLogErr (FL,trcBuf);
		return;
	}
#endif



	tot_cnt =0;
	while (fgets(buf,BUFSIZ_, fp) != NULL)
	{
		if(tot_cnt >= MAX_LINK) { while(fgets(buf,BUFSIZ_, fp) != NULL){;}; goto GO_OUT;}

		if(strstr(buf,"n1.") || strstr(buf,"n2."))
		{
			sscanf(buf, "%s%s%s",IN_linkName[in_cnt], tmp, IN_linkSts[in_cnt]);
			//fprintf(stderr,"IN-LINE link:%s sts:%s\n",IN_linkName[in_cnt], IN_linkSts[in_cnt]);
			tot_cnt++; in_cnt++;
		}else if(strstr(buf, "m.")){
			sscanf(buf, "%s%s%s",MO_linkName[mo_cnt], tmp, MO_linkSts[mo_cnt]);
			//fprintf(stderr,"MONITOR link:%s sts:%s\n",MO_linkName[mo_cnt], MO_linkSts[mo_cnt]);
			tot_cnt++; mo_cnt++;
		}
	}
	//fprintf(stderr,"== count : %d ===================================\n", tot_cnt);

	// TAP IN-LINE PORT는 12 개
	if(in_cnt != 12)
	{
		if(fp) fclose(fp);
		fp=NULL;
		retry_cnt++;
		sprintf(trcBuf, "[%s] IN-LINE PORT check fail. retry:%d (port count: %d)\n",
				__FUNCTION__,retry_cnt, in_cnt);
		trclib_writeLogErr (FL,trcBuf);
		goto RETRY_CHECK;
	}

	// TAP MONOTOR PORT는 10개
	if(mo_cnt != 10)
	{
		if(fp) pclose(fp);
		fp=NULL;
		retry_cnt++;
		sprintf(trcBuf, "[%s] MONITOR PORT check fail. retry:%d (port count: %d)\n",
				__FUNCTION__,retry_cnt,mo_cnt);
		trclib_writeLogErr (FL,trcBuf);
		goto RETRY_CHECK;
	}

	// IN-LINE PORT 상태 설정. (1~12)
	for(i=0; i<in_cnt; i++)
	{
		if(strstr(IN_linkSts[i], "up")){
			pTAP[i].status = 0; // up
		}else{
			pTAP[i].status = 1; // down
		}
	//	fprintf(stderr,"port(%d):%d\n", i, pTAP[i].status);
	}

	// MONITOR PORT 상태 설정. (13~22)
	for(i=0; i<mo_cnt; i++)
	{
		j = i+12;
		if(strstr(MO_linkSts[i], "up")){
			pTAP[j].status = 0; // up
		}else{
			pTAP[j].status = 1; // down
		}
	//	fprintf(stderr,"port(%d):%d\n", j, pTAP[j].status);
	}


GO_OUT:
	
	if(fp) pclose(fp);

}

/*--------------------------------------------------------------
   	디렉터 장비의 정보 (Manage Port 정보)  
--------------------------------------------------------------*/
/* Dnum : 디렉터 순서, IPAddrs : IP Address*/
void ManagePortChk(int Dnum,char *IPAddrs)
{

	int stat;
	stat = ping_test_with_ip (IPAddrs);

	/*INLINE_MAX(12개)+MONITOR_MAX(10개)+ManagePort(1개) 순서이므로 ...*/
	l3pd->l3ProbeDev[Dnum].gigaLanInfo[INLINE_MAX+MONITOR_MAX].status=stat;

#if 0		
	printf("%s] ManagePort Status (%d th) : %d\n",
			IPAddrs,
			INLINE_MAX+MONITOR_MAX+1,
			l3pd->l3ProbeDev[Dnum].gigaLanInfo[INLINE_MAX+MONITOR_MAX].status);
#endif

}

/*
1: SUCCESS
0: FAIL
 */
int findIndex(char *syntax_ori, char *idx)
{
    char *token;
    char search[]=" ";
	int len=strlen(idx);
	int i=0, cnt=0;
	char syntax[2048];

  //  if(strlen(syntax_ori) == 0|| strlen(idx)==0) return -1;
    if(syntax_ori == NULL|| idx == NULL) return -1;
	
	while(1){
		if(syntax_ori[i]!=' '){
			syntax_ori = &syntax_ori[i];
			break;
		}
		i++;
	}

	memset(syntax,0,sizeof(syntax));

	strncpy(syntax,syntax_ori,strlen(syntax_ori));

    /*첫번째 설정*/
    token = strtok((char *)syntax, (char *)search);
    //printf("%s\n",token);

	i=0;
	while(1){
		if(token[i]!=' '){
			token = &token[i];
			break;
		}
		i++;
	}

#if 0
    if(!strncasecmp(token,idx,strlen(idx))){
        //printf("find OK.. %s\n",token);
		return 1;
    }
#else

	for(i=0; i<len;i++){
		if(token[i]==idx[i]) cnt++;
		if(cnt == len) return 1;
	}
	cnt = 0;
#endif

    while(1) {
        token = strtok(NULL, search);
        if(token == NULL) break;
        //printf("%s\n",token);
#if 0
        if(!strncasecmp(token,idx,strlen(idx))){
            //printf("find OK...%s\n",token);
            return 1;
        }
#else
		for(i=0; i<len;i++){
			if(token[i]==idx[i]) cnt++;
			if(cnt == len) return 1;
		}
		cnt = 0;
		
#endif
    }
    return 0;
}

/*
by sjjeon
*/
int findCaseIndex(char *syntax_ori, char *idx)
{
    int i=0, size;
    char syntax[2048];

    if(syntax_ori == NULL|| idx == NULL) return -1;

    while(1){
        if(syntax_ori[i]!=' '){
            syntax_ori = &syntax_ori[i];
            break;
        }
        i++;
    }

    memset(syntax,0,sizeof(syntax));
    strncpy(syntax,syntax_ori,strlen(syntax_ori));

    size=strlen(syntax);

    for(i=0; i<size; i++){
        if(toupper(syntax[i])== toupper(idx[0]))
        {
            if(toupper(syntax[i+1])== toupper(idx[1]))
            {
                if(!strncasecmp(&syntax[i],idx,strlen(idx)))
                {
                    //printf("find OK...%s\n",idx);
                    return 1;
                }
            }
        }
    }
    return 0;
}

/*
	by sjjeon
	해당 경로에 파일이 존재 하는지 확인.
 */
int isFileCheck(char *filepath)
{

    struct stat dst;
	//char trcBuf[1024];

	//bzero(trcBuf, sizeof(trcBuf));

    if(stat(filepath, &dst) < 0){
        //sprintf(trcBuf,"[%s] Not found snmptrap log file (%s)\n",__FUNCTION__, filepath);
        //trclib_writeLogErr(FL, trcBuf);
        return -1;
    }

	return 1;
}

/*
main()
{
	linkCheck();
	filterCheck ();
	return 0;
}
*/
