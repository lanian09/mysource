#include <stdio.h>
int main()
{
	char *test="Release due to UE generated signalling connection release";
	char *test2=
"Requested Ciphering and/or Integrity Protection Algorithms not Supported";
	char *test3=
"Conflict with already existing Integrity protection and/or Ciphering information";
	char *test4=
"Requested Ciphering and/or Integrity Protection Algorithms not S";
	char *test5=
"Conflict with already existing Integrity protection and/or Ciphe";
	printf("test  len=%d:%s\n",strlen(test),test);
	printf("test2 len=%d:%s\n",strlen(test2),test2);
	printf("test3 len=%d:%s\n",strlen(test3),test3);
	printf("test4 len=%d:%s\n",strlen(test4),test4);
	printf("test5 len=%d:%s\n",strlen(test5),test5);
	return 0;
}
