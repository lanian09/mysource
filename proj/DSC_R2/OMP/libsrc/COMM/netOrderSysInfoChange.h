#ifndef __NET_ORDER_SYSINFO_CHANGE__
#define __NET_ORDER_SYSINFO_CHANGE__
#include "sfmconf.h"
/* ########################### Network To  Host ############################## */

void SFM_SysInfo_N2H(SFM_SysInfo *sysMsg);
void SFM_SysAlmInfo_N2H(SFM_SysAlmInfo *sysMsg);
void SFM_SysCommInfo_N2H(SFM_SysCommInfo *sysInfo);
void SFM_CpuInfo_N2H(SFM_CpuInfo *cpuInfo);
void SFM_MemInfo_N2H(SFM_MemInfo *memInfo);
void SFM_DiskInfo_N2H(SFM_DiskInfo *diskInfo);
void SFM_LanInfo_N2H(SFM_LanInfo *LanInfo);
void SFM_ProcInfo_N2H(SFM_ProcInfo *procInfo);
void SFM_QueInfo_N2H(SFM_QueInfo *queInfo);
void SFM_LanInfo_N2H(SFM_LanInfo *lanInfo);
void SFM_SysDupSts_N2H(SFM_SysDupSts *sysDupSts);
void SFM_SysSuccRate_N2H(SFM_SysSuccRate *sysSuccRate);
void SFM_SysDBSts_N2H(SFM_SysDBSts *sysDBSts);
void SFM_SysRSRCInfo_N2H(SFM_SysRSRCInfo *sysRSRCInfo);
void SFM_NTPSts_N2H(SFM_NTPSts *NTPSts);
void SFM_SysSpecInfo_N2H(SFM_SysSpecInfo *SysSpecInfo);
void SFM_HpUxHWInfo_N2H(SFM_HpUxHWInfo *hpUxHWInfo);
void SFM_HpUxHWInfo_s_N2H(SFM_HpUxHWInfo_s *HpUxHWInfo_s);
void SFM_SpecInfoSMS_N2H(SFM_SpecInfoSMS *diskInfo);
void SFM_SysSuccRateIpInfo_N2H(SFM_SysSuccRateIpInfo *SuccRateIpInfo);
void SuccRateIpInfo_N2H(SuccRateIpInfo *rateIpInfo);
void SuccRate_RadiusInfo_N2H(SuccRate_RadiusInfo *radiusInfo);
void SFM_HwAlmFlag_N2H(SFM_HwAlmFlag *HwAlmFlag);
void SFM_HwAlmFlag_s_N2H(SFM_HwAlmFlag_s *hwAlmFlag_s);
void SFM_NMSInfo_N2H(SFM_NMSInfo *NMSInfo);


/* ########################### Host To  Network ############################## */

void SFM_SysInfo_H2N(SFM_SysInfo *sysMsg);
void SFM_SysAlmInfo_H2N(SFM_SysAlmInfo *sysMsg);
void SFM_SysCommInfo_H2N(SFM_SysCommInfo *sysInfo);
void SFM_CpuInfo_H2N(SFM_CpuInfo *cpuInfo);
void SFM_MemInfo_H2N(SFM_MemInfo *memInfo);
void SFM_DiskInfo_H2N(SFM_DiskInfo *diskInfo);
void SFM_LanInfo_H2N(SFM_LanInfo *LanInfo);
void SFM_ProcInfo_H2N(SFM_ProcInfo *procInfo);
void SFM_QueInfo_H2N(SFM_QueInfo *queInfo);
void SFM_LanInfo_H2N(SFM_LanInfo *lanInfo);
void SFM_SysDupSts_H2N(SFM_SysDupSts *sysDupSts);
void SFM_SysSuccRate_H2N(SFM_SysSuccRate *sysSuccRate);
void SFM_SysDBSts_H2N(SFM_SysDBSts *sysDBSts);
void SFM_SysRSRCInfo_H2N(SFM_SysRSRCInfo *sysRSRCInfo);
void SFM_NTPSts_H2N(SFM_NTPSts *NTPSts);
void SFM_SysSpecInfo_H2N(SFM_SysSpecInfo *SysSpecInfo);
void SFM_HpUxHWInfo_H2N(SFM_HpUxHWInfo *hpUxHWInfo);
void SFM_HpUxHWInfo_s_H2N(SFM_HpUxHWInfo_s *HpUxHWInfo_s);
void SFM_SpecInfoSMS_H2N(SFM_SpecInfoSMS *diskInfo);
void SFM_SysSuccRateIpInfo_H2N(SFM_SysSuccRateIpInfo *SuccRateIpInfo);
void SuccRateIpInfo_H2N(SuccRateIpInfo *rateIpInfo);
void SuccRate_RadiusInfo_H2N(SuccRate_RadiusInfo *radiusInfo);
void SFM_HwAlmFlag_H2N(SFM_HwAlmFlag *HwAlmFlag);
void SFM_HwAlmFlag_s_H2N(SFM_HwAlmFlag_s *hwAlmFlag_s);
void SFM_NMSInfo_H2N(SFM_NMSInfo *NMSInfo);


/* ########################### DEBUG PRINT ############################## */

void SFM_SysInfo_PRINT(SFM_SysInfo *sysMsg);
void SFM_SysAlmInfo_PRINT(SFM_SysAlmInfo *sysMsg);
void SFM_SysCommInfo_PRINT(SFM_SysCommInfo *sysInfo);
void SFM_CpuInfo_PRINT(SFM_CpuInfo *cpuInfo);
void SFM_MemInfo_PRINT(SFM_MemInfo *memInfo);
void SFM_DiskInfo_PRINT(SFM_DiskInfo *diskInfo);
void SFM_LanInfo_PRINT(SFM_LanInfo *LanInfo);
void SFM_ProcInfo_PRINT(SFM_ProcInfo *procInfo);
void SFM_QueInfo_PRINT(SFM_QueInfo *queInfo);
void SFM_LanInfo_PRINT(SFM_LanInfo *lanInfo);
void SFM_SysDupSts_PRINT(SFM_SysDupSts *sysDupSts);
void SFM_SysSuccRate_PRINT(SFM_SysSuccRate *sysSuccRate);
void SFM_SysDBSts_PRINT(SFM_SysDBSts *sysDBSts);
void SFM_SysRSRCInfo_PRINT(SFM_SysRSRCInfo *sysRSRCInfo);
void SFM_NTPSts_PRINT(SFM_NTPSts *NTPSts);
void SFM_SysSpecInfo_PRINT(SFM_SysSpecInfo *SysSpecInfo);
void SFM_HpUxHWInfo_PRINT(SFM_HpUxHWInfo *hpUxHWInfo);
void SFM_HpUxHWInfo_s_PRINT(SFM_HpUxHWInfo_s *HpUxHWInfo_s);
void SFM_SpecInfoSMS_PRINT(SFM_SpecInfoSMS *diskInfo);
void SFM_SysSuccRateIpInfo_PRINT(SFM_SysSuccRateIpInfo *SuccRateIpInfo);
void SuccRateIpInfo_PRINT(SuccRateIpInfo *rateIpInfo);
void SuccRate_RadiusInfo_PRINT(SuccRate_RadiusInfo *radiusInfo);
void SFM_HwAlmFlag_PRINT(SFM_HwAlmFlag *HwAlmFlag);
void SFM_HwAlmFlag_s_PRINT(SFM_HwAlmFlag_s *hwAlmFlag_s);
void SFM_NMSInfo_PRINT(SFM_NMSInfo *NMSInfo);

#endif /*__NET_ORDER_SYSINFO_CHANGE__*/
