#ifndef __CONFLIB_H__
#define __CONFLIB_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/errno.h>
 
#define CONFLIB_MAX_TOKEN_LEN   64
 
extern int errno;

extern int conflib_getNthTokenInFileSection (char*, char*, char*, int, char*);
extern int conflib_getTokenCntInFileSection (char*, char*, char*);
extern int conflib_getNTokenInFileSection (char*, char*, char*, int, char[][CONFLIB_MAX_TOKEN_LEN]);
extern int conflib_getStringInFileSection (char*, char*, char*, char*);
extern int conflib_seekSection (FILE*, char*);
extern int conflib_getStringInSection (FILE*, char*, char*);


#endif /*__CONFLIB_H__*/

