#ifndef __SNMPIF_H__
#define	__SNMPIF_H__

#define	MAX_SCAN_SIZE			32
#define	MAX_SUBSYS_COUNT		8

typedef struct _st_SubSys
{
	int				dType;		/*	Sub System Type			*/
	int				dNo;		/*	Sub System Number		*/
	unsigned int	uiBIP;		/*	Sub Sustem IP Address	*/
	int				dFlag;		/*	Sub System Active Flag	*/
} st_SubSys, *pst_SubSys;

typedef struct _st_SubSysList
{
    int			dCount;
    st_SubSys	stSubSys[MAX_SUBSYS_COUNT];
} st_SubSysList, *pst_SubSysList;

#endif	/*	__SNMP_H__	*/
