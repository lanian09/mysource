#ifndef __CHSMD_HW_H__
#define __CHSMD_HW_H__

#define FILE_MESSAGE	"/var/log/messages"
#define FILE_FAN		"/proc/cpqfan"

#define POW_ENCLO_UP	"D302JTK7D008"
#define POW_ENCLO_DN	"D302JTK7D014"

#define LSIZE           256

#define TYPE_DAG3_80S	0	/* Iu-PS */
#define TYPE_DAG3_70G	1	/* Gn,Gi */
#define TYPE_DAG4_3GE	2	/* Gn,Gi add 2006.08.10 */
#define TYPE_DAG4_5GE	3
#define TYPE_DAG7_5G2   4

#define PORT_STATUS_UP  1	/* DAG STATUS : PORT ALIVE */

extern int dGetHWSts(char *flag, int cnt, int type);
extern int dGetDagSts(char *flag, int cnt, int type);
extern int dCheckMsgStatus(void);
extern int dCheckDagType(void);
extern int Init_Enclosure(void);
extern int dCheckHW(void);


#endif /* __CHSMD_HW_H__ */
