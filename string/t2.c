#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum _en_SysType{
	SYSTYPE_TAF = 0,
	SYSTYPE_TAM,
	SYSTYPE_DIRECTOR,
	SYSTYPE_SWITCH
};

#define SYSTYPE_TAF_MSG      "LOCATE = TAF"
#define SYSTYPE_TAM_MSG      "LOCATE = TAM_APP"
#define SYSTYPE_DIRECTOR_MSG "LOCATE = DIRECTOR"
#define SYSTYPE_SWITCH_MSG   "LOCATE = SWITCH"
#define SYS_NAME "TAM_SA1"

typedef struct _st_sub2{
	char szDesc[30];	
} stSub2;

struct _st_sub{
	int dCount;
	stSub2 stInfo[3];
} gstSub;

int cvt_msg(char *cont, char *msg, int ucNTAFID, int ucNTAMID)
{
    int  len, plen, type;
    char *pidx, buf[128], szSysName[24];
	char *msgs[] = { SYSTYPE_TAF_MSG, SYSTYPE_TAM_MSG, SYSTYPE_DIRECTOR_MSG, SYSTYPE_SWITCH_MSG };

	
	len = sizeof( msgs )/ sizeof( char *);
	for(type = 0, pidx = NULL; type < len; type++ ){
		sprintf(buf, "%s", msgs[type]);
		pidx = strstr( cont, buf );
		if( pidx != NULL ){
			break;
		}
	}
	
	if( type == len ){
		return -1;
	}

    /* TAM Alias */ 
    if( gstSub.dCount < ucNTAMID ){
        sprintf(szSysName, "TAM_APP %02d", ucNTAMID);
    }
    else{
        sprintf(szSysName, "%s", gstSub.stInfo[ucNTAMID-1].szDesc);
    }

    plen = strlen(buf);
    switch(type){
        case SYSTYPE_TAM:
            plen = (pidx - cont) + plen -7; // 7 <- 'TAM_APP'
            strncpy(msg, cont, plen);
            msg[plen] = 0x00;

            len = strlen(msg);
            sprintf(&msg[len], "%s", szSysName);

            pidx += strlen(buf) + 3;
            break;

        case SYSTYPE_TAF:
            plen = (pidx - cont) + plen -3; // 3 <- 'TAF' 
            strncpy(msg, cont, plen );
            msg[plen] = 0x00;

            len = strlen(msg);
            sprintf(&msg[len], "%s / ", szSysName);
            pidx += strlen(buf) - 3;
            break;

		case SYSTYPE_DIRECTOR:
            plen = (pidx - cont) + plen -8; // 8 <- 'DIRECTOR'
            strncpy(msg, cont, plen);
            msg[plen] = 0x00;

            len = strlen(msg);
            sprintf(&msg[len], "%s / ", szSysName);

            pidx += strlen(buf) - 8;
			break;

		case SYSTYPE_SWITCH:
            plen = (pidx - cont) + plen -6; // 6 <- 'SWITCH'
            strncpy(msg, cont, plen);
            msg[plen] = 0x00;

            len = strlen(msg);
            sprintf(&msg[len], "%s / ", szSysName);

            pidx += strlen(buf) - 6;
			break;
    }
    len  = strlen(msg);
    sprintf(&msg[len], "%s", pidx);

    msg[len + strlen(pidx)] = 0x00;

    return strlen(msg);

}

int main()
{
	int len, plen;
	char newbuf[2048],buf[2048];
	char *pidx,buf2[512];

	gstSub.dCount = 3;
	sprintf(gstSub.stInfo[0].szDesc, "TAM_SIM_1");
	sprintf(gstSub.stInfo[1].szDesc, "TAM_SIM_2");
	sprintf(gstSub.stInfo[2].szDesc, "TAM_SIM_3");
	
/*
struct _st_sub{
	int dCount;
	stSub2 stInfo[3];
} stSub;
*/


/*
	sprintf(buf, "%s",
				"TAM_APP 2010-12-05 03:47:54 SUN\n"
				"##  A2250: LOAD TAF NIFO ALARM CLEARED\n"
				"   CLASS  = MAJOR\n"
				"   LOCATE = TAF 05 / NIFO\n"
				"   INFORM = NORMAL [ 0%]\n"
				"COMPLETED\n");

*/
/*
*** A1350: H/W DIRECTOR M/C FAULT OCCURED
   CLASS  = CRITICAL
   LOCATE = DIRECTOR 04 / M/C MONITOR_PORT[7]
   INFORM = DOWN
COMPLETED

*** A1350: H/W DIRECTOR M/C FAULT OCCURED
   CLASS  = CRITICAL
   LOCATE = DIRECTOR 04 / M/C MONITOR_PORT[2]
   INFORM = DOWN
COMPLETED

*** A1410: H/W SWITCH  FAULT OCCURED
   CLASS  = CRITICAL
   LOCATE = SWITCH 02 /  SWITCH_PORT[24]
   INFORM = DOWN
COMPLETED

TAM_APP 2010-12-02 18:21:38 THU
##  A2170: LOAD TAM_APP NIFO ALARM CLEARED
   CLASS  = MAJOR
   LOCATE = TAM_SANGAM1 / NIFO
   INFORM = NORMAL [ 6%]
COMPLETED
]
[COND:1636][12/02 18:21:38][DBG] [SEND COND MESSAGE IDX[0] IP[172.30.57.245] SFD[8] LEN[270] SUCCESS]
[COND:1636][12/02 18:21:38][DBG] [SEND COND MESSAGE IDX[4] IP[172.30.125.210] SFD[12] LEN[270] SUCCESS]
[COND:1636][12/02 18:21:38][DBG] [SEND COND MESSAGE IDX[6] IP[172.30.71.68] SFD[14] LEN[270] SUCCESS]
[COND:1636][12/02 18:21:38][DBG] [SEND COND MESSAGE IDX[7] IP[172.30.122.246] SFD[15] LEN[270] SUCCESS]
[COND:1636][12/02 18:21:38][DBG] [SEND COND MESSAGE IDX[8] IP[172.26.12.28] SFD[16] LEN[270] SUCCESS]
[COND:1636][12/02 18:21:38][DBG] [[GET MSG] PRCID=7 SID=1 MID=3 BLEN=278 GETLEN=358 NID=0]
[COND:1636][12/02 18:21:38][DBG] [ COND MESSAGE : 
TAM_APP 2010-12-02 18:21:38 THU
**  A2260: LOAD TAF TRAFFIC ALARM OCCURED
   CLASS  = MAJOR
   LOCATE = TAF 01 (TAM_SANGAM1)/ TRAFFIC
   INFORM = OVER MAJOR LEVEL [90%]
COMPLETED
]
*/
	sprintf(buf,"%s",
	"TAM_APP 2010-09-06 10:58:13 MON\n A2210: LOAD TAF CPU ALARM CLEARED\n   CLASS  = NORMAL\n   LOCATE = TAF 01 / CPU\n   INFORM = NORMAL [ 0%]\nCOMPLETED\n");
	len = cvt_msg(buf, newbuf,1,2);

#if 0

	sprintf(buf2, "LOCATE = ");
	plen = strlen(buf2);
	pidx = strstr(buf,buf2);

	/* head */
	strncpy(newbuf,buf, (pidx-buf) );
	newbuf[pidx-buf] = 0x00;

	/* body */
	plen = strlen(newbuf);
	sprintf(&newbuf[plen],"LOCATE = %s", "TAM_SANGAM1");
	plen = strlen(newbuf);
	
	/* tail */
	pidx += strlen(buf2) + strlen("TAF 00");
	sprintf(&newbuf[plen], "%s", pidx);
	

	
	
	len = strlen(buf);
#endif
	printf("buflen=%d, cont=\n%s\n",len, buf);
	printf("cvs_cont=\n%s\n", newbuf);

	sprintf(buf,"%s",
	"TAM_APP 2010-09-06 10:58:13 MON\n A2210: LOAD TAF CPU ALARM CLEARED\n   CLASS  = NORMAL\n   LOCATE = TAF 01 / CPU\n   INFORM = NORMAL [ 0%]\nCOMPLETED\n");
	len = cvt_msg(buf, newbuf,2,1);
	printf("buflen=%d, cont=\n%s\n",len, buf);
	if( len < 0 )
		printf("NOT CONVERTED\n");
	else
		printf("cvs_cont=\n%s\n", newbuf);

	sprintf(buf,"%s",
	"TAM_APP 2010-09-06 10:59:01 MON\n A4110: CHANNEL TAM_APP TAF FAULT CLEARED\n   CLASS  = NORMAL\n   LOCATE = TAM_APP 00 / CH1 TAF 192.168.0.11\n   INFORM = UP\nCOMPLETED\n");
	len = cvt_msg(buf, newbuf,3,1);
	printf("buflen=%d, cont=\n%s\n",len, buf);
	if( len < 0 )
		printf("NOT CONVERTED\n");
	else
		printf("cvs_cont=\n%s\n", newbuf);

	sprintf(buf,"%s",
	"TAM_APP 2010-09-06 10:58:17 MON\n A1210: H/W TAF POWER FAULT CLEARED\n   CLASS  = NORMAL\n   LOCATE = TAF 01 / PWR B(Left)\n   INFORM = ON\nCOMPLETED\n");
	len = cvt_msg(buf, newbuf,1,2);
	printf("buflen=%d, cont=\n%s\n",len, buf);
	if( len < 0 )
		printf("NOT CONVERTED\n");
	else
	printf("cvs_cont=\n%s\n", newbuf);


	sprintf(buf,"%s",
	"*** A1410: H/W SWITCH  FAULT OCCURED\n CLASS  = CRITICAL\n LOCATE = SWITCH 02 /  SWITCH_PORT[24]\n INFORM = DOWN\n COMPLETED\n");
	len = cvt_msg(buf, newbuf,2,4);
	printf("buflen=%d, cont=\n%s\n",len, buf);
	if( len < 0 )
		printf("NOT CONVERTED\n");
	else
	printf("cvs_cont=\n%s\n", newbuf);

	sprintf(buf,"%s",
	"*** A1350: H/W DIRECTOR M/C FAULT OCCURED\n    CLASS  = CRITICAL\n    LOCATE = DIRECTOR 04 / M/C MONITOR_PORT[7]\n    INFORM = DOWN\n COMPLETED\n");
	len = cvt_msg(buf, newbuf,1,3);
	printf("buflen=%d, cont=\n%s\n",len, buf);
	if( len < 0 )
		printf("NOT CONVERTED\n");
	else
	printf("cvs_cont=\n%s\n", newbuf);

	sprintf(buf,"%s",
	"*** A1350: H/W DIRECTOR M/C FAULT OCCURED\n    CLASS  = CRITICAL\n    LOCATE = DIRECTOR 04 / M/C MONITOR_PORT[2]\n    INFORM = DOWN\n COMPLETED\n");
	len = cvt_msg(buf, newbuf,1,2);
	printf("buflen=%d, cont=\n%s\n",len, buf);
	if( len < 0 )
		printf("NOT CONVERTED\n");
	else
	printf("cvs_cont=\n%s\n", newbuf);
	return;
}
