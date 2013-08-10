#ifndef __CAPD_TEST_H__
#define __CAPD_TEST_H__


/**A.1*  File Inclusion *******************************************************/
#include <dirent.h>
#include <typedef.h>

typedef struct _st_PortStatus_
{
	unsigned int	uiCurrCnt;
	unsigned int	uiLastCnt;
	unsigned int	uiRetryCnt;
	unsigned int	uiReserved;
}st_PortStatus, *pst_PortStatus;

typedef struct _st_RP_Signal_
{
    UINT    uiCreateTime;
    UINT    uiCreateMTime;
    char    szIMSI[16];

    UINT    uiLastUpdateTime;
    UINT    uiLastUpdateMTime;

    USHORT  usStatusCode;
    USHORT  usReserved[3];

    UINT    uiGREKey;
    UINT    uiServingPCF;
    UINT    uiHomeAgent;
    UINT    uiSvcOption;
    UINT    uiFMux;
    UINT    uiRMux;

    char    szBSMSC[14];
    UCHAR   ucAppType;
    UCHAR   ucReserved;

    UINT    uiUpLCPStartTime;
    UINT    uiUpLCPStartMTime;
    UINT    uiDownLCPStartTime;
    UINT    uiDownLCPStartMTime;

    UINT    uiIPCPStartTime;
    UINT    uiIPCPStartMTime;

    UINT    uiPPPSetupTime;
    UINT    uiPPPSetupMTime;

    UINT    uiPPPTermTime;
    UINT    uiPPPTermMTime;

    UINT    uiAuthReqTime;
    UINT    uiAuthReqMTime;

    UINT    uiDNSReqTime;
    UINT    uiDNSReqMTime;

    UINT    uiTCPSynTime;
    UINT    uiTCPSynMTime;

    UINT    uiRPSetupDuration;
    UINT    uiAuthDuration;
    UINT    uiLCPDuration;
    UINT    uiIPCPDuration;
    UINT    uiDNSRepDuration;
    UINT    uiTCPSetupDuration;

    UINT    uiReleaseTime;
    UINT    uiReleaseMTime;

	UINT    uiIPAddr;
    UINT    uiSvcDestIP;

    UINT    uiDNSIP;
    UINT    uiNASIP;

    USHORT  usSvcPort;
    UCHAR   ucSvcProtocol;
    UCHAR   ucSvcGroup;
    UCHAR   ucSvcCode;
    UCHAR   ucSvcOptChgCount;
    UCHAR   ucBSMSCChgCount;
    UCHAR   ucReserved2;

    UCHAR   ucRegiRepCode;
    UCHAR   ucAuthMethod;
    UCHAR   ucAuthResult;
    UCHAR   ucIPChgCount;
    UCHAR   ucPPPSetupCount;
    UCHAR   ucDNSRet;
    UCHAR   ucAStartCount;
    UCHAR   ucAStopCount;

    UINT    uiUpPPPFrames;
    UINT    uiDownPPPFrames;
    UINT    uiUpPPPBytes;
    UINT    uiDownPPPBytes;

    UINT    uiUpFCSErrFrames;
    UINT    uiDownFCSErrFrames;
    UINT    uiUpFCSErrBytes;
    UINT    uiDownFCSErrBytes;

    UCHAR   ucRegiReqCount;
    UCHAR   ucRegiSuccCount;
    UCHAR   ucUpdateReqCount;
    UCHAR   ucUpdateAckCount;
    UCHAR   ucUpLCPReqCount;
    UCHAR   ucDownLCPReqCount;
    UCHAR   ucUpIPCPReqCount;
    UCHAR   ucDownIPCPReqCount;

} st_RP_Signal, *pst_RP_Signal;

#define DEF_PORTSTATUS_SIZE		sizeof(st_PortStatus)

#define DEF_ACTIVE		0x03	
#define DEF_STANDBY		0x01

/**B.1*  Definition of New Constants ******************************************/
#define STR_EXT_CAP			"cap"
#define PCAP_FILE_HDR_SIZE	24

#define DEF_CONTINUE		1
#define DEF_END			100

extern unsigned char *Send_Node_Head;

/**B.2*  Definition of New Type  **********************************************/
typedef struct _st_data_header_
{
	//unsigned long long	dwTime;
	unsigned int		capTime;
	unsigned int		capMTime;
	unsigned int		dwLen;
	unsigned int		dwLen2;
} st_PcapHdr_t, *pst_PcapHdr_t;
#define PCAP_DATA_HDR_SIZE	sizeof(st_PcapHdr_t)


/**C.1*  Declaration of Variables  ********************************************/

/**C.1*  Declaration of Variables  ********************************************/

/**D.1*  Definition of Functions  *********************************************/
int	dGetPREAIndex(char *data);
int	dSend_CAPD_Data(stMEMSINFO *pstMEMSINFO, S32 dIdx, U8 *pNode, U32 sec, int type);
int	parse_data(st_PcapHdr_t *pstHeader, char *szBuff);
int	FilterSOAP(const struct dirent *entry);
void test_func2(char *szDirName, int SLEEPCNT);

#endif /* __CAPD_TEST_H__ */

