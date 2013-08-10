/*
 * $Id $
 */

/*
 * Indicates no snap length limit is in effect 
 */
#define NO_SNAP_LIMIT -1

/*
 * Output parameter, rotate output file after this many bytes.
 * If zero, then no file rotation.
 */

unsigned	_rotate_size;

/*
 * Function called when the processor has an ERF
 * header and payload to output. Only the header
 * fields of the dag_record_t are valid. The header
 * is correct for the given payload. Returns the
 * number of records written.
 */
int             write_record(dag_record_t * header, void *payload);


/*******************************************************
 * ERF file functions
 *******************************************************/
/*
 * Write given ERF header and payload in ERF format.
 * Returns 1 if record successfully written and 0
 * otherwise.
 */
int             write_erf_record(dag_record_t * header, void *payload);
int             write_file (data_hdr_t * header, void *payload);

/*
 * Set the stream on which output is to be produced.
 */
void            set_erf_output(char *out);

/*
 * Close the currently open input (if any).
 */
void            close_erf_output(void);

/*
 * Set a snap length to apply to packets. Packets
 * longer than this value are truncated to the specified
 * value. To allow arbitrary length packets, set
 * snap_length to -1. If variable_length is nonzero then packets
 * can be of variable length, otherwise all packets
 * will have the specified length.
 */
void            set_erf_snap_length(int snap_length, int variable_length);
void            set_erf_record_alignment(int record_alignment);

/*******************************************************
 * PCAP file functions
 *******************************************************/
/*
 * Write given ERF header and payload in PCAP format.
 * Returns 1 if record successfully written and 0
 * otherwise.
 */
int             write_pcap_record(dag_record_t * header, void *payload);

/*
 * Set the stream on which output is to be produced.
 */
void            set_pcap_output(char *out);

/*
 * Close the currently open input (if any).
 */
void            close_pcap_output(void);

/*
 * Set a snap length to apply to packets. Packets
 * longer than this value are truncated to the specified
 * value.
 */
void            set_pcap_snap_length(int snap_length);

/*
 * Manually set the timezone to use with pcap packets.
 * The value should be the number of seconds west of GMT.
 * If this function is not called, then the timezone is
 * guessed based on the result of tzset().
 */
void            set_pcap_timezone(int tzone);

/*
 * Set the wirelength fudge value.
 * Defaults to 4.
 */
void            set_pcap_wlen_fudge(int n);

/*******************************************************
 * PRT file functions
 *******************************************************/
/*
 * Write an ASCII representation of a record. Returns 1
 * if record successfully written.
 */
int             write_prt_record(dag_record_t * header, void *payload);

/*
 * Set the stream on which output is to be produced.
 */
void            set_prt_output(char *out);

/*
 * Close the currently open input (if any).
 */
void            close_prt_output(void);

/*******************************************************
 * Null output device functions
 *******************************************************/
/*
 * Discard ERF header and payload.
 */
int             write_null_record(dag_record_t * header, void *payload);

/*
 * Stub function, included for completeness.
 */
void            set_null_output(char *out);

/*
 * Stub function, included for completeness.
 */
void            close_null_output(void);

/*
*	$Log $
*/

