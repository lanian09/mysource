#include <stdio.h>

enum en_test {
	ea = 0x01,
	eb = 0x02,
	ec = 0x04,
	ed = 0x08
};

void printo(char *str, int v)
{
	printf("%s=%d\n", str, v);
}

int main()
{
	int a;
	a = ea | eb;

	printo("ea|eb=", a);
	if (a&ea) printo("a|ea=", a&ea);
	if (a&eb) printo("a|eb=", a&eb);
	if (a&ec) printo("a|ec=", a&ec);
	if (a&ed) printo("a|ed=", a&ed);

	a = eb | ed;
	printo("eb|ed=", a);
	if (a&ea) printo("a|ea=", a&ea);
	if (a&eb) printo("a|eb=", a&eb);
	if (a&ec) printo("a|ec=", a&ec);
	if (a&ed) printo("a|ed=", a&ed);

	return(1);
}
