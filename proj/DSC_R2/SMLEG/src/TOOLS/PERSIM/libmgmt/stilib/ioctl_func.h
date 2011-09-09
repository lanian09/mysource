/*
 * Author : albam <albamc@gmail.com>
 * Copyright 2005 albam
 *
 * Local ioctl functions
 *
 * This program is NOT-free software; You can redistribute it
 * and/or modify it under the terms of the Albam's General Public
 * License (AGPL) as published by the albam.
 *
 * Albam's General Public License (AGPL)
 * DO NOT USE OR MODIFY MY SOURCES!!!
 *
 * Changes :
 *  - 2005/12/05 albam <albamc@gmail.com> : make initial version.
 */

#ifndef _IOCTL_FUNC_H_
#define _IOCTL_FUNC_H_

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <linux/if.h>
#include <linux/if_arp.h>
#include <linux/if_bonding.h>
#include <linux/route.h>
#include <linux/sockios.h>

#ifndef U32
#define U32
typedef __uint32_t u32;
#endif
#ifndef U16
#define U16
typedef __uint16_t u16;
#endif
#ifndef U8
#define U8
typedef __uint8_t u8;
#endif

#include <linux/ethtool.h>
#include <linux/mii.h>

static inline void
_saddr(struct sockaddr_in* sin, uint32_t addr, uint16_t port)
{
	sin->sin_family = AF_INET;
	sin->sin_addr.s_addr = addr;
	sin->sin_port = port;
}

static inline void
_shwaddr(struct sockaddr* sa, char* hwaddr)
{
	sa->sa_family = ARPHRD_ETHER;
	memcpy(sa->sa_data, hwaddr, IFHWADDRLEN);
}

int sioc_addrt(uint32_t, uint32_t, uint32_t, char*);
int sioc_delrt(uint32_t, uint32_t, uint32_t, char*);
int sioc_gifname(int, char*);
int sioc_gifconf(char*, int);
uint16_t sioc_gifflags(char*);
int sioc_sifflags(char*, uint16_t);
uint32_t sioc_gifaddr(char*);
int sioc_sifaddr(char*, uint32_t);
uint32_t sioc_gifdstaddr(char*);
int sioc_sifdstaddr(char*, uint32_t);
uint32_t sioc_gifbrdaddr(char*);
int sioc_sifbrdaddr(char*, uint32_t);
uint32_t sioc_gifnetmask(char*);
int sioc_sifnetmask(char*, uint32_t);
int sioc_gifmtu(char*);
int sioc_sifmtu(char*, int);
int sioc_sifname(char*, char*);
int sioc_sifhwaddr(char*, char*);
int sioc_gifhwaddr(char*, char*);
int sioc_gifindex(char*);
int sioc_sifhwbroadcast(char* name, char* addr);
int sioc_giftxqlen(char* name);
int sioc_siftxqlen(char* name, int qlen);
int sioc_ethtool(char* name, struct ethtool_drvinfo* info, uint32_t cmd, char* driver);
int sioc_gmiiphy(char* name, struct mii_ioctl_data* data);
int sioc_gmiireg(char* name, struct mii_ioctl_data* data);
int sioc_smiireg(char* name, struct mii_ioctl_data* data);
int sioc_darp(char*, uint32_t, char*, uint32_t);
int sioc_garp(char*, uint32_t, char*);
int sioc_sarp(char*, uint32_t, char*, int, uint32_t);
int sioc_bondenslave(char*master, char* slave);
int sioc_bondrelease(char* master, char* slave);
int sioc_bondsethwaddr(char* master, char* slave);
int sioc_bondchangeactive(char* master, char* slave);
int sioc_devprivate(char* dev, uint8_t* p, int n);

#endif

