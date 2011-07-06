#include <stdio.h>
#include <string.h>
#include <time.h>

enum {
	STMD_MIN = 0,
	STMD_HOUR,
	STMD_DAY,
	STMD_WEEK,
	STMD_MONTH
};

#define STAT_OFFSET_UNIT 300
#define STMD_1HOUR_OFFSET (60*60*24)

char *get_select_time(int time_type)
{
    time_t      now;
    struct tm   *pLocalTime;
    int         wday;
	char        selecttimeStamp[32];
   
    if (time_type == STMD_HOUR){ 
        now = time(0) + (60*60);
        now = now-(STAT_OFFSET_UNIT);// now - 300 5분 단위로 만들어 준다
    } else{ // DAY,WEEK,MONTH
        now = time(0);
        now = now-(STAT_OFFSET_UNIT);
    }

    if ((pLocalTime = (struct tm*)localtime((time_t*)&now)) == NULL) {
        strcpy(selecttimeStamp, "");
    } else {
        switch (time_type) {
            case    STMD_MIN:
                strftime (selecttimeStamp, 32, "%Y-%m-%d %H:%M", pLocalTime);
                break;
            case    STMD_HOUR:
                strftime (selecttimeStamp, 32, "%Y-%m-%d %H", pLocalTime);
                break;
            case    STMD_DAY:
                strftime (selecttimeStamp, 32, "%Y-%m-%d ", pLocalTime);
                break;
            case    STMD_WEEK:
                // 월요일의 시간이 구해진다
                if(!pLocalTime->tm_wday)
                    wday = 7;
                else
                    wday = pLocalTime->tm_wday;
                now = now - (wday * STMD_1HOUR_OFFSET) + STMD_1HOUR_OFFSET;
                pLocalTime = (struct tm*)localtime((time_t*)&now);
                strftime (selecttimeStamp, 32, "%Y-%m-%d 00:00:00", pLocalTime);
                break;
            case    STMD_MONTH:
                strftime (selecttimeStamp, 32, "%Y-%m-01 00:00:00", pLocalTime);
                break;
        }
    }
	
	printf("%s\n", selecttimeStamp);
}

int main()
{
	int i;
	printf("time check test start\n");
	for(i=0;i<5;i++){
		get_select_time(i);
	}
	return 0;
}
