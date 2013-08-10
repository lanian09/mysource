/*******************************************************************************
			DQMS Project

	Author   : Jae Seung Lee
	Section  : SI_SVC
	SCCS ID  : @(#)si_svc_main.c	1.1
	Date     : 01/21/05
	Revision History :
        '05. 01. 21		Initial
		'08. 01. 07		Update By LSH for review
		'08. 01. 14		Add By LSH for IUPS NTAM		

	Description :

	Copyright (c) uPRESTO 2005
*******************************************************************************/

/***** A.1 * File Include *******************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/ipc.h>
#include <unistd.h>

// DQMS
#include "procid.h"
#include "sshmid.h"
#include "path.h"
#include "msgdef.h"

// LIB
#include "loglib.h"
#include "verlib.h"
#include "memg.h"
#include "mems.h"
#include "cifo.h"
#include "nsocklib.h"

// OAM
#include "filedb.h"				/* pst_NTAM */
#include "almstat.h"			/* CRITICAL */

// .

/** B.1* DEFINITION OF NEW CONSTANTS **********************/
/** C.1* DEFINITION OF NEW TYPES **************************/
/** D.1* DECLARATION OF VARIABLES *************************/
char				szVersion[7] = "R4.0.0";

int					JiSTOPFlag;
int					Finishflag;
st_ClientInfo		stSock[MAX_RECORD];
time_t				tvLastCkTime;
st_SubSysInfoList	gstSubList;
stMEMGINFO			*pMEMGINFO;
st_Send_Info		gstSendInfo;
pst_NTAM			fidb;

// GIFO
stMEMSINFO  	 	*pMEMSINFO;
stCIFO				*pCIFO;

/** E.1* DEFINITION OF FUNCTIONS **************************/
/** E.2* DEFINITION OF FUNCTIONS **************************/
extern int dInitFidb(void);
extern int dInitProc(stMEMSINFO **pMEMSINFO, stCIFO **pCIFO);
extern int Check_ClientEvent(st_ClientInfo *stSock, st_FDInfo *stFD);
extern int dHandleMsgQMsg(st_ClientInfo *stNet, st_FDInfo *stFD, st_SI_DB *pSIDB);
extern void FinishProgram();
extern int dIsReceivedMessage(st_MsgQ *pstMsgQ);
extern int dHandleFile();
extern U8 *cmem_alloc(stMEMGINFO *pMEMGINFO);
extern void cmem_link_prev(stMEMGINFO *pMEMGINFO, U8 *pHead, U8 *pNew);


int main()
{
	int				i, dRet;
	time_t			tCurTime, tLastTime;
	st_sidb_node	*pNODE, *pDATA, *pHEAD, *pNEXT;
	st_SI_DB		*pSIDB;
	st_SI_DB		*pSUBSIDB;
    st_MsgQ         stMsgQ;
    st_MsgQSub      *pstMsgQSub = (st_MsgQSub *)&stMsgQ.llMType;

	st_FDInfo       stFD;

	// Initialize log
	dRet = log_init(S_SSHM_LOG_LEVEL, getpid(), SEQ_PROC_SI_DB, LOG_PATH"/SI_DB", "SI_DB");
	if(dRet < 0)
	{
		log_print(LOGN_WARN, LH"MAIN : Failed in Initialize LOGLIB Info [%d]", LT,  dRet);
		exit(-1);
	}

	/* FIDB */
	dRet = dInitFidb();
	if(dRet < 0)
	{
		log_print(LOGN_CRI, LH"FAILED IN Init_FIDB() [%d]", LT, dRet);
		exit(-2);	
	}
	
	// Set version
	if( (dRet = set_version(S_SSHM_VERSION, SEQ_PROC_SI_DB, szVersion)) < 0)
	{
		log_print(LOGN_WARN, LH"MAIN : Failed in Initialize Version Info [%d]", LT,  dRet);
		exit(-3);
	}
	
	gstSendInfo.cnt = 0;
	gstSendInfo.offset_Data = 0;

	/* PROCESS INITIAL : Msg_Q */
	dRet = dInitProc(&pMEMSINFO, &pCIFO);
	if(dRet < 0) {
		log_print(LOGN_CRI, LH"MAIN : FAILED IN dInitProc dRet[%d]", LT, dRet);
		exit(1);
	}

	memset(&stFD, 0x00, sizeof(st_FDInfo));

	dRet = dInitSockFd(&stFD, LGT_NMS_PORT_DB);
	if(dRet < 0) {
		log_print(LOGN_CRI, LH"MAIN : FAILED IN dInitSock dRet[%d]", LT, dRet);
		return -1;
	} /* end of if */

	for(i = 0; i< MAX_RECORD; i++)
	{
		stSock[i].tLastTime=0;
		stSock[i].dSysNo = 0;
		stSock[i].dSfd = 0;
		stSock[i].uiIP = 0;
		stSock[i].dFront = 0;
		stSock[i].dRear = 0;
		stSock[i].dBufSize = 0;
		stSock[i].dLastFlag = 0;
	} /* end of for */

	log_print(LOGN_CRI, LH"MAIN : SI_DB PROCESS INITIAL SUCCESS", LT);
	if(fidb->cInterlock[SI_DB_INTERLOCK] != MASK)
		fidb->cInterlock[SI_DB_INTERLOCK] = CRITICAL;

	/* PROCESS JOB START */
	tLastTime = time(NULL);
	while(JiSTOPFlag)
    {
		dRet = Check_ClientEvent(stSock, &stFD);
		if(dRet < 0) {
			log_print(LOGN_CRI, LH"MAIN : [CAN'T SELECT LOCAL ADDRESS=%d", LT, dRet);
            FinishProgram();
        } /* end of if */

		for(i = 0; i < 100 && JiSTOPFlag; i++) 
		{
			dRet = dIsReceivedMessage(&stMsgQ);
			if(dRet < 0) {
				usleep(0);
			} else {
				log_print(LOGN_DEBUG, LH"RCV MSG TYPE=%u SVCID=%u MSGID=%u", LT,
						pstMsgQSub->usType, pstMsgQSub->usSvcID, pstMsgQSub->usMsgID);

				switch(pstMsgQSub->usType)
				{
				case DEF_SVC:
					switch(pstMsgQSub->usSvcID)
					{
						case SID_LOG:

							pSUBSIDB = (st_SI_DB *) stMsgQ.szBody;
							if ((strstr(pSUBSIDB->filename, "FITC"))!= NULL || (strstr(pSUBSIDB->filename, "FIHT"))!=NULL || (strstr(pSUBSIDB->filename, "FNET"))!=NULL) {
								dHandleFile(pSUBSIDB->name, pSUBSIDB->date, pSUBSIDB->filename);
							} else {
								if((pNODE = (st_sidb_node *)cmem_alloc(pMEMGINFO)) == NULL) {
									log_print(LOGN_CRI, LH"cmem_alloc NULL CNT=%d", LT, gstSendInfo.cnt);
									pHEAD = (st_sidb_node *)cmem_ptr(pMEMGINFO, gstSendInfo.offset_Data);
									pDATA = pHEAD;

									if(pDATA != NULL)
									{
										pNEXT = cmem_entry(cmem_ptr(pMEMGINFO, ((st_sidb_node *)pDATA)->list.offset_next), st_sidb_node, list);
										if(pDATA == pNEXT) {
											gstSendInfo.offset_Data = 0;
										} 
										else if(pDATA == pHEAD) {
											gstSendInfo.offset_Data = cmem_offset(pMEMGINFO, (U8 *)pNEXT);
											pSIDB = &pDATA->stSIDB;
											dHandleFile(pSIDB->name, pSIDB->date, pSIDB->filename);
											memcpy(pSIDB, stMsgQ.szBody, sizeof(st_SI_DB));
											log_print(LOGN_DEBUG, LH"RCV MSGQ FILENAME=%.*s", LT, RNES_PKT_SIZE-1, pSIDB->name);
										}
										/*
										   gstSendInfo.cnt--;

										   cmem_unlink(pMEMGINFO, (U8 *)pDATA);
										   cmem_delete(pMEMGINFO, (U8 *)pDATA);
										 */
									}
									usleep(0);
								}
								else {
									pSIDB = &pNODE->stSIDB;
									memcpy(pSIDB, stMsgQ.szBody, sizeof(st_SI_DB));
									log_print(LOGN_DEBUG, LH"RCV MSGQ FILENAME=%.*s", LT, RNES_PKT_SIZE-1, pSIDB->name);
									if(gstSendInfo.offset_Data == 0) {
										gstSendInfo.offset_Data = cmem_offset(pMEMGINFO, (U8 *)pNODE);
										gstSendInfo.cnt++;
										dHandleMsgQMsg(stSock, &stFD, pSIDB);
									} else {
										cmem_link_prev(pMEMGINFO, cmem_ptr(pMEMGINFO, gstSendInfo.offset_Data), (U8 *)pNODE);
										gstSendInfo.cnt++;
									}
								}
							}
							break;
						default:
							log_print(LOGN_INFO, LH"UNKNOWN SVCID TYPE=%u SVCID=%u MSGID=%u", LT,
									pstMsgQSub->usType, pstMsgQSub->usSvcID, pstMsgQSub->usMsgID);
							break;
					}
					break;
				default:
					log_print(LOGN_INFO, LH"UNKNOWN TYPE TYPE=%u SVCID=%u MSGID=%u", LT,
							pstMsgQSub->usType, pstMsgQSub->usSvcID, pstMsgQSub->usMsgID);
					break;
				}
			}
		}

		tCurTime = time(NULL);
		if(tCurTime >= tLastTime + DEF_TIMER) {
			if(gstSendInfo.offset_Data != 0) {
        		pDATA = (st_sidb_node *)cmem_ptr(pMEMGINFO, gstSendInfo.offset_Data);;

        		if(pDATA != NULL)
        		{
					pSIDB = &pDATA->stSIDB;
					if(pSIDB->send_time + DEF_RETRANS_TIME < tCurTime) {
						dHandleMsgQMsg(stSock, &stFD, pSIDB);
					}
        		}
			}
			tLastTime = time(NULL);
		}

	} /* while-loop end */

	/* PROCESS END */
	FinishProgram();

	return 0;
} /* end of main */
