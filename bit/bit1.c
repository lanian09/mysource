#include <stdio.h>


int main()
{
	int    i;
	char   ca;
	struct _st_char{
		unsigned char a:1;
		unsigned char b:1;
		unsigned char c:1;
		unsigned char d:1;
		unsigned char e:1;
		unsigned char f:1;
		unsigned char g:1;
		unsigned char h:1;
	} *uca;

	ca = 0;
	uca = (struct _st_char*) &ca;
	for( i=0; i<8; i++ ){
		ca = 0x01 << i;
		printf(" << %d : a=%d b=%d c=%d d=%d e=%d f=%d g=%d h=%d\n", i, uca->a, uca->b, uca->c, uca->d, uca->e, uca->f, uca->g, uca->h);
	}

	return;
}
