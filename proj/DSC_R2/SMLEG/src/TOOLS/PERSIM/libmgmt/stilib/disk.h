#ifndef DISK_H
#define DISK_H

/* structure of disk usage information
 */
typedef struct disk {
	char name[32];			/* mounted name */
	unsigned long	total;	/* total size */
	unsigned long	avail;	/* avail size */
	unsigned long	free;	/* free size */
	unsigned long	used;	/* used size */
	int		block;			/* block size */
	int		pcnt;			/* percent of usage */
} disk_t;

#define MAXFS	64

/* get current system's disk information
 * returns Upon successful completion, 1 is returned.
 * otherwise, 0 is return
 */
int getdiskuse(disk_t *dp /* disk usage information */);

#endif
