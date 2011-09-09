/**A.1*  File Inclusion ***********************************/
#include <sys/shm.h>	/* shmget */
#include <sys/msg.h>	/* msgget */
#include <errno.h>		/* errno */
#include <string.h>		/* memset */

#include "ipclib.h"

int msgq_init(key_t qkey)
{
    int qid;

    /* attempt to create messgae queue */
    if( (qid = msgget(qkey, IPC_CREAT|QPERM)) == -1 )
        return E_MSGQ_GET;

    return qid;
}

int msgq_read(int qid, char *msg, int size, int flags)
{
    int ret;
    st_Qentry rEntry;

    if( (ret = msgrcv(qid, &rEntry, sizeof(st_Qentry)-sizeof(long), 0, flags )) < 0 )
    {
        if( errno != EINTR && errno != ENOMSG )
        {
            return E_MSGQ_RCV;
        }
        return E_MSGQ_GENERIC;
    }

    if( size < ret+sizeof(long) )
        return E_MSGQ_BUF;  /* buffer error */

    memcpy(msg, &rEntry, ret+sizeof(long));

    return ret+sizeof(long);
}

int msgq_write(int qid, char *msg, int size, int flags)
{
    int ret;
    st_Qentry sEntry;

    if(size > MAXOBN)
        return E_MSGQ_BUF;  /* buffer error */

    memcpy( &sEntry, msg, size);

    if( (ret = msgsnd(qid, &sEntry, size-sizeof(long), flags)) < 0 )
    {
        if(errno != EINTR && errno != EAGAIN)
        {
            return E_MSGQ_SND;
        }
        return E_MSGQ_GENERIC;
    }

    return size;
}

