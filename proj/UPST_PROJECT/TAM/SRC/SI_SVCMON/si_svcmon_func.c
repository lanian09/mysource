/*******************************************************************************
			DQMS Project

	Author   : Jae Seung Lee
	Section  : SI_SVCMON
	SCCS ID  : @(#)si_svcmon_func.c	1.1
	Date     : 01/21/05
	Revision History :
		'05. 01. 21		Initial
		'08. 01. 07		Update By LSH for review

	Description :

	Copyright (c) uPRESTO 2005
*******************************************************************************/

/***** A.1 * File Include *******************************/
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/msg.h>
#include "path.h"
#include "nsocklib.h"
#include "loglib.h"
#include "memg.h"
#include "clist_memg.h"
#include "si_svcmon_sock.h"
#include "si_svcmon_func.h"

/** B.1* DEFINITION OF NEW CONSTANTS **********************/
/** C.1* DEFINITION OF NEW TYPES **************************/
/** D.1* DECLARATION OF VARIABLES *************************/
st_SubSysInfoList	gstSubList;
st_Send_Info		gstSendInfo;
stMEMGINFO			*pMEMGINFO;
/** E.1* DEFINITION OF FUNCTIONS **************************/
/** E.2* DEFINITION OF FUNCTIONS **************************/

int dHandleMsgQMsg(st_ClientInfo *stNet, st_FDInfo *stFD, st_SI_DB *pSIDB)
{
	int		i, dRet;
	char	buf[BUFSIZ];

	if (dCheck_File(pSIDB) < 0) {
		return -1;
	}

	memset(buf, 0x00, RNES_PKT_SIZE);
	buf[0] = RNES_NE_FILENAME;
	memcpy(&buf[1], pSIDB->name, RNES_PKT_SIZE-1);

	pSIDB->send_time = time(NULL);
	for(i = 0; i < MAX_RECORD; i++)
	{
		if( (stNet[i].uiIP > 0) && (stNet[i].dSfd > 0))
		{
           	log_print(LOGN_DEBUG, "SEND DATA TYPE=%c FILENAME=%.*s", buf[0], RNES_PKT_SIZE-1, &buf[1]);
			if( (dRet = dSendPacket(stNet, i, stFD, buf, RNES_PKT_SIZE)) < 0)
				log_print(LOGN_CRI, "F=%s:%s.%d dSendPacket dRet=%d", __FILE__, __FUNCTION__, __LINE__, dRet);
		}
	}

	return 0;
}

int dHandleSocketMsg(st_ClientInfo *stNet, int dIdx, st_FDInfo *stFD, char *szBuf)
{
	int				dRet;
	char			buf[BUFSIZ];
	st_sidb_node	*pDATA, *pNEXT, *pHEAD;
	st_SI_DB		*pSIDB;

	log_print(LOGN_DEBUG, "RCV PKT INDEX=%d FLAG=%c", dIdx, szBuf[0]);

	switch(szBuf[0])
	{
		case RNES_NMS_HEARTBEAT:
			/* send heart-beat */
			dSendCheck(stNet, dIdx, stFD);
			break;

		case RNES_NMS_ACK:
			/* handle data */
			log_print(LOGN_DEBUG, "RCV FILE ACK [%.*s]", RNES_PKT_SIZE-1, &szBuf[1]);

			/* find data */
			pHEAD = (st_sidb_node *)cmem_ptr(pMEMGINFO, gstSendInfo.offset_Data);
			pDATA = pHEAD;

			while(pDATA != NULL)
			{
				pSIDB = &pDATA->stSIDB;
				pNEXT = cmem_entry(cmem_ptr(pMEMGINFO, ((st_sidb_node *)pDATA)->list.offset_next), st_sidb_node, list);
				if(!memcmp(pSIDB->name, &szBuf[1], RNES_PKT_SIZE-1))
				{
					if(pDATA == pNEXT) {
						gstSendInfo.offset_Data = 0;
					}
					else if(pDATA == pHEAD) {
						gstSendInfo.offset_Data = cmem_offset(pMEMGINFO, (U8 *)pNEXT);
					}
					gstSendInfo.cnt--;

					cmem_unlink(pMEMGINFO, (U8 *)pDATA);
					dHandleFile(pSIDB->name, pSIDB->date, pSIDB->filename);
					cmem_delete(pMEMGINFO, (U8 *)pDATA);
					break;
				}

				if(pHEAD == pNEXT) pNEXT = NULL;
				pDATA = pNEXT;
			}
        	pDATA = (st_sidb_node *)cmem_ptr(pMEMGINFO, gstSendInfo.offset_Data);
        	if(pDATA != NULL)
        	{
            	pSIDB = &pDATA->stSIDB;
				if (dCheck_File(pSIDB) >= 0) {
            		memset(buf, 0x00, RNES_PKT_SIZE);
            		buf[0] = RNES_NE_FILENAME;
            		memcpy(&buf[1], pSIDB->name, RNES_PKT_SIZE-1);
            		pSIDB->send_time = time(NULL);
            		log_print(LOGN_DEBUG, "SEND DATA TYPE=%c FILENAME=%.*s", buf[0], RNES_PKT_SIZE-1, &buf[1]);
            		if((dRet = dSendPacket(stNet, dIdx, stFD, buf, RNES_PKT_SIZE)) < 0) {
                		log_print(LOGN_CRI, "F=%s:%s.%d dSendPacket dRet=%d", __FILE__, __FUNCTION__, __LINE__, dRet);
            		}
				}
        	}
			break;
		default:
			log_print(LOGN_CRI, "UNKNOWN FLAG=%c", szBuf[0]);
			return -1;
	}

	return 0;
} /* end of dSendToProc */

int dHandleFile(char *name, char *date, char *filename)
{
	int		dRet;
	char	new[BUFSIZ];

	sprintf(new, "%s/%s", BACKUP_PATH, date);
	mkdir(new, 0777);

	sprintf(new, "%s/%s/%s", BACKUP_PATH, date, filename);

	log_print(LOGN_DEBUG, "MOVE FILE [%s] => [%s]", name, new);

	if( (dRet = rename(name, new)) < 0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d rename %d:%s", __FILE__, __FUNCTION__, __LINE__, errno, strerror(errno));
		return -1;
	}

	return 0;
}

int dCheck_Channel(int dSysNo, int dFlag, unsigned int uiIP)
{
#ifndef DNMS_CONN_DISABLE
	if(fidb->stNTAM.cInterlock[SI_SVCMON_INTERLOCK] != MASK)
	{
		if(dFlag == 0)
		{    /* DisConnect */
			log_print(LOGN_CRI, "CHECK CHANNEL : DEAD CHANNEL SYSNO=%d CONNECT-FLAG=%d OLD_STATUS=%d IP=%u",
				dSysNo, dFlag, fidb->stNTAM.cInterlock[SI_SVCMON_INTERLOCK], uiIP);
			fidb->stNTAM.cInterlock[SI_SVCMON_INTERLOCK] = CRITICAL;
		}
		else if(dFlag == 1)
		{
			log_print(LOGN_CRI, "CHECK CHANNEL : ALIVE CHANNEL SYSNO=%d CONNECT-FLAG=%d OLD_STATUS=%d IP=%u",
				dSysNo, dFlag, fidb->stNTAM.cInterlock[SI_SVCMON_INTERLOCK], uiIP);
			fidb->stNTAM.cInterlock[SI_SVCMON_INTERLOCK] = NORMAL;
		}
		else
		{
			log_print(LOGN_CRI, "CHECK CHANNEL : UNKOWN CHANNEL SYSNO=%d CONNECT-FLAG=%d OLD_STATUS=%d IP=%u",
				dSysNo, dFlag, fidb->stNTAM.cInterlock[SI_SVCMON_INTERLOCK], uiIP);
		}
	}
	else
	{
		log_print(LOGN_CRI, "CHECK CHANNEL : MASK CHANNEL SYSNO=%d CONNECT-FLAG=%d OLD_STATUS=%d IP=%u",
			dSysNo, dFlag, fidb->stNTAM.cInterlock[SI_SVCMON_INTERLOCK], uiIP);
	}
#endif

	return 0;
}

int dCheck_File(st_SI_DB *pSIDB)
{
	FILE *fp;
	st_sidb_node    *pHEAD, *pNEXT;

	if ((fp = fopen(pSIDB->name, "r")) == NULL)
	{
		log_print(LOGN_CRI, "FILE NOT EXISTS[%.*s]",RNES_PKT_SIZE-1, pSIDB->name);
		pHEAD= (st_sidb_node *)cmem_ptr(pMEMGINFO, gstSendInfo.offset_Data);

		pNEXT = cmem_entry(cmem_ptr(pMEMGINFO, ((st_sidb_node *)pHEAD)->list.offset_next), st_sidb_node, list);
		if(pHEAD == pNEXT) {
			gstSendInfo.offset_Data = 0;
		}
		else{
			gstSendInfo.offset_Data = cmem_offset(pMEMGINFO, (U8 *)pNEXT);
		}
		gstSendInfo.cnt--;

		cmem_unlink(pMEMGINFO, (U8 *)pHEAD);
		cmem_delete(pMEMGINFO, (U8 *)pHEAD);
		return -1;
	}
	else {
		fclose(fp);
		return 0;
	}
}
