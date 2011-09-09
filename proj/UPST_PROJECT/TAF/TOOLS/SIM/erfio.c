/*
 * Copyright (c) 2002-2005 Endace Technology Ltd, Hamilton, New Zealand.
 * All rights reserved.
 *
 * This source code is proprietary to Endace Technology Limited and no part
 * of it may be redistributed, published or disclosed except as outlined in
 * the written contract supplied with this product.
 *
 * $Id: erfio.c,v 1.1.1.1 2011/08/29 05:56:42 dcham Exp $
 *
 * Write ERF records in ERF format.
 */

/* dagconvert headers. */
#include "inputs.h"
#include "outputs.h"
#include "utils.h"

/* Endace headers. */
#include "dagapi.h"
#include "dagutil.h"


#ifndef O_LARGEFILE
#define O_LARGEFILE 0
#endif

/*
 * Output stream. 
 */
static FILE    *outfile = NULL;

static int      snap_length = NO_SNAP_LIMIT;

static int	record_alignment = 1;

/*
 * Variable or fixed length output? 
 */
#define STATUS_QUO -1
static int      variable_length = 1;

/*
 * Remember if warning about truncation has been printed 
 */
static int      trunc_warning = 0;

/*
 * Remember if warning about ATM snapping has been printed 
 */
static int      atm_warning = 0;

/*
 * Bytes written to file so far, and file rotation sequence
 */
static unsigned	bytes_written = 0;
static unsigned	rotate_num = 0;

/*
 * Save output file name for later use.
 */
static char *outname = NULL;

/*
 * Used for padding 
 */
static const char ZERO[64] = {0};

/*
 * Set the stream on which output is to be produced.
 */

void
set_erf_output(char *out)
{
	int outfd;
	char *oname;

	outname = out;

	printf("DEBUG: set_erf_output %s\n", out);

	if(out == NULL)
		outfile = stdout;
	else {
		if (rotate_size) {
			oname = calloc(1, strlen(out)+15);
			snprintf(oname, strlen(out)+15, "%s%04u.erf", out, rotate_num);
			dagutil_verbose("Set output file to %s\n", oname);
			outfd = open(oname, O_RDWR|O_CREAT|O_TRUNC|O_LARGEFILE, 0664);
			free(oname);
			bytes_written = 0;
		} else
			outfd = open(out, O_RDWR|O_CREAT|O_TRUNC|O_LARGEFILE, 0664);
		
		if(outfd == -1) {
			dagutil_panic("Could not open %s for writing.\n", out);
		}
		outfile = fdopen(outfd, "w");
		if(outfile == NULL) {
			dagutil_panic("Could not open %s for writing.\n", out);
		}
	}
}

/*
 * Close the currently open input (if any).
 */
void
close_erf_output(void)
{
	if(outfile != NULL && outfile != stdout)
		fclose(outfile);
	outfile = NULL;
}

/*
 * Set a snap length to apply to packets. Packets
 * longer than this value are truncated to the specified
 * value. To allow arbitrary length packets, set
 * snap_length to -1. If variable_length is nonzero then packets
 * can be of variable length, otherwise all packets
 * will have the specified length.
 */
void
set_erf_snap_length(int snap, int variable_length)
{
#ifdef PEDANTIC
	if(snap < -1
	   || (snap == NO_SNAP_LIMIT && variable_length == 0)) {
		dagutil_panic("Bad arguments to set_erf_snap_length.\n");
	} else if(variable_length == STATUS_QUO
		  && snap != NO_SNAP_LIMIT) {
		dagutil_panic("Must specify one of -F or -V to use this mode.\n");
	}
#endif

	/*
	 * make the snap length a multiple of four
	 */
	if(snap == NO_SNAP_LIMIT)
		snap_length = NO_SNAP_LIMIT;
	else
		snap_length = snap & ~0x3;

	if(snap_length != snap)
		dagutil_warning("snap length set to %d.\n", snap_length);
	variable_length = variable_length;

	/*
	 * reset warnings
	 */
	trunc_warning = 0;
	atm_warning = 0;
}

void
set_erf_record_alignment(int alignment)
{
	record_alignment = alignment;
}

/*
 * Write given ERF header and payload in ERF format.
 * Returns 1 if record successfully written and 0
 * otherwise.
 */

int
write_erf_record(dag_record_t * header, void *payload)
{
	int rlen;
	int wlen; /* record length, wire length */
	int payload_length; /* everything except ERF header */
	int data_length; /* payload_length less alignment */
	
	int alignment = 0; /* correction factor */
	int padding = 0; /* number of bytes of pad to add */
	int plen = 0; /* padding added */
	int	vpi,vci;

#ifdef PEDANTIC
	if(header == NULL) {
		dagutil_warning("Null record passed to write_erf_record\n");
		return 0;
	}
#endif


	/* check output is valid */
	if(outfile== NULL) {
		set_erf_output(outname);
	}

	rlen = ntohs(header->rlen);
	wlen = ntohs(header->wlen);
	data_length = payload_length = rlen - dag_record_size;
/*
	dump(header , rlen);
	dump(payload , payload_length);
*/

	/*
	 * for Ethernet packets there are 4 extra bytes not
	 * counted in the wlen field
	 */
	if ((header->type == TYPE_ETH) || (header->type == TYPE_COLOR_ETH))
		alignment = 4;
	data_length -= alignment;

/*	shjeon 0323	*/
#if 0
	/* complain about attempts to change ATM packets */
	if(!atm_warning && header->type == TYPE_ATM
	   && snap_length != NO_SNAP_LIMIT && snap_length != 52) {
		dagutil_warning("Attempt to alter snap_length of ATM traffic.\n");
		atm_warning = 1;
	}
#endif

	if(header->type == TYPE_ATM)	{
		vpi = (uint32_t)(ntohl (*((int *)(payload)))>>20)&0xff;
        vci = (uint32_t)(ntohl (*((int *)(payload)))>>4)&0xffff;
/*
		vpi = (uint32_t)(header->rec.atm.header)>>20&0xff;
        vci = (uint32_t)(header->rec.atm.header)>>4&0xffff;
*/
		printf ("#####     g_vpi [%d]  g_vci [%d]  vci [%d]  vpi [%d]   #####\n" ,
			g_vpi,g_vci,vci,vpi);
		
		if ((g_vci == -1) && (g_vpi != -1))	{
			/*	only VPI option	*/
			if (g_vpi != vpi)	{
				return 1 ;	
			}
		}
		else if ((g_vpi == -1) && (g_vci != -1))	{
			/* 	only VCI option	*/
			if (g_vci != vci)	{
				return 1;
			}
		}
		else if ((g_vpi != -1)	&& (g_vci != -1))	{
			if ((g_vci != vci) || (g_vpi != vpi))	{
				return 1;
			}
		}
		else if ((g_vci == -1) && (g_vpi == -1)) {
            /*  no option */		
        }
	}
#ifdef PEDANTIC
	if(rlen < dag_record_size) {
		dagutil_warning("Corruption of rlen field.\n");
		return 0;
	}
	if(payload == NULL && payload_length != 0) {
		dagutil_warning("Null payload when a payload was expected.\n");
		return 0;
	}
#endif

	if(variable_length != STATUS_QUO) {
		/*
		 * some modification of packet characteristics has been
		 * requested. The change could be going from fixed to
		 * variable, fixed to fixed, variable to fixed, or
		 * variable to variable, each with a change in snap
		 * length.
		 */

		if(variable_length == 0) {
			/*
			 * handle the situation where the caller has
			 * requested fixed length output. This usually
			 * means that some packets will need to be truncated
			 * and others padded to the correct length.
			 */

			header->flags.vlen = 0;
			header->rlen = htons(snap_length + dag_record_size + alignment);

			if(data_length > snap_length) {
				payload_length = snap_length + alignment;
			} else if(data_length < snap_length) {
				/* 
				 * need to pad the original data, but spit 
				 * out a warning if this occurs on previously
				 * truncated data, this warning is only printed
				 * once
				 */
				if(!trunc_warning && wlen > data_length) {
					dagutil_warning
					    ("Padding previously truncated packets.\n");
					trunc_warning = 1;
				}
				padding = snap_length - data_length;
			}
		} else {
			/*
			 * Variable length output is to be produced.
			 * Depending on whether a snap_length was
			 * specified some packets may have to be truncated.
			 * Also, snaps made with fixed length should have any
			 * padding removed.
			 */
			int was_variable = header->flags.vlen;

			header->flags.vlen = 1;

			/*
			 * if the current packet is fixed length, then we
			 * need to determine how much, if any, is padding.
			 * Any padding can then be neglected.
			 */
			if(!was_variable && wlen < data_length) {
				data_length = wlen;
				payload_length = data_length + alignment;
			}
			/*
			 * handle the situation where the requested snap
			 * length is less than the current packet length.
			 * This will nearly always involve truncation.
			 */
			if(snap_length != NO_SNAP_LIMIT
			   && data_length > snap_length)
			{
				header->rlen = htons(snap_length + dag_record_size + alignment);
				payload_length = snap_length + alignment;
			}
		}
	}

	if (record_alignment > 1)
	{
		int rem = (dag_record_size + payload_length + padding) % record_alignment;
		int extrapad = rem > 0 ? (record_alignment - rem) : 0;

		header->rlen = htons(ntohs(header->rlen)+extrapad);
		padding += extrapad;
	}
	
	/* Modify interface bits. */
	if (-1 != erf_output_iface)
	{
		header->flags.iface = erf_output_iface;
	}

	/* write header, return 0 if write fails */
	if(fwrite(header, dag_record_size, 1, outfile) != 1) {
		return 0;
	}
	/* write payload, return 0 if write fails */
	if(payload_length != 0
	   && fwrite(payload, payload_length, 1, outfile) != 1) {
		return 0;
	}

	/* write zero padding if required */
	while(padding>  0) {
		plen = padding<sizeof(ZERO)?padding:sizeof(ZERO);
		if(fwrite(ZERO, 1, plen, outfile) != plen) {
			return 0;
		}
		padding -= plen;
		bytes_written += plen;
	}

	if (rotate_size) {
		bytes_written += (dag_record_size + payload_length);
		if ((signed)bytes_written >= rotate_size) {
			printf("DEBUG: about to rotate\n");
			rotate_num++;
			bytes_written = 0;
			close_erf_output();
		}
	}

	return 1;
}



/*
 * Input stream 
 */
static FILE    *infile = NULL;
char * infilename = NULL;

/*
 * Reusable memory for records 
 */
static dag_record_t record;
/*
unsigned char cap_data[MAX_SIZE];
*/

/*
 * Next payload 
 */
static uint8_t    *payload = NULL;

/*
 * Set the stream from which input is obtained. If "in" is
 * null, then input is set to stdin.
 */
void
set_erf_input(char *in)
{
	infilename = in;

	if(in == NULL)
		infile = stdin;
	else {

#if defined(__linux__) || defined(__FreeBSD__) || (defined(__SVR4) && defined(__sun))

		int fd = open(in, O_RDONLY|O_LARGEFILE);
		if(fd == -1) {
			dagutil_panic("Could not open %s for reading.\n", in);
		}
		infile = fdopen(fd, "r");

#elif defined(_WIN32)

		infile = fopen(in, "rb");

#endif /* Platform-specific code. */

		if(infile == NULL) {
			dagutil_panic("Could not open %s for reading.\n", in);
		}
	}
}

/*
 * Close the current input stream.
 */
void
close_erf_input(void)
{
	if(infile != NULL && infile != stdin)
		fclose(infile);
	infile = NULL;
	if(payload != NULL)
		free(payload);
	payload = NULL;
}

/*
 * Get pointer to the ERF header for the next packet
 * in the input stream. Returns null if no further
 * packets are available.
 */ 


dag_record_t   *
get_next_erf_header(void)
{
	int rlen;

//#ifdef PEDANTIC
	if(infile == NULL) {
		dagutil_warning("Null input stream.\n");
		return NULL;
	}
//#endif

	if(feof(infile))
		return NULL;

	/* read the header into reusuable memory */
	if(fread(&record, dag_record_size, 1, infile) != 1) {
			return NULL;
	}

	rlen = ntohs(record.rlen);
	if (rlen == 20) {
		/* DS error truncates the packet, but the packet has effectively been padded to 28 bytes by the card. */
		rlen = 28;
	}
	rlen -= dag_record_size;

#ifdef PEDANTIC
	if(rlen < 0) {
		dagutil_panic("Record length corruption.\n");
	}
#endif

	/* allocate memory for and read the payload */
	if(rlen == 0) {
		/*
		 * In this case there is no payload, but we still want
		 * to return a non-null pointer. Therefore, if _payload
		 * is not yet allocate, get a single byte of memory
		 */
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
	
		if(fread(payload, 1, rlen, infile) != rlen) {
			dagutil_warning("Problem with payload.\n");
			return NULL;
		}
	}

	return &record;
}




/*
 * Returns a pointer to the payload of the most recent
 * packet. Returns null if there is no current packet.
 */
void*
get_erf_payload(void)
{
#if 0
	fprintf(stderr,"%02x : %02x --",*((unsigned char *)payload) , *((unsigned char *) (payload+1)));
	fprintf(stderr,"%02x : %02x \n",*((unsigned char *)payload+12) , *((unsigned char *) (payload+13)));
#endif
	return payload;
}
