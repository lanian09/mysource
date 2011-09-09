#include <tcp_gen.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <math.h>

int dGetMSGQSize(int qid, long *size)
{
	int ret;
	struct msqid_ds buf;

	ret = msgctl(qid, IPC_STAT, &buf);
	if(ret < 0)
	{
		return ret;
	}

	*size = buf.msg_cbytes;

	return 1;
}

int round( double value1 )
{
	double value2;
	int     value3;

	value2 = floor(value1);

	value2 = value1 - value2;

	if( value2 >= 0.5 )
	{
		value1 =  ceil(value1);
	}
	else
	{
		value1 = floor(value1);
	}

	value3 = (int)value1;

	return value3;
}

