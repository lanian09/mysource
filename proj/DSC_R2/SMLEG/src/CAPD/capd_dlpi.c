#include <sys/shm.h>
#include <errno.h>
#include <pcap.h>
#include <netinet/in.h>
#include <netinet/ip.h>

#include <sys/types.h>
#include <sys/dlpi.h>
#include <sys/stream.h>
#include <stdio.h>

#include <stropts.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <shmutil.h>
#include <utillib.h>
#include <time.h>
#include <eth_capd.h>

#include "comm_define.h"
#include "capd_def.h"
#include "nifo.h"
#include "mems.h"
#include "comm_util.h"
#include "capd_bfr.h"

/* Access macros for packed messages.
 *
 * Packed message format:
 *
 *    message type (M_*)    :   int, 4 byte
 *    message length        :   int, 4 byte
 *    message bytes         :   "message length" bytes
 *    alignment to 64 bit border:   0 ... 7 bytes
 *
 * This structure is repeated for all b_cont parts of a message.
 * These macros can be applied to the data buffer returned by getmsg().
 * The buffer must be 64 bit alined for the macros to function correctly.
 */
#define SSP_MSG_TYPE(p)     ( * (int *)(p) )
#define SSP_MSG_LEN(p)      ( * ((int *)(p) + 1) )
#define SSP_MSG_START(p)    ( (char *)(p) + 2 * sizeof(int) )
#define SSP_MSG_NEXT(p)     SSP_ALIGN8( SSP_MSG_START(p) + SSP_MSG_LEN(p) )
#define SSP_MSG_LEN_NEXT(p, l)  SSP_ALIGN8( SSP_MSG_START(p) + l ) 

/* default snap length (maximum bytes per packet to capture) */
#define SNAP_LEN 		1518
#define	MAX_RD_CNT  	100
#define DLPI_RCV_BUF	1024
/* TODO: 설정파일에서 읽어오는 부분 추가 */
int 	dCurDev;
int		gdDevCnt = 0;
unsigned int RAD_PACKET_CNT[2];
char	gszDevName[MAX_DEV_CNT][MAX_DEV_NAME_SIZE] = { "e1000g3", "e10005" };
extern stMEMSINFO       *pstMEMSINFO;
extern SFM_SysCommMsgType   *loc_sadb;
extern int dANAQid;
#ifdef BUFFERING
time_t              nowTime = 0;
time_t              oldTime = 0;
UINT                Diff_Node_Cnt = 0;
UINT                dPreCollection_Cnt= 0;
#endif /* BUFFERING */
extern int send_packet(UCHAR *pNode, T_CAPHDR *pHead, UCHAR *pBuffer);
extern int get_nifo_node (UCHAR *pNode, T_CAPHDR *pHead, UCHAR *pBuffer);
extern int dSend_CAPD_Data(stMEMSINFO *pstMEMSINFO, S32 dSndMsgQ, U8 *pNode, U32 sec);
void	keepalivelib_increase();

#define MAXDATA (64*1024)
#define IPADDR(d) ((d>>24)&0xff),((d>>16)&0xff),((d>>8)&0xff),(d&0xff)


#define WIDTH   16
int
dump(char *s,int len)
{
    char buf[BUFSIZ],lbuf[BUFSIZ],rbuf[BUFSIZ];
    unsigned char *p;
    int line,i;

    p =(unsigned char *) s;
    for(line = 1; len > 0; len -= WIDTH,line++) {
        memset(lbuf,0,BUFSIZ);
        memset(rbuf,0,BUFSIZ);

        for(i = 0; i < WIDTH && len > i; i++,p++) {
            sprintf(buf,"%02x ",(unsigned char) *p);
            strcat(lbuf,buf);
            sprintf(buf,"%c",(!iscntrl(*p) && *p <= 0x7f) ? *p : '.');
            strcat(rbuf,buf);
        }
        printf("%04x: %-*s    %s\n",line - 1,WIDTH * 3,lbuf,rbuf);
    }
    if(!(len%16)) printf("\n");
    return line;
}



int
dlpi_read(int fd, char *buf, int len)
{
    int flags;
    struct strbuf ctl, data;
    union DL_primitives dlp;
	// dl_unitdata_ind_t *up = (dl_unitdata_ind_t*)&dlp;

    data.buf = buf;
    data.maxlen = len;
    data.len = 0;
    ctl.buf = (char *) &dlp;
    ctl.maxlen = sizeof(dlp);
    ctl.len = 0;
    flags = 0;
    getmsg(fd, &ctl, &data, &flags);

#if 0
    printf("dl=>%d \n", up->dl_dest_addr_length);
    printf("do=>%d \n", up->dl_dest_addr_offset);
    printf("sl=>%d \n", up->dl_src_addr_length);
    printf("so=>%d \n", up->dl_src_addr_offset);
    if(ctl.len>0) dump(ctl.buf, ctl.len);
#endif

    return data.len;
}


int
dlpi_open(char *dev)
{
#if 0
    register char *cp;
    dl_info_ack_t *infop;
    dl_info_req_t inforeq;
    dl_phys_addr_req_t physaddrreq;
    dl_error_ack_t *edlp;
#endif
    struct    strbuf    ctl;
    union DL_primitives dlp;
    dl_bind_req_t    bindreq;
    dl_attach_req_t attachreq;
    dl_promiscon_req_t promisconreq;
    int fd, flags;
    
    fd = open(dev, O_RDWR);  /* for instance, /dev/elxl0 */
    
    /* attach to a specific interface */
    attachreq.dl_primitive = DL_ATTACH_REQ;
    attachreq.dl_ppa = 0;  /* assume we want /dev/xxx0 */
    ctl.maxlen = 0;
    ctl.len = sizeof(attachreq);
    ctl.buf = (char *)&attachreq;
    flags = 0;

    /* send attach req */                                                     
    putmsg(fd, &ctl, (struct strbuf *)NULL, flags);                           
    ctl.maxlen = sizeof(dlp);                                                 
    ctl.len = 0;                                                              
    ctl.buf = (char *)&dlp;                                                   
    
	/* get ok ack, may contain error */                                       
    getmsg(fd, &ctl, (struct strbuf*)NULL, &flags);                           
    memset((char *)&bindreq, 0, sizeof(bindreq));                             
    
	/* the following bind might not need to be done */                        
    bindreq.dl_primitive = DL_BIND_REQ;
    bindreq.dl_sap = 0;
    bindreq.dl_max_conind = 1;
    bindreq.dl_service_mode = DL_CLDLS;
    bindreq.dl_conn_mgmt = 0;
    bindreq.dl_xidtest_flg = 0;
    ctl.maxlen = 0;
    ctl.len = sizeof(bindreq);
    ctl.buf = (char *)&bindreq;
    flags = 0;
    
	/* send bind req */
    putmsg(fd, &ctl, (struct strbuf *)NULL, flags);
    ctl.maxlen = sizeof(dlp);
    ctl.len = 0;
    ctl.buf = (char *)&dlp;
    
	/* get bind ack */
    getmsg(fd, &ctl, (struct strbuf*)NULL, &flags);
    promisconreq.dl_primitive = DL_PROMISCON_REQ;
    promisconreq.dl_level = DL_PROMISC_PHYS;
    ctl.maxlen = 0;
    ctl.len = sizeof(promisconreq);
    ctl.buf = (char *)&promisconreq;
    flags = 0;
    
	/* send promiscuous on req */
    putmsg(fd, &ctl, (struct strbuf *)NULL, flags);
    ctl.maxlen = sizeof(dlp);
    ctl.len = 0;
    ctl.buf = (char *)&dlp;
    
	/* get get ok ack */
    getmsg(fd, &ctl, (struct strbuf*)NULL, &flags);

    promisconreq.dl_primitive = DL_PROMISCON_REQ;
    promisconreq.dl_level = DL_PROMISC_SAP;
    ctl.maxlen = 0;
    ctl.len = sizeof(promisconreq);
    ctl.buf = (char *)&promisconreq;
    flags = 0;
    
	/* send promiscuous on req */
    putmsg(fd, &ctl, (struct strbuf *)NULL, flags);
    ctl.maxlen = sizeof(dlp);
    ctl.len = 0;
    ctl.buf = (char *)&dlp;

    /* get get ok ack */
    getmsg(fd, &ctl, (struct strbuf*)NULL, &flags);

    return fd;
}

int open_device_dlpi (char *dev_path)
{
	fd_set 	fds, rfds;
	int	fd[MAX_DEV_CNT], maxfd;
	int nbyte, i;
	int ret;
	struct timeval  cap_t, tm;
#ifdef PKT_MON_PRT
	time_t pre_t=0, cur_t=0;
#endif
	UCHAR 		*pNode;
	T_CAPHDR 	*pHead;
	UCHAR 		*pBuffer;
	char tmpStr[24];

	cap_t.tv_sec = 0; cap_t.tv_usec = 0;
	memset(tmpStr, 0x00, sizeof(char)*24);
	dAppLog(LOG_DEBUG, "[open_device_dlpi] DevCnt=%d", gdDevCnt);

	for (i=0 ; i<gdDevCnt ; i++)
	{
		sprintf(tmpStr, "/dev/%s", gszDevName[i]);
		fd[i] = dlpi_open (tmpStr);
		dAppLog(LOG_DEBUG, "[open_device_dlpi] DevName=%s FD[%d]=%d", gszDevName[i], i, fd[i]);
	}

	if (gdDevCnt == 1) {
		maxfd = fd[0];
	}
	else if (gdDevCnt == 2) {
		if (fd[0] > fd[1]) maxfd = fd[0];
		else maxfd = fd[1];
	}
	else {
		dAppLog(LOG_DEBUG, "CAP DEVICE COUNT is WRONG");
		return -1;
	}

	FD_ZERO(&fds);
	for (i=0 ; i<gdDevCnt ; i++)
	{
		FD_SET(fd[i], &fds);
	}
	tm.tv_sec = 0;
	tm.tv_usec = 1; // 20 -> 1
#ifdef PKT_MON_PRT
	pre_t = cur_t = time(NULL);
#endif	
	while (1)
	{
		keepalivelib_increase();
	
/* ADD  : BY JUNE, 2010-05-10
 * DESC : BUFFERING 시에 (Send_Node_Cnt <= Collection_Cnt) 의 경우 처리.
 */
#ifdef BUFFERING
#if 1
		nowTime = time(0); 
		if (oldTime + COLLECTION_TIME < nowTime) {
			//if (Send_Node_Cnt && Send_Node_Cnt == Diff_Node_Cnt) {
			if (Send_Node_Cnt && Send_Node_Cnt < Collection_Cnt) {
				//dPreCollection_Cnt = Collection_Cnt;
				// Send Buffring Packet 
				if((ret = dSend_CAPD_Data(pstMEMSINFO, dANAQid, NULL, 0)) < 0) {
					dAppLog(LOG_CRI, "dSend_CAPD_Data[%d] ERR[%s", ret, strerror(-ret));
				}
				//Collection_Cnt = dPreCollection_Cnt/2;
				Collection_Cnt = COLLECTION_MIN;
			}   
			//Diff_Node_Cnt = Send_Node_Cnt;
			oldTime = nowTime;
		}
#endif
#endif /* BUFFERING */

		rfds = fds;
		if(select(maxfd+1, &rfds, NULL, NULL, &tm)<=0) {
			continue;
		}

		for (i=0, nbyte=0; i < MAX_DEV_CNT; i++)
		{
			if(FD_ISSET(fd[i], &rfds)) {
				/* ADD : by june, 2010-10-13
				   이론상으론 MAX_RD_CNT 까지 읽고 switching을 하는 것이 맞으나 2개의 interface 로 
				   traffic을 나누어 시험할 수 없는 상황이라 일단은 주석처리해 놓았다.
				   하지만 성능을 향상 시키는 방법이므로 시험 후 적용 할 수 있도록 한다.
				 */
//				for (j=0; j<MAX_RD_CNT; j++)
//				{
#ifdef PKT_MON_PRT
					cur_t = time(NULL);
#endif
					if( (pNode = nifo_node_alloc(pstMEMSINFO)) == NULL) {
						dAppLog(LOG_CRI, "[%s.%d] nifo_node_alloc NULL", __FUNCTION__, __LINE__);
						exit(0);
					}
					if( (pHead = (T_CAPHDR *)nifo_tlv_alloc(pstMEMSINFO, pNode, CAP_HEADER_NUM, CAP_HDR_SIZE, DEF_MEMSET_OFF)) == NULL ) {
						dAppLog(LOG_CRI, "[%s.%d] nifo_tlv_alloc [%d] NULL", __FUNCTION__, __LINE__, CAP_HEADER_NUM);
						nifo_node_delete(pstMEMSINFO, pNode);
						exit(0);
					}
					if( (pBuffer = nifo_tlv_alloc(pstMEMSINFO, pNode, ETH_DATA_NUM, DLPI_RCV_BUF, DEF_MEMSET_OFF)) == NULL ) {
						dAppLog(LOG_CRI, "[%s.%d] nifo_tlv_alloc [%d] NULL", __FUNCTION__, __LINE__, ETH_DATA_NUM);
						nifo_node_delete(pstMEMSINFO, pNode);
						exit(0);
					}
					
					//nbyte = dlpi_read(fd[i], pBuffer+14, DLPI_RCV_BUF); /* 14 ether */ /* DLPI_RCV_BUF -> 1024 */
					nbyte = dlpi_read(fd[i], pBuffer, DLPI_RCV_BUF); /* jjinri 14 ether delete */
					if(nbyte>0) {
#ifdef PKT_MON_PRT
						RAD_PACKET_CNT[i]++;
#endif
						//pHead->datalen = nbyte + 14;
						pHead->datalen = nbyte;
						send_packet(pNode, pHead, pBuffer);
					}
					else {
						dAppLog(LOG_DEBUG, "DLPI READ INVALID [BYTE=%d", nbyte);
						nifo_node_delete(pstMEMSINFO, pNode);
						break;
					}
#if PKT_MON_PRT
					if (cur_t-pre_t >60) {
						dAppLog(LOG_CRI, "PKT FD[%d] CNT[%u] nbyte:%d", fd[i], RAD_PACKET_CNT[i], nbyte);
						pre_t = cur_t;
					}
#endif
//				} // for end, by june, 2010-10-13
			}
#if PKT_MON_PRT
			if (cur_t-pre_t >60) {
				dAppLog(LOG_CRI, "PKT FD[%d] CNT[%u] nbyte:%d", fd[i], RAD_PACKET_CNT[i], nbyte);
				pre_t = cur_t;
			}
#endif
		} // for end
	}
}


int close_device(void)
{
	return 0;
}
