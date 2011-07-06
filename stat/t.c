#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>

#define PROC_NAME_LEN 16
#define BUF_LEN 50000
#define PROC_PATH "/proc"
#define PARENT_PATH ".."
#define HOME_PATH "."

char szTime[20];

char* convert_time( time_t when )
{
    struct tm *tm_when;

    memset( szTime, 0x00, 20 );

	tm_when = localtime( &when );

	sprintf( szTime, "%02d/%02d %02d:%02d", tm_when->tm_mon+1, tm_when->tm_mday, tm_when->tm_hour, tm_when->tm_min );
	//printf("szTime=%s\n", szTime);
	return szTime;
}

int GetProcessID(int type)
{
    int         fd,mypid;
    int         dPCnt = 0;
    FILE        *fp_tot;
    FILE        *fp_pro;
    DIR         *dirp;
    struct dirent *direntp;
    char        pname[PROC_NAME_LEN];
    char        tempbuf[BUF_LEN];

    char        szBuffer[20480];
    char        *szTmp;
    int         dReadSize;
    int         dTotVal;
    //unsigned int  dProcVal;
    long  lProcVal;
    unsigned long ulProcVal;
	long        starttime;

	char		szType[256];
	char		szFull[256];
	char		typename[20];

    memset(szBuffer, 0x00, 20480 );

switch(type){
	case 1:
		sprintf(szType, "/home/uamyd/test/stat/log/normal");
		sprintf(typename,"btime");
		break;
	case 2:
		sprintf(szType, "/home/uamyd/test/stat/log/hnormal");
		sprintf(typename,"btime");
		break;
	case 3:
		sprintf(szType, "/home/uamyd/test/stat/log/abnormal");
		sprintf(typename,"btime");
		break;
	default:
		sprintf(szType, "/home/uamyd/test/stat/log/normal");
		sprintf(typename,"btime");
		break;
}
	sprintf(szFull, "%s/%s", szType, typename);
printf("fullname=%s\n", szFull);
    if( (fp_tot = fopen( szFull, "r")) == NULL)
    {
        fprintf(stderr, "CANNOT OPEN BTIME FILE\n");
        return -1;
    }

    dReadSize = fread( szBuffer, 20480, 1, fp_tot);
	//sprintf(szTmp, "%s", szBuffer);
/*
    szTmp = strstr(szBuffer, "btime");

	sscanf( szTmp, "%*s %d", &dTotVal );
*/
	sscanf( szBuffer, "%d", &dTotVal);
    fclose(fp_tot);
//printf("read data=%s\n", szTmp);



    //if((dirp = opendir(PROC_PATH)) == (DIR *)NULL)
    if((dirp = opendir(szType)) == (DIR *)NULL)
    {
        fprintf(stderr, "\n\tCAN'T ACCESS PROCESS DIRECTORY (%s)\n", PROC_PATH);
		return -1;
    }
    while((direntp = readdir(dirp)) != NULL)
    {
        dPCnt++;
        if( dPCnt > 32000 )
            break;

        if(!strcmp(direntp->d_name, PARENT_PATH) ||
           !strcmp(direntp->d_name, HOME_PATH) ||
           !strcmp(direntp->d_name, "dis.log") ||
           !strcmp(direntp->d_name, "stat.log") ||
           !strcmp(direntp->d_name, "t.sh") ||
           !strcmp(direntp->d_name, "sa1_pi.log") ||
           !strcmp(direntp->d_name, "sa2_rp.log") ||
           !strcmp(direntp->d_name, "dj_pi.log") 
			) continue;

        if( !atoi(direntp->d_name) )
        {
            continue;
        }

        //sprintf(tempbuf, "%s/%s/cmdline", PROC_PATH, direntp->d_name);
        sprintf(tempbuf, "%s/%s", szType, direntp->d_name);

        fd = open(tempbuf, O_RDONLY);

        if(fd < 0)
        {
            close(fd);
            continue;
        }

/*
        memset( pname, 0x00, PROC_NAME_LEN);
        if( read(fd, pname, PROC_NAME_LEN-1) < 0 )
        {
            close(fd);
            continue;
        }
        else
        {
            close(fd);
        }


        pname[PROC_NAME_LEN-1] = 0x00;

		printf("check process = %s\n", name);
*/

 //       if( !strcmp(name, pname) || (strstr("gdb", pname) && strstr( name,pname) ))
  //      {
   //         sprintf(tempbuf, "%s/%s/stat", PROC_PATH, direntp->d_name);
   //         sprintf(tempbuf, "%s/%s/stat", PROC_PATH, direntp->d_name);

#if 1
            fp_pro = fopen( tempbuf, "r");

            dReadSize = fread( szBuffer, 20480, 1, fp_pro);

/*
            sscanf(szBuffer, "%*d %*s %*c %*d %*d %*d %*d %*d %*lu %*lu %*lu %*lu %*lu %*lu %*lu %*ld %*ld %*ld %*ld %*ld %*ld %lu %*lu %*ld %*lu %*lu %*lu %*lu %*lu %*lu %*lu %*lu %*lu %*lu %*lu %
*lu %*lu %*d %*d", &ulProcVal );
*/
            sscanf(szBuffer, "%d %s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %*u %*u %*d %*d %*d %*d %*d %*d "
                             "%lu %*u %*d %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*d %*d", &mypid, typename, &ulProcVal );
/*
2 (migration/0) S 1 1 1 0 -1 268468288 0 0 0 0 0 282 0 0 -100 -5 1 0 8 0 0 18446744073709551615 0 0 0 0 0 0 2147483647 0 0 18446744073709551615 0 0 17 0 99 1 0
3 3 0 5 0 0 0 0 0 96 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 235490 0 0 0 0 0 0 0 99614 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
*/

            fclose(fp_pro);

            //dProcVal = (unsigned int)ulProcVal/100;
            lProcVal = (long)ulProcVal/100;

            starttime = dTotVal + lProcVal;
//			printf("szBuff=%s\n", szBuffer);
			//printf("PROC=%s(%d)\t[%s]\tSTARTTIME=%ld dTotVal/dProcVal=%d/%u\n", typename, mypid, convert_time(starttime), starttime, dTotVal, dProcVal); 
			printf("PROC=%s(%d)\t[%s]\tSTARTTIME=%ld dTotVal/lProcVal=%d/%ld\n", typename, mypid, convert_time(starttime), starttime, dTotVal, lProcVal); 
			//convert_time(starttime);
#endif
            //starttime = time(&starttime);
            //return atoi(direntp->d_name);
			//return 0;
    //    }
    }

    closedir(dirp);

    return -2;
}

int main(int ac, char **av)
{
	if( ac == 2 ){
	GetProcessID(atoi(av[1]));
	}else{
	GetProcessID(0);
	}
	return 0;
}
