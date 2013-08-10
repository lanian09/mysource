
#ifndef __LEG_SM_SESS_H__
#define __LEG_SM_SESS_H__

/** 0. HEADER INCLUDE **/
#include "leg.h"
#include "comm_session.h"

/** 1. CONSTANT DEFINITION **/
//#define MAX_SM_HASHO_SIZE							524287
//#define MAX_SM_HASHO_SIZE							294287
#define MAX_SM_HASHO_SIZE							81349
#define MAX_CSID_SIZE           					16
#define	MAX_PROPERTY_NUM							2
#define	MAX_PROPERTY_KEY_SIZE						10	
#define	MAX_PROPERTY_VAL_SIZE						10


/** 2. STRUCTURE DEFINITION **/
typedef struct _sm_sess_key_ {
	UINT			uiSID;      							// SM Handler NUMBER, Sequence ID

} SM_SESS_KEY, *PSM_SESS_KEY;
#define SM_SESS_KEY_SIZE	sizeof(SM_SESS_KEY)

typedef struct _sm_sess_body_ {
	//UINT    	uiMsgType;							// 0:none, 1:ACC_START, 2:ACC_INTERIM, 3:ACC_STOP, 4:DISCONNECT 
	UINT    	uiOperMode;							// 0:none, 1:login, 2:logout
	TIMERNID	ullTimerID;
	SUBS_INFO	stSubs;
	//U8			ucKey[MAX_PROPERTY_NUM][MAX_PROPERTY_KEY_SIZE];
	//U8			ucVal[MAX_PROPERTY_NUM][MAX_PROPERTY_VAL_SIZE];

} SM_SESS_BODY, *PSM_SESS_BODY;
#define SM_SESS_BODY_SIZE	sizeof(SM_SESS_BODY)


/** 3. GLOBAL VARIABLE DEFINITION **/
extern stHASHOINFO             	*gpHashInfo_SM;
extern stTIMERNINFO            	*gpHashTimer_SM;
extern _mem_check      			*gpShmem;
extern time_t                  	glSessTimeout;


/** 4. GLOBAL FUNCTION PROTOTYPE DEFINITION **/
extern void init_sm_sess (int shm_key);
extern SM_SESS_BODY *get_sm_sess (SM_SESS_KEY *key);
extern SM_SESS_BODY *find_sm_sess (SM_SESS_KEY *key);
extern SM_SESS_BODY *add_sm_sess (SM_SESS_KEY *key, SM_SESS_BODY *body);
extern void del_sm_sess (SM_SESS_KEY *key, SM_SESS_BODY *body);
extern int del_sm_sess_key (SM_SESS_KEY *pkey);
extern void sm_timer_reconstructor (void);
extern int sm_sess_timeout (SM_SESS_KEY *pkey);

#endif /* __LEG_SM_SESS_H__ */
