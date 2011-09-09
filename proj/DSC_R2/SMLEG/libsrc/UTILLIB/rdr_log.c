#include <stdio.h>
#include <time.h>

FILE		*f_RDR_LOG_FILE;

int dOpenRDRLogFile()
{
	if((f_RDR_LOG_FILE = fopen(DEF_RDR_MSG_LOG, "a+")) == NULL) {
		return -1;
	}

	return 0;
}

int dWriteRDRMsg(pst_RDRInfo pstRDRInfo)
{
	fprintf(f_RDR_LOG_FILE, "[STD TIMESTAMP ] : [%d]", pstRDRInfo->tStdTimeStamp );
    fprintf(f_RDR_LOG_FILE, "[ACC SESS ID   ] : [%ld]", pstRDRInfo->llAccSessID );
    fprintf(f_RDR_LOG_FILE, "[REQ TIME      ] : [%d]", pstRDRInfo->tRequestTime );
    fprintf(f_RDR_LOG_FILE, "[RES TIME      ] : [%d]", pstRDRInfo->tResponseTime );
    fprintf(f_RDR_LOG_FILE, "[RDR SEQ NUM   ] : [%d]", pstRDRInfo->uiRDRSeqNum );
    fprintf(f_RDR_LOG_FILE, "[MDN           ] : [%s]", pstRDRInfo->szMDN );
    fprintf(f_RDR_LOG_FILE, "[TRAN COMPLETE ] : [0x%02x]", pstRDRInfo->ucTranComplete );
    fprintf(f_RDR_LOG_FILE, "[TRAN TERM FLAG] : [0x%02x]", pstRDRInfo->ucTranTermFlag );
    fprintf(f_RDR_LOG_FILE, "[HASH KEY      ] : [%s]", pstRDRInfo->szHashKey );
    fprintf(f_RDR_LOG_FILE, "[DEST IP ADDR  ] : [%s]", CVT_IPADDR(pstRDRInfo->uiDestIP) );
    fprintf(f_RDR_LOG_FILE, "[TERMINAL PORT ] : [%d]", pstRDRInfo->usTermPort );
    fprintf(f_RDR_LOG_FILE, "[DEST PORT     ] : [%d]", pstRDRInfo->usDestPort );
    fprintf(f_RDR_LOG_FILE, "[REQUEST SIZE  ] : [%d]", pstRDRInfo->uiReqSize );
    fprintf(f_RDR_LOG_FILE, "[RESPONSE SIZE ] : [%d]", pstRDRInfo->uiResSize );
    fprintf(f_RDR_LOG_FILE, "[REAL LENGTH   ] : [%d]", pstRDRInfo->uiRealLen );
    fprintf(f_RDR_LOG_FILE, "[CONTENT LENGTH] : [%d]", pstRDRInfo->uiContentLen );
    fprintf(f_RDR_LOG_FILE, "[REQ IP SIZE   ] : [%d]", pstRDRInfo->usReqIPSize );
    fprintf(f_RDR_LOG_FILE, "[RES IP SIZE   ] : [%d]", pstRDRInfo->usResIPSize );
    fprintf(f_RDR_LOG_FILE, "[METHOD TYPE   ] : [0x%02x]", pstRDRInfo->usMethodType );
    fprintf(f_RDR_LOG_FILE, "[RESULT CODE   ] : [%d]", pstRDRInfo->usResultCode );
    fprintf(f_RDR_LOG_FILE, "[URL           ] : [%s]", pstRDRInfo->szURL );
    fprintf(f_RDR_LOG_FILE, "[BILL ADD INFO ] : [%s]", pstRDRInfo->szBillAddInfo );

	return 0;
}
