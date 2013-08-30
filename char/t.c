#include <stdio.h>
#include <string.h>
#include <stdlib.h>

enum {
	_a = 5,
	_b, _c, _d, _e,
	_f = 20,
	_g
};

typedef struct _st_sss{
	int dNo;
	unsigned char ucType;
	unsigned char ucReserved[7];
	int aaa;
} st_test;

void test2()
{
	char cv[] = "5";
	unsigned char ucType;
	st_test stt;
	ucType = (unsigned char)atoi( cv ); 

	printf("NO = [%d][%c][0x%02x]\n", ucType, ucType, ucType);
	if( ucType < _g ){
		printf("1]small\n");
	}else printf("1]big\n");

	memset( &stt, 0x00, sizeof(st_test) );
	stt.ucType = ucType;

	printf("No2=[%d][%c][0x%02x]\n", stt.ucType, stt.ucType, stt.ucType);
	if( stt.ucType < _g ){
		printf("2]small\n");
	}else printf("2]big\n");
}

char * _2i_step01(int v)
{
	char *buffer;
	buffer = malloc(128);
	sprintf(buffer,"%d.%d", v/10, v%10);
	return buffer;
}

char * _2i_step05(int v)
{
	char *buffer;

	buffer = malloc(128);
	sprintf(buffer, "%d.%d", v/2, (v%2)*5);
	return buffer;
}

void test1()
{
	char a = 31;

	printf("a(%d)*0.1=%f\n", a,a*0.1);
	printf("a*0.1=%d.%d\n", a/10, a%10);
	printf("a*0.5=%d.%d\n", a/2, (a%2)*5);
	printf("a2*0.1=%s\n", _2i_step01((int)a));
	printf("a2*0.5=%s\n", _2i_step05((int)a));

}

enum test_ch {
	en_t1 = 0x00,
	en_t2 = 0x01,
	en_t3 = 254,
	en_t4 = 255
};

char *test_check(int v)
{
	switch(v) {
		case en_t1 : return "t1";
		case en_t2 : return "t2";
		case en_t3 : return "t3";
		case en_t4 : return "t4";
	}

	return "-";
}

void test3()
{
	char ac;
	unsigned char uc;

	ac = 253;
	uc = (char)253;

	printf("ac=0x%x:%d:%s\n", ac, ac, test_check(ac));
	printf("uc=0x%x:%d:%s\n", uc, uc, test_check(uc));

}

int print(void (*cb)())
{
	static test_cnt = 1;
	printf("=== test start(%d) ===\n", test_cnt++);
	cb();
	printf("=== test finished ===\n");
	return 0;
}

int main()
{
	print(test1);
	print(test2);
	print(test3);
}

