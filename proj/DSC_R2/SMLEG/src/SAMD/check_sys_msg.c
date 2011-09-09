
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
 DISK, POWER�� ���¸� MESSAGE FILE�� ���ؼ� �����ϴ� �Լ�
*******************************************************************************
 * by written june.
 * 1. ���� Message File�� ũ�Ⱑ ���� ũ�⺸�� ũ�� open�� file�� ������ �о� ó���ϵ�
 *    parsing�� string�� ���� �� ����ϴ� token�� ������ �ش� ���α����� ó���ϰ�
 *    ��ġ�� ����ϰ� sfdb�� ������ �����Ͽ� OMP�� �����Ѵ�.
 * 2. ���� Message File�� ũ�Ⱑ ���� ũ�⺸�� ������ log lotation���� �����ϰ�
 *    lotation ������ ������ ũ�⸦ �����Ͽ� ���� ũ�⺸�� ũ�� file�� ������ 1���� ����
 *    ó���ϰ� ���� �������� Message File�� ÷���� open ���������� ������ �о� 1���� ����
 *    ó���Ѵ�.
 * 3. ����] Message File ���� Ÿ�Ӱ� sfdb�� ���� OMC�� ���� Ÿ�Ӱ��� ���踦 ����Ͽ��� 
 *    �Ѵ�. �־��� ��� ���κ��� �����Ͽ����� ���� HW�� ���� ���º�ȭ�� 1�� �̻� 
 *    ���� �Ǿ������� �ұ��ϰ� OMC�� ����Ǵ� ������ ���� �ѹ��� ���� �ɼ��� �ִ�.
 *    �� ��� ���� Ÿ�Ӱ� ���� Ÿ���� �����Ͽ��� �Ѵ�. 
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

	/*** MESSAGE FILE�� ��ȭ�� ���� ��� **************************************/
	if (stStat.st_size == gdMsgOldSize) {
		/*fprintf (stderr, "Don't change\n");*/ return 1;
	}

	/*** MESSAGE FILE�� ��ȭ�� ������ ��� ************************************/
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
				 *    - loc_sadb->sysSts �� fan/power ���� �Ҵ� */
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
	/*** MESSAGE FILE�� ��ȭ�� �����Ǿ���, ������ ���� ����� ��� ***********
	 * log lotate �� ���� ���� ������ ó���ϴ� ��ƾ�̸�, ���� ������ ������
	 * "if (stStat.st_size > gdMsgOldSize)" �� �κп��� ó�� �ϵ��� ��.
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
	
		// lotation ���� message file�� ������ �� ������ ���ο� ������ �б� ���� ���� ��ġ �ʱ�ȭ
		if (stStatB.st_size <= gdMsgOldSize) {
			gdMsgOldSize = 0;
		}
	}
	return 1;
}

/*******************************************************************************
 ���ڿ��κ��� token������ �ܾ parsing�Ͽ� ��� WORD�� ���Ͽ�
 POWER�� H/W INDEX�� ���¸� üũ�ϴ� �Լ�.
 * szMsgBuf : messages file�� ���� line������ �о� ���� string
 * dIndex   : ���ڿ��κ��� POWER�� INDEX�� ã�� ȣ��η� �ѱ��.
 * Return   : �ش� POWER�� ����(0: NONE , 1: CRITICAL, 2: NOMAL)�� �����Ѵ�.
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
 ���ڿ��κ��� token������ �ܾ parsing�Ͽ� ��� WORD�� ���Ͽ�
 FAN�� H/W INDEX�� ���¸� üũ�ϴ� �Լ�.
 * szMsgBuf : messages file�� ���� line������ �о� ���� string
 * dIndex   : ���ڿ��κ��� POWER�� INDEX�� ã�� ȣ��η� �ѱ��.
 * Return   : �ش� POWER�� ����(0: NONE , 1: CRITICAL, 2: NOMAL)�� �����Ѵ�.
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
 LF(\n) �� ������ ������ string�� ���̸� ���ϴ� �Լ�.
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
