#include <stdio.h>

int file_exists(const char *fn)
{
	FILE *file = NULL;

	file = fopen(fn, "r");
	if (file) {
		fclose(file);
		return(1);
	}
	return(0);
}

int main()
{
	char *profile = "aaaa.profile";

	if (!file_exists(profile)){
		printf("out\n");
	}else{
		printf("in\n");
	}
}
