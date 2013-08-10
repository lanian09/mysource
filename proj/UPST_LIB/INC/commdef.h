/**		file	commdef.h
 *		- hash header file
 *
 *	 Copyright (c) 2006~ by Upresto Inc, Korea
 *	 All rights reserved
 *
 *   $Id: commdef.h,v 1.6 2011/09/04 11:51:53 dhkim Exp $
 * 
 *	 @Author	$Author: dhkim $
 *	 @version	$Revision: 1.6 $
 *	 @date		$Date: 2011/09/04 11:51:53 $
 *	 @warning	$type???로 정의된 것들만 사용가능
 *	 @todo		Makefile을 만들자
 *
 *	 @section	 Intro(소개)
 *		- hash header file
 *
 *	 @section	 Requirement
 *		@li 규칙에 틀린 곳을 찾아주세요.
 *
 **/


/* 필요한 header file include */
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netinet/in.h>
#include	<time.h>

#ifndef __COMM_DEFINE_H__
#define __COMM_DEFINE_H__

/* 필요한 define 문 선언 (삭제, 추가, 변경이 가능하다) 
	여러 사용자가 사용함으로 주의 요함 - 자신이 사용하는 것만 선언해서 쓰면 됨*/

extern int debug_print(int dIndex, char *fmt, ...);



/* 기본 define 문이다. 아래 값들은 변경은 가능하다. 
	-- FRINTF나 LOG_LEVEL 값 정도	*/
#if defined(COMMERCIALLOG)
#define 	LOG_LEVEL					4
#define 	LOG_BUG 					1
#define		FPRINTF(x,z...)		 	{													\
				switch((int)x){																	\
					case LOG_BUG:		debug_print(x,z);	break;							\
				}																			\
			}

#define		dAppLog(x,z...)		 	{													\
				switch((int)x){																	\
					case LOG_BUG:		debug_print(x,z);	break;							\
				}																			\
			}
#endif

#if defined(APPLOG)
#define 	LOG_LEVEL					4
#define 	LOG_BUG 					1
#define		FPRINTF(x,z...)		 	debug_print(x,z)
#define		dAppLog(x,z...)		 	debug_print(x,z)
#endif

#if	!defined(COMMERCIALLOG) && !defined(APPLOG)
#define 	LOG_LEVEL					stdout
#define 	LOG_BUG						stdout
#define		FPRINTF		 				fprintf			
#define		dAppLog(x,z...)		 	debug_print(x,z)
#endif

#define		FILEPRINT		fprintf

#define 	HIPADDR(d) 		((d>>24)&0xff),((d>>16)&0xff),((d>>8)&0xff),(d&0xff)
#define 	NIPADDR(d) 		(d&0xff),((d>>8)&0xff),((d>>16)&0xff),((d>>24)&0xff)
#define		ASSERT(x)		if(! (x) ){ printf("EXIT\n");	exit(-1); }
#define		MALLOC			malloc
#define		_FREE			free
// FREE - SNMPIF 에서 사용하는 /usr/include/net-snmp/library/snmp_impl.h 와 충돌
//#define		FREE			free

#define		NTOH32(_DEF_TO,_DEF_FROM)		 {												\
	U32	 _DEF_FROM_TEMP;																	\
	char *_def_to,*_def_from;											 					\
	_DEF_FROM_TEMP = (U32) _DEF_FROM;	 													\
	_def_to = (char *) &_DEF_TO;															\
	_def_from = (char *) &_DEF_FROM_TEMP;													\
	_def_to[0] = _def_from[3];																\
	_def_to[1] = _def_from[2];																\
	_def_to[2] = _def_from[1];																\
	_def_to[3] = _def_from[0];																\
}															

#define		NTOH64(_DEF_TO,_DEF_FROM)		 {												\
	U64	 _DEF_FROM_TEMP;																	\
	char *_def_to,*_def_from;											 					\
	_DEF_FROM_TEMP = (U64) _DEF_FROM;														\
	_def_to = (char *) &_DEF_TO;															\
	_def_from = (char *) &_DEF_FROM_TEMP;													\
	_def_to[0] = _def_from[7];																\
	_def_to[1] = _def_from[6];																\
	_def_to[2] = _def_from[5];																\
	_def_to[3] = _def_from[4];																\
	_def_to[4] = _def_from[3];																\
	_def_to[5] = _def_from[2];																\
	_def_to[6] = _def_from[1];																\
	_def_to[7] = _def_from[0];																\
}															
				
#define		NTOH64V2(_DEF_TO,_DEF_FROM)		 {												\
	U64	 _DEF_FROM_TEMP;																	\
	char *_def_to,*_def_from;											 					\
	_DEF_FROM_TEMP = (U64) _DEF_FROM;														\
	_def_to = (char *) &_DEF_TO;															\
	_def_from = (char *) &_DEF_FROM_TEMP;													\
	_def_to[0] = _def_from[3];																\
	_def_to[1] = _def_from[2];																\
	_def_to[2] = _def_from[1];																\
	_def_to[3] = _def_from[0];																\
	_def_to[4] = _def_from[7];																\
	_def_to[5] = _def_from[6];																\
	_def_to[6] = _def_from[5];																\
	_def_to[7] = _def_from[4];																\
}															

#define STG_DiffTIME64(ENDT,ENDM,STARTT,STARTM,RESULT)	{												\
			*(RESULT) = (((S64)ENDT * 1000000 + (S64)ENDM) - ((S64)STARTT * 1000000 + (S64)STARTM));	\
			if( *(RESULT) > (S64) 3600 * 24 * 1000000 ){ *(RESULT) = 0;}								\
			else if( *(RESULT) < (S64) 0 ){ *(RESULT) = 0;}												\
}
#define STG_Diff32(FIRST,SECOND,RESULT) {													\
			*RESULT = (S32) (FIRST - SECOND);												\
}
#define STG_Equal(FROM, TO) {																\
			/* 제대로 처리 되기 위해서는 #pragma pack(1)으로 선언해야함. */					\
			memcpy(TO , & (FROM) , sizeof(FROM));											\
}
#define STG_Percent4(FIRST,SECOND,RESULT) {													\
			if(SECOND == 0){ *RESULT = 0; }													\
			else { *RESULT = (U32) ( FIRST * 10000 / SECOND ); }							\
}
#define STG_Percent3(FIRST,SECOND,RESULT) {													\
			if(SECOND == 0){ *RESULT = 0; }													\
			else { *RESULT = (U32) ( FIRST * 1000 / SECOND ); }								\
}
#define STG_Percent2(FIRST,SECOND,RESULT) {													\
			if(SECOND == 0){ *RESULT = 0; }													\
			else { *RESULT = (U32) ( FIRST * 100 / SECOND ); }								\
}
#define AVERAGE(FIRST,SECOND,RESULT) {														\
			if(SECOND == 0){ *RESULT = 0; }													\
			else { *RESULT = (FLOAT) ( FIRST / SECOND ); }									\
}


#define TOUSHORT(x)	(USHORT)(*(x)<<8|*(x+1))
#define TOUINT(x)	(UINT)(*(x)<<24|*(x+1)<<16|*(x+2)<<8|*(x+3))
#define TOULONG(x)	(ULONG)(*(x)<<24|*(x+1)<<16|*(x+2)<<8|*(x+3))
#define TOINT64(x)	(INT64)((INT64)(*(x))<<56|(INT64)(*(x+1))<<48|(INT64)(*(x+2))<<40|(INT64)(*(x+3))<<32|(INT64)(*(x+4))<<24|(INT64)(*(x+5))<<16|(INT64)(*(x+6))<<8|(INT64)(*(x+7)))

#define LOUCHAR(w)	((UCHAR)((USHORT)(w) & 0xff))
#define HIUCHAR(w)	((UCHAR)((USHORT)(w) >> 8))

#define max(a,b)	(((a) > (b)) ? (a) : (b))
#define min(a,b)	(((a) < (b)) ? (a) : (b))

/* A_HTTP, A_IHTTP , snmp tool.h에 정의되어 있� �*/
#ifndef	TOLOWER
#define TOLOWER(out, in)								\
{														\
	switch(in)											\
	{													\
		case 'A': out = 'a'; break;						\
		case 'B': out = 'b'; break;						\
		case 'C': out = 'c'; break;						\
		case 'D': out = 'd'; break;						\
		case 'E': out = 'e'; break;						\
		case 'F': out = 'f'; break;						\
		case 'G': out = 'g'; break;						\
		case 'H': out = 'h'; break;						\
		case 'I': out = 'i'; break;						\
		case 'J': out = 'j'; break;						\
		case 'K': out = 'k'; break;						\
		case 'L': out = 'l'; break;						\
		case 'M': out = 'm'; break;						\
		case 'N': out = 'n'; break;						\
		case 'O': out = 'o'; break;						\
		case 'P': out = 'p'; break;						\
		case 'Q': out = 'q'; break;						\
		case 'R': out = 'r'; break;						\
		case 'S': out = 's'; break;						\
		case 'T': out = 't'; break;						\
		case 'U': out = 'u'; break;						\
		case 'V': out = 'v'; break;						\
		case 'W': out = 'w'; break;						\
		case 'X': out = 'x'; break;						\
		case 'Y': out = 'y'; break;						\
		case 'Z': out = 'z'; break;						\
		default: out = in; break;						\
	}													\
}
#endif

#define DECFROMHEXA(out, in)								\
{															\
	switch(in)												\
	{														\
		case '0': out = 0; break;							\
		case '1': out = 1; break;							\
		case '2': out = 2; break;							\
		case '3': out = 3; break;							\
		case '4': out = 4; break;							\
		case '5': out = 5; break;							\
		case '6': out = 6; break;							\
		case '7': out = 7; break;							\
		case '8': out = 8; break;							\
		case '9': out = 9; break;							\
		case 'A': out = 10; break;							\
		case 'a': out = 10; break;							\
		case 'B': out = 11; break;							\
		case 'b': out = 11; break;							\
		case 'C': out = 12; break;							\
		case 'c': out = 12; break;							\
		case 'D': out = 13; break;							\
		case 'd': out = 13; break;							\
		case 'E': out = 14; break;							\
		case 'e': out = 14; break;							\
		case 'F': out = 15; break;							\
		case 'f': out = 15; break;							\
	}														\
}

#define PROC_NAME_SIZE		17
#define PROC_NAME_LEN		(PROC_NAME_SIZE -1)

#define BUF_SIZE			1024
#define BUF_LEN				(BUF_SIZE -1)

/* A_INET, PRE_A */
#ifdef BUFFERING
#define COLLECTION_MIN		50
#else
#define COLLECTION_MIN		0
#endif

/* Multi block max count */
#define MAX_MP_NUM 			5
#define MAX_SMP_NUM			3

#define MAX_STMT_SIZE		1024
#define MAX_FILENAME_SIZE	256
#define MAX_SQLQUERY_SIZE	20480
#define DB_TABLE_NOT_EXIST	1146
#define DB_NOT_CONNECT		2002
#define MAX_SYS_CNT			9


/* 시간 정의 상수 */
#define SEC_OF_MIN			60
#define SEC_OF_5MIN			300
#define SEC_OF_HOUR			3600
#define SEC_OF_DAY			86400
#define SEC_OF_WEEK			604800
#define SEC_OF_MON			2592000
#define SEC_OF_YEAR			3153600


#endif /* __COMMDEF_H__ */
