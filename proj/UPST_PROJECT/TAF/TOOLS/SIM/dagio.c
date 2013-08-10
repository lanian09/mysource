/*
 * $Id: dagio.c,v 1.1.1.1 2011/08/29 05:56:42 dcham Exp $
 */

/* dagconvert headers. */
#include "inputs.h"
#include "utils.h"

/* Endace headers. */
#include "dagapi.h"
#include "dagutil.h"

/*
 * Input stream 
 */
#define DAGNAME_BUFSIZE 128

#if defined(__linux__) || defined(__FreeBSD__) || (defined(__SVR4) && defined(__sun)) || (defined(__APPLE__) && defined(__ppc__))
static char dagname[DAGNAME_BUFSIZE] = "/dev/dag0";
#elif defined(_WIN32)
static char dagname[DAGNAME_BUFSIZE] = "\\\\.\\dag0";
#endif /* _WIN32 */
static int dagfd;

/*
 * Reusable memory for records 
 */
/*
 * Next payload 
 */
#if defined(__linux__) || defined(__FreeBSD__) || (defined(__SVR4) && defined(__sun)) || (defined(__APPLE__) && defined(__ppc__))

static void *payload = NULL;
static void *dag_mem;

#elif defined(_WIN32)

static char *payload = NULL;
static char *dag_mem;

#endif /* Platform-specific code. */

static uint32_t bottom;
static uint32_t top;

/*
 * Set the stream from which input is obtained.
 */
void
set_dag_input(const char* name, char *args)
{
	/*
	 * setup device parameters 
	 */
	if((dagfd = dag_open((char*) name)) < 0)
		dagutil_panic("dag_open %s: %s\n", name, strerror(errno));

	if(dag_configure(dagfd, args == NULL ? "" : args) < 0)
		dagutil_panic("dag_configure %s: %s\n", name, strerror(errno));

	if((dag_mem = dag_mmap(dagfd)) == MAP_FAILED)
		dagutil_panic("dag_mmap %s: %s\n", name, strerror(errno));

	if(dag_start(dagfd) < 0)
		dagutil_panic("dag_start %s: %s\n", name, strerror(errno));

	/*
	 * Important! You have to ensure bottom is properly
	 * initialized to zero on startup, it won't give you
	 * a compiler warning if you make this mistake!
	 */
	bottom = 0;
	top = 0;

	strncpy(dagname, name, DAGNAME_BUFSIZE);
}

/*
 * Called when the DAG input is no longer needed so that any 
 * special shutdown tasks can be performed.
 */
void
close_dag_input()
{
	if(dag_stop(dagfd) < 0)
		dagutil_panic("dag_stop %s: %s\n", dagname, strerror(errno));
	if(dag_close(dagfd) < 0)
		dagutil_panic("dag_close %s: %s\n", dagname, strerror(errno));
}

/*
 * Get pointer to the ERF header for the next packet
 * in the input stream. Returns null if no further
 * packets are available.
 */
dag_record_t   *
get_next_dag_header()
{
	register dag_record_t *record;
	int             rlen;

#ifdef PEDANTIC
	if(dagfd < 0) {
		fprintf(stderr, "DAG not set up.\n");
		return NULL;
	}
#endif
	if(top - bottom < dag_record_size) {
		top = dag_offset(dagfd, &bottom, 0);
	}

	record = (dag_record_t *) (dag_mem + bottom);
	rlen = ntohs(record->rlen);
	if (rlen == 20) {
		/* DS error truncates the packet, but the packet has effectively been padded to 28 bytes by the card. */
		rlen = 28;
	}

	while((signed)(top - bottom) < rlen) {
		top = dag_offset(dagfd, &bottom, 0);
		record = (dag_record_t *)(dag_mem + bottom);
		rlen = ntohs(record->rlen);
		if (rlen == 20) {
			/* DS error truncates the packet, but the packet has effectively been padded to 28 bytes by the card. */
			rlen = 28;
		}
	}

	payload = ((char *) record) + dag_record_size;
	bottom += rlen;

	return record;
}

/*
 * Returns a pointer to the payload of the most recent
 * packet. Returns null if there is no current packet.
 */
void           *
get_dag_payload()
{
	return payload;
}

/*
*	$ Log : $
*/

