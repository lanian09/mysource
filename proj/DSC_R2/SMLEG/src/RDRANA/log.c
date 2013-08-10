#include <shmutillib.h>
#include <PI/tmf_cap.h>
#include "capd_def.h"
#include "capd_api.h"

void print_pktbuf(int level, st_pktbuf_array pktbuf[])
{
	dAppLog(level, "---------- ARRAY0 [%d] ----------", pktbuf[0].used_count);
	print_pktbuf_array(level, &pktbuf[0]);
	dAppLog(level, "---------- ARRAY1 [%d] ----------", pktbuf[1].used_count);
	print_pktbuf_array(level, &pktbuf[1]);
}

void print_pktbuf_array(int level, st_pktbuf_array *pktbuf)
{
	int idx = pktbuf->head;
	Capture_Header_Msg *p_hdr;

	while( idx >= 0 && idx < MAX_NODE )
	{
		p_hdr = (Capture_Header_Msg *)pktbuf->node[idx].buffer;
		dAppLog(level, "INDEX[%d] CAPTIME[%ld.%06ld] NEXT[%d]",
			idx, p_hdr->curtime, p_hdr->ucurtime, pktbuf->node[idx].next);
		idx = pktbuf->node[idx].next;
	}

	if( idx < -1 || idx >= MAX_NODE )
	{
		// Broken linked list.
		dAppLog(LOG_CRI, "LINKED LIST to be initialized.");
	}
}


