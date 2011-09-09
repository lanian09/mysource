#include "udrdb.h"

static 	char    version[8] = "R0.0.1";

main(int argc, char *argv[])
{

	if (mysql_initialize() < 0)
		exit(1);

	if (udrdb_proc() < 0)
		exit(1);
	
    return 1;
}
