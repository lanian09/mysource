#include <stdio.h>

int main()
{
	char *aid_str;

	aid_str = 0;

	if(!aid_str) printf("NOT NULL\n");
	else         printf("IS NULL\n");

	return 1;
}
