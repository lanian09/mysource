#include "samd.h"

#ifndef TRU64

#include <sys/mnttab.h>
#include <sys/statvfs.h>
#include <sys/vfstab.h>

extern int		trcFlag, trcLogFlag;
extern char		trcBuf[4096], trcTmp[1024];
extern SFM_SysCommMsgType *loc_sadb;
extern  STM_LoadOMPStatMsgType    system_statistic;

int mount_disk_check(char *mnt_mountp);

int get_diskUsage (void)
{
	FILE	*fp=NULL;
	int	ret, diskIdx;
	float	usage;
	struct mnttab	mnt_tab;
	struct statvfs	fs_info;

	if ((fp = fopen("/etc/mnttab", "r")) == NULL) {
    	sprintf(trcBuf,"[get_diskUsage] fopen fail[/etc/mnttab]; err=%d(%s)\n",
    			errno, strerror(errno));
    	trclib_writeLogErr (FL,trcBuf);
    	return -1;
	}

	while ((ret = getmntent(fp, &mnt_tab)) == 0)
	{
		if ((diskIdx = mount_disk_check(mnt_tab.mnt_mountp)) < 0)
			continue;

		if (statvfs (mnt_tab.mnt_mountp, &fs_info) < 0) {
			sprintf(trcBuf,"[get_diskUsage] statvfs fail[/etc/mnttab]; err=%d(%s)\n",
					errno, strerror(errno));
			trclib_writeLogErr (FL,trcBuf);
			if(fp)fclose(fp);
			return -1;
		}

		if (fs_info.f_blocks == 0 || fs_info.f_bavail == 0) {
			usage = 0;
		} else {
			usage = (((float)(fs_info.f_blocks - fs_info.f_bavail)) / (float)fs_info.f_blocks) * 1000;
		}

		loc_sadb->loc_disk_sts[diskIdx].disk_usage = (short)usage;
	}

	if(fp)fclose(fp);
	//////////////////

	if (ret == -1)
		return 1;

	switch(ret) {
		case MNT_TOOLONG :
#ifdef DEBUG
			fprintf (stdout, "[get_diskUsage] The line is too long");
#endif
			break;
		case MNT_TOOMANY :
#ifdef DEBUG
			fprintf (stdout, "[get_diskUsage] The line has too fields");
#endif
			break;
		case MNT_TOOFEW :
#ifdef DEBUG
			fprintf (stdout, "[get_diskUsage] The line has not enough fields");
#endif
			break;
	}
	return -1;
}


//------------------------------------------------------------------------------
// 관리대상 disk partition인지 확인하고, loc_sadb->loc_disk_sts에서의 index를 return한다.
//------------------------------------------------------------------------------
int mount_disk_check(char *mnt_mountp)
{
	int  i;

	for(i=0; i<loc_sadb->diskCount; i++) {
		if (!strcasecmp(loc_sadb->loc_disk_sts[i].diskName, mnt_mountp)) {
			return i;
		}
	}
	return -1;
}

#endif /* not_TRU64 */
