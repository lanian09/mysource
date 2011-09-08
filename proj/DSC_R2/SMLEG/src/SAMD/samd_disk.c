#include "samd.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>

#ifdef __LINUX__  /*sjjeon*/
#include <mntent.h>
#include <sys/vfs.h>
#else
#include <sys/mnttab.h>
#include <sys/statvfs.h>
#include <sys/vfstab.h>
#endif

#define LOGICAL_DRIVE_CHANGE	"Logical Drive Status Change"
#define PHYSICAL_DRIVE_CHANGE	"Physical Drive Status Change"
#define STATUS_IS_NOW			"Status is now"

#define LOGICAL_READ			0
#define LOGICAL_STATUS_READ		1
#define PHYSICAL_READ			2
#define PHYSICAL_STATUS_READ	3

extern int		trcFlag;
extern int		trcLogFlag;

extern char		trcBuf[TRCBUF_LEN];
extern char		trcTmp[TRCTMP_LEN];
extern char		systemModel[5];

extern DiskConfig				disk_conf;
extern SFM_SysCommMsgType		*loc_sadb;
extern STM_LoadMPStatMsgType	system_statistic;
//extern int findCaseIndex(char *syntax_ori, char *idx);

char	szMessageBody[409600*20];
off_t	gdMsgOldSize;
char	szEnclosure[2][32];

static char	readContinue;

#if 0
//                                 Drive  Target  SCSI
static char diskStatusMap[4][3] = { {1,     0,      2}, // Drive Number : 0
                                    {2,     1,      2}, // Drive Number : 1
                                    {1,     0,      1}, // Drive Number : 2
                                    {2,     1,      1}};// Drive Number : 3

//                                 Drive  Bay  Box
static char diskStatusMap2[4][3] = { {1,     1,      1}, // Drive Number : 0
                                    {1,     2,      1}, // Drive Number : 1
                                    {2,     3,      1}, // Drive Number : 2
                                    {2,     4,      1}};// Drive Number : 3
#define DISK_STATUS_NORMAL      0
#define DISK_STATUS_RECOVERING  1
#define DISK_STATUS_FAILED      2
#define DISK_STATUS_REBUILDING  3
#define DISK_STATUS_OK          4

static char diskStaus[4] = {DISK_STATUS_NORMAL, DISK_STATUS_NORMAL,
                            DISK_STATUS_NORMAL, DISK_STATUS_NORMAL };

#endif
int check_disk_status(void);

int get_diskUsage(void);
int mount_disk_check(char *mnt_mountp);
int check_messages_file(void);
int parse_string_num(char *msgs);

// 20100608 by dcham
extern char volumeName[20];
extern char diskNameList1[20];
extern char diskNameList2[20];

#ifdef __LINUX__
int get_diskUsage (void)
{
	int             dRet;
    int             i, disk_usage[SFM_MAX_DISK_CNT];
    struct statfs   buf;

    int     	    dPercent;
    int 	    Total_Percent;
    long long       llUsed;
    long long       Total_usage;
    long  	    Total_blocks;
    long      	    Total_frees;
    long       	    Total_bavail;

    for(i=0; i<loc_sadb->diskCount; i++) {
    	dRet = statfs(loc_sadb->loc_disk_sts[i].diskName, &buf);
	if(dRet < 0)
	{
//		printf("STATFS ERROR -> dRet = [%d]\n",dRet);
	        continue;
	}

	if (buf.f_blocks == 0 || buf.f_bavail == 0)
	{
		dPercent = 0;
	}
	else
	{
		llUsed = buf.f_blocks - buf.f_bfree;
		// Total_disk_usage check by helca 11.21
		//
	        Total_blocks  += buf.f_blocks;
	        Total_frees   += buf.f_bfree;
	        Total_bavail  += buf.f_bavail;

		dPercent = (long)(llUsed * 100.0 / (llUsed + buf.f_bavail) + 0.5);
        }

	loc_sadb->loc_disk_sts[i].disk_usage = htons((short)dPercent); // by helca 07.31
    }
    // by helca 11.21
    //

    Total_usage = Total_blocks - Total_frees;

    Total_Percent = (long)(Total_usage * 100.0 / (Total_usage + Total_bavail) + 0.5);
//    fprintf(stderr, "Total_usage: %d\n", Total_Percent);
    loc_sadb->total_disk_usage =  htons((short)Total_Percent); // by helca 11.21
//    fprintf(stderr, "Totol_usage_htons: %d\n", loc_sadb->total_disk_usage);
    return 1;
}
#else/* NOT LINUX */
int get_diskUsage(void)
{
	FILE			*fp;
	int				ret, diskIdx;
	float			usage;
	struct mnttab	mnt_tab;
	struct statvfs	fs_info;

	double 			sum_blocks = 0;
	double 			sum_bavail = 0;
    unsigned short  sum_usage = 0;
//	int				total_cnt = 0;
	int				cnt = 0;

	if( (fp = fopen("/etc/mnttab", "r")) == NULL)
	{
		sprintf(trcBuf,"[%s] fopen fail[/etc/mnttab]; err=%d(%s)\n", __FUNCTION__, errno, strerror(errno));
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	while( (ret = getmntent(fp, &mnt_tab)) == 0)
	{
// yhshin total disk usage 구하기 위해
//		if( (diskIdx = mount_disk_check(mnt_tab.mnt_mountp)) < 0)
//			continue;
		diskIdx = mount_disk_check(mnt_tab.mnt_mountp);

		if(statvfs (mnt_tab.mnt_mountp, &fs_info) < 0)
		{
			sprintf(trcBuf,"[%s] statvfs fail[/etc/mnttab]; err=%d(%s)\n", __FUNCTION__, errno, strerror(errno));
			trclib_writeLogErr(FL, trcBuf);
			fclose(fp);
			return -1;
		}

		if( (fs_info.f_blocks == 0) || (fs_info.f_bavail == 0))
			usage = 0;
		else {
			usage = (((float)(fs_info.f_blocks - fs_info.f_bavail)) / (float)fs_info.f_blocks) * 1000;
		}

		if (diskIdx >= 0) {
			cnt++;
			sum_blocks += fs_info.f_blocks;
			sum_bavail += fs_info.f_bavail;
			loc_sadb->loc_disk_sts[diskIdx].disk_usage = (short)usage;	
			sum_usage += (short)usage; // 07.17 usage  sum 
		}

	}
#if 0
		loc_sadb->total_disk_usage  = (sum_usage)/cnt/10;
#else
	// core 발생처리... cnt != 0 , sjjeon
	if(cnt > 0){
		//	loc_sadb->total_disk_usage  = ((sum_blocks - sum_bavail) / sum_blocks) *100;
		loc_sadb->total_disk_usage  = (sum_usage)/cnt/10;
	}else{
		sprintf(trcBuf,"[%s] disk mount load fail.(%d)\n", __FUNCTION__, cnt);
		trclib_writeLogErr(FL, trcBuf);
		fclose(fp);
		return -1;
	}
#endif

	fclose(fp);

	if(ret == -1)
		return 1;

	return -1;
}
#endif

//------------------------------------------------------------------------------
// 관리대상 disk partition인지 확인하고, loc_sadb->loc_disk_sts에서의 index를 return한다.
//------------------------------------------------------------------------------
int mount_disk_check(char *mnt_mountp)
{
	int  i;

	for(i = 0; i < loc_sadb->diskCount; i++)
	{
		if(!strcmp(loc_sadb->loc_disk_sts[i].diskName, mnt_mountp))
			return i;
	}

	return -1;
}

/*

by sjjeon
-  T2000 Sun 장비의 H/W Disk 정보 상태
-  H/W Disk 2 EA 
 */
int check_disk_status(void)
{
    FILE *fp =NULL;
    const int _BUFSIZE =512;
    char buf[_BUFSIZE];
    int i=0, retry_cnt=0;
	int bk_disk1, bk_disk2 ;
	char cmd[256]; 

	// 초기화 
	bk_disk1 = 1;	//DEAD 
	bk_disk2 = 1;	//DEAD 


RETRY_CHECK:

	// retry 실패..
	if(retry_cnt>3){
        sprintf(trcBuf, "[%s] re-try fail.", __FUNCTION__);
		trclib_writeLogErr(FL, trcBuf);
		goto DISK_OUT;
	}
        // 20100608 by dcham
	sprintf (cmd, "/usr/sbin/raidctl -l ");
        strcat(cmd, volumeName);
        strcat(cmd, " 2>&1 > /tmp/disklivecheck.txt");
	my_system (cmd);



    fp = fopen("/tmp/disklivecheck.txt","r");
    if(fp ==NULL){
        sprintf(trcBuf, "[%s] fopen error....", __FUNCTION__ );
		trclib_writeLogErr(FL, trcBuf);
		retry_cnt++;
        goto RETRY_CHECK;
    }


	// init 
	i = 0;
    while(fgets(buf, _BUFSIZE, fp) != NULL)
    {
		if (i>=2) continue;

		// 0.0.0 : first disk  20100608 by dcham
		if(strstr(buf, diskNameList1))
		{
			if(strstr(buf,"GOOD")){
				bk_disk1 = 0;
				//fprintf(stderr,"disk1 info : %d\n", 0);
			}
			i++;	
			continue;
				
		}
		// 0.1.0 : second disk check 20100608 by dcham
		else if(strstr(buf,diskNameList2))
		{
			if(strstr(buf,"GOOD")){
				bk_disk2 = 0;
				//fprintf(stderr,"disk2 info : %d\n", 0);
			}
			i++;
			continue;
		}

	} /*while*/

	// Disk1, Disk2 Check
	if(i!=2){
		retry_cnt++;
	    if(fp)fclose(fp);
		goto RETRY_CHECK;
	}

DISK_OUT:
	loc_sadb->sysHW[get_hwinfo_index ("DISK1")].status = bk_disk1;
	loc_sadb->sysHW[get_hwinfo_index ("DISK2")].status = bk_disk2;
	sprintf(&loc_sadb->sysSts.diskSts[0].StsName[0], "DISK1");
	loc_sadb->sysSts.diskSts[0].status = bk_disk1;
	sprintf(&loc_sadb->sysSts.diskSts[1].StsName[0], "DISK2");
	loc_sadb->sysSts.diskSts[1].status = bk_disk2;

    if(fp)fclose(fp);
	return 0;
}
/* End of check_disk_status */

/*******************************************************************************
 MESSAGE FILE에 변화가 있는지 확인하는 함수
*******************************************************************************/
int check_messages_file(void)
{
	FILE    *fp;
	int		dLen, strLen;

	char	szBuffer[256];
    char    tmpBuff[409600];

	struct stat 	stStat;

	stat( MESSAGE_FILE, &stStat );

	if( stStat.st_size < gdMsgOldSize )
	{
		/*** MESSAGE FILE이 새로 업데이트 된 경우 *****************************/
		sprintf(trcBuf, "[check_messages_file] messages FILE update prev[%ld] old[%ld]\n",
			stStat.st_size, gdMsgOldSize);
		trclib_writeLogErr (FL,trcBuf);

		fp = fopen( MESSAGE_FILE_1, "r" );
        if( fp == NULL )
        {
            sprintf(trcBuf, "[check_messages_file] messages FILE OPEN ERROR [%d] [%s]\n",
                      errno, strerror(errno) );
			trclib_writeLogErr (FL,trcBuf);
            return -1;
        }

		dLen = 0;

		fseek( fp, gdMsgOldSize, SEEK_SET );

        memset(szBuffer, 0x00, sizeof(szBuffer));
        memset(tmpBuff, 0x00, sizeof(tmpBuff));
        while( fgets(szBuffer, 256, fp) != NULL )
        {
            if(strlen(szBuffer) == 0)
                break;
            strcpy(&tmpBuff[dLen], &szBuffer[0] );
            dLen += strlen(szBuffer);
            memset(szBuffer, 0x00, sizeof(szBuffer));
        }

        fclose(fp);

		fp = fopen( MESSAGE_FILE, "r" );
        if( fp == NULL )
        {
            sprintf(trcBuf, "[check_messages_file] messages FILE OPEN ERROR [%d] [%s]\n",
                      errno, strerror(errno) );
			trclib_writeLogErr (FL,trcBuf);
            return -1;
        }

        memset(szBuffer, 0x00, sizeof(szBuffer));
        while( fgets(szBuffer, 256, fp) != NULL ){
            strLen = strlen(szBuffer);
            if(szBuffer[strLen-1] == '\n'){
                strcpy(&tmpBuff[dLen], &szBuffer[0] );
                dLen += strLen;
            }else{
                break;
            }
            memset(szBuffer, 0x00, sizeof(szBuffer));
        }

        fclose(fp);

        gdMsgOldSize += dLen;

        if(readContinue)
            strcat(szMessageBody, tmpBuff);
        else
            strcpy(szMessageBody, tmpBuff);


		return 1;
	}
	else if( stStat.st_size > gdMsgOldSize ){
		/*** MESSAGE FILE에 추가된 경우 ***************************************/
		sprintf(trcBuf, "[check_messages_file] messages FILE alter prev[%ld] old[%ld]\n",
			stStat.st_size, gdMsgOldSize);
		trclib_writeLogErr (FL,trcBuf);

		fp = fopen( MESSAGE_FILE, "r" );
		if( fp == NULL )
		{
            sprintf(trcBuf, "[check_messages_file] messages FILE OPEN ERROR [%d] [%s]\n",
                      errno, strerror(errno) );
			trclib_writeLogErr (FL,trcBuf);
			return -1;
		}

		fseek( fp, gdMsgOldSize, SEEK_SET );

		dLen = 0;
        memset(szBuffer, 0x00, sizeof(szBuffer));
        memset(tmpBuff, 0x00, sizeof(tmpBuff));
		while( fgets(szBuffer, 256, fp) != NULL )
		{
            strLen = strlen(szBuffer);
            if(szBuffer[strLen-1] == '\n'){
                strcpy(&tmpBuff[dLen], &szBuffer[0] );
                dLen += strLen;
            }else{
                break;
            }
            memset(szBuffer, 0x00, sizeof(szBuffer));
		}

		fclose(fp);

		gdMsgOldSize += dLen;

        if(readContinue)
            strcat(szMessageBody, tmpBuff);
        else
            strcpy(szMessageBody, tmpBuff);

		return 1;
	}

	return 0;
}

int parse_string_num(char *msgs)
{
	char	*pDiskNum, *p, buff[10];

	p = msgs;

	while(isspace(*p))
		p++;

	if(*p == 0x00)
		return -1;

	memset(buff, 0x00, sizeof(buff));
	pDiskNum = p;
	while(isdigit(*p))
		p++;

	if(*pDiskNum != 0x00)
	{
		memcpy(buff, pDiskNum, p - pDiskNum);
		return (atoi(buff));
	}
	else
		return -1;
}
