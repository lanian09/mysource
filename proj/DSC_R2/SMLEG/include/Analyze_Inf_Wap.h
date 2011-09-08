#ifndef __ANALYZE_INF_WAP10_H__
#define __ANALYZE_INF_WAP10_H__

#include "Analyze_Ext_Abs.h"

#pragma pack(1)

// WAP Port(App Code)
#define APP_WAP_PUSH            2948
#define APP_WAP_PUSH_SEC        2949
#define APP_WAP_PUSH_OTA_HTTP   4035
#define APP_WAP_PUSH_OTA_SEC    4036
#define APP_WAP_NON_CON_SESSION 9200
#define APP_WAP_CON_SESSION     9201
#define APP_WAP_SEC_NON_CON     9202
#define APP_WAP_SEC_SESSION     9203
#define APP_WAP_VCARD           9204
#define APP_WAP_VCAL            9205
#define APP_WAP_VCARD_SEC       9206
#define APP_WAP_VCAL_SEC        9207



/*
 * Field names.
 */     
#define FN_ACCEPT               0x00
#define FN_ACCEPT_CHARSET_DEP   0x01    /* encoding version 1.1, deprecated */
#define FN_ACCEPT_ENCODING_DEP  0x02    /* encoding version 1.1, deprecated */
#define FN_ACCEPT_LANGUAGE      0x03
#define FN_ACCEPT_RANGES        0x04
#define FN_AGE                  0x05
#define FN_ALLOW                0x06
#define FN_AUTHORIZATION        0x07
#define FN_CACHE_CONTROL_DEP    0x08    /* encoding version 1.1, deprecated */
#define FN_CONNECTION           0x09
#define FN_CONTENT_BASE         0x0A
#define FN_CONTENT_ENCODING     0x0B
#define FN_CONTENT_LANGUAGE     0x0C
#define FN_CONTENT_LENGTH       0x0D
#define FN_CONTENT_LOCATION     0x0E
#define FN_CONTENT_MD5          0x0F
#define FN_CONTENT_RANGE_DEP    0x10    /* encoding version 1.1, deprecated */
#define FN_CONTENT_TYPE         0x11
#define FN_DATE                 0x12
#define FN_ETAG                 0x13
#define FN_EXPIRES              0x14
#define FN_FROM                 0x15
#define FN_HOST                 0x16
#define FN_IF_MODIFIED_SINCE    0x17
#define FN_IF_MATCH             0x18
#define FN_IF_NONE_MATCH        0x19
#define FN_IF_RANGE             0x1A
#define FN_IF_UNMODIFIED_SINCE  0x1B
#define FN_LOCATION             0x1C
#define FN_LAST_MODIFIED        0x1D
#define FN_MAX_FORWARDS         0x1E
#define FN_PRAGMA               0x1F
#define FN_PROXY_AUTHENTICATE   0x20
#define FN_PROXY_AUTHORIZATION  0x21
#define FN_PUBLIC               0x22
#define FN_RANGE                0x23
#define FN_REFERER              0x24
#define FN_RETRY_AFTER          0x25
#define FN_SERVER               0x26
#define FN_TRANSFER_ENCODING    0x27
#define FN_UPGRADE              0x28
#define FN_USER_AGENT           0x29
#define FN_VARY                 0x2A
#define FN_VIA                  0x2B
#define FN_WARNING              0x2C
#define FN_WWW_AUTHENTICATE     0x2D
#define FN_CONTENT_DISPOSITION  0x2E
#define FN_X_WAP_APPLICATION_ID 0x2F
#define FN_X_WAP_CONTENT_URI    0x30
#define FN_X_WAP_INITIATOR_URI  0x31
#define FN_ACCEPT_APPLICATION   0x32
#define FN_BEARER_INDICATION    0x33
#define FN_PUSH_FLAG            0x34
#define FN_PROFILE              0x35
#define FN_PROFILE_DIFF         0x36
#define FN_PROFILE_WARNING      0x37
#define FN_EXPECT               0x38
#define FN_TE                   0x39
#define FN_TRAILER              0x3A
#define FN_ACCEPT_CHARSET       0x3B    /* encoding version 1.3 */
#define FN_ACCEPT_ENCODING      0x3C    /* encoding version 1.3 */
#define FN_CACHE_CONTROL        0x3D    /* encoding version 1.3 */
#define FN_CONTENT_RANGE        0x3E    /* encoding version 1.3 */
#define FN_X_WAP_TOD            0x3F
#define FN_CONTENT_ID           0x40
#define FN_SET_COOKIE           0x41
#define FN_COOKIE               0x42
#define FN_ENCODING_VERSION     0x43
#define FN_PROFILE_WARNING14    0x44    /* encoding version 1.4 */
#define FN_CONTENT_DISPOSITION14 0x45   /* encoding version 1.4 */
#define FN_X_WAP_SECURITY       0x46
#define FN_CACHE_CONTROL14      0x47    /* encoding version 1.4 */
#define FN_EXPECT15             0x48    /* encoding version 1.5 */
#define FN_X_WAP_LOC_INVOCATION 0x49
#define FN_X_WAP_LOC_DELIVERY   0x4A

enum E_RET { 
    RET_IS_LONG_INT = 1,
    RET_IS_UINTVAR,
    RET_IS_STRING,
    RET_IS_SHORT_INT
};

typedef union _WTP_CONCATE_HDR {
    struct  
    {
#if LINUX
        UCHAR len:7;
        UCHAR flag:1; 
#else 
        UCHAR flag:1; 
        UCHAR len:7;
#endif

        UCHAR data[1];
    } short_len;

    struct  
    {
#if LINUX
        UCHAR len_1:7;
        UCHAR flag:1; 
#else 
        UCHAR flag:1; 
        UCHAR len_1:7;
#endif

        UCHAR len_2;
        UCHAR data[1];
    } long_len;
} WTP_CONCATE_HDR;

#define WTP_CoNCATE_HDR_SIZE	sizeof(WTP_CONCATE_HDR)


typedef struct _WTP_HDR_BYTE {
    UCHAR header1;
    union
    {
        struct
        {
            UCHAR tid[2];
            UCHAR octet4[1];
            UCHAR data[1];
        } invoke;

        struct
        {
            UCHAR tid[2];
            UCHAR data[1];
        } result;

        struct
        {
            UCHAR tid[2];
            UCHAR data[1];
        } ack;

        struct
        {
            UCHAR tid[2];
            UCHAR reason[1];
            UCHAR data[1];
        } abort;

        struct
        {   
            UCHAR tid[2];
            UCHAR seq_num[1];
            UCHAR data[1];
        } segment_invoke;
        
        struct
        {   
            UCHAR tid[2];
            UCHAR sec_num[1];
            UCHAR data[1];
        } segment_result;
        
        struct
        {   
            UCHAR tid[2];
            UCHAR packet_num[1];
            UCHAR data[1];
        } negative_ack;
        
    } header2;
} WTP_HDR_BYTE;

#define WTP_HDR_BYTE_SIZE	sizeof(WTP_HDR_BYTE_SIZE)


typedef struct _WTP_HDR_BIT {
    union
    {
        struct
        {
#if LINUX
            UCHAR rid:1;
            UCHAR ttr:1;
            UCHAR gtr:1;
            UCHAR pdu_type:4;
            UCHAR con:1;
#else
            UCHAR con:1;
            UCHAR pdu_type:4;
            UCHAR gtr:1;
            UCHAR ttr:1;
            UCHAR rid:1;
#endif
        } invoke_result;    // invoke & result

        struct
        {
#if LINUX
            UCHAR rid:1;
            UCHAR res:1;
            UCHAR tve_tok:1;
            UCHAR pdu_type:4;
            UCHAR con:1;
#else
            UCHAR con:1;
            UCHAR pdu_type:4;
            UCHAR tve_tok:1;
            UCHAR res:1;
            UCHAR rid:1;
#endif
        } ack;              // ack

        struct
        {
#if LINUX
            UCHAR abort_type:3;
            UCHAR pdu_type:4;
            UCHAR con:1;
#else
            UCHAR con:1;
            UCHAR pdu_type:4;
            UCHAR abort_type:3;
#endif
        } abort;            // abort
        
    } octet1;
    
    UCHAR tid[2];
    
    union
    {
        struct
        {
#if LINUX
            UCHAR tcl:2;
            UCHAR res:2;
            UCHAR u_p:1;
            UCHAR tid_new:1;
            UCHAR ver:2;
#else
            UCHAR ver:2;
            UCHAR tid_new:1;
            UCHAR u_p:1;
            UCHAR res:2;
            UCHAR tcl:2;
#endif

            UCHAR data[1];
        } invoke;           // invoke

        struct
        {
            UCHAR data[1];
        } result_ack;       // result & ack

        struct
        {
            UCHAR reason;
            UCHAR data[1];
        } abort;            // abort
        
        struct
        {   
            UCHAR psn;
            UCHAR data[1];
        } segment;
        
        struct
        {   
            UCHAR num;
            UCHAR data[1];
        } negative_ack;
    
    } octet4;
} WTP_HDR;

#define WTP_HDR_SIZE	sizeof(WTP_HDR)

typedef union _TPI_HDR {
    struct      
    {       
#if LINUX
        UCHAR res:2;
        UCHAR type:1;    // type = 1
        UCHAR id:4;
        UCHAR con:1;
#else
        UCHAR con:1;
        UCHAR id:4;
        UCHAR type:1;    // type = 1
        UCHAR res:2;
#endif
        
        UCHAR len;
        UCHAR data[1];
    } long_tpi;
        
    struct
    {   
#if LINUX
        UCHAR len:2;
        UCHAR type:1;    // type = 0
        UCHAR id:4;
        UCHAR con:1;
#else
        UCHAR con:1;
        UCHAR id:4;
        UCHAR type:1;    // type = 0
        UCHAR len:2;
#endif

        UCHAR data[1];
    } short_tpi;
} TPI_HDR;  

#define TPI_HDR_SIZE	sizeof(TPI_HDR_SIZE)



/* definitions of WAP information for Analyzing */
#define MAX_CONTENT_TYPE_CNT	100
typedef struct _vals_content_types
{
	UCHAR	id;
	CHAR	media_type[128];
} vals_content_type;

const vals_content_type content_type[MAX_CONTENT_TYPE_CNT] = {
    /* Well-known media types */
    { 0x00, "*/*" },
    { 0x01, "text/*" },
    { 0x02, "text/html" },
    { 0x03, "text/plain" },
    { 0x04, "text/x-hdml" },
    { 0x05, "text/x-ttml" },
    { 0x06, "text/x-vCalendar" },
    { 0x07, "text/x-vCard" },
    { 0x08, "text/vnd.wap.wml" },
    { 0x09, "text/vnd.wap.wmlscript" },
    { 0x0A, "text/vnd.wap.channel" },
    { 0x0B, "multipart/*" },
    { 0x0C, "multipart/mixed" },
    { 0x0D, "multipart/form-data" },
    { 0x0E, "multipart/byteranges" },
    { 0x0F, "multipart/alternative" },
    { 0x10, "application/*" },
    { 0x11, "application/java-vm" },
    { 0x12, "application/x-www-form-urlencoded" },
    { 0x13, "application/x-hdmlc" },
    { 0x14, "application/vnd.wap.wmlc" },
    { 0x15, "application/vnd.wap.wmlscriptc" },
    { 0x16, "application/vnd.wap.channelc" },
    { 0x17, "application/vnd.wap.uaprof" },
    { 0x18, "application/vnd.wap.wtls-ca-certificate" },
    { 0x19, "application/vnd.wap.wtls-user-certificate" },
    { 0x1A, "application/x-x509-ca-cert" },
    { 0x1B, "application/x-x509-user-cert" },
    { 0x1C, "image/*" },
    { 0x1D, "image/gif" },
    { 0x1E, "image/jpeg" },
    { 0x1F, "image/tiff" },
    { 0x20, "image/png" },
    { 0x21, "image/vnd.wap.wbmp" },
    { 0x22, "application/vnd.wap.multipart.*" },
    { 0x23, "application/vnd.wap.multipart.mixed" },
    { 0x24, "application/vnd.wap.multipart.form-data" },
    { 0x25, "application/vnd.wap.multipart.byteranges" },
    { 0x26, "application/vnd.wap.multipart.alternative" },
    { 0x27, "application/xml" },
    { 0x28, "text/xml" },
    { 0x29, "application/vnd.wap.wbxml" },
    { 0x2A, "application/x-x968-cross-cert" },
    { 0x2B, "application/x-x968-ca-cert" },
    { 0x2C, "application/x-x968-user-cert" },
    { 0x2D, "text/vnd.wap.si" },
    { 0x2E, "application/vnd.wap.sic" },
    { 0x2F, "text/vnd.wap.sl" },
    { 0x30, "application/vnd.wap.slc" },
    { 0x31, "text/vnd.wap.co" },
    { 0x32, "application/vnd.wap.coc" },
    { 0x33, "application/vnd.wap.multipart.related" },
    { 0x34, "application/vnd.wap.sia" },
    { 0x35, "text/vnd.wap.connectivity-xml" },
    { 0x36, "application/vnd.wap.connectivity-wbxml" },
    { 0x37, "application/pkcs7-mime" },
    { 0x38, "application/vnd.wap.hashed-certificate" },
    { 0x39, "application/vnd.wap.signed-certificate" },
    { 0x3A, "application/vnd.wap.cert-response" },
    { 0x3B, "application/xhtml+xml" },
    { 0x3C, "application/wml+xml" },
    { 0x3D, "text/css" },
    { 0x3E, "application/vnd.wap.mms-message" },
    { 0x3F, "application/vnd.wap.rollover-certificate" },
    { 0x40, "application/vnd.wap.locc+wbxml"},
    { 0x41, "application/vnd.wap.loc+xml"},
    { 0x42, "application/vnd.syncml.dm+wbxml"},
    { 0x43, "application/vnd.syncml.dm+xml"},
    { 0x44, "application/vnd.syncml.notification"},
    { 0x45, "application/vnd.wap.xhtml+xml"},
    { 0x46, "application/vnd.wv.csp.cir"},
    { 0x47, "application/vnd.oma.dd+xml"},
    { 0x48, "application/vnd.oma.drm.message"},
    { 0x49, "application/vnd.oma.drm.content"},
    { 0x4A, "application/vnd.oma.drm.rights+xml"},
    { 0x4B, "application/vnd.oma.drm.rights+wbxml"},
    { 0x4C, "application/vnd.wv.csp+xml"},
    { 0x4D, "application/vnd.wv.csp+wbxml"}
#if 0
    /* The following media types are registered by 3rd parties */
    { 0x0201, "application/vnd.uplanet.cachop-wbxml" },
    { 0x0202, "application/vnd.uplanet.signal" },
    { 0x0203, "application/vnd.uplanet.alert-wbxml" },
    { 0x0204, "application/vnd.uplanet.list-wbxml" },
    { 0x0205, "application/vnd.uplanet.listcmd-wbxml" },
    { 0x0206, "application/vnd.uplanet.channel-wbxml" },
    { 0x0207, "application/vnd.uplanet.provisioning-status-uri" },
    { 0x0208, "x-wap.multipart/vnd.uplanet.header-set" },
    { 0x0209, "application/vnd.uplanet.bearer-choice-wbxml" },
    { 0x020A, "application/vnd.phonecom.mmc-wbxml" },
    { 0x020B, "application/vnd.nokia.syncset+wbxml" },
    { 0x020C, "image/x-up-wpng"},
    { 0x0300, "application/iota.mmc-wbxml"},
    { 0x0301, "application/iota.mmc-xml"},
    { 0x00, NULL }
#endif
};

#define MAX_WARNING_CNT 10
typedef struct _vals_warning
{
	USHORT	id;
	CHAR	warning[128];
} vals_warning;

const vals_warning warning[MAX_WARNING_CNT] =
{
	{ 10, "Response is stale" },
	{ 11, "Revalidation failed" },
	{ 12, "Disconnected operation" },
	{ 13, "Heuristic expiration" },
	{ 99, "Miscellaneous warning" },
	{ 14, "Transformation applied" },
	{ 0,  "Unknown" }
};


#define MAX_ABORT_CNT 30
typedef struct _vals_abort_reason
{
	UCHAR id;
	CHAR	abort_reason[128];
} vals_abort_reason;

const vals_abort_reason abort_reason[MAX_ABORT_CNT] =
{
	// provider
	// The abort was generated by the WTP provider itself.
	{ 0x00, "Unknown" },			// A generic error code indicating an unexpected error
	{ 0x01, "Protocol error" },		// The received PDU could not be interpreted. The structure may be wrong
	{ 0x02, "Invalid TID" },		// Only used by the Initiator as a negative result to the TID verification
	{ 0x03, "Not implemented" },	// The transaction could not be completed since the Responder does not support class 2 transaction
	{ 0x04, "Not implemented SAR" }, // The transaction could not be completed since the responder does not support SAR
	{ 0x05, "Not implemented user" }, // The transaction could not be completed since the responder does not support user acknowledgement
	{ 0x06, "WTP version one" },   // Current version is 1.  The initiator requested a different version that is not supported.
	{ 0x07, "Capacity temporarily exceeded" },  // Due to an overload situation the transaction can not be completed
	{ 0x08, "No response" },
	{ 0x09, "Message too large" },
	{ 0x0A, "Not implemented extended SAR" },
	// user
	{ 0xE0, "Protocol error" },  			// Protocol error, illegal PDU received
	{ 0xE1, "Session disconnected" },  		// Session has been disconected
	{ 0xE2, "Session suspend" }, 			// Session has been suspended
	{ 0xE3, "Session resumed" },			// Session has been resumed
	{ 0xE4, "Congestion" },					// The peer is congested and can not process the SDU
	{ 0xE5, "Connect failed" },				// The session connect failed
	{ 0xE6, "Max recv unit size exceed" },	// The Maximum Received Unit size was exceeded
	{ 0xE7, "Max req exceed" },				// The Maximum Outstanding Requests was exceeded
	{ 0xE8, "Peer request" },				// Peer Request
	{ 0xE9, "Network error" },				// Network error
	{ 0xEA, "User request" },				// User Request
	{ 0xEB, "User refused msg" },			// User refused Push message. No specific cause, no retries
	{ 0xEC, "Push msg can't be delivered" },//Push message cannot be delivered to intended distination
	{ 0xED, "Push msg discarded" },			// Push message discarded due to resource shortage
	{ 0xEE, "Content type can't be processed" } // Content type of Push message cannot be processed
	//{ NULL, "Invalid" }
};

#define MAX_TPI_CNT 10
typedef struct _vals_tpi
{
	UCHAR 	id;
	CHAR	tpi[128];
} vals_tpi;

const vals_tpi tpi[MAX_TPI_CNT] =
{
	{ 0x00, "Error" },
	{ 0x01, "Info" },
	{ 0x02, "Option" },
	{ 0x03, "PSN" },
	{ 0x04, "SDU Boundary" },
	{ 0x05, "Frame Boundary" }
	//{ NULL, "(Unknown)" }
};

#define MAX_OPTION_CNT 10
typedef struct _vals_option
{
	UCHAR id;
	CHAR  option[128];
} vals_option;

const vals_option option[MAX_OPTION_CNT] = 
{
	{ 0x01, "Maximum receive unit" },
	{ 0x02, "Total message size" },
	{ 0x03, "Delay trasmission timer" },
	{ 0x04, "Maximum group" },
	{ 0x05, "Current TID" },
	{ 0x06, "No cached TID" },
	{ 0x07, "NumGroups" }
	//{ NULL, "(Unknown)" }
};

/*
 * Bearer types (from the WDP specification).
 */
#define BT_IPv4					0x00
#define BT_IPv6					0x01
#define BT_GSM_USSD				0x02
#define BT_GSM_SMS				0x03
#define BT_ANSI_136_GUTS		0x04
#define BT_IS_95_SMS			0x05
#define BT_IS_95_CSD			0x06
#define BT_IS_95_PACKET_DATA	0x07
#define BT_ANSI_136_CSD			0x08
#define BT_ANSI_136_PACKET_DATA	0x09
#define BT_GSM_CSD				0x0A
#define BT_GSM_GPRS				0x0B
#define BT_GSM_USSD_IPv4		0x0C
#define BT_AMPS_CDPD			0x0D
#define BT_PDC_CSD				0x0E
#define BT_PDC_PACKET_DATA		0x0F
#define BT_IDEN_SMS				0x10
#define BT_IDEN_CSD				0x11
#define BT_IDEN_PACKET_DATA		0x12
#define BT_PAGING_FLEX			0x13
#define BT_PHS_SMS				0x14
#define BT_PHS_CSD				0x15
#define BT_GSM_USSD_GSM_SC		0x16
#define BT_TETRA_SDS_ITSI		0x17
#define BT_TETRA_SDS_MSISDN		0x18
#define BT_TETRA_PACKET_DATA	0x19
#define BT_PAGING_REFLEX		0x1A
#define BT_GSM_USSD_MSISDN		0x1B
#define BT_MOBITEX_MPAK			0x1C
#define BT_ANSI_136_GHOST		0x1D

#define MAX_BEARER_TYPE_CNT 30
typedef struct _vals_bearer_type
{
	UCHAR	id;
	CHAR	bearer_type[128];
} vals_bearer_type;

const vals_bearer_type bearer_type[MAX_BEARER_TYPE_CNT] =
{
	{ BT_IPv4,                 "IPv4" },
	{ BT_IPv6,                 "IPv6" },
	{ BT_GSM_USSD,             "GSM USSD" },
	{ BT_GSM_SMS,              "GSM SMS" },
	{ BT_ANSI_136_GUTS,        "ANSI-136 GUTS/R-Data" },
	{ BT_IS_95_SMS,            "IS-95 CDMA SMS" },
	{ BT_IS_95_CSD,            "IS-95 CDMA CSD" },
	{ BT_IS_95_PACKET_DATA,    "IS-95 CDMA Packet data" },
	{ BT_ANSI_136_CSD,         "ANSI-136 CSD" },
	{ BT_ANSI_136_PACKET_DATA, "ANSI-136 Packet data" },
	{ BT_GSM_CSD,              "GSM CSD" },
	{ BT_GSM_GPRS,             "GSM GPRS" },
	{ BT_GSM_USSD_IPv4,        "GSM USSD (IPv4 addresses)" },
	{ BT_AMPS_CDPD,            "AMPS CDPD" },
	{ BT_PDC_CSD,              "PDC CSD" },
	{ BT_PDC_PACKET_DATA,      "PDC Packet data" },
	{ BT_IDEN_SMS,             "IDEN SMS" },
	{ BT_IDEN_CSD,             "IDEN CSD" },
	{ BT_IDEN_PACKET_DATA,     "IDEN Packet data" },
	{ BT_PAGING_FLEX,          "Paging network FLEX(TM)" },
	{ BT_PHS_SMS,              "PHS SMS" },
	{ BT_PHS_CSD,              "PHS CSD" },
	{ BT_GSM_USSD_GSM_SC,      "GSM USSD (GSM Service Code addresses)" },
	{ BT_TETRA_SDS_ITSI,       "TETRA SDS (ITSI addresses)" },
	{ BT_TETRA_SDS_MSISDN,     "TETRA SDS (MSISDN addresses)" },
	{ BT_TETRA_PACKET_DATA,    "TETRA Packet data" },
	{ BT_PAGING_REFLEX,        "Paging network ReFLEX(TM)" },
	{ BT_GSM_USSD_MSISDN,      "GSM USSD (MSISDN addresses)" },
	{ BT_MOBITEX_MPAK,         "Mobitex MPAK" },
	{ BT_ANSI_136_GHOST,       "ANSI-136 GHOST/R-Data" }
	//{ 0,                       NULL }
};

#define MAX_CHAR_SET_CNT	300
typedef struct _vals_character_set 
{
	USHORT	id;
	CHAR 	character_set[128];
} vals_character_set;

const vals_character_set character_set[MAX_CHAR_SET_CNT] = 
{
    { 0x0000, "*" },
    { 0x0003, "us-ascii" },
    { 0x0004, "iso-8859-1" },
    { 0x0005, "iso-8859-2" },
    { 0x0006, "iso-8859-3" },
    { 0x0007, "iso-8859-4" },
    { 0x0008, "iso-8859-5" },
    { 0x0009, "iso-8859-6" },
    { 0x000A, "iso-8859-7" },
    { 0x000B, "iso-8859-8" },
    { 0x000C, "iso-8859-9" },
    { 0x000D, "iso-8859-10" },
    { 0x000E, "iso_6937-2-add" },
    { 0x000F, "jis_x0201" },
    { 0x0010, "jis_encoding" },
    { 0x0011, "shift_jis" },
    { 0x0012, "euc-jp" },
    { 0x0013, "extended_unix_code_fixed_width_for_japanese" },
    { 0x0014, "bs_4730" },
    { 0x0015, "sen_850200_c" },
    { 0x0016, "it" },
    { 0x0017, "es" },
    { 0x0018, "din_66003" },
    { 0x0019, "ns_4551-1" },
    { 0x001A, "nf_z_62-010" },
    { 0x001B, "iso-10646-utf-1" },
    { 0x001C, "iso_646.basic:1983" },
    { 0x001D, "invariant" },
    { 0x001E, "iso_646.irv:1983" },
    { 0x001F, "nats-sefi" },
    { 0x0020, "nats-sefi-add" },
    { 0x0021, "nats-dano" },
    { 0x0022, "nats-dano-add" },
    { 0x0023, "sen_850200_b" },
    { 0x0024, "ks_c_5601-1987" },
    { 0x0025, "iso-2022-kr" },
    { 0x0026, "euc-kr" },
    { 0x0027, "iso-2022-jp" },
    { 0x0028, "iso-2022-jp-2" },
    { 0x0029, "jis_c6220-1969-jp" },
    { 0x002A, "jis_c6220-1969-ro" },
    { 0x002B, "pt" },
    { 0x002C, "greek7-old" },
    { 0x002D, "latin-greek" },
    { 0x002E, "nf_z_62-010_(1973)" },
    { 0x002F, "latin-greek-1" },
    { 0x0030, "iso_5427" },
    { 0x0031, "jis_c6226-1978" },
    { 0x0032, "bs_viewdata" },
    { 0x0033, "inis" },
    { 0x0034, "inis-8" },
    { 0x0035, "inis-cyrillic" },
    { 0x0036, "iso_5427:1981" },
    { 0x0037, "iso_5428:1980" },
    { 0x0038, "gb_1988-80" },
    { 0x0039, "gb_2312-80" },
    { 0x003A, "ns_4551-2" },
    { 0x003B, "videotex-suppl" },
    { 0x003C, "pt2" },
    { 0x003D, "es2" },
    { 0x003E, "msz_7795.3" },
    { 0x003F, "jis_c6226-1983" },
    { 0x0040, "greek7" },
    { 0x0041, "asmo_449" },
    { 0x0042, "iso-ir-90" },
    { 0x0043, "jis_c6229-1984-a" },
    { 0x0044, "jis_c6229-1984-b" },
    { 0x0045, "jis_c6229-1984-b-add" },
    { 0x0046, "jis_c6229-1984-hand" },
    { 0x0047, "jis_c6229-1984-hand-add" },
    { 0x0048, "jis_c6229-1984-kana" },
    { 0x0049, "iso_2033-1983" },
    { 0x004A, "ansi_x3.110-1983" },
    { 0x004B, "t.61-7bit" },
    { 0x004C, "t.61-8bit" },
    { 0x004D, "ecma-cyrillic" },
    { 0x004E, "csa_z243.4-1985-1" },
    { 0x004F, "csa_z243.4-1985-2" },
    { 0x0050, "csa_z243.4-1985-gr" },
    { 0x0051, "iso_8859-6-e" },
    { 0x0052, "iso_8859-6-i" },
    { 0x0053, "t.101-g2" },
    { 0x0054, "iso_8859-8-e" },
    { 0x0055, "iso_8859-8-i" },
    { 0x0056, "csn_369103" },
    { 0x0057, "jus_i.b1.002" },
    { 0x0058, "iec_p27-1" },
    { 0x0059, "jus_i.b1.003-serb" },
    { 0x005A, "jus_i.b1.003-mac" },
    { 0x005B, "greek-ccitt" },
    { 0x005C, "nc_nc00-10:81" },
    { 0x005D, "iso_6937-2-25" },
    { 0x005E, "gost_19768-74" },
    { 0x005F, "iso_8859-supp" },
    { 0x0060, "iso_10367-box" },
    { 0x0061, "latin-lap" },
    { 0x0062, "jis_x0212-1990" },
    { 0x0063, "ds_2089" },
    { 0x0064, "us-dk" },
    { 0x0065, "dk-us" },
    { 0x0066, "ksc5636" },
    { 0x0067, "unicode-1-1-utf-7" },
    { 0x0068, "iso-2022-cn" },
    { 0x0069, "iso-2022-cn-ext" },
    { 0x006A, "utf-8" },
    { 0x006D, "iso-8859-13" },
    { 0x006E, "iso-8859-14" },
    { 0x006F, "iso-8859-15" },
    { 0x03E8, "iso-10646-ucs-2" },
    { 0x03E9, "iso-10646-ucs-4" },
    { 0x03EA, "iso-10646-ucs-basic" },
    { 0x03EB, "iso-10646-j-1" },
    { 0x03EB, "iso-10646-unicode-latin1" },
    { 0x03ED, "iso-unicode-ibm-1261" },
    { 0x03EE, "iso-unicode-ibm-1268" },
    { 0x03EF, "iso-unicode-ibm-1276" },
    { 0x03F0, "iso-unicode-ibm-1264" },
    { 0x03F1, "iso-unicode-ibm-1265" },
    { 0x03F2, "unicode-1-1" },
    { 0x03F3, "scsu" },
    { 0x03F4, "utf-7" },
    { 0x03F5, "utf-16be" },
    { 0x03F6, "utf-16le" },
    { 0x03F7, "utf-16" },
    { 0x07D0, "iso-8859-1-windows-3.0-latin-1" },
    { 0x07D1, "iso-8859-1-windows-3.1-latin-1" },
    { 0x07D2, "iso-8859-2-windows-latin-2" },
    { 0x07D3, "iso-8859-9-windows-latin-5" },
    { 0x07D4, "hp-roman8" },
    { 0x07D5, "adobe-standard-encoding" },
    { 0x07D6, "ventura-us" },
    { 0x07D7, "ventura-international" },
    { 0x07D8, "dec-mcs" },
    { 0x07D9, "ibm850" },
    { 0x07DA, "ibm852" },
    { 0x07DB, "ibm437" },
    { 0x07DC, "pc8-danish-norwegian" },
    { 0x07DD, "ibm862" },
    { 0x07DE, "pc8-turkish" },
    { 0x07DF, "ibm-symbols" },
    { 0x07E0, "ibm-thai" },
    { 0x07E1, "hp-legal" },
    { 0x07E2, "hp-pi-font" },
    { 0x07E3, "hp-math8" },
    { 0x07E4, "adobe-symbol-encoding" },
    { 0x07E5, "hp-desktop" },
    { 0x07E6, "ventura-math" },
    { 0x07E7, "microsoft-publishing" },
    { 0x07E8, "windows-31j" },
    { 0x07E9, "gb2312" },
    { 0x07EA, "big5" },
    { 0x07EB, "macintosh" },
    { 0x07EC, "ibm037" },
    { 0x07ED, "ibm038" },
    { 0x07EE, "ibm273" },
    { 0x07EF, "ibm274" },
    { 0x07F0, "ibm275" },
    { 0x07F1, "ibm277" },
    { 0x07F2, "ibm278" },
    { 0x07F3, "ibm280" },
    { 0x07F4, "ibm281" },
    { 0x07F5, "ibm284" },
    { 0x07F6, "ibm285" },
    { 0x07F7, "ibm290" },
    { 0x07F8, "ibm297" },
    { 0x07F9, "ibm420" },
    { 0x07FA, "ibm423" },
    { 0x07FB, "ibm424" },
    { 0x07FC, "ibm500" },
    { 0x07FD, "ibm851" },
    { 0x07FE, "ibm855" },
    { 0x07FF, "ibm857" },
    { 0x0800, "ibm860" },
    { 0x0801, "ibm861" },
    { 0x0802, "ibm863" },
    { 0x0803, "ibm864" },
    { 0x0804, "ibm865" },
    { 0x0805, "ibm868" },
    { 0x0806, "ibm869" },
    { 0x0807, "ibm870" },
    { 0x0808, "ibm871" },
    { 0x0809, "ibm880" },
    { 0x080A, "ibm891" },
    { 0x080B, "ibm903" },
    { 0x080C, "ibm904" },
    { 0x080D, "ibm905" },
    { 0x080E, "ibm918" },
    { 0x080F, "ibm1026" },
    { 0x0810, "ebcdic-at-de" },
    { 0x0811, "ebcdic-at-de-a" },
    { 0x0812, "ebcdic-ca-fr" },
    { 0x0813, "ebcdic-dk-no" },
    { 0x0814, "ebcdic-dk-no-a" },
    { 0x0815, "ebcdic-fi-se" },
    { 0x0816, "ebcdic-fi-se-a" },
    { 0x0817, "ebcdic-fr" },
    { 0x0818, "ebcdic-it" },
    { 0x0819, "ebcdic-pt" },
    { 0x081A, "ebcdic-es" },
    { 0x081B, "ebcdic-es-a" },
    { 0x081C, "ebcdic-es-s" },
    { 0x081D, "ebcdic-uk" },
    { 0x081E, "ebcdic-us" },
    { 0x081F, "unknown-8bit" },
    { 0x0820, "mnemonic" },
    { 0x0821, "mnem" },
    { 0x0822, "viscii" },
    { 0x0823, "viqr" },
    { 0x0824, "koi8-r" },
    { 0x0825, "hz-gb-2312" },
    { 0x0826, "ibm866" },
    { 0x0827, "ibm775" },
    { 0x0828, "koi8-u" },
    { 0x0829, "ibm00858" },
    { 0x082A, "ibm00924" },
    { 0x082B, "ibm01140" },
    { 0x082C, "ibm01141" },
    { 0x082D, "ibm01142" },
    { 0x082E, "ibm01143" },
    { 0x082F, "ibm01144" },
    { 0x0830, "ibm01145" },
    { 0x0831, "ibm01146" },
    { 0x0832, "ibm01147" },
    { 0x0833, "ibm01148" },
    { 0x0834, "ibm01149" },
    { 0x0835, "big5-hkscs" },
    { 0x08CA, "windows-1250" },
    { 0x08CB, "windows-1251" },
    { 0x08CC, "windows-1252" },
    { 0x08CD, "windows-1253" },
    { 0x08CE, "windows-1254" },
    { 0x08CF, "windows-1255" },
    { 0x08D0, "windows-1256" },
    { 0x08D1, "windows-1257" },
    { 0x08D2, "windows-1258" },
    { 0x08D3, "tis-620" },
    { 0x0000, "" }
};


#define MAX_APP_ID_CNT  15
typedef struct _vals_application_ids
{
	UCHAR 	id;
	CHAR	application[128];
} vals_application_ids;

const vals_application_ids application_ids[MAX_APP_ID_CNT] = {
    /* Well-known WAP applications */
    { 0x00, "x-wap-application:*" },
    { 0x01, "x-wap-application:push.sia" },
    { 0x02, "x-wap-application:wml.ua" },
    { 0x03, "x-wap-application:wta.ua" },
    { 0x04, "x-wap-application:mms.ua" },
    { 0x05, "x-wap-application:push.syncml" },
    { 0x06, "x-wap-application:loc.ua" },
    { 0x07, "x-wap-application:syncml.dm" },
    { 0x08, "x-wap-application:drm.ua" },
    { 0x09, "x-wap-application:emn.ua" },
    { 0x0A, "x-wap-application:wv.ua" }
#if 0
    /* Registered by 3rd parties */
    { 0x8000, "x-wap-microsoft:localcontent.ua" },
    { 0x8001, "x-wap-microsoft:IMclient.ua" },
    { 0x8002, "x-wap-docomo:imode.mail.ua" },
    { 0x8003, "x-wap-docomo:imode.mr.ua" },
    { 0x8004, "x-wap-docomo:imode.mf.ua" },
    { 0x8005, "x-motorola:location.ua" },
    { 0x8006, "x-motorola:now.ua" },
    { 0x8007, "x-motorola:otaprov.ua" },
    { 0x8008, "x-motorola:browser.ua" },
    { 0x8009, "x-motorola:splash.ua" },
    /* 0x800A: unassigned */
    { 0x800B, "x-wap-nai:mvsw.command" },
    /* 0x800C -- 0x800F: unassigned */
    { 0x8010, "x-wap-openwave:iota.ua" },
    /* 0x8011 -- 0x8FFF: unassigned */
    { 0x9000, "x-wap-docomo:imode.mail2.ua" },
    { 0x9001, "x-oma-nec:otaprov.ua" },
    { 0x9002, "x-oma-nokia:call.ua" },
    { 0x9003, "x-oma-coremobility:sqa.ua" },
    { 0x00, ""}
#endif
};

#define MAX_LANGUAGES_CNT 150
typedef struct _vals_languages 
{
	UCHAR	id;
	CHAR	languages[128];
} vals_languages;

const vals_languages languages[MAX_LANGUAGES_CNT] = {
    { 0x01, "Afar (aa)" },
    { 0x02, "Abkhazian (ab)" },
    { 0x03, "Afrikaans (af)" },
    { 0x04, "Amharic (am)" },
    { 0x05, "Arabic (ar)" },
    { 0x06, "Assamese (as)" },
    { 0x07, "Aymara (ay)" },
    { 0x08, "Azerbaijani (az)" },
    { 0x09, "Bashkir (ba)" },
    { 0x0A, "Byelorussian (be)" },
    { 0x0B, "Bulgarian (bg)" },
    { 0x0C, "Bihari (bh)" },
    { 0x0D, "Bislama (bi)" },
    { 0x0E, "Bengali; Bangla (bn)" },
    { 0x0F, "Tibetan (bo)" },
    { 0x10, "Breton (br)" },
    { 0x11, "Catalan (ca)" },
    { 0x12, "Corsican (co)" },
    { 0x13, "Czech (cs)" },
    { 0x14, "Welsh (cy)" },
    { 0x15, "Danish (da)" },
    { 0x16, "German (de)" },
    { 0x17, "Bhutani (dz)" },
    { 0x18, "Greek (el)" },
    { 0x19, "English (en)" },
    { 0x1A, "Esperanto (eo)" },
    { 0x1B, "Spanish (es)" },
    { 0x1C, "Estonian (et)" },
    { 0x1D, "Basque (eu)" },
    { 0x1E, "Persian (fa)" },
    { 0x1F, "Finnish (fi)" },
    { 0x20, "Fiji (fj)" },
    { 0x21, "Urdu (ur)" },
    { 0x22, "French (fr)" },
    { 0x23, "Uzbek (uz)" },
    { 0x24, "Irish (ga)" },
    { 0x25, "Scots Gaelic (gd)" },
    { 0x26, "Galician (gl)" },
    { 0x27, "Guarani (gn)" },
    { 0x28, "Gujarati (gu)" },
    { 0x29, "Hausa (ha)" },
    { 0x2A, "Hebrew (formerly iw) (he)" },
    { 0x2B, "Hindi (hi)" },
    { 0x2C, "Croatian (hr)" },
    { 0x2D, "Hungarian (hu)" },
    { 0x2E, "Armenian (hy)" },
    { 0x2F, "Vietnamese (vi)" },
    { 0x30, "Indonesian (formerly in) (id)" },
    { 0x31, "Wolof (wo)" },
    { 0x32, "Xhosa (xh)" },
    { 0x33, "Icelandic (is)" },
    { 0x34, "Italian (it)" },
    { 0x35, "Yoruba (yo)" },
    { 0x36, "Japanese (ja)" },
    { 0x37, "Javanese (jw)" },
    { 0x38, "Georgian (ka)" },
    { 0x39, "Kazakh (kk)" },
    { 0x3A, "Zhuang (za)" },
    { 0x3B, "Cambodian (km)" },
    { 0x3C, "Kannada (kn)" },
    { 0x3D, "Korean (ko)" },
    { 0x3E, "Kashmiri (ks)" },
    { 0x3F, "Kurdish (ku)" },
    { 0x40, "Kirghiz (ky)" },
    { 0x41, "Chinese (zh)" },
    { 0x42, "Lingala (ln)" },
    { 0x43, "Laothian (lo)" },
    { 0x44, "Lithuanian (lt)" },
    { 0x45, "Latvian, Lettish (lv)" },
    { 0x46, "Malagasy (mg)" },
    { 0x47, "Maori (mi)" },
    { 0x48, "Macedonian (mk)" },
    { 0x49, "Malayalam (ml)" },
    { 0x4A, "Mongolian (mn)" },
    { 0x4B, "Moldavian (mo)" },
    { 0x4C, "Marathi (mr)" },
    { 0x4D, "Malay (ms)" },
    { 0x4E, "Maltese (mt)" },
    { 0x4F, "Burmese (my)" },
    { 0x50, "Ukrainian (uk)" },
    { 0x51, "Nepali (ne)" },
    { 0x52, "Dutch (nl)" },
    { 0x53, "Norwegian (no)" },
    { 0x54, "Occitan (oc)" },
    { 0x55, "(Afan) Oromo (om)" },
    { 0x56, "Oriya (or)" },
    { 0x57, "Punjabi (pa)" },
    { 0x58, "Polish (po)" },
    { 0x59, "Pashto, Pushto (ps)" },
    { 0x5A, "Portuguese (pt)" },
    { 0x5B, "Quechua (qu)" },
    { 0x5C, "Zulu (zu)" },
    { 0x5D, "Kirundi (rn)" },
    { 0x5E, "Romanian (ro)" },
    { 0x5F, "Russian (ru)" },
    { 0x60, "Kinyarwanda (rw)" },
    { 0x61, "Sanskrit (sa)" },
    { 0x62, "Sindhi (sd)" },
    { 0x63, "Sangho (sg)" },
    { 0x64, "Serbo-Croatian (sh)" },
    { 0x65, "Sinhalese (si)" },
    { 0x66, "Slovak (sk)" },
    { 0x67, "Slovenian (sl)" },
    { 0x68, "Samoan (sm)" },
    { 0x69, "Shona (sn)" },
    { 0x6A, "Somali (so)" },
    { 0x6B, "Albanian (sq)" },
    { 0x6C, "Serbian (sr)" },
    { 0x6D, "Siswati (ss)" },
    { 0x6E, "Sesotho (st)" },
    { 0x6F, "Sundanese (su)" },
    { 0x70, "Swedish (sv)" },
    { 0x71, "Swahili (sw)" },
    { 0x72, "Tamil (ta)" },
    { 0x73, "Telugu (te)" },
    { 0x74, "Tajik (tg)" },
    { 0x75, "Thai (th)" },
    { 0x76, "Tigrinya (ti)" },
    { 0x77, "Turkmen (tk)" },
    { 0x78, "Tagalog (tl)" },
    { 0x79, "Setswana (tn)" },
    { 0x7A, "Tonga (to)" },
    { 0x7B, "Turkish (tr)" },
    { 0x7C, "Tsonga (ts)" },
    { 0x7D, "Tatar (tt)" },
    { 0x7E, "Twi (tw)" },
    { 0x7F, "Uighur (ug)" },
    { 0x81, "Nauru (na)" },
    { 0x82, "Faeroese (fo)" },
    { 0x83, "Frisian (fy)" },
    { 0x84, "Interlingua (ia)" },
    { 0x85, "Volapuk (vo)" },
    { 0x86, "Interlingue (ie)" },
    { 0x87, "Inupiak (ik)" },
    { 0x88, "Yiddish (formerly ji) (yi)" },
    { 0x89, "Inuktitut (iu)" },
    { 0x8A, "Greenlandic (kl)" },
    { 0x8B, "Latin (la)" },
    { 0x8C, "Rhaeto-Romance (rm)" },
    { 0x00, "" }
};


/* Functions *********/
BOOL AnalyzeIP_UDP_WAP(PANALYZE_INFO_WAP10 pInfo, PUCHAR pucBuffer, WORD wSize, USHORT usAppCode );
void AnalyzeIP_UDP_WAP_CONCAT_WTP(PANALYZE_INFO_WAP10 pInfo, PUCHAR pucBuffer, WORD wSize);
void AnalyzeIP_UDP_WAP_WTP_PDU(PANALYZE_INFO_WAP10 pInfo, PUCHAR pucBuffer, WORD wSize );
void AnalyzeIP_UDP_WAP_WTP_OCTET1(PANALYZE_INFO_WAP10 pInfo, WTP_HDR *wtp_hdr, UCHAR type);
void AnalyzeIP_UDP_WAP_WTP_OCTET2_3(PANALYZE_INFO_WAP10 pInfo, WTP_HDR *wtp_hdr);
UCHAR *AnalyzeIP_UDP_WAP_WTP_OCTET4(PANALYZE_INFO_WAP10 pInfo, WTP_HDR *wtp_hdr, WORD *wSize, UCHAR type);
WORD AnalyzeIP_UDP_WAP_WTP_TPI(PANALYZE_INFO_WAP10 pInfo, PUCHAR pucBuffer, WORD wSize);
WORD AnalyzeIP_UDP_WAP_WSP(PANALYZE_INFO_WAP10 pInfo, PUCHAR pucBuffer, WORD wSize);
UCHAR *AnalyzeWSP_Cap(PANALYZE_INFO_WAP10 pInfo, PUCHAR pucBuffer, int nSize);
void AnalyzeWSP_Headers(PANALYZE_INFO_WAP10 pInfo, PUCHAR pucBuffer, int nSize);
int GetVariableLengthWORD(UCHAR *pData, WORD *wLen);
int GetVariableLengthDWORD(UCHAR *pData, DWORD *dwLen);
int _decode(PUCHAR pVal, DWORD *dwVal, int *nLen);
PUCHAR pGetMediaType(UCHAR id);
PUCHAR pGetCharacterSet(WORD id);
PUCHAR pGetLanguages(UCHAR id);
PUCHAR pGetBearerType(UCHAR id);
PUCHAR pGetApplicationIds(UCHAR id);
DWORD _get_guintvar(PUCHAR tvb);
DWORD _uintvar(PUCHAR pVal, int *nLen);
int _insert_content(PANALYZE_INFO_WAP10 pInfo, UCHAR *pTemp);
UCHAR *AnalyzeIP_UDP_WAP_WSP_Connect(PANALYZE_INFO_WAP10 pInfo, PUCHAR pucBuffer, WORD *wSize);
UCHAR *AnalyzeIP_UDP_WAP_WSP_ConReply(PANALYZE_INFO_WAP10 pInfo, PUCHAR pucBuffer, WORD *wSize);
UCHAR *AnalyzeIP_UDP_WAP_WSP_Redirect(PANALYZE_INFO_WAP10 pInfo, PUCHAR pucBuffer, WORD *wSize);
UCHAR *AnalyzeIP_UDP_WAP_WSP_Disconnect(PANALYZE_INFO_WAP10 pInfo, PUCHAR pucBuffer, WORD *wSize);
UCHAR *AnalyzeIP_UDP_WAP_WSP_Reply(PANALYZE_INFO_WAP10 pInfo, PUCHAR pucBuffer, WORD *wSize);
UCHAR *AnalyzeIP_UDP_WAP_WSP_Get(PANALYZE_INFO_WAP10 pInfo, PUCHAR pucBuffer, WORD *wSize);
UCHAR *AnalyzeIP_UDP_WAP_WSP_Post(PANALYZE_INFO_WAP10 pInfo, PUCHAR pucBuffer, WORD *wSize);
UCHAR *AnalyzeIP_UDP_WAP_WSP_Push(PANALYZE_INFO_WAP10 pInfo, PUCHAR pucBuffer, WORD *wSize);
UCHAR *AnalyzeIP_UDP_WAP_WSP_Suspend(ANALYZE_INFO_WAP10 *pResult, PUCHAR pucBuffer, WORD *wSize);
UCHAR *AnalyzeIP_UDP_WAP_WSP_Resume(PANALYZE_INFO_WAP10 pInfo, PUCHAR pucBuffer, WORD *wSize );
UCHAR *AnalyzeIP_UDP_WAP_WSP_FragPDU(PANALYZE_INFO_WAP10 pInfo, PUCHAR pucBuffer, WORD *wSize);

#pragma pack(0)
#endif
