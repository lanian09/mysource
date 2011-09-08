#include "ifb_proto.h"


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main (int ac, char *av[])
{
	int		cnt, qkey;

	if (ac != 2) {
		fprintf(stderr,"  clrq  qkey\n");
		return -1;
	}

	qkey = strtol (av[1],0,16);

	if ((cnt = ifb_clearQ(qkey,1)) < 0)
		return -1;

	fprintf(stderr,"  clear msg count = %d\n", cnt);

	exit(0);

} //----- End of main -----//
