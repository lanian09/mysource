#include <sys/shm.h>
#include <errno.h>
#include <pcap.h>

#include "capd_def.h"
#include <shmutil.h>
#include <utillib.h>

#include "mems.h"

#include "comm_util.h"


/* default snap length (maximum bytes per packet to capture) */
#define SNAP_LEN 		1518
#define MAX_ETH_CNT		4

/* TODO: 설정파일에서 읽어오는 부분 추가 */
int 	dCurDev;
int		dev_active_count = 1;
char	dev_no[MAX_ETH_CNT][12] = { "e1000g0", };

extern void got_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet);
extern void		keepalivelib_increase();

int open_device(char *dev_path)
{
    char 		errbuf[PCAP_ERRBUF_SIZE];	/* error buffer */
//    pcap_t 		*handle[MAX_ETH_CNT];		/* packet capture handle */
    pcap_t 		*handle;		/* packet capture handle */

    char 		filter_exp[] = "tcp port 33000";       /* filter expression [3] */

    struct bpf_program fp;          		/* compiled filter program (expression) */

    bpf_u_int32			mask;				/* subnet mask */
    bpf_u_int32			net;				/* ip */

	int					i, dRet=0;

	/* Setting pcap handle */
//	for(i=0; i<dev_active_count; i++) {

		/* get network number and mask associated with capture device */   
		if ( (dRet = pcap_lookupnet(dev_no[i], &net, &mask, errbuf)) == -1) {              
			dAppLog(LOG_CRI, "[dRet:%d] Couldn't get netmask for device %s:err:[%s]", \
					dRet,dev_no[i], errbuf);                                              
			net = 0;                                                       
			mask = 0;                                                      
			dAppLog(LOG_CRI,"pcap_lookupnet() fail. dRet:%d, dev[%s]:errbuf:%s\n",dRet,dev_no[i],errbuf);
		}

		/* print capture info */                                           
		dAppLog(LOG_CRI, "[DEVICE:%s][NET:%u][MASK:%u]", dev_no[i], net, mask); 

		/* open capture device */                                          
//		handle[i] = pcap_open_live(dev_no[i], SNAP_LEN, 1, 1000, errbuf);           
		handle = pcap_open_live(dev_no[i], SNAP_LEN, 1, 1000, errbuf);           
//		if (handle[i] == NULL) {                                              
		if (handle == NULL) {                                              
			dAppLog(LOG_CRI, "Couldn't open device %s: %s", dev_no[i], errbuf); 
			exit(EXIT_FAILURE);                                            
		}                                                                  

		/* make sure we're capturing on an Ethernet device [2] */          
//		if (pcap_datalink(handle[i]) != DLT_EN10MB) {                         
		if (pcap_datalink(handle) != DLT_EN10MB) {                         
			dAppLog(LOG_CRI, "%s is not an Ethernet", dev_no[i]);               
			exit(EXIT_FAILURE);                                            
		}

#ifdef FILTER
		/* compile the filter expression */
//		if ( (dRet=pcap_compile(handle[i], &fp, filter_exp, 0, net)) == -1) {
		if ( (dRet=pcap_compile(handle, &fp, filter_exp, 0, net)) == -1) {
			dAppLog(LOG_CRI, "Couldn't parse filter %s: %s", filter_exp, pcap_geterr(handle));
			exit(EXIT_FAILURE);
		}

		/* apply the compiled filter */
//		if (pcap_setfilter(handle[i], &fp) == -1) {
		if (pcap_setfilter(handle, &fp) == -1) {
			dAppLog(LOG_CRI, "Couldn't install filter %s: %s", filter_exp, pcap_geterr(handle));
			exit(EXIT_FAILURE);
		}
#endif
		pcap_setnonblock(handle, 1, errbuf);

//	}
	
	struct pcap_pkthdr 	*pkt_header;
	const u_char 		*pkt_data;


	while(1) 
	{
		keepalivelib_increase();
		dRet=pcap_next_ex(handle, &pkt_header, &pkt_data);

		if(pkt_header->caplen) 
		{
			dCurDev = i;
			got_packet(NULL, pkt_header, pkt_data);
			pkt_header->caplen = 0;
		} 

		usleep(1);
	}

    /* cleanup */                                                      
    pcap_freecode(&fp);                                                
//	for (i=0; i<dev_active_count; i++) {
	pcap_close(handle);
	dAppLog(LOG_DEBUG, "pcap_close(). Bye!");
//	}

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

#endif
	return 0;
}
