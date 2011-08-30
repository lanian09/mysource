#include <stdio.h>
#include <string.h>

#define DEF_TEST "_TEST_STRING_"

char *STR = DEF_TEST;

int set(char* str)
{
	STR = str;
	return 0;
}

char* get()
{
	return &STR[0];
}

int main(int ac, char **av)
{
	printf("string=%s(%d)\n", STR,strlen(STR));
	if( ac > 1 ){
		set(av[1]);
		printf("string3=%s(%d)\n", STR,strlen(STR));
	}
	return 0;

}
