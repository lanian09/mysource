/*****************************************************************************
 @(#) $Id: conf.h,v 1.1.1.1 2011/04/19 14:13:58 june Exp $
 -----------------------------------------------------------------------------
 Copyright (C) 2002 SoftTeleware, Inc. <http://www.softteleware.com>
 All Rights Reserved.

 Last Modified $Date: 2011/04/19 14:13:58 $ by $Author: june $
 *****************************************************************************/

#ifndef CONF_H
#define CONF_H

#define CONFLEN BUFSIZ

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _cmdtab_ {
	char opmode[CONFLEN];
	char cmd[CONFLEN];
} CMDTAB;	

typedef struct _conftab_ {
	char label[CONFLEN];
	char val[CONFLEN];
} CONFTAB;

/* get current configuration information from file 
 * returns Upon successful completion, >0 returned
 * otherwise, -1 is return
 */
int getconf(char* path	/* fullpath of conf file  */,
			char* label	/* label name */,
			char* val	/* sting for getting value */);

/* strip all white space from input string-buffer
 */
void whitespace (char *s	/* string */);

/* btrim(right/left) input string
 */
void btrim(char *s	/* string */);
void btrip(char *s);

/* convert stiring to lowercase
 * returns case converted string pointer
 */
char *strlwr(char *s	/* string */);

/* convert stiring to uppercase
 * returns case converted string pointer
 */
char *strupr(char *s	/* string */);

int readconf(char* path,CONFTAB* tab);
int findlabel(char* path,char* label);
int modconf(char* path,char* label,char* val,char* comment);

#ifdef __cplusplus
}
#endif

#endif

