#include <sys/types.h>
#include <time.h>

typedef unsigned char   UCHAR;
typedef unsigned short  USHORT;
typedef short			SHORT;
typedef int             INT;
typedef unsigned int    UINT;
typedef long long       INT64;
typedef long            LONG;
typedef unsigned long   ULONG;

#define MAX_MIN_SIZE        17         /* Calling Station ID */
#define MAX_AUTH_SIZE       16
#define MAX_MSISDN_SIZE		17
#define MAX_USERNAME_SIZE	65
#define MAX_ESN_SIZE		9
#define MAX_BSMSC_SIZE		13
#define MAX_WINSVC_SIZE		7



/* RADIUS ADR */
#define ERROR_SCP_TIMEOUT               224    // SCP Timeout
#define ERROR_SCP_FAIL                  225    // SCP Fail
#define ERROR_UNASSIGNED_PREFIX         226   // unassigned prefix
#define ERROR_RADIUS_INTERNAL_ERROR     227  // internal error
#define ERROR_UNDEFINED_PARAMETER       228 // undefined parameter
#define ERROR_INVALID_PARAMETER_VALUE   229 // invalid parameter value
#define ERROR_PROXY_TIMEOUT             230  // ProxyTimeout
#define ERROR_ACCOUNT_BLANK             231 // ������ ����
#define ERROR_MIN_MISMATCH              232  // ������ ���� ����ġ
#define ERROR_MIN_ESN_MISMATCH          233  // MIN/ESN MISMATCH
#define ERROR_SCP_NOBALANCE             234  // SCP Nobalance
#define ERROR_MANDATORY_PARAMETER_ABSENT 236 // Mandatory Parameter Absent
#define ERROR_MIN_ALEADY                237     // MIN ALEADY EXIST


typedef struct _st_Category_Container
{
    time_t          dStartTime;             /* Category Start Time ���� ��Ŷ�� - ������ �ð� */
    time_t          dEndTime;               /* Category End Time ���� ��Ŷ�� ��- ���� �ð� */
    INT             dUploadDataOctets;      /* Category Upload Data Octets �ܸ�- ���� ������ ������ ��Ŷ��*/
    INT             dDownloadDataOctets;    /* Category Download Data Octets ��- ���� �ܸ��� ������ ��Ŷ��*/
    UINT            dAirTimeCharge;         /* Air Time Charge ��� �ð��� ���� �ݾ� */
    UINT            dPacketCharge;          /* Packet Charge ��� ��Ŷ���� ���� �ݾ� ���� */
    INT             dTimeDuration;          /* Category Time Duration ( EndTime - StartTime ) */
    INT             dReserv;
} st_Category_Container, *pst_Category_Container ;



typedef struct _st_ACCInfo
{
    UCHAR           ucCode;                 /* RADIUS CODE */
    UCHAR           ucID;                   /* RADIUS IDENTIFIER */
    UCHAR           ucMSILen;               /* MSISDN Len */
    UCHAR           ucSubsCapa;             /* Subscriber Capability */
    UCHAR           ucCallSubType;          /* Calling Subscriber Type */
    UCHAR           ucWSType;               /* WSType */
    UCHAR           ucUserLen;              /* User Name Field Length */
    UCHAR           ucNASType;              /* NAS-Type */
    UCHAR           szAuthen[MAX_AUTH_SIZE];/* Authenticator */

    UCHAR           ucPPPNego;              /* PPP Negotiation Duration */
    char            szReserved[3];          /* Reserved Field */
    INT             dRetryFlag;             /* RetryFlag */

    INT64           llAcntSessID;           /* Accounting Session ID */
    INT64           llCorrelID;             /* Correlation ID */

    UINT            uiUserIP;               /* Framed-IP-Addr */
    INT             dAStatType;             /* Acct-Status-Type */
    INT             dRelIndi;               /* Release Indicator */
    INT             dNumActive;             /* Number Active */
    UINT            uiNASIP;                /* NAS IP Addr */
    INT             dSvcOption;             /* Service-Option */
    time_t          dPPPSTime;              /* PPP Start Time */
    time_t          dPktSessSTime;          /* Packet Session Start Time */
    time_t          dEventTime;             /* Event Time */
    INT             dAcctInOctes;           /* Acct-Input-Octets */
    INT             dAcctOutOctes;          /* Acct-Output-Octets */
    INT             dTotalTime;             /* Total Time Duration & Active Time */ 

    UCHAR           szMSISDN[MAX_MSISDN_SIZE];  /* MSSIDN */
    UCHAR           szMIN[MAX_MIN_SIZE];    /* Calling-Station-ID */
    UCHAR           szUserName[MAX_USERNAME_SIZE];  /* User Name */

    /* Parameter Flag : Data Exist = 1, No Data = 0 */
    UCHAR           ucNumAF;                /* Num Active Flag */

    /* PDSN Only Parameter Flag : Data Exist = 1, No Data = 0 */
    UCHAR           ucPlanF;                /* Price Plan Flag */
        /* PPS ucPricePlan, llWinCallID, szWinSVC, dPPeriod, dTPeriod */
    UCHAR           ucPCFIPF;               /* PCF IP Flag */
    UCHAR           ucPPPSLen;              /* BS/MSC Address (PPP Start) Len = 0 No Data */
    UCHAR           ucSessSLen;             /* BS/MSC Address (Session Start) Len = 0 No Data */
    UCHAR           ucSessTLen;             /* BS/MSC Address (Session Stop) Len = 0 No D ata */
    UCHAR           ucHAIPF;                /* HA IP Flag */
    UCHAR           ucFMUXF;                /* Foward Mux Flag */
    UCHAR           ucRMUXF;                /* Reverse Mux Flag */
    UCHAR           ucIPTechF;              /* IP Technology Flag */
    UCHAR           ucIPQoSF;               /* IP QoS Flag */
    UCHAR           ucAirQoSF;              /* Air QoS Flag */


    /* IWF Only Parameter Flag : Data Exist = 1, No Data = 0 */
    UCHAR           ucPPPSTimeF;            /* PPP Start Time Flag */
    UCHAR           ucUDRSeqF;

    /* PDSN Only */
    time_t          dAcctSessTime;          /* Accounting Sesssion Time */
    INT             dPPeriod;               /* IN_PACKET_PERIOD Option */
    INT             dTPeriod;               /* IN_TIME_PERIOD Option */
    UINT            uiPCFIP;                /* PCF ip address */
    UINT            uiHAIP;                 /* HA ip address */
    INT             dFMUX;                  /* Foward Mux */
    INT             dRMUX;                  /* Reverse Mux */
    INT             dIPTech;                /* IP Technology */
    INT             dIPQoS;                 /* IP QoS */
    INT             dAirQoS;                /* Air QoS */

    INT             dWinID;                 /* Win Call ID Option */
    /* For 8 Byte Alignment dANRelIndi, dANRelIndi always Setting Field In ACC END MSG */ 
	INT             dANRelIndi;         /* AN Release Indicator */


    //INT             dReserved;              /* reserved */
    UINT             uiUDRSeq;              /* reserved */

    UCHAR           ucPricePlan;            /* Price Plan */
    UCHAR           szWinSvc[MAX_WINSVC_SIZE];  /* WIN_SVC Option */
    UCHAR           szESN[MAX_ESN_SIZE];    /* ESN */
    UCHAR           szPPPStart[MAX_BSMSC_SIZE];  /* BS/MSC Address (PPP Start) */
    UCHAR           szSessStart[MAX_BSMSC_SIZE]; /* BS/MSC Address (Session Start) */
    UCHAR           szSessStop[MAX_BSMSC_SIZE]; /* BS/MSC Address (Session Stop) */

    INT             dADR;                       /* ADR Field Only Used SESSIF */

} st_ACCInfo, *pst_ACCInfo;


typedef struct _st_RADIUS_HEAD
{
    UCHAR   ucCode;
    UCHAR   ucID;
    short   usLength;
    UCHAR   szAuth[16];
} st_RADIUS_HEAD, *pst_RADIUS_HEAD;

