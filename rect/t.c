#include <stdio.h>

typedef struct _st_rect{
	int x;
	int y;
} st_Rect, *pst_Rect;

st_Rect a,b;

int isIn2(int a, int b, int c)
{
	if( ( a <= c && c <= b ) ||
		( b <= c && c <= a ) ){
		return 1;
	}
	return 0;
	
}

int isIn(st_Rect p)
{
	if( isIn2(a.x, b.x, p.x) ){
		if( isIn2(a.y, b.y, p.y) ){
			return 1;
		}
	}
	return 0;
}

int main(int ac, char **av)
{

	st_Rect c;

	a.x = 1;
	a.y = 10;

	b.x = 10;
	b.y = 1;

	if( ac != 3 ){
		printf("param needs\n");
		return -1;
	}

	c.x = atoi(av[1]);
	c.y = atoi(av[2]);

	printf("c.x=%d, c.y=%d, isIn=%s\n", c.x, c.y, !isIn(c)?"NO":"YES");
	return 0;
}


