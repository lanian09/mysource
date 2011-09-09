/**
	@file		mmcd_mem.c
	@author
	@version
	@date		2011-07-14
	@brief		mmcd_mem.c
*/

/**
	Include headers
*/

/* SYS HEADER */
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <unistd.h>
/* LIB HEADER */
#include "nifo.h"
#include "cifo.h"
#include "gifo.h"
#include "config.h"
#include "loglib.h"
#include "utillib.h"
/* PRO HEADER */
#include "procid.h"
#include "msgdef.h"
#include "mmcdef.h"
#include "sockio.h"
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
/* LOC HEADER */
#include "cmd_user.h"
#include "cmd_load.h"
#include "mmcd_mem.h"

/**
	Declare var.
*/
extern st_MSG_BUF		gstMsgBuf[];				/*< mmcd_main.h */
extern st_MngPkt		cli_msg;					/*< mmcd_main.h */
extern char				g_cmd_line[];				/*< mmcd_main.h */
extern char				SmsName[];					/*< mmcd_main.h */
extern LIB_TBL			lib_tbl[];					/*< mmcd_main.h */
extern int				myBlockCode;				/*< mmcd_main.h */
extern int				gdMyQid;					/*< mmcd_main.h */

extern stMEMSINFO		*gpRECVMEMS;				/*< mmcd_main.h */
extern stCIFO			*pCIFO;						/*< mmcd_main.h */

/**
 *	Declare func.
 */
extern int cCheckTrendCmd(char cType, char *sCmdStr);
extern void set_istr(char *str);
extern int dGetConTblIdx(int dSockfd);
extern void help_proc(char *outstr);
extern int send_text_ack(int osfd, char *sBuf, int endFlag, int dConTblIdx);
extern int send_text_form(int osfd, char *sBuf, int endFlag, int dConTblIdx);
extern int Rebuild_tbl();
extern void CheckClient(int dReloadFlag);
extern char *cmd_get(In_Arg in_para[], char *outstr, mml_msg *mml, COM_TBL **cmd_ret);
extern char *ComposeHead();
extern int SetTimer(int esfd, mml_msg *snd_msg, COM_TBL *cmdptr, In_Arg in_para[]);
extern int Exe_Builtin(mml_msg *ml, int sockfd, In_Arg  in_para[]);

/**
 *	Implement func.
 */
/*******************************************************************************
 MERGE RECEIVE BUFFER
*******************************************************************************/
int dMergeBuffer( int dIdx, int dSfd, char* szTcpRmsg, int dRecvLen )
{
    long long llMagicNo = MAGIC_NUMBER;
    int     dPktSize;
    int     dRet;
    st_MngPkt    stRpkt;

    if( (dRecvLen + gstMsgBuf[dIdx].dWidx) < MAX_MNG_PKT_BUFSIZE )
    {
        memcpy( &gstMsgBuf[dIdx].szBuf[gstMsgBuf[dIdx].dWidx], &szTcpRmsg[0], dRecvLen );

        gstMsgBuf[dIdx].dWidx += dRecvLen;

		/*** COMPARE MAGIC NUMBER LENGTH ***/
        if( gstMsgBuf[dIdx].dWidx < 8 )
            return 0;

        if( *(long long*)&gstMsgBuf[dIdx].szBuf[0] == llMagicNo )
        {
            if( gstMsgBuf[dIdx].dWidx > MNG_PKT_HEAD_SIZE )
            {
				memset( &stRpkt, 0x00, sizeof(st_MngPkt) );

                stRpkt.head = *(pst_MngHead)&gstMsgBuf[dIdx].szBuf[0];

				/*** ADDED 2003.6.13 ***/
				if( MAX_MNGPKT_BODY_SIZE < stRpkt.head.usBodyLen )
					return -1;

                dPktSize = stRpkt.head.usBodyLen + MNG_PKT_HEAD_SIZE;

                if( dPktSize <= gstMsgBuf[dIdx].dWidx )
                {
                    memcpy( &stRpkt.data[0], &gstMsgBuf[dIdx].szBuf[MNG_PKT_HEAD_SIZE], stRpkt.head.usBodyLen );

                    /*** OPERATE MESSAGE ***/
					log_print(LOGN_DEBUG,
						"RECVPKT : SFD[%d] BodyLen[%d] SYSTYPE[%d] SYSNO[%d] SVCID[%x] MSGID[%x] MAGICNUMBER[%llx]\n",
						dSfd,
						stRpkt.head.usBodyLen, stRpkt.head.ucNTAMID, stRpkt.head.ucSysNo, stRpkt.head.ucSvcID,
						stRpkt.head.ucMsgID, stRpkt.head.llMagicNumber );

					memcpy( &cli_msg, &stRpkt, sizeof(stRpkt) );
                    dRet = dRcvPkt_Handle( dIdx, dSfd, stRpkt );

					gstMsgBuf[dIdx].dWidx -= dPktSize ;

					if( dRet < 0 )
						return -1;

                    if( gstMsgBuf[dIdx].dWidx > 0 )
                    {
                        memcpy( &gstMsgBuf[dIdx].szBuf[0], &gstMsgBuf[dIdx].szBuf[dPktSize],
								 gstMsgBuf[dIdx].dWidx  );

                        while( gstMsgBuf[dIdx].dWidx > MNG_PKT_HEAD_SIZE )
                        {
                            if( *(long long*)&gstMsgBuf[dIdx].szBuf[0] == llMagicNo )
                            {
								memset( &stRpkt, 0x00, sizeof(st_MngPkt) );
                                stRpkt.head = *(pst_MngHead)&gstMsgBuf[dIdx].szBuf[0];

								/*** ADDED 2003.6.13 ***/
								if( MAX_MNGPKT_BODY_SIZE < stRpkt.head.usBodyLen )
									return -1;

                                dPktSize = stRpkt.head.usBodyLen;

                                if( (dPktSize+MNG_PKT_HEAD_SIZE) <= gstMsgBuf[dIdx].dWidx )
                                {
                                    memcpy( &stRpkt.data[0], &gstMsgBuf[dIdx].szBuf[MNG_PKT_HEAD_SIZE], dPktSize );
                                   	/*** OPERATE MESSAGE ***/
									log_print(LOGN_DEBUG,
										"RECVPKT loop : SFD[%d] BodyLen[%d] SYSTYPE[%d] SYSNO[%d] SVCID[%x] MSGID[%x]"
										" MAGICNUMBER[%llx]\n",
									dSfd, stRpkt.head.usBodyLen, stRpkt.head.ucNTAMID, stRpkt.head.ucSysNo,
									stRpkt.head.ucSvcID, stRpkt.head.ucMsgID, stRpkt.head.llMagicNumber );

                                    gstMsgBuf[dIdx].dWidx -= MNG_PKT_HEAD_SIZE+dPktSize;

									memcpy( &cli_msg, &stRpkt, sizeof(stRpkt) );
                                    dRet = dRcvPkt_Handle( dIdx, dSfd, stRpkt );

									if( dRet < 0 )
										return -1;

                                    if( gstMsgBuf[dIdx].dWidx > 0 )
                                    {
                                        memcpy( &gstMsgBuf[dIdx].szBuf[0],
										&gstMsgBuf[dIdx].szBuf[MNG_PKT_HEAD_SIZE+dPktSize], gstMsgBuf[dIdx].dWidx );
                                    }
                                }
                                else
                                {
                                    break;
                                }
                            }
                            else
                            {
                    			log_print(LOGN_DEBUG,"$$$$$$$$$$$$$$$$$ FAIL dBufinIdx[%d]\n", gstMsgBuf[dIdx].dWidx);
								return -1;
                            	break;
                            }
                        }
                    }
                    else
                    {
                        gstMsgBuf[dIdx].dWidx = 0;
                    }

                    return 1;
                }
            }
        }
        else
        {
			log_print( LOGN_CRI, "LOST FRAME MAGIC[%16lld]\n", *(long long*)&gstMsgBuf[dIdx].szBuf[0]);
            gstMsgBuf[dIdx].dWidx = 0;
            return -1;
        }
    }
    else
    {
        log_print(LOGN_CRI, "BUFFER OVERFLOW INVOKE\n");
        return -2;
    }

    return 0;
}



/********************************************************************
    Function Name : dRcvPkt_Handle

    Parameter(s):
        int         dSsfd ; (input) send sockfd
        st_MyPKT    stMyPkt ; (input) recv Packet

    Function:

    Return:
        Success : 0
        Failure :   -1 :  General Fail


    History:
        Created By TUNDRA in 02.12.11

*********************************************************************/

int dRcvPkt_Handle( int dIdx, int dSsfd, st_MngPkt stMyPkt )
{
	int				i, j, dRet, dConTblIdx, blockcode;
	char			cli_data[3584], *strp;
	st_MngPkt		smesg;
	mml_msg			*ml;
	COM_TBL			*cmdptr;
	In_Arg			in_para[32];
	st_MsgQ			stMq;
	st_MsgQSub		*st_SubQ;
	st_TrendInfo	stTrendInfo;

	if( (stMyPkt.data==NULL) || (strlen(stMyPkt.data)==0))
		return -1;

	memset(cli_data, 0x00, sizeof(cli_data));

	strcpy(&cli_data[0], &stMyPkt.data[0]);
	cli_data[strlen(stMyPkt.data)] = 0x00;

	memset(g_cmd_line, 0x00, 256);

	strcpy(&g_cmd_line[0], &stMyPkt.data[0]);
	g_cmd_line[strlen(stMyPkt.data)] = 0x00;

	if(strchr(cli_data, '=') != NULL)
	{
		j = strlen(cli_data);

		for(i = 0; i < j ; i++)
		{
			if(cli_data[i] == ',')
				cli_data[i] = ' ';
		}
	}
	log_print(LOGN_DEBUG, "[INPUT COMMAND] [cli_data] : [%s]", cli_data);

	set_istr(cli_data);
	smesg.data[0]	= 0x00;
	dConTblIdx		= dGetConTblIdx(dSsfd);

	/*******************
		HELP READY
	********************/
	if(!strncasecmp(cli_data, "help", 4) || (strchr(cli_data, '?') != NULL))
	{
		if(!strcmp(cli_msg.head.userName, "ntasnms"))
		{
			sprintf( smesg.data, "GRAMMAR CHECK COMPLETE" );
			if( (dRet = send_text_ack(dSsfd, smesg.data, DBM_SUCCESS, dConTblIdx)) < 0)
				log_print(LOGN_CRI, "[ERROR] NMS SEND ACK");
		}

		smesg.data[0] = 0x00;
		help_proc(smesg.data);
		strcat(smesg.data, "COMPLETED\n");
		dRet = send_text_form(dSsfd, (char*)&smesg.data[0], 0, dConTblIdx);

		log_print(LOGN_DEBUG, "SEND HELP RESULT!!!");

		return 0;
	}

	/***************************
		REBUILD TABLE
	*****************************/
	if( strcmp( cli_data, "reload-SO-LIB") == 0 )
	{
		if( (dRet = Rebuild_tbl()) > 0)
			strcpy(smesg.data, "\n\n  SUCCESSFULLY RELOADED SHARED OBJECT AND RESOURCES\n");
		else
		{
			sprintf(smesg.data, "\n\n  SOME ERROR[%d] OCCURRED IN LOADING... ", dRet );
			strcat(smesg.data, "CALL uPRESTO IMMEDIATELY\n");
		}

		dRet = send_text_form( dSsfd, &smesg.data[0],0, dConTblIdx );
		/*
		* CONFORM TO OMP ReloadSOLIB
		*/
		CheckClient(1);
		log_print(LOGN_DEBUG, smesg.data);

		return 0;
	}

	/***************************
		24 HOURS TREND INFO
	*****************************/
	if( strcasecmp(cli_data, "DIS-TREND-INFO") == 0 )
	{
		memset(&stTrendInfo, 0x00, sizeof(st_TrendInfo));
		if( (dRet = cCheckTrendCmd(TYPE_TREND_INFO, cli_data)) < 0)
		{
			log_print(LOGN_CRI, "F=%s:%s.%d: ERROR IN cCheckTrendCmd(TYPE_TREND_INFO[%d], %s) dRet[%d]", __FILE__, __FUNCTION__, __LINE__,
				TYPE_TREND_INFO, cli_data, dRet);
			return 0;
		}
	}
	else if( strcasecmp(cli_data, "DIS-DEFECT-INFO") == 0 )
	{
		memset(&stTrendInfo, 0x00, sizeof(st_TrendInfo));
		if( (dRet = cCheckTrendCmd(TYPE_DEFECT_INFO, cli_data)) < 0)
		{
			log_print(LOGN_CRI, "F=%s:%s.%d: ERROR IN cCheckTrendCmd(TYPE_DEFECT_INFO[%d], %s) dRet[%d]", __FILE__, __FUNCTION__, __LINE__,
				TYPE_DEFECT_INFO, cli_data, dRet);
			return 0;
		}
	}

	ml = (mml_msg*)&stMq.szBody[0];
	/*********************************
	 INPUT COMMAND PARSHING
	**********************************/
	if( (strp = cmd_get(in_para, cli_data, ml, &cmdptr)) != NULL)
	{
		if( (strcasecmp(strp, "INPUT FORMAT ERROR")==0) || (strcasecmp(strp, "UNKNOWN COMMAND")==0))
			sprintf(smesg.data, "\n%s %s  RESULT = FAIL\n  REASON = %s\nCOMPLETED\n", SmsName, (char*)ComposeHead(), strp);
		else
			sprintf(smesg.data, "\n%s %sM%04d  %s\n  RESULT = FAIL\n  REASON = %s\nCOMPLETED\n",
				SmsName, (char*)ComposeHead(), lib_tbl[cmdptr->code].mcode, lib_tbl[cmdptr->code].msg_header, strp);

		/*** NMS : ACK *******************************************/
		if( !strcmp(cli_msg.head.userName, "ntasnms"))
		{
			if( (dRet = send_text_ack(dSsfd, smesg.data, DBM_FAILURE, dConTblIdx)) < 0)
			{
				log_print(LOGN_DEBUG, "[ERROR] NMS SEND ACK");
				return 0;
			}
		}
		else
		{
			dRet = send_text_form( dSsfd, smesg.data, DBM_FAILURE, dConTblIdx );
			if( dRet < 0 )
            {
                log_print(LOGN_DEBUG, "[ERROR] OMP SEND MSG");
				return 0;
            }
		}

        return 0;
	}
	else
	{
		if( !strcmp(cli_msg.head.userName, "ntasnms") )
		{
			sprintf(smesg.data, "GRAMMAR CHECK COMPLETE");
			dRet = send_text_ack(dSsfd, smesg.data, DBM_SUCCESS, dConTblIdx );
            if( dRet < 0 )
            {
                log_print(LOGN_DEBUG, "[ERROR] NMS SEND ACK");
            }
		}
	}

	log_print(LOGN_DEBUG, "COMMAND PARSING SUCCESS MsgID[%d] BlockCode[%d] msgBlockCode[%d]", ml->msg_id, lib_tbl[ml->msg_id%MAX_LIB_TABLE].block, myBlockCode);

	if( lib_tbl[ml->msg_id % MAX_LIB_TABLE].block != myBlockCode )
	{
		/*** OTHER PROCESS  SEND SET TIMER *****/
		dRet = SetTimer( dSsfd, ml, cmdptr, in_para );
		if( dRet < 0 )
		{
			if( dRet == -2 ) {
				sprintf( smesg.data,
					"\n%s %s  RESULT = FAIL\n  REASON = %s\nCOMPLETED\n",
					SmsName, (char *)ComposeHead (), "ALREADY EXECUTING COMMAND(BKUP-DATA)" );
			}
			else if( dRet == -3 ) {
				sprintf( smesg.data,
                    "\n%s %s  RESULT = FAIL\n  REASON = %s\nCOMPLETED\n",
                    SmsName, (char *)ComposeHead (), "ALREADY EXECUTING COMMAND(SRCH-MSG-LOG)" );
			}
			else if( dRet == -4 ) {
				sprintf( smesg.data,
                    "\n%s %s  RESULT = FAIL\n  REASON = %s\nCOMPLETED\n",
                    SmsName, (char *)ComposeHead (), "ALREADY EXECUTING COMMAND(SRCH-CDR)" );
			}
			else if(dRet == -5)
			{
				sprintf(smesg.data,
					"\n%s %s  RESULT = FAIL\n  REASON = %s\nCOMPLETED\n",
					SmsName, (char*)ComposeHead(), "Level 4 user can only access local NTAF-system");
			}
			else if(dRet == -6)
			{
				sprintf(smesg.data,
					"\n%s %s  RESULT = FAIL\n  REASON = %s\nCOMPLETED\n",
					SmsName, (char*)ComposeHead(), "Not available TAF NO, check a SUBSYS_INFO.dat");
			}
			else {
				sprintf( smesg.data,
        			"\n%s %s  RESULT = FAIL\n  REASON = %s\nCOMPLETED\n",
        			SmsName, (char *)ComposeHead (), "INVALID PARAMETER" );
			}

        	if( !strcmp(cli_msg.head.userName, "ntasnms") )
        	{
            	dRet = send_text_ack(dSsfd, smesg.data, DBM_FAILURE, dConTblIdx );
            	if( dRet < 0 )
            	{
                	log_print(LOGN_DEBUG, "[ERROR] NMS SEND ACK");
                	return 0;
            	}
        	}
        	else
        	{
            	dRet = send_text_form( dSsfd, smesg.data, DBM_FAILURE, dConTblIdx );
            	if( dRet < 0 )
            	{
                	log_print(LOGN_DEBUG, "[ERROR] OMP SEND MSG");
                	return 0;
            	}
        	}

			log_print(LOGN_DEBUG,"SetTimer Fail" );
			return 0;
		}

		st_SubQ = (st_MsgQSub*)&stMq.llMType;

		st_SubQ->usType = DEF_SYS;
		st_SubQ->usSvcID = SID_MML;
		st_SubQ->usMsgID = MID_MML_REQ;
		
		/* hhbaek - 함수 수정
		dRet = dMakeNID( SEQ_PROC_MMCD, &stMq.llNID );
		if( dRet < 0 )
		{
			log_print(LOGN_DEBUG,"MAKE NID FAIL" );
			return 0;
		}
		*/		
		util_makenid(SEQ_PROC_MMCD, &stMq.llNID);

		/* msgq ==> gifo */
		//stMq.dMsgQID = gdMyQid;
		stMq.dMsgQID = 0;
		stMq.ucProID = SEQ_PROC_MMCD;
		stMq.usRetCode = 0;

		stMq.usBodyLen = sizeof(mml_msg);

		stMq.llIndex = cli_msg.head.llIndex;

		blockcode = lib_tbl[ml->msg_id%MAX_LIB_TABLE].block;

		log_print(LOGN_DEBUG,
			"ml->msg_id[%u] lib_tbl[ml->msg_id modulation MAX_LIB_TABLE].block[%d]", ml->msg_id, blockcode );

		dRet = SendToPROC( &stMq, blockcode );

		log_print(LOGN_DEBUG,"[COMMAND] SendToPROC FUNCTION" );
	}
	else
	{
		/*** MMCD COMMAND HANDLE PROCESS **************************************/
		Exe_Builtin( ml, dSsfd, in_para );
		return 0;
	}

    return 1;
}

/* msgq ==> gifo */
//int dIsRcvedMessage(pst_MsgQ pstMsg)
int dIsRcvedMessage(pst_MsgQ *ppstMsg)
{
    //int 	dRet;

	/* msgq ==> gifo*/
	OFFSET	offset;

/* msgq ==> gifo */
#if 0
    dRet = msgrcv( gdMyQid, (st_MsgQ *)pstMsg, sizeof( st_MsgQ ) - sizeof(long int), 0, IPC_NOWAIT | MSG_NOERROR );
    if( dRet < 0 )
    {
        if( errno != EINTR && errno != ENOMSG )
        {
            log_print(LOGN_DEBUG,"MAJOR_ERROR - [FAIL:%d] MSGRCV MYQ : %s\n", errno, strerror(errno) );
            return -1;
        }

        return 0;
    }

	return dRet;
#endif
	
	offset = gifo_read(gpRECVMEMS, pCIFO, SEQ_PROC_MMCD);
	if(offset <= 0)
	{
		return -1;
	}
	else
	{
		*ppstMsg = (st_MsgQ *)nifo_get_value(gpRECVMEMS, DEF_MSGQ_NUM, offset);
		if(*ppstMsg == NULL)
		{
			log_print(LOGN_CRI, "FAILED IN nifo_get_value(st_MsgQ=%d), offset=%ld", DEF_MSGQ_NUM, offset);
			return -2;
		}
	}

    return 1;
}

/*******************************************************************************
 SEND COMMAND TO PROCESS
*******************************************************************************/
int SendToPROC(st_MsgQ *in_msg, int seqProcID)
{
	U8			*pNODE;
	OFFSET		offset;
	pst_MsgQ	pstMq;

	pNODE = nifo_node_alloc(gpRECVMEMS);
	if(pNODE == NULL)
	{
		log_print(LOGN_CRI, "[ERROR] nifo_node_alloc, NULL");
		return -1;
	}

	pstMq = (pst_MsgQ)nifo_tlv_alloc(gpRECVMEMS, pNODE, DEF_MSGQ_NUM, DEF_MSGQ_SIZE, DEF_MEMSET_OFF);
	if(pstMq == NULL)
	{
		log_print(LOGN_CRI, "[ERROR] nifo_tlv_alloc, NULL");
		nifo_node_delete(gpRECVMEMS, pNODE);
		usleep(0);
		return -2;
	}

	memcpy(pstMq, in_msg, DEF_MSGQ_SIZE);

	offset = nifo_offset(gpRECVMEMS, pNODE);
	if(gifo_write(gpRECVMEMS, pCIFO, SEQ_PROC_MMCD, seqProcID, offset) < 0)
	{
		// TODO gifo_write 실패시 재시도 하는 루틴 추가

		log_print(LOGN_CRI, "[ERROR] gifo_write(from=%d:MMCD, to=%d), offset=%ld", SEQ_PROC_MMCD, seqProcID, offset);
		nifo_node_delete(gpRECVMEMS, pNODE);
		usleep(0);
		return -3;
	}

	

/* msgq ==> gifo */
#if 0
    int i;
    int snd_qid;

	pst_MsgQSub pstMsgQSub;

	pstMsgQSub = (pst_MsgQSub)&in_msg->llMType;

	log_print(LOGN_DEBUG, "TYPE:[%d] SVCID:[%d] MSGID:[%d]",
		pstMsgQSub->usType, pstMsgQSub->usSvcID, pstMsgQSub->usMsgID );

	log_print(LOGN_DEBUG, "Proc Qid[%d] usBodyLen[%d] mml[%d]",
		MsgID, in_msg->usBodyLen, sizeof(mml_msg) );

    if ((snd_qid = msgget (MsgID, 0666 | IPC_CREAT )) < 0)
    {
        log_print(LOGN_CRI,"[%d] [FAIL:%d] MSGGET : %s",
            MsgID, errno, strerror(errno) );
        return -1;
    }

	i = sizeof(st_MsgQ) - MAX_MSGBODY_SIZE + in_msg->usBodyLen;

	log_print(LOGN_DEBUG, "STAT COMMAND : SEND SIZE:[%d] BODY;[%d] HEAD:[%d]",
		i, in_msg->usBodyLen, DEF_MSGHEAD_LEN );

    if( msgsnd(snd_qid, in_msg, i, IPC_NOWAIT) < 0 )
    {
        log_print(LOGN_CRI,"QID[%d] [%d] [FAIL:%d] MSGSND : %s",
                snd_qid, MsgID, errno, strerror(errno));
        return -1;
    }
    else
    {
    	log_print(LOGN_DEBUG,"[MMCD]->[%d] Qid=[%d] Size=[%d]",
        MsgID, snd_qid, i);
        return 1;
    }
#endif

    return 1;
}
