#ifndef __SHMUTIL_LIB_HEADER_FILE___
#define __SHMUTIL_LIB_HEADER_FILE___

#include <ipaf_sem.h>
#include <ipaf_shm.h>
#include "eth_capd.h"

/* function prototype define */

extern	int		semid_capindex;

// SHMUTIL_LIB.c
int Read_m_shm( int dIndex, T_CAP *P, unsigned char *puBuffer );
//int Write_m_shm( T_CAP *P, int dLength, unsigned char *puBuffer );
int Write_m_shm( T_CAP *P, int datalen, T_CAPHDR *caphdr, void *data );
int dGetLastReadPos( long *LastReadPos, T_CAP *P );

#if 0
int Read_Trace_shm( T_Trace_Mem *P, unsigned char *puBuffer );
int Write_Trace_shm( T_Trace_Mem *P, int dLength, unsigned char *puBuffer, int BufferSize );
#endif

#endif

