#ifndef _STDARG_H_
#define _STDARG_H_

#undef __P
#undef __V

#ifdef __STDC__
#include <stdarg.h>
#define __V(x)  x
#define __P(x)  x
#else
#include <varargs.h>
#define __V(x)  (va_alist) va_dcl
#define __P(x)  ()
#define const
#endif

#endif
