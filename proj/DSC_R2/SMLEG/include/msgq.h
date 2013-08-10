
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include <errno.h>

#pragma pack(1)
extern int errno;

#define	QPERM		0666
#define MAXOBN		3072
#define MAXPRIOR	10

typedef struct {
	long mtype;
	char mtext[MAXOBN+1];
} q_entry;

int Init_msgq( key_t q_key );
int Read_msgq( int q_id, char *msg, int size, int flags );
int Write_msgq( int q_id, char *msg, int size, int flags );

#pragma pack()
