/*
 * $Id $
 */

/* DAG tree headers. */
#include "dagapi.h"


extern int rotate_size;
extern int fcs_bits;
extern int erf_output_iface;
extern int g_first ;
extern int g_vpi;
extern int g_vci;


typedef struct data_header  {
	uint64_t		dwTime;
    unsigned int    dwLen;
    unsigned int    dwLen2;
} data_hdr_t;


/*
 * Get pointer to the ERF header for the next packet
 * in the input stream. Returns null if no further
 * packets are available.
 */
dag_record_t   *get_next_header(void);

/*
 * Returns a pointer to the payload of the most recent
 * packet. Returns null if there is no current packet.
 */
void           *get_payload(void);

/*
 * Set the name of the stream from which input is obtained.
 */
void            set_input(char *name);

/*
 * Called when the input is no longer needed so that any 
 * special shutdown tasks can be performed.
 */
void            close_input();

/*******************************************************
 * ERF file functions
 *******************************************************/
/*
 * Get pointer to the ERF header for the next packet
 * in the input stream. Returns null if no further
 * packets are available.
 */
dag_record_t   *get_next_erf_header(void);
dag_record_t   *get_next_cap_header(void);

/*
 * Returns a pointer to the payload of the most recent
 * packet. Returns null if there is no current packet.
 */
void           *get_erf_payload(void);

/*
 * Set the stream from which input is obtained.
 */
void            set_erf_input(char *in);

/*
 * Close the current input stream.
 */
void            close_erf_input(void);

/*******************************************************
 * Legacy file functions
 *******************************************************/
/*
 * Get pointer to the ERF header for the next packet
 * in the input stream. Returns null if no further
 * packets are available.
 */
dag_record_t   *get_next_legacy_header(void);

/*
 * Returns a pointer to the payload of the most recent
 * packet. Returns null if there is no current packet.
 */
void           *get_legacy_payload(void);

/*
 * Set the stream from which input is obtained.
 */
void            set_legacy_input(char *in, int input_type);

/*
 * Close the current input stream.
 */
void            close_legacy_input(void);

/*******************************************************
 * Direct form DAG card functions
 *******************************************************/
/*
 * Get pointer to the ERF header for the next packet
 * in the input stream from a DAG card. Returns null 
 * if no further packets are available.
 */
dag_record_t   *get_next_dag_header(void);

/*
 * Returns a pointer to the payload of the most recent
 * packet. Returns null if there is no current packet.
 */
void           *get_dag_payload(void);

/*
 * Set the stream from which input is obtained.  The given dagname is expected
 * to be a unix device (/dev/dag0 for example) and args is a string comprised
 * of space separated arguments that is set to the dag via dag_configure.
 */
void            set_dag_input(const char *dagname, char *args);

/*
 * Called when the DAG input is no longer needed so that any 
 * special shutdown tasks can be performed.
 */
void            close_dag_input();

/*******************************************************
 * PCAP file functions
 *******************************************************/
/*
 * Get pointer to the ERF header for the next packet
 * in a PCAP file. Returns null if no further packets are available.
 */
dag_record_t   *get_next_pcap_header(void);

/*
 * Returns a pointer to the payload of the most recent
 * packet. Returns null if there is no current packet.
 */
void           *get_pcap_payload(void);

/*
 * Set the stream from which input is obtained.
 */
void            set_pcap_input(char *filename);

/*
 * Called when the PCAP input is no longer needed so that any 
 * special shutdown tasks can be performed.
 */
void            close_pcap_input();

/*******************************************************
 * Null input device functions
 *******************************************************/
/*
 * Returns NULL.
 */
dag_record_t   *get_next_null_header(void);

/*
 * Returns NULL.
 */
void           *get_null_payload(void);

/*
 * Stub function, included for completeness.
 */
void            set_null_input(char *in);

/*
 * Stub function, included for completeness.
 */
void            close_null_input(void);

/*
*	$Log $
*/

