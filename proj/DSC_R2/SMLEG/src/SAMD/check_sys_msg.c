
/**A. File Inclusion *******************************************************/
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <check_sys_msg.h>
//#include <sfm_msgtypes.h>
#include "samd.h"

/** B. Declaration of Variables  ********************************************/
int   gdMsgOldSize;

/** C. Declaration of Extern Variables  *************************************/

extern SFM_SysCommMsgType	*loc_sadb;
extern char     trcBuf[TRCBUF_LEN];

/*******************************************************************************
 DISK, POWER의 상태를 MESSAGE FILE을 통해서 감시하는 함수
*******************************************************************************
 * by written june.
 * 1. 읽을 Message File의 크기가 읽은 크기보다 크면 open한 file의 끝까지 읽어 처리하되
 *    parsing한 string의 내용 중 기대하던 token이 있으면 해당 라인까지만 처리하고
 *    위치를 기록하고 sfdb로 정보를 갱신하여 OMP로 전달한다.
 * 2. 읽은 Message File의 크기가 읽은 크기보다 작으면 log lotation으로 간주하고
 *    lotation 이전의 파일의 크기를 조사하여 읽은 크기보다 크면 file의 끝까지 1번과 같이
 *    처리하고 새로 생성도니 Message File의 첨부터 open 시점까지의 내용을 읽어 1번과 같이
 *    처리한다.
 * 3. 주의] Message File 감시 타임과 sfdb를 통한 OMC로 보고 타임과의 관계를 고려하여야 
 *    한다. 최악의 경우 라인별로 감시하였으나 같은 HW에 대한 상태변화가 1번 이상 
 *    변경 되었음에도 불구하고 OMC로 보고되는 내용은 최종 한번만 보고 될수도 있다.
 *    이 경우 감시 타임과 보고 타임을 조정하여야 한다. 
 */
int dCheckHW_SysMsg(void)
{
	char	*szTmpBuf1;
	char	szMsgBuf[SYS_MESSAGE_LINE_SIZE];
	FILE	*fp;
	int 	dHwIndex=-1;
	int 	dHwStatus=0;
	struct 	stat 	stStat, stStatB;

	stat(SYS_MESSAGE_FILE, &stStat);

	/*** MESSAGE FILE에 변화가 없는 경우 **************************************/
	if (stStat.st_size == gdMsgOldSize) {
		/*fprintf (stderr, "Don't change\n");*/ return 1;
	}

	/*** MESSAGE FILE에 변화가 감지된 경우 ************************************/
	else if (stStat.st_size > gdMsgOldSize) {
		fp = fopen( SYS_MESSAGE_FILE, "r" );
		if (fp == NULL) {
			sprintf(trcBuf,"[%s] fopen fail[%s]; err=%d(%s)\n"
					, __FUNCTION__, MESSAGE_FILE, errno, strerror(errno));
			trclib_writeLogErr (FL,trcBuf);
			return -1;
		}

		fseek( fp, gdMsgOldSize, SEEK_SET );

		while (fgets(szMsgBuf, SYS_MESSAGE_LINE_SIZE, fp) != NULL)
		{
			/* CHECK POWER */
			if ((szTmpBuf1 = strstr(szMsgBuf, "PSU")) != NULL) {
				dHwStatus =	dStatusPower(szMsgBuf, &dHwIndex);
				if (!strcasecmp(loc_sadb->sysHW[dHwIndex].StsName, "PWR1")) {
					sprintf(loc_sadb->sysSts.pwrSts[0].StsName, "PWR1");
					loc_sadb->sysSts.pwrSts[0].status = dHwStatus - 1;
				}
				else if (!strcasecmp(loc_sadb->sysHW[dHwIndex].StsName, "PWR2")) {
					sprintf(loc_sadb->sysSts.pwrSts[1].StsName, "PWR1");
					loc_sadb->sysSts.pwrSts[1].status = dHwStatus - 1;
				}
			}
			/* CHECK FAN */
			else if ((szTmpBuf1 = strstr(szMsgBuf, "SYS_FAN")) != NULL) {
				dHwStatus =	dStatusFan(szMsgBuf, &dHwIndex);
				if (!strcasecmp(loc_sadb->sysHW[dHwIndex].StsName, "FAN1")) {
					sprintf(loc_sadb->sysSts.pwrSts[0].StsName, "FAN1");
					loc_sadb->sysSts.pwrSts[0].status = dHwStatus - 1;
				}
				else if (!strcasecmp(loc_sadb->sysHW[dHwIndex].StsName, "FAN2")) {
					sprintf(loc_sadb->sysSts.pwrSts[1].StsName, "FAN2");
					loc_sadb->sysSts.pwrSts[1].status = dHwStatus - 1;
				}
				else if (!strcasecmp(loc_sadb->sysHW[dHwIndex].StsName, "FAN3")) {
					sprintf(loc_sadb->sysSts.pwrSts[2].StsName, "FAN3");
					loc_sadb->sysSts.pwrSts[2].status = dHwStatus - 1;
				}
			}
			gdMsgOldSize += strLen_LF(szMsgBuf);
			if (dHwStatus > 0) {
				loc_sadb->sysHW[dHwIndex].status = dHwStatus - 1;
				/* ADD: BY JUNE, 2010-01-06
				 *    - loc_sadb->sysSts 에 fan/power 정보 할당 */
#if 1
				sprintf(trcBuf,"[%s] hw[%d] status:%d FP(%d) loc_sadb(%d)\n"
						, __FUNCTION__, dHwIndex, dHwStatus, gdMsgOldSize, loc_sadb->sysHW[dHwIndex].status);
				trclib_writeLogErr (FL,trcBuf);
#endif
				break;
			}
			dHwStatus = dHwIndex = -1;
		}

		if (dHwStatus == 0)
			gdMsgOldSize = stStat.st_size;
		
		fclose(fp);
	}
	/*** MESSAGE FILE에 변화가 감지되었고, 파일이 새로 변경된 경우 ***********
	 * log lotate 된 경우로 이전 파일을 처리하는 루틴이며, 새로 생성된 파일은
	 * "if (stStat.st_size > gdMsgOldSize)" 이 부분에서 처리 하도록 함.
	 */
	else if (stStat.st_size < gdMsgOldSize) {
		
		stat(SYS_MESSAGE_FILE_1, &stStatB);
		
		if (stStatB.st_size > gdMsgOldSize) {
			fp = fopen( SYS_MESSAGE_FILE_1, "r" );
			if (fp == NULL) {
				sprintf(trcBuf,"[%s] fopen fail[%s]; err=%d(%s)\n"
						, __FUNCTION__, MESSAGE_FILE_1, errno, strerror(errno));
				trclib_writeLogErr (FL,trcBuf);
				return -1;
			}
			fseek(fp, gdMsgOldSize, SEEK_SET);
			while (fgets(szMsgBuf, SYS_MESSAGE_LINE_SIZE, fp) != NULL)
			{
				/* CHECK POWER */
				if ((szTmpBuf1 = strstr(szMsgBuf, "PSU")) != NULL) {
					dHwStatus =	dStatusPower(szMsgBuf, &dHwIndex);
				}
				/* CHECK FAN */
				else if ((szTmpBuf1 = strstr(szMsgBuf, "SYS_FAN")) != NULL) {
					dHwStatus =	dStatusFan(szMsgBuf, &dHwIndex);
				}
				gdMsgOldSize += strLen_LF(szMsgBuf);
				if (dHwStatus > 0) {
					loc_sadb->sysHW[dHwIndex].status = dHwStatus - 1;
#if 1
					sprintf(trcBuf,"[%s] hw[%d] status:%d FP(%d)\n"
							, __FUNCTION__, dHwIndex, dHwStatus, gdMsgOldSize);
					trclib_writeLogErr (FL,trcBuf);
#endif
					break;
				}
				dHwStatus = dHwIndex = 0;
			}
			
			if (dHwStatus == 0)
				gdMsgOldSize = stStat.st_size;
			
			fclose(fp);
		}
	
		// lotation 이전 message file을 끝가지 다 읽으면 새로운 파일을 읽기 위해 읽은 위치 초기화
		if (stStatB.st_size <= gdMsgOldSize) {
			gdMsgOldSize = 0;
		}
	}
	return 1;
}

/*******************************************************************************
 문자열로부터 token단위로 단어를 parsing하여 기대 WORD를 통하여
 POWER의 H/W INDEX와 상태를 체크하는 함수.
 * szMsgBuf : messages file로 부터 line단위로 읽어 들인 string
 * dIndex   : 문자열로부터 POWER의 INDEX를 찾아 호출부로 넘긴다.
 * Return   : 해당 POWER의 상태(0: NONE , 1: CRITICAL, 2: NOMAL)를 리턴한다.
*******************************************************************************/
int	dStatusPower(char *szMsgBuf, int *dIndex)
{
	char 	*ptr=NULL, *next=NULL, *token=NULL;
	int 	dRet=0;
	
	ptr = szMsgBuf;

	while (1)
	{
		token = (char*)strtok_r(ptr," ",&next);
		if ((token==NULL) || (strcmp(token, "\n")==NULL)) break;

		else if (strncasecmp(token, "PS0", 3)==NULL) {
			*dIndex = get_hwinfo_index ("PWR1");	
		}
		else if (strncasecmp(token, "PS0.", 4)==NULL) {
			*dIndex = get_hwinfo_index ("PWR1");	
		}
		else if (strncasecmp(token, "PS1", 3)==NULL) {
			*dIndex = get_hwinfo_index ("PWR2");	
		}
		else if (strncasecmp(token, "PS1.", 4)==NULL) {
			*dIndex = get_hwinfo_index ("PWR2");	
		}
#if 0
		else if (strncasecmp(token, "inserted.", 9)==NULL) {
			dRet = 1;
		}
#endif
		else if (strncasecmp(token, "removed.", 8)==NULL) {
			dRet = 2;
		}
		else if (strncasecmp(token, "unavailable", 11)==NULL) {
			dRet = 2;
		}
		else if (strncasecmp(token, "failed", 6)==NULL) {
			dRet = 2;
		}
		else if (strncasecmp(token, "fail", 4)==NULL) {
			dRet = 2;
		}

		ptr = next;
	}
	return dRet;
}

/*******************************************************************************
 문자열로부터 token단위로 단어를 parsing하여 기대 WORD를 통하여
 FAN의 H/W INDEX와 상태를 체크하는 함수.
 * szMsgBuf : messages file로 부터 line단위로 읽어 들인 string
 * dIndex   : 문자열로부터 POWER의 INDEX를 찾아 호출부로 넘긴다.
 * Return   : 해당 POWER의 상태(0: NONE , 1: CRITICAL, 2: NOMAL)를 리턴한다.
*******************************************************************************/
int	dStatusFan(char *szMsgBuf, int *dIndex)
{
	char 	*ptr=NULL, *next=NULL, *token=NULL;
	int 	dRet=0;
	
	ptr = szMsgBuf;

	while (1)
	{
		token = (char*)strtok_r(ptr," ",&next);
		if ((token==NULL) || (strcmp(token, "\n")==NULL)) break;

		else if (strncasecmp(token, "FT0/FM0", 7)==NULL) {
			*dIndex = get_hwinfo_index ("FAN1");
		}
		else if (strncasecmp(token, "FT0/FM1", 7)==NULL) {
			*dIndex = get_hwinfo_index ("FAN2");
		}
		else if (strncasecmp(token, "FT0/FM2", 7)==NULL) {
			*dIndex = get_hwinfo_index ("FAN3");
		}
		else if (strncasecmp(token, "inserted.", 9)==NULL) {
			dRet = 1;
		}
		else if (strncasecmp(token, "removed.", 8)==NULL) {
			dRet = 2;
		}
		else if (strncasecmp(token, "unavailable", 11)==NULL) {
			dRet = 2;
		}
		else if (strncasecmp(token, "failed", 6)==NULL) {
			dRet = 2;
		}
		else if (strncasecmp(token, "fail", 4)==NULL) {
			dRet = 2;
		}

		ptr = next;
	}
	return dRet;
}

/*******************************************************************************
 LF(\n) 을 만날때 까지의 string의 길이를 구하는 함수.
*******************************************************************************/
int strLen_LF(const char* ptr)
{
	int i = 0;

	while (ptr[i++] != '\n') {}
	return (i);
}

#if 0
int main (void)
{
	while (1)
	{
		dCheckHW ();

		sleep(1);
	}

	return 0;
}
#endif
