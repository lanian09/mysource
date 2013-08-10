/* 
    $Id: conf.c,v 1.1.1.1 2011/04/19 14:13:46 june Exp $

    DATE        : 2006.4.18
    FILE_NAME   : 


    Copyright (c) 2005-2006 by uPRESTO Inc, Korea
    All rights reserved.
*/

#include <stdio.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>

#include "capd_def.h"
#include <utillib.h>
#include "conf.h"

int		ifswap_flag = 0;
int		mirror_flag = 0;
int		save_flag = 0;


void
to_lower(char *s)
{
	char    *p;

	for (p = s; *p; p++) {
		*p = tolower(*p);
	}
}

char *
del_front_whitespace(char *s)
{
	while (*s != '\0') {
		if (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\r') {
			*s++ = '\0';
		}
		else {
			break;
		}
	}
	return s;
}

void
del_back_whitespace(char *s)
{
	char	*c;

	c = s + strlen(s)-1;

	while (c >= s) {
		if (*c == ' ' || *c == '\t' || *c == '\n' || *c == '\r') {
			*c-- = '\0';
		}
		else {
			return;
		}
	}
}

void
del_comment(char *s)
{
	char	*p;

	if ((p = strchr(s, '#')) != NULL) {
		*p = '\0';
	}
}

void
del_separator(char *s)
{
	char	*p;

	if ((p = strchr(s, '=')) != NULL) {
		*p = ' ';
	}
}

char *
value_comma(char *s)
{
	static char		*p = NULL;
	char			*token;

	if (s != NULL) {
		p = s;
	}
	if (p == NULL) {
		return NULL;
	}
	token = del_front_whitespace(p);
	del_back_whitespace(token);
	if ((p = strchr(token, ',')) != NULL) {
		*p++ = '\0';
		del_back_whitespace(token);
	}
	return token;
}

int
get_line(FILE *fp, char *key, char *value, int *direc)
{
	char	buf[BUFSIZ];
	char	*s;

	/* check if eof */
	if (fgets(buf, BUFSIZ, fp) == NULL) {
		return -1;
	}

	s = buf;
	s = del_front_whitespace(s);
	del_comment(s);
	del_separator(s);

	to_lower(s);
	
	if (*s == '\0') {
		return 0;
	}

	if (sscanf(s, "%s %s %d", key, value,direc) != 3) {
		return 0;
	}
	else {
		return 3;
	}
}

int
is_num(char *n)
{
	char	*p;

	for (p = n; *p; p++) {
		if (!isdigit(*p)) {
			return 0;
		}
	}
	return 1;
}

int
is_hexnum(char *n)
{
	char 	*p;

	if (strncmp(n, "0x", 2) && strcmp(n, "0X")) {
		return 0;
	}

	p = n+2;
	if (*p == '\0') {
		return 0;
	}
	while (*p != '\0') {
		if (!isxdigit(*p++)) {
			return 0;
		}
	}
	return 1;
}

void
get_config(char *path)
{
	FILE	*fp;
	char	key[BUFSIZ], value[BUFSIZ];
	int 	direc;
	int		key_val;

	if ((fp = fopen(path, "r")) == NULL) {
		dAppLog(LOG_CRI, "%s : cannot open configuration file (%s)", __FUNCTION__, path);
		exit(1);
	}
	while ((key_val = get_line(fp, key, value, &direc)) != -1) {
		if (key_val == 3) {
//			do_config(key, value,&direc);
			;
		}
	}
	fclose(fp);
}
