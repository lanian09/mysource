/*
   $Id: mmc.h,v 1.1.1.1 2011/04/19 14:13:43 june Exp $

 	-----------------------------------------------------------------------------
	Copyright (C) 2004-2005 by LINK@sys Inc, Korea
	All Rights Reserved.
*/

#ifndef MMC_H
#define MMC_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

#undef BUFSIZ
#define BUFSIZ	8192

//#define const
#define volatile

#define T_NONE      0x0000
#define T_IPADDR    0x0001      /* I */
#define T_NUMERIC   0x0010      /* N */
#define T_STRING    0x0100      /* S */
#define T_COMMAND   0x1000      /* C */

#undef BUFSIZE
#define BUFSIZE 64

typedef struct _hist_ {
    struct _hist_ *next;
    struct _hist_ **prev;
    int  mmcid;
    char s[BUFSIZ];            /* command string           */
} hist_t;

typedef struct _mmc_ {
    char s[BUFSIZE];            /* command string           */
    int  depth;                 /* mmc command depth        */
    int  cont;                  /* continue or not          */
    char type;                  /* command token type       */
    char f[BUFSIZE];            /* command parsed format    */
    char c[BUFSIZE];            /* comment for help         */
    void (*func)(char*,...);    /* callback function        */
} mmc_t;

extern mmc_t *mmclist;

typedef struct _token_ {
    char    s[BUFSIZE];     /* s: token string  */
    int     t;              /* t: token type    */
} token_t;


extern char mmcbuf[BUFSIZ],locbuf[BUFSIZ];
extern int 	mmcfd,mmifd;
extern int 	mmcfunc;
extern int 	mmcsucc;
extern struct sockaddr_in 	mmcsin,mmisin;


extern char* p_mmcbuf;
extern void (*callback_mmc_func)();
extern int mmcgets(char* s);
extern void mmchelp(char*,...);
extern int pparse(char *cmd,int cont);
extern int mparse(char *cmd,int cont,int fd,struct sockaddr_in sin);
extern void mprintf(char * fmt, ...);

extern void regmmc(void* list);
#ifdef __cplusplus
}
#endif


#endif


/*
	$Log: mmc.h,v $
	Revision 1.1.1.1  2011/04/19 14:13:43  june
	성능 패키지
	
	Revision 1.1.1.1  2011/01/20 12:18:51  june
	DSC CVS RECOVERY
	
	Revision 1.1  2009/05/09 09:41:01  dsc
	init
	
	Revision 1.1  2008/12/12 00:07:21  yhshin
	*** empty log message ***
	
	Revision 1.3  2008/11/24 14:25:22  sjjeon
	*** empty log message ***
	
	Revision 1.2  2008/11/22 12:27:02  yhshin
	수정
	
	Revision 1.1  2008/11/06 05:43:03  sjjeon
	oam lib
	
	Revision 1.1.1.1  2008/10/20 06:55:48  sjjeon
	accelerator server
	
	Revision 1.1  2008/01/11 12:14:34  june
	*** empty log message ***
	
	Revision 1.3  2005/05/30 01:57:01  yhshin
	cum
	
	Revision 1.2  2005/03/30 10:44:00  yhshin
	다른 mmc로 변경
	
	Revision 1.1  2005/01/28 07:20:14  yhshin
	include file init.
 */
