#include "samd.h"
#include "commlib.h"
#include "sfm_msgtypes.h"

#ifdef TRU64

#include <sys/mount.h>

extern	char	trcBuf[4096], trcTmp[1024];
extern	int		trcFlag, trcLogFlag;
extern	SFM_SysCommMsgType *loc_sadb;


get_diskUsage()
{
	int		i;
	long	hdd_used,hdd_avail;
	struct statfs	fsbuf;

	for (i=0 ; i < loc_sadb->diskCount ; i++)
	{
		if (statfs(loc_sadb->loc_disk_sts[i].diskName, &fsbuf) < 0) {
			sprintf(trcBuf," statfs[%s], [%d] => %s\n",loc_sadb->loc_disk_sts[i].diskName, errno, strerror(errno));
			trclib_writeLogErr(FL, trcBuf);
			exit(1);
		}
		hdd_used = (long)(fsbuf.f_blocks - fsbuf.f_bfree)*fsbuf.f_fsize/1024;
		hdd_avail = (long)fsbuf.f_bavail*fsbuf.f_fsize/1024;
		loc_sadb->loc_disk_sts[i].disk_usage = 1000 * hdd_used / (hdd_used+hdd_avail);
	}
}

#endif /* TRU64 */
