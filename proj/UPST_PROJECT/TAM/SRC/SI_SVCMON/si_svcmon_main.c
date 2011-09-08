/*******************************************************************************
			DQMS Project

	Author   : Jae Seung Lee
	Section  : SI_SVCMON
	SCCS ID  : @(#)si_svcmon_main.c	1.1
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

#include "memg.h"
#include "loglib.h"
#include "path.h"
#include "mems.h"
#include "cifo.h"
#include "gifo.h"
#include "nifo.h"
#include "sshmid.h"
#include "filedb.h"
#include "procid.h"
#include "msgdef.h"
#include "nsocklib.h"
#include "almstat.h"
#include "utillib.h"
#include "verlib.h"
#include "clist_memg.h"
#include "si_svcmon_sock.h"
#include "si_svcmon_msgq.h"
#include "si_svcmon_init.h"
#include "si_svcmon_func.h"

/** B.1* DEFINITION OF NEW CONSTANTS **********************/
/** C.1* DEFINITION OF NEW TYPES **************************/
/** D.1* DECLARATION OF VARIABLES *************************/
int					dMyQid;
int					JiSTOPFlag;
int					Finishflag;

int                 dSrvSfd;
int                 Numfds;

fd_set              Rfds;

time_t				tvLastCkTime;

char	vERSION[7] = "R4.0.0";

st_WNTAM            *fidb;
stMEMGINFO			*pMEMGINFO;
stMEMSINFO          *pMEMSINFO;
stCIFO              *gpCIFO;
st_Send_Info		gstSendInfo;
st_ClientInfo		stSock[MAX_RECORD];
st_SubSysInfoList	gstSubList;

/** E.1* DEFINITION OF FUNCTIONS **************************/
/** E.2* DEFINITION OF FUNCTIONS **************************/

int main(void)
{
	int				i, dRet;
	time_t			tCurTime, tLastTime;
	st_sidb_node	*pNODE, *pDATA, *pHEAD, *pNEXT;
	st_SI_DB		*pSIDB;
	st_FDInfo		stFD;
    st_MsgQ			stMsgQ;
    st_MsgQSub		*pstMsgQSub = (st_MsgQSub *)&stMsgQ.llMType;

	log_init(S_SSHM_LOG_LEVEL, getpid(), SEQ_PROC_SI_SVCMON, LOG_PATH"/SI_SVCMON", "SI_SVCMON");

	if( (dRet = Init_Fidb()) < 0)
	{
		log_print(LOGN_CRI,"F=%s:%s.%d: FAILED IN Init_FIDB() errno[%d-%s]", __FILE__, __FUNCTION__, __LINE__, errno, strerror(errno));
		exit(-2);
	}

	pMEMSINFO = nifo_init_zone((unsigned char *)"M_LOG", SEQ_PROC_M_LOG, FILE_NIFO_ZONE);
	if( pMEMSINFO == NULL ){
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN nifo_init, NULL",  __FILE__, __FUNCTION__, __LINE__);
		exit(0);
	}

	gpCIFO = gifo_init_group(FILE_CIFO_CONF, FILE_GIFO_CONF);
	if( gpCIFO == NULL ){
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN gifo_init_group. cifo=%s, gifo=%s",
				__FILE__, __FUNCTION__, __LINE__, FILE_CIFO_CONF, FILE_GIFO_CONF);
		exit(0);
	}

	if( (dRet = set_version(S_SSHM_VERSION, SEQ_PROC_SI_SVCMON, vERSION)) != 0)
		log_print( LOGN_CRI, "set_version error(ret=%d,idx=%d,ver=%s)", dRet,SEQ_PROC_MOND,vERSION);

	gstSendInfo.cnt		= 0;
	gstSendInfo.offset_Data	= 0;

	/* PROCESS INITIAL : Msg_Q */
	if( (dRet = dInitProc()) < 0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: ERROR IN dInitProc() dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
		exit(-5);
	}

	memset(&stFD, 0x00, sizeof(st_FDInfo));

	if( (dRet = dInitSockFd(&stFD, LGT_NMS_PORT_RNES)) < 0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: ERROR IN dInitSockFd() dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
		exit(-6);
	} /* end of if */

	for(i = 0; i< MAX_RECORD; i++)
	{
		stSock[i].tLastTime	= 0;
		stSock[i].dSysNo	= 0;
		stSock[i].dSfd		= 0;
		stSock[i].uiIP		= 0;
		stSock[i].dFront	= 0;
		stSock[i].dRear		= 0;
		stSock[i].dBufSize	= 0;
		stSock[i].dLastFlag	= 0;
	}
	log_print(LOGN_CRI, "F=%s:%s.%d: SI_SVCMON PROCESS INITIAL SUCCESS", __FILE__, __FUNCTION__, __LINE__);
#ifdef DNMS_CONN_DISABLE
	fidb->stNTAM.cInterlock[SI_DNMS_INTERLOCK] = NOT_EQUIP;
#else
	if(fidb->stNTAM.cInterlock[SI_DNMS_INTERLOCK] != MASK)
		fidb->stNTAM.cInterlock[SI_DNMS_INTERLOCK] = CRITICAL;
#endif

	/* PROCESS JOB START */
	tLastTime = time(NULL);
	while(JiSTOPFlag)
	{
		if( (dRet = Check_ClientEvent(stSock, &stFD)) < 0)
		{
			log_print(LOGN_CRI, "MAIN : [CAN'T SELECT LOCAL ADDRESS=%d", dRet);
			FinishProgram();
		} /* end of if */

		for(i = 0; i < 100 && JiSTOPFlag; i++)
		{
			dRet = dIsReceivedMessage(&stMsgQ);
			if(dRet < 0) {
				usleep(0);
			} else {
				log_print(LOGN_DEBUG, "RCV MSG TYPE=%u SVCID=%u MSGID=%u",
				pstMsgQSub->usType, pstMsgQSub->usSvcID, pstMsgQSub->usMsgID);

				switch(pstMsgQSub->usType)
				{
				case DEF_SYS:
					switch(pstMsgQSub->usSvcID)
					{
					case SID_STATUS:
						switch(pstMsgQSub->usMsgID)
						{
						case MID_SVC_MONITOR_INF:
							pSIDB = (st_SI_DB *)&stMsgQ.szBody[0];
							log_print(LOGN_DEBUG, "RCV MSGQ INF FILENAME=%.*s", RNES_PKT_SIZE-1, pSIDB->name);
							break;
						case MID_SVC_MONITOR:
							if((pNODE = (st_sidb_node *)cmem_alloc(pMEMGINFO)) == NULL) {
								log_print(LOGN_CRI, "F=%s:%s.%d cmem_alloc NULL CNT=%d", __FILE__, __FUNCTION__, __LINE__, gstSendInfo.cnt);
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
										log_print(LOGN_DEBUG, "RCV MSGQ FILENAME=%.*s", RNES_PKT_SIZE-1, pSIDB->name);
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
								log_print(LOGN_DEBUG, "RCV MSGQ FILENAME=%.*s", RNES_PKT_SIZE-1, pSIDB->name);
								if(gstSendInfo.offset_Data == 0) {
									gstSendInfo.offset_Data = cmem_offset(pMEMGINFO, (U8 *)pNODE);
									gstSendInfo.cnt++;
									dHandleMsgQMsg(stSock, &stFD, pSIDB);
								} else {
									cmem_link_prev(pMEMGINFO, cmem_ptr(pMEMGINFO, gstSendInfo.offset_Data), (U8 *)pNODE);
									gstSendInfo.cnt++;
								}
							}
							break;
						default:
							log_print(LOGN_CRI, "UNKNOWN SVCID TYPE=%u SVCID=%u MSGID=%u",
									pstMsgQSub->usType, pstMsgQSub->usSvcID, pstMsgQSub->usMsgID);
							break;
						}
						break;
					default:
						log_print(LOGN_CRI, "UNKNOWN SVCID TYPE=%u SVCID=%u MSGID=%u",
								pstMsgQSub->usType, pstMsgQSub->usSvcID, pstMsgQSub->usMsgID);
						break;
					}
					break;
				default:
					log_print(LOGN_CRI, "UNKNOWN TYPE TYPE=%u SVCID=%u MSGID=%u",
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
	}
	FinishProgram();

	return 0;
} /* end of main */
