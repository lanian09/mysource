#ifndef SAMD_NTP_H
#define SAMD_NTP_H

#define NTP_INITIAL		0x00
#define NTP_NORMAL		0x01
#define NTP_CRITICAL	0x02

typedef struct _st_NTP {
	unsigned char	ucSymF;
	unsigned char	ucTF;
	unsigned char	ucResev[6];
	unsigned short	usWhen;
	unsigned short	usPoll;
	unsigned short	usReach;
	unsigned short	usST;
	float			fDelay;
	float			fOffset;
	float			fJitter;
	float			fResefv;
	char			szIP[32];
	char			szRefIP[32];
} st_NTP, *pst_NTP;

typedef struct _st_NTP_STS {
	int		dNtpCnt;
	int		dReserv;
	st_NTP	stNTP[32];
} st_NTP_STS, *pst_NTP_STS;

#endif /* SAMD_NTP_H */
