#include "sfm_snmp.h"

/*########################### Network To Host ##############################*/
/*SFM_L3PD Main*/
void SFM_L3PD_N2H(SFM_L3PD *l3pd) ;
void SFM_L3ProbeDev_N2H(SFM_L3ProbeDev *L3ProbeDev);
void SFM_PDCpuInfo_N2H(SFM_PDCpuInfo *cpuInfo);
void SFM_PDMemInfo_N2H(SFM_PDMemInfo *memInfo);
void SFM_PdHwAlmFlag_s_N2H(SFM_PdHwAlmFlag_s * hwAlmflag);
void SFM_PDFanInfo_N2H(SFM_PDFanInfo *fanInfo);
void SFM_PDGigaLanInfo_N2H(SFM_PDGigaLanInfo * gigaLanInfo);

/*########################### Host To Network ##############################*/
/*SFM_L3PD Main*/
void SFM_L3PD_H2N(SFM_L3PD *l3pd) ;
void SFM_L3ProbeDev_H2N(SFM_L3ProbeDev *L3ProbeDev);
void SFM_PDCpuInfo_H2N(SFM_PDCpuInfo *cpuInfo);
void SFM_PDMemInfo_H2N(SFM_PDMemInfo *memInfo);
void SFM_PdHwAlmFlag_s_H2N(SFM_PdHwAlmFlag_s * hwAlmflag);
void SFM_PDFanInfo_H2N(SFM_PDFanInfo *fanInfo);
void SFM_PDGigaLanInfo_H2N(SFM_PDGigaLanInfo * gigaLanInfo);


void SFM_L2DEV_H2N(SFM_L2Dev *l2dev);
