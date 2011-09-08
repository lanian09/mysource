/**A.1*  File Inclusion ***********************************/
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "typedef.h"
#include "mmcdef.h"

static char statcrmtime[81];
extern char *MH_ErrMess(short);
extern int CONT_FLAG;


void P_stat_load (char *buf, dbm_msg_t *msg, short *sCurCnt, short *sTotCnt) 
{
	char tmp[1024];

	if( msg->common.mml_err< 0)
	{
		strcat (buf, "\n  RESULT = FAIL");
		sprintf (tmp, "\n  REASON = %s\n", MH_ErrMess(msg->common.mml_err));
		strcat (buf, tmp);
	} else {
		strcat (buf, "\n  RESULT = SUCCESS");
		sprintf (tmp, "\n  %s\n", msg->data ); 
		strcat (buf, tmp);
	}
	
	return;
}

char *crStatTime(time_t when)
{
    strftime(statcrmtime, 80, "%m-%d %H:%M", localtime((time_t *)&when));
    return statcrmtime;
}

char *crStatTime2(time_t when)
{
    strftime(statcrmtime, 80, "%m-%d %H:%M", localtime((time_t *)&when));
    return statcrmtime;
}

