/*
 * Copyright (c) 2005 Endace Technology Ltd, Hamilton, New Zealand.
 * All rights reserved.
 *
 * This source code is proprietary to Endace Technology Limited and no part
 * of it may be redistributed, published or disclosed except as outlined in
 * the written contract supplied with this product.
 *
 * $Id: dag_platform.h,v 1.1.1.1 2011/08/29 05:56:42 dcham Exp $
 */

#ifndef DAG_PLATFORM_H
#define DAG_PLATFORM_H


/* Cross-platform POSIX headers.
 * Headers that do not exist on ALL supported platforms should be in the relevant platform-specific headers.
 */
#include <fcntl.h>

/* Cross-platform C Standard Library headers.
 * Headers that do not exist on ALL supported platforms should be in the relevant platform-specific headers.
 */
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#if defined(__FreeBSD__)

#include "dag_platform_freebsd.h"

#elif defined(__linux__)

#include "dag_platform_linux.h"

#elif (defined(__APPLE__) && defined(__ppc__))

#include "dag_platform_macosx.h"

#elif (defined(__SVR4) && defined(__sun))

#include "dag_platform_solaris.h"

#elif defined(_WIN32)

#include "dag_platform_win32.h"

#else
#error Compiling on an unsupported platform - please contact <support@endace.com> for assistance.
#endif /* Platform-specific code. */
#include <regex.h>

#endif /* DAG_PLATFORM_H */
