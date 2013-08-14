#include <stdio.h>

#define bi a
#define B(i) ((bi>>i) & 0x01)

int main()
{
	struct stt {
		unsigned char a8:1;
		unsigned char a7:1;
		unsigned char a6:1;
		unsigned char a5:1;
		unsigned char a4:1;
		unsigned char a3:1;
		unsigned char a2:1;
		unsigned char a1:1;
	} *ab;

	int a;

	a = 0;

	ab = (struct stt*)&a;

	ab->a1 = 1;
	ab->a3 = 1;
	ab->a5 = 1;

	printf("a=%x%x%x%x %x%x%x%x\n", B(7), B(6), B(5), B(4), B(3), B(2), B(1), B(0));

	return;


}
