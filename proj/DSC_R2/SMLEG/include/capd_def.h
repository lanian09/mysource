#ifndef __CAPD_DEF_H__
#define __CAPD_DEF_H__

#pragma pack(1)

#include <sys/time.h>

#include <ipaf_svc.h>
//#include <dagapi.h>
#include <sfm_msgtypes.h>

extern int dag_devnum;
extern int dag_fd;
extern int loop_continue;
SFM_SysCommMsgType *SFMSysCommMsgType;
extern char syslabel[];

/*
 * Size of Ethernet payload (from DAG source)
 */
extern int fcs_bits;
#define ETHERNET_WLEN(h)    (ntohs((h)->wlen) - (fcs_bits >> 3))
#define ETHERNET_SLEN(h)    dagutil_min(ETHERNET_WLEN(h), ntohs((h)->rlen) - dag_record_size - 2)

#define MAX_DEV_CNT     2
#define MAX_DEV_NAME_LEN			15
#define MAX_DEV_NAME_SIZE			MAX_DEV_NAME_LEN+1	


void SetupSignal(void);
void CatchSignal(int sig);
void IgnoreSignal(int sig);
int close_device(void);
void conv_ts2tv(uint64_t ts, struct timeval *tv);
void do_packet_capture(void);
//int handle_ethernet(dag_record_t *cap_rec);
//int open_device(char *dagname_buf);
int open_device(void);
ULONG read_one_packet(void *buffer, ULONG bufferlen);
void do_action(int port, int len, char *data, struct timeval *tmv);
int init_ipc(void);

#pragma pack(0)
#endif /* __CAPD_DEF_H__ */

