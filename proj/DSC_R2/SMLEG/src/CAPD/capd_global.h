#ifndef __CAPD_GLOBAL_H__
#define __CAPD_GLOBAL_H__

#include <sfm_msgtypes.h>

int dag_devnum;
int dag_fd;
int fcs_bits = 32; /* default Ethernet FCS bits size */
int loop_continue = 1;
SFM_SysCommMsgType *SFMSysCommMsgType = NULL;
char syslabel[64] = {0};

#endif /* __CAPD_GLOBAL_H__ */

