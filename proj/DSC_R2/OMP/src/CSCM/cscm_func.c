#include "cscm.h"
#include "cscm_snmp.h"
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include "conf.h"

#define NETSNMP_DS_APP_DONT_FIX_PDUS 0
#define MAX_RETRY_CNT	3

/* Global Variables */
/* 2009.04.14 by dhkim */
char		g_szRDR_IPAddr[MAX_PROBE_DEV_NUM][20];

SFM_L2Dev	*g_pstL2Dev;
char        g_szL2_IPaddr[MAX_PROBE_DEV_NUM][20];
int			L2COUNT;

/* Static Variables */
struct snmp_session		g_stL2SW_Session[MAX_PROBE_DEV_NUM];
struct snmp_session		*g_pstL2SW_SessHandle[MAX_PROBE_DEV_NUM];

#define MAX_STRING_SIZE 64

// get info main
void* get_l2sw_Info_main(void *arg);

/* Extern global variables */
extern char	trcBuf[4096], trcTmp[1024];
extern void ping_test (void);

void setShell_L2_status_shm(int sysType, unsigned int *sts_arr);
int getShell_L2_status_snmp(int sysType);

#ifndef FL
#define FL __FILE__, __LINE__
#endif


int	init_L2_snmp(void)
{
	int	i;

	init_snmp("cscm_snmp");

	for(i = 0; i < L2COUNT; i++){

		snmp_sess_init(&g_stL2SW_Session[i]);
		g_stL2SW_Session[i].version = SNMP_VERSION_1;
		g_stL2SW_Session[i].community = SNMP_L2_COMMUNITY;
		g_stL2SW_Session[i].community_len = strlen(SNMP_L2_COMMUNITY);
		g_stL2SW_Session[i].peername = g_szL2_IPaddr[i];
		g_stL2SW_Session[i].timeout = SNMP_L2_TIMEOUT;
		g_stL2SW_Session[i].retries = SNMP_L2_RETRIES;

		g_pstL2SW_SessHandle[i] = snmp_open(&g_stL2SW_Session[i]);
		if(g_pstL2SW_SessHandle[i] == NULL){
#ifdef DEBUG
			fprintf(stderr, "snmp_open error %s\n", g_szL2_IPaddr[i]);
#endif
			return -1;
		}
	}

	return 0;
}

void init_L2_status_array(unsigned int *sts_arr)
{
	int	i;

	for(i = 0; i < MAX_L2_OID_NUMBER; i++){
		if(i >= SNMP_L2_OID_RANGE_IFN_START && i <= SNMP_L2_OID_RANGE_IFN_END)
			sts_arr[i] = 0;
		else if(i >= SNMP_L2_OID_RANGE_CPU_START && i <= SNMP_L2_OID_RANGE_CPU_END)
			sts_arr[i] = 0;
		else if(i >= SNMP_L2_OID_RANGE_MEM_START && i <= SNMP_L2_OID_RANGE_MEM_END)
			sts_arr[i] = 0;
	}
}

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

void set_L2_status_array(int index, unsigned int *status, char *value)
{
	if(index >= SNMP_L2_OID_RANGE_CPU_START && index <= SNMP_L2_OID_RANGE_CPU_END)
		*status = parse_uint_value(value);
	else if(index >= SNMP_L2_OID_RANGE_MEM_START && index <= SNMP_L2_OID_RANGE_MEM_END)
		*status = parse_uint_value(value);
	else if(index >= SNMP_L2_OID_RANGE_IFN_START && index <= SNMP_L2_OID_RANGE_IFN_END)
		*status = parse_updown_value(value);
	else
		// do nothing
		*status = 0;
}

void set_L2_status_shm(int pdindex, unsigned int *status)
{
//	float	memFree = 0, memTotal = 0, memUsed = 0;
	int	i;

	for(i = 0; i < MAX_L2_OID_NUMBER; i++)
	{
		if(i >= SNMP_L2_OID_RANGE_IFN_START && i <= SNMP_L2_OID_RANGE_IFN_END) {
			g_pstL2Dev->l2Info[pdindex].portInfo[i].status = status[i];

		}
		else if(i >= SNMP_L2_OID_RANGE_CPU_START && i <= SNMP_L2_OID_RANGE_CPU_END) {
			if(i == SNMP_L2_OID_RANGE_CPU_START)
				g_pstL2Dev->l2Info[pdindex].cpuInfo.usage = status[i];
		}
		else if(i >= SNMP_L2_OID_RANGE_MEM_START && i <= SNMP_L2_OID_RANGE_MEM_END) {
#if 0
			if(i == SNMP_L2_OID_RANGE_MEM_START)
				memUsed = status[i];
			if(i == (SNMP_L2_OID_RANGE_MEM_START+1)){
				memFree = status[i];
				memTotal = memFree + memUsed;
				if( memTotal > 0 && memUsed > 0 )
					g_pstL2Dev->l2Info[pdindex].memInfo.usage = (unsigned char)((memUsed*100)/memTotal);
				else
					g_pstL2Dev->l2Info[pdindex].memInfo.usage = 0;
			}
#endif
			g_pstL2Dev->l2Info[pdindex].memInfo.usage = 0;
		}
	}
}

#define SNMP_GET_ERROR		-1
#define PARSE_ERROR			-2
#define TIME_OUT_ERROR		-3

int get_L2_snmp(int pdindex, char *oid_name, void *result, int res_len)
{

	struct snmp_pdu		*pdu;
	struct snmp_pdu		*response;
	struct variable_list	*vars;

	oid		name[MAX_OID_LEN];
	size_t	name_length;
	char	snmpbuf[256];

	int		status, ret_len;
	int		iRet = 0;

	/* initialize roop variables */
	pdu = snmp_pdu_create(SNMP_MSG_GET);
	name_length = MAX_OID_LEN;
	memset(snmpbuf, 0x00, sizeof(snmpbuf));
	ret_len = 0;
	if(!snmp_parse_oid(oid_name, name, &name_length)){
		sprintf(trcBuf, "[get_L2_snmp]OID parse error[%s]\n", oid_name);
		trclib_writeLogErr(FL, trcBuf);
		return PARSE_ERROR;
	}else
		snmp_add_null_var(pdu, name, name_length);

retry:
	status = snmp_synch_response(g_pstL2SW_SessHandle[pdindex], pdu, &response);
	if(status == STAT_SUCCESS){
		if(response->errstat == SNMP_ERR_NOERROR){
			for(vars = response->variables;vars;
				vars = vars->next_variable){
				print_variable(vars->name, vars->name_length, vars);
				ret_len = snprint_value(snmpbuf, sizeof(snmpbuf), vars->name, vars->name_length, vars);
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
		sprintf(trcBuf, "[get_L2_snmp] Timeout:No Response from %s. OID[%s]\n",
				g_stL2SW_Session[pdindex].peername, oid_name);
		trclib_writeLogErr(FL, trcBuf);
		iRet = TIME_OUT_ERROR;
	}else{
		sprintf(trcBuf, "[get_L2_snmp] SNMP_GET error from %s. OID[%s]\n",
				g_stL2SW_Session[pdindex].peername, oid_name);
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

/*
   by sjjeon
  l2sw snmp get shell script
*/
int getShell_L2_status_snmp(int sysType)
{
	const int _SIZE = 512;
	char buf[_SIZE], trcBuf[1024], out_file[64], cmd[100]; // 20110228 by dcham, out_file/cmd/trace declare 
	FILE *fp=NULL;
	unsigned int	sts_arr[MAX_L2_OID_NUMBER];
	int i=0, retry_cnt=0;

	bzero(trcBuf, sizeof(trcBuf));
    /* 20110228 by dcham, outfile/cmd initialize */	
	bzero(out_file, sizeof(out_file));
	bzero(cmd, sizeof(cmd));


_RETRY_L2SW_CHECK_:

	if(retry_cnt > 3){
		sprintf(trcBuf, "[%s] re-try check fail.(cnt:%d).\n", __FUNCTION__, retry_cnt);
		trclib_writeLogErr(FL, trcBuf);
		goto GO_OUT;
	}

	//memset(sts_arr,1,sizeof(sts_arr));  // 초기화를 1 :  장애 ...
	for(i=0; i<MAX_L2_OID_NUMBER; i++)
	{
		if(i>=SNMP_L2_OID_RANGE_CPU_START && i<=SNMP_L2_OID_RANGE_MEM_END){
			sts_arr[i] = 0; //초기화 0 : usage : mem, cpu
		}else{
			sts_arr[i] = 1; //초기화 1 : fault : port
		}
	}
    /* 20110228 by dcham, popen => system */
	if(sysType==0) {
		sprintf(out_file,"/tmp/scea_l2_sts.txt");
	    sprintf(cmd, "/DSC/SCRIPT/snmp-l2switch.sh l2a > %s", out_file);
		//pp = popen("/DSC/SCRIPT/snmp-l2switch.sh l2a", "r");
	}
	else {
		sprintf(out_file,"/tmp/sceb_l2_sts.txt");
	    sprintf(cmd, "/DSC/SCRIPT/snmp-l2switch.sh l2b > %s", out_file);
		//pp = popen("/DSC/SCRIPT/snmp-l2switch.sh l2b", "r");
	}
#if 0
	if(pp==NULL)
	{
		sprintf(trcBuf, "[%s] popen error..\n",__FUNCTION__);
		trclib_writeLogErr(FL, trcBuf);
		retry_cnt++;
		//commlib_microSleep(1000000); //sleep 1 sec
		sleep(1);
		goto _RETRY_L2SW_CHECK_;
	}
#endif

	system(cmd);

	fp = fopen(out_file, "r");
	if(fp==NULL)
	{
		sprintf(trcBuf, "[%s] fopen error..\n",__FUNCTION__);
		trclib_writeLogErr(FL, trcBuf);
		retry_cnt++;
		goto _RETRY_L2SW_CHECK_;
	}

	bzero(buf, _SIZE);

	i=0; // init
	while(fgets(buf, _SIZE, fp)!=NULL)
	{
		if(i>MAX_L2_OID_NUMBER) continue;
		// port 정보.
		if(i<=SNMP_L2_OID_RANGE_IFN_END){
			if(strstr(buf,"up")){
				sts_arr[i] = 0;  // up
			} 
		// cpu/mem 정보
		}else if(i>=SNMP_L2_OID_RANGE_CPU_START && i<=SNMP_L2_OID_RANGE_MEM_END){
				sts_arr[i] = atoi(buf);
		}
		i++;
	}

	if(i != MAX_L2_OID_NUMBER) {
		retry_cnt++;	
		if(fp) fclose(fp);
		sprintf(trcBuf, "[%s] index:%d(success:%d)..\n", __FUNCTION__,i,MAX_L2_OID_NUMBER);
		trclib_writeLogErr(FL, trcBuf);
		//commlib_microSleep(1000000); //sleep 1 sec
		sleep(1); // 20110228 by dcham sleep(1)=>sleep(3)
		goto _RETRY_L2SW_CHECK_;
	}

GO_OUT:
	if(fp)fclose(fp);
	setShell_L2_status_shm(sysType, sts_arr);

	return 0;
}


/*
	set L2 Info
 
 */
void setShell_L2_status_shm(int sysType, unsigned int *sts_arr)
{
    int i;
	unsigned int cpuAVG=0;
	unsigned int memUsage=0;
	float use, total;

	for(i = 0; i <= SNMP_L2_OID_RANGE_IFN_END; i++)
	{
		g_pstL2Dev->l2Info[sysType].portInfo[i].status = sts_arr[i];
	}

	for(i=SNMP_L2_OID_RANGE_CPU_START; i<=SNMP_L2_OID_RANGE_CPU_END; i++)
	{
		cpuAVG += sts_arr[i];
	}
	// 3 개 cpu 정보 평균값 ... 
	cpuAVG = cpuAVG/(SNMP_L2_OID_RANGE_CPU_END-SNMP_L2_OID_RANGE_CPU_START+1);
	g_pstL2Dev->l2Info[sysType].cpuInfo.usage = cpuAVG;

	// memory usage (%)
	use = sts_arr[SNMP_L2_OID_RANGE_MEM_START];
	total = use + sts_arr[SNMP_L2_OID_RANGE_MEM_END];
	memUsage = (int)((use / total) *100);
	g_pstL2Dev->l2Info[sysType].memInfo.usage = memUsage;

	return;
}

