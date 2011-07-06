#include <stdio.h>
#include <string.h>
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

int main()
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
