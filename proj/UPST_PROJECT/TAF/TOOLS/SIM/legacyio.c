/*
 * $Id $
 */

/* dagconvert headers. */
#include "inputs.h"
#include "outputs.h"
#include "utils.h"

/* DAG tree headers. */
#include "dagapi.h"
#include "dagutil.h"

/* POSIX headers. */
#include <fcntl.h>
#ifndef _WIN32 
#include <netinet/in.h>
#include <unistd.h>
#endif /* _WIN32 */
/* C Standard Library headers. */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#ifndef	O_LARGEFILE
#define	O_LARGEFILE	0
#endif

/*
 * Input stream 
 */
static FILE    *infile = NULL;

typedef struct cell {
	int64_t		ts;
	uint32_t	crc;
	uint32_t	atm;
	uint16_t	llcsnap[4];
	uint32_t	pload[10];
} cell_t;

typedef struct ether {
	int64_t		ts;
	uint16_t	wlen;
	uint8_t	dmac[6];
	uint8_t	smac[6];
	uint16_t	etype;
	uint32_t	pload[10];
} ether_t;

typedef struct pos {			/* PoS comes with the new DAG record format */
	int64_t		ts;
	uint32_t	slen;		/* snap len in this record, must be 64 for now */
	uint32_t	wlen;		/* length of the packet on the wire, seems to include FCS */
	uint8_t	chdlc;		/* Cisco HDLC header */
	uint8_t	unused;		/* padding */
	uint16_t	etype;		/* EtherType */
	uint32_t	pload[11];	/* one more word than ATM and Ether */
} pos_t;

typedef union legacy_record {
	int64_t		ts;
	cell_t		cell;
	ether_t		ether;
	pos_t		pos;
} legacy_record_t;

/*
 * Reusable memory for records 
 */
static legacy_record_t	legacy_record;
static dag_record_t	record;

static int		input_type;

/*
 * Next payload 
 */
#ifndef _WIN32
static void    *payload = NULL;
#else /* _WIN32 */
static uint8_t    *payload = NULL;
#endif /* WIN32 */

/*
 * Set the stream from which input is obtained. If "in" is
 * null, then input is set to stdin.
 */
void
set_legacy_input(char *in, int in_type)
{
	int fd;

	input_type = in_type;

	if(in == NULL)
		infile = stdin;
	else {
		fd = open(in, O_RDONLY|O_LARGEFILE);
		if(fd == -1) {
			dagutil_panic("Could not open %s for reading.\n", in);
		}
		infile = fdopen(fd, "r");
		if(infile == NULL) {
			dagutil_panic("Could not open %s for reading.\n", in);
		}
	}
}

/*
 * Close the current input stream.
 */
void
close_legacy_input()
{
	if(infile != NULL && infile != stdin)
		fclose(infile);
	infile = NULL;
	payload = NULL;
}

/*
 * Get pointer to the ERF header for the next packet
 * in the input stream. Returns null if no further
 * packets are available.
 */
dag_record_t   *
get_next_legacy_header()
{

#ifdef PEDANTIC
	if(infile == NULL) {
		dagutil_warning("Null input stream.\n");
		return NULL;
	}
#endif

	if(feof(infile))
		return NULL;

	/* read the record into reusuable memory */
	if(fread(&legacy_record, sizeof(legacy_record), 1, infile) != 1) {
		return NULL;
	}

	record.ts = legacy_record.ts;
	record.type = input_type;
	switch (input_type) {
	case TYPE_ETH:
		{ uint8_t *wlp = (uint8_t *)&legacy_record.ether.wlen;
		record.rlen = htons(72);
		record.wlen = htons((wlp[1] << 8) + wlp[0]);
		/*
		 * The ERF payload which we are trying to emulate starts
		 * with two dummy bytes to force long alignment. So nuke
                 * the legacy wlen (which happens to be two bytes before
                 * the real payload) and point to it to achieve the same
                 * net payload
                 */ 
		legacy_record.ether.wlen = 0;
		payload = (uint8_t*)&legacy_record.ether.wlen;
		break;
		}
	case TYPE_HDLC_POS:
		record.rlen = htons(64);
		record.wlen = htons((uint16_t)ntohl(legacy_record.pos.wlen));
		payload = &legacy_record.pos.chdlc;
		break;
	default:
		record.rlen = htons(68);
		record.wlen = htons(52);
		payload = (uint8_t*)&legacy_record.cell.atm;
		break;
	}

	return &record;
}

/*
 * Returns a pointer to the payload of the most recent
 * packet. Returns null if there is no current packet.
 */
void*
get_legacy_payload()
{
	return payload;
}

/*
*	$Log $
*/
