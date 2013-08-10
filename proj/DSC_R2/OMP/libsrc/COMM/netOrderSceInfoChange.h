#include "sfm_snmp.h"
#include "sfmconf.h"

/*########################### Network To Host ##############################*/

void SFM_SCE_N2H(SFM_SCE *sce); /*SCE Main*/
void SFM_SCEDev_N2H(SFM_SCEDev *sceDev);
void SCE_SYS_INFO_N2H(SCE_SYS_INFO *sysInfo);
void SCE_SYS_USAGE_INFO_N2H(SCE_SYS_USAGE_INFO *sysUseInfo);
/* hjjung */
void SCE_SYS_NUM_INFO_N2H(SCE_SYS_NUM_INFO *sysUseInfo);
void SCE_SYS_STATUS_INFO_N2H(SCE_SYS_STATUS_INFO *sysStsInfo);

/*########################### Host To Network ##############################*/

void SFM_SCE_H2N(SFM_SCE *sce); /*SCE Main*/
void SFM_SCEDev_H2N(SFM_SCEDev *sceDev);
void SCE_SYS_INFO_H2N(SCE_SYS_INFO *sysInfo);
void SCE_SYS_USAGE_INFO_H2N(SCE_SYS_USAGE_INFO *sysUseInfo);
/* hjjung */
void SCE_SYS_NUM_INFO_H2N(SCE_SYS_NUM_INFO *sysUseInfo);
void SCE_SYS_STATUS_INFO_H2N(SCE_SYS_STATUS_INFO *sysStsInfo);

/*###########################    DEBUG PRINT  ##############################*/

void SFM_SCE_PRINT(SFM_SCE *sce); /*SCE Main*/
void SFM_SCEDev_PRINT(SFM_SCEDev *sceDev);
void SCE_SYS_INFO_PRINT(SCE_SYS_INFO *sysInfo);
void SCE_SYS_USAGE_INFO_PRINT(SCE_SYS_USAGE_INFO *sysUseInfo);
/* hjjung */
void SCE_SYS_NUM_INFO_PRINT(SCE_SYS_NUM_INFO *sysNumInfo);
void SCE_SYS_STATUS_INFO_PRINT(SCE_SYS_STATUS_INFO *sysStsInfo);
/* hjjung */
void SFM_LEG_CPS_H2N (SFM_CALL *pCallData);
void SCE_FLOW_INFO_N2H(SCE_FLOW_INFO *flow);
void SFM_TPS_CALL_H2N (SFM_CALL *pCallData); // added by dcham 20110525 for TPS

