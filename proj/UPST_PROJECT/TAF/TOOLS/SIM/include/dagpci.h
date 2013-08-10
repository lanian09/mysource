#ifndef __KERNEL__ 
#ifndef _WIN32
# include <inttypes.h>
#else
#include <wintypedefs.h> 
#endif /* _WIN32 */
#endif

#ifndef DAGPCI_H
#define DAGPCI_H

#define PCI_VENDOR_ID_DAG	0x121b
#define PCI_VENDOR_ID_ENDACE	0xeace

/* DAG card PCI Device IDs */
#define PCI_DEVICE_ID_DAG3_20	0x3200
#define PCI_DEVICE_ID_DAG3_50	0x3500
#define PCI_DEVICE_ID_DAG3_50E	0x350E
#define PCI_DEVICE_ID_DAG3_60D	0x360D
#define PCI_DEVICE_ID_DAG3_60E	0x360E
#define PCI_DEVICE_ID_DAG3_68E	0x368E
#define PCI_DEVICE_ID_DAG3_70T	0x3707
#define PCI_DEVICE_ID_DAG3_78E	0x378E
#define PCI_DEVICE_ID_DAG3_80	0x3800

#define PCI_DEVICE_ID_DAG4_10	0x4100
#define PCI_DEVICE_ID_DAG4_11	0x4110
#define PCI_DEVICE_ID_DAG4_20	0x4200
#define PCI_DEVICE_ID_DAG4_23	0x4230
#define PCI_DEVICE_ID_DAG4_22E	0x422E
#define PCI_DEVICE_ID_DAG4_23E	0x423E
#define PCI_DEVICE_ID_DAG4_30	0x4300
#define PCI_DEVICE_ID_DAG4_30E	0x430E
#define PCI_DEVICE_ID_DAG4_40E	0x440E

#define PCI_DEVICE_ID_DAG6_00	0x6000
#define PCI_DEVICE_ID_DAG6_10	0x6100
#define PCI_DEVICE_ID_DAG6_20	0x6200

#define PCI_DEVICE_ID_DAG7_10	0x7100
#define PCI_DEVICE_ID_DAG7_00E	0x700E

#define PCI_DEVICE_ID_DAGX1_00	0xa100

/* DAG Copro device ID codes */
#define COPRO_DEVICE_ID_NONE	0
#define COPRO_DEVICE_ID_SC128	255
#define COPRO_DEVICE_ID_SC256	254
#define COPRO_DEVICE_ID_PROTO	240

/* Coprocessor type codes. */
typedef enum
{
	kCoprocessorNotFitted,
	kCoprocessorPrototype, /* HW Rev 1 (Prototype!), Prior to Sep 2003 */
	kCoprocessorSC128,     /* HW Rev 2, Sep/Oct 2003 or newer */
	kCoprocessorSC256,     /* IDT copro 2005 or newer */
	kCoprocessorUnknown,
	kCoprocessorNotSupported

} coprocessor_t;

#ifdef __KERNEL__
extern struct pci_device_id dag_pci_tbl[];
#endif

#if defined(DAGC_EXPORTS) 
typedef __int32 uint32_t;
#endif /* DAGC_EXPORTS */

char * dag_device_name(uint32_t device, uint32_t flag);
char * dag_copro_name(coprocessor_t device, uint32_t flag);


#endif /* DAGPCI_H */
