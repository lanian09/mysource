#include "scem.h"
#include "scem_snmp.h"
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include "conf.h"

#define NETSNMP_DS_APP_DONT_FIX_PDUS 0
#define MAX_RETRY_CNT	3

/* Global Variables */
/* 2009.04.14 by dhkim */
SFM_SCE		*g_pstSCEInfo;
char		g_szSCE_IPAddr[MAX_PROBE_DEV_NUM][20];
char		g_szRDR_IPAddr[MAX_PROBE_DEV_NUM][20];
int			SCECOUNT;

/* Static Variables */
struct snmp_session		g_stSCE_Session[MAX_PROBE_DEV_NUM];
struct snmp_session		*g_pstSCE_SessHandle[MAX_PROBE_DEV_NUM];

#define MAX_STRING_SIZE 64
static unsigned char	g_szSCEVersion[MAX_STRING_SIZE];

/* Extern global variables */
extern char	trcBuf[4096], trcTmp[1024];

void set_SCE_status_shm(int pdindex, unsigned int *status);
void setShell_SCE_status_shm(int sysType, unsigned int *sts_arr, char *sceVer);
void set_SCE_status_array(int index, unsigned int *status, char *value);
int verify_sce_status(int sysType, unsigned int *status, int doLog);
int getShell_SCE_status_snmp(int sysType);
int getShell_SNMP_SceFlow(int sysType);

#ifndef FL
#define FL __FILE__, __LINE__
#endif

unsigned int parse_uint_value(char *value)
{
	char	*tok_p;
	char	buff[20], *p;
	char	tok_colon = ':';
	
	tok_p = strrchr(value, tok_colon);
	if(!tok_p){
		sprintf(trcBuf, "[%s] strrchr error. tok_p == NULL.\n", __FUNCTION__);
		trclib_writeLogErr(FL, trcBuf);
		return 0;
	}

	tok_p++; // jump ':'
	p = buff;

	for(;*tok_p == ' ';tok_p++);
	for(;isdigit(*tok_p);tok_p++, p++) *p = *tok_p;
	*p = 0x00;

	return (unsigned int)(atoi(buff));
}

/*
	by sjjeon 
	get snmp value 
	0 : success
	-1 : fail
 */
int parse_uint_value_ex(char *value, unsigned int *out)
{
	char    *tok_p=NULL;
	char    buff[20];
	char    *tok_colon[] = {"INTEGER:", "Gauge32:"};
	int     len, i;
	char    *ps=NULL, *pe=NULL;

	*out = 0; /* init */
	for(i=0; i<2; i++){
		tok_p = strstr(value,(const char*) tok_colon[i]);
		if(tok_p!=NULL) break;
	}

	if(tok_p == NULL){
		sprintf(trcBuf, "[%s] strstr error. find token fail..\n", __FUNCTION__);
		trclib_writeLogErr(FL, trcBuf);
		return -1;
	}

	//printf("%s\n",tok_p);
	tok_p += strlen(tok_colon[i]);
	for(;*tok_p == ' ';tok_p++);  // pass ' '
	for(;!isdigit(*tok_p);tok_p++);
	ps=tok_p; // number start

	for(;isdigit(*tok_p);tok_p++);
	pe=tok_p; // number end

	len  = pe-ps;
	if(len>0){
		strncpy(buff,ps,len);
		buff[len] = '\0';
		*out = atoi(buff);
		//  fprintf(stdout,"%d\n",*out);
	}else{
		sprintf(trcBuf, "[%s] sce-snmp get value error\n", __FUNCTION__ );
		trclib_writeLogErr(FL, trcBuf);
		*out = 0;
		return -1;
	}

	return 0;
}/* End of parse_uint_value_ex */


/*
	by sjjeon 
	get snmp string value 
	0 : success
	-1 : fail
 */
int parse_string_value_ex(char *value, char *out)
{
	char    *tok_p=NULL;
	char    tok_colon[] = "STRING:";

	tok_p = strstr(value,(const char*) tok_colon);

	if(tok_p == NULL){
		sprintf(trcBuf, "[%s] strstr error. find token fail..\n", __FUNCTION__);
		trclib_writeLogErr(FL, trcBuf);
		return -1;
	}

	tok_p += strlen(tok_colon);

	for(;*tok_p == ' ';tok_p++);  // pass ' '
	if(strlen(tok_p)>MAX_STRING_SIZE) {
		strncpy(out, tok_p, MAX_STRING_SIZE);
	}else{
		strcpy(out, tok_p);
	}
	return 0;

}/* End of parse_string_value_ex*/

unsigned int parse_updown_value(char *value)
{
	char	*up = "up";
	char	*p;

	p = strstr(value, up);

	if(p)
		return 1;
	else
		return 2;
}

#define SNMP_GET_ERROR		-1
#define PARSE_ERROR			-2
#define TIME_OUT_ERROR		-3

/*
   by sjjeon
	sce 상태값을 검증.
	-상태값 중 fail/down등의 상태면 -1 return.
	- usage 상태는 제외한다. (cpu, mem 등)
param2 : 0 -> no log , other -> log
	return ( normal: 0 )
 */
int verify_sce_status(int sysType, unsigned int *status, int doLog)
{
	int i, fail_cnt=0; // fail_cnt : 상태가 abnormal 일때 count
	char sceType[6]={0x00,};
	if(!sysType) sprintf(sceType, "sce-a");
	else sprintf(sceType, "sce-b");
	
	for(i=0; i< MAX_SCE_NUMBER; i++)
	{
		/* mgmt port 상태 체크 , up : 2 */
		if(i >= SNMP_OID_RANGE_SCE_PORT_START && i <= SNMP_OID_RANGE_SCE_PORT_END)
		{
			// mgmt port만 fail check 한다. (나머지 port는 check 하지 않는다.)
			// mgmt log 만 남긴다.  (port 0, 1)
			if((i-SNMP_OID_RANGE_SCE_PORT_START)>=2)
				continue;

			if(status[i] != 2){
				fail_cnt++;
				if(!doLog) continue;
				sprintf(trcBuf, "[%s] %s  mgmt port(%d) fail(%d) \n",
						__FUNCTION__, sceType, i-SNMP_OID_RANGE_SCE_PORT_START, status[i]);
				trclib_writeLogErr(FL, trcBuf);
			}
		}
		else if(i >= SNMP_OID_RANGE_SCE_SYS_START && i <= SNMP_OID_RANGE_SCE_SYS_END)
		{
			switch(i){
				case SNMP_OID_SCE_SYS_STATUS:
					if(status[i] != 3) // normal : 3
					{
						fail_cnt++;
						if(!doLog) continue;
						sprintf(trcBuf, "[%s] %s status abnormal: %d \n", __FUNCTION__,sceType, status[i]);
						trclib_writeLogErr(FL, trcBuf);
					}
					break;
				case SNMP_OID_SCE_SYS_POWER_ALARM:
					if(status[i] != 2) // normal : 2
					{
						fail_cnt++;
						if(!doLog) continue;
						sprintf(trcBuf, "[%s] %s power status abnormal: %d \n", __FUNCTION__,sceType, status[i]);
						trclib_writeLogErr(FL, trcBuf);
					}
					break;
				case SNMP_OID_SCE_SYS_FAN_ALARM:
					if(status[i] != 2) // normal : 2
					{
						fail_cnt++;
						if(!doLog) continue;
						sprintf(trcBuf, "[%s] %s fan status abnormal: %d \n", __FUNCTION__,sceType, status[i]);
						trclib_writeLogErr(FL, trcBuf);
					}
					break;
				case SNMP_OID_SCE_SYS_TEMP_ALARM:
					if(status[i] != 2){ // normal : 2
						fail_cnt++;
						if(!doLog) continue;
						sprintf(trcBuf, "[%s] %s temp status abnormal: %d \n", __FUNCTION__, sceType,status[i]);
						trclib_writeLogErr(FL, trcBuf);
					}
					break;
			}
		}
		else if(i == SNMP_OID_RANGE_SCE_DESTA_RDR_END){
			if(status[i] != 2){	/* RDR SCEA CONNECTION, up : 2 */
				fail_cnt++;
				if(!doLog) continue;
				sprintf(trcBuf, "[%s] %s(for sce-a) rdr-connection abnormal: %d \n", __FUNCTION__, sceType, status[i]);
				trclib_writeLogErr(FL, trcBuf);
			}
			break;
		}
		else if(i == SNMP_OID_RANGE_SCE_DESTB_RDR_END){
			if(status[i] != 2){	/* RDR SCEB CONNECTION, up : 2 */
				fail_cnt++;
				if(!doLog) continue;
				sprintf(trcBuf, "[%s] %s(for sce-b) rdr-connection abnormal: %d \n", __FUNCTION__, sceType, status[i]);
				trclib_writeLogErr(FL, trcBuf);
			}
		}
	}
	return fail_cnt;
}

/*
  by sjjeon
  sce snmp get SCE shell script
*/
int getShell_SCE_status_snmp(int sysType)
{
	const int _SIZE = 1024;
	unsigned int	sts_arr[MAX_SCE_NUMBER], rvalue=0;
	char sce_ver[MAX_STRING_SIZE], cmd[100];
	char buf[_SIZE], out_file[64], trace[512];
	int i=0, rv=0, retry_cnt=0, search_rv=0;
	FILE *fp=NULL;

	bzero(out_file, sizeof(out_file));
	bzero(cmd,sizeof(cmd));
	bzero(sce_ver,sizeof(sce_ver));


	if(sysType==0){
		sprintf(out_file,"/tmp/scea_sts.txt");
		sprintf(cmd, "/DSC/SCRIPT/snmp-sce.sh scea > %s", out_file);
		//sprintf(cmd, "/DSC/SCRIPT/snmp-sce.sh scea" );
	}
	else
	{
		sprintf(out_file,"/tmp/sceb_sts.txt");
		sprintf(cmd, "/DSC/SCRIPT/snmp-sce.sh sceb > %s", out_file);
		//sprintf(cmd, "/DSC/SCRIPT/snmp-sce.sh sceb");
	}

RETRY_CHECK:
	// 재시도 3회 check
	if(retry_cnt >= MAX_RETRY_CNT){
		//sprintf(trace, "[%s] get sce infomation. re-try(%d)..\n", __FUNCTION__,retry_cnt);
		//trclib_writeLogErr(FL, trace);
		search_rv = -1;
		goto GO_OUT;
	}

	// 재시도 retry out 되지 않았으면 초기화 한다. 
	for(i=0;i<MAX_SCE_NUMBER;i++)
		sts_arr[i] = 0;

	system(cmd);

	fp = fopen(out_file, "r");
	//fp = popen(cmd, "r");
	if(fp==NULL)
	{
		sprintf(trace , "[%s] fopen error..\n",__FUNCTION__);
		trclib_writeLogErr(FL, trace );
		retry_cnt++;
		goto RETRY_CHECK;
	}

	bzero(buf,sizeof(buf));
	i=0;	// 초기화 
	while(fgets(buf,_SIZE,fp)!=NULL)
	{
		if(i>MAX_SCE_NUMBER) continue;
		/* snmp get 결과가 STRING 일때..*/
		if(strstr(buf,"PCUBE")){
			if(strstr(buf, "STRING:"))
			{
				if(i==SNMP_OID_RANGE_SCE_VERSION){
					parse_string_value_ex(buf, g_szSCEVersion);
				//	fprintf(stdout,"[%d:str]%s\n",i, g_szSCEVersion);
				}
				/* snmp get 결과가 unint 일때..*/
			}else{
				if(i<SNMP_OID_RANGE_SCE_VERSION){
					rv = parse_uint_value_ex(buf, &rvalue);
					if(rv ==0){
						sts_arr[i] = rvalue;
					}
				//	fprintf(stdout,"[%d]:%ld\n",i, sts_arr[i]);
				}
			}
			rvalue = 0;
			i++;
		}
	}
	
	if(i != MAX_SCE_NUMBER){
		retry_cnt++;
		sprintf(trcBuf, "[%s] sce snmpget fail. (index:%d, retry_cnt:%d)\n",
				__FUNCTION__,i, retry_cnt); 
		trclib_writeLogErr(FL, trcBuf);
	//	commlib_microSleep(1000000); //sleep 1 sec
		if(fp)fclose(fp);	
		fp = NULL;
		sleep(3);
		goto RETRY_CHECK;
	}

	search_rv = 0;

GO_OUT:
	if(fp)fclose(fp);	
	fp = NULL;
	
	// sce 상태값 검증.
	// abnormal 상태 와 re-try count가 re-try max 이하 일때, Re-try
	// re-try max 일때만 로그를 남긴다. 
#if 0
	if(verify_sce_status(sysType,sts_arr, retry_cnt==MAX_RETRY_CNT?1:0)>0 && retry_cnt <= MAX_RETRY_CNT)
#else
	if((verify_sce_status(sysType,sts_arr, 1)!=0) && (retry_cnt < MAX_RETRY_CNT))
#endif
	{
		retry_cnt++;
		sleep(1);
		goto RETRY_CHECK;
	}
	
	set_SCE_status_shm(sysType, sts_arr);
	return search_rv;
}

/*
	* ADD: BY JUNE, 2010-08-22
	*  > SCE FLOW에 대한 SNMP GET 처리 추가
	*  > LINE 671 - 757
 */
int getShell_SNMP_SceFlow(int sysType)
{
	const int _SIZE = 1024;
	unsigned int	sts_arr[MAX_SCE_FLOW_NUMBER], rvalue=0;
	char cmd[100];
	char buf[_SIZE], out_file[64], trace[512];
	FILE *fp=NULL;
	int i=0, rv=0, retry_cnt=0, rst_flag=0, flow_num=0;

	bzero(out_file, sizeof(out_file));
	bzero(cmd,sizeof(cmd));

	if(sysType==0){
		sprintf(out_file,"/tmp/scea_flow.txt");
		sprintf(cmd, "/DSC/SCRIPT/snmp-sce-flow.sh scea > %s", out_file);
	}
	else {
		sprintf(out_file,"/tmp/sceb_flow.txt");
		sprintf(cmd, "/DSC/SCRIPT/snmp-sce-flow.sh sceb > %s", out_file);
	}

RETRY_CHECK:
	// 재시도 3회 check
	if(retry_cnt >= MAX_RETRY_CNT) {
		//sprintf(trace, "[%s] get sce infomation. re-try(%d)..\n", __FUNCTION__,retry_cnt);
		//trclib_writeLogErr(FL, trace);
		rst_flag = -1;
		goto GO_OUT;
	}

	// 재시도 retry out 되지 않았으면 초기화 한다. 
	for(i=0;i<MAX_SCE_FLOW_NUMBER;i++) sts_arr[i] = 0;

	// Command Execution
	system(cmd);

	fp = fopen(out_file, "r");
	if(fp==NULL) {
		sprintf(trace , "[%s] fopen error..\n",__FUNCTION__);
		trclib_writeLogErr(FL, trace );
		retry_cnt++;
		goto RETRY_CHECK;
	}

	bzero(buf,sizeof(buf)); i=0;	// 초기화 
	while(fgets(buf,_SIZE,fp)!=NULL)
	{
		if(i>MAX_SCE_FLOW_NUMBER) break;
		if(strstr(buf,"PCUBE")){
			rv = parse_uint_value_ex(buf, &rvalue);
			if(rv==0){
				sts_arr[i] = rvalue;
				flow_num += sts_arr[i];
			}
			//	fprintf(stdout,"[%d]:%ld\n",i, sts_arr[i]);
			rvalue=0;
			i++;
		}
	}
	
	if(i != MAX_SCE_FLOW_NUMBER){
		retry_cnt++;
		sprintf(trcBuf, "[%s] sce_flow snmpget fail. (index:%d, retry_cnt:%d)\n"
				, __FUNCTION__,i, retry_cnt); 
		trclib_writeLogErr(FL, trcBuf);
		if(fp){
			fclose(fp); fp = NULL;
		}
		sleep(1);
		goto RETRY_CHECK;
	}
	rst_flag = 0;

GO_OUT:
	if(fp) {
		fclose(fp);	fp=NULL;
	}
	
	if (rst_flag<0) {
		sprintf(trcBuf, "[%s] sce_flow snmpget fail. (retry_cnt:%d, flow:%d)\n"
				, __FUNCTION__, retry_cnt, flow_num); 
		trclib_writeLogErr(FL, trcBuf);
	}

	return flow_num;
}
/*End of getShell_SNMP_SceFlow */


/*
	set L2 Info
 
 */
int	init_SCE_snmp()
{
	int	i;

	init_snmp("samd_snmp");

	for(i = 0; i < SCECOUNT; i++){

		snmp_sess_init(&g_stSCE_Session[i]);
		g_stSCE_Session[i].version = SNMP_VERSION_1;
		g_stSCE_Session[i].community = "public";
		g_stSCE_Session[i].community_len = strlen("public");
		g_stSCE_Session[i].peername = g_szSCE_IPAddr[i];
		g_stSCE_Session[i].timeout = 5 * 100000L;
		g_stSCE_Session[i].retries = 2;

		g_pstSCE_SessHandle[i] = snmp_open(&g_stSCE_Session[i]);
		if(g_pstSCE_SessHandle[i] == NULL){
#ifdef DEBUG
			fprintf(stderr, "snmp_open error %s\n", g_szSCE_IPAddr[i]);
#endif
			return -1;
		}
	}

	return 0;

}

void init_SCE_status_array(unsigned int *sts_arr)
{
	int	i;
	for(i = 0; i < MAX_SCE_NUMBER; i++){
		if(i >= SNMP_OID_RANGE_SCE_CPU_START && i <= SNMP_OID_RANGE_SCE_CPU_END)
			sts_arr[i] = 0;
		else if(i >= SNMP_OID_RANGE_SCE_MEM_START && i <= SNMP_OID_RANGE_SCE_MEM_END)
			sts_arr[i] = 0;
		else if(i >= SNMP_OID_RANGE_SCE_SYS_START && i <= SNMP_OID_RANGE_SCE_SYS_END)
			sts_arr[i] = 0;
		else if(i == SNMP_OID_RANGE_SCE_MODULE)
			sts_arr[i] = 0;
		else if(i >= SNMP_OID_RANGE_SCE_LINK_START && i <= SNMP_OID_RANGE_SCE_LINK_END)
			sts_arr[i] = 0;
		else if(i >= SNMP_OID_RANGE_SCE_PORT_START && i <= SNMP_OID_RANGE_SCE_PORT_END)
			sts_arr[i] = 0;
#if 0
		else if(i >= SNMP_OID_RANGE_SCE_RDR_START && i <= SNMP_OID_RANGE_SCE_RDR_END)
			sts_arr[i] = 0;
#else
		else if(i >= SNMP_OID_RANGE_SCE_DESTA_RDR_START && i <= SNMP_OID_RANGE_SCE_DESTB_RDR_END)
			sts_arr[i] = 0;
#endif
	}
}

void set_SCE_status_array(int index, unsigned int *status, char *value)
{
	int occur_flag = 0;

	if(index >= SNMP_OID_RANGE_SCE_CPU_START && index <= SNMP_OID_RANGE_SCE_CPU_END) {
		*status = parse_uint_value(value);
		if ((*status > 100) && (index < 6)) {
			occur_flag = 1;
		}
	}
	else if(index >= SNMP_OID_RANGE_SCE_MEM_START && index <= SNMP_OID_RANGE_SCE_MEM_END)
		*status = parse_uint_value(value);
	else if(index >= SNMP_OID_RANGE_SCE_SYS_START && index <= SNMP_OID_RANGE_SCE_SYS_END)
		*status = parse_uint_value(value);
	else if(index == SNMP_OID_RANGE_SCE_MODULE)
		*status = parse_uint_value(value);
	else if(index >= SNMP_OID_RANGE_SCE_LINK_START && index <= SNMP_OID_RANGE_SCE_LINK_END)
		*status = parse_uint_value(value);
	else if(index >= SNMP_OID_RANGE_SCE_PORT_START && index <= SNMP_OID_RANGE_SCE_PORT_END)
		*status = parse_uint_value(value);
#if 0
	else if(index >= SNMP_OID_RANGE_SCE_RDR_START && index <= SNMP_OID_RANGE_SCE_RDR_END)
		*status = parse_uint_value(value);
#else
	else if(index >= SNMP_OID_RANGE_SCE_DESTA_RDR_START && index <= SNMP_OID_RANGE_SCE_DESTB_RDR_END)
		*status = parse_uint_value(value);
#endif
	else if(index == SNMP_OID_RANGE_SCE_VERSION )
		sprintf( g_szSCEVersion, "%s", value);
	else
		*status = 0; // do nothing

	if (occur_flag==1) {
		sprintf(trcBuf, "[SCE TROUBLE] OID IDX[%d] VALUE IS [%d]\n", index, *status);
		trclib_writeLogErr(FL, trcBuf);
		*status = 0;
	}
#if 0 /* by june */
printf(" index : %d, status : %d, value : %s\n", index, *status, value);
#endif	
}

void set_SCE_status_shm(int pdindex, unsigned int *status)
{
	float	memFree = 0, memTotal = 0, memUsed = 0;
	char operMode[24],linkOnFailure[24];
	int	i;

	bzero(operMode, sizeof(operMode));
	bzero(linkOnFailure, sizeof(linkOnFailure));

	for(i = 0; i < MAX_SCE_NUMBER; i++){
		if(i >= SNMP_OID_RANGE_SCE_CPU_START && i <= SNMP_OID_RANGE_SCE_CPU_END){
			if(i/SNMP_SCE_CPU_CNT == 0)
				g_pstSCEInfo->SCEDev[pdindex].cpuInfo[i%SNMP_SCE_CPU_CNT].usage = status[i];		/* CPU */
			else if(i/SNMP_SCE_CPU_CNT == 1)
				g_pstSCEInfo->SCEDev[pdindex].memInfo[i%SNMP_SCE_CPU_CNT].usage = status[i];		/* MEMORY */
			else if(i/SNMP_SCE_CPU_CNT == 2)
				g_pstSCEInfo->SCEDev[pdindex].flowlossInfo[i%SNMP_SCE_CPU_CNT].usage = status[i];

	//		sprintf(trcBuf, "[%s] SCE(i:%d) Usage : %d\n", i, status[i]);
	//		trclib_writeLogErr(FL, trcBuf);
				
		}else if(i >= SNMP_OID_RANGE_SCE_MEM_START && i <= SNMP_OID_RANGE_SCE_MEM_END)
		{ 			/* DISK */
			if(i == SNMP_OID_RANGE_SCE_MEM_START)
				memUsed = status[i];
			if(i == (SNMP_OID_RANGE_SCE_MEM_START+1))
			{
				memFree = status[i];
				memTotal = memFree + memUsed;
				if( memTotal > 0 && memUsed > 0 ) {
					g_pstSCEInfo->SCEDev[pdindex].diskInfo.usage = (unsigned short)((memUsed*100)/memTotal);
				} else {
					g_pstSCEInfo->SCEDev[pdindex].diskInfo.usage = 0;
				}
			}

		}else if(i == SNMP_OID_RANGE_SCE_MODULE){
			/* PORT MODUME */
			g_pstSCEInfo->SCEDev[pdindex].portModuleStatus.status = status[i];
		}
		else if(i >= SNMP_OID_RANGE_SCE_LINK_START && i <= SNMP_OID_RANGE_SCE_LINK_END)
		{	/* LINK LIST */
			g_pstSCEInfo->SCEDev[pdindex].portLinkStatus[i-SNMP_OID_RANGE_SCE_LINK_START].status = status[i];
			// add by sjjeon : link mode 상태 version 정보에 추가.
			if(i == SNMP_OID_RANGE_SCE_LINK_START) // linkOperMode
			{
				sprintf(operMode, "     [Act: ");
				if(status[i] == 2){
					strcat(operMode,"BYPASS]");
				}else if(status[i] == 3){
					strcat(operMode,"FORWARDING]");
				}else if(status[i] == 4){
					strcat(operMode," CUTOFF]");
				}else{
					strcat(operMode,"UNKNOWN]");
				}
				//fprintf (stderr, "1:%d, %s\n", status[i], operMode);
			}else if(i == SNMP_OID_RANGE_SCE_LINK_END){   // linkAdminModeOnFailure
				sprintf(linkOnFailure, "     [Fail: ");
				if(status[i] == 2){
					strcat(linkOnFailure,"BYPASS]"); 
				}else if(status[i] == 4) {
					strcat(linkOnFailure,"CUTOFF]");
				}else{
					strcat(linkOnFailure,"UNKNOWN]");
				}
				//fprintf (stderr, "2:%d, %s\n", status[i], linkOnFailure);
			}
		}else if(i >= SNMP_OID_RANGE_SCE_PORT_START && i <= SNMP_OID_RANGE_SCE_PORT_END)
		{	/* PORT LIST */
			g_pstSCEInfo->SCEDev[pdindex].portStatus[i-SNMP_OID_RANGE_SCE_PORT_START].status = status[i];
		}else if(i >= SNMP_OID_RANGE_SCE_SYS_START && i <= SNMP_OID_RANGE_SCE_SYS_END)
		{
			switch(i)
			{
				case SNMP_OID_SCE_SYS_STATUS:														/* SYS OPERATION */
					g_pstSCEInfo->SCEDev[pdindex].sysStatus.status = status[i];
					break;

				case SNMP_OID_SCE_SYS_POWER_ALARM:													/* POWER */
					g_pstSCEInfo->SCEDev[pdindex].pwrStatus.status = status[i];
					break;

				case SNMP_OID_SCE_SYS_FAN_ALARM:													/* FAN */
					g_pstSCEInfo->SCEDev[pdindex].fanStatus.status = status[i];
					break;

				case SNMP_OID_SCE_SYS_TEMP_ALARM:													/* TEMPERATURE */
					g_pstSCEInfo->SCEDev[pdindex].tempStatus.status = status[i];
					break;

				case SNMP_OID_SCE_SYS_VOLT_ALARM:													/* VOLTAGE */
					g_pstSCEInfo->SCEDev[pdindex].voltStatus.status = status[i];
					break;

				case SNMP_OID_SCE_SYS_INTRO_USER:													/* IINTRO USER */
					g_pstSCEInfo->SCEDev[pdindex].sysInfo.intro_user = status[i];
					break;

				case SNMP_OID_SCE_SYS_ACTIVE_USER:													/* ACTIVE USER */
					g_pstSCEInfo->SCEDev[pdindex].sysInfo.active_user = status[i];
					/* hjjung_20100823 */
					g_pstSCEInfo->SCEDev[pdindex].userInfo.num = status[i];
					break;
				default :
					break;

			}
		}
		else if(i >= SNMP_OID_RANGE_SCE_DESTA_RDR_START && i <= SNMP_OID_RANGE_SCE_DESTA_RDR_END){
			if(i == SNMP_OID_RANGE_SCE_DESTA_RDR_START)						/* RDR STATUS(1) */
				g_pstSCEInfo->SCEDev[pdindex].rdrStatus[0].status = status[i];
			else if(i == SNMP_OID_RANGE_SCE_DESTA_RDR_END)
				g_pstSCEInfo->SCEDev[pdindex].rdrConnStatus[0].status = status[i];						/* RDR CONNECTION */
		}
		else if(i >= SNMP_OID_RANGE_SCE_DESTB_RDR_START && i <= SNMP_OID_RANGE_SCE_DESTB_RDR_END){
			if(i == SNMP_OID_RANGE_SCE_DESTB_RDR_START)						/* RDR STATUS(2) */
				g_pstSCEInfo->SCEDev[pdindex].rdrStatus[1].status = status[i];
			else if(i == SNMP_OID_RANGE_SCE_DESTB_RDR_END)
				g_pstSCEInfo->SCEDev[pdindex].rdrConnStatus[1].status = status[i];						/* RDR CONNECTION */
		}
		else if(i == SNMP_OID_RANGE_SCE_VERSION) {
			/** 
			  version buffer Max size 확인할것...추가시 buffer를 더 주어야 한다.
			  */
			sprintf(g_pstSCEInfo->SCEDev[pdindex].sysInfo.version," ");
			strncat(g_pstSCEInfo->SCEDev[pdindex].sysInfo.version,g_szSCEVersion,14);
			//sprintf(g_pstSCEInfo->SCEDev[pdindex].sysInfo.version, "%s", g_szSCEVersion);
			btrim(g_pstSCEInfo->SCEDev[pdindex].sysInfo.version);
			// 버젼 정보 뒤에 sce mode 상태 추가.
			if(strlen(operMode)>0) 
				strcat(g_pstSCEInfo->SCEDev[pdindex].sysInfo.version, operMode);
			if(strlen(linkOnFailure)>0) 
				strcat(g_pstSCEInfo->SCEDev[pdindex].sysInfo.version, linkOnFailure);
			
			//fprintf (stderr, "%s\n", g_pstSCEInfo->SCEDev[pdindex].sysInfo.version);
		}
	} 
}

int get_SCE_snmp(int pdindex, char *oid_name, void *result, int res_len)
{

	struct snmp_pdu		*pdu;
	struct snmp_pdu		*response;
	struct variable_list	*vars;

	oid		name[MAX_OID_LEN];
	size_t		name_length;
	char		snmpbuf[256];

	int		status, ret_len;
	int		iRet = 0;

	/* initialize roop variables */
	pdu = snmp_pdu_create(SNMP_MSG_GET);
	name_length = MAX_OID_LEN;
	memset(snmpbuf, 0x00, sizeof(snmpbuf));
	ret_len = 0;
	if(!snmp_parse_oid(oid_name, name, &name_length)){
		sprintf(trcBuf, "[get_SCE_snmp]OID parse error[%s]\n", oid_name);
		trclib_writeLogErr(FL, trcBuf);
//		printf("============>> LOG_SJJEON|%s|%s|%d",oid_name, name, name_length);
		return PARSE_ERROR;
	}else
		snmp_add_null_var(pdu, name, name_length);

retry:
	status = snmp_synch_response(g_pstSCE_SessHandle[pdindex], pdu, &response);
	if(status == STAT_SUCCESS){
		if(response->errstat == SNMP_ERR_NOERROR){
			for(vars = response->variables;vars;
				vars = vars->next_variable){
				ret_len = snprint_value(snmpbuf, sizeof(snmpbuf), vars->name,
								vars->name_length, vars);
			}
		}else{
#ifdef DEBUG
			fprintf(stderr, "Error in packet\nReason: %s\n",
				snmp_errstring(response->errstat));
			if(response->errindex != 0){
				fprintf(stderr, "Failed object: ");
				for(count = 1, vars = response->variables;
					vars && count != response->errindex;
					vars = vars->next_variable, count++)
					/* EMPTY */
					if(vars){
						fprint_objid(stderr, vars->name, vars->name_length);
					}
					fprintf(stderr, "\n");
			}
#endif
			
			/*
			 * retry if the errored variable was successfully removed
			 */
			if(!netsnmp_ds_get_boolean(NETSNMP_DS_APPLICATION_ID,
				NETSNMP_DS_APP_DONT_FIX_PDUS)){
				pdu = snmp_fix_pdu(response, SNMP_MSG_GET);
				snmp_free_pdu(response);
				response = NULL;
				if(pdu != NULL)
					goto retry;

			}
			iRet = SNMP_GET_ERROR;

		}
	}else if(status == STAT_TIMEOUT){
		sprintf(trcBuf, "[get_SCE_snmp]Timeout:No Response from %s. OID[%s]\n",
				g_stSCE_Session[pdindex].peername, oid_name);
		trclib_writeLogErr(FL, trcBuf);
		iRet = TIME_OUT_ERROR;
	}else{
		sprintf(trcBuf, "[get_SCE_snmp]SNMP_GET error from %s. OID[%s]\n",
				g_stSCE_Session[pdindex].peername, oid_name);
		trclib_writeLogErr(FL, trcBuf);
		iRet = SNMP_GET_ERROR;
	} /* endif -- STAT_SUCCESS */

	if(response)
		snmp_free_pdu(response);

	if(iRet >=0 ){
		strncpy(result, snmpbuf, res_len);
	}

	return iRet;
}

int close_SCE_snm()
{
	int     i;

	for(i = 0; i < SCECOUNT; i++)
		snmp_close(g_pstSCE_SessHandle[i]);

	return 1;
}
/* end */

