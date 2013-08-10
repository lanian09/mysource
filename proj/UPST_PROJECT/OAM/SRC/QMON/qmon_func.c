/**A.1*  File Include *************************************/

/* SYS HEADER */
#include <unistd.h>		/* USLEEP(3) */
/* LIB HEADER */
#include "mems.h"		/* stMEMSINFO */
#include "cifo.h"		/* stCIFO */
#include "clisto.h"		/* U8, OFFSET */
#include "gifo.h"		/* gifo_read(), gifo_write() */
#include "loglib.h"
/* PRO HEADER */
#include "msgdef.h"		/* st_MsgQ */
#include "procid.h"
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
/* LOC HEADER */
#include "qmon_func.h"

/**B.1*  Definition of New Constants **********************/
/**B.2*  Definition of New Type  **************************/
/**C.1*  Declaration of Variables ( Local ) ***************/
/**C.2*  Declaration of Variables ( External ) ************/
stMEMSINFO *gpRECVMEMS;
stCIFO	  *gpCIFO;

/**D.1*  Definition of Functions  ( Local ) ***************/
/**D.2*  Definition of Functions  ( External ) ************/
int dGetNode(unsigned char **ppNODE, pst_MsgQ *ppstMsgQ)
{
	*ppNODE = NULL;
	*ppstMsgQ = NULL;

	*ppNODE = nifo_node_alloc(gpRECVMEMS);
	if( *ppNODE == NULL ){
		log_print(LOGN_WARN, LH" FAILED IN nifo_node_alloc,"EH,
				LT,ET);
		return -1;
	}

	*ppstMsgQ = (pst_MsgQ)nifo_tlv_alloc(gpRECVMEMS, *ppNODE, DEF_MSGQ_NUM, DEF_MSGQ_SIZE, DEF_MEMSET_OFF);
	if( *ppstMsgQ == NULL ){
		log_print(LOGN_WARN, LH" FAILED IN nifo_tlv_alloc, return NULL",LT);
		nifo_node_delete(gpRECVMEMS, *ppNODE);
		return -2;
	}

	return 0;
}

int dMsgrcv(pst_MsgQ *ppstMsg)
{
	OFFSET offset;

	if( (offset = gifo_read(gpRECVMEMS, gpCIFO, SEQ_PROC_QMON)) <= 0 ){
		usleep(0);
		return -1;
	}

	*ppstMsg = (pst_MsgQ)nifo_get_value(gpRECVMEMS, DEF_MSGQ_NUM, offset);
	if( *ppstMsg == NULL ){
		log_print(LOGN_CRI, LH" FAILED IN nifo_get_value(st_MsgQ=%d), offset=%ld", LT, DEF_MSGQ_NUM, offset);
		return -2;
	}
	return 0;
}

int dMsgsnd(int procID, U8 *pNODE)
{

	OFFSET offset;

	offset = nifo_offset(gpRECVMEMS, pNODE);
	
	if(gifo_write(gpRECVMEMS, gpCIFO, SEQ_PROC_QMON, procID, offset) < 0) {
        log_print(LOGN_CRI, LH" FAILED IN gifo_write(QMON:%d > TARGET:%d), offset=%ld"EH ,
                LT, SEQ_PROC_QMON, procID, offset,ET);
		nifo_node_delete(gpRECVMEMS, pNODE);
        usleep(0);
        return -1;
    }
    log_print(LOGN_INFO,LH" SND CHSMD:%d TARGET:%d, offset=%ld",
            LT, SEQ_PROC_QMON, procID, offset);
    return 0;
}
