#include "samd.h"


void strtoupper( char* buff)
{
	while(*buff != 0x00)
	{
		*buff = toupper(*buff);
		buff++;
	}
}


