#ifndef __STG_DEF_H__
#define __STG_DEF_H__

/* 필요한 define 문 선언 (삭제, 추가, 변경이 가능하다) 
	여러 사용자가 사용함으로 주의 요함 - 자신이 사용하는 것만 선언해서 쓰면 됨*/

#if 0
extern int debug_print(int dIndex, char *fmt, ...);


/* 기본 define 문이다. 아래 값들은 변경은 가능하다. 
	-- FRINTF나 LOG_LEVEL 값 정도  */
#if defined(COMMERCIALLOG)
#define 	LOG_LEVEL           		4
#define 	LOG_BUG 	        		1
#define		FPRINTF(x,z...)   	 	{													\
				switch((int)x){																	\
					case LOG_BUG:		debug_print(x,z);	break;							\
				}																			\
			}

#define		dAppLog(x,z...)   	 	{													\
				switch((int)x){																	\
					case LOG_BUG:		debug_print(x,z);	break;							\
				}																			\
			}
#endif

#if defined(APPLOG)
#define 	LOG_LEVEL           		4
#define 	LOG_BUG 	        		1
#define		FPRINTF(x,z...)   	 	debug_print(x,z)
#define		dAppLog(x,z...)   	 	debug_print(x,z)
#endif

#if	!defined(COMMERCIALLOG) && !defined(APPLOG)
#define 	LOG_LEVEL           		stdout
#define 	LOG_BUG   	        		stdout
#define		FPRINTF   	 				fprintf			
#define		dAppLog(x,z...)   	 	debug_print(x,z)
#endif

#define		FILEPRINT		fprintf
#endif

#if defined(APPLOG)
#include "utillib.h"
#define 	LOG_LEVEL           		4
#define 	LOG_BUG 	        		1
#define		FPRINTF   	 				dAppLog
#endif

#if	!defined(APPLOG)
#define 	LOG_LEVEL           		stdout
#define 	LOG_BUG   	        		stdout
#define		FPRINTF   	 				fprintf			
#endif


#define		FILEPRINT		fprintf


#define 	HIPADDR(d) 		((d>>24)&0xff),((d>>16)&0xff),((d>>8)&0xff),(d&0xff)
#define 	NIPADDR(d) 		(d&0xff),((d>>8)&0xff),((d>>16)&0xff),((d>>24)&0xff)
#define		ASSERT(x)		if(! (x) ){ printf("EXIT\n");  exit(-1); }
#define		MALLOC			malloc
#define		FREE			free

#define		NTOH32(_DEF_TO,_DEF_FROM)   	 {												\
	U32	 _DEF_FROM_TEMP;																	\
	char *_def_to,*_def_from;											 					\
    _DEF_FROM_TEMP = (U32) _DEF_FROM;     													\
	_def_to = (char *) &_DEF_TO;															\
	_def_from = (char *) &_DEF_FROM_TEMP;													\
    _def_to[0] = _def_from[3];        														\
    _def_to[1] = _def_from[2];        														\
    _def_to[2] = _def_from[1];        														\
    _def_to[3] = _def_from[0];        														\
}															

#define		NTOH64(_DEF_TO,_DEF_FROM)   	 {												\
	U64	 _DEF_FROM_TEMP;																	\
	char *_def_to,*_def_from;											 					\
    _DEF_FROM_TEMP = (U64) _DEF_FROM;    													\
	_def_to = (char *) &_DEF_TO;															\
	_def_from = (char *) &_DEF_FROM_TEMP;													\
    _def_to[0] = _def_from[7];        														\
    _def_to[1] = _def_from[6];        														\
    _def_to[2] = _def_from[5];        														\
    _def_to[3] = _def_from[4];        														\
    _def_to[4] = _def_from[3];        														\
    _def_to[5] = _def_from[2];        														\
    _def_to[6] = _def_from[1];        														\
    _def_to[7] = _def_from[0];  															\
}															
				
#define		NTOH64V2(_DEF_TO,_DEF_FROM)   	 {												\
	U64	 _DEF_FROM_TEMP;																	\
	char *_def_to,*_def_from;											 					\
    _DEF_FROM_TEMP = (U64) _DEF_FROM;    													\
	_def_to = (char *) &_DEF_TO;															\
	_def_from = (char *) &_DEF_FROM_TEMP;													\
    _def_to[0] = _def_from[3];        														\
    _def_to[1] = _def_from[2];        														\
    _def_to[2] = _def_from[1];        														\
    _def_to[3] = _def_from[0];        														\
    _def_to[4] = _def_from[7];        														\
    _def_to[5] = _def_from[6];        														\
    _def_to[6] = _def_from[5];        														\
    _def_to[7] = _def_from[4];  															\
}															

#define STG_DiffTIME64(ENDT,ENDM,STARTT,STARTM,RESULT)	{												\
			*(RESULT) = (((S64)ENDT * 1000000 + (S64)ENDM) - ((S64)STARTT * 1000000 + (S64)STARTM));	\
			if( *(RESULT) > (S64) 3600 * 24 * 1000000 ){ *(RESULT) = 0;}								\
			else if( *(RESULT) < (S64) 0 ){ *(RESULT) = 0;}												\
}
#define STG_Diff32(FIRST,SECOND,RESULT) {													\
			*(RESULT) = (S32) (FIRST - SECOND);												\
}
#define STG_Equal(FROM, TO) {																\
			/* 제대로 처리 되기 위해서는 #pragma pack(1)으로 선언해야함. */					\
			memcpy(TO , & (FROM) , sizeof(FROM));											\
}
#define STG_Percent4(FIRST,SECOND,RESULT) {													\
			if(SECOND == 0){ *(RESULT) = 0; }													\
			else { *(RESULT) = (U32) ( FIRST * 10000 / SECOND ); }							\
}
#define STG_Percent3(FIRST,SECOND,RESULT) {													\
			if(SECOND == 0){ *(RESULT) = 0; }													\
			else { *(RESULT) = (U32) ( FIRST * 1000 / SECOND ); }								\
}
#define STG_Percent2(FIRST,SECOND,RESULT) {													\
			if(SECOND == 0){ *(RESULT) = 0; }													\
			else { *(RESULT) = (U32) ( FIRST * 100 / SECOND ); }								\
}
#define AVERAGE(FIRST,SECOND,RESULT) {														\
			if(SECOND == 0){ *(RESULT) = 0; }													\
			else { *(RESULT) = (FLOAT) ( FIRST / SECOND ); }									\
}

#define STG_inc_ifequal(FIRST,SECOND,RESULT) {														\
			if(FIRST == SECOND){ (*(RESULT))++; }													\
}
#define STG_inc_gt(FIRST,SECOND,RESULT) {														\
			if(FIRST > SECOND){ (*(RESULT))++; }													\
}
#define STG_inc_2if(FIRST, FIRST_val, SECOND, SECOND_val, RESULT) {														\
			if((FIRST == FIRST_val) && (SECOND == SECOND_val)){ (*(RESULT))++; }													\
}
#define STG_ifaccu(FIRST, SECOND, RESULT) {														\
			if(FIRST == SECOND) { (*(RESULT)) += (*(RESULT)); }													\
}
#define STG_timemax(STIME, SMTIME, RETTIME, RETMTIME) {														\
			if(STIME > *(RETTIME)) { *(RETTIME) = STIME; *(RETMTIME) = SMTIME; } 				\
			else if ((STIME == *(RETTIME)) && (SMTIME > *(RETMTIME))) { *(RETTIME) = STIME; *(RETMTIME) = SMTIME; } 				\
}

#define STG_INTEGER		1
#define STG_STRING		2
#define STG_IP			3
#define STG_DEF			4

#endif 
