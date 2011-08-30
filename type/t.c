#include <stdio.h>

#define _2B (0x8000+1)
#define _4B (0x80000000+1)
#define _8B (0x8000000000000000+1)
int main()
{
	long double ld = 0x1234123412341234L +0.12345;
	short s = _2B;
	unsigned short us = _2B;
	int   i = _4B;
	unsigned int ui = _4B;
	long  l = _8B;
	unsigned long ul = _8B;
	long long ll = _8B;
	unsigned long long ull = _8B;

	printf("%llf = ldval\n", ld);
	printf("%d:%d=short\n", sizeof(short int),s);
	printf("%d:%u=unsigned short\n", sizeof(unsigned short int),us);
	printf("%d:%d=int\n", sizeof(int),i);
	printf("%d:%u=unsigned int\n", sizeof(unsigned int),ui);
	printf("%d:%ld=long int\n", sizeof(long int),l);
	printf("%d:%lu=unsigned long int\n", sizeof(unsigned long int),ul);
	printf("%d:%lld=long long int\n", sizeof(long long int),ll);
	printf("%d:%llu=unsigned long long int\n", sizeof(unsigned long long int),ull);
	printf("%d=float\n", sizeof(float));
	printf("%d=double\n", sizeof(double));
	printf("%d=long double\n", sizeof(long double));
}
