#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>

int test()
{
	time_t          tTmp, tCurrent, tPrev = 0;
    struct tm       stTime, stPrevTime, stTmp;
	int i=0, year, mon, day;
	unsigned int ttt;
	struct tm st_Tm;

	tTmp = time(NULL);
	ttt = (unsigned int)tTmp;
	i = (int)tTmp;

	localtime_r( (const time_t*)&tTmp, &st_Tm);
	st_Tm.tm_hour = 0;//atoi( hour );
	st_Tm.tm_min = 0;//atoi( min );
	st_Tm.tm_sec = 0;//atoi( sec );
	
	

   mktime( &st_Tm );

	printf("convert time = (time_t)%u, (ui)%u, (d)%d(%u) \n", tTmp, ttt, i,i);
	printf("convert binary(%d:%d)\n", mktime( &st_Tm ), (((int)tTmp/86400)*86400) );
	
	tCurrent = 0;
	tPrev = 0;
	for(i=0;i< 30;i++ ){
		tCurrent = time(0);

		localtime_r(&tCurrent,&stTime);
		localtime_r(&tPrev, &stPrevTime);

		printf( "[%d] tCurrent=%u, tPrev=%u\n",i, tCurrent,tPrev);
		printf( "[%d] stTime =%d-%d-%d %d:%d:%d, stPrevTime =%d-%d-%d %d:%d:%d\n",i,
			stTime.tm_year+1900, stTime.tm_mon+1, stTime.tm_mday, stTime.tm_hour, stTime.tm_min, stTime.tm_sec ,
			stPrevTime.tm_year+1900, stPrevTime.tm_mon+1, stPrevTime.tm_mday, 
			stPrevTime.tm_hour, stPrevTime.tm_min, stPrevTime.tm_sec );
		tPrev = tCurrent;
		sleep(1);
	}
	return 0;
}

void printTime(struct tm stm)
{
	printf("year  : %d\n", stm.tm_year+1900);
	printf("month : %d\n", stm.tm_mon+1);
	printf("day   : %d\n", stm.tm_mday);
	printf("hour  : %d\n", stm.tm_hour);
	printf("min   : %d\n", stm.tm_min);
	printf("sec   : %d\n", stm.tm_sec);
}

void cvtTime(struct tm *stm)
{
	stm->tm_min = stm->tm_min +1;
}

int test3(time_t s, time_t e)
{
	struct tm sttm_s, sttm_e;

	localtime_r( (const time_t*)&s, &sttm_s );
	localtime_r( (const time_t*)&e, &sttm_e );
	printf("start_time=%d, end_time=%d\n", s,e);
	printf("=== start time ===\n");
	printTime(sttm_s);
	cvtTime(&sttm_s);
	printTime(sttm_s);
	printf("=== end time ===\n");
	printTime(sttm_e);
	return 0;
}

int isNEXT(unsigned int n, unsigned int e)
{
	struct tm stn, ste;

	if( n > e ){
		return 0;
	}

	localtime_r( (const time_t*)&n, &stn );
	localtime_r( (const time_t*)&e, &ste );
	
	if( stn.tm_year < ste.tm_year ||
		stn.tm_mon  < ste.tm_mon  ||
		stn.tm_mday < ste.tm_mday ||
		stn.tm_hour < ste.tm_hour ||
		stn.tm_min  < ste.tm_min
	){
		return 1;
	}
}

time_t NEXT_MIN(unsigned int t)
{
	struct tm stm;
	localtime_r((const time_t*)&t, &stm);
	printf("PATH(%u) = /%04d/%02d/%02d/%02d/[0_X]/%02d.DAT\n",
		t,stm.tm_year+1900, stm.tm_mon+1, stm.tm_mday,
		stm.tm_hour, stm.tm_min);

	//stm.tm_min++;
	stm.tm_sec = 0;
	//printf("mktime=>%u\n", mktime(&stm));
	return mktime(&stm);
}

void test4(unsigned int s, unsigned int e)
{
	unsigned int n,t;
	t = NEXT_MIN(s);
	n = s;
	while( isNEXT(n,e) ){
		NEXT_MIN(n);
		if( t ){
			n = t;
			t = 0;
		}
		n = n +60;
	}
}

unsigned int getNextMin(unsigned int t, char *szBuf)
{
	struct tm stm;
	time_t tt;

	tt = (time_t)t;
	localtime_r( (const time_t*)&tt, &stm );
	sprintf(szBuf, "PATH2(%u) = /%04d/%02d/%02d/%02d/%02d",
		t, stm.tm_year+1900, stm.tm_mon+1, stm.tm_mday,
		stm.tm_hour, stm.tm_min);

	//return t+60;
	stm.tm_min++;
	return mktime(&stm);
}

int dGetTailNoWithIMSI(char *val)
{
	int i;
	//e.g. "450061032275557"
	if( strlen(val) != 15 ){
		return -1;
	}

	for( i = 0; i< strlen(val); i++ ){
		if( !isdigit(val[i]) ){
			printf("ERROR IMSI=%s\n", val);
			return -2;
		}
	}
	return atoi(&val[13]);
}

int dGetTailNoWithIP(char *val)
{
	int dRet;
	unsigned int uiIP;

	printf("IPVal=%s\n", val);
	dRet = inet_pton(AF_INET, val, &uiIP);
    if( dRet < 0 ){
		printf("invalid format(1):%s, dRet=%d\n",val,dRet);
		return -1;
    }else if( !dRet ){
		printf("invalid format(2):%s, dRet=%d\n",val,dRet);
		return -2;
	}

    uiIP = ntohl(uiIP);
	printf("IPTail=%d\n", uiIP&0xFF);
	return (uiIP & 0xff);
}

int getTailNo(int type, char *val)
{
	switch(type){
		case 0://imsi
			return dGetTailNoWithIMSI(val);
		case 1://ip
			return dGetTailNoWithIP(val);
	}
	return -3;
}

void ccc( char *src, int no )
{
	sprintf( &src[strlen(src)], "_%03d",no );
	strcat(src, ".pcap");
}

void test2(unsigned int s, unsigned int e)
{
	char szBuf[256];
	char szIP[60];//="192.168.0.36";
	char szIMSI[60];//="450061032275557";
	int  type= 0;
	int  d;
	unsigned int n;

	sprintf(szIP,"192.168.0.0");
	sprintf(szIMSI,"450081032275500");
	n = s;
	while( isNEXT(n,e) ){
		//printf("TIME=%u :: ",n);
		n = getNextMin(n,szBuf);
		printf("TIME=%s\n", szBuf);
		ccc( szBuf, getTailNo(1,szIP) );
		//sprintf( &szBuf[strlen(szBuf)],"_%03d",getTailNo(1, szIP) );
		printf("TIME2=%s\n", szBuf);
	}
	printf("IMSI=%s, TAILNO=%03d\n", szIMSI, getTailNo(0,szIMSI));
	if( getTailNo(1,szIMSI) < 0 ){
		printf("NEGATIVE\n");
	}
	printf("IP  =%s, TAILNO=%03d\n", szIP, getTailNo(1,szIMSI));
	printf("IP  =%s, TAILNO=%03d\n", szIP, getTailNo(1,szIP));
	
}


int main(int ac, char **av)
{
	int i;

	//test();
	if( ac == 3 ){
		test2( atoi(av[1]), atoi(av[2]) );
	}

	printf("time_t=%d\n", sizeof(time_t));
	char *ab="3cd";

	for( i = 0; i < strlen(ab); i++ ){
	if( isdigit(ab[i]) )
		printf("NUMBER=%c\n",ab[i]);
	else
		printf("IS NOT NUMBER=%c\n",ab[i]);
	}

	time_t ct;
	ct = time(NULL);
	printf("curtime=%ld, minortime=%ld\n", ct, ct/60*60);
	return 0;
}
