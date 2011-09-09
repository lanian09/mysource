/*
 * $Id $
 */

/* dagconvert headers. */
#include "inputs.h"
#include "outputs.h"
#include "utils.h"

/* Endace headers. */
#include "dagapi.h"
#include "dagutil.h"

/* libpcap headers. */
#include "pcap.h"


#ifndef O_LARGEFILE
#define O_LARGEFILE 0
#endif

#define PCAP_FILE_HDR_SIZE  24
#define PCAP_DATA_HDR_SIZE  16
#define MAX_SIZE			1500

/*
 * Filename for output, use "-" or NULL for stdout 
 */
static char    *outfilename;

/*
 * Structure to use in writing pcap records 
 */
static void    *dumper = NULL;

/*
 * Reuseable memory for pcap packet header 
 */
static struct pcap_pkthdr pkthdr;
unsigned char cap_data[MAX_SIZE];

/*
 * Snap length to use 
 */
static int      snap_length = NO_SNAP_LIMIT;

/*
 * Bytes written to file so far, and file rotation sequence
 */
static uint32_t	bytes_written = 0;
static uint32_t	rotate_num = 0;

/*
 * Prepare the output stream for writing pcap style records.
 */
static void*
prepare_output(dag_record_t * erf, void *payload, char *out)
{
    int linktype = 0;
    int snapshot = 0;
    pcap_t *p;
    pcap_dumper_t *dumper;
    char *oname = NULL;

	switch (erf->type) {
		case TYPE_ATM:
			linktype = DLT_ATM_RFC1483;
			snapshot = ATM_SNAPLEN;
			break;
		
		case TYPE_ETH:
		case TYPE_COLOR_ETH:
			linktype = DLT_EN10MB;
			snapshot = DEFAULT_SNAPLEN;
			break;
		
		case TYPE_HDLC_POS:
		case TYPE_COLOR_HDLC_POS:
			if (ntohs(*(uint16_t *)payload) == 0xff03)
			{
				linktype = DLT_PPP_SERIAL;
			}
			else
			{
				linktype = DLT_CHDLC;
			}
			snapshot = DEFAULT_SNAPLEN;
			break;
		
		default:
			dagutil_panic("internal error %s line %d\n", __FILE__, __LINE__);
	}

	/* override length with default for variable length */
	if(erf->flags.vlen)
		snapshot = DEFAULT_SNAPLEN;

	/* override default length with requested length */
	if(snap_length != NO_SNAP_LIMIT) {
#ifdef PEDANTIC
		if(erf->type == TYPE_ATM
		   && snap_length != ATM_SNAPLEN)
			dagutil_warning("Attempt to alter snap_length of ATM traffic.\n");
#endif
		snapshot = snap_length;
	}

        if ((p = pcap_open_dead(linktype, snapshot)) == NULL) {
            return NULL;
        }

	if (rotate_size) {
		oname = calloc(1, strlen(out)+15);
		snprintf(oname, strlen(out)+15, "%s%04u.pcap", out, rotate_num);
		dagutil_verbose("Set output file to %s\n", oname);
		dumper = pcap_dump_open(p, oname);
		free(oname);
		bytes_written += sizeof(struct pcap_file_header);
	} else
		dumper = pcap_dump_open(p, out);

        pcap_close(p);

        return dumper;
}

/*
 * Set the stream on which output is to be produced.
 */
void
set_pcap_output(char *out)
{
	/*
	 * cannot establish output stream until such time
	 * as we know what the type of data is
	 */
	outfilename = out;
	dumper = NULL;
}

/*
 * Manually set the snap length. If this routine is not
 * called, then a value is assumed based on the length
 * of the first packet saved.
 */
void
set_pcap_snap_length(int snap)
{
	snap_length = snap;
}

/*
 * Close the currently open input (if any).
 */
void
close_pcap_output()
{

	if(dumper)
		pcap_dump_close(dumper);
	dumper = NULL;
}

/*
 * Write given PCAP header and payload in PCAP format.
 * Returns 1 if record successfully written and 0
 * otherwise.
 */
int
#ifndef _WIN32
write_pcap_record(dag_record_t * header, void *payload)
#else /* _WIN32 */
write_pcap_record(dag_record_t * header, char *payload)
#endif /* WIN32 */
{
	register uint64_t ts;

#ifdef PEDANTIC
	if(header == NULL) {
		dagutil_warning("Null record passed to write_pcap_record\n");
		return 0;
	}
	if(payload == NULL) {
		dagutil_warning("Null payload passed to write_pcap_record\n");
		return 0;
	}
#endif

	if(!dumper) {
		/* initiate pcap output with appropriate type */
		dumper =
		    prepare_output(header, payload, outfilename == NULL ? "-" : outfilename);
		if(dumper == NULL)
			dagutil_panic("Could not open %s for writing.\n", outfilename);
	}
	/*
	 * convert between timestamp formats, this used to
	 * swap in situ, but now makes local copy
	 */
	ts = swapll(header->ts);
	//ts = header->ts;
	pkthdr.ts.tv_sec = (long)(ts >> 32);
	ts = ((ts & 0xffffffffULL) * 1000 * 1000);
	ts += (ts & 0x80000000ULL) << 1;	/* rounding */
	pkthdr.ts.tv_usec = (long)(ts >> 32);
	if(pkthdr.ts.tv_usec >= 1000000)
	{
		pkthdr.ts.tv_usec -= 1000000;
		pkthdr.ts.tv_sec += 1;
	}

	/* dump packet according to type */
	switch (header->type)
	{
		case TYPE_ATM:
			pkthdr.caplen = ATM_SLEN(header);
			pkthdr.len = ATM_WLEN(header);
			/* skip ATM header */
			pcap_dump(dumper, &pkthdr, payload + 4);
			break;
		
		case TYPE_ETH:
		case TYPE_COLOR_ETH:
			pkthdr.caplen = ETHERNET_SLEN(header);
			pkthdr.len = ETHERNET_WLEN(header);
			/* skip offset and pad bytes from Ethernet capture */
			pcap_dump(dumper, &pkthdr, payload + 2);
			break;
		
		case TYPE_HDLC_POS:
		case TYPE_COLOR_HDLC_POS:
			pkthdr.caplen = HDLC_SLEN(header);
			pkthdr.len = HDLC_WLEN(header);
			pcap_dump(dumper, &pkthdr, payload);
			break;
		
		default:
			dagutil_panic("unknown ERF record type %d\n", header->type);
	}
	
	if (rotate_size) {
		/* XXX opaque sizeof(struct pcap_sf_pkthdr) = 16 today */
		bytes_written += (pkthdr.caplen + 16);
		if ((int)bytes_written >= rotate_size) {
			rotate_num++;
			bytes_written = 0;
			close_pcap_output();
		}
	}

	return 1;
}

/*
 * Input stream 
 */
static FILE    *infile = NULL;
static dag_record_t		cap_record;

/*
 * Reusable memory for records 
 * static dag_record_t _record;
 */
/*
 * Next payload 
 */
#ifndef _WIN32
static void    *payload = NULL;
#else /* _WIN32 */
static char    *payload = NULL;
#endif /* _WIN32 */
/*
 * Set the stream from which input is obtained.
 */
void
set_pcap_input(char *in)
{
	int fd;
#if 0
	fprintf (stderr,"set_pcap_input() input file name [%s] \n",in);
#endif
	if(in == NULL)
		infile = stdin;
	else
	{
		fd = open(in, O_RDONLY|O_LARGEFILE);
		if (fd == -1) {
			dagutil_panic("Could not open %s for reading.\n", in);
		}
		infile = fdopen(fd, "r");
		fprintf(stderr,"[%s] fopen success....\n",__FUNCTION__);
		if (infile == NULL) {
			dagutil_panic("Could not open %s for reading.\n", in);
		}
	}
}

/*
 * Close the current input stream.
 */
void
close_pcap_input()
{
	if(infile != NULL && infile != stdin)
		fclose(infile);
	infile = NULL;
}

/*
 * Get pointer to the ERF header for the next packet
 * in the input stream. Returns null if no further
 * packets are available.
 */
dag_record_t   *
get_next_pcap_header()
{
	dagutil_panic("Reading PCAP records is not implemented yet.\n");
	return NULL;
}




dag_record_t   *
get_next_cap_header(void)
{
    int rlen;
/*
	struct data_hdr_t	*cap_header;
*/
/*
	int				cap_time;
	int				cap_utime;
*/


#if 0
    fprintf (stderr, "================ get_next_cap_header : g_first %d=================== \n",g_first);
#endif
    if(infile == NULL) { 
        dagutil_warning("Null input stream.\n");
        return NULL;
    }

    if(feof(infile))    {
        g_first = 0;
        return NULL;
    }

    if(g_first == 1)    { 
        /* read the header into reusuable memory */
        if(fread(&cap_data, PCAP_FILE_HDR_SIZE, 1, infile) != 1) { 
            return NULL;
        }       
        g_first = 0;
        fprintf (stderr, "read Success -----------> etherreal or pacp  header  g_first [%d] \n",g_first);
    }

    memset(cap_data, 0x00, MAX_SIZE);
    if(fread(&cap_data, PCAP_DATA_HDR_SIZE, 1, infile) != 1) { 
        return NULL;
    }
#if 0
    fprintf (stderr, "read Success -----------> data header  \n");
#endif
    
    rlen = (((data_hdr_t *)(cap_data))->dwLen);
	cap_record.rlen = htons(rlen);
/*
	cap_record.ts =  (((data_hdr_t *)(cap_data))->dwuSec);
	cap_record.ts = cap_record.ts<<32;
	cap_record.ts =  (((data_hdr_t *)(cap_data))->dwSec);
*/
/*
	cap_utime =  (((data_hdr_t *)(cap_data))->dwuSec);
	cap_record.ts = htonl(cap_time);
	cap_record.ts = cap_record.ts << 32;
	cap_time =  (((data_hdr_t *)(cap_data))->dwSec);
	cap_record.ts = htonl(cap_time);
*/
	cap_record.ts = (((data_hdr_t *)(cap_data))->dwTime);
		


#if 0
    fprintf(stderr, "========   rlen [%d] cap_record.rlen[%d]========\n",rlen,htons(cap_record.rlen));
#endif

        
    /* allocate memory for and read the payload */
    if(rlen == 0) { 
        /*      
         * In this case there is no payload, but we still want
         * to return a non-null pointer. Therefore, if _payload
         * is not yet allocate, get a single byte of memory
         */
        fprintf(stderr, "rlen == 0  \n");
        if(payload == NULL) {
            payload = malloc(1);
        }
    } else {
        /*
         * In the usual case, simply resize the _payload buffer
         * to be large enough for the data
         */
        payload = realloc(payload, rlen);
        if(payload == NULL) {
            dagutil_panic("Could not get memory for payload.\n");
        }
        if(fread(payload, rlen,1, infile) < 0) {
            dagutil_warning("Problem with payload.\n");
            fprintf(stderr, "fread error : Raw Data  \n");
            return NULL;
        }
#if 0
        fprintf(stderr, "fread sucess: Raw Data  rlen[%d] \n",rlen);
#endif
    }
    return &cap_record;
}


/*
 * Returns a pointer to the payload of the most recent
 * packet. Returns null if there is no current packet.
 */
void           *
get_pcap_payload()
{
	/* dagutil_panic("Reading PCAP records is not implemented yet.\n"); */
	return payload;
}

/*
*	$Log $
*/
