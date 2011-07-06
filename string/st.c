#include <stdio.h>
#include <string.h>

int tap(int isPara)
{
	char str[100], buf[100];
	sprintf(str, "SELECT * FROM ABC");
	if( isPara ){
		switch(isPara){
			case 0x01: sprintf(buf,"WHERE aaa=%u\n", 123); break;
			case 0x10: sprintf(buf,"WHERE bbbbbb\n"); break;
			case 0x11: sprintf(buf,"WHERE cccccc\n"); break;
			default : buf[0] = 0x0; break;
		}
		strcat(str, buf);
	}
	printf("str=%s\n", str);
	return 0;
}

int main()
{
	int aa = 0;
	aa |= 0x01; tap(aa);
	aa |= 0x10; tap(aa);
	aa |= 0x11; tap(aa);
	aa = 0;    tap(aa);
	return 0;
}
