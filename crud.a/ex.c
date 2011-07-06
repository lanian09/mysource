#include <stdio.h>

int addition(int a, int b);
int multiplication(int a, int b);

int main()
{
	int result;
	result = addition(1,2);
	printf("addition result is : %d\n", result);
	result = multiplication(3,2);
	printf("multiplication result is : %d\n", result);
}
