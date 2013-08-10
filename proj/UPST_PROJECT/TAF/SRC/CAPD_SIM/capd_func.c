/*******************************************************************************
                DQMS Project

   Author   : Lee Dong-Hwan
   Section  : CAPD 
   SCCS ID  : @(#)capd_main.c (V1.0)
   Date     : 07/02/09
   Revision History :
        '09.    07. 02. initial

   Description :

   Copyright (c) uPRESTO 2005
*******************************************************************************/

/** A. FILE INCLUSION *********************************************************/
#include "capd_func.h"

/** B. DEFINITION OF NEW CONSTANTS ********************************************/
/** C. DEFINITION OF NEW TYPES ************************************************/
/** D. DECLARATION OF VARIABLES ***********************************************/
/** E.1 DEFINITION OF FUNCTIONS ***********************************************/
/** E.2 DEFINITION OF FUNCTIONS ***********************************************/
/*******************************************************************************

*******************************************************************************/
int open_device(char *dagname_buf)
{
    struct timeval  maxwait;
    struct timeval  poll;
    daginf_t        *daginfo;

    dagutil_set_progname("DAG_CAPD");

	/* Set up default DAG device. */
    if (-1 == dag_parse_name(dagname_buf, dagname, DAGNAME_BUFSIZE, &dag_devnum)) {
        log_print(LOGN_CRI, "%s: FAIL[dag_parse_name] [%s]", __FUNCTION__, strerror(errno));
        exit(errno);
    }

    if((dag_fd = dag_open(dagname)) < 0) {
        log_print(LOGN_CRI, "%s: FAIL[dag_open] [%s]", __FUNCTION__, strerror(errno));
        exit(errno);
    }

    /* No option configured now.*/
    buffer[0] = 0;
    if(dag_configure(dag_fd, buffer) < 0) {
        log_print(LOGN_CRI, "%s: FAIL[dag_configure] [%s]", __FUNCTION__, strerror(errno));
        exit(errno);
    }

    if(dag_attach_stream(dag_fd, dag_devnum, 0, 0) < 0) {
        log_print(LOGN_CRI, "%s: FAIL[dag_attach_stream] [%s]", __FUNCTION__, strerror(errno));
        exit(errno);
    }

    if(dag_start_stream(dag_fd, dag_devnum) < 0) {
        log_print(LOGN_CRI, "%s: FAIL[dag_start_stream] [%s]", __FUNCTION__, strerror(errno));
        exit(errno);
    }

    /* Query the card first for special cases. */
    daginfo = dag_info(dag_fd);
    if ((0x4200 == daginfo->device_code) || (0x4230 == daginfo->device_code)) {
        /* DAG 4.2S and 4.23S already strip the FCS. */
        /* Stripping the final word again truncates the packet. */
        /* (pcap-dag.c of libpcap) */
        fcs_bits = 0;
    }
    log_print(LOGN_INFO, "DEVICE[%04x]: %s [FCS: %d]",
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

    log_print( LOGN_DEBUG, "dag_devnum:%d", dag_devnum );

    return 0;
}


/*******************************************************************************

*******************************************************************************/
int close_device(void)
{
    if( dag_stop_stream(dag_fd, dag_devnum) < 0 )
        dagutil_panic("dag_stop_stream %s:%u: %s\n", dagname, dag_devnum, strerror(errno));

    if(dag_detach_stream(dag_fd, dag_devnum) < 0)
        dagutil_panic("dag_detach_stream %s:%u: %s\n", dagname, dag_devnum, strerror(errno));

    if( dag_close(dag_fd) < 0 )
        dagutil_panic("dag_close %s:%u: %s\n", dagname, dag_devnum, strerror(errno));

    return 0;
}


/*******************************************************************************

*******************************************************************************/
void do_packet_capture(void)
{
    int 				ret;
    static uint8_t 		*bottom = NULL;
    static uint8_t 		*top = NULL;
    ULONG total_read, 	buffer_len, read_size, diff;

    /*
     * The main Dag capture loop, wait for data and deliver.
     */
    while( loop_continue )
    {
        top = dag_advance_stream(dag_fd, dag_devnum, &bottom);
        if(top == NULL) {
            dagutil_panic("dag_advance_stream %s:%u: %s\n", dagname, dag_devnum, strerror(errno));
            log_print(LOGN_CRI, "%s: FAIL[dag_start_stream] [%s]", __FUNCTION__, strerror(errno));
            close_device();
            exit(errno);
        }

        diff = top - bottom; 
        if( diff > 0 ) {   
            /* read multiple packets */
            buffer_len = diff;
            total_read = 0;
            while( total_read < diff )
            {
                read_size = read_one_packet(&bottom[total_read], buffer_len, 0 );
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
        }
		else {
			curTime = time(NULL);
            if(oldTime + 5 < curTime) {
                if(Send_Node_Cnt && Send_Node_Cnt == Diff_Node_Cnt) {
                    /* Send Buffring Packet  */
                    //if((ret = dSend_CAPD_Data(pstMEMSINFO, myqid, NULL, 0)) < 0) {
                    if((ret = dSend_CAPD_Data(pstMEMSINFO, NULL, 0)) < 0) {
                        log_print(LOGN_CRI, "[%s.%d] dSend_CAPD_Data [%d][%s]", __FUNCTION__, __LINE__, ret, strerror(-ret));
                    }
                    Collection_Cnt = 50;    // COLLECTIONCNT_MIN 으로 설정
                }
                Diff_Node_Cnt = Send_Node_Cnt;
                oldTime = curTime;
            }
		}

		/* CHECK ACTIVE & STANDBY STATUS */
		curTime = time(NULL);
		if( chkTime != curTime ) {

			if( stPortStatus[0].uiLastCnt == stPortStatus[0].uiCurrCnt ) {
				stPortStatus[0].uiRetryCnt++;

				if( stPortStatus[0].uiRetryCnt > 2 ) {
					fidb->mirrorsts[0] = CRITICAL;

					if( fidb->mirrorActsts[0] == DEF_ACTIVE && fidb->mirrorsts[1] != CRITICAL ) {
						fidb->mirrorActsts[0] = DEF_STANDBY;
						fidb->mirrorActsts[1] = DEF_ACTIVE;

						log_print( LOGN_CRI, "CHANGE ACTIVE 0 -> 1..");
					}
				}
			}
			else {
				fidb->mirrorsts[0] = NORMAL;
				stPortStatus[0].uiRetryCnt = 0;
				stPortStatus[0].uiLastCnt = stPortStatus[0].uiCurrCnt;
			}

			if( stPortStatus[1].uiLastCnt == stPortStatus[1].uiCurrCnt ) {
                stPortStatus[1].uiRetryCnt++;

                if( stPortStatus[1].uiRetryCnt > 2 ) {
                    fidb->mirrorsts[1] = CRITICAL;

                    if( fidb->mirrorActsts[1] == DEF_ACTIVE && fidb->mirrorsts[0] != CRITICAL ) {
                        fidb->mirrorActsts[1] = DEF_STANDBY;
                        fidb->mirrorActsts[0] = DEF_ACTIVE;
	
						log_print( LOGN_CRI, "CHANGE ACTIVE 1 -> 0..");
                    }
                }
            }
            else {
                fidb->mirrorsts[1] = NORMAL;
                stPortStatus[1].uiRetryCnt = 0;
                stPortStatus[1].uiLastCnt = stPortStatus[1].uiCurrCnt;
            }	

			chkTime = curTime;

			log_print( LOGN_DEBUG, "[0] STS:0X%02X ACT:%u [1] STS:0X%02X ACT:%u",
								fidb->mirrorsts[0], fidb->mirrorActsts[0], fidb->mirrorsts[1], fidb->mirrorActsts[1] ); 
		}
    }
}

/*******************************************************************************

*******************************************************************************/
ULONG read_one_packet(void *buffer, ULONG bufferlen, int dDagNum )
{
    dag_record_t *cap_rec = (dag_record_t *)buffer;
    int caplen;

    if( bufferlen < sizeof(dag_record_t)-1 )
        return 0;

    /* Only ERF-timestamp is in Little-Endian. */
    /* All other fields are in Big-Endian. */
    caplen = ntohs(cap_rec->rlen);

    /* remained buffer size check */
    if( caplen > bufferlen )
        return 0;

    switch( cap_rec->type )
    {
        case TYPE_ETH: /* Ethernet */
            handle_ethernet(cap_rec, dDagNum);
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
            log_print(LOGN_INFO, "NO NEED TO HANDLE: TYPE[%d]", cap_rec->type);
            break;

        default:
            log_print(LOGN_WARN, "INVALID TYPE CHECK: TYPE[%d]", cap_rec->type);
            break;
    }

    return caplen;
}

/*******************************************************************************

*******************************************************************************/
int handle_ethernet(dag_record_t *cap_rec, int dDagNum )
{
    int caplen, eth_len, rlen, port, dRet;
    struct timeval captime;
    eth_rec_t *eth_rec = &cap_rec->rec.eth;

    eth_len = ETHERNET_WLEN(cap_rec);
    caplen = ETHERNET_SLEN(cap_rec);
    rlen = ntohs(cap_rec->rlen);
    conv_ts2tv(cap_rec->ts, &captime);
    port = cap_rec->flags.iface+1; /* iface = 0, 1 */

    /* FOR STAT 
    llCount++;
    llBytes += caplen;
    */
	stPortStatus[port-1].uiCurrCnt++;

	if( fidb->mirrorActsts[port-1] == DEF_STANDBY )
		return 1;

    log_print(LOGN_INFO, "DAG_NUM[%02d] RLEN[%d] ETH_LEN[%d] CAPLEN[%d] CAPTIME[%ld.%06ld] PORT[%d]",
        dDagNum, rlen, eth_len, caplen, captime.tv_sec, captime.tv_usec, cap_rec->flags.iface);

    dRet = do_action(port, caplen, eth_rec->dst, &captime);
	if( dRet < 0 ) {
		log_print( LOGN_INFO, "[%s.%d] ERROR IN do_action dRet:%d", __FUNCTION__, __LINE__, dRet );
		return -1;
	}

    return 0;
}


/*******************************************************************************

*******************************************************************************/
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

#define WIDTH   16
int dump_DebugString(char *debug_str, char *s, int len)
{
	char buf[BUFSIZ],lbuf[BUFSIZ],rbuf[BUFSIZ];
	unsigned char *p;
	int line,i;

	log_print(LOGN_DEBUG,"### %s",debug_str);
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
		log_print(LOGN_DEBUG,"%04x: %-*s    %s",line - 1,WIDTH * 3,lbuf,rbuf);
	}
	return line;
}

/*******************************************************************************

*******************************************************************************/
int do_action(int port, int len, char *data, struct timeval *tmv)
{
    int             ret;
//	T_CAPHDR        *pstCAPHead;
    Capture_Header_Msg        *pstCAPHead;

    if(len > 1518)  {   /* ethernet packet size */
        log_print(LOGN_CRI, "Ethernet Packet Size Error, input length =%d", len);
    }

    /* ADD BY YOON 2008.10.14 */
    if( (pstNode = nifo_node_alloc(pstMEMSINFO)) == NULL) {
        log_print( LOGN_WARN, "[%s.%d] nifo_node_alloc NULL", __FUNCTION__, __LINE__);
		usleep(1);
		return -1;
    }

    if( (pstCAPHead = (Capture_Header_Msg *)nifo_tlv_alloc(pstMEMSINFO, pstNode, CAP_HEADER_NUM, CAP_HDR_SIZE, DEF_MEMSET_OFF)) == NULL ) {
        log_print( LOGN_CRI, "[%s.%d] nifo_tlv_alloc [%d] NULL", __FUNCTION__, __LINE__, CAP_HEADER_NUM);
		nifo_node_delete(pstMEMSINFO, pstNode);
		return -2;
    }

    if( (pstBuffer = nifo_tlv_alloc(pstMEMSINFO, pstNode, ETH_DATA_NUM, len, DEF_MEMSET_OFF)) == NULL ) {
        log_print( LOGN_CRI, "[%s.%d] nifo_tlv_alloc [%d] NULL", __FUNCTION__, __LINE__, ETH_DATA_NUM);
		nifo_node_delete(pstMEMSINFO, pstNode);
		return -3;
    }

    pstCAPHead->bRtxType 	= port;
    pstCAPHead->curtime 	= tmv->tv_sec;
    pstCAPHead->ucurtime 	= tmv->tv_usec;
    pstCAPHead->datalen 	= len;

    memcpy(pstBuffer, data, pstCAPHead->datalen);

    //if((ret = dSend_CAPD_Data(pstMEMSINFO, myqid, pstNode, pstCAPHead->curtime)) < 0) {
    if((ret = dSend_CAPD_Data(pstMEMSINFO, pstNode, pstCAPHead->curtime)) < 0) {
        log_print( LOGN_CRI, "[%s.%d] dSend_CAPD_Data [%d][%s]", __FUNCTION__, __LINE__, ret, strerror(-ret));
		nifo_node_delete(pstMEMSINFO, pstNode);
		return -4;
    }

	return 1;

} /**** end of do_action ******/


/*******************************************************************************

*******************************************************************************/
//int dSend_CAPD_Data(stMEMSINFO *pstMEMSINFO, S32 dSndMsgQ, U8 *pNode, U32 sec)
int dSend_CAPD_Data(stMEMSINFO *pstMEMSINFO, U8 *pNode, U32 sec)
{
	OFFSET offset;

#ifdef MEM_TEST
    nifo_node_delete(pstMEMSINFO, pNode);
#else
    /*
     * SIGTERM, SIGUSR1 시그널을 받은 경우에는 
     * 버퍼링하고 있는 패킷을 모두 전송 한 후에 종료 한다. 
     */
    if(pNode == NULL) {
        Collection_Cnt = 0;
    } else {
        if(Send_Node_Head) {
            nifo_node_link_nont_prev(pstMEMSINFO, Send_Node_Head, pNode);
        } else {
            Send_Node_Head = pNode;
        }
        Send_Node_Cnt++;
        check_pkt++;
    }

    if(Send_Node_Cnt > Collection_Cnt)
	{
		offset = nifo_offset(pstMEMSINFO, pNode);
		if(gifo_write(pstMEMSINFO, pstCIFO, guiSeqProcID, guiSeqProcID, offset) < 0)
		{
			log_print(LOGN_CRI, "[ERROR] gifo_write(from=%d:MMCD, to=%d:MMCD), offset=%d",
								SEQ_PROC_MMCD, SEQ_PROC_MMCD, offset);
			nifo_node_delete(pstMEMSINFO, pNode);
			usleep(0);
		}
		else
		{
			if(pNode == NULL)
			{
				log_print(LOGN_CRI, LH"### Send Buffered Packet[%d]", Send_Node_Cnt);				
			}

			Send_Node_Cnt = 0;
			Send_Node_Head = NULL;
		}
		/*
        UINT             dRet;

        if((dRet = nifo_msg_write(pstMEMSINFO, dSndMsgQ, Send_Node_Head)) < 0) {
            log_print(LOGN_CRI, "[%s][%s.%d] nifo_msg_write dRet[%d][%s]", __FILE__, __FUNCTION__, __LINE__, dRet, strerror(-dRet));
            return -1;
        } else {
            if(pNode == NULL)
                log_print(LOGN_CRI, "### Send Buffered Packet[%d]", Send_Node_Cnt);

            Send_Node_Cnt = 0;
            Send_Node_Head = NULL;
        }
		*/
    }
	

    /*
     * 패킷이 들어오는 속도에 따라 버퍼링의 개수를 조절한다. 
     */
#ifdef BUFFERING
    if( (old_time + COLLECTION_TIME) < sec) {
        if( (check_pkt / COLLECTION_TIME) >  (Collection_Cnt * COLLECTION_MULTIPLY) ) {
            Collection_Cnt *= COLLECTION_MULTIPLY;
            if(Collection_Cnt > COLLECTION_MAX) {
                Collection_Cnt = COLLECTION_MAX;
            }
        } else if( (check_pkt / COLLECTION_TIME) < (Collection_Cnt / COLLECTION_MULTIPLY) ) {
            Collection_Cnt /= COLLECTION_MULTIPLY;
            if(Collection_Cnt < COLLECTION_MIN) {
                Collection_Cnt = COLLECTION_MIN;
            }
        }
        log_print(LOGN_INFO, "CHECK_PKT:%d COLLECTION_CNT:%d", check_pkt, Collection_Cnt);
        check_pkt = 0;
        old_time = sec;
    }
#endif /* BUFFERING */

#endif
    return 0;
}
