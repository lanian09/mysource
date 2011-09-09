/*
 * $Id $
 */

#include "dagnew.h"
#include "dagapi.h"

#include <stdlib.h>
#include <stdio.h>
#ifndef _WIN32
#include <unistd.h>
#include <netinet/in.h>
#endif /* _WIN32 */
#include "inputs.h"
#include "outputs.h"
#include "utils.h"

/*
 * Stub function, does nothing.
 */
void
set_null_output(char *out)
{
}

/*
 * Stub function does nothing.
 */
void
close_null_output()
{
}

/*
 * Discard ERF header and payload.
 */
int
write_null_record(dag_record_t * header, void *payload)
{
	return 1;
}

/*
 * Stub function, does nothing.
 */
void
set_null_input(char *in)
{
}

/*
 * Stub function, does nothing.
 */
void
close_null_input()
{
}

/*
 * Returns NULL.
 */
dag_record_t   *
get_next_null_header()
{
	return NULL;
}

/*
 * Returns NULL.
 */
void           *
get_null_payload()
{
	return NULL;
}

/*
*	$Log	$
*/
