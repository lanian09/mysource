#ifndef __SCTP_SERV_H__
#define __SCTP_SERV_H__

/* SYS HEADER */
/* LIB HEADER */
#include "typedef.h"
#include "Analyze_Ext_Abs.h"
/* PRO HEADER */
#include "capdef.h"
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
/* LOC HEADER */

#define DEF_SCTP_DATA			0
#define DEF_SCTP_INIT			1
#define DEF_SCTP_INIT_ACK		2
#define DEF_SCTP_SACK			3
#define DEF_SCTP_HEART			4
#define DEF_SCTP_HEART_ACK		5
#define DEF_SCTP_ABORT			6
#define DEF_SCTP_SHUTDOWN		7
#define DEF_SCTP_SHUTDOWN_ACK	8
#define DEF_SCTP_ERROR			9
#define DEF_SCTP_COOKIE_ECHO	10
#define DEF_SCTP_COOKIE_ACK		11
#define DEF_SCTP_ECNE			12
#define DEF_SCTP_CWR			13
#define DEF_SCTP_SHUTDOWN_COM	14

#define DEF_CHECK_CHUNKTYPE		0xC0
#define DEF_CHUNK_SKIP1			0x80
#define DEF_CHUNK_SKIP2			0xC0

#define DEF_DIAMETER_PORT		3868

#define DEF_CHECK_PARATYPE		0xC000
#define DEF_PARA_SKIP1			0x8000
#define DEF_PARA_SKIP2			0xC000

/** DEFINE PARAMETER TYPE **/
#define DEF_PARA_HEARTINFO		1
#define	DEF_PARA_IP4			5

/** DEFINE VAIABLE TYPE SIZE **/
#define DEF_USHORT_SIZE			sizeof(USHORT)
#define DEF_UINT_SIZE			sizeof(UINT)


/** DEFINE SETUP STATUS **/
#define DEF_INIT				0x01
#define DEF_INIT_ACK			0x02
#define DEF_COOKIE_ECHO			0x04
#define DEF_COOKIE_ACK			0x08

/** DEFINE CLOSE STATUS **/
#define DEF_SHUTDOWN			0x01
#define DEF_SHUTDOWN_ACK		0x02
#define DEF_SHUTDOWN_COM		0x04

/** DEFINE CAUSE CODE **/
#define DEF_CAUSE_INVALID_STID	0x01
#define DEF_CAUSE_MISS_MEN		0x02
#define DEF_CAUSE_COOKIE_ERR	0x03
#define DEF_CAUSE_OUT_RESOURCE	0x04

#define DEF_CAUSE_UNRESOL_ADDR	0x05
#define DEF_CAUSE_UNRECO_TYPE	0x06
#define DEF_CAUSE_INVALID_MEN	0x07
#define DEF_CAUSE_UNRECO_PARA	0x08
#define DEF_CAUSE_NO_DATA		0x09
#define DEF_CAUSE_SHTTING_DN	0x0a

/** CHUNK FLAG INFORMATION **/
#define DEF_NONORDER			0x04
#define DEF_ONE_PACKET			0x03
#define DEF_FIRST_PACKET		0x02
#define DEF_LAST_PACKET			0x01
#define DEF_MID_PACKET			0x00

typedef struct _st_SCTPCOMMON
{
	UCHAR		SrcPort[2];
	UCHAR		DestPort[2];
	UCHAR		VerifiTag[4];
	UCHAR		CheckSum[4];
} st_SCTPCOMMON, *pst_SCTPCOMMON;

typedef struct _st_SCTPHeader
{
	USHORT		usSrcPort;
	USHORT		usDestPort;
	UINT		uiVerifiTag;
	UINT		uiCheckSum;
} st_SCTPHeader, *pst_SCTPHeader;

typedef struct _st_SCTPDATA
{	UCHAR		Type;
	UCHAR		Flag;
	UCHAR		ChunkLen[2];
} st_SCTPDATA, *pst_SCTPDATA;

#define DEF_SCTPHEADER_SIZE     	sizeof(st_SCTPHeader)

typedef struct _st_SCTPChunkHeader
{
	UCHAR		ucType;
	UCHAR		ucFlag;
	USHORT		usChunkLen;
} st_SCTPChunkHeader, *pst_SCTPChunkHeader;

#define DEF_SCTPCHUNKHEADER_SIZE	sizeof(st_SCTPChunkHeader)

typedef struct _st_INIT
{
	UCHAR		INITTag[4];
	UCHAR       RWND[4];
	UCHAR       OutStream[2];
	UCHAR       InStream[2];
	UCHAR       InitTSN[4];	
} st_INIT, *pst_INIT;

#define DEF_INIT_SIZE				sizeof(st_INIT)

typedef struct _st_SACK
{
	UCHAR		CumulTSN[4];
	UCHAR		RWND[4];
	UCHAR		NoGapAck[2];
	UCHAR		NoDupl[2];
} st_SACK, *pst_SACK;

#define DEF_SACK_SIZE				sizeof(st_SACK)

typedef struct _st_ERROR
{
	UCHAR		CAUSE[2];
	UCHAR		LENGTH[2];
} st_ERROR, *pst_ERROR;

#define DEF_ERROR_SIZE				sizeof(st_ERROR)

typedef struct _st_SysIPHashKey
{
	UINT	uiSrcIP;
	UINT	uiDestIP;
} st_SysIPHashKey, *pst_SysIPHashKey;

#define DEF_SYSIPKEY_SIZE			sizeof(st_SysIPHashKey)

typedef struct _st_SysIPHashData
{
	UINT	uiSysInfo;
} st_SysIPHashData, *pst_SysIPHashData;

#define DEF_SYSIPDATA_SIZE			sizeof(st_SysIPHashData)

typedef struct _st_DATA
{
	UCHAR       TSN[4];
	UCHAR       STREAMID[2];
	UCHAR       STREAMSEQ[2];
	UCHAR       PROTOID[4];
} st_DATA, *pst_DATA;

#define DEF_DATA_SIZE               sizeof(st_DATA)

extern int dAnalyzeSCTP(Capture_Header_Msg *pCAPHEAD, INFO_ETH *pINFOETH, UCHAR *pNODE);
extern int 	dCheckOptionParameter( USHORT usType );
extern int 	dAnalyzeDATA( PASSO_DATA pstMMDB, UCHAR *pucData, UCHAR *pucETHData, pst_SCTPChunkHeader pstChunkHeader, INFO_ETH *pINFOETH, Capture_Header_Msg *pCAPHEAD );
extern int 	dAnalyzeSACK( PASSO_DATA pstMMDB, UCHAR *pucData, pst_SCTPChunkHeader pstChunkHeader, INFO_ETH *pINFOETH, Capture_Header_Msg *pCAPHEAD );
extern int 	dAnalyzeINIT( PASSO_DATA pstMMDB, UCHAR *pucData, pst_SCTPChunkHeader pstChunkHeader );
extern int 	dAnalyzeSHUTDOWN( PASSO_DATA pstMMDB, UCHAR *pucData, pst_SCTPChunkHeader pstChunkHeader, Capture_Header_Msg *pCAPHEAD );
extern int 	dAnalyzeHEART( PASSO_DATA pstMMDB, UCHAR *pucData, pst_SCTPChunkHeader pstChunkHeader );
extern int 	dAnalyzeCOOKIE( PASSO_DATA pstMMDB, UCHAR *pucData, pst_SCTPChunkHeader pstChunkHeader );
extern int 	dAnalyzeABORT( PASSO_DATA pstMMDB, UCHAR *pucData, pst_SCTPChunkHeader pstChunkHeader );
extern int 	dAnalyzeERROR( PASSO_DATA pstMMDB, UCHAR *pucData, pst_SCTPChunkHeader pstChunkHeader );
extern int 	dCheckErrorCode( PASSO_DATA pstMMDB, UCHAR *pucData, USHORT usChunkDataLen );
extern int 	dOperationSACK( PASSO_DATA pstMMDB, Capture_Header_Msg *pCAPHEAD, UINT uiSackTSN );
extern int 	dCloseAssociation( PASSO_DATA pstMMDB );

#endif /* __SCTP_SERV_H__ */
