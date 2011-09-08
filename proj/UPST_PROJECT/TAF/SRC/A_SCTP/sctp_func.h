#ifndef __SCTP_FUNC_H__
#define __SCTP_FUNC_H__

/* SYS HEADER */
/* LIB HEADER */
#include "typedef.h"
/* PRO HEADER */
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
/* LOC HEADER */
#include "sctpstack.h"

extern UINT 	uiCrc32(UCHAR *src, int dDataLen, UINT uiCrcInit );
extern void 	InitASSODATA( PASSO_DATA pstMMDB );
extern void 	InitStackNode( pSTACK_LIST pstStackNode );

#endif /* __SCTP_FUNC_H__ */
