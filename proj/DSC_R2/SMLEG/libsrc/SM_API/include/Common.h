#ifndef __OSAL_COMMON_H__
#define __OSAL_COMMON_H__               1

#include "BasicTypes.h"

#ifdef __cplusplus
    #define OSAL_EXTERN extern "C"
#else
    #define OSAL_EXTERN 
#endif

#ifdef WIN32
#ifndef inline
#define inline  __inline
#endif

#ifndef snprintf
#define snprintf _snprintf
#endif

#ifdef SMAPI_EXPORTS
#define OSAL_DllExport   __declspec( dllexport )
#else 
/* seen by the importing source code of the dll */
#define OSAL_DllExport   __declspec( dllimport )
#endif

#else
/* non win32 definitions */
#define OSAL_DllExport   

#endif


/* The following types are declared by some VxWorks .h files, and they are used
   By ansi-c files which do not allow multiple typedefs of the same type. */
#ifndef __INCvxTypesOldh
typedef int         STATUS;
//typedef int         BOOL;
typedef unsigned int bool;


#ifdef __cplusplus
typedef int         (*FUNCPTR) (...);   /* pfunction returning int */
typedef void        (*VOIDFUNCPTR) (...);   /* pfunction returning void */
#else
typedef int         (*FUNCPTR) ();  /* pfunction returning int */
typedef void        (*VOIDFUNCPTR) ();  /* pfunction returning void */
#endif

#endif

#if !defined(FALSE) /* || (FALSE!=0) */
#define FALSE       0
#endif

#if !defined(TRUE) /* || (TRUE!=1) */
#define TRUE        1
#endif


#ifndef OK
#define OK 0
#endif


#define LEG_ERROR (-1)

/* Here we teach C compilers about bool. We need to ensure that their bool */
/* is the same bool as the C++, so we define it by the compiler. */
/*
#ifndef __cplusplus
  #ifdef __GNUC__
    #if CPU == PPC604
      typedef unsigned int bool;
    #elif CPU == PENTIUM
      typedef unsigned char bool;
    #else
      #error PLEASE CHECK WHAT IS THE RIGHT DEFINITION FOR THIS COMPILER.
    #endif
  #else
    #ifdef _WINDOWS
      typedef unsigned char bool;
    #else
      #error PLEASE CHECK WHAT IS THE RIGHT DEFINITION FOR THIS COMPILER.
    #endif
  #endif
#endif
*/
/* Existing VxWorks BSP code uses this type, we supply it only for such projects. */
/*
#ifdef TYPEDEF_BOOLEAN
typedef bool boolean;
#endif
*/
/* Status */
typedef enum status {FAIL, SUCCESS} Status;



#ifndef ENABLE
#define ENABLE  1
#endif

#ifndef DISABLE
#define DISABLE 0
#endif

/* removed the rest of SANB_Common.h from here */

/* some additions */

/* Used both by SANB_SemTake() and by SANB_MsgQReceive() */
#define OSAL_WAIT_FOREVER        (-1)
#define OSAL_NO_WAIT              (0)


#ifndef MIN
#define MIN(a,b)				((a <= b)?a:b)
#endif 

#ifndef MAX
#define MAX(a,b)				((a >= b)?a:b)
#endif 

#define OUT
#define IN

#endif



