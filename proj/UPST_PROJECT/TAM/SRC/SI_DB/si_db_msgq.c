/*******************************************************************************
            UPRESTO W-NTAS Project
    Author      :   JOSHUA
    Section     :   NTAM M_SIGNAL_MSGQ
    SCCS ID     :   %W%
    DATE        :   %G%

    Revision History :
        06. 06. 01      Initial

    Description:

    Copyright (C) UPRESTO 2005
*******************************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <unistd.h>

// DQMS
#include "procid.h"

// LIB
#include "loglib.h"
#include "mems.h"
#include "nifo.h"
#include "gifo.h"
#include "cifo.h"

// .
#include "si_db_msgq.h"

extern stMEMSINFO	*pMEMSINFO;
extern stCIFO		*pCIFO;

int dIsReceivedMessage(st_MsgQ *pstMsgQ)
{
	OFFSET	offset;

	offset = gifo_read(pMEMSINFO, pCIFO, SEQ_PROC_SI_DB);
	if(offset <= 0)
	{
		usleep(0);
		return -1;
	}

	pstMsgQ = (pst_MsgQ)nifo_get_value(pMEMSINFO, DEF_MSGQ_NUM, offset);
    if(pstMsgQ == NULL)
	{
        log_print(LOGN_CRI, LH"FAILED IN nifo_get_value(st_MsgQ=%d), offset=%ld", LT, DEF_MSGQ_NUM, offset);
        return -2;
    }

    return 1;
} /* end of dIsReceivedMessage */

int dSendMsg(int dProcSeq, st_MsgQ *pstMsgQ)
{
	int			dRet;
	U8			*pNODE;
	OFFSET		offset;
	pst_MsgQ	pShmMsgq;

	// NODE �Ҵ�
	pNODE = nifo_node_alloc(pMEMSINFO);
	if(pNODE == NULL)
	{
		log_print(LOGN_CRI, LH"FAILED IN nifo_node_alloc", LT);
		return -1;
	}

	// NODE�� TLV �Ҵ�
	pShmMsgq = (pst_MsgQ)nifo_tlv_alloc(pMEMSINFO, pNODE, DEF_MSGQ_NUM, DEF_MSGQ_SIZE, DEF_MEMSET_OFF);
	if(pShmMsgq == NULL)
	{
		log_print(LOGN_CRI, LH"FAILED IN nifo_tlv_alloc, NULL", LT);
		nifo_node_delete(pMEMSINFO, pNODE);
		return -2;
	}
	// �Ķ���ͷ� �Ѿ�� st_MsgQ ����ü���� ���� �Ҵ���� �޸𸮿� ����
	memcpy(pShmMsgq, pstMsgQ, sizeof(st_MsgQ));

	// GIFO ����
	offset = nifo_offset(pMEMSINFO, pNODE);
	dRet = gifo_write(pMEMSINFO, pCIFO, SEQ_PROC_SI_DB, dProcSeq, offset);
	if(dRet < 0)
	{
		log_print(LOGN_CRI, LH"FAILED IN gifo_write from=SI_DB:%d, to=%d, offset=%ld", LT,
					SEQ_PROC_SI_DB, dProcSeq, offset);
		nifo_node_delete(pMEMSINFO, pNODE);
		usleep(0);
		return -3;
	}
	else
	{
		log_print(LOGN_INFO, LH"GIFO_WRITE to PROC_SEQ=%d, dRet=%d", LT, dProcSeq, dRet);
	}

    return dRet;
}

