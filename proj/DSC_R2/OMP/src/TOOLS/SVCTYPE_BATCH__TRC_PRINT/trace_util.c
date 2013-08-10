#include <trace_proto.h>

int is_dirpath(char *path)
{
	struct stat dirstat;

	if(stat(path, &dirstat) < 0)
		return 0;
	if(!(S_IFDIR & dirstat.st_mode))
		return 0;

	return 1;
}
