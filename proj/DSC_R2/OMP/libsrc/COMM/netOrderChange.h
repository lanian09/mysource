#ifndef __NET_ORDER_CHANGE__
#define __NET_ORDER_CHANGE__

#include "sfm_msgtypes.h"

/*########################### Network To Host ##############################*/
void SFM_SysCommMsgType_N2H(SFM_SysCommMsgType *sysMsg); /*SystemCommon Main*/

void SFM_SysCommProcSts_N2H(SFM_SysCommProcSts *sysProc);
void SFM_SysCommDiskSts_N2H(SFM_SysCommDiskSts *sysDisk);
void SFM_SysCommLanSts_N2H(SFM_SysCommLanSts *sysLan);
void SFM_SysCommQueSts_N2H(SFM_SysCommQueSts *sysQue);
void SFM_SysCommLinkSts_N2H(SFM_SysCommLinkSts *sysLink);
void SFM_SysDuplicationSts_N2H(SFM_SysDuplicationSts *sysDup);
void SFM_SysDBConnSts_N2H(SFM_SysDBConnSts *sysDup);
void SFM_SysSuccessRate_N2H(SFM_SysSuccessRate *sysRate);
void SFM_SysIFBond_N2H(SFM_SysIFBond *sysBond);
void SFM_SysRsrcLoad_N2H(SFM_SysRsrcLoad *sysRsrc);
void SFM_SysSts_N2H(SFM_SysSts *sysSts);
void SFM_diskSts_N2H(SFM_diskSts *diskSts);
void SFM_cpuSts_N2H(SFM_cpuSts *cpuSts);
void SFM_linkSts_N2H(SFM_linkSts *linkSts);
void SFM_pwrSts_N2H(SFM_pwrSts *pwrSts);
void SFM_fanSts_N2H(SFM_fanSts *fanSts);
void SFM_Duia_N2H(SFM_Duia *sysDuia);

/*########################### Host To Network  ##############################*/
void SFM_SysCommMsgType_H2N(SFM_SysCommMsgType *sysMsg); /*SystemCommon Main*/

void SFM_SysCommProcSts_H2N(SFM_SysCommProcSts *sysProc);
void SFM_SysCommDiskSts_H2N(SFM_SysCommDiskSts *sysDisk);
void SFM_SysCommLanSts_H2N(SFM_SysCommLanSts *sysLan);
void SFM_SysCommQueSts_H2N(SFM_SysCommQueSts *sysQue);
void SFM_SysCommLinkSts_H2N(SFM_SysCommLinkSts *sysLink);
void SFM_SysDuplicationSts_H2N(SFM_SysDuplicationSts *sysDup);
void SFM_SysDBConnSts_H2N(SFM_SysDBConnSts *sysDup);
void SFM_SysSuccessRate_H2N(SFM_SysSuccessRate *sysRate);
void SFM_SysIFBond_H2N(SFM_SysIFBond *sysBond);
void SFM_SysRsrcLoad_H2N(SFM_SysRsrcLoad *sysRsrc);
void SFM_SysSts_H2N(SFM_SysSts *sysSts);
void SFM_diskSts_H2N(SFM_diskSts *diskSts);
void SFM_cpuSts_H2N(SFM_cpuSts *cpuSts);
void SFM_linkSts_H2N(SFM_linkSts *linkSts);
void SFM_pwrSts_H2N(SFM_pwrSts *pwrSts);
void SFM_fanSts_H2N(SFM_fanSts *fanSts);
void SFM_Duia_H2N(SFM_Duia *sysDuia);


#ifdef __DEBUG__
/*########################### Print SFM member value ##############################*/
void SFM_SysCommMsgType_PRINT(SFM_SysCommMsgType *sysMsg); /*SystemCommon Main*/

void SFM_SysCommProcSts_PRINT(SFM_SysCommProcSts *sysProc);
void SFM_SysCommDiskSts_PRINT(SFM_SysCommDiskSts *sysDisk);
void SFM_SysCommLanSts_PRINT(SFM_SysCommLanSts *sysLan);
void SFM_SysCommQueSts_PRINT(SFM_SysCommQueSts *sysQue);
void SFM_SysCommLinkSts_PRINT(SFM_SysCommLinkSts *sysLink);
void SFM_SysDuplicationSts_PRINT(SFM_SysDuplicationSts *sysDup);
void SFM_SysDBConnSts_PRINT(SFM_SysDBConnSts *sysDup);
void SFM_SysSuccessRate_PRINT(SFM_SysSuccessRate *sysRate);
void SFM_SysIFBond_PRINT(SFM_SysIFBond *sysBond);
void SFM_SysRsrcLoad_PRINT(SFM_SysRsrcLoad *sysRsrc);
void SFM_SysSts_PRINT(SFM_SysSts *sysSts);
void SFM_diskSts_PRINT(SFM_diskSts *diskSts);
void SFM_cpuSts_PRINT(SFM_cpuSts *cpuSts);
void SFM_linkSts_PRINT(SFM_linkSts *linkSts);
void SFM_pwrSts_PRINT(SFM_pwrSts *pwrSts);
void SFM_fanSts_PRINT(SFM_fanSts *fanSts);
void SFM_Duia_PRINT(SFM_Duia *sysDuia);
#endif /* __DEBUG__ */

#endif /*__NET_ORDER_CHANGE__*/
