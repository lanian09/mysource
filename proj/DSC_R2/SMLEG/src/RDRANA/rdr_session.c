/* File Include */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/shm.h>

#include "define.h"
#include "utillib.h"
#include "comm_session.h"




/* Definition of New Constants */
stHASHOINFO             *pHInfo_rad;

rad_sess_body *find_rad_sess (rad_sess_key *pkey)
{
	stHASHONODE             *pstHashNode;
	rad_sess_body			*pBody;

	pstHashNode = hasho_find (pHInfo_rad, (U8 *)pkey);
	if (pstHashNode == NULL) {
		return NULL;
	}

	pBody = (rad_sess_body *)RAD_HASHO_PTR (pHInfo_rad, pstHashNode->offset_Data);
	
	return pBody;
}
#if 0
char * find_imsi_rad_sess (rad_sess_key *pkey)
{
	rad_sess_body           *pBody;

	pBody = find_rad_sess(pkey); 
	if (pBody != NULL) {
		return (pBody->IMSI);
	}

	return NULL;
}
#endif

void init_session (int shm_key)
{

    pHInfo_rad = hasho_init (shm_key/*SHM_RAD_SESS_KEY*/, sizeof (rad_sess_key), sizeof(rad_sess_key), sizeof(rad_sess_body), MAX_RAD_HASHO_SIZE, NULL, 0);
    if (pHInfo_rad == NULL) {
        dAppLog (LOG_CRI, "FAIL] sid hasho_init ERROR[%d][%s]", errno, strerror(errno));
        exit(1);
    }
}

