#ifndef __VERSION_H__
#define __VERSION_H__

#include <sys/shm.h>

#include "config.h" /* MAX_SW_COUNT */

#define E_UNKNOWN				-1
#define E_VER_SHMGET			-2
#define E_VER_SHMGET2			-3
#define E_VER_SHMAT				-4
#define E_VER_SHMDT				-5
#define E_VER_SHMCTL			-6

#define VER_EXIST				0
#define VER_CREATE				1
#define VER_ATTACH				2
#define VER_REMOVE				3


#define DEF_VERSION_SIZE        7       /* Inserted By LSH in 2004.04.05 */

typedef struct _st_Version
{
	char	szVersion[MAX_SW_COUNT][DEF_VERSION_SIZE];
} st_Version;

extern st_Version *g_stVersion;

extern int  set_version(key_t key, int prc_idx, char *ver);
extern void	get_version(int prc_idx, char *ver);
extern int  remove_version(key_t ShmKey);

#endif /* __VERSION_H__ */
