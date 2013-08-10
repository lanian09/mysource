#include <stdio.h>
#include <stdlib.h>


int cvt(char *s)
{
	double t;

	if (!s) return 0;

	t = atof(s);
	return ((int)(t*10))/5;
}
int main()
{
	char *ab = "7.5";
	double dab;

	dab = atof(ab);
	printf("dab=%lf\n", dab);
	printf("(int)dab*10=%d\n", (int)(dab*10));
	printf("dab(base0.5)=%d\n", ((int)(dab*10))/5);
	printf("cvteddab=%d\n", cvt(ab));
}
