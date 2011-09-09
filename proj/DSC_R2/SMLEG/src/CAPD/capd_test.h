
#ifndef __CAPD_TEST_H__
#define __CAPD_TEST_H__

/**A.1*  File Inclusion *******************************************************/
#include <dirent.h>

/**B.1*  Definition of New Constants ******************************************/
#define STR_EXT_CAP			"cap"
#define PCAP_FILE_HDR_SIZE	24

#define DEF_CONTINUE		1
#define DEF_END			100

/**B.2*  Definition of New Type  **********************************************/
typedef struct _st_data_header_
{
	//unsigned long long	dwTime;
	unsigned int		capTime;
	unsigned int		capMTime;
	unsigned int		dwLen;
	unsigned int		dwLen2;
} st_PcapHdr_t, *pst_PcapHdr_t;
#define PCAP_DATA_HDR_SIZE	sizeof(st_PcapHdr_t)

typedef struct _st_data_order_
{
	char capTime[4];
	char capMTime[4];
	char dwLen[4];
	char dwLen2[4];
} st_PcapOrder_t, *pst_PcapOrder_t;
#define PCAP_DATA_ORDER_SIZE	sizeof(st_PcapOrder_t)

/**C.1*  Declaration of Variables  ********************************************/

/**C.1*  Declaration of Variables  ********************************************/

/**D.1*  Definition of Functions  *********************************************/
int	parse_data(st_PcapHdr_t *pstHeader, char *szBuff);
int	FilterSOAP(const struct dirent *entry);
void test_func2(char *szDirName, int SLEEPCNT);

#endif /* __CAPD_TEST_H__ */
