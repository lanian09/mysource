#include <string.h>
#include <errno.h>

#include "msgq.h"

int Init_msgq( key_t q_key )
{
	int q_id;

	/* attempt to create messgae queue */
	if( (q_id = msgget(q_key, IPC_CREAT|QPERM)) == -1 )
		return -1;
	
	return q_id;
}

int Read_msgq( int q_id, char *msg, int size, int flags )
{
	int ret;

	if( (ret = msgrcv(q_id, msg, sizeof(q_entry)-sizeof(long), 0, flags )) < 0 ) {
		if( errno != EINTR && errno != ENOMSG ) {
			return -1;
		}
		return 1;
	}

	return 0;
}

int Write_msgq( int q_id, char *msg, int size, int flags )
{
	int ret;

	if( (ret = msgsnd(q_id, msg, size-sizeof(long), flags )) < 0 ) {
		if( errno != EINTR && errno != EAGAIN ) {
			return -1;
		}
		return 1;
	}
	return 0;
}
