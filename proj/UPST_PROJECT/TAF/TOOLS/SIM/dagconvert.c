/*
 * Copyright (c) 2002-2005 Endace Technology Ltd, Hamilton, New Zealand.
 * All rights reserved.
 *
 * This source code is proprietary to Endace Technology Limited and no part
 * of it may be redistributed, published or disclosed except as outlined in
 * the written contract supplied with this product.
 *
 * $Id: dagconvert.c,v 1.1.1.1 2011/08/29 05:56:42 dcham Exp $
 *
 * Takes network traffic traces of various formats, allows filtering and
 * packet manipulations to be performed and writes modified traces out in
 * various formats.
 */

/* dagconvert headers. */
#include "utils.h"
#include "inputs.h"
#include "outputs.h"

/* Endace headers. */
#include "dagapi.h"
#include "dagutil.h"

#include <netinet/ip.h>

/* libpcap header. */
#include <pcap.h> /* needed to compile BPF filter	*/


/* CVS Header */
static const char* const kCvsHeader = "$Id: dagconvert.c,v 1.1.1.1 2011/08/29 05:56:42 dcham Exp $";
static const char* const kRevisionString = "$Revision: 1.1.1.1 $";


#define 	NIPADDR(d) 		(d&0xff),((d>>8)&0xff),((d>>16)&0xff),((d>>24)&0xff)

/* CJLEE */
int g_nte = 0 ;
int g_mn = 0 ;
int g_first_packet = 0 ;
unsigned int  g_first_packet_time = 0 ;
unsigned int  g_first_system_time = 0 ;
unsigned int g_gap;
int 				g_sendsockfd;
struct sockaddr_in  g_servaddr;
unsigned short	g_port = 0;

/* SHJEON	*/
int	g_vpi = -1;
int	g_vci = -1;

long long	g_sendCnt = 0;

#pragma pack(1)
struct ether_erf_header {
	uint32_t            sec;
	uint32_t           usec;
	uint8_t             type;
	flags_t             flags;
	uint16_t            rlen;
	uint16_t            lctr;
	uint16_t            wlen;
	uint16_t            pad;
}erf_hdr;

struct sim_header {
	uint32_t            sec;
	uint32_t           usec;
	uint32_t             direction;
}sim_hdr;


struct	mac_hdr	{
	char				s_mac[6];
	char				d_mac[6];
	uint16_t 			mac_type;
}machdr;

struct imsi_hdr	{
	uint16_t 	 		imsi_len;
	char 				imsi[16];
};
#pragma pack()



/*
 * enumerates the various I/O types currently supported 
 */
enum types {
	NONE = 0,
	ERF,
	DAG,
	PCAP,
	PRT,
	ATM,
	ETH,
	POS,
	NUL
};

/*
 * types used to determine which io functions to call 
 */
static enum types input_type;
static enum types output_type;

/*
 * time is seconds to spend capturing data - default (<= 0) is unlimited 
 */
static int      capture_time;

/*
 * io device specific variables 
 */
static int      snap_length;
static int      variable_length;
static int      record_alignment;

/* Exported via inputs.h. */
int rotate_size = 0;
int fcs_bits = 32;
int erf_output_iface = -1;
int g_first = 1;

/*
 * the arguments passed to the DAG card 
 */
static char    *extra_args;

/*
 * which ERF errors should be filtered 
 */
static uint8_t filter_options = 0;

/*
 * masks for bit positions in ERF flags byte 
 */
#define FILTER_TRUNC_MASK 0x08
#define FILTER_RX_MASK 0x10
#define FILTER_DS_MASK 0x20

/*
 * which interfaces to accept packets from 
 */
static int interface_permit = 0;

/*
 * each interface is a bit in _interface_permit 
 */
#define INTERFACE_A 1
#define INTERFACE_B 2
#define INTERFACE_C 4
#define INTERFACE_D 8

/*
 * BPF filter 
 */
static struct bpf_insn *bpf_insn = NULL;
static char* filter_string = NULL;

/*
 * counter variables used to hold statistics by the report function 
 */
static uint64_t in_count;
static uint64_t out_count;
static uint64_t byte_count;

unsigned int X_send_cnt = 0;	/* Start # */
unsigned int Z_send_cnt = 0xffffffff;	 /* End # */

static char usage_text[] =
"dagconvert - Endace DAG file conversion utility.\n"
"Usage: dagconvert [options]\n"
"    -d <device>            DAG device name\n"
"    -h,--help,--usage      display help (this page)\n"
"    -v,--verbose           increase verbosity\n"
"    --version              display version information\n"
"    -i <filename>          input file\n"
"    -o <filename>          output file\n"
"    -r N[k|m|g]            change output file after N Bytes.\n"
"                           k, m, g suffixes for kilobytes, megabytes, gigabytes.\n"
"    -s <snaplen>           output snap length\n"
"    -t <seconds>           capture period in seconds\n"
"    -T <in_type:out_type>  input and output types (see list of types below)\n"
"    -A <int>               output record alignment (ERF only)\n"
"    -V                     select variable length output (ERF only)\n"
"    -F                     select fixed length output (ERF only)\n"
"    -G                     specify GMT offset in seconds (pcap only)\n"
"    -c 0|16|32             specify number of bits in FCS checksum (pcap only)\n"
"    -f <list>              comma separated list of filters (see list of filters below)\n"
"    -b <BPF>               specify a BPF style filter\n"
"    -p 0|1|2|3             specify an interface to write into output ERF records\n"
"    -N  IPADDR(string)     전송하고자 하는 호스트 IP (NTE) #\n"
"    -M  <int>              Mobile Terminator처럼 현재 보낸 시간으로 동작   1:현재 시간으로 변경 , 2 : 원래 시간\n"
"    -P  <int>              전송하고자 하는 호스트 IP PORT (NTE) #\n"
"    -U  <int>              VCI #\n"
"    -W  <int>              VPI #\n"
"    -X  <int>              Start Record #\n"
"    -Z  <int>              End Record #\n"
"사용 예 1\n"
"	./dagconvert -i test.cap -o a.cap -T PCAP:ERF -X 1 -Z 10 -N 172.1.1.1 -P 2006 \n"
"사용 예 2 : 단말에서 보내는 모양 처럼 (초간격으로 보냄) : 현재 시간으로 변경 \n"
"	./dagconvert -i test.cap -o a.cap -T PCAP:ERF -X 1 -Z 100 -N 172.1.1.1 -P 2006  -M 1\n"
"struct sim_header {					\n"
"	uint32_t            sec;       	 	\n"
"	uint32_t           usec;			\n"
"	uint32_t             direction;		\n"
"}sim_hdr;								\n"
" direction : up 1 , down 2 			\n"

"\n"
"Supported types:\n"
"    dag    ERF direct from DAG device (input only)\n"
"    erf    ERF (extensible record format) file\n"
"    atm    legacy ATM file (input only)\n"
"    eth    legacy Ethernet file (input only)\n"
"    pos    legacy PoS file (input only)\n"
"    null   produces no input or output\n"
"    pcap   libpcap format file (output only)\n"
"    prt    ASCII text packet dump (output only)\n"
"\n"
"Supported filters:\n"
"    rx       filter out rx errors (link layer)\n"
"    ds       filter out ds errors (framing)\n"
"    trunc    filter out truncated packets\n"
"    a,b,c,d  filter on indicated interface(s)\n"
;

/*
 * used by signal handler when ctrl-C is detected 
 */

#if defined(_WIN32)
void close_devices(void);
#endif /* _WIN32 */

static jmp_buf jmpbuf;
static void
anysig(int sig)
{
#if defined(__FreeBSD__) || defined(__linux__) || (defined(__SVR4) && defined(__sun)) || (defined(__APPLE__) && defined(__ppc__))

	longjmp(jmpbuf, 1); 

#elif defined(_WIN32)

	/* Windows does not support longjmp inside a signal handler. */
	close_devices();
	exit(EXIT_SUCCESS);

#endif /* _WIN32 */
}


static uint32_t seconds = 0;
static void
alarm_second_sig(int sig)
{
	seconds++;
}

#if defined(_WIN32)
/*Windows does not have strcasecmp*/
int strcasecmp(const char *s1, const char *s2)
{
	return stricmp(s1,s2);
}
#endif

/*
 * Private function used to set input and output types.  It tests to see that
 * a value has not already been set to some other type - if it has then a
 * panic message is displayed. 
 */
static void
set_type(enum types *type, enum types value)
{
	if(*type == NONE || *type == value) {
		*type = value;
	} else {
		dagutil_panic("Multiple %s types specified.\n",
		      type == &input_type ? "input" : "output");
	}
}

/*
 * Parses the given type_string obatined from the -T option and sets the input
 * and output types based on its contents.  A panic message is displayed if an
 * illegal/unknown type keyword is detected.
 */
static void
set_types(char *type_string)
{
	char* type_state;
	char           *arg;
	char           *input = NULL;
	char           *output = NULL;
	char           *types[] =
	    { "NONE", "ERF", "DAG", "PCAP", "PRT", "NULL" };

	fprintf(stderr, "[%s] \n",__FUNCTION__);
	if(type_string == NULL) {
		dagutil_panic("Type argument is NULL\n");
	}
	arg = strtok_r(type_string, ":", &type_state);
	if(arg == NULL) {
		dagutil_verbose("No input or output type given, using defaults.\n");
	}

	if(type_string[0] != ':') {
		input = arg;
		output = strtok_r(NULL, ":", &type_state);
	} else {
		output = arg;
	}

	if(input == NULL) {
	} else if(strcasecmp(input, "ERF") == 0) {
		set_type(&input_type, ERF);
	} else if(strcasecmp(input, "DAG") == 0) {
		set_type(&input_type, DAG);
	} else if(strcasecmp(input, "PCAP") == 0) {
		set_type(&input_type, PCAP);
	} else if(strcasecmp(input, "ATM") == 0) {
		set_type(&input_type, ATM);
	} else if(strcasecmp(input, "ETH") == 0) {
		set_type(&input_type, ETH);
	} else if(strcasecmp(input, "POS") == 0) {
		set_type(&input_type, POS);
	} else if(strcasecmp(input, "NULL") == 0) {
		set_type(&input_type, NUL);
	} else {
		dagutil_panic("Illegal input type '%s'.\n", input);
	}
	dagutil_verbose("Input type set to %s.\n", types[input_type]);

	if(output == NULL) {
	} else if(strcasecmp(output, "ERF") == 0) {
		set_type(&output_type, ERF);
	} else if(strcasecmp(output, "PCAP") == 0) {
		set_type(&output_type, PCAP);
	} else if(strcasecmp(output, "PRT") == 0) {
		set_type(&output_type, PRT);
	} else if(strcasecmp(output, "NULL") == 0) {
		set_type(&output_type, NUL);
	} else {
		dagutil_panic("Illegal output type '%s'.\n", output);
	}
	dagutil_verbose("Output type set to %s.\n", types[output_type]);
}

/*
 * Set filter options. Expects a comma separated
 * list of filters.
 */
static void
set_filters(char *filters)
{
	char* filter_state;
	char           *arg;

	if(filters == NULL || strlen(filters) == 0)
		dagutil_panic("Bad input to set_filters\n");

	filter_options = 0;

	arg = strtok_r(filters, ",", &filter_state);
	if(arg == NULL)
		return;

	while (NULL != arg)
	{
		if(strcasecmp(arg, "rx") == 0)
			filter_options |= FILTER_RX_MASK;
		else if(strcasecmp(arg, "ds") == 0)
			filter_options |= FILTER_DS_MASK;
		else if(strcasecmp(arg, "trunc") == 0)
			filter_options |= FILTER_TRUNC_MASK;
		else if(strcasecmp(arg, "a") == 0)
			interface_permit |= INTERFACE_A;
		else if(strcasecmp(arg, "b") == 0)
			interface_permit |= INTERFACE_B;
		else if(strcasecmp(arg, "c") == 0)
			interface_permit |= INTERFACE_C;
		else if(strcasecmp(arg, "d") == 0)
			interface_permit |= INTERFACE_D;
		else
			dagutil_warning("unknown filter %s\n", arg);

		arg = strtok_r(NULL, ",", &filter_state);
	}

	dagutil_verbose("Filter mask set to %d\n", filter_options);
	dagutil_verbose("Interface filter mask set to %d\n", interface_permit);
}

/*
 * A post setup function that sets input and output types to a default value
 * if they have not been set already.
 */
static void
check_types_set(void)
{
	if(input_type == NONE) {
		input_type = ERF;
	}
	if(output_type == NONE) {
		output_type = ERF;
	}
}

/*
 * Parses the given length_string to an int and sets the snap length to that
 * value.
 */
static void
set_snap_length(char *length_string)
{
	char           *end;
	int             value = strtol(length_string, &end, 10);

	if(*end != '\0') {
		dagutil_panic("Could not convert '%s' to an integer.\n",
		      length_string);
	}
	if(value < -1) {
		value = NO_SNAP_LIMIT;
	}
	snap_length = value;

	dagutil_verbose("Snap length set to %d\n", snap_length);
}

/*
 * Parses the given alignment_string to an int and sets the record alignment to that
 * value.
 */
static void
set_record_alignment(char *alignment_string)
{
	char* end;
	int value = strtol(alignment_string, &end, 10);

	if(*end != '\0') {
		dagutil_panic("Could not convert '%s' to an integer.\n",
		      alignment_string);
	}
	if(value < 1) {
		value = 1;
	}
	record_alignment = value;

	dagutil_verbose("Output record alignment set to %d\n", record_alignment);
}

/*
 * Set the length of the FCS checksum.
 * value.
 */
static void
set_fcs_bits(char *fcs_string)
{

	char* end;
	int value = strtol(fcs_string, &end, 10);

	if(*end != '\0') {
		dagutil_panic("Could not convert '%s' to an integer.\n",
		      fcs_string);
	}
	fcs_bits = value;
	if(value != 0 && value != 16 && value != 32)
		dagutil_warning("Strange CRC bit length specified, continuing anyway.\n");

	dagutil_verbose("FCS length set to %d\n", fcs_bits);
}

static void
set_capture_time(char *time_string)
{
	char* end;
	int value = strtol(time_string, &end, 10);

	if(*end != '\0') {
		dagutil_panic("Could not convert '%s' to an integer.\n",
		      time_string);
	}
	if(value < 0) {
		value = 0;
	}
	capture_time = value;

	dagutil_verbose("Capture time set to %d seconds.\n", capture_time);
}

static void
set_erf_output_iface(int iface)
{
	erf_output_iface = iface;

	dagutil_verbose("Output interface for ERF records will be set to %d.\n", erf_output_iface);
}

/*
 * Opens input and output files if the input and output types are
 * appropriate.  If in_file_name is null then stdin is used.  If out_file_name
 * is null the stdout is used.
 */
static void
open_devices(char *in_file_name, char *out_file_name)
{
	if(in_file_name != NULL && strcmp(in_file_name, "-") == 0) {
		in_file_name = NULL;
	}
	fprintf(stderr,"open_devices : input_type [%d] , pcap is 3\n",input_type);
	switch (input_type) {
	  case ERF:
		  set_erf_input(in_file_name);
		  break;
	  case DAG:
		  set_dag_input(in_file_name, extra_args);
		  break;
	  case PCAP:
		  set_pcap_input(in_file_name);
		  break;
	  case ATM:
		  set_legacy_input(in_file_name, TYPE_ATM);
		  break;
	  case ETH:
		  set_legacy_input(in_file_name, TYPE_ETH);
		  break;
	  case POS:
		  set_legacy_input(in_file_name, TYPE_HDLC_POS);
		  break;
	  case NUL:
		  set_null_input(in_file_name);
		  break;
	  default:
		  dagutil_panic("No input type set.\n");
		  break;
	}
	dagutil_verbose("Set input %s to %s.\n",
		input_type == DAG ? "device" : "file",
		in_file_name == NULL ? "stdin" : in_file_name);

	if(out_file_name != NULL && strcmp(out_file_name, "-") == 0) {
		out_file_name = NULL;
	}
	switch (output_type) {
	  case ERF:
		  set_erf_output(out_file_name);
		  break;
	  case PCAP:
		  set_pcap_output(out_file_name);
		  break;
	  case PRT:
		  set_prt_output(out_file_name);
		  break;
	  case NUL:
		  set_null_output(out_file_name);
		  break;
	  default:
		  dagutil_panic("No output type set.\n");
		  break;
	}
	dagutil_verbose("Set output file to %s.\n",
		out_file_name == NULL ? "stdout" : out_file_name);
}

/*
 * Closes the input and output devices by calling appropriate device specific
 * functions.
 */
static void
close_devices(void)
{
	switch (input_type) {
	  case ERF:
		  close_erf_input();
		  break;
	  case DAG:
		  close_dag_input();
		  break;
	  case PCAP:
		  close_pcap_input();
		  break;
	  case ATM:
	  case ETH:
	  case POS:
		  close_legacy_input();
		  break;
	  case NUL:
		  close_null_input();
		  break;
	  default:
		  dagutil_panic("No input type set.\n");
		  break;
	}

	switch (output_type) {
	  case ERF:
		  close_erf_output();
		  break;
	  case PCAP:
		  close_pcap_output();
		  break;
	  case PRT:
		  close_prt_output();
		  break;
	  case NUL:
		  close_null_output();
		  break;
	  default:
		  dagutil_panic("No output type set.\n");
		  break;
	}

	dagutil_verbose("\nRecords Processed: %"PRIu64" (%"PRIu64") Bytes Processed: %"PRIu64"\n",
		in_count, out_count, byte_count);
	dagutil_verbose("Closed input and output.\n");
}

/*
 * Called before processing occurs to set any parameters for input and output
 * devices.
 */
static void
set_parameters(void)
{
	switch (input_type) {
	  case ERF:
	  case DAG:
	  case PCAP:
		fprintf(stderr,"setparameters : PCAP \n");
	  case ATM:
	  case ETH:
	  case POS:
	  case NUL:
		  break;
	  default:
		  dagutil_panic("No input type set.\n");
		  break;
	}

	switch (output_type) {
	  case ERF:
		  set_erf_snap_length(snap_length, variable_length);
		  set_erf_record_alignment(record_alignment);
		  break;
	  case PCAP:
		  set_pcap_snap_length(snap_length);
		  break;
	  case PRT:
	  case NUL:
		  break;
	  default:
		  dagutil_panic("No output type set.\n");
		  break;
	}
}

/*
 * Generates a simple single line stats report to stderr about the packets
 * that have passed through the system.
 */
static void
report(void)
{
	static struct timeval last;
	static uint64_t last_in_count,
	                last_out_count,
	                last_byte_count;
	struct timeval  now,
	                time_diff;
	double          in_rate,
	                out_rate,
	                byte_rate = 0,
	                second;

	gettimeofday(&now, NULL);
	if(!timerisset(&last)) {
		/*
		 * Initial call 
		 */
		last = now;
		return;
	}

	if(dagutil_get_verbosity() >= 1) {
		uint64_t in_count_now = in_count;
		uint64_t out_count_now = out_count;
		uint64_t byte_count_now = byte_count;

		timersub(&now, &last, &time_diff);

		second =
		    (double) time_diff.tv_sec +
		    ((double) time_diff.tv_usec / 1000000.0);
		in_rate = (double) (in_count_now - last_in_count) / second;
		out_rate = (double) (in_count_now - last_in_count) / second;
		byte_rate = (double) (byte_count_now - last_byte_count) / second;

		fprintf(stderr,
	"Rate: %.1f (%.1f) rec/s %.1f MB/s Total: %"PRIu64" (%"PRIu64") recs %.1f MB     \r",
			in_rate, out_rate, byte_rate / 1048576.0, in_count_now,
			out_count_now,
			(double) byte_count_now / (1024.0 * 1024.0));

		last_in_count = in_count_now;
		last_out_count = out_count_now;
		last_byte_count = byte_count_now;
		last = now;
	}
}

/*
 * a link to the report alarm handler if it exists 
 */
static void (*second_handler) (int);

static void
alarm_report_sig(int sig)
{
	/*
	 * call second alarm handler if it exists 
	 */
	if(second_handler != NULL && second_handler != SIG_DFL)
		second_handler(sig);
	report();
}

/*
 * Catches alarm signals (send by the one second timer) and decrements the
 * capture_time.  When the capture time is less than or equal to zero the
 * _keep_going flag is reset.
 */
static void
alarm_capture_sig(int sig)
{
	/*
	 * call second alarm handler if it exists 
	 */
	if(second_handler != NULL && second_handler != SIG_DFL)
		second_handler(sig);
	report();

	capture_time--;
	if(capture_time <= 0)
	{
		close_devices();
		exit(EXIT_SUCCESS);
	}
}

// Time signal handling
/* 
	Windows does not have a timer signal in the same form as 
	Unix (i.e. no SIGALRM) therefore a more Windows based 
	approach is required for setting up reporting and 
	termination after a given runtime
*/
#if defined(_WIN32)
VOID CALLBACK
time_handler(LPVOID lpArgToCompletionRoutine, DWORD dwTimerLowValue, DWORD dwTimerHighValue){

//	if(second_handler != NULL && second_handler != SIG_DFL)
//		second_handler(sig);
	report();

	if(capture_time>0)
	{
		capture_time--;
		if(capture_time <= 0)
		{
			close_devices();
			exit(EXIT_SUCCESS);
		}
	}
}
#endif /* _WIN32 */


/*
 * Adds the capture alarm handler.
 */
static void
add_alarm_handler(void)
{
#if defined(__FreeBSD__) || defined(__linux__) || (defined(__SVR4) && defined(__sun)) || (defined(__APPLE__) && defined(__ppc__))

	struct sigaction sigact;
	struct sigaction sigactold;

	sigact.sa_handler =
	    (capture_time >= 1) ? alarm_capture_sig : alarm_report_sig;

	sigact.sa_flags = SA_RESTART;
	if(sigaction(SIGALRM, &sigact, &sigactold) < 0)
		dagutil_panic("sigaction SIGALRM: %s\n", strerror(errno));
	/*
	 * set second handler 
	 */
	second_handler = sigactold.sa_handler;

#elif defined(_WIN32)

   // Create a  timer.
	if(SetTimer(NULL, 34, 1000, (TIMERPROC)time_handler)==0)
        dagutil_panic("Set Timer failed (%d)\n", GetLastError());

#endif /* Platform-specific code. */
}

/*
 * This function is called to perform BPF filtering on packet data.
 * If the supplied expression has not yet been compiled, then it
 * is compiled. The same compiled code is them applied on subsequent
 * calls. The return value is the result of calling the underlying
 * bpf_filter function.
 *
 * This was mostly taken from dagfilter.c.
 */
static int
bpffilter(dag_record_t * header, void *payload)
{
	unsigned char *dp = (unsigned char *) payload;
	unsigned int slen,
	                wlen;

	if(filter_string && !bpf_insn) {

		struct bpf_program fcode;
		int snapshot = 0;
		int linktype = 0;
		pcap_t *p;
		/*
		 *  select compile parameters based on link type
		 */
		switch (header->type) {
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
				linktype = DLT_CHDLC;
				snapshot = DEFAULT_SNAPLEN;
				break;
			
			default:
				dagutil_panic("unknown type error %s line %d\n", __FILE__, __LINE__);
		}


		if ((p = pcap_open_dead(linktype, snapshot)) == NULL) {
			dagutil_panic("pcap_open_dead: out of memory\n");
		}

		/*
		 * compile with optimization and no netmask
		 */
		if(pcap_compile(p, &fcode, filter_string, 1, 0))
			dagutil_panic("bpf compilation error: %s\n", pcap_geterr(p));

		pcap_close(p);

		/*
		 * remember the compiled code
		 */
		bpf_insn = fcode.bf_insns;
	}

	switch (header->type)
	{
		case TYPE_ATM:
			/*
			* The wire length is gone with AAL5, need to fake it
			* from IP header length field
			*/
			dp += 4; /* bpf doesn't account for atm header */
			wlen = ATM_WLEN(header);
			slen = ATM_SLEN(header);
			break;
		
		case TYPE_ETH:
		case TYPE_COLOR_ETH:
			dp += 2; /* 2 extra bytes in Ethernet records */
			wlen = ETHERNET_WLEN(header);
			slen = ETHERNET_SLEN(header);
			break;
		
		case TYPE_HDLC_POS:
		case TYPE_COLOR_HDLC_POS:
			wlen = HDLC_WLEN(header);
			slen = HDLC_SLEN(header);
			break;
		
		default:
			dagutil_panic("Unknown type %s line %d\n", __FILE__, __LINE__);
	}

	/*
	 * We currently ignore the value of the filter return code, this
	 * could be changed with future variable length packet structures.
	 */
	return bpf_filter(bpf_insn, dp, wlen, slen);
}

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


#if 1

int
Start_Stop(int opt)
{

	struct 	ether_erf_header		*nte_ether_erf_hdr;
	struct 	mac_hdr					*nte_mac_hdr;
	struct	imsi_hdr				*nte_imsi_hdr;
	int ret;
	unsigned short send_len=0;
	struct timeval tv;
	unsigned int  nowt;

	char	nte_send[5000];

	return 0;

	memset(nte_send,0x00,5000);
	send_len = sizeof(struct ether_erf_header)+sizeof(struct mac_hdr) + sizeof(struct imsi_hdr);

	nte_ether_erf_hdr = (struct ether_erf_header *) nte_send;

	/* ERF Header   */
	nowt = (unsigned int) time(NULL);
	gettimeofday(&tv,NULL);
	nte_ether_erf_hdr->sec  = htonl(nowt);
	nte_ether_erf_hdr->usec  = htonl(tv.tv_usec);
	nte_ether_erf_hdr->type   = 2;
	nte_ether_erf_hdr->flags.iface    = 1;
	nte_ether_erf_hdr->flags.vlen     = 0;
	nte_ether_erf_hdr->flags.trunc    = 0;
	nte_ether_erf_hdr->flags.rxerror  = 0;
	nte_ether_erf_hdr->flags.dserror  = 0;
	nte_ether_erf_hdr->flags.pad      = 0;
	nte_ether_erf_hdr->pad    		= 0x0000;
	nte_ether_erf_hdr->rlen  	=  htons(send_len);

	/* MAC hdr	*/
	nte_mac_hdr = (struct mac_hdr *) (nte_send + sizeof(struct ether_erf_header));
	if (opt == 1)	{						
		nte_mac_hdr->mac_type = htons(0x0801);
	}
	else if (opt == 2)	{		
		nte_mac_hdr->mac_type = htons(0x0802);
	}

	nte_imsi_hdr = (struct imsi_hdr *) (nte_send + sizeof(struct ether_erf_header) +sizeof(struct mac_hdr));
	sprintf((char *) &(nte_imsi_hdr->imsi[0]),"000450162707102");
	nte_imsi_hdr->imsi_len = htons(15);


	fprintf(stderr, "Before snedto START (g_sendsockfd)  \n");
	ret = sendto(g_sendsockfd , nte_send ,send_len ,0x0,(struct sockaddr *)&g_servaddr, sizeof(g_servaddr));

	if(ret< 0) {
		fprintf(stderr, "START sendto error (g_sendsockfd)  \n");
	} else {
		fprintf(stderr, "START sendto success (g_sendsockfd)  \n");
	}

	return ret;
}
#endif




/*
 * The heart of dagconvert.  Reads ERF headers and payloads from an input
 * reader, applies any filtering and sends (possibly modified) ERF headers and
 * payloads to output writer.
 */
static void
process(void)
{
	dag_record_t   *header;
	
	unsigned char  *payload;
/*
	dag_record_t   *(*get_next_header) ();
*/
	void	   *(*get_next_header) ();
	void           *(*get_payload) ();

#if 0
	int             (*write_record) (dag_record_t *, void *);
#endif 
	int             (*write_record) (dag_record_t *, void *);

	int				sockd = 0;
	int				on = 1;
	int				ret;

	struct sockaddr_in      mysocket;
    int             slen;
    struct ip	    *iphdr;
    struct udphdr   *udphdr;
#if defined(_WIN32)
	MSG msg;
#endif /* _WIN32 */

	/*
	 * switch input method based on input type 
	 */
	switch (input_type) {
	  case ERF:
      fprintf(stderr, "input : ERF .. cap data  ....\n");
		  get_next_header = (void *) &get_next_erf_header;
		  get_payload = &get_erf_payload;
		  break;
	  case DAG:
      fprintf(stderr, "input : DAG\n");
		  get_next_header = (void *) &get_next_dag_header;
		  get_payload = &get_dag_payload;
		  break;
	  case PCAP:
      fprintf(stderr, "input : PCAP\n");
		  get_next_header = (void *) &get_next_cap_header;
		  get_payload = &get_pcap_payload;
		  break;
	  case ATM:
	  case ETH:
	  case POS:
		  get_next_header = (void *) &get_next_legacy_header;
		  get_payload = &get_legacy_payload;
		  break;
	  case NUL:
		  get_next_header = (void *) &get_next_null_header;
		  get_payload = &get_null_payload;
		  break;
	  default:
		  dagutil_panic("No input type set.\n");
		  break;
	}

	/*
	 * switch output method based on output type 
	 */
	switch (output_type) {
	  case ERF:
      fprintf(stderr, "output : ERF\n");
		  write_record = &write_erf_record;
		  break;
	  case PCAP:
      fprintf(stderr, "output : PCAP\n");
		  write_record = &write_pcap_record;
		  break;
	  case PRT:
		  write_record = &write_prt_record;
		  break;
	  case NUL:
		  write_record = &write_null_record;
		  break;
	  default:
		  dagutil_panic("No output type set.\n");
		  break;
	}

	in_count = 1;
	out_count = 0; 
	byte_count = 0;
	

	if(!g_nte){
			/*      Raw Socket      */
			if((sockd = socket(AF_INET,SOCK_RAW,IPPROTO_RAW)) < 0)  {
					perror("socket");
			}

			if(setsockopt(sockd,IPPROTO_IP,IP_HDRINCL,(char *)&on,sizeof(on)) < 0)  {
					perror("setsockopt");
					exit(1);
			}
	}

	while((header = get_next_header()) != NULL) {

		int rlen;
		unsigned long long lts;
		struct timeval tv;
		struct timeval *ptv;

		payload = get_payload();
#if 0
		fprintf(stderr,"in_count %d , X %d , Z %d\n",in_count,X_send_cnt,Z_send_cnt);
#endif
		if( (X_send_cnt <= in_count) && (in_count <= Z_send_cnt) ){ /* 5000 */
			lts = header->ts;
			tv.tv_sec = lts >> 32;
			lts  = (( lts & 0xffffffffULL) * 1000 * 1000);
			lts += (lts &   0x80000000ULL) << 1;	// rounding 
			tv.tv_usec = lts >> 32;
			if(tv.tv_usec >= 1000000) {
				tv.tv_usec -= 1000000;
				tv.tv_sec += 1;
			}
#ifdef LOG
			fprintf(stderr," [%d] sec %7u usec %7u type %d : iface %d : rlen %4d : lctr %4d : wlen %4d : sizeof(record) %3d : input %d \n"
					,(unsigned int)in_count
					,(unsigned int) tv.tv_sec
					,(unsigned int) tv.tv_usec
					,(char) header->type
					,header->flags.iface
					,ntohs(header->rlen)
					,ntohs(header->lctr)
					,ntohs(header->wlen)
					,sizeof(dag_record_t)
					,input_type
				   );
#endif
			if (input_type == ERF)	{
				fprintf(stderr,"==================== ERF =====================\n");
				fprintf(stderr,": payload- etype %04x "
						,ntohs( *((unsigned short *) (payload + 14)))
				   		);
				if( ntohs( *((unsigned short *) (payload + 14))) == 0x0800){
					/*
					   struct ip *iphdr;
					 */
					iphdr = (struct ip *) (payload + 16);
					fprintf(stderr,": [%d][%d] protocol %4d "
							,iphdr->ip_v
							,iphdr->ip_hl
							,iphdr->ip_p
						   );
					if(iphdr->ip_p == 17 || iphdr->ip_p == 6){
						slen = ntohs(iphdr->ip_len);
						udphdr = (struct udphdr *) (iphdr +1);
						fprintf(stderr,": dport %4d   sport %4d    len %d"
								,ntohs(udphdr->dest)
								,ntohs(udphdr->source),slen
							   );
						memset(&mysocket,'\0',sizeof(mysocket));
						mysocket.sin_family = AF_INET;
						mysocket.sin_port = htons(udphdr->dest);
						mysocket.sin_addr.s_addr = iphdr->ip_dst.s_addr;
						ret = sendto(sockd,(payload+16),slen,0x0,(struct sockaddr *)&mysocket, sizeof(mysocket));
						if (ret < 0)	{
							fprintf(stderr,": #####  send error  count[%d] ####",(int) in_count);
						}				
					}
					fprintf(stderr,"\n");
					/* dump(payload,ntohs(header->rlen)); */
				}
			}
			else if (input_type == PCAP)	{
				ptv = (struct timeval *)  &header->ts;
#ifdef LOG
				fprintf(stderr,"==================== PCAP =====================\n");
				fprintf(stderr,"그냥 ptv_sec = %u , ptv_usec %u    tv_sec=%u\n", ptv->tv_sec , ptv->tv_usec,tv.tv_sec);
				fprintf(stderr,"바꾸어  ptv_sec = %u , ptv_usec %u   tv_sec=%u\n", ntohl(ptv->tv_sec) , htonl(ptv->tv_usec),ntohl(tv.tv_sec));
#endif
				/*
				   cap_header = (data_hdr_t *)header;
				 */
				if(g_nte){
					char senderf[8000];
					struct sim_header *p_sim_hdr;
					int send_len = ntohs(header->rlen);
					char mac1[6] ; 
					char mac2[6]; 
					unsigned int  nowt;

					if(g_first_packet == 0){
						g_first_packet_time = (unsigned int) ptv->tv_sec;
						g_first_system_time = (unsigned int) time(NULL);
						g_gap =  g_first_system_time - g_first_packet_time;
						g_first_packet++;
					}
					mac1[0] = 0x20;
					mac1[1] = 0x53;
					mac1[2] = 0x45;
					mac1[3] = 0x4e;
					mac1[4] = 0x44;
					mac1[5] = 0x04;

					mac2[0] = 0x04;
					mac2[1] = 0x00;
					mac2[2] = 0x40;
					mac2[3] = 0x00;
					mac2[4] = 0x00;
					mac2[5] = 0x00;

					p_sim_hdr = (struct sim_header *) &senderf[0];
					memset(senderf,0x00,8000);

					iphdr = (struct ip *) (payload + 14);
					{
						unsigned char *ppp;
						/* fprintf(stderr,"%d.%d.%d.%d\n",NIPADDR(iphdr->ip_dst.s_addr)); */
						ppp = (char *) (&iphdr->ip_dst.s_addr);
#ifdef LOG
						fprintf(stdout,"%d  %d\n",ppp[0],ppp[3]);
#endif
						/* ppp[0] 가 10.  */
						if(ppp[0] == 10 || ppp[0] == 211){
						 	p_sim_hdr->direction    = htonl(2);
						} else {
						 	p_sim_hdr->direction    = htonl(1);
						}
					}

					/* ERF Header	*/

					nowt = time(NULL);
#ifdef LOG
					printf("nowt %u , ptv->tv_sec = %u , tv.tv_usec=%u , direction %d\n",(unsigned int) nowt,((unsigned int) ptv->tv_sec),(unsigned int )(ptv->tv_usec),htonl(p_sim_hdr->direction));
#endif
					g_sendCnt++;

					if((g_sendCnt % 10000) == 0) {
						printf("@@@+++### SEND CNT[%lld]\n", g_sendCnt);
					}

					if(g_mn == 1){
						unsigned int ptime;
						ptime = (ptv->tv_sec);
						if(g_gap > nowt - ptime){
							printf("gap %d , nowt %d , ptime %d nowt-tv.tv_sec %d\n",g_gap,nowt,ptime,nowt-ptime);
							fprintf(stderr,"sleep 1\n");
							sleep (1);
						}
						p_sim_hdr->sec  = htonl(nowt);
						p_sim_hdr->usec  = htonl(tv.tv_usec);
					} else if(g_mn == 2){
						while(1) {
							unsigned int ptime;
							nowt = time(NULL);
							ptime = (ptv->tv_sec);
							if(g_gap > nowt - ptime){
								printf("gap %d , nowt %d , ptime %d nowt-tv.tv_sec %d\n",g_gap,nowt,ptime,nowt-ptime);
								fprintf(stderr,"sleep 1\n");
								sleep (1);
							} 
							else 
								break;	
						}
						p_sim_hdr->sec  = htonl(ptv->tv_sec);
						p_sim_hdr->usec  = htonl(tv.tv_usec);
					} else if(g_mn == 3){
						while(1) {
							unsigned int ptime;
							nowt = time(NULL);
							ptime = (ptv->tv_sec);
							if(g_gap > nowt - ptime){
								printf("gap %d , nowt %d , ptime %d nowt-tv.tv_sec %d\n",g_gap,nowt,ptime,nowt-ptime);
								fprintf(stderr,"sleep 1\n");
								sleep (1);
							} 
							else 
								break;	
						}
						nowt = time(NULL);
						p_sim_hdr->sec  = htonl(nowt);
						p_sim_hdr->usec  = htonl(tv.tv_usec);
					} else if(g_mn == 4){
						while(1) {
							unsigned int ptime;
							nowt = time(NULL);
							ptime = (ptv->tv_sec);
							if(g_gap > nowt - ptime){
								printf("gap %d , nowt %d , ptime %d nowt-tv.tv_sec %d\n",g_gap,nowt,ptime,nowt-ptime);
								fprintf(stderr,"sleep 1\n");
								sleep (1);
							} 
							else 
								break;	
						}
						gettimeofday(&tv, NULL);
						p_sim_hdr->sec  = htonl(tv.tv_sec);
						p_sim_hdr->usec  = htonl(tv.tv_usec);
					}
					else if(g_mn == 5){
						while(1) {
							unsigned int ptime;
							nowt = time(NULL);
							ptime = (ptv->tv_sec);
							if(g_gap > nowt - ptime){
								printf("gap %d , nowt %d , ptime %d nowt-tv.tv_sec %d\n",g_gap,nowt,ptime,nowt-ptime);
								fprintf(stderr,"sleep 1\n");
								sleep (1);
							} 
							else 
								break;	
						}
						usleep(20000);
						gettimeofday(&tv, NULL);
						p_sim_hdr->sec  = htonl(tv.tv_sec);
						p_sim_hdr->usec  = htonl(tv.tv_usec);
					}
					else {
						p_sim_hdr->sec  = htonl(ptv->tv_sec);
						p_sim_hdr->usec  = htonl(tv.tv_usec);
					}
/*
					p_sim_hdr->usec  = htonl(ptv->tv_usec);
*/

					send_len += sizeof(struct sim_header);

					memcpy((senderf+sizeof(struct sim_header)) , payload, ntohs(header->rlen) );

#ifdef LOG
					fprintf(stderr,"sizeof %d flags 0x%02x rlen %d send_len %d \n",sizeof(struct sim_header),*((unsigned char*) &p_sim_hdr->direction),ntohs(header->rlen),send_len);
#endif

					ret = sendto(g_sendsockfd , senderf ,send_len ,0x0,(struct sockaddr *)&g_servaddr, sizeof(g_servaddr));
					if(ret < 0)	{
						fprintf(stderr, "sendto error (g_sendsockfd)  \n");
					}
#ifdef LOG
					fprintf(stderr, "sendto success (g_sendsockfd)  \n");
#endif
					
				}	else	{  
				/* -N option이 없을때 */
				if( ntohs( *((unsigned short *) (payload + 12))) == 0x0800){
						/*
					   	struct ip *iphdr;
					 	*/
						iphdr = (struct ip *) (payload + 14);
						fprintf(stderr,": [%d][%d] protocol %4d "
							,iphdr->ip_v
							,iphdr->ip_hl
							,iphdr->ip_p
						   );
						if(iphdr->ip_p == 17 || iphdr->ip_p == 6){
							slen = ntohs(iphdr->ip_len);
							udphdr = (struct udphdr *) (iphdr +1);
							fprintf(stderr,": dport %4d   sport %4d    len %d\n"
								,ntohs(udphdr->dest)
								,ntohs(udphdr->source),slen
							   );
							memset(&mysocket,'\0',sizeof(mysocket));
							mysocket.sin_family = AF_INET;
							mysocket.sin_port = htons(udphdr->dest);
							mysocket.sin_addr.s_addr = iphdr->ip_dst.s_addr;
							ret = sendto(sockd,(payload+14),slen,0x0,(struct sockaddr *)&mysocket, sizeof(mysocket));
							if (ret < 0)	{
								fprintf(stderr,": #####  send error  count[%d] ####\n",(int) in_count);
							}				
							fprintf(stderr,": sendto success......\n");
						}
						fprintf(stderr,"\n");
						/* dump(payload,ntohs(header->rlen)); */
					}
				/*
				   fprintf(stderr,"%02x : %02x --",*((unsigned char *)payload) , *((unsigned char *) (payload+1)));
				   fprintf(stderr,"%02x : %02x \n",*((unsigned char *)payload+12) , *((unsigned char *) (payload+13)));
				 */
				}
			}

#if 1
			if(input_type == ERF)	{
				rlen = ntohs(header->rlen);
				fprintf (stderr,"ERF rlen : [%d] \n",rlen);
			}
			else	{
				rlen = header->rlen;
	#ifdef LOG
				fprintf (stderr,"PCAP rlen : [%d] \n",rlen);
	#endif
			}
			if (rlen == 20) {
				// DS error truncates the packet, but the packet has effectively been padded to 28 bytes by the card. 
				rlen = 28;
			}
#else
			rlen = ntohs(header->rlen);
			fprintf (stderr,"ERF rlen : [%d] \n",rlen);
#endif
			
			byte_count += rlen;
	#ifdef LOG
			fprintf(stderr,"byte_count [%d] \n",(int) byte_count);
	#endif
			/*
			 * apply any filters here 
			 */

			/*
			 * filter based on ERF flags 
			 * for efficiency the dag_header is cast to an anonymous
			 * structure which replaces the bitfields of dag_header_t
			 * with an u_char
			 */
			if(filter_options && (((struct {
								uint64_t ts;
								unsigned char type;
								unsigned char flags;}
								*)header)->
						flags & filter_options)) {
				/* filter suggests packet should be discarded */
			} else if(filter_string && !bpffilter(header, payload)) {
				/* filter suggests packet should be discarded */
			} else if(!interface_permit
					|| (interface_permit &
						(1 << header->flags.iface))) {
				/*
				 * accept packet if no particular interface was
				 * selected or if packet matches one of the selected
				 * interfaces
				 */
				out_count++;
				if(write_record(header, payload) == 0) {
					dagutil_panic("Failed to write data to output.\n");
				}
			}
		} 
		in_count++;
	}
	#define NTE_END  2
	if(g_nte){
		fprintf (stderr,"NTE END  \n");
		Start_Stop(NTE_END);	
	}
	return ;
}

/*
 * Parses command line arguments setting appropriate parameters in input and
 * output devices and the initiates the processing.
 */



int
main(int argc, char **argv)
{
	int opt;
	int len;
	int	ret;

	char* in_file_name = NULL;
	char* out_file_name = NULL;
	char* more = NULL;
	/*
	 * setup properties 
	 */
	input_type = NONE;
	output_type = NONE;
	capture_time = 0;
	snap_length = NO_SNAP_LIMIT;
	variable_length = -1;
	rotate_size = 0;

	dagutil_set_progname("dagconvert");

	while((opt = getopt(argc, argv, "U:W:N:M:P:X:Z:A:b:c:d:f:FG:hi:o:p:r:s:t:T:Vv-:")) != EOF)
	{
		fprintf(stderr,"[getopt] : opt[%c] \n",opt);
		switch (opt) {
			case 'M':
				g_mn = (unsigned int) atoi(optarg);						
				if(g_mn == 1) fprintf(stderr,"변경된 시간으로 단말 처럼 전송\n");
				if(g_mn == 2) fprintf(stderr,"원래의 시간으로 단말 처럼 전송\n");
				if(g_mn == 5) fprintf(stderr,"현재의 시간으로 패킷당 1초 Sleep 이후 전송\n");
				break;
			case 'N':
				g_nte = 1;	
				fprintf(stderr,"optarg [%s] \n",optarg); 
				g_sendsockfd = socket(AF_INET,SOCK_DGRAM,0);
				if (g_sendsockfd < 0)	{
					fprintf(stderr,"socket error : g_sendsockfd \n");
					break;
				}
				fprintf(stderr,"======  socket success  : g_sendsockfd ======\n");

				bzero(&g_servaddr,sizeof(g_servaddr));
				g_servaddr.sin_family = AF_INET;
				if(g_port != 0) g_servaddr.sin_port = htons(g_port);
				else g_servaddr.sin_port = htons(2006);
				g_servaddr.sin_addr.s_addr = inet_addr(optarg);

				ret = connect(g_sendsockfd, (struct sockaddr_in *) &g_servaddr,sizeof(g_servaddr));
				if (ret < 0)	{
					fprintf (stderr,"connect error \n");
					break;
				}
				fprintf (stderr,"connect OK ...  \n");
				#define NTE_START  1
				fprintf (stderr,"NTE START  \n");
				Start_Stop(NTE_START);	
				break;
		  case 'P':
				g_port = (unsigned int) atoi(optarg);						
				g_servaddr.sin_port = htons(g_port);
				printf ("UDP Port [%d] \n", g_port);
				break;
/*	shjeon 03.23	*/
			/* VCI	*/
		  case 'U':
				g_vci = (unsigned int) atoi(optarg);						
				printf ("VCI optarg [%d] \n", g_vci);
				break;
			/* VPI	*/
		  case 'W':
				g_vpi = (unsigned int) atoi(optarg);						
				printf ("VPI optarg [%d] \n", g_vpi);
				break;

		  case 'X':
			  X_send_cnt = (unsigned int) atoi(optarg);
			  printf("X option : %d\n",X_send_cnt );
			  break;
		  case 'Z':
			  Z_send_cnt = (unsigned int) atoi(optarg);
			  printf("Z option : %d\n",Z_send_cnt );
			  break;
		  case 'A':
			  set_record_alignment(optarg);
			  break;

		  case 'b':
			  filter_string = optarg;
			  break;

		  case 'c':
			  set_fcs_bits(optarg);
			  break;

		  case 'd':
			  set_type(&input_type, DAG);
			  in_file_name = optarg;
			  break;

		  case 'f':
			  set_filters(optarg);
			  break;

		  case 'F':
			  variable_length = 0;
			  break;

		  case 'G':
			  /* GMT offset no longer supported */
			  break;

		  case '?':
		  case 'h':
			  printf("dagconvert (DAG %s) %s\n", kDagReleaseVersion, kRevisionString);
			  printf(usage_text);
			  exit(EXIT_SUCCESS);
			  break;

		  case 'i':
			  in_file_name = optarg;
			  break;

		  case 'o':
			  out_file_name = optarg;
			  break;

		  case 'p':
				{
					int output_iface = strtol(optarg, NULL, 0);
					
					if ((0 <= output_iface) && (output_iface <= 3))
					{
						set_erf_output_iface(output_iface);
					}
					else
					{
						dagutil_warning("unrecognised output interface %s\n", optarg);
					}
				}
				break;

		  case 'r':
			  rotate_size = strtoul(optarg, &more, 0);
			  switch(*more) {
			  case '\0':
				  break;
			  case 'G':
			  case 'g':
				  rotate_size *= ONE_GIBI;
				  break;
			  case 'M':
			  case 'm':
				  rotate_size *= ONE_MEBI;
				  break;
			  case 'K':
			  case 'k':
				  rotate_size *= ONE_KIBI;
				  break;
			  default:
				  dagutil_warning("unrecognized character '%c' after -r <N> specification\n",
				      *more);
				  break;
			  }
			  if((*more != '\0') && (*++more != '\0'))
				  dagutil_warning("extra character '%c' after -r <N> specification\n", *more);
			  break;

		  case 's':
			  set_snap_length(optarg);
			  break;

		  case 't':
			  set_capture_time(optarg);
			  break;

		  case 'T':
			  set_types(optarg);
			  break;

		  case 'V':
			  variable_length = 1;
			  break;

		  case 'v':
			  dagutil_inc_verbosity();
			  dagutil_verbose("Verbose level set to %d\n", dagutil_get_verbosity());
			  break;

		  case '-':
			  if (strcmp(optarg, "help") == 0 || strcmp(optarg, "usage") == 0)
			  {
				  printf("dagconvert (DAG %s) %s\n", kDagReleaseVersion, kRevisionString);
				  printf(usage_text);
				  exit(EXIT_SUCCESS);
			  }
			  else if (strcmp(optarg, "verbose") == 0)
			  {
				  dagutil_inc_verbosity();
				  dagutil_verbose("Verbose level set to %d\n", dagutil_get_verbosity());
			  }
			  else if (strcmp(optarg, "version") == 0)
			  {
				  printf("dagconvert (DAG %s) %s\n", kDagReleaseVersion, kRevisionString);
				  exit(EXIT_SUCCESS);
			  }
			  else
			  {
				  dagutil_panic("unknown option '%s', see -h for help on usage\n", optarg);
			  }
			  break;

		  default:
			  dagutil_panic("unknown option, see -h for help on usage\n");
			  break;
		}
	}

	/*
	 * send any extra arguments to DAG if it is the input device 
	 */
	argc -= optind;
	argv += optind;
	len = 0;
	for(opt = 0; opt < argc; opt++) {
		len += (int)strlen(argv[opt]) + 1;
	}
	extra_args = (char *) malloc(sizeof(char) * len + 1);
	for(opt = 0; opt < argc; opt++) {
		strcat(extra_args, argv[opt]);
		strcat(extra_args, " ");
	}
	extra_args[len] = '\0';

	if(strlen(extra_args) && input_type != DAG) {
		dagutil_warning("Unused args '%s'\n", extra_args);
	}

	/*
	 * check that input/output types have been set to something 
	 */
	check_types_set();

	/*
	 * setup input and output device parameters 
	 */
	set_parameters();

	/*
	 * open input and output files 
	 */
	open_devices(in_file_name, out_file_name);

	/*
	 * finished with the extra args at this point 
	 */
	free(extra_args);

	dagutil_set_signal_handler(anysig);

	if(setjmp(jmpbuf))
	{
		goto endprocess;
	}

	/*
	 * start timer that fires every 1 second 
	 */
	dagutil_set_timer_handler(alarm_second_sig, 1);

	/*
	 * add the capture and report alarms 
	 */
	add_alarm_handler();

	/*
	 * initialize the report counters 
	 */
	report();

	/*
	 * do the processing 
	 */
	process();

      endprocess:
	/*
	 * close input and output files 
	 */
	close_devices();

	sleep(10000);

	return EXIT_SUCCESS;
}
