//#include <daginf.h>
//#include "dagapi.h"
//#include "dagutil.h"
#include <sys/shm.h>
#include <errno.h>

#include "capd_def.h"
#include <shmutil.h>
#include <utillib.h>

#include "mems.h"

#define OPTBUFSIZE ONE_KIBI
#define MAX_READ 1*ONE_MEBI

static char buffer[OPTBUFSIZE] = {0,};
static char dagname[DAGNAME_BUFSIZE];
static int flag_a, flag_b;

void        keepalivelib_increase();

time_t 					curTime, oldTime=0;
unsigned int    		Diff_Node_Cnt=0;
extern 	unsigned int    Collection_Cnt;
extern 	unsigned int    Send_Node_Cnt;

extern stMEMSINFO 		*pstMEMSINFO;
extern int 				dANAQid;

extern int dSend_CAPD_Data(stMEMSINFO *pstMEMSINFO, S32 dSndMsgQ, U8 *pNode, U32 sec);

int open_device(char *dagname_buf)
{
#if 0
	struct timeval maxwait;
	struct timeval poll;
	daginf_t *daginfo;

	dagutil_set_progname("DAG_CAPD");

	/* Set up default DAG device. */
	if (-1 == dag_parse_name(dagname_buf, dagname, DAGNAME_BUFSIZE, &dag_devnum))
	{
		dAppLog(LOG_CRI, "%s: FAIL[dag_parse_name] [%s]", __FUNCTION__, strerror(errno));
		exit(errno);
	}

	if((dag_fd = dag_open(dagname)) < 0)
	{
		dAppLog(LOG_CRI, "%s: FAIL[dag_open] [%s]", __FUNCTION__, strerror(errno));
		exit(errno);
	}

	/* No option configured now.*/
	buffer[0] = 0;
	if(dag_configure(dag_fd, buffer) < 0)
	{
		dAppLog(LOG_CRI, "%s: FAIL[dag_configure] [%s]", __FUNCTION__, strerror(errno));
		exit(errno);
	}

	if(dag_attach_stream(dag_fd, dag_devnum, 0, 0) < 0)
	{
		dAppLog(LOG_CRI, "%s: FAIL[dag_attach_stream] [%s]", __FUNCTION__, strerror(errno));
		exit(errno);
	}

	if(dag_start_stream(dag_fd, dag_devnum) < 0)
	{
		dAppLog(LOG_CRI, "%s: FAIL[dag_start_stream] [%s]", __FUNCTION__, strerror(errno));
		exit(errno);
	}

    /* Query the card first for special cases. */
    daginfo = dag_info(dag_fd);
    if ((0x4200 == daginfo->device_code) || (0x4230 == daginfo->device_code))
    {
        /* DAG 4.2S and 4.23S already strip the FCS. */
		/* Stripping the final word again truncates the packet. */
		/* (pcap-dag.c of libpcap) */
        fcs_bits = 0;
    }
	dAppLog(LOG_INFO, "DEVICE[%04x]: %s [FCS: %d]",
		daginfo->device_code, dag_device_name(daginfo->device_code, 1), fcs_bits);

	/*
	 * Initialise DAG Polling parameters.
	 */
	timerclear(&maxwait);
	maxwait.tv_usec = 100 * 1000; /* 100ms timeout */
	timerclear(&poll);
	poll.tv_usec = 10 * 1000; /* 10ms poll interval */

	/* 32kB minimum data to return */
	dag_set_stream_poll(dag_fd, dag_devnum, 32*ONE_KIBI, &maxwait, &poll);

#endif
	return 0;
}

int close_device(void)
{
#if 0
	if( dag_stop_stream(dag_fd, dag_devnum) < 0 )
		dagutil_panic("dag_stop_stream %s:%u: %s\n", dagname, dag_devnum, strerror(errno));

	if(dag_detach_stream(dag_fd, dag_devnum) < 0)
		dagutil_panic("dag_detach_stream %s:%u: %s\n", dagname, dag_devnum, strerror(errno));

	if( dag_close(dag_fd) < 0 )
		dagutil_panic("dag_close %s:%u: %s\n", dagname, dag_devnum, strerror(errno));

	return 0;
#endif
}

void do_packet_capture(void)
{
#if 0
	static uint8_t *bottom = NULL;
	static uint8_t *top = NULL;
	ULONG total_read, buffer_len, read_size, diff;
	int 			ret;

	/*
	 * The main Dag capture loop, wait for data and deliver.
	 */
	while( loop_continue )
	{
		keepalivelib_increase();

		flag_a = flag_b = 0;
		top = dag_advance_stream(dag_fd, dag_devnum, &bottom);
		if(top == NULL) {
			dAppLog(LOG_CRI, "%s: FAIL[dag_start_stream] [%s]", __FUNCTION__, strerror(errno));
			close_device();
			exit(errno);
		}
		diff = top - bottom;

		if( diff > 0 ) {
#if 0
			if( strcasecmp(syslabel, "BSDA") == 0 ) {
				SFMSysCommMsgType->loc_system_dup.mirr_a_status = 1;
			}
			else {
				SFMSysCommMsgType->loc_system_dup.mirr_b_status = 1;
			}
#endif

			/* read multiple packets */
			buffer_len = diff;
			total_read = 0;
			while( total_read < buffer_len ) {
				read_size = read_one_packet(&bottom[total_read], buffer_len);
				if( read_size < 0 ) {
					close_device();
					exit(-1);
				} 
				else if( read_size == 0 )
					break;

				total_read += read_size;
				buffer_len -= read_size;
			}
			bottom += total_read;

			if( flag_a==1 )
				SFMSysCommMsgType->loc_system_dup.mirr_a_status = 1;
			else
				SFMSysCommMsgType->loc_system_dup.mirr_a_status = 0;
			if( flag_b==1 )
				SFMSysCommMsgType->loc_system_dup.mirr_b_status = 1;
			else
				SFMSysCommMsgType->loc_system_dup.mirr_b_status = 0;
		}
		else {
#if 0
			if( strcasecmp(syslabel, "BSDA") == 0 ) {
				SFMSysCommMsgType->loc_system_dup.mirr_a_status = 0;
			}
			else {
				SFMSysCommMsgType->loc_system_dup.mirr_b_status = 0;
			}
#endif
			SFMSysCommMsgType->loc_system_dup.mirr_a_status = 0;
			SFMSysCommMsgType->loc_system_dup.mirr_b_status = 0;

#ifdef BUFFERING
			curTime = time(NULL);
			if(oldTime + 5 < curTime) {
				if(Send_Node_Cnt && Send_Node_Cnt == Diff_Node_Cnt) {
					// Send Buffring Packet 
					if((ret = dSend_CAPD_Data(pstMEMSINFO, dANAQid, NULL, 0)) < 0) {
						dAppLog(LOG_CRI, "[%s.%d] dSend_CAPD_Data [%d][%s]", __FUNCTION__, __LINE__, ret, strerror(-ret));
					}
					Collection_Cnt = 50;	// COLLECTIONCNT_MIN 으로 설정
				} 
				Diff_Node_Cnt = Send_Node_Cnt;
				oldTime = curTime;
			}
#endif /* BUFFERING */
		}
		//dAppLog(LOG_CRI, "RX PORT_A[%d] PORT_B[%d]",
			//SFMSysCommMsgType->loc_system_dup.mirr_a_status,
			//SFMSysCommMsgType->loc_system_dup.mirr_b_status);
	}
#endif
}

ULONG read_one_packet(void *buffer, ULONG bufferlen)
{
	int rlen;
	dag_record_t *cap_rec = (dag_record_t *)buffer;

	if( bufferlen < sizeof(dag_record_t)-1 ) {
		dAppLog(LOG_CRI, "SIZE CHECK 1");
		return 0;
	}

	/* Only ERF-timestamp is in Little-Endian. */
	/* All other fields are in Big-Endian. */
	rlen = ntohs(cap_rec->rlen);

	/* remained buffer size check */
	if( rlen > bufferlen ) {
		dAppLog(LOG_CRI, "SIZE CHECK 2");
		return 0;
	}

	switch( cap_rec->type ) 
	{
		case TYPE_ETH: /* Ethernet */
			handle_ethernet(cap_rec);
			break;

		case TYPE_LEGACY:
		case TYPE_HDLC_POS:
		case TYPE_ATM:
		case TYPE_AAL5:
		case TYPE_MC_HDLC:
		case TYPE_MC_RAW:
		case TYPE_MC_ATM:
		case TYPE_MC_RAW_CHANNEL:
		case TYPE_MC_AAL5:
		case TYPE_COLOR_HDLC_POS:
		case TYPE_COLOR_ETH:
			dAppLog(LOG_INFO, "NO NEED TO HANDLE: TYPE[%d]", cap_rec->type);
			break;
		default:
			dAppLog(LOG_WARN, "INVALID TYPE CHECK: TYPE[%d]", cap_rec->type);
			break;
	}

	return rlen;
}

int handle_ethernet(dag_record_t *cap_rec)
{
	int 			caplen, eth_len, rlen, port;
	struct timeval 	captime;
	eth_rec_t 		*eth_rec = &cap_rec->rec.eth;

	eth_len = ETHERNET_WLEN(cap_rec);
	caplen = ETHERNET_SLEN(cap_rec);
	rlen = ntohs(cap_rec->rlen);
	conv_ts2tv(cap_rec->ts, &captime);
	port = cap_rec->flags.iface; /* iface = 0, 1 */

	if( port==0 )
		flag_a = 1;
	else
		flag_b = 1;
/*
	dAppLog(LOG_INFO, "PORT[%d] RLEN[%4d] ETH_LEN[%4d] CAPLEN[%4d] CAPTIME[%ld.%06ld]",
		port, rlen, eth_len, caplen, captime.tv_sec, captime.tv_usec);
*/
	//log_hexa(eth_rec->dst, caplen);

	do_action(port, caplen, eth_rec->dst, &captime);

	return 0;
}

/* Convert ERF-timestamp to struct timeval. */
/* Refer to DAG Programming Guide. (page 40) */
void conv_ts2tv(uint64_t ts, struct timeval *tv)
{
	tv->tv_sec = (long)(ts >> 32);
	ts = ((ts & 0xffffffffULL) * 1000 * 1000);
	ts += ((ts & 0x80000000ULL) << 1);
	tv->tv_usec = (long)(ts >> 32);
	if( tv->tv_usec >= 1000000 )
	{
		tv->tv_usec -= 1000000;
		tv->tv_sec++;
	}
}

