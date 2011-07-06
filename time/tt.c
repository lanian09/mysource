#include <stdio.h>
#include <string.h>
#include <time.h>

#define STAT_OFFSET_UNIT 300
void printff3(time_t now)
{
    struct tm   *pLocalTime;
	char    inserttimeStamp3[32];

	if ((pLocalTime = (struct tm*)localtime((time_t*)&now)) == NULL) {
        strcpy(inserttimeStamp3, "");
    } else {
        strftime (inserttimeStamp3, 32, "%a", pLocalTime);
    }       
	printf("GET_INSERT_TIME3=%s\n", inserttimeStamp3);

}

void printff2(time_t now)
{
    struct tm   *pLocalTime;
	char    inserttimeStamp3[32];

	if ((pLocalTime = (struct tm*)localtime((time_t*)&now)) == NULL) {
        strcpy(inserttimeStamp3, "");
    } else {
        strftime (inserttimeStamp3, 32, "%Y-%m-%d %H:%M", pLocalTime);
    }       
	printf("GET_INSERT_TIME2=%s\n", inserttimeStamp3);

}

void printff(time_t now)
{
    struct tm   *pLocalTime;
	char    inserttimeStamp3[32];

    if ((pLocalTime = (struct tm*)localtime((time_t*)&now)) == NULL) {
        strcpy(inserttimeStamp3, ""); 
    } else {
        strftime (inserttimeStamp3, 32, "%Y-%m-%d %H:%M", pLocalTime);
    }   

	printf("GET_INSERT_TIME1=%s\n", inserttimeStamp3);
}

void get_insert_time3()
{   
    time_t      now,now2;
    
    now = time(0);
	now2 = (now/STAT_OFFSET_UNIT)*STAT_OFFSET_UNIT;
    
	printff(now);
	now = now + 300 - STAT_OFFSET_UNIT;
	now = (now/STAT_OFFSET_UNIT)*STAT_OFFSET_UNIT;
	printff(now);
	//printff(now);
	//printff3(now-STAT_OFFSET_UNIT);
	//printff2(now2);
    now = ((now-STAT_OFFSET_UNIT)/STAT_OFFSET_UNIT) *STAT_OFFSET_UNIT;
	//printff(now);
	//printff3(now-STAT_OFFSET_UNIT);
	//printff2(now2);
            
}   

int main()
{
	get_insert_time3();
	return 0;
}
