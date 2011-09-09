#ifndef CPU_H
#define CPU_H

/* get current system's cpu resources (cou load)
 * returns Upon successful completion, 1 is returned
 * otherwise, 0 is return
 */
int getcpuuse(int *n,	/* pointer for buffer to get cpu number */
			int *n1,/* pointer for buffer to get 1min cpu load */
			int *n2,/* pointer for buffer to get 5min cpu load */
			int *n3,/* pointer for buffer to get 15min cpu load */
			int *n4	/* pointer for buffer to get 1hour cpu load */);

#endif
