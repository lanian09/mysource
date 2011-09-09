
#ifndef __LEG_SESSION_H__
#define __LEG_SESSION_H__

#include "comm_session.h"


extern stHASHOINFO             	*pHInfo_rad;
extern stTIMERNINFO            	*pTimer_rad;
extern _mem_check      			*gpShmem;
extern time_t                  	gSessTimeout;

extern void init_session (int shm_key);
extern rad_sess_body * get_rad_sess (rad_sess_key *pkey);
extern void del_rad_sess (rad_sess_key *pkey, rad_sess_body *pbody);
extern int del_rad_sess_key (rad_sess_key *pkey);
extern rad_sess_body *find_rad_sess (rad_sess_key *pkey);
//extern char * find_imsi_rad_sess (rad_sess_key *pkey);
extern rad_sess_body *add_rad_sess (rad_sess_key *pkey, rad_sess_body *pbody);
extern rad_sess_body *get_rad_interim_sess (rad_sess_key *pkey, int *interim_ret);
extern void rad_timer_reconstructor (void);
//extern void rad_timer_reconstructor (stHASHOINFO *, stTIMERNINFO *);

#endif /* __LEG_SESSION_H__ */
