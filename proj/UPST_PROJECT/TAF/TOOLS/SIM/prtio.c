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


/*
 * Output stream. 
 */
static FILE    *outfile;

/*
 * Set the stream on which output is to be produced.
 */
void
set_prt_output(char *out)
{
	int fd;

	if(out == NULL)
		outfile = stdout;
	else {
		fd = open(out, O_RDWR|O_CREAT|O_TRUNC|O_LARGEFILE, 0664);
		if(fd == -1) {
			dagutil_panic("Could not open %s for writing.\n", out);
		}
		outfile = fdopen(fd, "w");
		if(outfile == NULL) {
			dagutil_panic("Could not open %s for writing.\n", out);
		}
	}
}

/*
 * Close the currently open input (if any).
 */
void
close_prt_output()
{
	if(outfile != NULL && outfile != stdout)
		fclose(outfile);
	outfile = NULL;
}

/*
 * Return an ASCII string name for a record type.
 */
static char*
gpp_name(uint8_t type)
{
	static char retbuf[64];

	switch (type) {
		case TYPE_LEGACY:
			return "LEGACY";
			
		case TYPE_HDLC_POS:
			return "ERF PoS";
			
		case TYPE_COLOR_HDLC_POS:
			return "ERF Colored PoS";
			
		case TYPE_ETH:
			return "ERF Ethernet";
			
		case TYPE_COLOR_ETH:
			return "ERF Colored Ethernet";
			
		case TYPE_ATM:
			return "ERF ATM";
			
		default:
			snprintf(retbuf, 64, "unknown ERF type %d\n", type);
			return retbuf;
	}
}

/*
 * Counts number of packets printed 
 */
static uint64_t packet_counter = 0ULL;

/*
 * Write an ASCII representation of a record. If the record
 * is successfully written 1 is returned.
 */
int
#if defined(__FreeBSD__) || defined(__linux__) || (defined(__SVR4) && defined(__sun)) || (defined(__APPLE__) && defined(__ppc__))
write_prt_record(dag_record_t * rec, void *payload)
#else /* _WIN32 */
write_prt_record(dag_record_t * rec, char *payload)
#endif /* _WIN32 */
{

	int             i;
	time_t          time;
	struct tm      *dtime;
	char            dtime_str[80];
	int             len;

	char           *pload = (char*)payload;
	char            abuf[17];

#ifdef PEDANTIC
	if(rec == NULL)
		dagutil_panic("Null record header passed to write_prt_record\n");
#endif

	packet_counter++;
	len = ntohs(rec->rlen);
	time = (time_t) (rec->ts >> 32);
	dtime = gmtime((time_t *) (&time));
	strftime(dtime_str, sizeof(dtime_str), "%Y-%m-%d %T", dtime);
	fprintf(outfile, "ts=0x%.16"PRIx64" %s.%07.0f UTC\n",
		rec->ts, dtime_str,
		(double) (rec->ts & 0xffffffffll) / 0x100000000ll *
		10000000);
	fprintf(outfile, "type: %s\n", gpp_name(rec->type));

	if(!rec->type) {
		dagutil_panic("Did not expect legacy data here.\n");
	} else {		/* ERF format */
		fprintf(outfile,
	"dserror=%d rxerror=%d trunc=%d vlen=%d iface=%d rlen=%d lctr=%d wlen=%d\n",
			(int) rec->flags.dserror, (int) rec->flags.rxerror,
			(int) rec->flags.trunc, (int) rec->flags.vlen,
			(int) rec->flags.iface, ntohs(rec->rlen),
			ntohs(rec->lctr), ntohs(rec->wlen));
		switch (rec->type) {
		  case TYPE_HDLC_POS:
		  case TYPE_COLOR_HDLC_POS:
			{
			  pos_rec_t *pos = (pos_rec_t *)payload;
			  fprintf(outfile, "hdlc header=0x%08x\n", (uint32_t) ntohl(pos->hdlc));
			  break;
			}
		  case TYPE_ETH:
		  case TYPE_COLOR_ETH:
			{
			  eth_rec_t *eth = (eth_rec_t *)payload;
			  fprintf(outfile, "offset=%d etype=0x%04x\n",
				  eth->offset,
				  ntohs(eth->etype));
			  fprintf(outfile,
	  "dst=%.2x:%.2x:%.2x:%.2x:%.2x:%.2x src=%.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n",
				  eth->dst[0], eth->dst[1],
				  eth->dst[2], eth->dst[3],
				  eth->dst[4], eth->dst[5],
				  eth->src[0], eth->src[1],
				  eth->src[2], eth->src[3],
				  eth->src[4],
				  eth->src[5]);
			  if(payload != NULL)
				  pload = (payload + 2);
			  break;
			}
		  case TYPE_ATM:
			{
			  atm_rec_t *atm = (atm_rec_t *)payload;
			  fprintf(outfile, "ATM header=0x%08x\n", (uint32_t) ntohl(atm->header));
			  break;
			}
		  default:
			  fprintf(outfile,
		  "print %"PRIu64": record printing not implemented for ERF type %d\n",
				  packet_counter, rec->type);
		}
	}
	if(payload != NULL) {
		if(dagutil_get_verbosity() == 1)
			for(i = 0; i < len; i++)
				fprintf(outfile, "%02x%s", *pload++ & 0xFF,
					((i & 0xF) == 0xF) ? "\n" : " ");
		else if(dagutil_get_verbosity() > 1) {
			memset(abuf, 0, 17);
			for(i = 0; i < len; i++) {
				abuf[i & 0xF] =
				    isprint(*pload & 0xFF) ? (*pload &
							      0xFF) : '.';
				fprintf(outfile, "%02x ", *pload++ & 0xFF);
				if((i & 0xF) == 0xF) {
					fprintf(outfile, "        %s\n",
						abuf);
					memset(abuf, 0, 17);
				}
			}
			if((i & 0xF) != 0xF) {
				while(i++ & 0xF) {
					fprintf(outfile, "   ");
				}
				fprintf(outfile, "        %s\n", abuf);
			}
		}
	}
	fprintf(outfile, "\n");

	return 1;
}

/*
 * Set the stream from which input is obtained.
 */
void
set_prt_input(char *in)
{
	dagutil_panic("prt is not a valid input device.\n");
}

/*
 * Close the current input stream.
 */
void
close_prt_input()
{
	dagutil_panic("prt is not a valid input device.\n");
}

/*
 * Get pointer to the ERF header for the next packet
 * in the input stream. Returns null if no further
 * packets are available.
 */
dag_record_t   *
get_next_prt_header()
{
	dagutil_panic("prt is not a valid input device.\n");
	return NULL;
}

/*
 * Returns a pointer to the payload of the most recent
 * packet. Returns null if there is no current packet.
 */
void           *
get_prt_payload()
{
	dagutil_panic("prt is not a valid input device.\n");
	return NULL;
}

/*
*	$Log $
*/
