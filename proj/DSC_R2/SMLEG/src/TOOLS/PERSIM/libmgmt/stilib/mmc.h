/*****************************************************************************
 @(#) $Id: mmc.h,v 1.1.1.1 2011/04/19 14:13:42 june Exp $
 -----------------------------------------------------------------------------
 Copyright (C) 2002 SoftTeleware, Inc. <http://www.softteleware.com>
 All Rights Reserved.

 Last Modified $Date: 2011/04/19 14:13:42 $ by $Author: june $
 *****************************************************************************/

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

#define const
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

extern void (*mmc_cb_err)(char*);
extern void mprintf(char * fmt, ...);
extern void regmmc(void* list);

#ifdef Linux
#if 0
#define DECLARE_VAR  							\
	va_list ap;									\
	char *arg[20] = { NULL, };					\
	char *arg1,*arg2,*arg3,*arg4,*arg5;			\
	char *arg6,*arg7,*arg8,*arg9,*arg10;		\
	char *arg11,*arg12,*arg13,*arg14,*arg15;	\
	char *arg16,*arg17,*arg18,*arg19,*arg20;	\
	int	 narg=0;								
												
#define GET_VAR_ARG(a...)  						\
	do {										\
	va_start(ap, ##a);							\
	while(arg[narg]=va_arg(ap, char*)) narg++;	\
	va_end(ap);									\
	arg1	= arg[0];							\
	arg2	= arg[1];							\
	arg3	= arg[2];							\
	arg4	= arg[3];							\
	arg5	= arg[4];							\
	arg6	= arg[5];							\
	arg7	= arg[6];							\
	arg8	= arg[7];							\
	arg9	= arg[8];							\
	arg10 	= arg[9];							\
	arg11	= arg[10];		 					\
	arg12	= arg[11];							\
	arg13	= arg[12];							\
	arg14	= arg[13];							\
	arg15	= arg[14];							\
	arg16	= arg[15];							\
	arg17	= arg[16];							\
	arg18	= arg[17];							\
	arg19	= arg[18];							\
	arg20 	= arg[19];							\
} while(0);
#else
#define DECLARE_VAR  							\
	va_list ap;									\
	char *arg[15] = { NULL, };					\
	char _arg[15][32];							\
	char *arg1,*arg2,*arg3,*arg4,*arg5;			\
	char *arg6,*arg7,*arg8,*arg9,*arg10;		\
	char *arg11,*arg12,*arg13,*arg14,*arg15;	\
	int	 narg=0;								
												
#define GET_VAR_ARG(a...)  						\
	do {										\
	va_start(ap, ##a);							\
	while((arg[narg]=va_arg(ap, char*)) && *arg[narg]) {		\
			bzero(_arg[narg], 32);				\
			strcpy(_arg[narg], arg[narg]);		\
			narg++; 							\
	}											\
	va_end(ap);									\
	arg1	= _arg[0];							\
	arg2	= _arg[1];							\
	arg3	= _arg[2];							\
	arg4	= _arg[3];							\
	arg5	= _arg[4];							\
	arg6	= _arg[5];							\
	arg7	= _arg[6];							\
	arg8	= _arg[7];							\
	arg9	= _arg[8];							\
	arg10 	= _arg[9];							\
	arg11	= _arg[10];		 					\
	arg12	= _arg[11];							\
	arg13	= _arg[12];							\
	arg14	= _arg[13];							\
	arg15	= _arg[14];							\
} while(0);




#endif

#endif

#ifdef __cplusplus
}
#endif


#endif
