#ifndef __TAIL_H__
#define __TAIL_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/time.h> 
#include <sys/stat.h> 
#include <fcntl.h> 
#include <errno.h>
#include <string.h>



#define		_LINE_BUFFER_LEN			1024

/* 2008-12-24
 * by june
 * Desc : tail을 적용할 대상 정보
 */ 
typedef struct _TAIL 
{ 
	FILE *fp;            	/* 파일 스트림 */ 
	char fileName[128];  	/* 현재 rdr 파일 이름 */
	char fullFName[128];  	/* 현재 rdr full path + filename */
	int  fd;             	/* 파일 스트림에 대한 파일 지정자 */
	int  revsize;        	/* 최근에 읽었던 파일 크기 */ 
	char *buf;				/* file 의 한 row씩 읽을 buffer */
	int  fdStat;			/* file 의 현재 상태 */
	int  operflag;			/* file monitoring 유무 판단 플레그 */
	int  rdrtype;			/* rdr type */

} TAIL; 

enum { _FD_STAT_NOTFOUND = -1, _FD_STAT_FIND = 0, _FD_STAT_OPEN, _FD_STAT_TMP, _FD_STAT_BKUP };

//extern TAIL *openTail(char *fname);
//extern int readTail(TAIL **RTAIL, char **buf, size_t size, int sec);
//extern void closeTail(TAIL *OTAIL);

#endif /* __TAIL_H__ */

#if 0
struct stat {
	dev_t         st_dev;      /* device */
	ino_t         st_ino;      /* inode */
	mode_t        st_mode;     /* protection */
	nlink_t       st_nlink;    /* number of hard links */
	uid_t         st_uid;      /* user ID of owner */
	gid_t         st_gid;      /* group ID of owner */
	dev_t         st_rdev;     /* device type (if inode device) */
	off_t         st_size;     /* total size, in bytes */
	blksize_t     st_blksize;  /* blocksize for filesystem I/O */
	blkcnt_t      st_blocks;   /* number of blocks allocated */
	time_t        st_atime;    /* time of last access */
	time_t        st_mtime;    /* time of last modification */
	time_t        st_ctime;    /* time of last change */
};
#endif

