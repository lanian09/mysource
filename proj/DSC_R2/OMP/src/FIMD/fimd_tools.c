#include "fimd_proto.h"
#include <ctype.h>

extern char		trcBuf[4096], trcTmp[1024];
extern int		trcFlag, trcLogFlag;


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int fimd_mmcHdlrVector_qsortCmp (const void *a, const void *b)
{
	return (strcasecmp (((FimdMmcHdlrVector*)a)->cmdName, ((FimdMmcHdlrVector*)b)->cmdName));
} //----- End of fimd_mmcHdlrVector_qsortCmp -----//



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int fimd_mmcHdlrVector_bsrchCmp (const void *a, const void *b)
{
	return (strcasecmp ((char*)a, ((FimdMmcHdlrVector*)b)->cmdName));
} //----- End of fimd_mmcHdlrVector_bsrchCmp -----//


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
char *fimd_printAlmLevel (unsigned char level)
{
	switch (level) {
		case SFM_ALM_NORMAL:   return("NORMAL");
		case SFM_ALM_MINOR:    return("MINOR");
		case SFM_ALM_MAJOR:    return("MAJOR");
		case SFM_ALM_CRITICAL: return("CRITICAL");
	}
	return ("");
} //----- End of fimd_printAlmLevel -----//


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
char *fimd_printAlmLevelSymbol (int level,int occurFlag)
{
    if(occurFlag == SFM_ALM_OCCURED){
        switch (level) {
            case SFM_ALM_MINOR:     return("*");
            case SFM_ALM_MAJOR:     return("**");
            case SFM_ALM_CRITICAL:  return("***");
            default:                return(""); 
        }
    }
    else{
        switch (level) {
            case SFM_ALM_MINOR:     return("#");
            case SFM_ALM_MAJOR:     return("##");
            case SFM_ALM_CRITICAL:  return("###");
            default:                return(""); 
        }
    }
    return ("");
} //----- End of fimd_printAlmLevel -----//


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
char *fimd_printProcStatus (unsigned char status)
{
	switch (status) {
		case SFM_STATUS_ALIVE: return("ALIVE");
		case SFM_STATUS_DEAD:  return("DEAD");
	}
	return ("");
} //----- End of fimd_printProcStatus -----//

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
char *fimd_printLanStatus (unsigned char status)
{
	switch (status) {
		case SFM_LAN_CONNECTED:    return("CONNECTED");
		case SFM_LAN_DISCONNECTED: return("DISCONNECTED");
	}
	return ("");
} //----- End of fimd_printProcStatus -----//

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
char *fimd_printDBStatus (unsigned char status)
{
    switch (status) {
        case SFM_DB_REPL_STARTED:         	return("DB_REP_STARTED");
        case SFM_DB_REPL_SYNC_RUN:          return("DB_REP_SYNC_RUN");
        case SFM_DB_REPL_STOP:          	return("DB_REP_STOP");
		case SFM_DB_REPL_GIVE_UP:		 	return("DB_REP_GIVE_UP");
		case SFM_DB_REPL_LAN_FAIL:		 	return("DB_REP_LAN_FAIL");
		case SFM_DB_CHECK_DOWN:			 	return("DB_CHECK_DOWN");
    }
    return ("");
} //----- End of fimd_printSS7LinkStatus -----//


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
char *fimd_printAlmDBStatus (unsigned char status)
{
    switch (status) {
        case SFM_DB_REPL_STARTED:         	return("STARTED");
        case SFM_DB_REPL_SYNC_RUN:          return("SYNC_RUN");
        case SFM_DB_REPL_STOP:          	return("STOP");
		case SFM_DB_REPL_GIVE_UP:		 	return("GIVE_UP");
		case SFM_DB_REPL_LAN_FAIL:		 	return("LAN_FAIL");
		case SFM_DB_CHECK_DOWN:			 	return("CHECK_DOWN");
    }
    return ("");
} //----- End of fimd_printSS7LinkStatus -----//


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void fimd_dumpSysAlmStat  (STM_SysFltStat *stat, int eqSysCnt)
{
	int		i;

	sprintf(trcBuf,"..... [fimd_dumpSysAlmStat ] eqSysCnt=%d\n", eqSysCnt);

	for (i=0; i<eqSysCnt; i++)
	{
		if (!strcasecmp (stat[i].sysName, "")) continue;

		sprintf(trcTmp,"%s-%s-%s\n",
				stat[i].sysType, stat[i].sysGroup, stat[i].sysName);
		strcat (trcBuf,trcTmp);

		sprintf(trcTmp,"  cpu=%d,%d,%d, mem=%d,%d,%d, disk=%d,%d,%d, lan=%d,%d,%d, proc=%d,%d,%d\n",
				stat[i].comm.cpu[0], stat[i].comm.cpu[1], stat[i].comm.cpu[2],
				stat[i].comm.mem[0], stat[i].comm.mem[1], stat[i].comm.mem[2],
				stat[i].comm.disk[0],stat[i].comm.disk[1],stat[i].comm.disk[2],
				stat[i].comm.lan[0], stat[i].comm.lan[1], stat[i].comm.lan[2],
				stat[i].comm.proc[0],stat[i].comm.proc[1],stat[i].comm.proc[2]);
		strcat (trcBuf,trcTmp);
	}

	fprintf(stdout,"%s\n",trcBuf);

	return;

} //----- End of fimd_dumpSysAlmStat  -----//


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
char *fimd_printHWStatus (unsigned char status)
{
	switch (status) {
		case SFM_HW_NOT_EQUIP: 	return("NOT_EQUIP");
	//	case SFM_HW_ABSENT:		return("ABSENT");
	//	case SFM_HW_ONLINE:		return("ONLINE");
	//	case SFM_HW_MIRROR:		return("MIRROR");
		case SFM_HW_DOWN:		return("ABNORMAL");
		case SFM_HW_UP:		    return("NORMAL");
	}
	return ("");
} //----- End of fimd_printHWStatus -----//

//------------------------------------------------------------------------------
// convByteOrd_SFM_OnbkTbl »èÁ¦
//------------------------------------------------------------------------------

char *int2dot (int ipaddr)
{
	static char dot_buf[20];
	struct  in_addr n_info;

	memset (dot_buf, 0, 20);

	n_info.s_addr = ipaddr;
	sprintf (dot_buf, "%s", inet_ntoa (n_info));

	return dot_buf;
}

void strtoupper(char *s1, int slen)
{
	int		i;

	for(i = 0; i < slen; i++){
		*s1 = toupper(*s1);
		s1++;
	}
}

void hw_name_mapping(char *s1, int slen, char *s2)
{
    int     i;
// sjjeon
/*
	if(!strncasecmp(s1, "pwr", 3)){
		if(s1[3] == '1')
			strcpy(s2, "PWR1");
		else if (s1[3] == '2')
			strcpy(s2, "PWR2");
		else if (s1[3] == '3')
			strcpy(s2, "PWR3");
		else if (s1[3] == '4')
			strcpy(s2, "PWR4");
		else
			;

		return;
	}

    if(!strncasecmp(s1, "fan", 3)){
        sprintf(s2, "FAN%c" , s1[3]);
        return;
    }

    if(!strncasecmp(s1, "disk", 4)){
        sprintf(s2, "DISK%c", s1[4]);
        return;
    }

    if(!strncasecmp(s1, "e1000g", 6)){
        sprintf(s2, "E1000G%c", s1[6]);
        return;
    }

    if(!strncasecmp(s1, "mysql", 5)){
        sprintf(s2, "MYSQL");
        return;
    }
*/

    for(i = 0; i < slen; i++){
        *s2++ = toupper(*s1++);
    }

    *s2 = 0x00;

    return;

}


void print_pmap()
{
	pid_t	procpid;
	char cmd[64];

	procpid = getpid();

	sprintf(cmd, "/usr/bin/pmap %d", (int)procpid);
	system(cmd);
}
