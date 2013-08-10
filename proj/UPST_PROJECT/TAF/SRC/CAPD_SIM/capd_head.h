#ifndef _CAPD_HEAD_H_
#define _CAPD_HEAD_H_

#include <typedef.h>

#define ETHERNET_WLEN(h)    (ntohs((h)->wlen) - (fcs_bits >> 3))
#define ETHERNET_SLEN(h)    dagutil_min(ETHERNET_WLEN(h), ntohs((h)->rlen) - dag_record_size - 2)

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

/* FUNCTION *******************************************************************/
void UserControlledSignal(int sign);
void IgnoreSignal(int sign);
void SetUpSignal();
void FinishProgram();

int open_device(char *dagname_buf);
int close_device(void);
ULONG read_one_packet(void *buffer, ULONG bufferlen, int dDagNum );
void do_packet_capture(void);
int handle_ethernet(dag_record_t *cap_rec, int dDagNum );
void conv_ts2tv(uint64_t ts, struct timeval *tv);
int do_action(int port, int len, char *data, struct timeval *tmv);
//int dSend_CAPD_Data(stMEMSINFO *pstMEMSINFO, S32 dSndMsgQ, U8 *pNode, U32 sec);
int dSend_CAPD_Data(stMEMSINFO *pstMEMSINFO, U8 *pNode, U32 sec);

extern void InitAppLog(pid_t pid, int procidx, char *logfilepath, char *proc_name);

#endif
