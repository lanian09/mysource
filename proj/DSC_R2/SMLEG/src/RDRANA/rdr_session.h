
#ifndef __LEG_SESSION_H__
#define __LEG_SESSION_H__

//#include "leg.h"

#include "memg.h"
#include "hasho.h"
#include "timerN.h"

#define RAD_HASHO_OFFSET(INFOPTR, ptr)  ((OFFSET) (((U8 *) (ptr)) - ((U8 *) INFOPTR)) )
#define RAD_HASHO_PTR(INFOPTR, offset)  ((U8 *) (((OFFSET) (offset)) + ((OFFSET) INFOPTR)) ) 

extern stHASHOINFO             *pHInfo_rad;

#define MAX_RAD_HASHO_SIZE		200000

typedef struct _rad_sess_key_ {
	U32		mobIP;
} rad_sess_key;
#define RAD_SESS_KEY_SIZE	sizeof(rad_sess_key)

#define MAX_SESS_IMSI_SIZE			16
typedef struct _rad_sess_body_ {
	U8				IMSI[MAX_SESS_IMSI_SIZE];
	TIMERNID		timerID;
	short			sPkgID;
}rad_sess_body;
#define RAD_SESS_BODY_SIZE	sizeof(rad_sess_body)

extern void init_session (int shm_key);
extern rad_sess_body *find_rad_sess (rad_sess_key *pkey);
//extern char * find_imsi_rad_sess (rad_sess_key *pkey);

#endif
