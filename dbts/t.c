#include <stdio.h>

int main()
{
	struct _st_myst{
		int dInt;
		unsigned int uInt;
		unsigned short usInt;
	} st_ms;
	
	int *pdInt;
	unsigned int *puInt;
	unsigned short *pusInt;
	short tmp;

	st_ms.dInt = 10;
	st_ms.uInt = 11;
	st_ms.usInt = 12;

	pdInt = &st_ms.dInt;
	puInt = &st_ms.uInt;
	pusInt= &st_ms.usInt;
	tmp = 234;

	printf("r ]dInt=%d, uInt=%ld, usInt=%hu, t=%d\n", st_ms.dInt, st_ms.uInt, st_ms.usInt, tmp);
	printf("p1]dInt=%d, uInt=%ld, usInt=%hu, t=%d\n", *pdInt, *puInt, *pusInt, tmp);

	st_ms.dInt = 13;
	st_ms.uInt = 14;
	st_ms.usInt = 15;

	printf("p2]dInt=%d, uInt=%ld, usInt=%hu, t=%d\n", *pdInt, *puInt, *pusInt, tmp);
	
}
