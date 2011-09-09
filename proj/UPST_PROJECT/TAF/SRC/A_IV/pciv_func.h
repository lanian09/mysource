#ifndef __PCIV_FUNC_H__
#define __PCIV_FUNC_H__

typedef struct  {
	char	 		protoStr[4];
	char 	 		protoVer[4];

	uint32_t 		result;

	char			reserved[12];

	uint32_t		cmdSize;
	unsigned long	contentSize;
} __attribute__((packed)) PCIV_Header_t ;

typedef struct {
	PCIV_Header_t	header;
	unsigned char*	command;
	unsigned char*	content;
} __attribute__((packed)) PCIV_Packet_t ;

#define URL_START_IDX   8
#define PCIV_PROT_STR	0x5a41434f
#define PCIV_PROT_VER	0x30313031
#define PCIV_HDR_SIZE	36
#define PCIV_HDR_RESULT_LENGTH 4
#define PCIV_HDR_CMD_LENGTH 4
#define PCIV_HDR_CONTENT_LENGTH 8
//#define TRUE 1 dcham
//#define FALSE 0 dcham

typedef enum {
	eSTART_TIME,
	eEND_TIME,

	eCLICK_TIME,
	eNAVIGATE_TIME,
	eISCOMPLETED_TIME,
	eGETALLHEADERS_TIME,

	eGETCHUNK_REQ_START_TIME,
	eGETCHUNK_REQ_END_TIME,
	eGETCHUNK_RESP_START_TIME,
	eGETCHUNK_RESP_END_TIME,

	eLAST_API_TIME,
	eBEFORE_LAST_API_TIME

}TimeEnum_t;

typedef enum {
	/* connection */
	eCheckAndVersion = 10,
	eExit,

	eOpen,
	eIsOpened,
	eClose,

	eGetServerList,
	eGetServerInfo,
	eGetBrokerMessage,

	/* navigate */
	eNavigate = 20,

	eClick,
	eObjectClick,

	eMouseOver,
	eGoForward,
	eGoBack,

	eStop,

	eIsCompleted,
	eWaitCompleted,

	/* image */
	eCheckRectImage = 40,
	eGetRectImage,

	eGetFullImage,
	eGetObjectImage,

	eGetObjectList,
	eGetChunkTrain,
	eSetChunk,
	eGetThumbnailImage,

	eIsChangedDocument,

	eCheckRectImageEx,

	/* data */
	eGetAllheaders = 60,
	eGetAnchorList,
	
	eGetCookies,
	eGetTitle,
	
	eGetTicket,
	eInitTicket,

	eGetInputData,
	eSetInputData,
	eSetSelectData,

	eGetLoginItems,
	eSetLoginItems,

	/* display */
	eGetFontSize = 80,
	eSetFontSize,

	eRefresh,
	eGetFullSize,
	eGetActiveWindow,
	eSetActiveWindow,
	eSetPopupWindow,
	eExecuteSnapshot,

	/* url */
	eGetCurrentUrl = 100,
	eGetExViewURL,
	eGetFlvUrl,

	/* Ext */
	eThisIsJustPing,
	eSetLCDSize,

	/* version 1.2 */
	eGetText,
	eMouseDrag,
	eRegionCheck,
	eMouseOverEx,
	eClickEx,
	eOpenEx,
	eSetTicketDuration,
	eSendXCData,
	eGetChunkTrainForce,
	eEvAppWithURL,
	eEvChangeSize,
	eEvChangeURL,
	eEvAlert,
	eEvPopupWindow,
	eEvXCExecute	

}CmdEnum_t;

typedef enum{
	eNone = 0,
	eNotRequest,
	eNotPageEndTime

	/*!!!!  use CmdEnum_t  10 ~ 102 !!!!*/
}ErrEnum_t;

#define eNOT_REQUEST		1
#define eNOT_PAGE_ENDTIME	2
#define eNOT_EXACT_SEQNUM	3

/* session *****************************************/
typedef struct _st_IV_Sess_Key {
	U32		uiCliIP;
	U16		usCliPort;
	U16		usReserved;
}IV_SESS_KEY;

typedef struct _st_IV_Sess {
	LOG_IV		szLOG;

	U32			uiNextSeqDn;
	U32			uiLastSeqDn;
	U32			uiNextSeqUp;
	U32			uiLastSeqUp;

	U32			bRequest; /* 1 : request */
	U32			uiLastActionCode; 
	U32			uiCmdRemindSize ; /* if fragmented, then this is not Zero. */
	U32			bNeedGetURLfromResponse; 
	U32			uiURLIdx;

	U32			bFirstGetChunkTrain;
	U32			bGotFirstRequest;
	U32			bSequenceNumDiff;

	STIME		uiBeforeLastAPITime;
	MTIME		uiBeforeLastAPIMTime;
	STIME		uiBeforeClickTime;
	MTIME		uiBeforeClickMTime;
	STIME		uiBeforeNavigateTime;
	MTIME		uiBeforeNavigateMTime;
	S64			llRequestTime;

	U32			bIsCompleted;

	U32			uiLastResponseResult;

//	U32			uiPacketCntforTest;

}IV_SESS_DATA;

#define IV_SESS_SIZE	sizeof(IV_SESS_DATA)
#define IV_SESS_KEY_SIZE sizeof(IV_SESS_KEY)



extern S32 dGetCALLSeqID(U32 uiClientIP);
extern S32 dSvcProcess(stMEMSINFO *pMEMSINFO, stHASHOINFO *pHASH, TCP_INFO *pTCPINFO, U8 *pNode, U8* pDATA);
extern void MakeHashKey(TCP_INFO *pTCPINFO, IV_SESS_KEY *pSESSKEY);
extern void MakeHashKey(TCP_INFO *pTCPINFO, IV_SESS_KEY *pSESSKEY);
extern U32 dMakeLOGInfo(stMEMSINFO *pMEMSINFO,IV_SESS_DATA *pSESS);
extern IV_SESS_DATA* pCreateSession(stMEMSINFO* pMEMSINFO, stHASHOINFO* pHASH, IV_SESS_KEY* pSESSKEY, TCP_INFO* pTCPINFO);
extern S32 dCloseSession(stMEMSINFO* pMEMSINFO, stHASHOINFO* pHASH, IV_SESS_KEY* pSESSKEY, IV_SESS_DATA* pSESS);
extern void PageInit(IV_SESS_DATA *pSESS);
extern S32 dCheckSeqNum( IV_SESS_DATA* pSESS, TCP_INFO *pTCPINFO, unsigned int* puiNextSeq, unsigned int* puiLastSeq );
extern void AddTransDuration( TCP_INFO* pTCPINFO, IV_SESS_DATA* pSESS );
extern U32 dProcMessage(stMEMSINFO* pMEMSINFO, stHASHOINFO* pHASH, IV_SESS_KEY* pSESSKEY, IV_SESS_DATA* pSESS, TCP_INFO* pTCPINFO, U8* pNode, U8* pINPUTDATA);
extern void MakePageLOG( IV_SESS_DATA* pSESS, TCP_INFO* pTCPINFO, stMEMSINFO* pMEMSINFO, int cmd);
extern void getArgument( PCIV_Packet_t* pPacket, int iLastIdx, int* iArgLength );
extern void SetCaptureTime( IV_SESS_DATA* pSESS, U16 usTimeType, TCP_INFO* pTCPINFO );
extern void SetTime( IV_SESS_DATA* pSESS, U16 usTimeType );
extern void PrintLOGINFO(LOG_IV* pLOG);
extern int parseCommand(PCIV_Packet_t* pPacket, CmdEnum_t* dCmd );
extern U8 *PrintRtx(U8 ucRtxType);
extern int dGetHeaderFromVersion0101(IV_SESS_DATA *pSESS, PCIV_Packet_t *pPCIV, U32 uiDataSize, U8 *pDATA);
extern int dGetHeaderFromVersion0102(IV_SESS_DATA *pSESS, PCIV_Packet_t *pPCIV, U32 uiDataSize, U8 *pDATA);
extern int dGetHeaderFromVersion0100(IV_SESS_DATA *pSESS, PCIV_Packet_t *pPCIV, U32 uiDataSize, U8 *pDATA);


#endif /* __PCIV_FUNC_H__ */
