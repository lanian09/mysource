/*
 * Copyright (c) 2004-2005 Endace Technology Ltd, Hamilton, New Zealand.
 * All rights reserved.
 *
 * This source code is proprietary to Endace Technology Limited and no part
 * of it may be redistributed, published or disclosed except as outlined in
 * the written contract supplied with this product.
 *
 * $Id: daginf.h,v 1.1.1.1 2011/08/29 05:56:42 dcham Exp $
 */

#ifndef DAGINF_H
#define DAGINF_H

#if defined(__linux__)

#ifndef __KERNEL__
#include <inttypes.h>  /* The Linux kernel has its own stdint types. */
#include <time.h>
#else
#include <linux/kernel.h>
#endif /* __KERNEL__ */

#elif defined(__FreeBSD__)

#ifndef _KERNEL
#include <inttypes.h>
#else
#include <sys/inttypes.h>  /* So does the FreeBSD kernel. */
#endif /* _KERNEL */
#include <time.h>

#elif defined(_WIN32)

#include <wintypedefs.h>
#include <time.h>

#elif defined(__SVR4) && defined(__sun)

#include <sys/types.h>

#elif defined(__APPLE__) && defined(__ppc__)

#include <inttypes.h>

#else
#error Compiling on an unsupported platform - please contact <support@endace.com> for assistance.
#endif /* Platform-specific code. */

typedef struct daginf {
	uint32_t		id;		/* DAG device number */
	uint32_t		phy_addr;	/* PCI address of large buffer */
	uint32_t		buf_size;	/* its size */
	uint32_t		iom_size;	/* iom size */
	uint16_t		device_code;	/* PCI device ID */
} daginf_t;

/* Bitfields for DAG_IOSETDUCK, Set_Duck_Field parameter */
/* Processing order is impled numerically ascending */
#define Set_Duck_Clear_Stats	0x0001
#define Set_Duck_Health_Thresh	0x0002

typedef struct duckinf
{
	uint32_t	Crystal_Freq;
	uint32_t	Synth_Freq;
	uint64_t	Last_Ticks;
	uint32_t	Resyncs;
	uint32_t	Bad_Pulses;
	uint32_t	Worst_Freq_Err, Worst_Phase_Err;
	uint32_t	Health_Thresh;
	uint32_t	Pulses, Single_Pulses_Missing, Longest_Pulse_Missing;
	uint32_t	Health, Sickness;
	int32_t		Freq_Err, Phase_Err;
	uint32_t	Set_Duck_Field;
	time_t		Stat_Start, Stat_End;
	uint64_t        Last_TSC;
} duckinf_t;

#endif /* DAGINF_H */
