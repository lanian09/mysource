#ifndef _CAPD_FUNC_H_
#define _CAPD_FUNC_H_


/**
 * Define constants
 */
#define ETHERNET_WLEN(h)		(ntohs((h)->wlen) - (fcs_bits >> 3))
#define ETHERNET_SLEN(h)		dagutil_min(ETHERNET_WLEN(h), ntohs((h)->rlen) - dag_record_size - 2)

#define OPTBUFSIZE				ONE_KIBI
#define MAX_READ				5000

#ifdef BUFFERING
#define COLLECTION_MIN			50
#else
#define COLLECTION_MIN			0
#endif

#define COLLECTION_MAX			100
#define COLLECTION_TIME			5
#define COLLECTION_MULTIPLY		2

#define DEF_ACTIVE				0x03	
#define DEF_STANDBY				0x01

/**
 *	Define structures
 */
typedef struct _st_PortStatus_
{
	unsigned int	uiCurrCnt;
	unsigned int	uiLastCnt;
	unsigned int	uiRetryCnt;
	unsigned int	uiReserved;
}st_PortStatus, *pst_PortStatus;
#define DEF_PORTSTATUS_SIZE		sizeof(st_PortStatus)

/**
 * Declare functions
 */
extern int open_device(char *dagname_buf);
extern int close_device(void);
extern void do_packet_capture(void);
extern ULONG read_one_packet(void *buffer, ULONG bufferlen, int dDagNum );
extern int handle_ethernet(dag_record_t *cap_rec, int dDagNum );
extern void conv_ts2tv(uint64_t ts, struct timeval *tv);
extern int dump_DebugString(char *debug_str, char *s, int len);
extern int do_action(int port, int len, char *data, struct timeval *tmv);
extern int dSend_CAPD_Data(stMEMSINFO *pstMEMSINFO, U8 *pNode, U32 sec);

#endif	/* _CAPD_FUNC_H_ */
