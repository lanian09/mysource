/******************************************************************************* 
        @file   sctp_func.c
 *      - A_SCTP 프로세스를 초기화 하는 함수들
 *
 *      Copyright (c) 2006~ by Upresto Inc. Korea
 *      All rights reserved
 *
 *      $Id: sctp_func.c,v 1.2 2011/09/06 02:07:44 dcham Exp $
 *
 *      @Author     $Author: dcham $
 *      @version    $Revision: 1.2 $
 *      @date       $Date: 2011/09/06 02:07:44 $
 *      @ref        sctp_func.c
 *
 *      @section    Intro(소개)
 *      - A_SCTP 메인 서비스 이외 함수들.
 *
 *      @section    Requirement
 *
*******************************************************************************/

/* INCLUDE ********************************************************************/

/* SYS HEADER */
/* LIB HEADER */
/* PRO HEADER */
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
/* LOC HEADER */

/* SYS HEADER */
/* LIB HEADER */
#include "loglib.h"
#include "utillib.h"
/* PRO HEADER */
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
/* LOC HEADER */
#include "sctp_func.h"
#include "sctpstack.h"

#define CRC32(crc,ch) (crc=(crc>>8)^crc32tab[(crc^(ch))&0xff])
/* VARIABLES ******************************************************************/
static const unsigned int crc32tab[256]={
0x00000000L, 0xF26B8303L, 0xE13B70F7L, 0x1350F3F4L,
0xC79A971FL, 0x35F1141CL, 0x26A1E7E8L, 0xD4CA64EBL,
0x8AD958CFL, 0x78B2DBCCL, 0x6BE22838L, 0x9989AB3BL,
0x4D43CFD0L, 0xBF284CD3L, 0xAC78BF27L, 0x5E133C24L,
0x105EC76FL, 0xE235446CL, 0xF165B798L, 0x030E349BL,
0xD7C45070L, 0x25AFD373L, 0x36FF2087L, 0xC494A384L,
0x9A879FA0L, 0x68EC1CA3L, 0x7BBCEF57L, 0x89D76C54L,
0x5D1D08BFL, 0xAF768BBCL, 0xBC267848L, 0x4E4DFB4BL,
0x20BD8EDEL, 0xD2D60DDDL, 0xC186FE29L, 0x33ED7D2AL,
0xE72719C1L, 0x154C9AC2L, 0x061C6936L, 0xF477EA35L,
0xAA64D611L, 0x580F5512L, 0x4B5FA6E6L, 0xB93425E5L,
0x6DFE410EL, 0x9F95C20DL, 0x8CC531F9L, 0x7EAEB2FAL,
0x30E349B1L, 0xC288CAB2L, 0xD1D83946L, 0x23B3BA45L,
0xF779DEAEL, 0x05125DADL, 0x1642AE59L, 0xE4292D5AL,
0xBA3A117EL, 0x4851927DL, 0x5B016189L, 0xA96AE28AL,
0x7DA08661L, 0x8FCB0562L, 0x9C9BF696L, 0x6EF07595L,
0x417B1DBCL, 0xB3109EBFL, 0xA0406D4BL, 0x522BEE48L,
0x86E18AA3L, 0x748A09A0L, 0x67DAFA54L, 0x95B17957L,
0xCBA24573L, 0x39C9C670L, 0x2A993584L, 0xD8F2B687L,
0x0C38D26CL, 0xFE53516FL, 0xED03A29BL, 0x1F682198L,
0x5125DAD3L, 0xA34E59D0L, 0xB01EAA24L, 0x42752927L,
0x96BF4DCCL, 0x64D4CECFL, 0x77843D3BL, 0x85EFBE38L,
0xDBFC821CL, 0x2997011FL, 0x3AC7F2EBL, 0xC8AC71E8L,
0x1C661503L, 0xEE0D9600L, 0xFD5D65F4L, 0x0F36E6F7L,
0x61C69362L, 0x93AD1061L, 0x80FDE395L, 0x72966096L,
0xA65C047DL, 0x5437877EL, 0x4767748AL, 0xB50CF789L,
0xEB1FCBADL, 0x197448AEL, 0x0A24BB5AL, 0xF84F3859L,
0x2C855CB2L, 0xDEEEDFB1L, 0xCDBE2C45L, 0x3FD5AF46L,
0x7198540DL, 0x83F3D70EL, 0x90A324FAL, 0x62C8A7F9L,
0xB602C312L, 0x44694011L, 0x5739B3E5L, 0xA55230E6L,
0xFB410CC2L, 0x092A8FC1L, 0x1A7A7C35L, 0xE811FF36L,
0x3CDB9BDDL, 0xCEB018DEL, 0xDDE0EB2AL, 0x2F8B6829L,
0x82F63B78L, 0x709DB87BL, 0x63CD4B8FL, 0x91A6C88CL,
0x456CAC67L, 0xB7072F64L, 0xA457DC90L, 0x563C5F93L,
0x082F63B7L, 0xFA44E0B4L, 0xE9141340L, 0x1B7F9043L,
0xCFB5F4A8L, 0x3DDE77ABL, 0x2E8E845FL, 0xDCE5075CL,
0x92A8FC17L, 0x60C37F14L, 0x73938CE0L, 0x81F80FE3L,
0x55326B08L, 0xA759E80BL, 0xB4091BFFL, 0x466298FCL,
0x1871A4D8L, 0xEA1A27DBL, 0xF94AD42FL, 0x0B21572CL,
0xDFEB33C7L, 0x2D80B0C4L, 0x3ED04330L, 0xCCBBC033L,
0xA24BB5A6L, 0x502036A5L, 0x4370C551L, 0xB11B4652L,
0x65D122B9L, 0x97BAA1BAL, 0x84EA524EL, 0x7681D14DL,
0x2892ED69L, 0xDAF96E6AL, 0xC9A99D9EL, 0x3BC21E9DL,
0xEF087A76L, 0x1D63F975L, 0x0E330A81L, 0xFC588982L,
0xB21572C9L, 0x407EF1CAL, 0x532E023EL, 0xA145813DL,
0x758FE5D6L, 0x87E466D5L, 0x94B49521L, 0x66DF1622L,
0x38CC2A06L, 0xCAA7A905L, 0xD9F75AF1L, 0x2B9CD9F2L,
0xFF56BD19L, 0x0D3D3E1AL, 0x1E6DCDEEL, 0xEC064EEDL,
0xC38D26C4L, 0x31E6A5C7L, 0x22B65633L, 0xD0DDD530L,
0x0417B1DBL, 0xF67C32D8L, 0xE52CC12CL, 0x1747422FL,
0x49547E0BL, 0xBB3FFD08L, 0xA86F0EFCL, 0x5A048DFFL,
0x8ECEE914L, 0x7CA56A17L, 0x6FF599E3L, 0x9D9E1AE0L,
0xD3D3E1ABL, 0x21B862A8L, 0x32E8915CL, 0xC083125FL,
0x144976B4L, 0xE622F5B7L, 0xF5720643L, 0x07198540L,
0x590AB964L, 0xAB613A67L, 0xB831C993L, 0x4A5A4A90L,
0x9E902E7BL, 0x6CFBAD78L, 0x7FAB5E8CL, 0x8DC0DD8FL,
0xE330A81AL, 0x115B2B19L, 0x020BD8EDL, 0xF0605BEEL,
0x24AA3F05L, 0xD6C1BC06L, 0xC5914FF2L, 0x37FACCF1L,
0x69E9F0D5L, 0x9B8273D6L, 0x88D28022L, 0x7AB90321L,
0xAE7367CAL, 0x5C18E4C9L, 0x4F48173DL, 0xBD23943EL,
0xF36E6F75L, 0x0105EC76L, 0x12551F82L, 0xE03E9C81L,
0x34F4F86AL, 0xC69F7B69L, 0xD5CF889DL, 0x27A40B9EL,
0x79B737BAL, 0x8BDCB4B9L, 0x988C474DL, 0x6AE7C44EL,
0xBE2DA0A5L, 0x4C4623A6L, 0x5F16D052L, 0xAD7D5351L,
};


/*******************************************************************************

*******************************************************************************/
UINT uiCrc32( UCHAR *src, int dDataLen, UINT uiCrcInit )
{
	UINT		uiCrc = ~0;
	int			dLen = uiCrcInit;

	for(dLen=0; dLen<dDataLen; dLen++ ) {
		CRC32(uiCrc, src[dLen]);
	}

	uiCrc = (UINT)util_cvtuint( (unsigned int)~uiCrc );

	return uiCrc;
}


/*******************************************************************************

*******************************************************************************/
UINT uiFinalizeCrc32( UINT uiCrc32 )
{
	UINT		uiResult;
	UCHAR		ucByte0, ucByte1, ucByte2, ucByte3;

	uiResult = ~uiCrc32;
	ucByte0 = uiResult & 0xff;
	ucByte1 = (uiResult>>8) & 0xff;
	ucByte2 = (uiResult>>16) & 0xff;
	ucByte3 = (uiResult>>24) & 0xff;

	uiResult = ((ucByte0 << 24) | (ucByte1 << 16) | (ucByte2 << 8) | ucByte3);

	return uiResult;
}


/*******************************************************************************

*******************************************************************************/
void InitASSODATA( PASSO_DATA pstMMDB )
{
	memset( &pstMMDB->stSessTime, 0x00, sizeof(struct timeval) );
	pstMMDB->uiReqCount		= 0;
	pstMMDB->uiReqFirst		= 0;
	pstMMDB->uiReqLast		= 0;

	pstMMDB->uiResCount		= 0;
	pstMMDB->uiResFirst		= 0;
	pstMMDB->uiResLast		= 0;
	
	memset( &pstMMDB->stUpdateTime, 0x00, sizeof(struct timeval) );

	pstMMDB->uiReqVerifTag	= 0;
	pstMMDB->uiResVerifTag	= 0;
	pstMMDB->uiReqInitTSN	= 0;
	pstMMDB->uiResInitTSN	= 0;

	pstMMDB->ucSetupStatus	= 0;
	pstMMDB->ucCloseStatus	= 0;
	
	pstMMDB->uiSrcIPFst		= 0;
	pstMMDB->uiSrcIPSnd		= 0;
	pstMMDB->uiDestIPFst	= 0;
	pstMMDB->uiDestIPSnd	= 0;
}


/*******************************************************************************

*******************************************************************************/
void InitStackNode( pSTACK_LIST pstStackNode )
{
	memset( &pstStackNode->stDataTime, 0x00, sizeof(struct timeval) );
	memset( &pstStackNode->stSackTime, 0x00, sizeof(struct timeval) );

	pstStackNode->uiChunkOffset		= 0;
}

/*******************************************************************************

*******************************************************************************/
UINT CVT_UINT( UINT value )
{
    union {
        UINT xValue;
        char ml[4];
    } u1, u2;

    u1.xValue = value;

    u2.ml[0] = u1.ml[3];
    u2.ml[1] = u1.ml[2];
    u2.ml[2] = u1.ml[1];
    u2.ml[3] = u1.ml[0];

    return u2.xValue;
}


/*******************************************************************************
 dIsRcvedMessage
*******************************************************************************/
#if 0
int dIsRcvedMessage(pst_SMsgQ pstMsgQ)
{
    int     dRet;

    dRet = msgrcv(gdMyQID, pstMsgQ, sizeof(st_SMsgQ), 0, IPC_NOWAIT | MSG_NOERROR);
    if(dRet < 0) {
        if(errno != EINTR && errno != ENOMSG) {
            log_print( LOGN_CRI, "[FAIL:%d] MSGRCV MYQ : [%s]", errno, strerror(errno));
            return -1;      /* Error */
        }

        return 0;   /* Do Nothing */
    }
    else {
        if(dRet != (pstMsgQ->usBodyLen + DEF_MSGHEAD_LEN - sizeof(long int)) ) {
            log_print( LOGN_CRI, "PROID[%d] MESSAGE SIZE ERROR RCV[%d]BODY[%d]HEAD[%d]",
                pstMsgQ->ucProID, dRet, pstMsgQ->usBodyLen, DEF_MSGHEAD_LEN );
            return 10;
        }
        else {
            pstMsgQ->szBody[pstMsgQ->usBodyLen] = 0x00;
            /* log_print( LOGN_INFO, "[MESSAGE RECEIVED] RCV[%d]BODY[%d]HEAD[%d]", dRet, pstMsgQ->usBodyLen, DEF_MSGHEAD_LEN ); */
        }
    }

    return dRet;        /* Good */
}
#endif

/*******************************************************************************
 dSendMsg
*******************************************************************************/
#if 0
int dSendMsg(int qid, pst_SMsgQ pstMsgQ)
{
    int     isize, dRet;

    pstMsgQ->dMsgQID = gdMyQID;

    isize = sizeof(st_SMsgQ) - MAX_MSGBODY_SIZE + pstMsgQ->usBodyLen - sizeof(long int);

    dRet = msgsnd(qid, pstMsgQ, isize, 0);
    if ( dRet < 0) {
        log_print( LOGN_CRI, "[Qid = %d] ERROR SEND : %d[%s]", qid, errno, strerror(errno));
        return -1;
    } else
        log_print( LOGN_INFO, "SEND TO MSGQ=%d", qid);

	return 1;
}
#endif

/*
* $Log: sctp_func.c,v $
* Revision 1.2  2011/09/06 02:07:44  dcham
* *** empty log message ***
*
* Revision 1.1.1.1  2011/08/29 05:56:42  dcham
* NEW OAM SYSTEM
*
* Revision 1.3  2011/08/17 07:24:32  dcham
* *** empty log message ***
*
* Revision 1.2  2011/08/05 02:38:56  uamyd
* A_SCTP modified
*
* Revision 1.1.1.1  2011/08/05 00:27:17  uamyd
* init DQMS2
*
* Revision 1.4  2011/01/11 04:09:09  uamyd
* modified
*
* Revision 1.1.1.1  2010/08/23 01:12:59  uamyd
* DQMS With TOTMON, 2nd-import
*
* Revision 1.3  2009/08/12 12:34:56  dqms
* DIAMETER 디버그
*
* Revision 1.2  2009/05/27 14:24:48  dqms
* *** empty log message ***
*
* Revision 1.1  2009/05/27 07:38:13  dqms
* *** empty log message ***
*
* Revision 1.1  2009/05/13 11:38:41  upst_cvs
* NEW
*
* Revision 1.1  2008/01/11 12:09:08  pkg
* import two-step by uamyd
*
* Revision 1.6  2007/06/01 11:06:58  doit1972
* MODIFY LOG INFO
*
* Revision 1.5  2007/05/30 06:44:08  doit1972
* ADD LOG INFO
*
* Revision 1.4  2007/05/11 08:33:24  doit1972
* MODIFY CRC32 FUNCTION
*
* Revision 1.3  2007/05/10 02:21:25  doit1972
* *** empty log message ***
*
* Revision 1.2  2007/05/04 12:31:38  doit1972
* ADD INIT FUNCTION
*
* Revision 1.1  2007/05/04 00:43:18  doit1972
* NEW FILE
*
*/
