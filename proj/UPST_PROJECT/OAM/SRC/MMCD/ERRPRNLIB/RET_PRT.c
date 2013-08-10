/**A.1*  File Inclusion ***********************************/
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>		/*	inet_ntoa(3)	*/
#include <arpa/inet.h>		/*	inet_ntoa(3)	*/
#include <netinet/in.h>		/*	inet_ntoa(3)	*/

#include "typedef.h"
#include "common_stg.h"		/* TRC_TYPE_IMSI */
#include "mmcdef.h"
#include "filter.h"
#include "watch_mon.h"
#include "watch_filter.h"	/* DEF_ALARMTYPE_IM_RECV */
#include "db_struct.h"

#define MAX_PRINT_SIZE	2000
#define DEF_IP_SIZE		16
enum _direction {
	UP_DIRECTION = 1,
	DOWN_DIRECTION,
	MAX_DIRECTION
};

int 	CONT_FLAG;

/**B.2*  Definition of New Type  **************************/
/**C.1*  Declaration of Variables  ************************/


int         g_dCnt;
int         g_dSvcCnt;
int         g_IPCnt;
int         g_dThrsCnt;
int         g_dViewCnt;
int         g_dCmdCnt;
int         g_dNetStatCnt;
int         g_dFltCnt;


/**D.1*  Definition of Functions  *************************/
extern char *MH_ErrMess(short);
/**D.2*  Definition of Functions  *************************/


void MakeStringTime(int dType, time_t dTime, char *szBuf)
{
    struct tm stTime;

    if(dType == 1)
        dTime -= 3600*9;

    if(dTime < 1)
        strcpy(szBuf, "NO DATETIME");
    else
    {
        localtime_r(&dTime, &stTime);
        sprintf(szBuf, "%04d-%02d-%02d %02d:%02d:%02d",
            stTime.tm_year + 1900, stTime.tm_mon + 1, stTime.tm_mday,
            stTime.tm_hour, stTime.tm_min, stTime.tm_sec);
    }
}

void CvtBinToHexa(int dLen, char *szSrc, char *szTrg)
{
    int     i;
    int     dOffset = 0;

    for(i=0; i<dLen ; i++)
    {
        sprintf(&szTrg[dOffset], "%2.2X", szSrc[i]);
        dOffset += 2;
    }

    szTrg[dOffset] = 0x00;
}

void P_default_function(char *buf, dbm_msg_t *res, short *dCurCnt, short *dTotCnt)
{
	int slen;

	slen = strlen(buf);
    if (res->common.mml_err < 0)
		sprintf(&buf[slen], "\n  RESULT = FAIL\n  REASON = %s", MH_ErrMess(res->common.mml_err) );
    else if (res->common.mml_err == DBM_SUCCESS && res->common.cont_flag == DBM_END)
        sprintf(&buf[slen], "\n  RESULT = SUCCESS\n%s", res->data);
	else if (res->common.mml_err == DBM_NOT_LOGON && res->common.cont_flag == DBM_END)
		sprintf(&buf[slen], "\n  RESULT = SUCCESS\n%s", res->data);
	else if (res->common.mml_err == DBM_SUCCESS && res->common.cont_flag == DBM_CONTINUE)
	{
		sprintf(&buf[slen], "\n  RESULT = CONTINUE\n%s", res->data);
		CONT_FLAG = CONT;
	}
	else
		sprintf(&buf[slen], "\n  RESULT = TOO MANY RESULTS\n%s", res->data);
}

void P_def_code_only(char *buf, dbm_msg_t *res, short *dCurCnt, short *dTotCnt)
{
	int slen;

	slen = strlen(buf);

    if (res->common.mml_err < 0)
		sprintf(&buf[slen], "\n  RESULT = FAIL\n  REASON = %s", MH_ErrMess(res->common.mml_err));
	else if (res->common.mml_err == DBM_SUCCESS && res->common.cont_flag == DBM_END )
		strcat(buf, "\n  RESULT = SUCCESS");
    else if (res->common.mml_err == DBM_SUCCESS && res->common.cont_flag == DBM_CONTINUE)
	{
		strcat(buf, "\n  RESULT = CONTINUE\n");
		CONT_FLAG = CONT;
	}
	else
		strcat(buf, "\n  RESULT = TOO MANY RESULTS");
}

char *pszTimeToDate(time_t tTime)
{
	static char buf[9];
	struct tm	*stTime;

	stTime = localtime(&tTime);
	sprintf(buf, "%02d-%02d %02d", stTime->tm_mon + 1, stTime->tm_mday, stTime->tm_hour);
	return buf;
}

/* 2003.2.19
 * p_general : APP에서 완성된 Text 형태로 MMC 결과를 받고
 *			   이를 argument buffer에 copy한다.
 */
void p_general (char *buf, dbm_msg_t *res, short *dCurCnt, short *dTotCnt)
{
	int	slen=0;

	slen = strlen (buf);

	if (res->common.mml_err < 0)
		sprintf (&buf[slen], "\n  RESULT = FAIL\n  REASON = %s", MH_ErrMess (res->common.mml_err) );
	else
		sprintf (&buf[slen], "\n  RESULT = SUCCESS\n%s", res->data);

	CONT_FLAG = res->common.cont_flag;
} /* End of p_general () */


void set_cont_flag(int a)
{
	CONT_FLAG = a;
}

int get_cont_flag()
{
	return CONT_FLAG;
}

void bkup_conf_data(char *buf, dbm_msg_t *res, short *dCurCnt, short *dTotCnt)
{
	int		slen;
	int		err, offset;
	char	tempbuf[1024];

    slen = strlen(buf);

	offset = 0;
	memcpy(&err, &res->data[0], 4);
	offset += 4;

	if(err == 0)
	{
		memcpy(tempbuf, res->data + offset, 1024);
		tempbuf[strlen(tempbuf) - 1] = 0x00;
        sprintf(&buf[slen], "\n RESULT = SUCCESS\n BACKUP FILE = %s", tempbuf);
	}
	else
	{
		memcpy(tempbuf, res->data + offset, 1024);
        sprintf(&buf[slen], "\n RESULT = FAIL\n REASON = %s", tempbuf);
	}

	return;
}



/* DSCPIF : 2003. 7. 22 */

int skip_note (char *line)
{
    int     i=0;

    while (line[i] == ' ' && line[i] != '\n') i++;

    if (line[i] == '#' || line[i] == '\n')
        return 1;

    else return -1;

} /* End of skip_note () */


void CVT_IPSTR(unsigned int uiIP, char *szDest, INT dDestLen)
{
	struct in_addr  stAddr;

	stAddr.s_addr = htonl(uiIP);
	inet_ntop(AF_INET, (void *)&stAddr, szDest, dDestLen);
}


void dis_ntam_conf(char *buf, dbm_msg_t *res, short *dCurCnt, short *dTotCnt)
{
	int			slen;
	char		tempbuf[MAX_PRINT_SIZE + 500], szTemp[1024];

	pst_Conf	pstList;

	slen = strlen(buf);
	if(res->common.mml_err < 0)
		sprintf(&buf[slen], "\n  RESULT = FAIL\n  REASON = %s", MH_ErrMess(res->common.mml_err));
	else if(res->common.mml_err == DBM_SUCCESS)
	{
		pstList = (pst_Conf)&res->data[0];

		sprintf(tempbuf, "\n ---------------------------------------------------------------------------------------");

		sprintf(szTemp, "\n SYSTEM_NO\t= %d", pstList->usSysNo);
		strcat(tempbuf, szTemp);
		sprintf(szTemp, "\n LOG_LEVEL\t= %d", pstList->usLogLevel);
		strcat(tempbuf, szTemp);
		sprintf(szTemp, "\n ---------------------------------------------------------------------------------------");
		strcat(tempbuf, szTemp);

		sprintf(&buf[slen], "\n RESULT = SUCCESS%s", tempbuf);
	}
}



void dis_ntam_alm(char *buf, dbm_msg_t *res, short *dCurCnt, short *dTotCnt)
{
    int				i, slen, dlen;
    char			tempbuf[MAX_PRINT_SIZE + 500], szTemp[1024];

	pst_AlmLevel_List    	pstList;

	slen = strlen(buf);
    if (res->common.mml_err < 0)
        sprintf(&buf[slen], "\n  RESULT = FAIL\n  REASON = %s", MH_ErrMess(res->common.mml_err) );
    else if (res->common.mml_err == DBM_SUCCESS )
    {
		sprintf(tempbuf, "\n  -----------------------------------------------------");
		sprintf(szTemp, "\n  Name        CRI LEVEL   MAJOR LEVEL   MINOR LEVEL");
		strcat(tempbuf, szTemp);
		sprintf(szTemp,"\n  -----------------------------------------------------");
		strcat(tempbuf, szTemp);

	    pstList = (pst_AlmLevel_List)&res->data[0];
		if(pstList->dCount < 1)
		{
			sprintf(szTemp,"\n     NO DATA");
			strcat(tempbuf, szTemp);
		}
	    else
	    {
	        for(i = 0; i < pstList->dCount; i++)
	        {
				  sprintf(szTemp, "\n  %-10s   %8d      %8d      %8d",
					  pstList->stAlmLevel[i].szTypeName, pstList->stAlmLevel[i].sCriticalLevel,
						pstList->stAlmLevel[i].sMajorLevel, pstList->stAlmLevel[i].sMinorLevel);

				strcat(tempbuf, szTemp);

				dlen = strlen(tempbuf);
         		if(dlen > MAX_PRINT_SIZE) {
                    sprintf(&tempbuf[dlen], "\n     TOTL = [%d], CUR = [%d]\n", pstList->dCount, i);
                    break;
				}
	        }
	    }

        if(  res->common.cont_flag == DBM_END )
		{
			sprintf(szTemp, "\n  .....................................................");
			strcat(tempbuf, szTemp);
			sprintf(szTemp, "\n  Total Count = %d ",pstList->dCount);
			strcat(tempbuf, szTemp);
			sprintf(szTemp,"\n  -----------------------------------------------------");
			strcat(tempbuf, szTemp);
            sprintf(&buf[slen], "\n RESULT = SUCCESS%s", tempbuf);
		}
        else
            sprintf(&buf[slen], "\n RESULT = TOO MANY RESULTS%s", tempbuf);
    }
}


void dis_ntaf_conf(char *buf, dbm_msg_t *res, short *dCurCnt, short *dTotCnt)
{
	int			slen;
	char		tempbuf[MAX_PRINT_SIZE + 500], szTemp[1024];
	pst_Conf	pstList;

	slen = strlen(buf);
	if(res->common.mml_err < 0)
		sprintf(&buf[slen], "\n  RESULT = FAIL\n  REASON = %s", MH_ErrMess(res->common.mml_err));
	else if(res->common.mml_err == DBM_SUCCESS)
	{
		pstList = (pst_Conf)&res->data[0];

		sprintf(tempbuf, "\n ---------------------------------------------------------------------------------------");

		sprintf(szTemp, "\n SYSTEM_NO\t= %d", pstList->usSysNo);
		strcat(tempbuf, szTemp);
		sprintf(szTemp,  "\n LOG_LEVEL\t= %d", pstList->usLogLevel);
		strcat(tempbuf, szTemp);
		sprintf(szTemp, "\n ---------------------------------------------------------------------------------------");
		strcat(tempbuf, szTemp);

		sprintf(&buf[slen], "\n RESULT = SUCCESS%s", tempbuf);
	}
}


void dis_ntaf_alm(char *buf, dbm_msg_t *res, short *dCurCnt, short *dTotCnt)
{
    int				i, slen, dlen;
    char			tempbuf[MAX_PRINT_SIZE + 500], szTemp[4096];

	pst_AlmLevel_List    	pstList;

	slen = strlen(buf);
    if (res->common.mml_err < 0)
        sprintf(&buf[slen], "\n  RESULT = FAIL\n  REASON = %s", MH_ErrMess(res->common.mml_err) );
    else if (res->common.mml_err == DBM_SUCCESS )
    {
	    pstList = (pst_AlmLevel_List)&res->data[0];

		sprintf(tempbuf, "\n  -----------------------------------------------------");
		sprintf(szTemp, "\n  SYSTEM NUMBER = %02d", pstList->dSysNo);
		strcat(tempbuf, szTemp);
		sprintf(szTemp, "\n  -----------------------------------------------------");
		strcat(tempbuf, szTemp);
		sprintf(szTemp, "\n  Name        CRI LEVEL   MAJOR LEVEL   MINOR LEVEL");
		strcat(tempbuf, szTemp);
		sprintf(szTemp,"\n  -----------------------------------------------------");
		strcat(tempbuf, szTemp);

		if(pstList->dCount < 1)
		{
			sprintf(szTemp,"\n     NO DATA");
			strcat(tempbuf, szTemp);
		}
	    else
	    {
	        for(i = 0; i < pstList->dCount; i++)
	        {
				  sprintf(szTemp, "\n  %-10s   %8d      %8d      %8d",
					  pstList->stAlmLevel[i].szTypeName, pstList->stAlmLevel[i].sCriticalLevel,
					pstList->stAlmLevel[i].sMajorLevel, pstList->stAlmLevel[i].sMinorLevel);

				strcat(tempbuf, szTemp);

				dlen = strlen(tempbuf);
         		if(dlen > MAX_PRINT_SIZE) {
                    sprintf(&tempbuf[dlen], "\n     TOTL = [%d], CUR = [%d]\n", pstList->dCount, i);
                    break;
				}
	        }
	    }

        if(  res->common.cont_flag == DBM_END )
		{
			if(pstList->dCount >= 1)
			{
				sprintf(szTemp, "\n  .....................................................");
				strcat(tempbuf, szTemp);
				sprintf(szTemp, "\n  Total Count = %d ",pstList->dCount);
				strcat(tempbuf, szTemp);
				sprintf(szTemp,"\n  -----------------------------------------------------");
				strcat(tempbuf, szTemp);
			}
            sprintf(&buf[slen], "\n RESULT = SUCCESS%s", tempbuf);
		}
        else
            sprintf(&buf[slen], "\n RESULT = TOO MANY RESULTS%s", tempbuf);
    }
}

void dis_flt_svr(char *buf, dbm_msg_t *res, short *dCurCnt, short *dTotCnt)
{
	int				i;
	size_t			szStrLen, szDestLen;
	char			tempbuf[MAX_PRINT_SIZE+500], sTemp[2048], sSrcIP[16], sSysType[16], sDesc[24];;
	st_SvcInfoList	*pstList;

	szStrLen = strlen(buf);
	if(res->common.mml_err < 0)
		sprintf(&buf[szStrLen], "\n  RESULT = FAIL\n  REASON = %s", (res->common.mml_err==eOVERMAXROW) ? res->data: MH_ErrMess(res->common.mml_err));
	else if(res->common.mml_err == DBM_SUCCESS)
	{
		sprintf(tempbuf,  "\n  ---------------------------------------------------------------------------------------------");
		sprintf(sTemp, "\n  %-16s  %6s  %4s  %9s  %6s  %6s  %7s  %-20s", "IP", "PORT", "FLAG", "SYSTYPE", "L4CODE", "L7CODE", "APPCODE", "DESCRIPTION");
		strcat(tempbuf, sTemp);
		sprintf(sTemp,"\n  ---------------------------------------------------------------------------------------------");
		strcat(tempbuf, sTemp);

		pstList = (pst_SvcInfoList)&res->data[0];
		if(pstList->dCount < 1)
		{
			sprintf(sTemp, "\n     NO DATA");
			strcat(tempbuf, sTemp);
		}
		else
		{
			for(i = 0; i < pstList->dCount; i++)
			{
				CVT_IPSTR(pstList->stSvcMmc[i].uSvcIP, sSrcIP, 16);
				switch(pstList->stSvcMmc[i].cSysType)
				{
					case PDSN_SYSTYPE:
						strcpy(sSysType, "PDSN");
						break;
					case AAA_SYSTYPE:
						strcpy(sSysType, "AAA");
						break;
					case LNS_SYSTYPE:
						strcpy(sSysType, "LNS");
						break;
					case SERVICE_SYSTYPE:
						strcpy(sSysType, "SERVICE");
						break;
					default:
						strcpy(sSysType, "-");
						break;
				}

				if(strlen(pstList->stSvcMmc[i].szDesc) > (szDestLen = sizeof(sDesc)-4))
				{
					strncpy(sDesc, pstList->stSvcMmc[i].szDesc, szDestLen);
					sprintf(&sDesc[szDestLen], "...");
				}
				else
					strncpy(sDesc, pstList->stSvcMmc[i].szDesc, sizeof(sDesc));

				sprintf(sTemp, "\n  %-16s  %6hu  %-4s  %-9s  %6hu  %6hu  %7hu  %-20s",
					sSrcIP, pstList->stSvcMmc[i].huPort, (pstList->stSvcMmc[i].cFlag == 0x00)?"RP":"PI", sSysType,
					pstList->stSvcMmc[i].huL4Code, pstList->stSvcMmc[i].huL7Code ,pstList->stSvcMmc[i].huAppCode,
					sDesc);

				strcat(tempbuf, sTemp);

				szDestLen = strlen(tempbuf);
				if(szDestLen > MAX_PRINT_SIZE)
				{
					sprintf(&tempbuf[szDestLen], "\n     TOTL = [%d], CUR = [%d]\n", pstList->dCount, i);
					break;
				}
			}
		}

		if(res->common.cont_flag == DBM_END)
		{
			sprintf(sTemp, "\n  .............................................................................................");
			strcat(tempbuf, sTemp);
			sprintf(sTemp, "\n  Total Count = %d ",g_dSvcCnt+ pstList->dCount);
			strcat(tempbuf, sTemp);
			sprintf(sTemp, "\n  ---------------------------------------------------------------------------------------------");
			strcat(tempbuf, sTemp);
			sprintf(&buf[szStrLen], "\n RESULT = SUCCESS%s", tempbuf);
			g_dSvcCnt = 0;
		}
		else if(res->common.cont_flag == DBM_CONTINUE)
		{
			sprintf(sTemp, "\n  ---------------------------------------------------------------------------------------------");
			strcat(tempbuf, sTemp);
			g_dSvcCnt = g_dSvcCnt+ pstList->dCount;
			sprintf(&buf[szStrLen], "\n RESULT = CONTINUE%s", tempbuf);
			CONT_FLAG = CONT;
		}
		else
			sprintf(&buf[szStrLen], "\n RESULT = TOO MANY RESULTS%s", tempbuf);
	}
}

void dis_flt_clt(char *buf, dbm_msg_t *res, short *dCurCnt, short *dTotCnt)
{
	int				i;
	char			tempbuf[MAX_PRINT_SIZE+500], szTemp[1024], szMNIP[DEF_IP_SIZE], sSysType[16];
	size_t			dlen, slen;
	struct in_addr	stAddr;
	unsigned int	uNetmask, uMovingBits;
	pst_NAS_MMC		pstList;

	slen = strlen(buf);
	if(res->common.mml_err < 0)
		sprintf(&buf[slen], "\n  RESULT = FAIL\n  REASON = %s", (res->common.mml_err==eOVERMAXROW) ? res->data: MH_ErrMess(res->common.mml_err));
	else if(res->common.mml_err == DBM_SUCCESS)
	{
		pstList  = (pst_NAS_MMC)&res->data[0];


		sprintf(tempbuf, "\n  ---------------------------------------------------------------------------------");
		sprintf(szTemp, "\n  %-16s  %-16s  %-4s  %-10s  %-20s", "MNIP", "NETMASK", "FLAG", "SYSTYPE", "DESCRIPTION");
		strcat(tempbuf, szTemp);
		sprintf(szTemp, "\n  ---------------------------------------------------------------------------------");
		strcat(tempbuf, szTemp);

		if(pstList->dCount < 1)
		{
			sprintf(szTemp, "\n     NO DATA");
			strcat(tempbuf, szTemp);
		}
		else
		{
			for(i = 0; i < pstList->dCount; i++){

				CVT_IPSTR(pstList->stNAS[i].uMNIP, szMNIP, DEF_IP_SIZE);

				if(pstList->stNAS[i].usNetMask){
					uNetmask		= 0xFFFFFFFF;
					uMovingBits		= 32 - pstList->stNAS[i].usNetMask;
					stAddr.s_addr	= htonl(((uNetmask >> uMovingBits) << uMovingBits));
				}else
					stAddr.s_addr	= 0;

				switch(pstList->stNAS[i].cSysType){
					case PCF_SYSTYPE:  strcpy(sSysType, "PCF");  break;
					case PDSN_SYSTYPE: strcpy(sSysType, "PDSN"); break;
					case AAA_SYSTYPE:  strcpy(sSysType, "AAA");  break;
					case LNS_SYSTYPE:  strcpy(sSysType, "LNS");  break;
					case MNIP_SYSTYPE: strcpy(sSysType, "MNIP"); break;
					case LAC_SYSTYPE:  strcpy(sSysType, "LAC");  break;
					case CRX_SYSTYPE:  strcpy(sSysType, "CRX");  break;
					default: strcpy(sSysType, "-"); break;
				}

				sprintf(szTemp, "\n  %-16s  %-16s  %-4s  %-10s  %-20s",
					szMNIP, inet_ntoa(stAddr), (pstList->stNAS[i].cFlag == 0x00)?"RP":"PI", sSysType, pstList->stNAS[i].szDesc);
				strcat(tempbuf, szTemp);
	

				if( (dlen = strlen(tempbuf)) > MAX_PRINT_SIZE){
					sprintf(&tempbuf[dlen], "\n     TOTL = [%d], CUR = [%d]\n", pstList->dCount, i);
					break;
				}
			}
		}

		if(res->common.cont_flag == DBM_END)
		{
			sprintf(szTemp, "\n  .................................................................................");
			strcat(tempbuf, szTemp);
			sprintf(szTemp, "\n  Total Count = %d ", g_IPCnt+pstList->dCount);
			strcat(tempbuf, szTemp);
			sprintf(szTemp, "\n  ---------------------------------------------------------------------------------");
			strcat(tempbuf, szTemp);

			sprintf(&buf[slen], "\n RESULT = SUCCESS%s", tempbuf);
			g_IPCnt = 0;
		}
		else if(res->common.cont_flag == DBM_CONTINUE)
		{
			sprintf(szTemp, "\n  ---------------------------------------------------------------------------------");
			strcat(tempbuf, szTemp);
			g_IPCnt = g_IPCnt + pstList->dCount;
			sprintf(&buf[slen], "\n RESULT = CONTINUE%s", tempbuf);
			CONT_FLAG = CONT;
		}
		else
			sprintf(&buf[slen], "\n RESULT = TOO MANY RESULTS%s", tempbuf);
	}
}

void dis_flt_sctp(char *buf, dbm_msg_t *res, short *dCurCnt, short *dTotCnt)
{
	int				i, slen;
	size_t			dlen;
	char			tempbuf[MAX_PRINT_SIZE+500], szTemp[1024], szMNIP[DEF_IP_SIZE], sSysType[16];
	pst_SCTP_MMC	pstList;

	slen = strlen(buf);
	if(res->common.mml_err < 0)
		sprintf(&buf[slen], "\n  RESULT = FAIL\n  REASON = %s", (res->common.mml_err==eOVERMAXROW) ? res->data: MH_ErrMess(res->common.mml_err));
	else if(res->common.mml_err == DBM_SUCCESS)
	{
		sprintf(tempbuf, "\n  ----------------------------------------------------------------");
		sprintf(szTemp, "\n  %-16s  %-7s  %-8s  %-7s  %-20s", "SCTPIP", "SYSTYPE", "DRECTION", "GROUPID", "DESC");
		strcat(tempbuf, szTemp);
		sprintf(szTemp, "\n  ----------------------------------------------------------------");
		strcat(tempbuf, szTemp);

		pstList = (pst_SCTP_MMC)&res->data[0];
		if(pstList->dCount < 1)
		{
			sprintf(szTemp, "\n     NO DATA");
			strcat(tempbuf, szTemp);
		}
		else
		{
			for(i = 0; i < pstList->dCount; i++)
			{
				CVT_IPSTR(pstList->stSCTP[i].uSCTPIP, szMNIP, DEF_IP_SIZE);
				switch(pstList->stSCTP[i].cSysType)
				{
					case HSS_SYSTYPE:
						strcpy(sSysType, "HSS");
						break;
					case CSCF_SYSTYPE:
						strcpy(sSysType, "CSCF");
						break;
					default:
						strcpy(sSysType, "-");
						break;
				}

				sprintf(szTemp,"\n  %-16s  %-7s  %-8s  %7hu  %-20s",
					szMNIP, sSysType, (pstList->stSCTP[i].cDirection==UP_DIRECTION)?"UP":((pstList->stSCTP[i].cDirection==DOWN_DIRECTION)?"DOWN":"UNKNOWN"),
					pstList->stSCTP[i].huGroupID, pstList->stSCTP[i].szDesc);
				strcat(tempbuf, szTemp);

				if( (dlen = strlen(tempbuf)) > MAX_PRINT_SIZE)
				{
					sprintf(&tempbuf[dlen], "\n     TOTL = [%d], CUR = [%d]\n", pstList->dCount, i);
					break;
				}
			}
		}

		if(res->common.cont_flag == DBM_END)
		{
			sprintf(szTemp, "\n  ................................................................");
			strcat(tempbuf, szTemp);
			sprintf(szTemp, "\n  Total Count = %d ", g_IPCnt+pstList->dCount);
			strcat(tempbuf, szTemp);
			sprintf(szTemp, "\n  ----------------------------------------------------------------");
			strcat(tempbuf, szTemp);

			sprintf(&buf[slen], "\n RESULT = SUCCESS%s", tempbuf);
			g_IPCnt = 0;
		}
		else if(res->common.cont_flag == DBM_CONTINUE)
		{
			sprintf(szTemp, "\n  ----------------------------------------------------------------");
			strcat(tempbuf, szTemp);
			g_IPCnt = g_IPCnt + pstList->dCount;
			sprintf(&buf[slen], "\n RESULT = CONTINUE%s", tempbuf);
			CONT_FLAG = CONT;
		}
		else
			sprintf(&buf[slen], "\n RESULT = TOO MANY RESULTS%s", tempbuf);
	}
}

void dis_thrs_info(char *buf, dbm_msg_t *res, short *dCurCnt, short *dTotCnt)
{
	int				i, slen, dlen;
	char			tempbuf[MAX_PRINT_SIZE + 500], szTemp[1024], sSvcType[11], sDesc[20];
	pst_Thres_MMC	pstList;

    slen = strlen(buf);
    if (res->common.mml_err < 0)
        sprintf(&buf[slen], "\n  RESULT = FAIL\n  REASON = %s", (res->common.mml_err==eOVERMAXROW) ? res->data: MH_ErrMess(res->common.mml_err));
    else if (res->common.mml_err == DBM_SUCCESS )
    {
        sprintf(tempbuf, "\n -------------------------------------------------------------------------------------------------------------");
        sprintf(szTemp, "\n %-10s %8s %6s %6s %6s %7s %7s %6s %6s %8s %8s %-20s",
			"TYPE","TCPSETUP","RESP","UP_T.P","DN_T.P","UP_RETR", "DN_RETR","UP_J.T","DN_J.T","UP_PK.LS","DN_PK.LS","DESC");
        strcat(tempbuf, szTemp);
        sprintf(szTemp,  "\n -------------------------------------------------------------------------------------------------------------");
        strcat(tempbuf, szTemp);

        pstList = (pst_Thres_MMC)&res->data[0];
        if(pstList->dCount < 1)
        {
			sprintf(szTemp, "\n     NO DATA");
			strcat(tempbuf, szTemp);
        }
        else
        {
            for(i = 0; i < pstList->dCount; i++)
            {
				switch(pstList->stThres[i].cSvcType)
				{
					case 1:
						sprintf(sSvcType, "A11");
						break;
					case 2:
						sprintf(sSvcType, "RADIUS");
						break;
					case 3:
						sprintf(sSvcType, "DIAMETER");
						break;
					case 4:
						sprintf(sSvcType, "TCP");
						break;
					case 5:
						sprintf(sSvcType, "FB");
						break;
					case 6:
						sprintf(sSvcType, "PCIV");
						break;
					case 7:
						sprintf(sSvcType, "WAP20");
						break;
					case 8:
						sprintf(sSvcType, "MENU_WIPI");
						break;
					case 9:
						sprintf(sSvcType, "2G");
						break;
					case 10:
						sprintf(sSvcType, "VOD");
						break;
					case 11:
						sprintf(sSvcType, "WIPI");
						break;
					case 12:
						sprintf(sSvcType, "JAVA");
						break;
					case 13:
						sprintf(sSvcType, "ODN");
						break;
					case 14:
						sprintf(sSvcType, "O2G");
						break;
					case 15:
						sprintf(sSvcType, "OVOD");
						break;
					case 16:
						sprintf(sSvcType, "OWIPI");
						break;
					case 17:
						sprintf(sSvcType, "RTS");
						break;
					case 18:
						sprintf(sSvcType, "SVOD");
						break;
					case 19:
						sprintf(sSvcType, "MBOX");
						break;
					case 20:
						sprintf(sSvcType, "MMS");
						break;
					case 21:
						sprintf(sSvcType, "TODAY");
						break;
					case 22:
						sprintf(sSvcType, "WIDGET");
						break;
					case 23:
						sprintf(sSvcType, "PHONE");
						break;
					case 24:
						sprintf(sSvcType, "EMS");
						break;
					case 25:
						sprintf(sSvcType, "FV");
						break;
					case 26:
						sprintf(sSvcType, "IM");
						break;
					case 27:
						sprintf(sSvcType, "VT");
						break;
				}

 				if(strlen(pstList->stThres[i].szDesc) > (sizeof(sDesc)-1))
				{
					strncpy(sDesc, pstList->stThres[i].szDesc, 16);
					sprintf(&sDesc[16], "...");
					sDesc[19] = 0x00;
				}
				else
					strncpy(sDesc, pstList->stThres[i].szDesc, sizeof(sDesc));

               sprintf(szTemp, "\n %-10s %8u %6u %6u %6u %7u %7u %6u %6u %8u %8u %-20s",
					sSvcType,
					pstList->stThres[i].uTCPSetupTime,
					pstList->stThres[i].uResponseTime,
					pstList->stThres[i].uUpThroughput,
					pstList->stThres[i].uDnThroughput,
					pstList->stThres[i].uUpRetransCount,
					pstList->stThres[i].uDnRetransCount,
					pstList->stThres[i].uUpJitter,
					pstList->stThres[i].uDnJitter,
					pstList->stThres[i].uUpPacketLoss,
					pstList->stThres[i].uDnPacketLoss,
					sDesc);

                strcat(tempbuf, szTemp);

                dlen = strlen(tempbuf);
                if(dlen > MAX_PRINT_SIZE) {
                    sprintf(&tempbuf[dlen], "\n     TOTAL = [%d], CUR = [%d]\n", pstList->dCount, i);
                    break;
                }
            }
        }
        if(  res->common.cont_flag == DBM_END )
        {
            sprintf(szTemp, "\n .............................................................................................................");
            strcat(tempbuf, szTemp);
            sprintf(szTemp, "\n  Total Count = %d ",g_dThrsCnt+pstList->dCount);
            strcat(tempbuf, szTemp);
            sprintf(szTemp, "\n -------------------------------------------------------------------------------------------------------------");
            strcat(tempbuf, szTemp);
            sprintf(&buf[slen], "\n RESULT = SUCCESS%s", tempbuf);
            g_dThrsCnt=0;
        }
        else if( res->common.cont_flag == DBM_CONTINUE ) {
            sprintf(szTemp, "\n -------------------------------------------------------------------------------------------------------------");
            strcat(tempbuf, szTemp);
            g_dThrsCnt = g_dThrsCnt+pstList->dCount;
            sprintf(&buf[slen], "\n RESULT = A_CONTINUE%s", tempbuf);
            CONT_FLAG = CONT;
        }
        else
            sprintf(&buf[slen], "\n RESULT = TOO MANY RESULTS%s", tempbuf);
    }
}

void dis_monthrs_info(char *buf, dbm_msg_t *res, short *dCurCnt, short *dTotCnt)
{
	int             	i;
	size_t				slen, dlen;
	char            	tempbuf[MAX_PRINT_SIZE + 500], szTemp[1024], sLocal[BUFSIZ], sSvcType[BUFSIZ], sAlmType[BUFSIZ], sDesc[18];
	char                szSVCIP[DEF_IP_SIZE]; //added by dcham 20110616

	pst_MON_Thres_List	pstList;

	slen = strlen(buf);
	if (res->common.mml_err < 0)
		sprintf(&buf[slen], "\n  RESULT = FAIL\n  REASON = %s", (res->common.mml_err==eOVERMAXROW) ? res->data: MH_ErrMess(res->common.mml_err));
	else if (res->common.mml_err == DBM_SUCCESS)
    {//dcham 20110616
        sprintf(tempbuf, "\n --------------------------------------------------------------------------------------------------------------------");
		sprintf(szTemp,"\n %-8s %-7s %-7s %-15s %5s %8s %8s %9s %6s %8s %7s %-18s",
			"BRANCHID","SYSTYPE","ALMTYPE","SVCIP","START","DAYRANGE","DAYRATE", "NIGHTRATE","DAYMIN","VALIDMIN","PEAK","DESC");
        strcat(tempbuf, szTemp);
        sprintf(szTemp, "\n --------------------------------------------------------------------------------------------------------------------");
        strcat(tempbuf, szTemp);

		pstList = (pst_MON_Thres_List)&res->data[0];
		if(pstList->dCount < 1)
        {
			sprintf(szTemp, "\n     NO DATA");
			strcat(tempbuf, szTemp);
        }
		else
		{
			for(i = 0; i < pstList->dCount; i++)
			{
				switch(pstList->stMonThreshold[i].huBranchID)
				{
					case 0:
						sprintf(sLocal, "NONE");
						break;
					case OFFICE_IDX_GS:
						sprintf(sLocal, "GASAN");
						break;
					case OFFICE_IDX_SA:
						sprintf(sLocal, "SANGAM");
						break;
					case OFFICE_IDX_JA:
						sprintf(sLocal, "JUNGANG");
						break;
					case OFFICE_IDX_IC:
						sprintf(sLocal, "INCHUN");
						break;
					case OFFICE_IDX_SW:
						sprintf(sLocal, "SUWON");
						break;
					case OFFICE_IDX_WJ:
						sprintf(sLocal, "WONJU");
						break;
					case OFFICE_IDX_BS:
						sprintf(sLocal, "BUSAN");
						break;
					case OFFICE_IDX_DG:
						sprintf(sLocal, "DAEGU");
						break;
					case OFFICE_IDX_GJ:
						sprintf(sLocal, "KWANGJU");
						break;
					case OFFICE_IDX_DJ:
						sprintf(sLocal, "DAEJUN");
						break;
					default:
						sprintf(sLocal, "UNKNOWN");
						break;
				}

				switch(pstList->stMonThreshold[i].cSvcType)
				{
					case SYSTEM_TYPE_SECTOR:
						sprintf(sSvcType, "SECTOR");
						break;
					case SYSTEM_TYPE_FA:
						sprintf(sSvcType, "FA");
						break;
					case SYSTEM_TYPE_BTS:
						sprintf(sSvcType, "BTS");
						break;
					case SYSTEM_TYPE_BSC:
						sprintf(sSvcType, "BSC");
						break;
					case SYSTEM_TYPE_PCF:
						sprintf(sSvcType, "PCF");
						break;
					case SYSTEM_TYPE_PDSN:
						sprintf(sSvcType, "PDSN");
						break;
					case SYSTEM_TYPE_AAA:
						sprintf(sSvcType, "AAA");
						break;
					case SYSTEM_TYPE_HSS:
						sprintf(sSvcType, "HSS");
						break;
					case SYSTEM_TYPE_LNS:
						sprintf(sSvcType, "LNS");
						break;
					case SYSTEM_TYPE_SERVICE:
						sprintf(sSvcType, "SERVICE");
						break;
					case SYSTEM_TYPE_ROAMAAA:
						sprintf(sSvcType, "ROAMAAA");
						break;
					case ROAM_ALARM_SYSTYPE:
						sprintf(sSvcType, "ROAM");
						break;
					default:
						sprintf(sSvcType, "UNKNOWN");
						break;
				}

				switch(pstList->stMonThreshold[i].cAlarmType)
				{
					case DEF_ALARMTYPE_CALL:
						sprintf(sAlmType, "CALL");
						break;
					case DEF_ALARMTYPE_RECALL:
						sprintf(sAlmType, "RECALL");
						break;
					case DEF_ALARMTYPE_AAA:
						sprintf(sAlmType, "AAA");
						break;
					case DEF_ALARMTYPE_HSS:
						sprintf(sAlmType, "HSS");
						break;
					case DEF_ALARMTYPE_LNS:
						sprintf(sAlmType, "LNS");
						break;
					case DEF_ALARMTYPE_MENU:
						sprintf(sAlmType, "MENU");
						break;
					case DEF_ALARMTYPE_DN:
						sprintf(sAlmType, "DN");
						break;
					case DEF_ALARMTYPE_STREAM:
						sprintf(sAlmType, "STREAM");
						break;
					case DEF_ALARMTYPE_MMS:
						sprintf(sAlmType, "MMS");
						break;
					case DEF_ALARMTYPE_WIDGET:
						sprintf(sAlmType, "WIDGET");
						break;
					case DEF_ALARMTYPE_PHONE:
						sprintf(sAlmType, "PHONE");
						break;
					case DEF_ALARMTYPE_EMS:
						sprintf(sAlmType, "EMS");
						break;
					case DEF_ALARMTYPE_BANK:
						sprintf(sAlmType, "BANK");
						break;
					case DEF_ALARMTYPE_FV:
						sprintf(sAlmType, "FV");
						break;
					case DEF_ALARMTYPE_IM:
						sprintf(sAlmType, "IM");
						break;
					case DEF_ALARMTYPE_VT:
						sprintf(sAlmType, "VT");
						break;
					case DEF_ALARMTYPE_ETC:
						sprintf(sAlmType, "ETC");
						break;
					case DEF_ALARMTYPE_CORP:
						sprintf(sAlmType, "CORP");
						break;
					case DEF_ALARMTYPE_REGI:
						sprintf(sAlmType, "REGI");
						break;
					case DEF_ALARMTYPE_INET:
						sprintf(sAlmType, "INET");
						break;
					case DEF_ALARMTYPE_RECVCALL:
						sprintf(sAlmType, "INET_RECV");
						break;
					case DEF_ALARMTYPE_IM_RECV:
						sprintf(sAlmType, "IM_RECV");
						break;
					case DEF_ALARMTYPE_VT_RECV:
						sprintf(sAlmType, "VT_RECV");
						break;
					default:
						sprintf(sAlmType, "UNKNOWN");
						break;
				}

				if(strlen(pstList->stMonThreshold[i].szDesc) > (sizeof(sDesc)-1))
				{
					strncpy(sDesc, pstList->stMonThreshold[i].szDesc, 14);
					sprintf(&sDesc[14], "...");
					sDesc[17] = 0x00;
				}
				else
					strncpy(sDesc, pstList->stMonThreshold[i].szDesc, sizeof(sDesc));

				CVT_IPSTR(pstList->stMonThreshold[i].dSvcIP,szSVCIP,DEF_IP_SIZE);

				sprintf(szTemp, "\n %-8s %-7s %-7s %-15s %5hu %8u %8u %9u %6u %8u %7u %-18s",
						sLocal, sSvcType, sAlmType, szSVCIP, //20110616 dcham
						pstList->stMonThreshold[i].cStartHour,
						pstList->stMonThreshold[i].cDayRange,
						pstList->stMonThreshold[i].huDayRate,
						pstList->stMonThreshold[i].huNightRate,
						pstList->stMonThreshold[i].uDayMinTrial,
						pstList->stMonThreshold[i].uNigthMinTrial,
						pstList->stMonThreshold[i].uPeakTrial,
						sDesc);

				strcat(tempbuf, szTemp);

				dlen = strlen(tempbuf);
				if(dlen > MAX_PRINT_SIZE)
				{
					//sprintf(&tempbuf[dlen], "\n     TOTAL = [%d], CUR = [%d]\n", pstList->dCount, i);
					break;
				}
			}
		}

		if(res->common.cont_flag == DBM_END)
		{
			sprintf(szTemp, "\n ....................................................................................................................");
			strcat(tempbuf, szTemp);
			sprintf(szTemp, "\n  Total Count = %d ",g_dThrsCnt+pstList->dCount);
			strcat(tempbuf, szTemp);
			sprintf(szTemp, "\n --------------------------------------------------------------------------------------------------------------------");
			strcat(tempbuf, szTemp);
			sprintf(&buf[slen], "\n RESULT = SUCCESS%s", tempbuf);
			g_dThrsCnt=0;
		}
		else if(res->common.cont_flag == DBM_CONTINUE)
		{
			sprintf(szTemp, "\n --------------------------------------------------------------------------------------------------------------------");
			strcat(tempbuf, szTemp);
			g_dThrsCnt = g_dThrsCnt+pstList->dCount;
			sprintf(&buf[slen], "\n RESULT = A_CONTINUE%s", tempbuf);
			CONT_FLAG = CONT;
		}
		else
			sprintf(&buf[slen], "\n RESULT = TOO MANY RESULTS%s", tempbuf);
    }
}

void dis_equip_info(char *buf, dbm_msg_t *res, short *dCurCnt, short *dTotCnt)
{
    int                     i, slen, dlen;
    char                    tempbuf[MAX_PRINT_SIZE + 500], szTemp[1024];
    char                    szSrcIP[16];
	pst_Info_Equip_MMC		pstList;

    slen = strlen(buf);
    if (res->common.mml_err < 0)
        sprintf(&buf[slen], "\n  RESULT = FAIL\n  REASON = %s", MH_ErrMess(res->common.mml_err) );
    else if (res->common.mml_err == DBM_SUCCESS ){
        sprintf(tempbuf, "\n  ------------------------------------------------------------------------------------------------------");
        sprintf(szTemp, "\n  EQUIP_IP        EQUIPTYPE  NETMASK  EQUTYPENAME   EQUIPNAME       MONFLAG DESC");
        strcat(tempbuf, szTemp);
        sprintf(szTemp, "\n  ------------------------------------------------------------------------------------------------------");
        strcat(tempbuf, szTemp);

        pstList = (pst_Info_Equip_MMC)&res->data[0];
        if(pstList->dCount < 1){
			sprintf(szTemp, "\n     NO DATA");
			strcat(tempbuf, szTemp);
        }
		else{
            for(i = 0; i < pstList->dCount; i++){
                CVT_IPSTR(pstList->stInfoEquip[i].uEquipIP, szSrcIP, 16);
                sprintf(szTemp, "\n  %-16s  %7hu  %7u  %-13s %-15s %-7s %-15s ",
                szSrcIP,
                pstList->stInfoEquip[i].cEquipType,
                pstList->stInfoEquip[i].uNetmask,
                pstList->stInfoEquip[i].szEquipTypeName,
                pstList->stInfoEquip[i].szEquipName,
				pstList->stInfoEquip[i].cMon1MinFlag?"1MIN":"5MIN",
                pstList->stInfoEquip[i].szDesc);
               strcat(tempbuf, szTemp);

                dlen = strlen(tempbuf);
                if(dlen > MAX_PRINT_SIZE){
                    sprintf(&tempbuf[dlen], "\n     TOTL = [%d], CUR = [%d]\n", pstList->dCount, i);
                    break;
                }
            }
        }
        if(  res->common.cont_flag == DBM_END ){
            sprintf(szTemp, "\n  ......................................................................................................");
            strcat(tempbuf, szTemp);
            sprintf(szTemp, "\n  Total Count = %d ",g_dCnt+ pstList->dCount);
            strcat(tempbuf, szTemp);
            sprintf(szTemp, "\n  ------------------------------------------------------------------------------------------------------");
            strcat(tempbuf, szTemp);

            sprintf(&buf[slen], "\n RESULT = SUCCESS%s", tempbuf);
            g_dCnt = 0;

        }
        else if( res->common.cont_flag == DBM_CONTINUE ){
            sprintf(szTemp, "\n  ------------------------------------------------------------------------------------------------------");
            strcat(tempbuf, szTemp);
            g_dCnt = g_dCnt+ pstList->dCount;

            sprintf(&buf[slen], "\n RESULT = CONTINUE%s", tempbuf);
            CONT_FLAG = CONT;
        }else
            sprintf(&buf[slen], "\n RESULT = TOO MANY RESULTS%s", tempbuf);
    }
}

void dis_user_info(char *buf, dbm_msg_t *res, short *dCurCnt, short *dTotCnt)
{
	int					i, slen, dlen;
	char				tempbuf[MAX_PRINT_SIZE + 500], szTemp[1024], szIP[16], szTime[20];
	time_t				tLastLogin;
	struct tm			stTime;
	pst_User_Add_List	pstList;

	slen = strlen(buf);
	if (res->common.mml_err < 0)
		sprintf(&buf[slen], "\n  RESULT = FAIL\n  REASON = %s", (res->common.mml_err==eOVERMAXROW) ? res->data: MH_ErrMess(res->common.mml_err));
	else if (res->common.mml_err == DBM_SUCCESS )
	{
        sprintf(tempbuf, "\n  ---------------------------------------------------------------------------------------");
        sprintf(szTemp,"\n  USERID     LEVEL   LAST_LOGIN_TIME       CONNECT_IP       LOCALNAME   CONTACT");
        strcat(tempbuf, szTemp);
        sprintf(szTemp, "\n  ---------------------------------------------------------------------------------------");
        strcat(tempbuf, szTemp);

        pstList = (pst_User_Add_List)&res->data[0];
        if(pstList->dCount < 1)
        {
			sprintf(szTemp, "\n     NO DATA");
			strcat(tempbuf, szTemp);
        }
        else
        {
            for(i = 0; i < pstList->dCount; i++)
            {
                if(pstList->stUserAdd[i].uLastLoginTime != 0)
				{
					tLastLogin = pstList->stUserAdd[i].uLastLoginTime;
                    localtime_r((time_t*)&tLastLogin, &stTime);
                    sprintf(szTime, "%04d-%02d-%02d %02d:%02d:%02d",
                        stTime.tm_year + 1900, stTime.tm_mon + 1, stTime.tm_mday,
                        stTime.tm_hour, stTime.tm_min, stTime.tm_sec);
                } else {
                    szTime[0] = 0x00;
                }


                CVT_IPSTR(pstList->stUserAdd[i].uConnectIP, szIP, 16);
                sprintf(szTemp,"\n  %-10s     %-2d  %-20s  %-16s %-10s  %-10s",
                pstList->stUserAdd[i].szUserName, pstList->stUserAdd[i].sSLevel,
                szTime, szIP, pstList->stUserAdd[i].szLocalName,
                pstList->stUserAdd[i].szContact);

                strcat(tempbuf, szTemp);

                dlen = strlen(tempbuf);
                if(dlen > MAX_PRINT_SIZE) {
                    sprintf(&tempbuf[dlen], "\n     TOTL = [%d], CUR = [%d]\n", pstList->dCount, i);
                    break;
                }
            }
        }
        if(  res->common.cont_flag == DBM_END )
        {
            sprintf(szTemp, "\n  .......................................................................................");
            strcat(tempbuf, szTemp);
            sprintf(szTemp, "\n  Total Count = %d ",g_dViewCnt+pstList->dCount);
            strcat(tempbuf, szTemp);
            sprintf(szTemp,"\n  ---------------------------------------------------------------------------------------");
            strcat(tempbuf, szTemp);
            g_dViewCnt=0;
            sprintf(&buf[slen], "\n RESULT = SUCCESS%s", tempbuf);
        }
        else if( res->common.cont_flag == DBM_CONTINUE ) {
            sprintf(szTemp,"\n  ---------------------------------------------------------------------------------------");
            strcat(tempbuf, szTemp);
            g_dViewCnt = g_dViewCnt+pstList->dCount;
            sprintf(&buf[slen], "\n RESULT = CONTINUE%s", tempbuf);
            CONT_FLAG = CONT;
        }
        else
            sprintf(&buf[slen], "\n RESULT = TOO MANY RESULTS%s", tempbuf);
    }
}

void dis_load_stat(char *buf, dbm_msg_t *res, short *dCurCnt, short *dTotCnt)
{
	int				i, slen, dlen, tlen, dReadCount;
	char			tempbuf[MAX_PRINT_SIZE + 500], szTemp[1024];
	char			szType[8], szSysType[4], szSystemID[3];
	char			szAvg[7], szMax[7], szMin[7];
	char			szStartTime[20], szFinishTime[20];
    struct tm		stTime;

	pst_SYS_STAT	pstSysStat;

	tlen = sizeof(st_SYS_STAT);
    slen = strlen(buf);
    if(res->common.mml_err < 0)
        sprintf(&buf[slen], "\n  RESULT = FAIL\n  REASON = %s", MH_ErrMess(res->common.mml_err));
    else if(res->common.mml_err == DBM_SUCCESS)
    {
		sprintf(tempbuf, "\n -----------------------------------------------------------------------------------------");
		sprintf(szTemp,  "\n %-8s %-7s  %5s  %-20s %-20s %7s %7s %7s", "TYPE", "SYSTYPE", "SYSID", "START_TIME", "FINISH_TIME", "AVG", "MAX", "MIN");
		strcat(tempbuf, szTemp);
		sprintf(szTemp, "\n -----------------------------------------------------------------------------------------");
		strcat(tempbuf, szTemp);

		for(dReadCount = 0, i = 0; i < 9; i++)
		{
	        pstSysStat = (pst_SYS_STAT)&res->data[tlen*i];
	        if(pstSysStat->usType == 100)
	        {
				if(i == 0)
				{
	            	strcat(tempbuf, "\n     NO DATA\n");
					break;
				}
				else
					continue;
	        }
	        else
	        {
				dReadCount++;
				memset(szType, 0x00, 7);
				switch(pstSysStat->usType)
				{
					case 0:
						sprintf(szType, "%s", "CPU");
						break;
					case 1:
						sprintf(szType, "%s", "MEM");
						break;
					case 2:
						sprintf(szType, "%s", "Queue");
						break;
					case 3:
						sprintf(szType, "%s", "Nifo");
						break;
					case 4:
						sprintf(szType, "%s", "Traffic");
						break;
					case 5:
						sprintf(szType, "%s", "Disk1");
						break;
					case 6:
						sprintf(szType, "%s", "Disk2");
						break;
					case 7:
						sprintf(szType, "%s", "Disk3");
						break;
					case 8:
						sprintf(szType, "%s", "Disk4");
						break;
				}

				memset(szSysType, 0x00, 4);
				switch(pstSysStat->usSysType)
				{
					case 0:
						sprintf(szSysType, "%s", "TAM");
						break;
					case 1:
						sprintf(szSysType, "%s", "TAF");
						break;
				}

				memset(szSystemID, 0x00, 3);
				if(pstSysStat->usSysID <= 32)
					sprintf(szSystemID, "%hu", pstSysStat->usSysID);

				memset(szStartTime, 0x00, 20);
				if(pstSysStat->starttime != 0)
				{
					localtime_r( (time_t*)&pstSysStat->starttime, &stTime);
					sprintf(szStartTime, "%04d-%02d-%02d %02d:%02d:%02d",
						stTime.tm_year+1900, stTime.tm_mon+1, stTime.tm_mday, stTime.tm_hour, stTime.tm_min, stTime.tm_sec);
				}

				memset(szFinishTime, 0x00, 20);
				if(pstSysStat->finishtime != 0)
				{
					memset(&stTime, 0x00, sizeof(stTime));
					localtime_r( (time_t*)&pstSysStat->finishtime, &stTime);
					sprintf(szFinishTime, "%04d-%02d-%02d %02d:%02d:%02d",
						stTime.tm_year+1900, stTime.tm_mon+1, stTime.tm_mday, stTime.tm_hour, stTime.tm_min, stTime.tm_sec);
				}

				memset(szAvg, 0x00, 7);
				if( (pstSysStat->uiAvg >= 0) && (pstSysStat->uiAvg <= 10000))
					sprintf(szAvg, "%5.2f%%", (float)pstSysStat->uiAvg/100);

				memset(szMax, 0x00, 7);
				if( (pstSysStat->uiMax >= 0) && (pstSysStat->uiMax <= 10000))
					sprintf(szMax, "%5.2f%%", (float)pstSysStat->uiMax/100);

				memset(szMin, 0x00, 7);
				if( (pstSysStat->uiMin >= 0) && (pstSysStat->uiMin <= 10000))
					sprintf(szMin, "%5.2f%%", (float)pstSysStat->uiMin/100);

				sprintf(szTemp,"\n %-8s %-7s  %5s  %-20s %-20s %7s %7s %7s",
					szType, szSysType, szSystemID, szStartTime, szFinishTime, szAvg, szMax, szMin);

				strcat(tempbuf, szTemp);

				dlen = strlen(tempbuf);
				if(dlen > MAX_PRINT_SIZE)
				{
					sprintf(&tempbuf[dlen], "\n     TOTL = [%d], CUR = [%d]\n", 9, dReadCount);
					break;
				}
	        }
		}
		sprintf(szTemp, "\n -----------------------------------------------------------------------------------------");
		strcat(tempbuf, szTemp);

		if(res->common.cont_flag == DBM_END)
			sprintf(&buf[slen], "\n RESULT = SUCCESS%s", tempbuf);
		else
			sprintf(&buf[slen], "\n RESULT = TOO MANY RESULTS%s", tempbuf);
    }
	return;
}

void dis_fault_stat(char *buf, dbm_msg_t *res, short *dCurCnt, short *dTotCnt)
{
	int				i, slen, dlen, tlen;
	char			tempbuf[MAX_PRINT_SIZE + 500], szTemp[1024];
	char			szType[7], szSysType[4], szSystemID[3];
	char			szCri[12], szMax[12], szMin[12], szStop[12];
	char			szStartTime[20], szFinishTime[20];
    struct tm		stTime;

	pst_SYS_STAT	pstSysStat;

	tlen		= sizeof(st_SYS_STAT);
	slen		= strlen(buf);
    if(res->common.mml_err < 0)
        sprintf(&buf[slen], "\n  RESULT = FAIL\n  REASON = %s", MH_ErrMess(res->common.mml_err));
    else if(res->common.mml_err == DBM_SUCCESS)
    {
		sprintf(tempbuf, "\n -------------------------------------------------------------------------------------------------");
		sprintf(szTemp,  "\n %-4s %-7s %5s  %-20s %-20s %8s %8s %8s %8s", "TYPE", "SYSTYPE", "SYSID", "START_TIME", "FINISH_TIME", "CRI", "MAJ", "MIN", "STOP");
		strcat(tempbuf, szTemp);
		sprintf(szTemp,  "\n -------------------------------------------------------------------------------------------------");
		strcat(tempbuf, szTemp);

		for(i = 0; i < 2; i++)
		{
	        pstSysStat = (pst_SYS_STAT)&res->data[tlen*i];
	        if(pstSysStat->usType == 100)
	        {
				if(i == 0)
		            strcat(tempbuf, "\n     NO DATA\n");
				break;
	        }
	        else
	        {
				memset(szType, 0x00, 7);
				switch(pstSysStat->usType)
				{
					case 1:
						sprintf(szType, "%s", "SW");
						break;
					case 0:
						sprintf(szType, "%s", "HW");
						break;
				}

				memset(szSysType, 0x00, 4);
				switch(pstSysStat->usSysType)
				{
					case 0:
						sprintf(szSysType, "%s", "TAM");
						break;
					case 1:
						sprintf(szSysType, "%s", "TAF");
						break;
				}

				memset(szSystemID, 0x00, 3);
				if(pstSysStat->usSysID <= 32)
					sprintf(szSystemID, "%hd", pstSysStat->usSysID);

				memset(szStartTime, 0x00, 20);
				if(pstSysStat->starttime != 0)
				{
					localtime_r( (time_t*)&pstSysStat->starttime, &stTime);
					sprintf(szStartTime, "%04d-%02d-%02d %02d:%02d:%02d",
						stTime.tm_year+1900, stTime.tm_mon+1, stTime.tm_mday, stTime.tm_hour, stTime.tm_min, stTime.tm_sec);
				}

				memset(szFinishTime, 0x00, 20);
				if(pstSysStat->finishtime != 0)
				{
					memset(&stTime, 0x00, sizeof(stTime));
					localtime_r( (time_t*)&pstSysStat->finishtime, &stTime);
					sprintf(szFinishTime, "%04d-%02d-%02d %02d:%02d:%02d",
						stTime.tm_year+1900, stTime.tm_mon+1, stTime.tm_mday, stTime.tm_hour, stTime.tm_min, stTime.tm_sec);
				}

				memset(szCri, 0x00, 12);
				sprintf(szCri, "%u", pstSysStat->uiCri);

				memset(szMax, 0x00, 12);
				sprintf(szMax, "%u", pstSysStat->uiMaj);

				memset(szMin, 0x00, 12);
				sprintf(szMin, "%u", pstSysStat->uiMin);

				memset(szStop, 0x00, 12);
				sprintf(szStop, "%u", pstSysStat->uiStop);

				sprintf(szTemp,"\n %-4s %-7s %5s  %-20s %-20s %8s %8s %8s %8s",
					szType, szSysType, szSystemID, szStartTime, szFinishTime, szCri, szMax, szMin, szStop);

				strcat(tempbuf, szTemp);

				dlen = strlen(tempbuf);
				if(dlen > MAX_PRINT_SIZE)
				{
					sprintf(&tempbuf[dlen], "\n     TOTL = [%d], CUR = [%d]\n", 8, i);
					break;
				}
	        }
		}
		sprintf(szTemp,  "\n -------------------------------------------------------------------------------------------------");
		strcat(tempbuf, szTemp);

		if(res->common.cont_flag == DBM_END)
			sprintf(&buf[slen], "\n RESULT = SUCCESS%s", tempbuf);
		else
			sprintf(&buf[slen], "\n RESULT = TOO MANY RESULTS%s", tempbuf);
    }
	return;
}

void dis_traffic_stat(char *buf, dbm_msg_t *res, short *dCurCnt, short *dTotCnt)
{
	int					slen, tlen;
	char				tempbuf[MAX_PRINT_SIZE + 500], szTemp[1024];

	pst_traffic_stat	pstTrafficStat;

	tlen		= sizeof(pst_traffic_stat);
    slen		= strlen(buf);
    if(res->common.mml_err < 0)
        sprintf(&buf[slen], "\n  RESULT = FAIL\n  REASON = %s", MH_ErrMess(res->common.mml_err));
    else if(res->common.mml_err == DBM_SUCCESS)
    {
        pstTrafficStat = (pst_traffic_stat)&res->data[0];

		sprintf(tempbuf, "\n  --------------------------------------------------------");
		sprintf(szTemp,  "\n  %-12s  %20s  %20s", "TYPE", "SUM(FRAMES)", "SUM(BYTES)");
		strcat(tempbuf, szTemp);
		sprintf(szTemp, "\n  --------------------------------------------------------");
		strcat(tempbuf, szTemp);

		sprintf(szTemp,  "\n  %-12s  %20u  %20lu", "THRUSTAT", pstTrafficStat->uThruStatFrames, pstTrafficStat->ulThruStatBytes);
		strcat(tempbuf, szTemp);

		sprintf(szTemp,  "\n  %-12s  %20u  %20lu", "TOTAL", pstTrafficStat->uTotStatFrames, pstTrafficStat->ulTotStatBytes);
		strcat(tempbuf, szTemp);

		sprintf(szTemp,  "\n  %-12s  %20u  %20lu", "IP", pstTrafficStat->uIPStatFrames, pstTrafficStat->ulIPStatBytes);
		strcat(tempbuf, szTemp);

		sprintf(szTemp,  "\n  %-12s  %20u  %20lu", "UDP", pstTrafficStat->uUDPStatFrames, pstTrafficStat->ulUDPStatBytes);
		strcat(tempbuf, szTemp);

		sprintf(szTemp,  "\n  %-12s  %20u  %20lu", "TCP", pstTrafficStat->uTCPStatFrames, pstTrafficStat->ulTCPStatBytes);
		strcat(tempbuf, szTemp);

		sprintf(szTemp,  "\n  %-12s  %20u  %20lu", "SCTP", pstTrafficStat->uSCTPStatFrames, pstTrafficStat->ulSCTPStatBytes);
		strcat(tempbuf, szTemp);

		sprintf(szTemp,  "\n  %-12s  %20u  %20lu", "ETC", pstTrafficStat->uETCStatFrames, pstTrafficStat->ulETCStatBytes);
		strcat(tempbuf, szTemp);

		sprintf(szTemp,  "\n  %-12s  %20u  %20lu", "IPError", pstTrafficStat->uIPErrorFrames, pstTrafficStat->ulIPErrorBytes);
		strcat(tempbuf, szTemp);

		sprintf(szTemp,  "\n  %-12s  %20u  %20lu", "UTCPError", pstTrafficStat->uUTCPErrorFrames, pstTrafficStat->ulUTCPErrorBytes);
		strcat(tempbuf, szTemp);

		sprintf(szTemp,  "\n  %-12s  %20u  %20lu", "FAIL Data", pstTrafficStat->uFailDataFrames, pstTrafficStat->ulFailDataBytes);
		strcat(tempbuf, szTemp);

		sprintf(szTemp,  "\n  %-12s  %20u  %20lu", "Filter Out", pstTrafficStat->uFilterOutFrames, pstTrafficStat->ulFilterOutBytes);
		strcat(tempbuf, szTemp);

		sprintf(szTemp, "\n  --------------------------------------------------------");
		strcat(tempbuf, szTemp);

		if(res->common.cont_flag == DBM_END)
			sprintf(&buf[slen], "\n RESULT = SUCCESS%s", tempbuf);
		else
			sprintf(&buf[slen], "\n RESULT = TOO MANY RESULTS%s", tempbuf);
    }
}

void dis_cmd_his(char *buf, dbm_msg_t *res, short *dCurCnt, short *dTotCnt)
{
	int			i;
	char		tempbuf[MAX_PRINT_SIZE + 500], sTemp[1024], sTimeBuf[30], sUserIP[DEF_IP_SIZE], sName[12], sCommand[32];
	time_t		tTmpTime;
	size_t		szLen, szTotalLen;
	struct tm	stTime;
	st_CmdList	*pstList;

    szTotalLen = strlen(buf);
    if (res->common.mml_err < 0)
        sprintf(&buf[szTotalLen], "\n  RESULT = FAIL\n  REASON = %s", MH_ErrMess(res->common.mml_err) );
    else if (res->common.mml_err == DBM_SUCCESS )
    {
        sprintf(tempbuf,  "\n ----------------------------------------------------------------------------------");
        sprintf(sTemp,"\n %-12s %-20s %-16s %-32s", "USER", "TIME", "CONNECTED IP", "COMMAND");
        strcat(tempbuf, sTemp);
        sprintf(sTemp, "\n ----------------------------------------------------------------------------------");
        strcat(tempbuf, sTemp);

        pstList = (pst_CmdList)&res->data[0];
        if(res->head.msg_len == 0)
        {
			sprintf(sTemp, "\n     NO DATA");
			strcat(tempbuf, sTemp);
        }
        else
        {
            for(i = 0; i < pstList->dCount ; i++)
            {
				if( (szLen = strlen(pstList->stCmd[i].szUserName)) > 8)
				{
					strncpy(sName, pstList->stCmd[i].szUserName, 8);
					sName[8] = 0x00;
					strcat(sName, "...");
				}
				else
					strncpy(sName, pstList->stCmd[i].szUserName, 12);

				tTmpTime = pstList->stCmd[i].uiTime;
                localtime_r(&tTmpTime, &stTime);
                sprintf(sTimeBuf, "%04d-%02d-%02d %02d:%02d:%02d", stTime.tm_year +1900, stTime.tm_mon + 1,
                                stTime.tm_mday, stTime.tm_hour, stTime.tm_min, stTime.tm_sec);

				CVT_IPSTR(pstList->stCmd[i].uiUserBIP, sUserIP, DEF_IP_SIZE);

				if( (szLen = strlen(pstList->stCmd[i].szCommand)) > 28)
				{
					strncpy(sCommand, pstList->stCmd[i].szCommand, 28);
					sCommand[28] = 0x00;
					strcat(sCommand, "...");
				}
				else
					strncpy(sCommand, pstList->stCmd[i].szCommand, 32);

                sprintf(sTemp, "\n %-12s %-20s %-16s %-32s", sName, sTimeBuf, sUserIP, sCommand);

                strcat(tempbuf, sTemp);
                szLen = strlen(tempbuf);
                if(szLen > MAX_PRINT_SIZE) {
                    sprintf(&tempbuf[szLen], "\n     TOTL = [%d], CUR = [%d]\n", pstList->dCount, i);
                    break;
                }
            }
        }
        if(  res->common.cont_flag == DBM_END )
        {
            if(pstList->dCount != 0) {
                sprintf(sTemp, "\n ..................................................................................");
                strcat(tempbuf, sTemp);
                sprintf(sTemp, "\n  Total Count = %d ", g_dCmdCnt + pstList->dCount);
                strcat(tempbuf, sTemp);
                sprintf(sTemp,"\n ----------------------------------------------------------------------------------");
                strcat(tempbuf, sTemp);
            }
            sprintf(&buf[szTotalLen], "\n RESULT = SUCCESS%s", tempbuf);
            g_dCmdCnt = 0;
        }
        else if( res->common.cont_flag == DBM_CONTINUE )
        {
            sprintf(sTemp,"\n ----------------------------------------------------------------------------------");
            strcat(tempbuf, sTemp);
            sprintf(&buf[szTotalLen], "\n RESULT = CONTINUE%s", tempbuf);
            g_dCmdCnt += pstList->dCount;
            CONT_FLAG = CONT;
        }
        else
            sprintf(&buf[szTotalLen], "\n RESULT = TOO MANY RESULTS%s", tempbuf);
    }
}

void dis_flt_his(char *buf, dbm_msg_t *res, short *dCurCnt, short *dTotCnt)
{
    int                 i, slen, dlen;
    char                tempbuf[MAX_PRINT_SIZE + 500], szTemp[1024];

    pst_CondMsg_List    pstList;

    slen = strlen(buf);
    if (res->common.mml_err < 0)
        sprintf(&buf[slen], "\n  RESULT = FAIL\n  REASON = %s", MH_ErrMess(res->common.mml_err) );
    else if(res->common.mml_err == DBM_SUCCESS )
    {
        pstList = (pst_CondMsg_List)&res->data[0];
        if(pstList->dCount == 0)
        {
			sprintf(szTemp, "\n     NO DATA");
			strcat(tempbuf, szTemp);
        }
        else
        {
            sprintf(tempbuf, "\n");
            //sprintf(tempbuf, "\n ------------------------------------------------------");
            for(i = 0; i < pstList->dCount ; i++)
            {
                //MakeStringTime(pstList->stCONDMsg[i].uiTime, szTimeBuf);
                sprintf(szTemp,"\n %s", pstList->stCONDMsg[i].szMessage);

                strcat(tempbuf, szTemp);
                dlen = strlen(tempbuf);
                if(dlen > MAX_PRINT_SIZE) {
                    sprintf(&tempbuf[dlen], "\n     TOTL = [%d], CUR = [%d]\n", pstList->dCount, i);
                    break;
                }
            }
        }

        if(  res->common.cont_flag == DBM_END )
        {
            if(pstList->dCount != 0)
            {
            //    sprintf(szTemp, "\n ......................................................");
           ////     strcat(tempbuf, szTemp);
                sprintf(szTemp, "\n TOTAL COUNT = %d ", g_dFltCnt + pstList->dCount);
                strcat(tempbuf, szTemp);
             //   sprintf(szTemp,"\n ------------------------------------------------------");
             //   strcat(tempbuf, szTemp);

            }

            sprintf(&buf[slen], "\n RESULT = SUCCESS%s", tempbuf);
            g_dFltCnt = 0;
        }
        else if( res->common.cont_flag == DBM_CONTINUE )
        {
          //  sprintf(szTemp,"\n ------------------------------------------------------");
            //strcat(tempbuf, szTemp);
            sprintf(&buf[slen], "\n RESULT = CONTINUE%s", tempbuf);
            g_dFltCnt += pstList->dCount;
            CONT_FLAG = CONT;
        }
        else
            sprintf(&buf[slen], "\n RESULT = TOO MANY RESULTS%s", tempbuf);
    }
}

/*
	trace_tbl의 해당 파일로부터 shared memory가 초기화 될 때 파일로부터 로딩한다.
	 - Writer: Han-jin Park
	 - DAte: 2008.09.19
*/
void dis_trc_info(char *buf, dbm_msg_t *res, short *dCurCnt, short *dTotCnt)
{
	int				i, slen;
	char			tempbuf[3500], szTemp[1024], regDateStr[17];
	struct tm		regDate;
	time_t			startDate;

	int				dCnt;
	st_TraceList	*pstList;

	memset(tempbuf, 0x00, sizeof(tempbuf));
	slen = strlen(buf);
	if(res->common.mml_err < 0)
		sprintf(&buf[slen], "\n  RESULT = FAIL\n  REASON = %s", MH_ErrMess(res->common.mml_err));
	else if(res->common.mml_err == DBM_SUCCESS)
	{
		pstList = (st_TraceList*)&res->data[0];

		sprintf(tempbuf, "\n -------------------------------------------------------------------------------------");
		sprintf(szTemp, "\n SYSTEM NUMBER = %d", pstList->dSysNo);
		strcat(tempbuf, szTemp);
		sprintf(szTemp, "\n -------------------------------------------------------------------------------------");
		strcat(tempbuf, szTemp);
		sprintf(szTemp, "\n %-9s  %-16s  %-16s  %-13s  %19s", "TYPE", "TRACE VALUE", "REG TIME", "ESTIMATE HOUR", "REGID");
		strcat(tempbuf, szTemp);
		sprintf(szTemp, "\n --------------------------------------------------------------------------------------");
		strcat(tempbuf, szTemp);

		if(pstList->count == 0)
		{
			sprintf(szTemp, "\n     NO DATA FOR TRACE");
			strcat(tempbuf, szTemp);
		}
		else
		{
			dCnt = (res->head.msg_len-(sizeof(int)*2)) / sizeof(st_TraceInfo);
			for(i = 0; i < dCnt; i++)
			{
				memset(&regDate, 0x00, sizeof(struct tm));
				startDate = pstList->stTraceInfo[i].tExpiredTime - (pstList->stTraceInfo[i].usEstimatedTime * 3600);
				localtime_r((const time_t*)&startDate, &regDate);

				regDateStr[0] = 0x00;
				sprintf(regDateStr, "%04d-%02d-%02d %02d:%02d", regDate.tm_year+1900, regDate.tm_mon+1, regDate.tm_mday, regDate.tm_hour, regDate.tm_min);
				regDateStr[16] = 0x00;

				if(pstList->stTraceInfo[i].dType == TRC_TYPE_IMSI)
				{
					sprintf(szTemp, "\n %-9s  %-16s  %-16s  %13u  %19s", "IMSI", pstList->stTraceInfo[i].stTraceID.szMIN,
						regDateStr, pstList->stTraceInfo[i].usEstimatedTime, pstList->stTraceInfo[i].adminID);
				}
				else if(pstList->stTraceInfo[i].dType == TRC_TYPE_MDN)
				{
					sprintf(szTemp, "\n %-9s  %-16s  %-16s  %13u  %19s", "CTN", pstList->stTraceInfo[i].stTraceID.szMIN,
						regDateStr, pstList->stTraceInfo[i].usEstimatedTime, pstList->stTraceInfo[i].adminID);
				}
				else if(pstList->stTraceInfo[i].dType == TRC_TYPE_ROAM_IMSI)
				{
					sprintf(szTemp, "\n %-9s  %-16s  %-16s  %13u  %19s", "ROAM_IMSI", pstList->stTraceInfo[i].stTraceID.szMIN,
						regDateStr, pstList->stTraceInfo[i].usEstimatedTime, pstList->stTraceInfo[i].adminID);
				}
				else if(pstList->stTraceInfo[i].dType == TRC_TYPE_ROAM_MDN)
				{
					sprintf(szTemp, "\n %-9s  %-16s  %-16s  %13u  %19s", "ROAM_CTN", pstList->stTraceInfo[i].stTraceID.szMIN,
						regDateStr, pstList->stTraceInfo[i].usEstimatedTime, pstList->stTraceInfo[i].adminID);
				}
				strcat(tempbuf, szTemp);
			}
		}

		if(res->common.cont_flag == DBM_END)
		{
			if(pstList->count != 0)
			{
				sprintf(szTemp, "\n .....................................................................................");
				strcat(tempbuf, szTemp);
				sprintf(szTemp, "\n TOTAL COUNT = %d ", pstList->count);
				strcat(tempbuf, szTemp);
				sprintf(szTemp, "\n -------------------------------------------------------------------------------------");
				strcat(tempbuf, szTemp);
			}
			sprintf(&buf[slen], "\n RESULT = SUCCESS%s", tempbuf);
		}
		else if(res->common.cont_flag == DBM_CONTINUE)
		{
			if(pstList->count != 0)
			{
				sprintf(szTemp, "\n -------------------------------------------------------------------------------------");
				strcat(tempbuf, szTemp);
			}
			sprintf(&buf[slen], "\n RESULT = CONTINUE%s", tempbuf);
		}

	}
}

