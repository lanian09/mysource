#include <stdio.h>
#include <stdlib.h>

int powd( int i, int y )
{
    int k;
    int t;

    t = 1;

    for( k=0; k < y ; k++ )
    {
        t *= i;
    }

    return t;
}

char getItoH(char ch)
{
	char ret;
	switch(ch){
		case '0': ret = 0x00; break;
		case '1': ret = 0x01; break;
		case '2': ret = 0x02; break;
		case '3': ret = 0x03; break;
		case '4': ret = 0x04; break;
		case '5': ret = 0x05; break;
		case '6': ret = 0x06; break;
		case '7': ret = 0x07; break;
		case '8': ret = 0x08; break;
		case '9': ret = 0x09; break;
		case 'a': case 'A' : ret = 0x0a; break;
		case 'b': case 'B' : ret = 0x0b; break;
		case 'c': case 'C' : ret = 0x0c; break;
		case 'd': case 'D' : ret = 0x0d; break;
		case 'e': case 'E' : ret = 0x0e; break;
		case 'f': case 'F' : ret = 0x0f; break;
	}
	return ret;
}

void printf_value(short *value)
{
	int i;
	for(i=0;i<8 ;i++){
		printf("%d\t",value[i]);
	}
	printf("\n");
}

int main(int ac, char **av)
{
	int i,m,l,h;
	int temp;
	short value[16][8];

	char Hexa[3];
	int val;
	short j[16],k;
	int idx;
	
	char buf[17];
	int len = 0;

	struct timeval start,finish;
	unsigned int sec,sec2,sec3,sec4;
	int usec,usec2,usec3,usec4;

	if( ac != 2 ){
		printf("error ac != 2\n");
		exit(1);
	}
	strncpy( buf, av[1], 16);
	buf[16] = 0x00;
	len = strlen(buf);
	if( len != 16 ){
		printf("str len != 16[%d][%s]\n",len,buf);
		exit(1);
	}

	printf("original logic >>>\n");
	memset( value, 0x00, 16*8 );
	gettimeofday(&start,NULL);
	for ( i=0 ; i < 8 ;i++)
    {
        memcpy( Hexa, &buf[idx], 2 );
        Hexa[2] = 0x00;
        idx += 2;
        sscanf( Hexa, "%x", &val );
        j[i] = val;

        for( k = 0; k<8 ; k++ )
        {
            value[i][k] = j[i]&powd(2,k);
        }
		printf_value(value[i]);
    }
	gettimeofday(&finish,NULL);
	sec = finish.tv_sec - start.tv_sec;
	usec = finish.tv_usec - start.tv_usec; 
	if( usec < 0 ){
		sec--;
		usec += 1000000; 
	}

	printf("my logic >>>\n");
	memset( value, 0x00, 16*8 );

	gettimeofday(&start,NULL);
	for( i=0,m=0; i< 8; i++,m=m+2 ){
		for(h=0,l=1;h< 8;h++){
			temp = getItoH(buf[m])*16+getItoH(buf[m+1]);
			value[i][h]=l&temp;
			l*=2;
		}
		printf_value(value[i]);
		
	}
	gettimeofday(&finish,NULL);
	memset( value, 0x00, 16*8 );
	sec2 = finish.tv_sec - start.tv_sec;
	usec2 = finish.tv_usec - start.tv_usec; 
	if( usec2 < 0 ){
		sec2--;
		usec2 += 1000000; 
	}

	printf("my logic2[ 16* <<4 ] >>>\n");

    gettimeofday(&start,NULL);
    for( i=0,m=0; i< 8; i++,m=m+2 ){
        for(h=0,l=1;h< 8;h++){
            temp = (getItoH(buf[m])<<4)+getItoH(buf[m+1]);
            value[i][h]=l&temp;
            l*=2;
        }
        printf_value(value[i]);

    }
    gettimeofday(&finish,NULL);
	memset( value, 0x00, 16*8 );
    sec3 = finish.tv_sec - start.tv_sec;
    usec3 = finish.tv_usec - start.tv_usec;
    if( usec3 < 0 ){
        sec3--;
        usec3 += 1000000;
    }

	printf("my logic3[ deleted temp ] >>>\n");
    gettimeofday(&start,NULL);
    for( i=0,m=0; i< 8; i++,m=m+2 ){
        for(h=0,l=1;h< 8;h++){
            value[i][h]=l&( (getItoH(buf[m])*16 )+getItoH(buf[m+1]) );
            l*=2;
        }
        printf_value(value[i]);

    }
    gettimeofday(&finish,NULL);

    sec4 = finish.tv_sec - start.tv_sec;
    usec4 = finish.tv_usec - start.tv_usec;
    if( usec4 < 0 ){
        sec4--;
        usec4 += 1000000;
    }

    printf("\n result >>>\n");
	printf(" original - elapsed time :%u.%06d\n",sec,usec);
	printf(" my - elapsed time :%u.%06d\n",sec2,usec2);
    printf(" my2 - elapsed time :%u.%06d\n",sec3,usec3);
    printf(" my3 - elapsed time :%u.%06d\n",sec4,usec4);

	return 0;
}
