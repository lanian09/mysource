
#ifndef __SESSION_H__
#define __SESSION_H__

#include "timerN.h"
#include "hashg.h"
#include "hasho.h"
#include "memg.h"
#include "uepoll.h"

extern stHASHOINFO	*pHashInfo;
extern stTIMERNINFO	*pTmrNInfo;
extern int sess_gSessionCnt;

#if 1

#define FD_HASHO_OFFSET(ptr)  ((OFFSET) (((U8 *) (ptr)) - ((U8 *) pHashInfo)) )
#define FD_HASHO_PTR(offset)  ((U8 *) (((OFFSET) (offset)) + ((OFFSET) pHashInfo)) )

#define MAX_FD_HASHO_SIZE		5000
#define MAX_TCP_OPEN    		4000
#define MAX_BUF_SIZE        	2048


typedef struct _st_sockfd {
    int             sockfd;
} hs_key;

#define HKEY_FD_SIZE  sizeof(hs_key)

enum {CLI_CONNECT=0, CLI_REQ, CLI_REQ_DONE, CLI_RSP, CLI_CLOSE};

typedef struct _st_sockfd_body {
    struct sockaddr_in 	cli_sa;			/**< Client Address **/
    unsigned int    dest_ipaddr;		/**< Server IP */
    unsigned short  dest_port;			/**< Server Port */
	int 			efd;
	int 			cfd;
	int				cli_status;
	TIMERNID		tmr_id;
	int 			timeout;
	int				rlen;
	char 			rbuf[1024*2];
	char			*data;
} hs_body;

#define HBODY_FD_SIZE  sizeof(hs_body)

extern hs_body * find_fd_session (hs_key *hkey);
extern hs_body * add_fd_session (hs_key *hkey, hs_body *pstBody);
extern void del_fd_session (int sockfd);
extern hs_body * get_fd_session (hs_key *hkey);
extern void sessionRelease (int efd, int cfd, long tid);
extern void invoke_send_wait_timeout (hs_key *pkey);
#endif

#endif /* __SESSION_H__*/
