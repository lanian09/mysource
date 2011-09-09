#ifndef COMM_VERSION_H
#define COMM_VERSION_H

#include  "conflib.h"
#include "sysconf.h"
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>
#include "ipaf_shm.h"			// yhshin

#if 0 // yhshin
#define OMP_VER_MNG_NUM		10
#define VERSION_STR_LEN		8

#define	OMP_VER_INX_IXPC	0
#define	OMP_VER_INX_FIMD	1
#define	OMP_VER_INX_COND	2
#define	OMP_VER_INX_STMD	3
#define	OMP_VER_INX_MMCD	4
#define	OMP_VER_INX_MCDM	5
#define	OMP_VER_INX_SAMD	6
#define	OMP_VER_INX_NMSIF	7
#define	OMP_VER_INX_UDRCOL	8

typedef struct {
	char	ver[OMP_VER_MNG_NUM][VERSION_STR_LEN];
}stVersion;
#endif

typedef struct {
	char    *name;
	int     index;
}VersionIndexTable;

int init_version_shm();
int set_proc_version(int index, char *prcver);
char * get_proc_version(char *procname);
int detatch_version_shm();
#endif  /* COMM_VERSION_H */
