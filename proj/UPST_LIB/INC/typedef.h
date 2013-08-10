/**
 *	common_stg.h 에서 가져온 type
 */
#ifndef	__TYPEDEF_H__
#define	__TYPEDEF_H__

#define DEF			long
#define FLOAT		float
#define IP4			int
#define MTIME		int
#define OFFSET		long
#define S16			short
#define S32			int
#define S64			long long
#define S8			char
#define STIME		int
#define STRING		unsigned char
#define U16			unsigned short
#define U32			unsigned int
#define U64			unsigned long long
#define U8			unsigned char
#define UTIME64		unsigned long long
#define X8			unsigned char

/**
 *	TAF/INC/typedef.h 에서 가져온 type
 */
/**
 *	WATAS, NTAS, WNTAS, IPAS에서 사용하던 typedef
 */
typedef int					INT;
typedef int 				BOOL;
typedef unsigned int		UINT;
typedef unsigned int		DWORD;
typedef char				CHAR;
typedef char				TCHAR;
typedef unsigned char		UCHAR;
typedef unsigned char		BYTE;
typedef short				SHORT;
typedef unsigned short		USHORT;
typedef unsigned short		WORD;
typedef long				LONG;
typedef long long			INT64;
typedef unsigned long		ULONG;
typedef unsigned long long	UINT64;

typedef char				*PTCHAR;
typedef unsigned char		*PUCHAR;

typedef struct timeval		st_TimeVal;

typedef struct _STRING_SHORT
{
    char    szValue[40];
    USHORT  usValue;
} string_short;


/**
 *	TAM/INC/typedef.h 에서 가져온 type
 */

typedef struct _st_timeval_t {
    int tv_sec;
    int tv_usec;
} st_timeval_t;

#endif	/* __TYPEDEF_H__ */
