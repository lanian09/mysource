#include <stdio.h>
#include <string.h>

#define TYPE_IMSI 1
char *getTypeStr(int type)
{
	if( type == TYPE_IMSI )
		return "IMSI";
	else
		return "IP";
}

int main(void)
{
	char buf[16];
	const int _IMSI_SIZE = 15;
	char *pdot = NULL;
	char regVal[16] = "450061024447290";
	int	dType = 0;
	int len;

	memset(buf, 0x00, sizeof(buf));

	if( buf[0] != 0 )
		printf("NOT NULL\n");
	else
		printf("NULL\n");

	len = strlen(regVal);

	pdot = strchr(regVal, '.');
	if( pdot != NULL )
		printf("%s\n",pdot);
	else
		printf("pdot is NULL\n");

	printf("len:%d\n",len);
	if( (strlen(regVal) == _IMSI_SIZE)  &&  (pdot = strchr(regVal,'.') == NULL ))
			dType = 1;

	printf("%s\n",getTypeStr(dType));



	return 0;
}
