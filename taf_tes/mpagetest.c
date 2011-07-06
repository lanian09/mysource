#include<stdio.h>
#include<stdlib.h>
#include<linux/ipc.h>
#include<linux/msg.h>
#include<errno.h>
#include<string.h>


#define MAX_MIN_SIZE        16         /* Calling Station ID */
#define DEF_IMSI_LEN   MAX_MIN_SIZE
#define MAX_MODELINFO_LEN                      16
#define MAX_BROWSERINFO_LEN            32
#define MAX_URL_LEN                 64
#define MAX_URL_SIZE                MAX_URL_LEN
#define MAX_HASH_SIZE                                  MAX_HASHCODE_LEN
#define MAX_CPNAME_LEN                         16
#define MAX_SVCNAME_LEN                        32
#define MAX_NAME_SIZE  64
#define MAX_PROTO_LEN   16
#define MAX_HASHCODE_LEN 17
#define MAX_HOSTNAME_LEN 40
#define MAX_EXTRA_SIZE 32
#define MAX_MSGBODY_SIZE 6144

typedef unsigned char   UCHAR;
typedef short           SHORT;
typedef unsigned short  USHORT;
typedef int             INT;
typedef unsigned int    UINT;
typedef long long       INT64;
typedef long            LONG;
typedef unsigned long   ULONG;

typedef unsigned char   *PUCHAR;
typedef unsigned int    DWORD;
typedef unsigned short  WORD;
typedef int BOOL;
typedef char            TCHAR;
typedef char            *PTCHAR;
typedef unsigned char   BYTE;

typedef struct _st_MsgQSub
{
    USHORT  usType;         /* 5000 : SYS, 1000 : SVC */
    UCHAR   usSvcID;        /* SERVICE ID */
    UCHAR   usMsgID;        /* MESSAGE ID */
} st_MsgQSub, *pst_MsgQSub;


typedef struct _st_MsgQ
{
    long int    llMType;                    /* Message Type */
    int         uiReserved;

    INT64       llNID;                      /* Message Unique ID */

    UCHAR       ucNTAFID;                   /* IPAF PAIR ID if ucNaType = 2, Used szIPAFID[1] */
    UCHAR       ucProID;                    /* Process ID : Example = SEQ_PROC_SESSIF */
    UCHAR       ucNTAMID;                   /* IPAF PAIR ID if ucNaType = 2, Used szIPAFID[1] */
    char        szReserved[5];              /* Call Reference Number */

    INT64       llIndex;                    /* Local DB Index Number */

    INT         dMsgQID;                    /* Source Queue ID */
    USHORT      usBodyLen;                  /* Receive Message Body Length */
    USHORT      usRetCode;                  /* RetCode */

    UCHAR       szMIN[MAX_MIN_SIZE];        /* MIN Number : 16*/
    UCHAR       szExtra[MAX_EXTRA_SIZE];    /* Shared Information : 32*/
    UCHAR       szBody[MAX_MSGBODY_SIZE];   /* Packet Message : 6144*/

} st_MsgQ, *pst_MsgQ;

#define DEF_MSGHEAD_LEN (sizeof(st_MsgQ) - MAX_MSGBODY_SIZE)
#define DEF_SYS 5000
#define SID_LOG 50
#define MID_LOG_PAGE 53

typedef struct _st_Page_Log_Insert {
    UINT    uCliIP;
    int     dFirstTcpSynTime;
    int     dFirstTcpSynMTime;
    int     dLastTcpSynTime;
    int     dLastTcpSynMTime;
   
    USHORT  usSvcGroup;
    USHORT  usSvcCode;
    USHORT  usPageID;
    UINT    uiCreateTime;
    UINT    uiCreateMTime;

    UCHAR   szMIN[MAX_MIN_SIZE+1];
    USHORT  usNSAPI;
    USHORT  usSvcOpt;
    USHORT  usSACInfo;
    USHORT  usMSC;

    USHORT  usSvcArea;
    USHORT  usRNCNum;
    USHORT  usNodeBNum;
    USHORT  usSecNum;
    UINT    uiNodeBCode;    /* added by uamyd0626 2006.07.30 */

    USHORT  usWSType;
    USHORT  usSubSysNo;
	USHORT  usPI_SvcOption;
    UINT    uNas_IP;
    char    szPI_MIN[MAX_MIN_SIZE];

    char    szModel[MAX_MODELINFO_LEN];
    char    szBrowserInfo[MAX_BROWSERINFO_LEN];
    USHORT  usFirstTransID;
    USHORT  usPredictReqCnt;
    USHORT  usHttpTrial;

    USHORT  usExecuteReqCnt;
    USHORT  usMethod;
    char    szAbsPath[MAX_URL_LEN];
    char    szHashKey[MAX_HASH_SIZE];
    int     dMenuStartReqTime;

    int     dMenuStartReqMTime;
    int     dLastHttpReqTime;
    int     dLastHttpReqMTime;
    int     dLastMNAckTime;
    int     dLastMNAckMTime;

    UINT    uTotRedirectDelayTime;
    UINT    uTotT1Time;
    UINT    uTotT2Time;
    UINT    uTotT3Time;
    UINT    uTotT4Time;

    UINT    uTotT5Time;
    UINT    uTotT6Time;
    UINT    uTotT6Text;
    UINT    uTotT6Image;
    UINT    uTotT6Sound;

	UINT    uUpDataSize;
    UINT    uDownDataSize;
    UINT    uUpBodySize;
    UINT    uDownBodySize;
    UINT    uTotUpPackCnt;

    UINT    uTotDownPackCnt;
    UINT    uFirstURLSize;
    UINT    uTotURLSize;
    USHORT  usMaxURLSize;
    UINT    uTotRetransUpPackCnt;

    UINT    uTotRetransDownPackCnt;
    USHORT  usErrorCode;
    USHORT  usLastUserError;
    USHORT  usContTextCnt;
    USHORT  usContImageCnt;

    USHORT  usContSoundCnt;
    USHORT  usContUndefCnt;
    USHORT  usLastContFlag;
    USHORT  usTotRedirectCnt;
    USHORT  usMaxContRedirectCnt;

    char    szCPName[MAX_CPNAME_LEN+2];
    char    szSvcName[MAX_SVCNAME_LEN+2];
    char    szMenuCode[6];
    char    szMenuName[MAX_NAME_SIZE*4];
    USHORT  usMenuType;

    char    szHostName[MAX_HOSTNAME_LEN+2];
    UINT    uMenuHtmlSize;
    UINT    uHtmlParseTime;
    int     dMenuFinishTime;
	int     dMenuFinishMTime;

    USHORT  usSvcType;
    USHORT  usBrewCmd;
    USHORT  usItemType;
    UINT    uItemID;
    char    szAppVer[MAX_PROTO_LEN+2];

    UINT    uContentUpSize;
    UINT    uContentDownSize;
    UINT    uContentUpTime;
    UINT    uContentDownTime;
    USHORT  usScenario;

} st_Page_Log_Insert, *pst_Page_Log_Insert;

void Print_PAGE(st_Page_Log_Insert *Data)
{
    printf( "[CLIENT IP                ] : [%u]\n", Data->uCliIP );
    printf( "[FIRST TCP SYN TIME       ] : [%d]\n", Data->dFirstTcpSynTime );
    printf( "[FIRST TCP SYN MTIME      ] : [%d]\n", Data->dFirstTcpSynMTime );
    printf( "[LAST TCP SYN TIME        ] : [%d]\n", Data->dLastTcpSynTime );
    printf( "[LAST TCP SYN MTIME       ] : [%d]\n", Data->dLastTcpSynMTime );

    printf( "[MIN                      ] : [%s]\n", Data->szMIN );
    printf( "[MODEL                    ] : [%s]\n", Data->szModel );
    printf( "[BROWSER INFO             ] : [%s]\n", Data->szBrowserInfo );

    printf( "[METHOD                   ] : [%hu]\n", Data->usMethod );
    printf( "[ABS PATH                 ] : [%s]\n", Data->szAbsPath );
    printf( "[HASH KEY                 ] : [%s]\n", Data->szHashKey );

    printf( "[ERROR CODE               ] : [%hu]\n", Data->usErrorCode );
    printf( "[LAST USER ERROR          ] : [%hu]\n", Data->usLastUserError );
#if 0
    printf( "[SVC GROUP                ] : [%hu]\n", Data->usSvcGroup );
    printf( "[SVC CODE                 ] : [%hu]\n", Data->usSvcCode );
    printf( "[PAGE ID                  ] : [%hu]\n", Data->usPageID );
    printf( "[SVC OPTION               ] : [%hu]\n", Data->usPI_SvcOption );
    printf( "[MAS_IP                   ] : [%u]\n", Data->uNas_IP );
    printf( "[FIRST TRANS ID           ] : [%hu]\n", Data->usFirstTransID );
    printf( "[PREDICT REQUEST COUNT    ] : [%hu]\n", Data->usPredictReqCnt );
    printf( "[HTTP TRIAL               ] : [%hu]\n", Data->usHttpTrial );
    printf( "[EXECUTE REQUEST COUNT    ] : [%hu]\n", Data->usExecuteReqCnt );
    printf( "[MENU START REQUEST TIME  ] : [%d]\n", Data->dMenuStartReqTime );
    printf( "[MENU START REQUEST MTIME ] : [%d]\n", Data->dMenuStartReqMTime );
    printf( "[LAST HTTP REQUEST TIME   ] : [%d]\n", Data->dLastHttpReqTime );
    printf( "[LAST HTTP REQUEST MTIME  ] : [%d]\n", Data->dLastHttpReqMTime );
    printf( "[LAST MNACK TIME          ] : [%d]\n", Data->dLastMNAckTime );
    printf( "[LAST MNACK MTIME         ] : [%d]\n", Data->dLastMNAckMTime );
    printf( "[TOTREDIRECTDELAYTIME     ] : [%u]\n", Data->uTotRedirectDelayTime );
    printf( "[TOT T1 TIME              ] : [%u]\n", Data->uTotT1Time );
    printf( "[TOT T2 TIME              ] : [%u]\n", Data->uTotT2Time );
    printf( "[TOT T3 TIME              ] : [%u]\n", Data->uTotT3Time );
    printf( "[TOT T4 TIME              ] : [%u]\n", Data->uTotT4Time );
    printf( "[TOT T5 TIME              ] : [%u]\n", Data->uTotT5Time );
    printf( "[TOT T6 TIME              ] : [%u]\n", Data->uTotT6Time );
    printf( "[TOT T6 TEXT              ] : [%u]\n", Data->uTotT6Text );
    printf( "[TOT T6 IMAGE             ] : [%u]\n", Data->uTotT6Image );
    printf( "[TOT T6 SOUND             ] : [%u]\n", Data->uTotT6Sound );
    printf( "[UP DATA SIZE             ] : [%u]\n", Data->uUpDataSize );
    printf( "[DOWN DATA SIZE           ] : [%u]\n", Data->uDownDataSize );
    printf( "[UP BODY SIZE             ] : [%u]\n", Data->uUpBodySize );
    printf( "[DOWN BODY SIZE           ] : [%u]\n", Data->uDownBodySize );
    printf( "[TOT UP PACKET COUNT      ] : [%u]\n", Data->uTotUpPackCnt );
    printf( "[TOT DOWN PACKET COUNT    ] : [%u]\n", Data->uTotDownPackCnt );
    printf( "[FIRST URL SIZE           ] : [%u]\n", Data->uFirstURLSize );
    printf( "[TOT URL SIZE             ] : [%u]\n", Data->uTotURLSize );
    printf( "[MAX URL SIZE             ] : [%hu]\n", Data->usMaxURLSize );
    printf( "[TOT RETRANS UP PACKET    ] : [%u]\n", Data->uTotRetransUpPackCnt );
    printf( "[TOT RETRANS DOWN PACKET  ] : [%u]\n", Data->uTotRetransDownPackCnt );
    printf( "[CONTEXT COUNT            ] : [%hu]\n", Data->usContTextCnt );
    printf( "[CONTIMAGE COUNT          ] : [%hu]\n", Data->usContImageCnt );
    printf( "[CONTSOUND COUNT          ] : [%hu]\n", Data->usContSoundCnt );
    printf( "[CONTUNDEF COUNT          ] : [%hu]\n", Data->usContUndefCnt );
    printf( "[LAST CONTFLAG            ] : [%hu]\n", Data->usLastContFlag );
    printf( "[TOT REDIRECT COUNT       ] : [%hu]\n", Data->usTotRedirectCnt );
    printf( "[MAX CONTREDIRECT COUNT   ] : [%hu]\n", Data->usMaxContRedirectCnt );
    printf( "[CPNAME                   ] : [%s]\n", Data->szCPName );
    printf( "[SVC NAME                 ] : [%s]\n", Data->szSvcName );
    printf( "[MENU CODE                ] : [%s]\n", Data->szMenuCode );
    printf( "[MENU NAME                ] : [%s]\n", Data->szMenuName );
    printf( "[MENU TYPE                ] : [%hu]\n", Data->usMenuType );
    printf( "[HOSt NAME                ] : [%s]\n", Data->szHostName );
    printf( "[MENU HTML SIZE           ] : [%u]\n", Data->uMenuHtmlSize );
    printf( "[HTML PARSE TIME          ] : [%u]\n", Data->uHtmlParseTime );
    printf( "[MENU FINISH TIME         ] : [%d]\n", Data->dMenuFinishTime );
    printf( "[MENU FINISH MTIME        ] : [%d]\n", Data->dMenuFinishMTime );
    printf( "[SVC TYPE                 ] : [%hu]\n", Data->usSvcType );
    printf( "[BREWCMD                  ] : [%hu]\n", Data->usBrewCmd );
    printf( "[ITEM TYPE                ] : [%hu]\n", Data->usItemType );
    printf( "[ITEM ID                  ] : [%u]\n", Data->uItemID );
    printf( "[APP VERSION              ] : [%s]\n", Data->szAppVer );
    printf( "[CONTENT UP SIZE          ] : [%u]\n", Data->uContentUpSize );
    printf( "[CONTENT DOWN SIZE        ] : [%u]\n", Data->uContentDownSize );
    printf( "[CONTENT UP TIME          ] : [%u]\n", Data->uContentUpTime );
    printf( "[CONTENT DOWN TIME        ] : [%u]\n", Data->uContentDownTime );
    printf( "[SCENARIO                 ] : [%hu]\n", Data->usScenario );
#endif
}

int send_message( char *qbuf )
{
	int     result, dMsgLen, qid;
	st_MsgQ stMsg;
	pst_MsgQSub pstSub;

	memset( &stMsg, 0x00, sizeof(stMsg));
	if( (qid = msgget(8009,IPC_CREAT | 0666 )) < 0 ){
		printf("FAIL msgget\n");
		return(-1);
	}

	pstSub = (pst_MsgQSub)&stMsg.llMType;
	pstSub->usType = DEF_SYS;
	pstSub->usSvcID = SID_LOG;
	pstSub->usMsgID = MID_LOG_PAGE;
	
	stMsg.llMType = 3;
	stMsg.ucProID = 99;
	stMsg.llIndex = 0;

	stMsg.usBodyLen = sizeof(st_Page_Log_Insert);
	stMsg.usRetCode = 0 ;

	memcpy( &stMsg.szBody, qbuf, stMsg.usBodyLen );
	dMsgLen = DEF_MSGHEAD_LEN + stMsg.usBodyLen;

	if((result = msgsnd( qid, &stMsg, dMsgLen, 0)) == -1)
	{
		printf("FAIL msgsnd\n");
		return(-1);
	}

}

int main()
{
	int i;
	
	st_Page_Log_Insert rmesg[10];
	pst_Page_Log_Insert pst;

	for( i=0;i<10;i++ ){
		pst = &rmesg[i];

		pst->uCliIP =				176724973;
		pst->dFirstTcpSynTime =                 1194365989+(i*60);
		pst->dFirstTcpSynMTime =                56471-(i*10);
		pst->dLastTcpSynTime =                  1194365989+(i*60);
		pst->dLastTcpSynMTime =                 56471-(i*10);;
		pst->usSvcGroup =                       1;
		pst->usSvcCode =                        2;
		pst->usPageID =                         42;
		pst->usPI_SvcOption =                   87;
		pst->uNas_IP = 3556145570;
		sprintf(pst->szMIN,"450081300083488");
		pst->usFirstTransID =                   26;
		pst->usPredictReqCnt =                  1;
		pst->usHttpTrial =                      1;
		pst->usExecuteReqCnt =                  0;
		pst->usMethod =                         1;
		sprintf(pst->szAbsPath,"http://cfissnd25.magicn.com/snd_ndwn/52225333_3_01026451496.ndw");
		sprintf(pst->szHashKey,"7A323FA6E8870DCB");
		sprintf(pst->szHostName,"cfissnd25.magicn.com");
		sprintf(pst->szModel,"IM_U210K"); 
		sprintf(pst->szBrowserInfo,"KUN/2.2.1");
		pst->dMenuStartReqTime =                1194366098;
		pst->dMenuStartReqMTime =               976554;
		pst->dLastHttpReqTime =                 1194366098;
		pst->dLastHttpReqMTime =                976554;
		pst->dLastMNAckTime =                   0;
		pst->dLastMNAckMTime =                  0;
		pst->uTotRedirectDelayTime =            0;
		pst->uTotT1Time =                       0;
		pst->uTotT2Time =                       75944;
		pst->uTotT3Time =                       0;
		pst->uTotT4Time =                       0;
		pst->uTotT5Time =                       0;
		pst->uTotT6Time =                       0;
		pst->uTotT6Text =                       0;
		pst->uTotT6Image =                      0;
		pst->uTotT6Sound =                      0;
		pst->uUpDataSize =                      910;
		pst->uDownDataSize =                    314229;
		pst->uUpBodySize =                      0;
		pst->uDownBodySize =                    313934;
		pst->uTotUpPackCnt =                    1;
		pst->uTotDownPackCnt =                  216;
		pst->uFirstURLSize =                    109;
		pst->uTotURLSize =                      109;
		pst->usMaxURLSize =                     109;
		pst->uTotRetransUpPackCnt =             0;
		pst->uTotRetransDownPackCnt =           17;
		pst->usErrorCode =                      200;
		pst->usLastUserError =                  5463;
		pst->usContTextCnt =                    0;
		pst->usContImageCnt =                   0;
		pst->usContSoundCnt =                   0;
		pst->usContUndefCnt =                   1;
		pst->usLastContFlag =                   0;
		pst->usTotRedirectCnt =                 0;
		pst->usMaxContRedirectCnt =             0;
		sprintf(pst->szCPName,"CP");
		sprintf(pst->szSvcName,"SVC");
		sprintf(pst->szMenuCode,"MC");
		sprintf(pst->szMenuName,"MN");
		pst->usMenuType =                       2;
		pst->uMenuHtmlSize =                    0;
		pst->uHtmlParseTime =                   0;
		pst->dMenuFinishTime =                  0;
		pst->dMenuFinishMTime =                 0;
		pst->usSvcType =                        2;
		pst->usBrewCmd =                        0;
		pst->usItemType =                       0;
		pst->uItemID =                          0;
		sprintf(pst->szAppVer,"AV");
		pst->uContentUpSize =                   0;
		pst->uContentDownSize =                 0;
		pst->uContentUpTime =                   0;
		pst->uContentDownTime =                 0;
		pst->usScenario =                       0;
		                                        
	}

	rmesg[9].usLastUserError = 45463;
	for( i=0;i<10;i++){
		printf("\n\n=== running [%d] ===\n",i);
		Print_PAGE(&rmesg[i]);
	}

	for( i=0;i<10;i++ ){
		printf("\n\n=== test snd count[%d]==\n",i);
		send_message( (char*)&rmesg[i]);
		
	}

	

	
	
	printf("1\n");

	
	return 0;
}
